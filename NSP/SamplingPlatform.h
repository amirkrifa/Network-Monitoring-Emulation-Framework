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

#ifndef _SAMPLING_PLATFORM_H_
#define _SAMPLING_PLATFORM_H_
#include <iostream>
#include "TraficLoader.h"
#include <string>
#include <list>
#include <map>
#include "TimeStamp.h"
#include "CommonStructs.h"
#include "exception.h"
#include <semaphore.h>
#include <fstream>
using namespace std;

namespace AmDev {


typedef list<packet *> ListOfPackets;
class TransitNetwork;
class As;
class SoftflowdInterface;
struct couple;
class SamplingManager;
	
typedef list<As *> ListOfAs;
typedef list<TransitNetwork *> ListOfTransitNetworks;

// Used for the dispatchin method number 1
typedef struct percentageCount
{
	inline percentageCount(int a, int b)
	{
		percentage = a;
		count = b;
	}
	inline int Increment()
	{
		count = (count + 1) % percentage;
		return count;
	}
	int percentage;
	int count;
}percentageCount;

class SamplingPlatform {
public:
  	SamplingPlatform();
  	void StartTrafficLoader(char *);
  	virtual ~SamplingPlatform();
 	TrafficLoader * GetTrafficLoader() const {return Tl;}
  	void LoadTrafficFile(string &trafficFile);
	
	void AddEdgeAS(string &sTopologyFile);
  	void AddNewAs(string & IpClass, string &IpRange, unsigned int method,string & IpsListFile);
	void AddNewAs(int iAsNumber, int iAsMaxIps, int iAsImportance,int minInterval, int maxInterval, string & IpClass, string &ipRangeStart, string &ipRangeStart, int ipMask, int dispatchingMethod);
	bool GenerateTrafficBetween(int iAsSource, int iAsDest);
	void ShowListOfAs();
    	
	void AddNewTransitNetwork(string &sTopologyFile);
	void ShowListOfTransitNetworks();
  
	bool GenerateTrafficBetween(list<couple> lCouples);
	string  GetTrafficBetweenFilePath(int iAsSource, int iAsDest) const {return TrafficMatrix[iAsSource][iAsDest];}
	
	bool ConnectAsTo(TransitNetwork * const );
	bool ConnectTransitNetworksToAss();
	
	void ShowGeneratedTopology();
	
	void UpdateTheListOfPacketMatrix();
	void StartSendingPackets();
	void WaitUntilPacketsForwrdingToSoftflowd();
	
	// pcap based API
	static void PerPacketCallBackFunction(u_char *user_data, const struct pcap_pkthdr* phdr, const u_char *pkt);
	As * GetAs(int iNumber);
	list<couple> lAsCouples;
	bool ValidateCouple(int a, int b);
	
	// Map used for the dispatching method number 1
	map<int, percentageCount> asImportance;
	int SelectAs();
	int GetRandomAsNumber();
	int SelectAsRandomlyBasedOnWeights();	
	
	// Methods used to load a binary tcpdump file
	void PushPacket(packet *, int AsSource, int AsDest);
	void LoadBinaryTrafficFile(string &btf);
	void LoadListOfBinaryTrafficFiles(string &btf);
	inline int GetDispatchingMethod()
	{
		return dispatchingMethod;
	}
	
	inline int GetIpMask()
	{
		return ipMask;	
	}
	
	ListOfAs ptrListOfAs;
	inline void SetLoading(bool t)
	{
		loading = t;
	}
	inline bool GetLoading()
	{
		return loading;
	}
	inline void GetSendingSem()
	{
		sem_wait(&sendingSem);
	}

	inline void ReleaseSendingSem()
	{
		sem_post(&sendingSem);
	}
	inline bool DataNeeded()
	{
		return ((timeAxisSize - lastTimeAxisSize) < TIME_AXIS_SLOT) || (timeAxisSize == lastTimeAxisSize);
	}
	inline void SetLoadingStartTime()
	{
		startTime.SetToTheSystemTime();
	}
	inline void SetLoadingEndTime()
	{
		endTime.SetToTheSystemTime();
	}

	inline void IncrementTimeAxis(TimeStamp &t)
	{
		timeAxis.push_back(t);
		timeAxisSize++;
	}
	TransitNetwork * GetTransitNetworkNumber(int tnn)
	{
		int i = 0;
		for(ListOfTransitNetworks::iterator iter = ptrListOfTransitNetwork.begin(); iter!=ptrListOfTransitNetwork.end(); iter++, i++)
		{
			if(i == tnn)
				return *iter;
		}
		return NULL;
	}
	int ChangeSamplingRate(string ipAdr, int p1, int p2);
	list<TimeStamp> timeAxis;
	TimeStamp currentTs;
	TimeStamp maxTs;
	bool ExploreTransitNetworks();
private:
	void UpdateTheListOfPacket(const int AsSrc,const int AsDest);
	void  SendPacket(packet *);
	void SendTheCurrentListOfPackets();
	unsigned int  UpdateTheListOfPacketsToSend(TimeStamp &ts);
	void SetPacketShiftTime(packet *p);
	unsigned int GetLostPackets(TimeStamp &ts);

private:	
	string TrafficMatrix[TrafficMatrixMaxSize][TrafficMatrixMaxSize];
	ListOfPackets ListOfPacketMatrix[TrafficMatrixMaxSize][TrafficMatrixMaxSize];
	unsigned int iNumberOfAs;
	unsigned int iNumberOfTransitNetworks;
	TrafficLoader *Tl;
	
	ListOfTransitNetworks ptrListOfTransitNetwork;
	ListOfPackets pListOfPacketToSend;
	unsigned long int numberOfPackets;
	SoftflowdInterface * ptrSoftflowdInterface;
	SamplingManager * sm;
	// Used in the dispatching method number 1 for RR
	int lastSelectedAs;
	int dispatchingMethod;
	int ipMask;
	// Time stamps used to control the time taken by the data loading proccess
	sem_t sendingSem;	
	TimeStamp startTime;
	TimeStamp endTime;
	TimeStamp loadingTime;
	bool loading;
	int lastTimeAxisSize;
	int timeAxisSize;
	TimeStamp emulationStartTime;
	TimeStamp emulationCurrentTime;
	TimeStamp traceStartTime;
	TimeStamp traceCurrentTime;
	unsigned int waitPeriod;
};
}
#endif

