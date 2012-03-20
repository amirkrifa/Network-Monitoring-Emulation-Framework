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

#include "Router.h"
#include "Link.h"
#include "CommonStructs.h"
#include "SoftflowdInterface.h"
#include "TimeStamp.h"

namespace AmDev{

extern TimeStamp currentTs;

Router::Router(int uAd, int id,SoftflowdInterface * sfi,int iMonitor,map<string, InterfaceSamplingStatus * > tmpMap, list<string> listEAs):Node(uAd)
{
  	try
	{
		
		done = false;
		// Init the random numbers generator
		time_t seconds;
		time(&seconds);
		srand((unsigned int)seconds);
		listNodes = NULL;
	
		if(iMonitor != 0 && iMonitor != 1 && iMonitor != -1)
			throw NspException("Invalid router monitoring status.",__FILE__,__LINE__);
		else iIsMonitor = iMonitor;
		
		if(sfi == NULL) throw NspException("Invalid interface to Softflowd.",__FILE__,__LINE__);
		ptrSoftflowdInterface=sfi;
		iRouterNumber=id;
		
		// Copy the Interfaces Staus Map
		if(!tmpMap.empty())
		{
			map<string, InterfaceSamplingStatus *>::iterator iter;
			for(iter = tmpMap.begin(); iter!= tmpMap.end(); iter++)
			{
				if(iter->second != NULL)
				{
					interfaceSamplingStatus.insert(pair<string, InterfaceSamplingStatus * >(iter->first, iter->second));
					iter->second = NULL;
				}
				else throw NspException("Invalid Status map.",__FILE__, __LINE__);
			}
		} else throw NspException("Invalid Status map.",__FILE__, __LINE__);
		
		// Copy the list of Edge ASs
		if(!listEAs.empty())
		{
			list<string>::iterator iterE = listEAs.begin();
			while(iterE != listEAs.end())
			{
				listEdgeAS.push_back(*iterE);
				iterE++;
			}
		}
		
		char idR[10];
		sprintf(idR, "%i", id);
		
		string inPacketsLog;
		inPacketsLog.append(idR);
		inPacketsLog.append("-inPacket.txt");
		
		string outPacketsLog;
		outPacketsLog.append(idR);
		outPacketsLog.append("-outPacket.txt");
		
		logInPackets.open(inPacketsLog.c_str(),ios::out|ios::app);
		logOutPackets.open(outPacketsLog.c_str(),ios::out|ios::app);
		
		inPacketsNumber = 0;
		outPacketsNumber = 0;
	}
	catch(exception &e)
	{
		cout<<e.what()<<endl;
	}
}

Router::~Router()
{ 
	WriteLogFiles();
	logOutPackets.close();
	logInPackets.close();
	// Cleaning the interface sampling status map
	if(!interfaceSamplingStatus.empty())
	{
		map<string, InterfaceSamplingStatus*>::iterator iter;
		while(!interfaceSamplingStatus.empty())
		{
			iter = interfaceSamplingStatus.begin();
			free(iter->second);
			interfaceSamplingStatus.erase(iter);
		}
	}
	if(!listEdgeAS.empty())
	{
		listEdgeAS.clear();
	}
	FreeListNodes();
}

void Router::WriteLogFiles()
{
	char id[20];
	sprintf(id,"router number: %i\n", iRouterNumber);
	logOutPackets<< id<<endl;
	logInPackets << id<<endl;	
	logOutPackets << outPacketsNumber<<endl;
	logInPackets << inPacketsNumber<<endl;
}

void Router::ShowDetails()
{
   	cout<<"          Monitor flag: "<<iIsMonitor<<endl; 
  	cout<<"          Number of adapters: "<< iNumberOfNetworkAdapters<<endl;
  	int i=1,res=0;
  	list<string>::iterator IpsIter; 
  	for(IpsIter=ListOfAdapters.begin();IpsIter!=ListOfAdapters.end();IpsIter++)
  	{
    		cout<<"              Adapter: "<<(*IpsIter)<<endl;
    		i++;
  	}
  	for(LinkList::iterator iter=ptrListOfLinks.begin();iter!=ptrListOfLinks.end();iter++)
  	{
    		cout<<"              Link ( adapter1: "<<(*iter)->GetAdapter1()<<" ,  adapter2: "<<(*iter)->GetAdapter2()<<", FinalDests: "<< (*iter)->GetListOfDestinations()<<" )"<<endl;
  	}
}

// Return the monitoring status of a given interface
int Router::GetInterfaceMonitoringStatus(string &interface, int inOut)
{
try
{
	map<string, InterfaceSamplingStatus *>::iterator iter;
	for(iter = interfaceSamplingStatus.begin(); iter != interfaceSamplingStatus.end(); iter++)
	{
		if(iter->first.compare(interface) == 0)
		{
			// We found the interface
			if(inOut == 0)
			{
				// Looking for the input interface monitoring and sampling status
				return (iter->second)->in;
			}
			else if(inOut == 1) 
			{
				// Looking for the output interface monitoring satatus
				return (iter->second)->out;
			
			}
		}
	}
	// We dont found the interface
	return 0;
}
catch(exception &e)
{
	cout << e.what() <<endl;
}	
}

// Forwards a given packet 
void Router::ForwardPacket(packet*pPacket,int & iSecondsToSleep, string & sNextInterface,Link * pCurrentLink, string  inInterface)
{
try 
{
	
  	if(pPacket==NULL)
	  	throw NspException("Invalid packet.",__FILE__,__LINE__);
  	if(pCurrentLink==NULL)
	  	throw NspException("Incorrect link.",__FILE__,__LINE__);
  	int iNumberOfNetworkAdapters;
  	Link *pChoosedLink=NULL;
  	int iMinCost=100;
  
  	string sFinalDest("As");
  	char szAsNumber[10] ;
  	sprintf(szAsNumber,"%i",pPacket->iAsDestNumber);
  	sFinalDest.append(szAsNumber);
	string tmpInterfaceTowardDest;
  	for(LinkList::iterator iter = ptrListOfLinks.begin();iter!=ptrListOfLinks.end();iter++)
  	{ 
		if((*iter)->IsItAFinalDest(sFinalDest))
      		{
			(*iter)->GetNextInterfaceTowrd(sFinalDest, tmpInterfaceTowardDest);
        		if((*iter)->GetCost() < iMinCost  && ( (*iter) != pCurrentLink || pPacket->iAsSourceNumber == pPacket->iAsDestNumber) && !IsItYourAdapter(tmpInterfaceTowardDest))
         		{ 
           			pChoosedLink = (*iter);
           			iMinCost = (*iter)->GetCost();
         		}
      		}
  	}

  	if(pChoosedLink!=NULL)
  	{ 
		pChoosedLink->GetNextInterfaceTowrd(sFinalDest, tmpInterfaceTowardDest);
		sNextInterface.assign(tmpInterfaceTowardDest);
    		string oppositeInterface(pChoosedLink->GetTheOppositeAdapter(sNextInterface));
		// Send the packet to the softflowd interface if this node is a monitor
    		if(ptrSoftflowdInterface!=NULL)
		{
			if(iIsMonitor == 1)
			{	
				pPacket->iResponsibleNode = GetRouterId();
				if(GetInterfaceMonitoringStatus(inInterface, 0) == 1 )
				{
					if(RandomPeriodicNodeSampling((char*)inInterface.c_str()))
					{
						// Monitoring is on in the Input interface
						strcpy(pPacket->szMonitorIp, inInterface.c_str());
						ptrSoftflowdInterface->PreparePacketToBeSent(pPacket);
					}
					
				}
				if(GetInterfaceMonitoringStatus(oppositeInterface, 1) == 1 )
				{
					// Monitoring is on in the Output interface
					if(RandomPeriodicNodeSampling((char*)oppositeInterface.c_str()))
					{
						strcpy(pPacket->szMonitorIp, oppositeInterface.c_str());
						ptrSoftflowdInterface->PreparePacketToBeSent(pPacket);
					}
				} 
				
			}else if((iIsMonitor == 0))
			{
				#ifdef SHOW_PACKETS_FORWRDING
				cout <<"Router number "<<iRouterNumber<<" at "<<currentTs.GetTime()<<" : "<<"forwarding a packet of size "<< pPacket->lLength<<" from "<<pPacket->szIpSource<<"( AS "<<pPacket->iAsSourceNumber<<")"<<" to "<<pPacket->szIpDest<<" within As number:  "<<pPacket->iAsDestNumber<<endl;
				#endif
			}
		}
		else throw NspException("Invalid pointer to the softflowd interface.",__FILE__,__LINE__);
		pChoosedLink = NULL;
  	}else throw NspException("Invalid Link.", __FILE__ , __LINE__);
}

catch(exception &e)
{
	cout << "Link: "<<pCurrentLink->GetAdapter1()<<" "<<pCurrentLink->GetAdapter2()<<endl;
	cout<<e.what()<<endl;
	pPacket->ShowDetails();
}
}

void Router::ForwardPacketE(packet*pPacket,int & iSecondsToSleep, Router * destRouter, string inInterface, string & interfaceOutIn)
{
try 
{
	if(GetRouterId() == 1)
	{	
		cout <<"forwarding a packet of size "<< pPacket->lLength<<" from "<<pPacket->szIpSource<<"( AS "<<pPacket->iAsSourceNumber<<")"<<" to "<<pPacket->szIpDest<<" within As number:  "<<pPacket->iAsDestNumber<<endl;
	}
	if(pPacket==NULL)
	  	throw NspException("Invalid packet.",__FILE__,__LINE__);
  	

	char szAsId[10];
  	sprintf(szAsId,"%i",pPacket->iAsSourceNumber);
  	string sSource(string("As")+string(szAsId));
	
  	string sFinalDest("As");
  	char szAsNumber[10] ;
  	sprintf(szAsNumber,"%i",pPacket->iAsDestNumber);
  	sFinalDest.append(szAsNumber);
	Link *	pChoosedLink;
	string interfaceWithinDest;
	
	if(destRouter != this)
	{	
		pChoosedLink = GetLinkTowards(destRouter, interfaceWithinDest);
		interfaceOutIn = interfaceWithinDest;
	}
	else 
	{
		pChoosedLink = GetLink(sFinalDest);
		interfaceOutIn = sFinalDest;
		interfaceWithinDest = sFinalDest;
	}

  	if(pChoosedLink!=NULL)
  	{ 
		string oppositeInterface(pChoosedLink->GetTheOppositeAdapter(interfaceWithinDest));
		// Send the packet to the softflowd interface if this node is a monitor
    		if(ptrSoftflowdInterface!=NULL)
		{
			if(iIsMonitor == 1)
			{	
				pPacket->iResponsibleNode = GetRouterId();
				
				if(sSource.compare(sFinalDest) == 0)
				{	
					//In
					if(GetInterfaceMonitoringStatus(inInterface, 0) == 1 )
					{
						if(RandomPeriodicNodeSampling((char*)inInterface.c_str()))
						{
							// Monitoring is on in the Input interface
		
							strcpy(pPacket->szMonitorIp, inInterface.c_str());
							ptrSoftflowdInterface->PreparePacketToBeSent(pPacket);
							inPacketsNumber++;
						}
					}
					
					// Out
					if(GetInterfaceMonitoringStatus(oppositeInterface, 1) == 1 )
					{
						// Monitoring is on in the Output interface
						if(RandomPeriodicNodeSampling((char*)oppositeInterface.c_str()))
						{

							strcpy(pPacket->szMonitorIp, oppositeInterface.c_str());
							ptrSoftflowdInterface->PreparePacketToBeSent(pPacket);
							outPacketsNumber++;
						}
					} 
				}
				else
				{
					if(GetInterfaceMonitoringStatus(inInterface, 0) == 1 )
					{
						if(RandomPeriodicNodeSampling((char*)inInterface.c_str()))
						{
							// Monitoring is on in the Input interface
	
							strcpy(pPacket->szMonitorIp, inInterface.c_str());
							ptrSoftflowdInterface->PreparePacketToBeSent(pPacket);
							inPacketsNumber++;
						}
					}
					
					// Out
					if(GetInterfaceMonitoringStatus(oppositeInterface, 1) == 1 )
					{
						// Monitoring is on in the Output interface
						if(RandomPeriodicNodeSampling((char*)oppositeInterface.c_str()))
						{

							strcpy(pPacket->szMonitorIp, oppositeInterface.c_str());
							ptrSoftflowdInterface->PreparePacketToBeSent(pPacket);
							outPacketsNumber++;
						}
					} 
				
				}
				
			}else if((iIsMonitor == 0))
			{
				#ifdef SHOW_PACKETS_FORWRDING
				cout <<"Router number "<<iRouterNumber<<" at "<<currentTs.GetTime()<<" : "<<"forwarding a packet of size "<< pPacket->lLength<<" from "<<pPacket->szIpSource<<"( AS "<<pPacket->iAsSourceNumber<<")"<<" to "<<pPacket->szIpDest<<" within As number:  "<<pPacket->iAsDestNumber<<endl;
				#endif
			}
		}
		else throw NspException("Invalid pointer to the softflowd interface.",__FILE__,__LINE__);
		pChoosedLink = NULL;
  	}else 
	{	
		cout<< "CurrentRouterId: "<<GetRouterId()<<endl;
		cout<< "Final Dest: "<< sFinalDest<<endl;
		throw NspException("Invalid Link.", __FILE__ , __LINE__);
	}
}

catch(exception &e)
{
	cout<<e.what()<<endl;
	pPacket->ShowDetails();
}
}

bool Router::IsEdgeTo(string & as)
{
try
{
	if(as.length() == 0)
		throw NspException("Invalid AS.", __FILE__, __LINE__);
	if(listEdgeAS.empty())
		return false;
	list<string>::iterator iter;
	iter = listEdgeAS.begin();
	while(iter != listEdgeAS.end())
	{
		if(iter->compare(as) == 0)
			return true;
		iter++;
	}
	return false;
}

catch(NspException &e)
{
	cout<<e.what()<<endl;
}
}

// Changing the sampling rate
int Router::ChangeSamplingRate(string adapter, int srP1, int srP2)
{
try
{
	char tmpIp[16];
	memset(tmpIp, '\0', 16);
	strcpy(tmpIp, (char*)adapter.c_str());
	fprintf(stdout, "Changing the sampling rate of the monitor %s to %i/%i.\n", tmpIp, srP1, srP2);	
	SamplingNode * sn = FindSamplingNode(tmpIp);
	if(sn != NULL)
	{
		sem_wait(&sn->sem);
			sn->samplingRateP1 = srP1;
			sn->samplingRateP2 = srP2;
			sn->samplingRateChanged = 1;
		sem_post(&sn->sem);
		sn = NULL;
		return 1;
	} else 
	{
		fprintf(stdout, "Error occured while trying to change the sampling rate: monitor %s was not found.\n", tmpIp);	
		return 0;	
	}
}
catch(exception &e)
{
	cout << e.what()<<endl;
}
}

SamplingNode * Router::AddNode(char * ip)
{
	char tmpIp[16];
	memset(tmpIp, '\0', 16);
	strcpy(tmpIp, ip);
	if(listNodes == NULL)
	{
		cout<<"Setting the sampling rate for: "<<tmpIp<<" to: "<<INITIAL_SAMPLING_RATE_P1<<"/"<<INITIAL_SAMPLING_RATE_P2<<endl;
		listNodes = (SamplingNode * )malloc(sizeof(SamplingNode));
		listNodes->next = NULL;
		listNodes->samplingRateP1 = INITIAL_SAMPLING_RATE_P1;
		listNodes->samplingRateP2 = INITIAL_SAMPLING_RATE_P2;
		listNodes->samplingRateChanged = 1;
		listNodes->samplingRateAxis = 0;
		listNodes->samplingTab = NULL;
		memset(listNodes->ipAddr, '\0', 16);
		strcpy(listNodes->ipAddr, tmpIp);
		sem_init(&listNodes->sem, 0, 1);
		// Init the random numbers generator
		time_t seconds;
		time(&seconds);
		srand((unsigned int)seconds);
		return listNodes;
	}else 
	{
		SamplingNode* sn = listNodes;
		while(sn->next != NULL)
		{
			sn = sn->next;
		}
		
		cout<<"Setting the sampling rate for: "<<tmpIp<<" to: "<<INITIAL_SAMPLING_RATE_P1<<"/"<<INITIAL_SAMPLING_RATE_P2<<endl;
		// Adding the node
		sn->next = (SamplingNode * )malloc(sizeof(SamplingNode));
		sn = sn->next;
		sn->samplingRateP1 = INITIAL_SAMPLING_RATE_P1;
		sn->samplingRateP2 = INITIAL_SAMPLING_RATE_P2;
		sn->samplingRateChanged = 1;
		sn->samplingRateAxis = 0;
		sn->samplingTab = NULL;
		memset(sn->ipAddr, '\0', 16);
		strcpy(sn->ipAddr, tmpIp);
		sem_init(&sn->sem, 0, 1);
		sn->next = NULL;
		return sn;
	}
		
}


// Look for a sampling Node given its ip addr, if not found create a new one
SamplingNode * Router::GetSamplingNode(char * ip)
{
	char tmpIp[16];
	memset(tmpIp, '\0', 16);
	strcpy(tmpIp, ip);
	if(listNodes == NULL)
	{
		// Adding a new node
		SamplingNode * tmpNode = AddNode(tmpIp);
		return tmpNode;	
	}
	else
	{
		SamplingNode * tmp = listNodes;
		while(tmp != NULL)
		{
			if(strcmp(tmp->ipAddr, tmpIp) == 0)
			{
				// Node found
				return tmp;
			}
			tmp = tmp->next;
		}
		if(tmp == NULL)
		{
			// Add a new Node 
			return AddNode(tmpIp);
		}
		return NULL;
	}
}


// Try to find a sampling node, returns NULL if not found
SamplingNode * Router::FindSamplingNode(char * ip)
{
	char tmpIp[16];
	memset(tmpIp, '\0', 16);
	strcpy(tmpIp, ip);
	if(listNodes == NULL)
	{
		return NULL;	
	}
	else
	{
		SamplingNode * tmp = listNodes;
		while(tmp != NULL)
		{
			if(strcmp(tmp->ipAddr, tmpIp) == 0)
			{
				// Node found
				return tmp;
			}
			tmp = tmp->next;
		}
		return tmp;
	}
}


// Deletes the listNodes
void Router::FreeListNodes()
{
	SamplingNode * tmpNode = NULL;
	if(listNodes != NULL)
	{
		while(listNodes != NULL)
		{
			tmpNode = listNodes;
			listNodes = listNodes->next;
			sem_destroy(&tmpNode->sem);
			free(tmpNode);
		}
	}
}




// Should be included in a thread safe code block
// Used to generate random choices
void Router::GenerateRandomChoicesForNode(SamplingNode * sn)
{
	if(sn->samplingRateChanged == 1)
	{
		// Delete the last random choices
		if(sn->samplingTab != NULL)
			free(sn->samplingTab);
		sn->samplingTab = (int *)malloc(sizeof(int) * sn->samplingRateP2);
		sn->samplingRateChanged = 0;
	}

	int i = 0;
	for(; i < sn->samplingRateP2; i++)
	{
		sn->samplingTab[i] = 0;
	}

	map<int, int> tmpIndexes;

	for(i = 0; i < sn->samplingRateP1; i++)
	{
		int tmpIndex = rand() % sn->samplingRateP2;

		while(tmpIndexes.find(tmpIndex) != tmpIndexes.end())
		{
			tmpIndex = rand() % sn->samplingRateP2;				
		}

		sn->samplingTab[tmpIndex] = 1;

		tmpIndexes.insert(make_pair<int, int>(tmpIndex, 1));
	}

	tmpIndexes.clear();

	
}

// a thread safe method that returns the sampling rate status of a given node
int Router::GetNodeSamplingRateStatus(SamplingNode *sn)
{
	int tmp;
	sem_wait(&sn->sem);
		tmp = sn->samplingRateChanged;
	sem_post(&sn->sem);
	return tmp;
}

// A thread safe method used to init the sampling rate status of a given node
void Router::InitNodeSamplingRateStatus(SamplingNode *sn)
{
	sem_wait(&sn->sem);
		sn->samplingRateAxis = 0;
	sem_post(&sn->sem);
}


/* Returns wether we should take the current packet or not. (decision / node)
*  0 : We have to escape the current packet.
*  1 : We have to consider the current packet.
*/
int Router::RandomPeriodicNodeSampling(char * ip)
{
	char tmpIp[16];
	memset(tmpIp, '\0', 16);
	strcpy(tmpIp, ip);
	
	// Retrive the SamplingNode
	SamplingNode *sn = GetSamplingNode(ip);
	if(sn == NULL)
	{
		fprintf(stderr, "%s:%i Invalid Sampling Node.", __FILE__, __LINE__);	
		return 0;
	}
	
	// Thread safe block
	if(GetNodeSamplingRateStatus(sn) == 1)
	{
		InitNodeSamplingRateStatus(sn);	
	}
	// Thread safe block
	
	// Thread safe block
	sem_wait(&sn->sem);
	
		if(sn->samplingRateP1 == 0)
		{
			// We dont take any packet
			sem_post(&sn->sem);	
			return 0;
		}else if(sn->samplingRateP1 == sn->samplingRateP2)
		{
			// We take all the packets
			sem_post(&sn->sem);	
			return 1;
		}else if(sn->samplingRateP1 < sn->samplingRateP2)
		{
			//We take samplingRateP1 packets over samplingRateP2
			if(sn->samplingRateAxis == 0)
			{
				// Generating new numbers
				GenerateRandomChoicesForNode(sn);
				int ret = sn->samplingTab[sn->samplingRateAxis];
				sn->samplingRateAxis ++;
				sem_post(&sn->sem);
				return ret;
			}else if(sn->samplingRateAxis < (sn->samplingRateP2 - 1))
			{
				int ret = sn->samplingTab[sn->samplingRateAxis];
				sn->samplingRateAxis++;
				sem_post(&sn->sem);	
				return ret;
			
			} else if(sn->samplingRateAxis == (sn->samplingRateP2 -1))
			{
				int ret = sn->samplingTab[sn->samplingRateAxis];	
				sn->samplingRateAxis = 0;	
				sem_post(&sn->sem);	
				return ret;
			}
		} else if(sn->samplingRateP1 > sn->samplingRateP2)
		{
			fprintf(stderr, "Invalid Sampling Rate.");
			sem_post(&sn->sem);	
			return 0;
		}
// Thread safe block


	
}

list<Link*> Router::GetListOfLinks()
{
	return ptrListOfLinks;
}

list<string> Router::GetListOfAdapters()
{
	return ListOfAdapters;
}
bool Router::WasVesitor(Router *p, string interface)
{
	if(!visitedBy.empty() || p == NULL || interface.length() ==0)
	{	
		map<Router*, string>::iterator iter =  visitedBy.begin();
		while(iter != visitedBy.end())
		{
			if(iter->first == p && iter->second.compare(interface) == 0)
				{return true;}
			iter++;
		}
		return false;
	}else 	return false;
}

void Router::ShowTheMapOfVisitedRouters()
{
	map<string, list<Router*> >::iterator iter = visitedRouters.begin();
	cout<<"Router Number: "<<GetRouterId()<<endl;
	while(iter != visitedRouters.end())
	{
		cout <<"Starting from the interface: "<<iter->first<<endl;
		list<Router*>::iterator iter2 = iter->second.begin();
		while(iter2 != iter->second.end())
		{
			cout<<"          RouteriterId: "<<(*iter2)->GetRouterId() <<endl;
			iter2++;
		}
		iter++;
	}

}

void Router::AddVisitedRouter(string i, Router* r)
{
	map<string, list<Router*> >::iterator iter;
	if(visitedRouters.empty())
	{
		list<Router*> tmp;
		tmp.push_back(r);
		visitedRouters.insert(make_pair<string, list<Router*> >(i, tmp));
		
		
	}else
	{
		if((iter = visitedRouters.find(i)) != visitedRouters.end())
			iter->second.push_back(r);
		else
		{
			list<Router*> tmp;
			tmp.push_back(r);
			visitedRouters.insert(make_pair<string, list<Router*> >(i, tmp));
		}
	}
}
string Router::GetEdgeAs()
{
	if(!listEdgeAS.empty())
	{
		list<string>::iterator iter = listEdgeAS.begin();
		return *iter;
	}
}
void Router::UpdateLinksDests()
{
	map<string, list<Router*> >::iterator iter = visitedRouters.begin();
	while(iter != visitedRouters.end())
	{
		Link * l = GetLink(iter->first);
		string ci = iter->first;
		string oi = l->GetTheOppositeAdapter(ci);
		list<Router*>::iterator iterR = iter->second.begin();
		while(iterR != iter->second.end())
		{
			
			l->mFinalDest.insert(make_pair<string, string>((*iterR)->GetEdgeAs(), oi));
			iterR++;
		}
		iter++;
	}
}

Router * Router::GetNextRouterToward(Router * r)
{
	if(r == this)
		return this;
	Router * tmp;
	Router * next = r;
	tmp = next;
	while((next = prevRouters[next]) != this)
	{
		if(next == NULL) break;
		tmp = next;
	}

	return tmp;
}

void Router::ShowDirectRoutersCost()
{
	cout << "Current Router ID: "<<this->GetRouterId()<<endl;
	for(map<Router*, int >::iterator iter = directRoutersCosts.begin(); iter != directRoutersCosts.end();iter++)
	{
		cout <<"	RouterId: "<< (iter->first)->GetRouterId()<<" cost: "<<iter->second<<endl;
	}

}

void Router::ShowAllRoutersCost()
{
	cout << "Current Router ID: "<<this->GetRouterId()<<endl;
	for(map<Router*, int >::iterator iter = allRoutersCosts.begin(); iter != allRoutersCosts.end();iter++)
	{
		cout <<"	RouterId: "<< (iter->first)->GetRouterId()<<" cost: "<<iter->second<<endl;
	}

}

	
void Router::ShowRoutersPaths()
{
	cout << "Current Router ID: "<<this->GetRouterId()<<endl;
	for(map<Router*, Router* >::iterator iter =  prevRouters.begin();iter !=  prevRouters.end();iter++)
	{
		cout << "Rp: "<<(iter->first)->GetRouterId()<<" Rn: "<<(iter->second)->GetRouterId()<<endl;
	}

}

Link* Router::GetLinkTowards(Router* dest, string & nextAdapter)
{	
	
	for(LinkList ::iterator iter = ptrListOfLinks.begin(); iter != ptrListOfLinks.end(); iter++)
	{
		if(dest->IsItYourAdapter((*iter)->GetAdapter1()))
		{
			nextAdapter = (*iter)->GetAdapter1();
			return (*iter);
		}
		if(dest->IsItYourAdapter((*iter)->GetAdapter2()))
		{
			nextAdapter = (*iter)->GetAdapter2();
			return (*iter);
		}
	
	}
	// Not a direct Node
	Router *directR = GetNextRouterToward(dest);
	string tmp;
	Link * tmpL = GetLinkTowards(directR, tmp);
	nextAdapter = tmp;
	return tmpL;
	
	
}
void Router::DrawDifferentPaths()
{
	for(map<Router*, int >::iterator iter = allRoutersCosts.begin();iter != allRoutersCosts.end();iter++)
	{
		nextRouter[iter->first] = this->GetNextRouterToward(iter->first);
	}
}

Router * Router::GetAlreadyCalculatedNextRouterToward(Router * r)
{
	return nextRouter[r];
}

void Router::ShowInterfacesSamplingStatus()
{
	cout<<"Router Id: "<<GetRouterId()<<endl;
	for(map<string, InterfaceSamplingStatus * >::iterator iter=interfaceSamplingStatus.begin();iter!=interfaceSamplingStatus.end();iter++)
	{
		cout<<"		Interface: "<<iter->first<<" In: "<<(iter->second)->in<<" out: "<<(iter->second)->out<<endl;
	}
}


}
