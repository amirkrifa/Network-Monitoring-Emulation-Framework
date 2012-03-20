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

#include "SoftflowdInterface.h"
#include "CommonStructs.h"
#include "exception.h"
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <sys/un.h>
#include <stdio.h>
#include <signal.h>
#include <netdb.h>
#include "TimeStamp.h"

string SOFTFLOWD_HOST_NAME("sim2.inria.fr");
int SOFTFLOWD_HOST_PORT = 2618;

namespace AmDev
{

bool SoftflowdInterface::GetInterfaceStatus()
{
	bool tmp;
	sem_wait(&interfaceStatusSem);
		tmp=boolInterfaceStatus;
	sem_post (&interfaceStatusSem);
	return tmp;
}

void SoftflowdInterface::SetInterfaceStatus(bool status) 
{
	sem_wait (&interfaceStatusSem);
		boolInterfaceStatus=status;
	sem_post (&interfaceStatusSem);
}

bool SoftflowdInterface::GetRemainingPackets()
{
	bool tmp;
	sem_wait (&semListPackets);
		tmp = packetsBuffer.empty();
	sem_post (&semListPackets);
	return !tmp;
}

// Function used to transform the packet struct to a message which will be transmitted to softflowd

void SoftflowdInterface::GetPacketMessage(packet *ptrPacket, string &res)
{
	try
	{
		string stringRes;	
		string protocol;
		char tmp[10];
		//cout <<"	Sending the packet: "<<ptrPacket->iAsSourceNumber<<" "<<ptrPacket->iAsDestNumber<<" "<<" "<<ptrPacket->szIpSource<<" "<<ptrPacket->szIpDest <<" "<<ptrPacket->iResponsibleNode<<endl;		
		// Responsible node
		if(ptrPacket->iResponsibleNode < 0 || strlen(ptrPacket->szMonitorIp) <= 0)
			throw NspException("Invalid responsible node id.",__FILE__,__LINE__);
		stringRes.append(ptrPacket->szMonitorIp);
		stringRes.append("#");

		// Ip source Adr
		if(strlen(ptrPacket->szIpSource) == 0)
			throw NspException("Invalid packet's source ip adr.",__FILE__,__LINE__);
		stringRes.append(ptrPacket->szIpSource);
		stringRes.append("#");

		// Ip Dest Adr
		if(strlen(ptrPacket->szIpDest) == 0)
			throw NspException("Invalid packet's destination ip adr.",__FILE__,__LINE__);
		stringRes.append(ptrPacket->szIpDest);
		stringRes.append("#");

		// The length of the packet
		if(ptrPacket->lLength<0)
			throw NspException("Invalid packet's length.",__FILE__,__LINE__);
		memset(tmp,'\0',10);
		sprintf(tmp,"%ld",ptrPacket->lLength);
		stringRes.append(tmp);
		stringRes.append("#");

		// The packet TimeStamp (sec)
		TimeStamp tmpTs(0, 0);
		ptrPacket->GetTimeStamp(tmpTs);
		if(tmpTs.GetSecond() < 0)
			throw NspException("Invalid packet's time stamp.",__FILE__,__LINE__);
		memset(tmp,'\0',10);
		sprintf(tmp,"%ld", tmpTs.GetSecond());
		stringRes.append(tmp);
		stringRes.append("#");
		
		// The packet TimeStamp (usec)
		if(tmpTs.GetuSecond() < 0)
			throw NspException("Invalid packet's time stamp.",__FILE__,__LINE__);
		memset(tmp,'\0',10);
		sprintf(tmp,"%ld", tmpTs.GetuSecond());
		stringRes.append(tmp);
		stringRes.append("#");

		// The packet source port
		if(strlen(ptrPacket->szSourcePort) > 0 )
		{
			stringRes.append(ptrPacket->szSourcePort);
			stringRes.append("#");
		}
		else if(strlen(ptrPacket->szSourcePort) == 0 && strcmp(ptrPacket->szProtocol,"ICMP") == 0 )
		{
			stringRes.append("0");
			stringRes.append("#");
		} else 
			throw NspException("Invalid packet's source port.",__FILE__,__LINE__);

		// The packet dest port
		if(strlen(ptrPacket->szDestPort) == 0 && strcmp(ptrPacket->szProtocol,"ICMP") != 0)
			throw NspException("Invalid packet's destination port.",__FILE__,__LINE__);
		else if(strlen(ptrPacket->szDestPort) > 0)
		{
			stringRes.append(ptrPacket->szDestPort);
			stringRes.append("#");
		} else if(strlen(ptrPacket->szDestPort) == 0 && strcmp(ptrPacket->szProtocol,"ICMP") == 0)
		{
			stringRes.append("0");
			stringRes.append("#");
		}

		// The packet source AS number
		if(ptrPacket->iAsSourceNumber<0)
			throw NspException("Invalid packet's AS source number.",__FILE__,__LINE__);
		memset(tmp,'\0',10);
		sprintf(tmp,"%i",ptrPacket->iAsSourceNumber);
		stringRes.append(tmp);
		stringRes.append("#");

		// The packet dest AS number
		if(ptrPacket->iAsDestNumber<0)
			throw NspException("Invalid packet's AS destination number.",__FILE__,__LINE__);
		memset(tmp,'\0',10);
		sprintf(tmp,"%i",ptrPacket->iAsDestNumber);
		stringRes.append(tmp);
		stringRes.append("#");
		
		// packet's protocol
		if(strlen(ptrPacket->szProtocol) == 0)
			{ptrPacket->ShowDetails();throw NspException("Invalid packet's protocol.",__FILE__,__LINE__);}
		stringRes.append(ptrPacket->szProtocol);
		protocol.assign(ptrPacket->szProtocol);
		

		// Protocol's related fields
		if(protocol.compare("UDP") == 0)
		{
			stringRes.append("###");

			// Nothing to do
		}
		else if(protocol.compare("TCP") == 0) 
		{
			// Tcp packet
			if(ptrPacket->fragment)
			{
				stringRes.append("#");
				stringRes.append("1");
				stringRes.append("#");
				memset(tmp,'\0',10);
				sprintf(tmp,"%i",ptrPacket->fragmentSeqNumber);
				stringRes.append(tmp);
								
			}else {stringRes.append("#");stringRes.append("0");stringRes.append("#");}

			// Adding The TCP Flags
			stringRes.append("#");
			memset(tmp,'\0',10);
			tmp[0] =(char)ptrPacket->tcpFlags;
			stringRes.append(tmp);

		} else if(protocol.compare("ICMP") == 0)
		{
			// ICMP packet
			stringRes.append("###");
		}
	
		// Indicate if the packet has been already forwarded to softflowd or not
  		if(ptrPacket->forwardedToSoftflowd)
  		{
 			stringRes.append("#");
  			stringRes.append("1");	
  		}else 
  		{
  			stringRes.append("#");
  			stringRes.append("0");	
  		}
											
		res.assign(stringRes);
	}

	catch(exception &e)
	{
		cout<<e.what()<<endl;
		ptrPacket->ShowDetails();
	}

}



SoftflowdInterface::SoftflowdInterface(void)
{
	try
	{
		sem_init(&mainThreadStatusSem, 0 , 1);
		mainThreadEnded = false;
		boolInterfaceStatus=true;
		sem_init(&semListPackets, 0, 1);
		sem_init (&interfaceStatusSem, 0, 1);
	}
	catch(exception &e)
	{
		cout <<e.what()<<endl;
	}
}

SoftflowdInterface::~SoftflowdInterface(void)
{
	try
	{
		// Stops the thread
		boolInterfaceStatus=false;
		// cleaning the remaining packets.
		sem_wait(&semListPackets);
		if(!packetsBuffer.empty())
		{
			packetsBuffer.clear();
		}
		sem_post(&semListPackets);

		sem_destroy(&semListPackets);
		sem_destroy(&interfaceStatusSem);
	}
	catch(exception &e)
	{
		cout <<e.what()<<endl;
	}
	
}

// runs the interface thread.
void SoftflowdInterface::StartTheInterfaceThread()
{
	
	try
	{
		structThreadParam * ptrParam= (structThreadParam*)malloc(sizeof(structThreadParam));
	    	if(ptrParam==NULL)
			throw NspException("Memory allocation problem.",__FILE__,__LINE__);
		ptrParam->ptrSoftflowdInterface=this;
		ptrParam->pkt =NULL;
		pthread_attr_t threadAttr;

		if (pthread_attr_init (&threadAttr) != 0) 
		{
			throw NspException("pthread_attr_init error.",__FILE__,__LINE__);
		}

		if (pthread_attr_setdetachstate (&threadAttr,PTHREAD_CREATE_DETACHED) != 0) 
		{
			throw NspException("pthread_attr_setdetachstate error.",__FILE__,__LINE__);
		}
		
		if (pthread_create (&interfaceThread,&threadAttr,SoftflowdThreadProcess, (void *)ptrParam) < 0) 
		{
			throw NspException("pthread_create error.",__FILE__,__LINE__);
		}
	}
	catch(exception &e)
	{
		cout<<e.what()<<endl;
	}
}

// function used by the thread, which will send the packets to the Softflowd service 
void * SoftflowdInterface::SoftflowdThreadProcess (void * arg)
{
    	int     sock;
    	int     j, slen, rlen;

    	char   rdata[2000]; 
    	struct  sockaddr_un servaddr; 
	struct  sockaddr_in udpServAddr;
    	int     fromlen;
	struct hostent *pServer;
	structThreadParam * threadParam=(structThreadParam*)arg;
	try
	{	
		if(threadParam->ptrSoftflowdInterface == NULL||threadParam==NULL)
			throw NspException("Invalid thread parameter.",__FILE__,__LINE__);
	
		#ifdef USE_AF_UNIX_SOCKET_TOWORD_SOFTFLOWD
		if ((sock = socket(AF_UNIX, SOCK_DGRAM, 0)) < 0) 
		{
		    throw NspException("AF_UNIX socket creation error",__FILE__,__LINE__);
		}
		#endif
		#ifdef USE_UDP_SOCKET_TOWORD_SOFTFLOWD
		if ((sock = socket(AF_INET, SOCK_DGRAM, 0)) < 0) 
		{
		    throw NspException("AF_INET socket creation error",__FILE__,__LINE__);
		}
		#endif
        
		// Set up address structure for server socket
		#ifdef USE_AF_UNIX_SOCKET_TOWORD_SOFTFLOWD
		bzero(&servaddr, sizeof(servaddr));
		servaddr.sun_family = AF_UNIX;
		strcpy(servaddr.sun_path, SOFTFLOWD_SOCKET_PATH);
		#endif
		#ifdef USE_UDP_SOCKET_TOWORD_SOFTFLOWD
		pServer = gethostbyname(SOFTFLOWD_HOST_NAME.c_str());
		bzero(&udpServAddr, sizeof(udpServAddr));
		udpServAddr.sin_family = AF_INET;
		memcpy(&udpServAddr.sin_addr.s_addr, pServer->h_addr, pServer->h_length);
		udpServAddr.sin_port = htons(SOFTFLOWD_HOST_PORT);
		#endif
		
		string dataRes;
		listPackets::iterator iterPackets;
		while(threadParam->ptrSoftflowdInterface->GetInterfaceStatus() || !threadParam->ptrSoftflowdInterface->packetsBuffer.empty() )
		{
			
			if(!threadParam->ptrSoftflowdInterface->packetsBuffer.empty())
			{
			
				threadParam->ptrSoftflowdInterface->GetListOfPacketSem();

				iterPackets = threadParam->ptrSoftflowdInterface->packetsBuffer.begin();
				
				while((*iterPackets) != NULL)
				{	
					dataRes.clear();
					threadParam->ptrSoftflowdInterface->GetPacketMessage(*iterPackets, dataRes);
					
					if(dataRes.length() == 0) throw NspException("Invalid packet's returned data.",__FILE__,__LINE__);
				
					{
						// Multi thread Version, possible problem with valgrind
						/*	
						sendingThrerad * st = (sendingThread *)malloc(sizeof(sendingThread));
						st->sock = sock;
						strcpy(st->data, (char*)dataRes.c_str());
 
						pthread_attr_t threadAttr;
						pthread_t sendingThread;
						if (pthread_attr_init (&threadAttr) != 0) 
						{
							throw NspException("pthread_attr_init error.",__FILE__,__LINE__);
						}

						if (pthread_attr_setdetachstate (&threadAttr,PTHREAD_CREATE_DETACHED) != 0) 
						{
							throw NspException("pthread_attr_setdetachstate error.",__FILE__,__LINE__);
						}
		
						if (pthread_create (&sendingThread,&threadAttr,SendData, (void *)st) < 0) 
						{
							throw NspException("pthread_create error.",__FILE__,__LINE__);
						}
						*/
					
						// Mono thread version 
							
						#ifdef USE_AF_UNIX_SOCKET_TOWORD_SOFTFLOWD
						slen = sendto(sock, (char*)dataRes.c_str(), dataRes.length(), 0,(struct sockaddr *) 	&servaddr,sizeof(servaddr));
						#endif
						#ifdef USE_UDP_SOCKET_TOWORD_SOFTFLOWD
						slen = sendto(sock, (char*)dataRes.c_str(), dataRes.length(), 0,(struct sockaddr *) &udpServAddr,sizeof(udpServAddr));
						#endif
						if (slen<0) 
						{
							  throw NspException("Error occured while executing 	sendto.",__FILE__,__LINE__);
						}
						else 
						{
							#ifdef SHOW_SOFTFLOWD_INTERFACE_ACTIVITY
								cout<<"SoftflowdInterface, "<<slen<<" bytes forwarded: "<<dataRes<<endl;
							#endif
					 
						}
							
						free(*iterPackets);
						threadParam->ptrSoftflowdInterface->packetsBuffer.erase(iterPackets);
						
					}
					if(!threadParam->ptrSoftflowdInterface->packetsBuffer.empty())
						iterPackets = threadParam->ptrSoftflowdInterface->packetsBuffer.begin();
					else break;
					
				}
				threadParam->ptrSoftflowdInterface->ReleaseListOfPacketSem();
			}
			
		}
		
		//Sending the end message
		#ifdef USE_AF_UNIX_SOCKET_TOWORD_SOFTFLOWD
		slen = sendto(sock, "end", strlen("end"), 0,(struct sockaddr *) &servaddr,sizeof(servaddr));
		if (slen<0) 
		{
			#ifdef SHOW_SOFTFLOWD_INTERFACE_ACTIVITY
				cout<< "Error occured while executing sendto."<<endl;
			#endif
		}
		else 
		{
			cout<<"SoftflowdInterface, the end message has been sent."<<endl;
		}
		#endif

		#ifdef USE_UDP_SOCKET_TOWORD_SOFTFLOWD
		slen = sendto(sock, "end", strlen("end"), 0,(struct sockaddr *) &udpServAddr,sizeof(udpServAddr));
		if (slen<0) 
		{
			#ifdef SHOW_SOFTFLOWD_INTERFACE_ACTIVITY
				cout<< "Error occured while executing sendto."<<endl;
			#endif
		}
		else 
		{
			cout<<"SoftflowdInterface, the end message has been sent."<<endl;
		}
		#endif

		close(sock);
		threadParam->ptrSoftflowdInterface->MainThreadExit();
		free(threadParam);
		pthread_exit (0);
	}
	catch(exception &e)
	{
		cout<<e.what()<<endl;
		close(sock);
		free(threadParam);
		pthread_exit (0);
	}
}

void * SoftflowdInterface::SendData(void * arg)
{
try
{
	struct  sockaddr_un servaddr; 
	struct  sockaddr_in udpServAddr;
    	struct hostent *pServer;
	
	int slen = 0;
	sendingThread * st = (sendingThread *)arg;

	if(st->data == NULL)
		throw NspException("Invalid Data.",__FILE__,__LINE__);

	// Set up address structure for server socket 
	#ifdef USE_AF_UNIX_SOCKET_TOWORD_SOFTFLOWD
	bzero(&servaddr, sizeof(servaddr));
	servaddr.sun_family = AF_UNIX;
	strcpy(servaddr.sun_path, SOFTFLOWD_SOCKET_PATH);
	slen = sendto(st->sock, st->data, strlen(st->data), 0,(struct sockaddr *) &servaddr,sizeof(servaddr));
	
	if (slen < 0) 
	{
		#ifdef SHOW_SENDING_STATUS
		cout<< "Error occured while executing sendto."<<endl;
		#endif
	}
	else 
	{
		#ifdef SHOW_SENDING_STATUS
		cout<<"SoftflowdInterface, "<<slen<<" bytes forwarded: "<<st->data<<endl;
		#endif
	}
	#endif

	#ifdef USE_UDP_SOCKET_TOWORD_SOFTFLOWD
	pServer = gethostbyname(SOFTFLOWD_HOST_NAME.c_str());
	bzero(&udpServAddr, sizeof(udpServAddr));
	udpServAddr.sin_family = AF_INET;
	memcpy(&udpServAddr.sin_addr.s_addr, pServer->h_addr, pServer->h_length);
	udpServAddr.sin_port = htons(SOFTFLOWD_HOST_PORT);
	
	slen = sendto(st->sock, st->data, strlen(st->data), 0,(struct sockaddr *) &udpServAddr,sizeof(udpServAddr));
	if (slen < 0) 
	{
		#ifdef SHOW_SENDING_STATUS
		cout<< "Error occured while executing sendto."<<endl;
		#endif
	}
	else 
	{
		#ifdef SHOW_SENDING_STATUS
		cout<<"SoftflowdInterface, "<<slen<<" bytes forwarded: "<<st->data<<endl;
		#endif
	}
	#endif

	if(st != NULL)
		free(st);
	pthread_exit (0);
	
}
catch(exception &e)
{
	cout << e.what() <<endl;
}
}

// Push a packet in the list so the packet will be sent by the thread.
void SoftflowdInterface::PreparePacketToBeSent(packet *pkt)
{

	try
	{
		
		// Make a copy of the packet
		struct packet *tmp=(struct packet *)malloc(sizeof(struct packet));
		tmp->lLength = pkt->lLength;

		TimeStamp tmpTs(0, 0);
		pkt->GetTimeStamp(tmpTs);		
		tmp->SetTimeStamp(tmpTs);

		strcpy(tmp->szIpSource, pkt->szIpSource);
		strcpy(tmp->szIpDest, pkt->szIpDest);
		strcpy(tmp->szSourcePort, pkt->szSourcePort);
		strcpy(tmp->szDestPort, pkt->szDestPort);
		strcpy(tmp->szProtocol, pkt->szProtocol);
		tmp->fragment = pkt->fragment;
		tmp->fragmentSeqNumber = pkt->fragmentSeqNumber;
		tmp->forwardedToSoftflowd = pkt->forwardedToSoftflowd;

		pkt->forwardedToSoftflowd = true;

		strcpy(tmp->szMonitorIp, pkt->szMonitorIp);
		tmp->iResponsibleNode = pkt->iResponsibleNode;
		tmp->iAsSourceNumber = pkt->iAsSourceNumber;
		tmp->iAsDestNumber = pkt->iAsDestNumber;
		tmp->tcpFlags = pkt->tcpFlags;
		
		{
			// Multi Thread Version
		/*	
			if(tmp->AmIValid())
			{
				structThreadParam * p = (structThreadParam * )malloc(sizeof(structThreadParam));
				p->ptrSoftflowdInterface = this;
				p->pkt = tmp;
				tmp = NULL;
		
				pthread_attr_t threadAttr;
				pthread_t pushingThread;
				if (pthread_attr_init (&threadAttr) != 0) 
				{
					throw NspException("pthread_attr_init error.",__FILE__,__LINE__);
				}
		
				if (pthread_attr_setdetachstate (&threadAttr,PTHREAD_CREATE_DETACHED) != 0) 
				{
					throw NspException("pthread_attr_setdetachstate error.",__FILE__,__LINE__);
				}

				if (pthread_create (&pushingThread,&threadAttr,AddPacketToTheBufferThread, (void *)p) < 0) 
				{
					throw NspException("pthread_create error.",__FILE__,__LINE__);
				}
			} else 
			{	
				tmp->ShowDetails();
				free(tmp);
				throw NspException("Invalid Packet.",__FILE__,__LINE__); 
			}
		*/
		}
		{	
			// Mono Thread version
			if(tmp->AmIValid())
			{
				AddPacketToTheBuffer(tmp);
				tmp = NULL;
			}
			else 
			{	
				tmp->ShowDetails();
				free(tmp);
				throw NspException("Invalid Packet.",__FILE__,__LINE__); 
			}
			
		}
	}
	catch(exception &e)
	{
		cout<<e.what()<<endl;
	}
}

void * SoftflowdInterface::AddPacketToTheBufferThread(void * arg)
{
	try
	{
		structThreadParam * p = (structThreadParam * )arg;
		
		if(p->pkt==NULL) throw NspException("Packet error, please verify that the node is sending a correct packet into the Softflowd Interface.",__FILE__,__LINE__);
		
		p->ptrSoftflowdInterface->AddPacketToTheBuffer(p->pkt);

		p->pkt = NULL;
		p->ptrSoftflowdInterface = NULL;

		if(p)
			free(p);

		pthread_exit (0);
	}
	catch(exception &e)
	{
		cout<<e.what()<<endl;
	}
}

void  SoftflowdInterface::AddPacketToTheBuffer(packet * pkt)
{
	try
	{
		// putting the packet into the buffer.
		GetListOfPacketSem();
			packetsBuffer.push_back(pkt);
			pkt=NULL;
		ReleaseListOfPacketSem();
	}
	catch(exception &e)
	{
		cout<<e.what()<<endl;
	}
}

}


