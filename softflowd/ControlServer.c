/*

  Copyright (C) 2010  INRIA, Planète Team

  Authors: 
  ------------------------------------------------------------
  Amir Krifa			:  Amir.Krifa@sophia.inria.fr
  Imed Lassoued	            :  Imed.Lassoued@sophia.inria.fr
  Chadi Barakat			:  Chadi.Barakat@sophia.inria.fr
  ------------------------------------------------------------

This program is free software; you can redistribute it and/or
modify it under the terms of the GNU General Public License version 3
as published by the Free Software Foundation.

This program is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
GNU General Public License for more details.

You should have received a copy of the GNU General Public License
along with this program; if not, write to the Free Software
Foundation, Inc., 59 Temple Place - Suite 330, Boston, MA  02111-1307, USA.
*/

#include "ControlServer.h"
#include <stdio.h>
#include <sys/socket.h> 
#include <arpa/inet.h>  
#include <stdlib.h>    
#include <string.h>    
#include <unistd.h>    
#include <pthread.h>
#include "sys-tree.h"
#include "convtime.h"
#include "treetype.h"
#include "common.h"
#include "softflowd.h"
#include "SamplingModule.h"

int StopsControlServer = 0; 
extern sem_t stop_control_server;
int control_server_listening_port = CONTROL_SERVER_DEFAULT_PORT;

void SetStopsControlServer()
{
	sem_wait(&stop_control_server);
		StopsControlServer = 1;
	sem_post(&stop_control_server);
}

int GetStopsControlServer()
{
	int ret;
	sem_wait(&stop_control_server);
		ret= StopsControlServer;
	sem_post(&stop_control_server);
	return ret;
}

int StartControlServer()
{
	
	csThreadParam * ptrParam= (struct csThreadParam*)malloc(sizeof(struct csThreadParam));
	if(ptrParam==NULL)
		logit(LOG_ERR,"%s:%i: Memory allocation problem.",__FILE__,__LINE__);

	pthread_attr_t threadAttr;
	pthread_t controlThread;

	if (pthread_attr_init (&threadAttr) != 0) 
	{
		logit(LOG_ERR,"%s:%i: pthread_attr_init error.",__FILE__,__LINE__);
	}

	if (pthread_attr_setdetachstate (&threadAttr,PTHREAD_CREATE_DETACHED) != 0) 
	{
		logit(LOG_ERR,"%s:%i: pthread_attr_setdetachstate error.",__FILE__,__LINE__);
	}
		
	if (pthread_create (&controlThread, &threadAttr, ThreadProcessFunction, (void *)ptrParam) < 0) 
	{
		logit(LOG_ERR,"%s:%i: pthread_create error.",__FILE__,__LINE__);
	}
}

void SendEndMsg()
{
	int sock;
	struct sockaddr_in server;
	
	if ((sock = socket(AF_INET, SOCK_STREAM, 0)) < 0)
	{
		fprintf(stderr, "%s:%i Opening stream socket.\n",__FILE__, __LINE__);
		exit(-1);
	}

	bzero((char *) &server, sizeof(server));
	server.sin_family = AF_INET;
	inet_aton("127.0.0.1", &server.sin_addr);
	server.sin_port = htons((u_short) control_server_listening_port);
      
	if (connect(sock, (struct sockaddr *) &server, sizeof(server)) < 0)
	{
		fprintf(stderr, "%s:%i A problem occured while trying to connect to the server.\n",__FILE__, __LINE__);
		close(sock);
		return -1;
	}
      
	// Sending the control message
	if (write(sock, "end", sizeof("end")) < 0)
	{
		fprintf(stderr, "%s:%i A problem occured while trying to send the end message.\n",__FILE__, __LINE__);
		close(sock);

	}
	else 
	{
		printf("The end control message has been successfully sent.\n");
	}	
      
	close(sock);
}

static void  * ThreadProcessFunction(void *arg)
{
	int servSock; 
	int clntSock;
	struct sockaddr_in ServAddr;
	struct sockaddr_in ClntAddr;
	unsigned short ServPort;	
	unsigned int clntLen;

	csThreadParam * ptrParam= (struct csThreadParam*)(arg);
	if(ptrParam == NULL)
		logit(LOG_ERR,"%s:%i: Memory allocation problem.",__FILE__,__LINE__);

	ServPort = control_server_listening_port;

	if ((servSock = socket(PF_INET, SOCK_STREAM, IPPROTO_TCP)) < 0)
		logit(LOG_ERR,"%s:%i socket() failed",__FILE__,__LINE__);

    	memset(&ServAddr, 0, sizeof(ServAddr)); 
	ServAddr.sin_family = AF_INET;
	ServAddr.sin_addr.s_addr = htonl(INADDR_ANY); 
	ServAddr.sin_port = htons(ServPort); 

	if (bind(servSock, (struct sockaddr *) &ServAddr, sizeof(ServAddr)) < 0)
		logit(LOG_ERR,"%s:%i bind() failed",__FILE__,__LINE__);

	if (listen(servSock, MAXPENDING) < 0)
		logit(LOG_ERR,"%s:%i listen() failed",__FILE__,__LINE__);
	
	logit(LOG_DEBUG,"%s:%i Control server started on port: %i.",__FILE__,__LINE__, control_server_listening_port);
	while (1) 
	{
		/* Set the size of the in-out parameter */
		clntLen = sizeof(ClntAddr);
		if ((clntSock = accept(servSock, (struct sockaddr *) &ClntAddr,&clntLen)) < 0)
			logit(LOG_ERR,"%s:%i accept() failed",__FILE__,__LINE__);

		HandleTCPClient(clntSock);
		if(GetStopsControlServer())
			break;
	}
	logit(LOG_DEBUG,"The Control Server's thread is shutting down.\n");
	// Free the argument parameter 
	if(ptrParam != NULL)
		free(ptrParam);
	pthread_exit (0);
}

void HandleTCPClient(int clntSocket)
{
	char buf[CONTROL_BUFFER_SIZE];
	int numberR = -1;
	memset(buf, '\0', CONTROL_BUFFER_SIZE);
	if((numberR = read(clntSocket, buf,CONTROL_BUFFER_SIZE))  < 0)
	{
		logit(LOG_ERR,"%s:%i Error occured while reading the control message.\n", __FILE__, __LINE__);
		close(clntSocket);
	} else if(strcmp(buf, "end") != 0)
	{
		char monitorIp[16];
		char szSAmplingRateP1[20];
		char szSAmplingRateP2[20];
		memset(monitorIp, '\0', 16);
		memset(szSAmplingRateP1, '\0', 20);
		memset(szSAmplingRateP2, '\0', 20);
		
		int ret;
		ret = get_control_packet_part(buf, 2, monitorIp);
		if(ret < 0 )
		{
			logit(LOG_ERR,"%s:%i Error occured while parsing the control message.\n", __FILE__, __LINE__);
		}

		ret = get_control_packet_part(buf, 0, szSAmplingRateP1);
		if(ret < 0 )
		{
			logit(LOG_ERR,"%s:%i Error occured while parsing the control message.\n", __FILE__, __LINE__);
		}

		ret = get_control_packet_part(buf, 1, szSAmplingRateP2);
		if(ret < 0 )
		{
			logit(LOG_ERR,"%s:%i Error occured while parsing the control message.\n", __FILE__, __LINE__);
		}

		// Assign The new sampling rate
		if(ChangeSamplingRateInNode(atoi(szSAmplingRateP1), atoi(szSAmplingRateP2), monitorIp) != 0)
		{
			// Error occured while trying to change the sampling rate
			if (write(clntSocket, "Error occured while trying to change the sampling rate, invalid monitor address.", sizeof("Error occured while trying to change the sampling rate, invalid monitor address.")) < 0)
			{
				fprintf(stderr, "%s:%i A problem occured while trying to send the ack message.\n",__FILE__, __LINE__);
				close(clntSocket);
			}
		}else
		{
			if (write(clntSocket, "The sampling rate is updated.", sizeof("The sampling rate is updated.")) < 0)
			{
				fprintf(stderr, "%s:%i A problem occured while trying to send the ack message.\n",__FILE__, __LINE__);
				close(clntSocket);
			}
		}
		close(clntSocket);

	} else if(strcmp(buf, "end") == 0)
	{
		close(clntSocket);
		SetStopsControlServer();
	}
}

/*
* Function used to extract the different parts of a packet received from the emulator
*/
static int get_control_packet_part(char *complete_packet,int part_number,char * resulting_part)
{
	if(part_number <= MAX_CONTROL_MESSAGE_PARTS - 1)
	{
		/* Looking for a field from the network layer */
		int i=0, offset=0;
		int error=0;
		char * sep_position=NULL,* prev_sep_position=NULL;

		if(part_number == 0)
		{
			sep_position = strchr(complete_packet,'#');
			if(strncpy(resulting_part,complete_packet,sep_position-complete_packet) == NULL)
			{
				logit(LOG_ERR,"%s:%i Error occured while parsing the received data from the emulator.\n",__FILE__,__LINE__);
				return -1;
			}
			else return 0;
		}
		
		prev_sep_position = complete_packet;
		sep_position = complete_packet;

		for(;i <= part_number;i++)
		{
			prev_sep_position = sep_position;
			if(i == MAX_CONTROL_MESSAGE_PARTS -1 )
			{
				/*The last part of the message*/
				
				if(strncpy(resulting_part, prev_sep_position+1, (complete_packet+ strlen(complete_packet))-prev_sep_position-1) == NULL)
				{
					logit(LOG_ERR,"%s:%i Error occured while parsing the received data from the emulator.\n",__FILE__,__LINE__);
						return -1;
				}
				else return 0;
			}else if((sep_position=strchr(prev_sep_position+1,'#'))==NULL)
			{
				error=1;	
				break;
			}
		}

		if(!error)
		{
			if(strncpy(resulting_part,prev_sep_position+1,sep_position-prev_sep_position-1)==NULL)
			{
				logit(LOG_ERR,"%s:%i Error occured while parsing the received data %s from the emulator; part: %i.\n", __FILE__, __LINE__, complete_packet, part_number);
				return -1;
			}
			else return 0;
		} else 
			{
				logit(LOG_ERR,"%s:%i Error occured while parsing the received data %s from the emulator; part: %i.\n", __FILE__, __LINE__, complete_packet, part_number);
				return -1;
			}
	} else
		{
			logit(LOG_ERR,"%s:%i Error occured while parsing the received data %s from the emulator; part: %i.\n", __FILE__, __LINE__, complete_packet, part_number);
			return -1;
		}
}

