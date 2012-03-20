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

#include "SamplingManager.h"
#include <stdio.h>
#include <sys/socket.h> /* for socket(), bind(), and connect() */
#include <arpa/inet.h>  /* for sockaddr_in and inet_ntoa() */
#include <stdlib.h>     /* for atoi() and exit() */
#include <string.h>     /* for memset() */
#include <unistd.h>     /* for close() */
#include <pthread.h>
#include "CommonStructs.h"
#include "SamplingPlatform.h"
#include "exception.h"
#include "TransitNetwork.h"
namespace AmDev
{
int StopsControlServer = 0; 
sem_t stop_control_server;
int control_server_listening_port = CONTROL_SERVER_DEFAULT_PORT;


SamplingManager::SamplingManager(SamplingPlatform * sp)
{
try
{
	if(sp == NULL)
		throw NspException("Invalid Sampling Platform pointer",__FILE__, __LINE__);
	this->sp = sp;
	sem_init(&stop_control_server,0,1);
}
catch(exception &e)
{
	cout<<e.what()<<endl;
}
}

SamplingManager::~SamplingManager()
{
	sem_destroy(&stop_control_server);
}
void SamplingManager::SetStopsControlServer()
{
	sem_wait(&stop_control_server);
		StopsControlServer = 1;
	sem_post(&stop_control_server);
}

int SamplingManager::GetStopsControlServer()
{
	int ret;
	sem_wait(&stop_control_server);
		ret= StopsControlServer;
	sem_post(&stop_control_server);
	return ret;
}

int SamplingManager::StartControlServer()
{
try
{
	csThreadParam * ptrParam= (struct csThreadParam*)malloc(sizeof(struct csThreadParam));
	if(ptrParam==NULL)
		throw NspException("Memory allocation problem",__FILE__,__LINE__);
	ptrParam->sm = this;

	pthread_attr_t threadAttr;
	pthread_t controlThread;

	if (pthread_attr_init (&threadAttr) != 0) 
	{
		throw NspException("pthread_attr_init error.",__FILE__,__LINE__);
	}

	if (pthread_attr_setdetachstate (&threadAttr,PTHREAD_CREATE_DETACHED) != 0) 
	{
		throw NspException("pthread_attr_setdetachstate error.",__FILE__,__LINE__);
	}
		
	if (pthread_create (&controlThread, &threadAttr, ThreadProcessFunction, (void *)ptrParam) < 0) 
	{
		throw NspException("pthread_create error.",__FILE__,__LINE__);
	}
}
catch(exception &e)
{
	cout<<e.what()<<endl;
}
}

void SamplingManager::SendEndMsg()
{
try
{
	int sock;
	struct sockaddr_in server;
	
	if ((sock = socket(AF_INET, SOCK_STREAM, 0)) < 0)
	{
		throw NspException("Error occured while Opening stream socket.",__FILE__,__LINE__);
	}

	bzero((char *) &server, sizeof(server));
	server.sin_family = AF_INET;
	inet_aton("127.0.0.1", &server.sin_addr);
	server.sin_port = htons((u_short) control_server_listening_port);
      
	if (connect(sock, (struct sockaddr *) &server, sizeof(server)) < 0)
	{
		throw NspException("A problem occured while trying to connect to the server.",__FILE__,__LINE__);
		close(sock);
	}
      
	// Sending the control message
	if (write(sock, "end", sizeof("end")) < 0)
	{
		throw NspException("A problem occured while trying to send the end message.",__FILE__,__LINE__);
		close(sock);

	}
	else 
	{
		printf("The end control message has been successfully sent.\n");
	}	
      
	close(sock);
}
catch(exception&e)
{
	cout<<e.what()<<endl;
}
}

void  * SamplingManager::ThreadProcessFunction(void *arg)
{
try
{
	int servSock; 
	int clntSock;
	struct sockaddr_in ServAddr;
	struct sockaddr_in ClntAddr;
	unsigned short ServPort;	
	unsigned int clntLen;

	csThreadParam * ptrParam= (struct csThreadParam*)(arg);
	if(ptrParam == NULL)
		throw NspException("Memory allocation problem.",__FILE__,__LINE__);

	ServPort = control_server_listening_port;

	if ((servSock = socket(PF_INET, SOCK_STREAM, IPPROTO_TCP)) < 0)
		throw NspException("ocket() failed",__FILE__,__LINE__);

    	memset(&ServAddr, 0, sizeof(ServAddr)); 
	ServAddr.sin_family = AF_INET;
	ServAddr.sin_addr.s_addr = htonl(INADDR_ANY); 
	ServAddr.sin_port = htons(ServPort); 

	if (bind(servSock, (struct sockaddr *) &ServAddr, sizeof(ServAddr)) < 0)
		throw NspException("bind() failed",__FILE__,__LINE__);

	if (listen(servSock, MAXPENDING) < 0)
		throw NspException("listen() failed",__FILE__,__LINE__);
	
	fprintf(stdout,"\n%s:%i Control server started on port: %i.\n",__FILE__,__LINE__, control_server_listening_port);
	while (!ptrParam->sm->GetStopsControlServer()) 
	{
		/* Set the size of the in-out parameter */
		clntLen = sizeof(ClntAddr);
		if ((clntSock = accept(servSock, (struct sockaddr *) &ClntAddr,&clntLen)) < 0)
			throw NspException("accept() failed",__FILE__,__LINE__);

		ptrParam->sm->HandleTCPClient(clntSock);
		if(ptrParam->sm->GetStopsControlServer())
			break;
	}
	fprintf(stdout,"\nThe Control Server's thread is shutting down.\n");
	close(clntSock);
	close(servSock);
	// Free the argument parameter 
	if(ptrParam != NULL)
		free(ptrParam);
	pthread_exit (0);
}

catch(exception &e)
{
	cout<<e.what()<<endl;
}
}

void SamplingManager::HandleTCPClient(int clntSocket)
{
try
{	
	char buf[CONTROL_BUFFER_SIZE];
	int numberR = -1;
	memset(buf, '\0', CONTROL_BUFFER_SIZE);
	if((numberR = read(clntSocket, buf,CONTROL_BUFFER_SIZE))  < 0)
	{
		throw NspException("Error occured while reading the control message.",__FILE__,__LINE__);
		close(clntSocket);
	} else if(strcmp(buf, "end") != 0 && strncmp(buf, "GetPath",4) != 0)
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
			throw NspException("Error occured while parsing the control message.",__FILE__,__LINE__);
		}

		ret = get_control_packet_part(buf, 0, szSAmplingRateP1);
		if(ret < 0 )
		{
			throw NspException("Error occured while parsing the control message.",__FILE__,__LINE__);
		}

		ret = get_control_packet_part(buf, 1, szSAmplingRateP2);
		if(ret < 0 )
		{
			throw NspException("Error occured while parsing the control message.",__FILE__,__LINE__);
		}

		// Assign The new sampling rate
		
		if(sp->ChangeSamplingRate(string(monitorIp), atoi(szSAmplingRateP1), atoi(szSAmplingRateP2)) != 1)
		{
			// Error occured while trying to change the sampling rate
			if (write(clntSocket, "Error occured while trying to change the sampling rate, invalid monitor address.\n", sizeof("Error occured while trying to change the sampling rate, invalid monitor address.\n")) < 0)
			{
				fprintf(stderr, "%s:%i A problem occured while trying to send the ack message.\n",__FILE__, __LINE__);
				close(clntSocket);
			}
		}else
		{
			if (write(clntSocket, "The sampling rate is updated.", sizeof("The sampling rate is updated.")) < 0)
			{
				fprintf(stderr, "%s:%i A problem occured while trying to send the ack message.\n",__FILE__, __LINE__);
			}
		}
		
		close(clntSocket);

	} else if(strcmp(buf, "end") != 0 && strncmp(buf, "GetPath",4) == 0)
	{
		// PATH request received 
		char command[16];
		char asSource[20];
		char asDest[20];
		memset(command, '\0', 16);
		memset(asSource, '\0', 20);
		memset(asDest, '\0', 20);
		
		int ret;
		ret = get_control_packet_part(buf, 0, command);
		if(ret < 0 )
		{
			throw NspException("Error occured while parsing the control message.",__FILE__,__LINE__);
		}

		ret = get_control_packet_part(buf, 1, asSource);
		if(ret < 0 )
		{
			throw NspException("Error occured while parsing the control message.",__FILE__,__LINE__);
		}

		ret = get_control_packet_part(buf, 2, asDest);
		if(ret < 0 )
		{
			throw NspException("Error occured while parsing the control message.",__FILE__,__LINE__);
		}
		
		// TODO: we suppose that we have only one transit network for the moment with the number 0
		// It could be more than that and so, we will need to change the path request
		TransitNetwork * tn = sp->GetTransitNetworkNumber(0);
		string path;
		tn->GetInterfaceBasedPathPathFromTo(atoi(asSource), atoi(asDest), path);
		
		// Sending back the path
		if (write(clntSocket, (char*)path.c_str(), path.length()) < 0)
		{
			fprintf(stderr, "%s:%i A problem occured while trying to send the path.\n",__FILE__, __LINE__);
		}
		close(clntSocket);	

	}
	else if(strcmp(buf, "end") == 0)
	{
		close(clntSocket);
		SetStopsControlServer();
	}
}
catch(exception &e)
{
	cout<<e.what()<<endl;
}
}

/*
* Function used to extract the different parts of a packet received from the emulator
*/
int SamplingManager::get_control_packet_part(char *complete_packet,int part_number,char * resulting_part)
{
try
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
				throw NspException("Error occured while parsing the received data from the emulator.",__FILE__,__LINE__);
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
					throw NspException("Error occured while parsing the received data from the emulator.",__FILE__,__LINE__);
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
				throw NspException("Error occured while parsing the received data",__FILE__,__LINE__);
				
			}
			else return 0;
		} else 
			{
				throw NspException("Error occured while parsing the received data",__FILE__,__LINE__);
			}
	} else
		{
			throw NspException("Error occured while parsing the received data",__FILE__,__LINE__);
		}
}
catch(exception &e)
{
	cout<<e.what()<<endl;
}
}
}

