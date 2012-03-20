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

#ifndef _ROUTER_
#define _ROUTER
#include<string>
#include "Node.h"
#include "exception.h"
#include <map>
#include<list>
#include <semaphore.h>
#include <fstream>
using namespace std;


namespace AmDev{

typedef struct SamplingNode
{
	int samplingRateP1;
	int samplingRateP2;
	int samplingRateChanged;
	int samplingRateAxis;
	char ipAddr[16];
	int * samplingTab;
	sem_t sem;
	struct SamplingNode * next;

}SamplingNode;

typedef struct InterfaceSamplingStatus
{
	int in;
	int out;
}InterfaceSamplingStatus;

struct packet;
class Link;
class SoftflowdInterface;
class Router : public Node {
public:
	Router(int iNumberOfAdapters, int id,SoftflowdInterface * sfi,int iMonitor,map<string, InterfaceSamplingStatus * >, list<string>  );
	virtual ~Router();
	void ShowDetails();
	void ForwardPacket(packet*,int & iSecondsToSleep, string & sNextInterface, Link * pCurrentLink, string  inInterface );
	void ForwardPacketE(packet*pPacket,int & iSecondsToSleep, Router * destRouter, string  inInterface, string & in);
	
	inline int GetRouterId() const {return iRouterNumber;}
	int GetInterfaceMonitoringStatus(string &interface, int inOut);
	bool IsEdgeTo(string & as);
	string GetEdgeAs();
	LinkList GetListOfLinks();
	list<string> GetListOfAdapters();
	Router * GetNextRouterToward(Router * r);
	Router * GetAlreadyCalculatedNextRouterToward(Router * r);
	void DrawDifferentPaths();

	// We suppose that each two routers are connected via one single link	
	Link* GetLinkTowards(Router*dest, string& nextAdapter);
	
	// Sampling related functions
	int ChangeSamplingRate(string adapter, int srP1, int srP2);
	SamplingNode * AddNode(char * ip);
	SamplingNode * GetSamplingNode(char * ip);
	SamplingNode * FindSamplingNode(char * ip);
	void FreeListNodes();
	int RandomPeriodicNodeSampling(char * ip);
	void InitNodeSamplingRateStatus(SamplingNode *sn);
	int GetNodeSamplingRateStatus(SamplingNode *sn);
	void GenerateRandomChoicesForNode(SamplingNode * sn);
	

	bool WasVesitor(Router *, string );
	void AddVisitedRouter(string, Router*);
	void UpdateLinksDests();
	
	map<Router*, string> visitedBy;
	map<string, list<Router*> > visitedRouters;
	map<Router*, int > directRoutersCosts;
	map<Router*, int > allRoutersCosts;
	map<Router*, Router* > prevRouters;
	map<Router*, Router*> nextRouter;
	void ShowRoutersPaths();
	void ShowAllRoutersCost();
	void ShowDirectRoutersCost();
	void ShowTheMapOfVisitedRouters();
	void ShowInterfacesSamplingStatus();
	void WriteLogFiles();
private:
	fstream logInPackets;
	fstream logOutPackets;
	unsigned int inPacketsNumber;
	unsigned int outPacketsNumber;
	
	int iRouterNumber;
	SoftflowdInterface * ptrSoftflowdInterface;
	int iIsMonitor;
	list<string> listEdgeAS;
	map<string, InterfaceSamplingStatus * > interfaceSamplingStatus;
	SamplingNode * listNodes;
	// Used for the Dijkstra shortest path algorithm
	bool done;

	
};
}
#endif
