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

#ifndef _SOFTFLOWD_INTERFACE_
#define _SOFTFLOWD_INTERFACE_
#include <list>
#include "CommonStructs.h"
#include<iostream>
#include <pthread.h>
#include <semaphore.h>

using namespace std;

namespace AmDev
{

typedef list<packet *> listPackets;
class SoftflowdInterface;
typedef struct structThreadParam
{
	SoftflowdInterface * ptrSoftflowdInterface;
	packet * pkt;
}structThreadParam;

typedef struct sendingThrerad
{
	char data[2000];
	int sock;
} sendingThread;

class SoftflowdInterface
{
public:
	SoftflowdInterface(void);
	static void * SoftflowdThreadProcess (void * arg);
	static void * SendData (void * arg);
	static void * AddPacketToTheBufferThread(void * arg);	
 	void  AddPacketToTheBuffer(packet * arg);	
	void StartTheInterfaceThread();
	void PreparePacketToBeSent(packet *);
	
	bool GetInterfaceStatus();
	void SetInterfaceStatus(bool status);
	bool GetRemainingPackets();
	void GetPacketMessage(packet *ptrPacket, string &res);
	inline void GetListOfPacketSem()
	{
		sem_wait(&semListPackets);
	}
	inline void ReleaseListOfPacketSem()
	{
		sem_post(&semListPackets);
	}
	inline bool GetMainThreadStatus()
	{
		bool tmp;
		sem_wait(&mainThreadStatusSem);
			tmp = mainThreadEnded;
		sem_post(&mainThreadStatusSem);
		return tmp;
	}
	
	inline void MainThreadExit()
	{
		
		sem_wait(&mainThreadStatusSem);
			mainThreadEnded = true;
		sem_post(&mainThreadStatusSem);
	}
	
	~SoftflowdInterface(void);
	listPackets packetsBuffer;

private:

	sem_t semListPackets;
	sem_t interfaceStatusSem;
	bool boolInterfaceStatus;

	bool mainThreadEnded;
	sem_t mainThreadStatusSem;

	pthread_t interfaceThread;
	
	
};
}
#endif
