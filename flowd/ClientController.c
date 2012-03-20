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

#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <netdb.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "ClientController.h"
#include <unistd.h>

int ValidateSamplingRate(int p1, int p2)
{
	if(p1 > p2)
		return -1;
	if( p1 == p2 && p2 == 0)
		return -1;
	if(p1 != 0 && p2 == 0 )
		return -1;
	if(p1 != 0 && p2 == 0 )
		return -1;
	if( p1 < 0 || p2 < 0)
		return -1;
	return 0;
}

//returns -1 in case of error, 0 otherwise
/*
* remoteServer; the remote control server attached to softflowd.
* remotePort: the remote port where the control server is listening. 
* (p1, p2): the new sampling rate.
* monitorAdr: the monitor Adr, either in the emulator or the one of the machine where softflowd is running. 
*/
int SendControlPacket(char *remoteServer, int remotePort, int p1, int p2, char * monitorAdr )
{
	int sock;   
	struct sockaddr_in server;
	struct sockaddr_in client; 
	int clientLen; 
	struct hostent *hp; 
	char newSamplingRate[100];
	char buf[1024];
	char tmp[20];

	
	if ((sock = socket(AF_INET, SOCK_STREAM, 0)) < 0)
	{
		fprintf(stderr, "%s:%i Opening stream socket.\n",__FILE__, __LINE__);
		exit(-1);
	}

	if(ValidateSamplingRate(p1, p2) < 0)
	{
		// Invalid sampling rate
		fprintf(stderr,"Please specify a valid sampling rate.\n");
		return -1;
	}

	memset(newSamplingRate, '\0', 100);
	sprintf(tmp, "%i", p1);
	strcpy(newSamplingRate, tmp);
	strcat(newSamplingRate, "#");
	sprintf(tmp, "%i", p2);
	strcat(newSamplingRate, tmp);
	strcat(newSamplingRate, "#");
	strcat(newSamplingRate, monitorAdr);

	bzero((char *) &server, sizeof(server));
	server.sin_family = AF_INET;
	inet_aton(remoteServer, &server.sin_addr);
	server.sin_port = htons((u_short) remotePort);
      
	if (connect(sock, (struct sockaddr *) &server, sizeof(server)) < 0)
	{
		fprintf(stderr, "%s:%i A problem occured while trying to connect to the server.\n",__FILE__, __LINE__);
		close(sock);
		return -1;
	}
      
	// Sending the control message
	if (write(sock, newSamplingRate, sizeof(newSamplingRate)) < 0)
	{
		fprintf(stderr, "%s:%i A problem occured while trying to send the control message.\n",__FILE__, __LINE__);
		close(sock);
		return -1;
	}
	else 
	{
		printf("The sampling control message [%s] has been successfully sent.\n", newSamplingRate);
		printf("Waiting for the Ack ...\n");
		memset(buf, '\0', 1024);	
		if(read(sock, buf, 1024)  < 0)
		{
			fprintf(stderr, "%s:%i A problem occured while trying to read the ack.\n",__FILE__, __LINE__);
			close(sock);
			return -1;
		}
		printf("Ack: %s\n",buf);
	}	
      
	close(sock);
	return 0;

}



//returns -1 in case of error, 0 otherwise
/*
* remoteServer; the remote control server attached to softflowd.
* remotePort: the remote port where the control server is listening. 
* as1: The AS source.
* as2: The AS Dest.
* Path: returned path, the list of routers within the path' input interface,  exemple: 10.0.0.170#10.0.0.182#10.0.0.101# 
*/
int SendPathRequestPacket(char *remoteServer, int remotePort, int as1, int as2, char *path)
{
	int sock;
	struct sockaddr_in server;
	struct sockaddr_in client; 
	int clientLen; 
	struct hostent *hp; 
	char command[100];
	char buf[1024];
	char tmp[20];

	fprintf(stdout, "server: %s port: %i as1: %i as2: %i\n",remoteServer, remotePort, as1, as2);
	if ((sock = socket(AF_INET, SOCK_STREAM, 0)) < 0)
	{
		fprintf(stderr, "%s:%i Opening stream socket.\n",__FILE__, __LINE__);
		exit(-1);
	}

	
	memset(command, '\0', 100);
	strcpy(command, "GetPath");
	strcat(command, "#");

	sprintf(tmp, "%i", as1);
	strcat(command, tmp);
	strcat(command, "#");

	sprintf(tmp, "%i", as2);
	strcat(command, tmp);
	strcat(command, "#");

	bzero((char *) &server, sizeof(server));
	server.sin_family = AF_INET;
	inet_aton(remoteServer, &server.sin_addr);
	server.sin_port = htons((u_short) remotePort);
      
	if (connect(sock, (struct sockaddr *) &server, sizeof(server)) < 0)
	{
		fprintf(stderr, "%s:%i A problem occured while trying to connect to the server.\n",__FILE__, __LINE__);
		close(sock);
		return -1;
	}
      
	// Sending the control message
	if (write(sock, command, sizeof(command)) < 0)
	{
		fprintf(stderr, "%s:%i A problem occured while trying to send the control message.\n",__FILE__, __LINE__);
		close(sock);
		return -1;
	}
	else 
	{
		printf("The path request message [%s] has been successfully sent.\n", command);
		printf("Waiting for the PATH ...\n");
		memset(buf, '\0', 1024);
		if(read(sock, buf, 1024)  < 0)
		{
			fprintf(stderr, "%s:%i A problem occured while trying to read the path.\n",__FILE__, __LINE__);
			close(sock);
			return -1;
		}
		strcpy(path, buf);
		printf("Received Path: %s\n", buf);
	}	
      
	close(sock);
	return 0;
}

void Usage()
{
	fprintf(stderr,"Usage: ./ChangeSR\n\
	-i [Indicates the server Ip where the NSP emulator is running]\n\
	-p [Indicates the server port where the NSP emulator is running]\n\
	-a [Indicates the first part of the sampling rate]\n\
	-b [Indicates the second part of the sampling rate]\n\
	-m [Indicates the monitor Ip running whithin the emulator]\n");
}
