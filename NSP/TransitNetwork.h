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

#ifndef _TRANSIT_NETWORK_H_
#define _TRANSIT_NETWORK_H_
#include "Network.h"
#include <list>
#include <map>
using namespace std;
#include "exception.h"

namespace AmDev{
class As;
class Router;
class Link;
struct packet;
struct InterfaceSamplingStatus;
class SoftflowdInterface;
typedef list<Router *> RouterList;
typedef list<Link*> LinkList;
typedef list<As *> AsList;
class TransitNetwork :public Network{
public:
  	TransitNetwork();
  	virtual ~TransitNetwork();
  	void AddLink(string &sAdapter1,string &sAdapter2,int iBandwidth,int iCost,int iDelay,int iNumberOfDests,map<string,string> mFinalDest);
  	void AddRouter(int iNumber,int iNumberOfAdapters,list<string> lIps,SoftflowdInterface*,int iIsMonitor, map<string, InterfaceSamplingStatus *>, list<string>);
  	void ConnectToAs( As  *const );
  	void AddSourceAs(As *const);
  	void AddDestAs(As *const);
  	inline void SetTheNumberOfRouters(int i){iNumberOfRouters=i;}
  	inline void SetTheNumberOfLinks(int i){iNumberOfLinks=i;}
  	void ShowDetails();
  	void ShowListOfLinks();
  	bool CouldYouForwrdTo(int iAsNumber);
	void ForwardPacket(packet *);
	void ForwardPacketE(packet *);
	int ChangeSamplingRate(string ipAdr, int p1, int p2);
	void ExploreGraph();
	Router * GetRouterFromId(int id);
	void ShowPathFromTo(int id1, int id2);
	void GetInterfaceBasedPathPathFromTo(int id1, int id2, string &path);
	Router * GetTheEdgeRouterOf(string AS);
	
private:
  	string GetPathForThePacket(packet * pPacket);
  	inline unsigned int GetNumberOfLinks() const {return ptrListOfLinks.size();}
  	inline unsigned int GetNumberOfRouters() const {return ptrListOfRouters.size();}
  	Link *GetLink(string sSourceIp,string sDestIp,string sFinalDest);
	
  	Router *GetRouter(string sAdapter, bool lookingForAnEdge);
	bool ExploreNodeGraph(Router * currentNode, Router * startNode, int cc, string startInterface );
	void Dijkstra(Router* r);

private:
  	AsList ptrListOfDestAs;
  	AsList ptrListOfSrcAs;
  	int iNumberOfRouters;
  	int iNumberOfLinks;
  	RouterList ptrListOfRouters;
  	LinkList   ptrListOfLinks;
	
};
}
#endif

