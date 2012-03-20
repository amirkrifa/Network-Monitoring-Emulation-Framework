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

#ifndef _COMMON_STRUCTS_
#define _COMMON_STRUCTS_
#include "exception.h"
#include <pthread.h>
#include <string>
#include<iostream>
#include <fstream>
#include <semaphore.h>
#ifndef TIME_STAMP_H
#include "TimeStamp.h"
#endif
using namespace std;

namespace AmDev{

//SoftflowdInterface.cpp
#define SOFTFLOWD_SOCKET_PATH	"../sfi.interface"
#define bzero(ptr, len)  memset((ptr), 0, (len))

//TraficLoader.cpp
#define MAX_TL_LOOP_PACKETS  1000

// SamplingPlatform.cpp
#define TrafficMatrixMaxSize 40
#define TIME_AXIS_SLOT 5
#define MAX_ITERATIONS 10

// Router.cpp
#undef SHOW_PACKETS_FORWRDING 
// Softflowdinterface.cpp

#undef SHOW_SOFTFLOWD_INTERFACE_ACTIVITY 
#undef USE_AF_UNIX_SOCKET_TOWORD_SOFTFLOWD
#define USE_UDP_SOCKET_TOWORD_SOFTFLOWD

#ifdef USE_AF_UNIX_SOCKET_TOWORD_SOFTFLOWD
#undef USE_UDP_SOCKET_TOWORD_SOFTFLOWD
#endif

#ifdef USE_UDP_SOCKET_TOWORD_SOFTFLOWD
#undef USE_AF_UNIX_SOCKET_TOWORD_SOFTFLOWD
#endif



// SamplingPlatform.cpp
#undef SHOW_THE_REMAINING_NUMBER_OF_PACKETS 
#undef SHOW_PACKETS_DETAILS_WHEN_DISPATCHING
#undef SHOW_IPV6_PACKETS
// More memory usage, if defined
#undef MAINTAIN_IPS_IN_AS

// TimeStamp.cpp
#undef SHOW_TIME_TO_WAIT_BETWEEN_TWO_PACKETS
// SoftflowdInterface.cpp
#undef SHOW_SENDING_STATUS
 
#define CONTROL_SERVER_DEFAULT_PORT 2618

// Used in Router.c
#define INITIAL_SAMPLING_RATE_P1 1 
#define INITIAL_SAMPLING_RATE_P2 100

//struct which represent the packet to be treated

typedef struct packet{

	unsigned int usecond;
	unsigned int second;
	char szMonitorIp[20];
	long int lLength;
	char szIpSource[20];
	char szIpDest[20];
	char szSourcePort[20];
	char szDestPort[20];
	char szProtocol[10];
	int iResponsibleNode;
	int iAsSourceNumber;
	int iAsDestNumber;
	bool fragment;
	bool ipv4;
	bool ipv6;
	unsigned long int fragmentSeqNumber;
	bool forwardedToSoftflowd;
	unsigned char tcpFlags;
	void Init()
	{
		usecond = 0;
		second = 0;
		lLength = 0;
		memset(szIpSource,'\0',20);
		memset(szIpDest,'\0',20);
		memset(szDestPort,'\0',20);
		memset(szSourcePort,'\0',20);
		memset(szProtocol,'\0',10);
		
		iResponsibleNode = 0;
		iAsSourceNumber = 0;
		iAsDestNumber = 0;
		fragment = false;
		ipv4 = false;
		ipv6 = false;
		fragmentSeqNumber = 0;
		forwardedToSoftflowd = false;
	}
	void LogInfos(fstream &file)
	{
		cout<<second<<":"<<usecond<<"    "<<szIpSource <<"   "<<szIpDest<<"    "<<szSourcePort<<"		"<<szDestPort<<"    "<<szProtocol<<"    "<<lLength<<endl;
	}

	bool AmIValid()
	{
		return (usecond >= 0 && second >= 0 && lLength >= 0	&& strlen(szIpSource) > 0 && strlen(szIpDest) > 0 && strlen(szSourcePort) > 0 && strlen(szDestPort) > 0 && strlen(szProtocol) > 0 && iAsSourceNumber >= 0 && iAsDestNumber >= 0 ) || ((usecond >= 0 && second >= 0 && lLength >= 0	&& strlen(szIpSource) > 0 && strlen(szIpDest) > 0 && strlen(szProtocol) > 0 && iAsSourceNumber >= 0 && iAsDestNumber >= 0 ) && strcmp(szProtocol, "ICMP") == 0);	
	}

	void GetTimeStamp(TimeStamp &ts)
	{
		ts.SetuSecond(usecond);
		ts.SetSecond(second);
	}
	void SetTimeStamp(unsigned int usec, unsigned int sec)
	{
		usecond = usec;
		second = sec;
	}
	void SetTimeStamp(TimeStamp &ts)
	{
		usecond = ts.GetuSecond();
		second = ts.GetSecond();
	}
	void ShowDetails()
	{
		
		cout<<" Packet's time stamp: "<<second<<":"<<usecond<<endl;
		cout<<" Packet's length: "<<lLength<<endl;
		cout<<" Packet's Source Ip: "<<szIpSource<<endl;
		cout<<" Packet's Destination Ip: "<<szIpDest<<endl;
		cout<<" Packet's Source port: "<<szSourcePort<<endl;
		cout<<" Packet's Destination port: "<<szDestPort<<endl;
		cout<<" Packet's Responsible Node: "<<iResponsibleNode<<endl;
		cout<<" Packet's Source AS number: "<<iAsSourceNumber<<endl;
		cout<<" Packet's Destination AS number: "<<iAsDestNumber<<endl;
		cout<<" Packet's protocol: "<<szProtocol<<endl;
		cout<<" Packet's seq number: "<<fragmentSeqNumber<<endl;
		cout<<" ------------------------------------------------ "<<endl;
		
	}
	
}packet;

typedef struct couple{
  couple(int a,int b) {AsSource=a;AsDest=b;}
  int AsSource;
  int AsDest;
}couple;


}
#endif
