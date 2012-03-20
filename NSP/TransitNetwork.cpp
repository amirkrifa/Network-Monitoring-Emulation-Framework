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

#include "TransitNetwork.h"
#include "Link.h"
#include "Router.h"
#include "As.h"
#include "CommonStructs.h"
#include "SoftflowdInterface.h"
#include "Node.h"
#define GRAPHSIZE 2048
#define INFINITY GRAPHSIZE*GRAPHSIZE

namespace AmDev{

extern TimeStamp currentTs;
extern TimeStamp maxTs;

TransitNetwork::TransitNetwork()
{

}

TransitNetwork::~TransitNetwork()
{
  	LinkList::iterator iterLink;
  	RouterList::iterator iterRouter;
  
  	while(!ptrListOfRouters.empty())
  	{
   	 	iterRouter=ptrListOfRouters.begin();
    		delete *iterRouter;
		ptrListOfRouters.erase(iterRouter);
  	}
  
  	while(!ptrListOfLinks.empty())
  	{
    		iterLink = ptrListOfLinks.begin();
		delete *iterLink;
    		ptrListOfLinks.erase(iterLink);
  	}

  	if(!ptrListOfDestAs.empty())
    		ptrListOfDestAs.clear();
  	if(!ptrListOfSrcAs.empty())
    		ptrListOfSrcAs.clear();
}

void TransitNetwork::AddLink(string &sAdapter1,string &sAdapter2,int iBandwidth,int iCost,int iDelay,int iNumberOfDests,map<string,string> mFinalDest)
{
try
{
  	if(sAdapter1.length() == 0 || sAdapter2.length() == 0)
		throw NspException("Error while trying to add a new link.",__FILE__,__LINE__);
  	if(ptrListOfLinks.size()<iNumberOfLinks)
   	{
     		Link *l = new Link(this, iCost,iBandwidth,iDelay,sAdapter1,sAdapter2,iNumberOfDests,mFinalDest);
     		ptrListOfLinks.push_back(l);
     		l = NULL;
   	}
  	else throw NspException("Invalid number of links.",__FILE__,__LINE__); 
}

catch(exception &e)
{
	cout <<e.what()<<endl;
}
}
void TransitNetwork::AddRouter(int iNumber,int iNumberOfAdapters,list<string> lIps ,SoftflowdInterface*sfi,int iIsMonitor, map<string, InterfaceSamplingStatus *> tmpMap, list<string> listEdgeAs)
{
try
{
   	if(ptrListOfRouters.size() < iNumberOfRouters)
    	{
      		Router * r = new Router(iNumberOfAdapters,iNumber,sfi,iIsMonitor, tmpMap, listEdgeAs);
      		list<string>::iterator iter=lIps.begin();
		while(iter!=lIps.end())
      		{
        		Link * pLink=NULL;
        		pLink = GetLink(*iter,*iter,string("AddingARouter"));
        		if(pLink)
          		{
				r->AddLink(pLink);
				r->AddAdapterAdr(*iter);
        		}
        		iter++;
      		}

      		ptrListOfRouters.push_back(r);
    	}
   	else throw NspException("The size of the list of routers is incorrect.",__FILE__,__LINE__);
}

catch(exception &e)
{
	cout<<e.what()<<endl;
}
}
Link * TransitNetwork::GetLink(string sSourceIp,string sDestIp,string sFd)
{
try
{
  	LinkList::iterator iter = ptrListOfLinks.begin();
  	while(iter!=ptrListOfLinks.end())
  	{ 
    		if((*iter)->IsItOneOfTheAdapters(sSourceIp) && (*iter)->IsItAFinalDest(sFd))
      			return (*iter);
    		if((*iter)->IsItOneOfTheAdapters(sSourceIp) && sFd.compare("AddingARouter")==0)
      			return (*iter);
			iter++;
  	}
	cout<<"Final Dest: "<<sFd<<endl;
  	throw NspException(" Link not found.",__FILE__,__LINE__);
	
  	return NULL;
  
}

catch(exception &e)
{
  cout<<e.what()<<endl;
}
}
void TransitNetwork::AddSourceAs(As  *const ptrAs)
{
try
{
  	if(ptrAs==NULL)
    		throw NspException("Invalid ptr to AS.",__FILE__,__LINE__);
  	bool found=false;
  	AsList::iterator AsIter=ptrListOfSrcAs.begin();
  	while(AsIter!=ptrListOfSrcAs.end())
  	{
   		if(ptrAs->GetNumber()==(*AsIter)->GetNumber())
       			{found=true;break;}
  	}
  	if(!found)
    		{ptrListOfSrcAs.push_back(ptrAs);}
}

catch(exception &e)
{
	cout<<e.what()<<endl;
}
}


void TransitNetwork::AddDestAs(As * const ptrAs)
{
try
{
  	if(ptrAs==NULL)
    		throw NspException("Invalid ptr to AS.",__FILE__,__LINE__);

  	bool found=false;
  	AsList::iterator AsIter=ptrListOfDestAs.begin();
  	while(AsIter!=ptrListOfDestAs.end())
  	{
    		if(ptrAs->GetNumber()==(*AsIter)->GetNumber())
       			{found=true;break;}
    		AsIter++;
  	}
  	if(!found)
    		{ptrListOfDestAs.push_back(ptrAs);}
}

catch(exception &e)
{
	cout<<e.what()<<endl;
}
}

void TransitNetwork::ConnectToAs(As * const ptrAs)
{
try
{
  	if(ptrAs==NULL)
    		throw NspException("Invalid ptr to AS.",__FILE__,__LINE__);

  	if(!ptrListOfLinks.empty())
  	{
    		string sAdr("As");
    		char szAsNumber[10] ;
    		sprintf(szAsNumber,"%i",ptrAs->GetNumber());
    		sAdr.append(szAsNumber);
    		LinkList::iterator LinkIter;
    		for(LinkIter = ptrListOfLinks.begin();LinkIter!=ptrListOfLinks.end();LinkIter++)
    		{
      
      			if((*LinkIter)->IsItOneOfTheAdapters(sAdr))
      			{ 
        			ptrAs->ConnectToTransitNetwork(this);
        			if((*LinkIter)->IsItAFinalDest(sAdr))
          				AddDestAs(ptrAs);
        			else AddSourceAs(ptrAs);
      			}
    		}
  	}
}

catch(exception &e)
{
	cout<<e.what()<<endl;
}
}

void TransitNetwork::ShowDetails()
{
  	cout << "        Total number of Nodes: "<<iNumberOfRouters<<endl;
  	cout << "        Total number of Links: "<<iNumberOfLinks<<endl;
  	RouterList::iterator NodeIter;int i=1;
  	for(NodeIter = ptrListOfRouters.begin();NodeIter!=ptrListOfRouters.end();NodeIter++)
  	{
     	cout<<"        Node number: "<<i<<endl;
     	(*NodeIter)->ShowDetails();i++;
  	}
}
void TransitNetwork::ShowListOfLinks()
{
  	LinkList::iterator LinksIter;
  	int i=1;
  	for(LinksIter=ptrListOfLinks.begin();LinksIter!=ptrListOfLinks.end();LinksIter++)
 	 {
    		cout<<"    Link Number: "<<i<<" Adapter1: "<<(*LinksIter)->GetAdapter1()<<" Adapter2: "<<(*LinksIter)->GetAdapter2()<<" List of destinations: "<<(*LinksIter)->GetListOfDestinations()<<endl;
    		i++;
 	}
}

// Returns if it is possible or not to forward a packet to the given As
bool TransitNetwork::CouldYouForwrdTo(int iAsNumber)
{
try
{ 
  	if(iAsNumber < 0) 
	{
		char tmp[10];
		sprintf(tmp,"%i",iAsNumber);
		string sTmp("Invalid AS number: ");sTmp.append(tmp);sTmp.append(".");
		throw NspException((char *)sTmp.c_str(),__FILE__,__LINE__);
	}
  	if(ptrListOfDestAs.empty())
    		throw NspException("The list of dest AS(s) is empty.",__FILE__,__LINE__);
  	else 
      	{
        	for(AsList::iterator iter = ptrListOfDestAs.begin();iter != ptrListOfDestAs.end();iter++)
        	{
          		if((*iter)->GetNumber() == iAsNumber)
            			return true;
        	}
        	return false;
      	}
  	return false;
}

catch(exception &e)
{
	cout<<e.what()<<endl;
}
}

// Forward a packet to the different nodes belonging to the path 
void TransitNetwork::ForwardPacket(packet *pPacket)
{
try
{
  	if(pPacket == NULL)
	  	throw NspException("Invalid packet.",__FILE__,__LINE__);
	char szAsId[10];
  	sprintf(szAsId,"%i",pPacket->iAsSourceNumber);
  	string sCurrentNode(string("As")+string(szAsId));

  	sprintf(szAsId,"%i",pPacket->iAsDestNumber);
  	string sFinalDest(string("As")+string(szAsId));
	int iSecondsToSleep;
        string sNextInterface;
        
			

  	Link * pLink = GetLink(sCurrentNode,string(""),sFinalDest);
  	if(pLink)
  	{
		string tmpInterfaceTowardDest;
		// Verfiying if the packet should stay or not with the current AS
		if(sCurrentNode.compare(sFinalDest) == 0)
		{
			// forward the packet to the edge router
			pLink->GetNextInterfaceTowrd(sFinalDest,tmpInterfaceTowardDest);
			Router * pRouter = GetRouter(tmpInterfaceTowardDest, true);
      			if(pRouter)
        		{ 
					iSecondsToSleep = 0;
					sNextInterface.clear();
					pRouter->ForwardPacket(pPacket,iSecondsToSleep,sNextInterface,pLink, tmpInterfaceTowardDest);
	  				sCurrentNode.assign(sNextInterface);
          		} else 
				{
					cout << tmpInterfaceTowardDest<<endl;
					cout <<"Final Dest == Current Node: "<<sFinalDest<<endl;
					pPacket->ShowDetails();
					throw NspException("Could not find the next router.",__FILE__,__LINE__); 
				}	
		} 
		
    		while(sCurrentNode.compare(sFinalDest) != 0)
    		{  
			pLink->GetNextInterfaceTowrd(sFinalDest, tmpInterfaceTowardDest);
      			Router * pRouter=GetRouter(tmpInterfaceTowardDest, false);
      			if(pRouter)
        		{ 
				iSecondsToSleep = 0;
				sNextInterface.clear();
				pLink->GetNextInterfaceTowrd(sFinalDest, tmpInterfaceTowardDest);
          			pRouter->ForwardPacket(pPacket,iSecondsToSleep,sNextInterface,pLink, tmpInterfaceTowardDest);
	  			
				if(sNextInterface.compare(sCurrentNode)==0) 
				{
					throw NspException("A loop without an exit point.",__FILE__,__LINE__);
				}
				
	  			sCurrentNode.clear();
          			sCurrentNode.assign(sNextInterface);
          			pLink = GetLink(sNextInterface,string(""),sFinalDest);
          			if(pLink == NULL)  
				{
					pPacket->ShowDetails();
					throw NspException("Invalid link.",__FILE__,__LINE__); 
				}
        		} else 
			{
				pPacket->ShowDetails();
				throw NspException("Could not find the next router.",__FILE__,__LINE__); 
			}
    		}
													
    		if(sCurrentNode.compare(sFinalDest) == 0)
    		{
      			// Packet arrived to destination
			#ifdef SHOW_PACKETS_FORWRDING  
      			cout <<"At "<<currentTs.GetTime()<<" a packet of size "<< pPacket->lLength<<" from "<<pPacket->szIpSource<<" to "<<pPacket->szIpDest<<" within As number:  "<<pPacket->iAsDestNumber<<" arrived to destination."<<endl;
			#endif
			
    		}
  	} else {throw NspException("Invalid link.",__FILE__,__LINE__); }
}

catch(exception &e)
{
	cout <<e.what()<<endl;
}
}

// Forward a packet to the different nodes belonging to the path 
void TransitNetwork::ForwardPacketE(packet *pPacket)
{
try
{
  	
	if(pPacket == NULL)
	  	throw NspException("Invalid packet.",__FILE__,__LINE__);
	char szAsId[10];
  	sprintf(szAsId,"%i",pPacket->iAsSourceNumber);
  	string sCurrentInterface(string("As")+string(szAsId));
	Router * srcR = GetTheEdgeRouterOf(sCurrentInterface);
	Router * currentR = srcR;
  	sprintf(szAsId,"%i",pPacket->iAsDestNumber);
  	string sFinalDest(string("As")+string(szAsId));
	Router * dstR = GetTheEdgeRouterOf(sFinalDest);
		
	Router * nextR = srcR->GetNextRouterToward(dstR);
	
	
	string inInterface = srcR->GetLink(sCurrentInterface)->GetTheOppositeAdapter(sCurrentInterface);
	
	int iSecondsToSleep;
        
	Link * pLink = NULL;	
	
	
	// Verfiying if the packet should stay or not with the current AS
	if(currentR == dstR)
	{	
		
		// forward the packet to the edge router
		
      		if(nextR)
        	{ 
			iSecondsToSleep = 0;
			string tmp;
			nextR->ForwardPacketE(pPacket,iSecondsToSleep,dstR, inInterface, tmp);
	  	} else 
		{
			pPacket->ShowDetails();
			throw NspException("Could not find the next router.",__FILE__,__LINE__); 
		}	
	} else
	{
 
		string tmpDest = inInterface;
		while(tmpDest.compare(sFinalDest) != 0)
		{  	
			if(currentR)
			{ 
				iSecondsToSleep = 0;
				string nextInInterface;
				currentR->ForwardPacketE(pPacket,iSecondsToSleep,dstR, inInterface, nextInInterface);
				inInterface = nextInInterface;
				tmpDest = inInterface;
				currentR = currentR->GetAlreadyCalculatedNextRouterToward(dstR);
				
			} else 
			{
				pPacket->ShowDetails();
				throw NspException("Could not find the next router.",__FILE__,__LINE__); 
			}
		}
	}											
    	if(currentR == dstR)
    	{
      		// Packet arrived to destination
		#ifdef SHOW_PACKETS_FORWRDING  
      		cout <<" a packet of size "<< pPacket->lLength<<" from "<<pPacket->szIpSource<<" to "<<pPacket->szIpDest<<" within As number:  "<<pPacket->iAsDestNumber<<" arrived to destination."<<endl;
		#endif
	}

}

catch(exception &e)
{
	cout <<e.what()<<endl;
}
}
// Return the router's pointer given an adapter @
Router * TransitNetwork::GetRouter(string sAdapter, bool lookingForAnEdge)
{
try 
{
  	if(sAdapter.length() == 0) throw NspException("Invalid adapter.",__FILE__,__LINE__);
  	for(RouterList::iterator iter=ptrListOfRouters.begin();iter!=ptrListOfRouters.end();iter++)
  	{
    		if((*iter)->IsItYourAdapter(sAdapter))
      			return (*iter);
		if(lookingForAnEdge)
		{
			if((*iter)->IsEdgeTo(sAdapter))
				return (*iter);
		}
  	}
  	return NULL;
}

catch(exception &e)
{
	cout <<e.what()<<endl;
}
}


// TODO: Returns the path which should be followed by a given packet
string TransitNetwork::GetPathForThePacket(packet * pPacket)
{
/* 
 string sCurrentNode;
  string sPath;
  sCurrentNode.assign(pPacket->szIpSource);
  string sFinalDest(pPacket->szIpDest);
  while(sCurrentNode.compare(sFinalDest)!=0)
  {
    Link * pLink=GetLink(sCurrentNode,string(""),0);
    if(pLink!=NULL)
      { 
        sPath.append(pLink->GetDestIp());
        sPath.append("-");
        sCurrentNode.clear();
	Router* pRouter=GetRouter(pLink->GetDestIp());
        
     //   sCurrentNode.assign(pRouter->GetExitAdapter(pPacket));
      }
  }
*/
}


// Changing the sampling rate of a given monitor
int TransitNetwork::ChangeSamplingRate(string ipAdr, int p1, int p2)
{
try
{
	RouterList::iterator iter = ptrListOfRouters.begin();
	while(iter != ptrListOfRouters.end())
	{
		if((*iter)->IsItYourAdapter(ipAdr))
		{
			// Router found , changing the sampling rate
			return (*iter)->ChangeSamplingRate(ipAdr, p1, p2);
		}
		iter++;
	}
	return -1;
}
catch(exception &e)
{
	cout<<e.what()<<endl;
}
}

bool TransitNetwork::ExploreNodeGraph(Router * currentNode, Router * startNode, int cc, string startInterface )
{
		if(currentNode == startNode && cc == 1 || currentNode == NULL)
		{
			return false;
		}
		else
		{ 	
			list<string> listAdapters = currentNode->GetListOfAdapters();
			
			
			for(list<string>::iterator iterA = listAdapters.begin(); iterA != listAdapters.end(); iterA++)
			{
				// Get the current link
				Link * l = currentNode->GetLink(*iterA);
				// Get the opposite adapter
				string oa = l->GetTheOppositeAdapter(*iterA);
				if(oa.find_first_of("As") != string::npos)
					continue;
				// Get the next router
				Router * next = GetRouter(oa, false);
				if(next == NULL)
				{
				}
				string tmp;
				if(currentNode == startNode && cc == 0)
					tmp = *iterA;
				else tmp = startInterface;

				if(next!= currentNode && !next->WasVesitor(startNode, tmp) && next != startNode)
				{
						
						startNode->AddVisitedRouter(tmp, next);
						
						
						next->visitedBy.insert(make_pair<Router*,string>(startNode, tmp));
						
						if(currentNode == startNode &&  cc == 0)
							ExploreNodeGraph(next, startNode, 1, *iterA);
						else ExploreNodeGraph(next, startNode, cc, startInterface);
				}else
				{
					//Already visited or equal to the calling node
					
				}

				if(currentNode == startNode && cc == 0)
				{
					// Update the direct routers cost
					startNode->directRoutersCosts.insert(make_pair<Router*, int>(next, l->GetCost())); 

					for(RouterList::iterator iter = ptrListOfRouters.begin(); iter != ptrListOfRouters.end(); iter++)
					{
						if(*iter != NULL)
							(*iter)->visitedBy.clear();					
					}
				}
				
				// Update the list of destination based on the list of visited routers and the iterA
					
				
			}
			return true;
		}
	
}

void TransitNetwork::ExploreGraph()
{
	for(RouterList::iterator iter = ptrListOfRouters.begin(); iter != ptrListOfRouters.end(); iter++)
	{
		if(*iter != NULL)
			ExploreNodeGraph(*iter, *iter,0, string(""));
		
		(*iter )->UpdateLinksDests();
	}

	for(RouterList::iterator iter = ptrListOfRouters.begin(); iter != ptrListOfRouters.end(); iter++)
	{
		Dijkstra(*iter);
		(*iter)->DrawDifferentPaths();
	}
	//ShowPathFromTo(1, 23);
	
}

void TransitNetwork::Dijkstra(Router* r) 
{
	int i, k;
	// Indicates the visited routers
	int visited[GRAPHSIZE];
	int n = r->visitedRouters.size();
	
	map<string, list<Router*> >::iterator iter = r->visitedRouters.begin();
	while(iter != r->visitedRouters.end())
	{
		list<Router*>::iterator iterR = iter->second.begin();
		while(iterR != iter->second.end())
		{
			r->allRoutersCosts.insert(make_pair<Router*, int>(*iterR, INFINITY));
			// the i-th element has not yet been visited	
			visited[(*iterR)->GetRouterId()] = 0;
			iterR++;
		}
		iter++;
	}
	// Copy the direct routers costs 
	for(map<Router*, int >::iterator iter = r->directRoutersCosts.begin(); iter != r->directRoutersCosts.end();iter++)
	{
		r->allRoutersCosts[iter->first] = r->directRoutersCosts[iter->first];
	}
	

	for(map<Router*, int >::iterator iter2 = r->allRoutersCosts.begin(); iter2 != r->allRoutersCosts.end(); iter2++)
	{
		Router * mini = NULL;
		for(map<Router*, int >::iterator iter3 = r->allRoutersCosts.begin(); iter3 != r->allRoutersCosts.end(); iter3++)
		{

			if(!visited[(iter3->first)->GetRouterId()] && (mini == NULL || iter3->second < r->allRoutersCosts[mini] ))
			{
				{
					mini = iter3->first;
				}
			}
		}
		
		visited[mini->GetRouterId()] = 1;

		for(map<Router*, int >::iterator iter3 = r->allRoutersCosts.begin(); iter3 != r->allRoutersCosts.end(); iter3++)
		{

			if(mini->directRoutersCosts.find(iter3->first) != mini->directRoutersCosts.end())	
			{
				if(r->allRoutersCosts[mini] + mini->directRoutersCosts[iter3->first] < r->allRoutersCosts[iter3->first] )
				{
					r->allRoutersCosts[iter3->first] = r->allRoutersCosts[mini] + mini->directRoutersCosts[iter3->first];
					r->prevRouters.insert(make_pair<Router*, Router*>(iter3->first, mini));
			

				}
			}
		}
	}

}

Router * TransitNetwork::GetRouterFromId(int id)
{
	for(RouterList::iterator iter = ptrListOfRouters.begin();iter != ptrListOfRouters.end();iter++)
	{
		if((*iter)->GetRouterId() == id)
			return (*iter);
	}
	return NULL;
}

void TransitNetwork::ShowPathFromTo(int id1, int id2)
{
	Router * start = GetRouterFromId(id1);
	Router * end = GetRouterFromId(id2);
	cout << "The path from: "<< id1<<" to: "<<id2<<endl;
	if(start != NULL && end != NULL)
	{
		cout<< "start: "<<start->GetRouterId()<<endl;
		start = start->GetNextRouterToward(end);
		while(start != end)
		{
			cout<< "Id: "<<start->GetRouterId()<<endl;
			start = start->GetNextRouterToward(end);
			
		}
		cout<< "end: "<<end->GetRouterId()<<endl;
			
	}
}

// Returning the path formaed bay the Input Interfaces
void TransitNetwork::GetInterfaceBasedPathPathFromTo(int id1, int id2, string & path)
{
	
	Router * start = GetRouterFromId(id1);
	Router * end = GetRouterFromId(id2);
	Router * tmp = NULL;
	path.clear();
	if(start != NULL && end != NULL)
	{
		tmp = start;
		start = start->GetNextRouterToward(end);
		string interfaceWithinDest;

		Link* pChoosedLink = tmp->GetLinkTowards(start, interfaceWithinDest);
		
		if(path.length() > 0)
			path.append(sizeof(char), '#');
		path.append(interfaceWithinDest);
		bool oneHop = true;
		while(start != end)
		{
			if(oneHop)
				oneHop = false;
			tmp = start;
			start = start->GetNextRouterToward(end);
			interfaceWithinDest.clear();
			Link* pChoosedLink = tmp->GetLinkTowards(start, interfaceWithinDest);
					
			if(start != end)
			{
				if(path.length() > 0)
					path.append(sizeof(char), '#');
				path.append(interfaceWithinDest);
			}
		}
		if(path.length() > 0)
			path.append(sizeof(char), '#');
		if(!oneHop)
		{
			path.append(interfaceWithinDest);
			if(path.length() > 0)
				path.append(sizeof(char), '#');
		}
		
		
	}
		
	cout<<" Asking for the path from "<<id1<<" to "<<id2<<endl;
	cout<<" Returned Path: "<<path<<endl;
	ShowPathFromTo(id1, id2);
}

Router * TransitNetwork::GetTheEdgeRouterOf(string AS)
{
	for(RouterList::iterator iter = ptrListOfRouters.begin();iter != ptrListOfRouters.end();iter++)
	{
		if((*iter)->GetEdgeAs().compare(AS) == 0)
			return (*iter);
	}
	return NULL;
}

}


