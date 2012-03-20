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
#include "As.h"
#include <stdlib.h>
#include <fstream>
#include <map>
#include "CommonStructs.h"
#include "SoftflowdInterface.h"
#include "Router.h"
#include "SamplingPlatform.h"
#include "SamplingManager.h"
namespace AmDev
{
extern int StopsControlServer;

SamplingPlatform::SamplingPlatform():iNumberOfAs(0),iNumberOfTransitNetworks(0),numberOfPackets(0),lastSelectedAs(1),dispatchingMethod(2)
{
	try
	{
		srand(time(NULL));
		lastTimeAxisSize = 0;
		timeAxisSize = 0;
		sem_init(&sendingSem, 0, 1);
		loading = false;
		// Creating the interface to softflowd
		ptrSoftflowdInterface = new SoftflowdInterface();
		// Starting the thread which will manage the interface to softflowd
		if(ptrSoftflowdInterface==NULL)
			throw NspException("Error occured while starting the softflowd interface thread.",__FILE__,__LINE__);
		else ptrSoftflowdInterface->StartTheInterfaceThread();
		// Creating the sampling manager
		this->sm = new SamplingManager(this);
		// Starting the Sampling Manager
		this->sm->StartControlServer();
		
	}

	catch(exception &e)
	{
		cout<<e.what()<<endl;
	}
}

SamplingPlatform::~SamplingPlatform()
{


sem_destroy(&sendingSem);

//cleaning the time axis
timeAxis.clear();

if(!asImportance.empty())
	asImportance.clear();

// Stopping the softflowd interface thread
if(ptrSoftflowdInterface!=NULL)
{
	ptrSoftflowdInterface->SetInterfaceStatus(false);
	delete ptrSoftflowdInterface;
}
	
ListOfAs::iterator iter;
ListOfTransitNetworks::iterator iter2;

//  Cleaning the list of As
if(!ptrListOfAs.empty())
{
	while(!ptrListOfAs.empty())
	{
		iter =ptrListOfAs.begin();
		delete *iter;
		ptrListOfAs.erase(iter);
	}
}

//Cleaning the list of Transit Networks
if(!ptrListOfTransitNetwork.empty())
{
	while(!ptrListOfTransitNetwork.empty())
	{
		iter2 =ptrListOfTransitNetwork.begin();
		delete *iter2;
		ptrListOfTransitNetwork.erase(iter2);
	}
}

// Cleaning the uploaded packets
list<couple>::iterator iterCouples;
string sCommand;
for(iterCouples=lAsCouples.begin();iterCouples!=lAsCouples.end();iterCouples++)
{
	ListOfPackets::iterator iterLP= ListOfPacketMatrix[(*iterCouples).AsSource][(*iterCouples).AsDest].begin();
	while(iterLP!= ListOfPacketMatrix[(*iterCouples).AsSource][(*iterCouples).AsDest].end())
	{ 
		free(*iterLP);
		iterLP++;
	}
	ListOfPacketMatrix[(*iterCouples).AsSource][(*iterCouples).AsDest].clear();
	// Cleaning temporary files 
	sCommand.append("rm -f ");
	sCommand.append(TrafficMatrix[(*iterCouples).AsSource][(*iterCouples).AsDest]);
	try
  	{
		if(system(sCommand.c_str()))
			throw NspException("Error encountred while executing the system() function.",__FILE__,__LINE__);
    		sCommand.clear();
  	}
	catch(exception &e)
	{
		cout<<e.what()<<endl;
	}
}

  
if(!pListOfPacketToSend.empty()) pListOfPacketToSend.clear();
if(!lAsCouples.empty()) lAsCouples.clear();
// Deleting the sampling manager
if(this->sm)
	delete this->sm;

}

void SamplingPlatform::StartTrafficLoader(char* szFn)
{
try
{
	if(szFn==NULL)
		throw NspException("Error encountred while starting the traffic loader.",__FILE__,__LINE__);
	Tl= new TrafficLoader(szFn);
}

catch(exception &e)
{
	cout<<e.what()<<endl;
}
}

void SamplingPlatform::AddNewAs(string & IpClass, string &IpRange, unsigned int method,string & IpsListFile)
{
try
{
	if((IpClass.length()==0 && IpRange.length()==0)||IpsListFile.length()==0)
		throw NspException("Error encountred while adding a new AS.",__FILE__,__LINE__);
	if(method==1)
	{
		// Grep ips based on the given class
		// Generating the differents Classes
		if(ptrListOfAs.empty())
		{

		//       We Create first the list of all Ips
			string IpsListFile("IpsList");
      			Tl->ParseIpsBasedOnClass(IpsListFile);
      			string ResultingFile(IpClass+string(".txt"));
      			iNumberOfAs++;
      			As * ptrAs=new As(ResultingFile,iNumberOfAs);
	  		if(ptrAs==NULL)
        			throw NspException("Error encountred while creating a new AS.",__FILE__,__LINE__);
	 		ptrListOfAs.push_back(ptrAs);
      			ptrAs=NULL;

    		}else
			{

				//       We suppose that the List of Ips is already created
     				 string ResultingFile(IpClass+string(".txt"));
      				 iNumberOfAs++;
      				As * ptrAs=new As(ResultingFile,iNumberOfAs);
	  			if(ptrAs==NULL)
        				throw NspException("Error encountred while creating a new AS.",__FILE__,__LINE__);
      				ptrListOfAs.push_back(ptrAs);
      				ptrAs=NULL;

    			}
  	}else if(method==2)
         {
          // Grep Ips based on the given range
         }  
}
catch(exception &e)
{
	cout<<e.what()<<endl;
}
}

// overloading the Method AddNewas
void SamplingPlatform::AddNewAs(int iAsNumber, int iAsMaxIps, int iAsImportance,int minInterval, int maxInterval, string & ipClass, string &ipRangeStart, string &ipRangeEnd, int ipMask, int dispatchingMethod )
{
try
{
	if((ipClass.length() == 0 && ipRangeStart.length() == 0 && ipRangeEnd.length() == 0))
		throw NspException("Error encountred while adding a new AS.",__FILE__,__LINE__);
	iNumberOfAs++;
	As * ptrAs=new As(iAsNumber, iAsMaxIps, iAsImportance,minInterval, maxInterval, (char *) ipClass.c_str(), (char *)ipRangeStart.c_str(), (char*)ipRangeEnd.c_str(), ipMask, dispatchingMethod);
	if(ptrAs==NULL)
		throw NspException("Error encountred while creating a new AS.",__FILE__,__LINE__);
	ptrListOfAs.push_back(ptrAs);
	ptrAs=NULL;

}
catch(exception &e)
{
	cout<<e.what()<<endl;
}
}

// Adds the list of edge ASs which were already described in the xml file
void SamplingPlatform::AddEdgeAS(string &sTopologyFile)
{
try
{
	if(sTopologyFile.length()==0)
		throw NspException("Invalid topology file.",__FILE__,__LINE__);
	if(Tl->IsReadable(sTopologyFile))
	{
    		TiXmlDocument doc((char *) sTopologyFile.c_str());
		bool bLoad=doc.LoadFile();
		if(bLoad)
		{
			TiXmlElement* pRoot=doc.FirstChildElement("EdgeNetwork");
			if(pRoot==NULL)
				throw NspException("Error while parsing the xml file.",__FILE__,__LINE__);
			
			// Extracting the links
			TiXmlElement* pElemListAs = pRoot->FirstChildElement("ListOfAs");
			if(pElemListAs == NULL)
				throw NspException("Error while parsing the xml file.",__FILE__,__LINE__);
			// The number of ASs
			int NumberOfAs = atoi(pElemListAs->Attribute("NumberOfAs"));
			lAsCouples.clear();
			for(int i = 1; i<= NumberOfAs;i++)
				for(int j=1; j <= NumberOfAs; j++)
				{
						lAsCouples.push_back(couple(i, j));
						
				}
			this->dispatchingMethod = atoi(pElemListAs->Attribute("DispatchingMethod"));
			this->ipMask = atoi(pElemListAs->Attribute("IpMask"));

			TiXmlElement* pElemAS = pRoot->FirstChildElement("ListOfAs")->FirstChildElement("AS");
			if(pElemAS==NULL)
				throw NspException("Error while parsing the xml file.",__FILE__,__LINE__);
			char *res;
			// Extract the different ASs and their attributes
			for(pElemAS;pElemAS;pElemAS=pElemAS->NextSiblingElement("AS"))
			{
				int iAsNumber = atoi(pElemAS->Attribute("Number"));
				int iAsMaxIps = atoi(pElemAS->Attribute("MaxNumberOfIps"));
				int iAsImportance = atoi(pElemAS->Attribute("Importance"));
				int minInterval = atoi(pElemAS->Attribute("Min"));
				int maxInterval = atoi(pElemAS->Attribute("Max"));
				
				res = (char*)pElemAS->Attribute("UniqueIpClass");
				string sIpClass;
				if(res != NULL)
					sIpClass.assign(res);
				else 	throw NspException("Error occured while parsing the EdgeAsFile.",__FILE__,__LINE__);

				res = (char*)pElemAS->Attribute("IpsRangeStart");	
				string sIpsRangeStart;
				if(res != NULL)
					sIpsRangeStart.assign(res);
				else 	throw NspException("Error occured while parsing the EdgeAsFile.",__FILE__,__LINE__);


				res = (char*)pElemAS->Attribute("IpsRangeEnd");
				string sIpsRangeEnd;
				if(res != NULL)
					sIpsRangeEnd.assign(res);
				else 	throw NspException("Error occured while parsing the EdgeAsFile.",__FILE__,__LINE__);

				if(dispatchingMethod == 1 || dispatchingMethod == 3)
				{
					// Method based on the Ip Mask
					asImportance.insert(pair<int, percentageCount>(iAsNumber,percentageCount(iAsImportance, 0)));
				}
				// Adding a new AS
				this->AddNewAs(iAsNumber, iAsMaxIps, iAsImportance, minInterval, maxInterval, sIpClass, sIpsRangeStart, sIpsRangeEnd, ipMask, dispatchingMethod);
			}

			// Extracting the different data records files 
			/*
			TiXmlElement* pElemFile = pRoot->FirstChildElement("DataRecordsFiles")->FirstChildElement("File");
			if(pElemFile==NULL)
				throw NspException("Error while parsing the xml file.",__FILE__,__LINE__);

			// Extract the different DataRecords Files and their attributes
			
			for(pElemFile; pElemFile; pElemFile = pElemFile->NextSiblingElement("File"))
			{
				int iAsSourceNumber = atoi(pElemFile->Attribute("AsSourceNumber"));
				int iAsDestNumber = atoi(pElemFile->Attribute("AsDestNumber"));
				
				res = (char*)pElemFile->Attribute("AbsolutePath");
				if(res!= NULL)
					TrafficMatrix[iAsSourceNumber][iAsDestNumber].assign(res); 
				else
					throw NspException("Error occured while parsing the EdgeAsFile.",__FILE__,__LINE__);
			
				// Adding the new AS couple
				lAsCouples.push_back(couple(iAsSourceNumber, iAsDestNumber));
			}
			*/
		fprintf(stdout,"ASs Configuration file %s loaded.\n", sTopologyFile.c_str());
		} else   throw NspException("Error while parsing the xml file.",__FILE__,__LINE__);
	}else   throw NspException("Error while parsing the xml file.",__FILE__,__LINE__);
}

catch(exception &e)
{
	cout<<e.what()<<endl;
}

}

// Method used as part of the dispatching method number 2, Selects the next As based on the relative percentages
int SamplingPlatform::SelectAs()
{
try
{	if(asImportance.empty())
	{
		throw NspException("AS not found.", __FILE__, __LINE__);
	}

	map<int, percentageCount>::iterator iter = asImportance.find(lastSelectedAs);

	if(iter == asImportance.end())
		throw NspException("AS not found.", __FILE__, __LINE__);

	if((iter->second).Increment() == 0)
	{
		// Pass to the next As
		lastSelectedAs = (lastSelectedAs + 1) % asImportance.size();
		if(lastSelectedAs == 0) lastSelectedAs = asImportance.size();
		return lastSelectedAs;
	} else 
	{
		// Stay within the same AS
		return lastSelectedAs;
	}
}

catch(exception &e)
{
	cout<<e.what()<<endl;
}
}

int SamplingPlatform::SelectAsRandomlyBasedOnWeights()
{
try
{	if(asImportance.empty())
	{
		throw NspException("AS not found.", __FILE__, __LINE__);
	}

	lastSelectedAs = GetRandomAsNumber();
	return lastSelectedAs;
}

catch(exception &e)
{
	cout<<e.what()<<endl;
}
}

int SamplingPlatform::GetRandomAsNumber()
{
try
{
	int randomPosition = rand()%101;
	
	ListOfAs::iterator iterAs=ptrListOfAs.begin();
	while(iterAs != ptrListOfAs.end())
	{
		if((*iterAs)->IsItWithinInterval(randomPosition) == 1)
			break;
		iterAs++;
	}
	if(iterAs == ptrListOfAs.end())
		throw NspException("AS not found.", __FILE__, __LINE__);

	return (*iterAs)->GetNumber(); 
}

catch(exception &e)
{
	cout<<e.what()<<endl;
}

}
// Parse and load ASCII Traffic file 
void SamplingPlatform::LoadTrafficFile(string &trafficFile)
{
try
{
	if(Tl->IsReadable(trafficFile))
  	{
		ifstream file;
    		file.open(trafficFile.c_str(),ios::out);
    		string sDataRecord;
    		getline(file,sDataRecord);
		
			while(!sDataRecord.empty())
    			{
				if(Tl->IsItValid(sDataRecord))
				{	
		  
    					string sourceIp = Tl->GetDataRecordSourceIp(sDataRecord);
					string destinationIp = Tl->GetDataRecordDestIp(sDataRecord);
					string protocol = Tl->GetDataRecordProtocol(sDataRecord);
					string sourcePort = Tl->GetDataRecordSourcePort(sDataRecord);
					string destinationPort = Tl->GetDataRecordDestPort(sDataRecord);
					
 					
	      				if(sourceIp.length() > 0 && destinationIp.length() > 0)
      					{
						list<couple>::iterator iterAs = lAsCouples.begin();
						while(iterAs != lAsCouples.end())
						{
							As * AsSource = GetAs(iterAs->AsSource);
							As * AsDest = GetAs(iterAs->AsDest);
							if(AsSource == NULL || AsDest == NULL)
								throw NspException("Invalid As number given in the xml configuration file.",__FILE__, __LINE__);
							if(AsSource->VerifyIpAdr(sourceIp,1) && AsDest->VerifyIpAdr(destinationIp,1))
							{
								// Add the Ips to ASs
								AsSource->AddIp(sourceIp);
								AsDest->AddIp(destinationIp);
							
								// Adding the packets to the list
								long int lPacketSize = Tl->GetPacketSizeFromTheDataRecord(sDataRecord);
      								long int lTimeStamp = Tl->GetPacketTimeStampFromTheDataRecord(sDataRecord);
    								long int lTimeStampSecond = Tl->GetPacketTimeStampSecondFromTheDataRecord(sDataRecord);
					
								// Filling in the packet
								packet *p = (packet*) malloc(sizeof(packet));
        							p->lLength = lPacketSize;
        							
								p->SetTimeStamp(lTimeStamp,lTimeStampSecond);
								
								
								TimeStamp tmpTs(0, 0);
								p->GetTimeStamp(tmpTs);	
								if(maxTs.Compare(tmpTs) > 0)
        							{
									maxTs.Copy(tmpTs);
								}

								// building the timestamp axis
								timeAxis.push_back(tmpTs);
								
        						strcpy(p->szIpDest,destinationIp.c_str());
        			     		strcpy(p->szIpSource,sourceIp.c_str());
        						strcpy(p->szDestPort,sourcePort.c_str());
								strcpy(p->szSourcePort,destinationPort.c_str());
								strcpy(p->szProtocol, protocol.c_str());
								if(Tl->IsFragment(sDataRecord))
								{
									p->fragment = true;
									p->fragmentSeqNumber = Tl->GetFragmentSeqNumber(sDataRecord);
								}else {p->fragment = false; p->fragmentSeqNumber = 0;}
	
								
								p->iResponsibleNode=-1;
								p->iAsSourceNumber = iterAs->AsSource;
        							p->iAsDestNumber = iterAs->AsDest;
        							
								if(p->iAsSourceNumber < 0 || p->iAsDestNumber < 0)
									throw NspException("Invalid ASs number.",__FILE__,__LINE__);
								ListOfPacketMatrix[iterAs->AsSource][iterAs->AsDest].push_back(p);
        							p=NULL;
							}	
							iterAs++;
						}
        					
					}
      				}
				sDataRecord.assign("");
        			getline(file,sDataRecord);
    			}
	cout<<__FILE__<<":"<<__LINE__<<" Packets and Ips are idspatched correctly, TimeAxis: "<<timeAxis.size()<<endl;
  	}else  throw NspException("Error encountred while reading the traffic file.",__FILE__,__LINE__);
}
catch(exception &e)
{
	cout<<e.what()<<endl;
}
}
// Adds a new transit network which is already described in the xml file
void SamplingPlatform::AddNewTransitNetwork(string &sTopologyFile)
{
try
{
	if(sTopologyFile.length()==0)
		throw NspException("Invalid topology file.",__FILE__,__LINE__);
	if(Tl->IsReadable(sTopologyFile))
	{
    		TiXmlDocument doc((char *) sTopologyFile.c_str());
		bool bLoad=doc.LoadFile();
		if(bLoad)
		{
			TransitNetwork * ptrTransitNetwork = new TransitNetwork();
			ptrListOfTransitNetwork.push_back(ptrTransitNetwork);
			iNumberOfTransitNetworks++;

			// Extract the attributes of the transit Network
			int iNumberOfRouters=0;
			int iNumberOfLinks=0;     
			TiXmlElement* pRoot=doc.FirstChildElement("TransitNetwork");
			if(pRoot==NULL)
				throw NspException("Error while parsing the xml file.",__FILE__,__LINE__);
			iNumberOfRouters=atoi(pRoot->Attribute("NumberOfRouters"));
			iNumberOfLinks=atoi(pRoot->Attribute("NumberOfLinks"));
			ptrTransitNetwork->SetTheNumberOfRouters(iNumberOfRouters);
			ptrTransitNetwork->SetTheNumberOfLinks(iNumberOfLinks);
																			
			// Extracting the links
			TiXmlElement* pElemLinks = pRoot->FirstChildElement("Links");
			if(pElemLinks == NULL)
				throw NspException("Error while parsing the xml file.",__FILE__,__LINE__);
			TiXmlElement* pElemLink = pRoot->FirstChildElement("Links")->FirstChildElement("Link");
			if(pElemLink == NULL)
				throw NspException("Error while parsing the xml file.",__FILE__,__LINE__);
			map<string,string> mFinalDest;
			for(pElemLink;pElemLink;pElemLink=pElemLink->NextSiblingElement("Link"))
			{
				int iBandwidth = atoi(pElemLink->Attribute("Bandwidth"));
				string sAdapter1(pElemLink->Attribute("Adapter1"));
				string sAdapter2(pElemLink->Attribute("Adapter2"));
				int iLinkNumber = atoi(pElemLink->Attribute("Number"));
				int iCost = atoi(pElemLink->Attribute("Cost"));
				int iDelay = atoi(pElemLink->Attribute("Delay"));
				int iNumberOfDest = atoi(pElemLink->Attribute("NumberOfDest"));
				// Extract the list of final destinations
        
				TiXmlElement* pElemDest = pElemLink->FirstChildElement("Dest");
				if(pElemDest == NULL)
					throw NspException("Error while parsing the xml file.",__FILE__,__LINE__);
				for(pElemDest;pElemDest;pElemDest=pElemDest->NextSiblingElement("Dest"))
				{
					string sFd(pElemDest->Attribute("Destination"));
					int iDestNumber = atoi(pElemDest->Attribute("Number"));
					string sNextInterfaceTowardFd(pElemDest->Attribute("InterfaceTowardDest"));
					mFinalDest.insert(pair<string,string>(sFd,sNextInterfaceTowardFd));
				}

				if(mFinalDest.empty()) cout<<"mFinalDest.size() == 0"<<endl;
				ptrTransitNetwork->AddLink(sAdapter1,sAdapter2,iBandwidth,iCost,iDelay,iNumberOfDest,mFinalDest);
				mFinalDest.clear();
			}

			// Extracting the nodes
			TiXmlElement* pElemRouter = pRoot->FirstChildElement("Nodes")->FirstChildElement("Router");
			if(pElemRouter == NULL)
				throw NspException("Error while parsing the xml file.",__FILE__,__LINE__);
			int iRouterNumber=0;
			int iRouterNumberOfAdapters=0;
			int iIsMonitor=0;
			list<string > listedgeAs;
			for(pElemRouter;pElemRouter;pElemRouter=pElemRouter->NextSiblingElement("Router"))
			{
				// Router Number
				iRouterNumber=atoi(pElemRouter->Attribute("Number"));
				// Number of Adapters of the Router 
				iRouterNumberOfAdapters=atoi(pElemRouter->Attribute("NumberOfAdapters"));
				// Is it a monitoring point
				iIsMonitor=atoi(pElemRouter->Attribute("IsMonitor"));
				// Get The Edge AS
				listedgeAs.push_back(string(pElemRouter->Attribute("EdgeAs")));
				// Get the list of adapters
				TiXmlElement* pElemInterface = pElemRouter->FirstChildElement("Interface");
				if(pElemInterface == NULL)
					throw NspException("Error while parsing the xml file.",__FILE__,__LINE__);
				list<string> lAdapters;
				map<string, InterfaceSamplingStatus*> tmpMap;
				for(pElemInterface;pElemInterface;pElemInterface=pElemInterface->NextSiblingElement("Interface"))
				{
					string sIp(pElemInterface->Attribute("Ip"));
					// Interfaces status
					int inStatus = atoi(pElemInterface->Attribute("In"));
					int outStatus = atoi(pElemInterface->Attribute("Out"));
					InterfaceSamplingStatus *iss = (InterfaceSamplingStatus *)malloc(sizeof(InterfaceSamplingStatus));
					iss->in = inStatus;
					iss->out = outStatus;
					tmpMap.insert(pair<string, InterfaceSamplingStatus * >(sIp, iss));
						
					string sName(pElemInterface->GetText());
					lAdapters.push_back(sIp);
				}
				// Adding the router to the transit Network
       
				ptrTransitNetwork->AddRouter(iRouterNumber,iRouterNumberOfAdapters,lAdapters,ptrSoftflowdInterface,iIsMonitor, tmpMap, listedgeAs);
				tmpMap.clear();
				listedgeAs.clear();
			}
			cout << "Topology File: "<<sTopologyFile.c_str()<<" loaded."<<endl;
		} else   throw NspException("Error while parsing the xml file.",__FILE__,__LINE__);
	}else   throw NspException("Error while parsing the xml file.",__FILE__,__LINE__);
	  
}

catch(exception &e)
{
	cout<<e.what()<<endl;
}

}

void SamplingPlatform::ShowListOfAs() 
{
	ListOfAs::iterator iter=ptrListOfAs.begin();
	while(iter!=ptrListOfAs.end())
	{
		(*iter)->ShowDetails();
		iter++;
	}
}

As * SamplingPlatform::GetAs(int iNumber)
{
try
{
	if(iNumber<0)
		throw NspException("Invalid AS number.",__FILE__,__LINE__);
	ListOfAs::iterator iter=ptrListOfAs.begin();
	while(iter!=ptrListOfAs.end())
	{
		if((*iter)->GetNumber()==iNumber)
			return *iter;
		iter++;
	}
	return NULL;
}
catch(exception &e)
{
	cout<<e.what()<<endl;
}
}

bool SamplingPlatform::GenerateTrafficBetween(list<couple> lCouples)
{
try
{
	if(lCouples.empty())
		throw NspException("Error while generating traffic between AS(s).",__FILE__,__LINE__);
	As * ptrAsSource=NULL, *ptrAsDest=NULL;
	string sCouples;
	for (list<couple>::iterator iter=lCouples.begin();iter!=lCouples.end(); iter++)
	{
		ptrAsSource=GetAs(iter->AsSource);
		ptrAsDest=GetAs(iter->AsDest);
		if(ptrAsSource == NULL || ptrAsDest == NULL)
			throw NspException("Invalid As number given in the xml configuration file.",__FILE__, __LINE__);
		if(ptrAsSource!=NULL && ptrAsDest!=NULL)
		{ 
			ptrAsSource->GenerateIpsListFile();
			ptrAsDest->GenerateIpsListFile();

			sCouples.append(ptrAsSource->GetListOfIpsFilePath()+string(" ")); 
			sCouples.append(ptrAsDest->GetListOfIpsFilePath()+string(" ")); 

			TrafficMatrix[iter->AsSource][iter->AsDest].assign(ptrAsSource->GetListOfIpsFilePath()+string("-")+ptrAsDest->GetListOfIpsFilePath());
			lAsCouples.push_back(couple(iter->AsSource,iter->AsDest));

		}
		else {  throw NspException("Could not find the correct AS.",__FILE__,__LINE__);}
  	}
}
catch(exception &e)
{
  	cout<<e.what()<<endl;
}
}

// Generates the traffic file just between one couple of AS(s)
bool SamplingPlatform::GenerateTrafficBetween(int iAsSource, int iAsDest)
{
try
{
	As * ptrAsSource = GetAs(iAsSource);
  	As * ptrAsDest = GetAs(iAsDest);
  	if(ptrAsSource == NULL || ptrAsDest == NULL)
		throw NspException("Invalid As number given in the xml configuration file.",__FILE__, __LINE__);
  	if(ptrAsSource!=NULL && ptrAsDest!=NULL)
  	{
   		 //  Parse SrcIpsFile DstIpsFile DumpFile -a ClassifyTraffic
    		ptrAsSource->GenerateIpsListFile();
    		ptrAsDest->GenerateIpsListFile();
    		string sCommand;
    		sCommand.append("./Parse ");
   		sCommand.append(ptrAsSource->GetListOfIpsFilePath()+string(" "));
   		sCommand.append(ptrAsDest->GetListOfIpsFilePath()+string(" "));
   		sCommand.append(Tl->GetDumpFilePath()+string(" "));
   		sCommand.append(string("-a ClassifyTraffic"));
    
    		system((char *)sCommand.c_str());
    		// Adding the resulting file to the traffic matrix
    		TrafficMatrix[iAsSource][iAsDest].assign(ptrAsSource->GetListOfIpsFilePath()+string("-")+ptrAsDest->GetListOfIpsFilePath());
    		lAsCouples.push_back(couple(iAsSource,iAsDest));
    		ptrAsSource=NULL;
    		ptrAsDest=NULL;
    		return true;
   	}else   throw NspException("Invalid AS(s) couple.",__FILE__,__LINE__);
	  
    	ptrAsSource=NULL;
    	ptrAsDest=NULL;
  	return false;
}

catch(exception &e)
{
	cout<<e.what()<<endl;
}
}


bool SamplingPlatform::ConnectAsTo(TransitNetwork* const ptrTn)
{
try
{
  	if(ptrTn == NULL)
    		throw NspException("Invalid transit Network.",__FILE__,__LINE__);
	  
  	ListOfAs::iterator AsIter;
  	if(!ptrListOfAs.empty() )
  	{
    		for(AsIter=ptrListOfAs.begin();AsIter!=ptrListOfAs.end();AsIter++)
    		{
      			if(*AsIter!=NULL)
        			ptrTn->ConnectToAs(*AsIter);
	  		else   throw NspException("Invalid AS.",__FILE__,__LINE__);
		}
    		return true;
  	} else return false;
}

catch(exception &e)
{
  cout<<e.what()<<endl;
}
}

bool SamplingPlatform::ConnectTransitNetworksToAss()
{
try
{
  	if(ptrListOfTransitNetwork.empty())
     		throw NspException("The list of transit networks is empty.",__FILE__,__LINE__);
  	ListOfTransitNetworks::iterator TnIter;
  	for(TnIter=ptrListOfTransitNetwork.begin();TnIter!=ptrListOfTransitNetwork.end();TnIter++)
  	{
    		ConnectAsTo(*TnIter); 
  	}
  	return true;
}

catch(exception &e)
{
	cout<<e.what()<<endl;
}

}

bool SamplingPlatform::ExploreTransitNetworks()
{
try
{
  	if(ptrListOfTransitNetwork.empty())
     		throw NspException("The list of transit networks is empty.",__FILE__,__LINE__);
  	ListOfTransitNetworks::iterator TnIter;
  	for(TnIter=ptrListOfTransitNetwork.begin();TnIter!=ptrListOfTransitNetwork.end();TnIter++)
  	{
    		(*TnIter)->ExploreGraph();
  	}
  	return true;
}

catch(exception &e)
{
	cout<<e.what()<<endl;
}

}
void SamplingPlatform::ShowGeneratedTopology()
{
  	ListOfAs::iterator AsIter;
  	cout<<"------------------------ Generated Topology----------------------------"<<endl;
  	cout<<"Number of As(s): "<<iNumberOfAs<<endl;
  	cout<<"Number of Transit Networks: "<<iNumberOfTransitNetworks<<endl;
  	for(AsIter=ptrListOfAs.begin();AsIter!=ptrListOfAs.end();AsIter++)
  	{
    		(*AsIter)->ShowDetails();
  	}
  	cout<<"-----------------------------------------------------------------------"<<endl;
}


void SamplingPlatform::ShowListOfTransitNetworks()
{
  	ListOfTransitNetworks::iterator TnIter;int i=1;
  	for(TnIter=ptrListOfTransitNetwork.begin();TnIter!=ptrListOfTransitNetwork.end();TnIter++)
  	{
    		cout<<"Transit Netwoork Number: "<<i<<endl;
    		(*TnIter)->ShowListOfLinks();
  	}
}


// read the traffic file and upload it to the mat(*iter)->rix
void SamplingPlatform::UpdateTheListOfPacket(const int AsSrc,const int AsDest)
{
try
{
  	if(Tl->IsReadable(TrafficMatrix[AsSrc][AsDest]))
  	{
		ifstream file;
    		file.open(TrafficMatrix[AsSrc][AsDest].c_str(),ios::out);
    		string sDataRecord;
    		getline(file,sDataRecord);
		while(!sDataRecord.empty())
    		{
      			long int lPacketSize = Tl->GetPacketSizeFromTheDataRecord(sDataRecord);
      			long int lTimeStamp = Tl->GetPacketTimeStampFromTheDataRecord(sDataRecord);
    			long int lTimeStampSecond = Tl->GetPacketTimeStampSecondFromTheDataRecord(sDataRecord);
 			
      			if(lPacketSize!=-1 && lTimeStamp!=-1)
      			{
        			packet *p = (packet*) malloc(sizeof(packet));
        			p->lLength = lPacketSize;
				p->SetTimeStamp(lTimeStamp, lTimeStampSecond);
        			
				TimeStamp tmpTs(0, 0);
				p->GetTimeStamp(tmpTs);
				if(maxTs.Compare(tmpTs) > 0)
        			{
					maxTs.Copy(tmpTs);

				}
				
				strcpy(p->szIpDest,Tl->GetDataRecordDestIp(sDataRecord).c_str());
        			strcpy(p->szIpSource,Tl->GetDataRecordSourceIp(sDataRecord).c_str());
        			strcpy(p->szDestPort,Tl->GetDataRecordDestPort(sDataRecord).c_str());
				strcpy(p->szSourcePort, Tl->GetDataRecordSourcePort(sDataRecord).c_str());
				strcpy(p->szProtocol, Tl->GetDataRecordProtocol(sDataRecord).c_str());
				if(Tl->IsFragment(sDataRecord))
				{
					p->fragment = true;
					p->fragmentSeqNumber = Tl->GetFragmentSeqNumber(sDataRecord);
				} else {p->fragment = false; p->fragmentSeqNumber = 0;}
				p->iResponsibleNode=-1;
				p->iAsSourceNumber=AsSrc;
        			p->iAsDestNumber=AsDest;
        			ListOfPacketMatrix[AsSrc][AsDest].push_back(p);
        			p=NULL;
      			}
        		getline(file,sDataRecord);
    		}
    
  	}else  throw NspException("Error encountred while reading the traffic file.",__FILE__,__LINE__);

}

catch(exception &e)
{
  cout<<e.what()<<endl;
}
}

// read all the traffic files and upload them to the matrix
void SamplingPlatform::UpdateTheListOfPacketMatrix()
{
try
{
  	if(lAsCouples.empty())
    		throw NspException("Error while reading the list of AS(s) couples.",__FILE__,__LINE__);
  	list<couple>::iterator iter;
  	for(iter=lAsCouples.begin();iter!=lAsCouples.end();iter++)
  	{
		UpdateTheListOfPacket(iter->AsSource,iter->AsDest);
 	}
}

catch(exception &e)
{
  	cout<<e.what()<<endl;
}
}

// Updates the list of packets to be sent at a given time stamp
unsigned int SamplingPlatform::UpdateTheListOfPacketsToSend(TimeStamp &ts)
{
try
{
	unsigned int numberOfPacktes = 0;
  	if(lAsCouples.empty())
   		throw NspException("Error while reading the list of AS(s).",__FILE__,__LINE__);

  	pListOfPacketToSend.clear();
  	list<couple>::iterator iter;
	TimeStamp tmpTs(0, 0);
  	for(iter=lAsCouples.begin();iter!=lAsCouples.end();iter++)
  	{
		if(!ListOfPacketMatrix[(*iter).AsSource][(*iter).AsDest].empty())
	  	{
    			ListOfPackets::iterator iter2 = ListOfPacketMatrix[(*iter).AsSource][(*iter).AsDest].begin();
    			while(iter2 != ListOfPacketMatrix[(*iter).AsSource][(*iter).AsDest].end())
    			{ 
				
				(*iter2)->GetTimeStamp(tmpTs);
      				if(ts.Compare(tmpTs) == 0 )
        			{
						// Setting the shif time 
						SetPacketShiftTime(*iter2);
						//Adding the packet
						pListOfPacketToSend.push_back(*iter2);
						numberOfPacktes++;
						// We should remove each packet we already sent
						iter2 = ListOfPacketMatrix[(*iter).AsSource][(*iter).AsDest].erase(iter2);
						continue;				
				} else if(ts.Compare(tmpTs) > 0 )
				{
					// The packets are stored in the lists in increasing order of their timestamps
					break;
				}

				iter2++;
    			}
		}
  	}
	return numberOfPacktes;
}

catch(exception &e)
{
	cout<<e.what()<<endl;
}
}


// Return the number of packets lost 
unsigned int SamplingPlatform::GetLostPackets(TimeStamp &ts)
{
try
{
	unsigned int numberOfPacktes = 0;
  	if(lAsCouples.empty())
   		throw NspException("Error while reading the list of AS(s).",__FILE__,__LINE__);
	list<couple>::iterator iter;
  	TimeStamp tmpTs(0, 0);
  	for(iter=lAsCouples.begin();iter!=lAsCouples.end();iter++)
  	{
		if(!ListOfPacketMatrix[(*iter).AsSource][(*iter).AsDest].empty())
	  	{
    			ListOfPackets::iterator iter2 = ListOfPacketMatrix[(*iter).AsSource][(*iter).AsDest].begin();
    			while(iter2!= ListOfPacketMatrix[(*iter).AsSource][(*iter).AsDest].end())
    			{ 
				
				(*iter2)->GetTimeStamp(tmpTs);
      				if(ts.Compare(tmpTs) <= 0 )
        			{
					numberOfPacktes++;
					
				} else if(ts.Compare(tmpTs) > 0 )
				{
					// The packets are stored in the lists in increasing order of their timestamps
					break;
				}

				iter2++;
    			}
		}
  	}
	return numberOfPacktes;
}

catch(exception &e)
{
	cout<<e.what()<<endl;
}
}


// Used to send a given packet
void  SamplingPlatform::SendPacket(packet *pPacket)
{
try
{
	//cout<< "Ip Source : "<<pPacket->szIpSource<<" Ip Dest: "<<pPacket->szIpDest<<" As source number: "<<pPacket->iAsSourceNumber<<" Dest As Number: "<<pPacket->iAsDestNumber<<endl;
  	if(pPacket==NULL)
      		throw NspException("Trying to send an invalid packet.",__FILE__,__LINE__);
  	GetAs(pPacket->iAsSourceNumber)->SendPacket(pPacket);

}

catch(exception &e)
{
  	cout<<e.what()<<endl;
}
}

// Used to send the current list of packets present in pListOfPacketToSend
void SamplingPlatform::SendTheCurrentListOfPackets()
{
try
{
	ListOfPackets::iterator iter;
  	while(!pListOfPacketToSend.empty())
  	{
		iter = pListOfPacketToSend.begin();
    		SendPacket(*iter);
		//Remove the packet
		free(*iter);
		pListOfPacketToSend.erase(iter);
		
		numberOfPackets --;
		#ifdef SHOW_THE_REMAINING_NUMBER_OF_PACKETS
		cout <<" Number of remaining packets: "<<numberOfPackets<<endl;
		#endif
  	}
}

catch(exception &e)
{
	cout<<e.what()<<endl;
}
}

// Main Emulator method, it starts sending the packets from the different source AS(s) to the different dest AS(s)
void SamplingPlatform::StartSendingPackets()
{

try
{
	list<TimeStamp>::iterator  iterAxis;

	emulationStartTime.SetToTheSystemTime();

	TimeStamp lastTs(0, 0);
	bool stopSending = false;
	unsigned int uaxe = 0;
	int iteration = 0;
	unsigned int numberOfPackets = 0;
	
	iterAxis = timeAxis.begin();
	TimeStamp tmpTrace;
	TimeStamp tmpSystem;
	TimeStamp tmpInterval;
	list<TimeStamp>::iterator timeIter;
	waitPeriod = 10;
	
	while(!stopSending)
	{	
		if(!DataNeeded())
		{	
			GetSendingSem();
			if((uaxe < timeAxisSize) || (iteration > MAX_ITERATIONS && uaxe > 0 ))
			{
			
				if(GetLoading())
				{
					TimeStamp::GetTimeToWaitBetween(startTime, endTime, loadingTime);
					SetLoading(false);
				}
		
				while(uaxe < timeAxisSize)
				{	
					timeIter = timeAxis.begin();
					if(uaxe == 0)
					{
						currentTs.Copy(*timeIter);
						lastTs.Copy(*timeAxis.begin());
						// Set the trace start time
						traceStartTime.Copy(*timeIter);
						traceCurrentTime.Copy(*timeIter);
						// Setthe emulation start Time
						emulationStartTime.SetToTheSystemTime();
						emulationCurrentTime.SetToTheSystemTime();
					}
					else
					{
						lastTs.Copy(currentTs);
						currentTs.Copy(*timeIter);
						// Set the trace current time
						traceCurrentTime.Copy(currentTs);
						emulationCurrentTime.SetToTheSystemTime();
					}
					
					TimeStamp::GetTimeToWaitBetween(traceStartTime, traceCurrentTime, tmpTrace);
					TimeStamp::GetTimeToWaitBetween(emulationStartTime, emulationCurrentTime, tmpSystem);
					TimeStamp::GetTimeToWaitBetween(lastTs, currentTs, tmpInterval);
					
					
					//else if(tmpSystem.GetSecond() > tmpTrace.GetSecond() - 2)
					//cout <<"lastTs: "<<lastTs.GetTime()<<" currentTs: "<<currentTs.GetTime()<<" loadingTime: "<<loadingTime.GetTime()<<" uaxe: "<<uaxe<<" timeAxisSize: "<<timeAxisSize<<" Elapsed Trace Time: "<<tmpTrace.GetTime()<<" Elasped Emulating Time: "<<tmpSystem.GetTime()<<endl;
		
					//printf("Elapsed Trace Time: %s Elapsed Emulating Time: %s\n",  tmpTrace.GetTime().c_str(), tmpSystem.GetTime().c_str());
					numberOfPackets = UpdateTheListOfPacketsToSend(currentTs);
					//cout << "At "<<currentTs.GetTime()<<" "<<numberOfPackets<<" packets will be sent."<<endl;
					
					if(tmpSystem.Compare(tmpTrace) > 0 && uaxe % waitPeriod == 0)
					{
						TimeStamp::WaitBetween(lastTs, currentTs);
						// updating the wait period
						if(waitPeriod > 1 && tmpInterval.GetuSecond() > 1000)
							waitPeriod--; 
					}
					
					SendTheCurrentListOfPackets();
					
					//unsigned int lost = GetLostPackets(currentTs);

					uaxe++;
					
					timeAxis.erase(timeIter);
					
					if(!Tl->GetLoadingThreadStatus())
						stopSending = true;
	
				}	
				lastTimeAxisSize = uaxe;

				iteration = 0;
			}

			iteration++;
			loadingTime.Init();
			ReleaseSendingSem();

		} else usleep(1); 
	}
		
		cout <<"Traffic loading status: "<<Tl->GetLoadingThreadStatus()<<" sending status: "<<stopSending<<"  Shuuting down the sampling platform."<<endl;
		
		// Stopping the Sampling manager
		cout<<"Stopping the SamplingControlServer."<<endl;
		sm->SetStopsControlServer();
		
		// Stopping the softflowd interface thread
		if(ptrSoftflowdInterface!=NULL)
		{
			ptrSoftflowdInterface->SetInterfaceStatus(false);
			WaitUntilPacketsForwrdingToSoftflowd();
			delete ptrSoftflowdInterface;
			ptrSoftflowdInterface = NULL;
		}
		
			
}

catch(exception &e)
{
	cout<<e.what()<<endl;
}
}

void SamplingPlatform::SetPacketShiftTime(packet *p)
{
try
{
	if(p == NULL)
		throw NspException("Invalid packet.",__FILE__,__LINE__);
	p->SetTimeStamp(loadingTime);
}
catch(exception &e)
{
	cout << e.what()<<endl;
}
}

void SamplingPlatform::WaitUntilPacketsForwrdingToSoftflowd()
{
	while(!ptrSoftflowdInterface->GetMainThreadStatus())
		sleep(1);
}

bool SamplingPlatform::ValidateCouple(int a, int b)
{
try
{
	if(!lAsCouples.empty())
	{
		list<couple>::iterator iter;
		for(iter = lAsCouples.begin(); iter != lAsCouples.end(); iter++)
		{
			if(iter->AsSource == a && iter->AsDest == b)
				return true;

			// We tolerate also the packets coming and going to the same AS
			if((a == b) && (a == iter->AsDest || a == iter->AsDest))
				return true;
		}
		return false;
	} else throw NspException("Invalid AS couples.", __FILE__, __LINE__);
}

catch(exception &e)
{

}
}

// pcap based Api 

/*
 * Per-packet callback function from libpcap. Pass the packet (if it is IP)
 * sans datalink headers to process_packet.
 */

void SamplingPlatform::PerPacketCallBackFunction(u_char *user_data, const struct pcap_pkthdr* phdr, const u_char *pkt)
{
try
{
	fstream log;
	int s, af;
	struct CB_CTXT *cb_ctxt = (struct CB_CTXT *)user_data;
	struct timeval tv;
	
	s = TrafficLoader::DatalinkCheck(cb_ctxt->linktype, pkt, phdr->caplen, &af);
	if (s < 0 || (!cb_ctxt->want_v6 && af == AF_INET6)) 
	{
		#ifdef SHOW_IPV6_PACKETS 
		throw NspException("Ignoring an Ipv6 packet.",__FILE__,__LINE__);
		#endif

	} else {
		
		packet *currentPacket = (packet*) malloc(sizeof(packet));
		currentPacket->Init();
		tv.tv_sec = phdr->ts.tv_sec;
		tv.tv_usec = phdr->ts.tv_usec;
		currentPacket->usecond = phdr->ts.tv_usec;
		currentPacket->second = phdr->ts.tv_sec;
		currentPacket->iResponsibleNode = -1;	
		
		
		
		if (TrafficLoader::ProcessPacket(pkt + s, af, phdr->caplen - s, phdr->len - s, &tv, currentPacket) == PP_MALLOC_FAIL)
		{
			cb_ctxt->fatal = 1;
			free(currentPacket);
			throw NspException("Error ocuured while processing the packet received from pcap.",__FILE__, __LINE__);
		}
		else 
		{	
			if(!currentPacket->AmIValid())
			{
				free(currentPacket);
			}
			else
			{
				cb_ctxt->tl->AddPacket();
				string sourceIp(currentPacket->szIpSource);
				string destinationIp(currentPacket->szIpDest);
				TimeStamp tmpTs(0, 0);
				currentPacket->GetTimeStamp(tmpTs);	
					
				int ipMask = cb_ctxt->sp->GetIpMask();
				
				
	
				switch(cb_ctxt->sp->GetDispatchingMethod())
				{
				case 1:
					{
						// Dispatching based on the Ip mask
						// Applying the Ip mask and calculating the prefix
						int sourceDispatched = 0;
						int destinationDispatched = 0;
											
						string sourceIpPref(AmDev::As::ApplyMask(sourceIp, ipMask));
						string destIpPref(AmDev::As::ApplyMask(destinationIp, ipMask));
						ListOfAs::iterator iter;
						As * sourceAs;
						As *destAs; 
						for(iter = cb_ctxt->sp->ptrListOfAs.begin(); iter != 	cb_ctxt->sp->ptrListOfAs.end(); iter++)
						{
							if((*iter)->FindPrefix(sourceIpPref, 0) == 1 && sourceDispatched == 0)
							{
								currentPacket->iAsSourceNumber = (*iter)->GetNumber();
								sourceDispatched = 1;
								sourceAs = *iter;
							}
	
							if((*iter)->FindPrefix(destIpPref, 1) == 1 && destinationDispatched == 0)
							{
								currentPacket->iAsDestNumber = (*iter)->GetNumber();
								destAs = *iter;
								destinationDispatched = 1;
							}
							
							if(sourceDispatched == 1 && destinationDispatched == 1)
								break;	
						}
	
						if(!sourceDispatched)
						{
							// add the source ip prefix
							int selectedAsNumber = cb_ctxt->sp->SelectAsRandomlyBasedOnWeights();
							As* selectedAs = cb_ctxt->sp->GetAs(selectedAsNumber);
							
							if(selectedAs == NULL)
							{
								cout <<"Selected AS number: "<<selectedAsNumber<<endl;
								throw NspException("The selected AS is invalid.",__FILE__, 	__LINE__);
							}		
							selectedAs->AddPrefix(sourceIpPref, 0);
							currentPacket->iAsSourceNumber = selectedAsNumber;
							sourceDispatched = 1;
							if(sourceIpPref.compare(destIpPref) == 0)
							{
								// the source and dest will belong to the same AS
								currentPacket->iAsDestNumber = selectedAsNumber;
								destinationDispatched = 1;
								destAs = selectedAs;
							}
							sourceAs = selectedAs;
							selectedAs = NULL;
						}
	
						if(!destinationDispatched)
						{
							// add the destination ip prefix
							int selectedAsNumber = cb_ctxt->sp->SelectAsRandomlyBasedOnWeights();
							As* selectedAs = cb_ctxt->sp->GetAs(selectedAsNumber);
							
							if(selectedAs == NULL)
							{	
								cout << "Selected AS number: "<<selectedAsNumber<<endl;
								throw NspException("Invalid AS.",__FILE__, __LINE__);
							}
							selectedAs->AddPrefix(destIpPref, 1);
							destAs = selectedAs;
							selectedAs = NULL;
							currentPacket->iAsDestNumber = selectedAsNumber;
							destinationDispatched = 1;
						}
		
						if(sourceDispatched == 0 || destinationDispatched == 0)
						{
							throw NspException("Error occured while trying to dispatch 	packet.",__FILE__, __LINE__);
						}
	
						if(sourceDispatched && destinationDispatched && 	cb_ctxt->sp->ValidateCouple(currentPacket->iAsSourceNumber, currentPacket->iAsDestNumber))
						{
			
			
							// packet dispatched, so add it to the list, so it will be sent	
							// We will ignore packets with the same AS source and destination
							cb_ctxt->sp->numberOfPackets++;
	
							// Verify the packet's time stamp
							if(cb_ctxt->sp->maxTs.Compare(tmpTs) > 0)
	  						{
								cb_ctxt->sp->maxTs.Copy(tmpTs);
							}
						
							// Adding Ips to ASs
							#ifdef MAINTAIN_IPS_IN_AS
							sourceAs->AddIp(sourceIp);
							if(sourceAs != destAs)
								destAs->AddIp(destinationIp);
							#endif

							cb_ctxt->sp->IncrementTimeAxis(tmpTs);
							cb_ctxt->sp->PushPacket(currentPacket, currentPacket->iAsSourceNumber, currentPacket->iAsDestNumber);
							//currentPacket->LogInfos(log);
							#ifdef SHOW_PACKETS_DETAILS_WHEN_DISPATCHING
							currentPacket->ShowDetails();
							#endif
							currentPacket = NULL;
							break;
						}
	
						// Free the allocated memory
						// it could be an invalid packet: either not supported protocol or invalid packet details
						free(currentPacket);
					
					}
				break;
				case 2:
					{
					
					// Dispatching based on the Ip's Class				
					list<couple>::iterator iterAs = cb_ctxt->sp->lAsCouples.begin();
					while(iterAs != cb_ctxt->sp->lAsCouples.end())
					{
						As * AsSource = cb_ctxt->sp->GetAs(iterAs->AsSource);
						As * AsDest = cb_ctxt->sp->GetAs(iterAs->AsDest);
						if(AsSource == NULL || AsDest == NULL)
							throw NspException("Invalid As number given in the xml 	configuration file.",__FILE__, __LINE__);	
			
						if(AsSource->VerifyIpAdr(sourceIp,1) && 	AsDest->VerifyIpAdr(destinationIp,1))
						{
							cb_ctxt->sp->numberOfPackets++;
							// Add the Ips to ASs
							AsSource->AddIp(sourceIp);
							AsDest->AddIp(destinationIp);
						
							// Adding the packets to the list
							long int lPacketSize = currentPacket->lLength;
										
									
							// Verify the packet's time stamp
							if(cb_ctxt->sp->maxTs.Compare(tmpTs) > 0)
  							{
								cb_ctxt->sp->maxTs.Copy(tmpTs);
							}
				
							currentPacket->iAsSourceNumber = iterAs->AsSource;
       						currentPacket->iAsDestNumber = iterAs->AsDest;
       						if(currentPacket->iAsSourceNumber < 0 || 	currentPacket->iAsDestNumber < 0)
							throw NspException("Invalid ASs number.",__FILE__,__LINE__);
							cb_ctxt->sp->IncrementTimeAxis(tmpTs);
							cb_ctxt->sp->PushPacket(currentPacket, iterAs->AsSource, iterAs->AsDest);
						
							#ifdef SHOW_PACKETS_DETAILS_WHEN_DISPATCHING
							currentPacket->ShowDetails();
							#endif
							currentPacket = NULL;
						}
						iterAs++;
					}
				
					}
				break;
				case 3:
					{
	
		
						// Dispatching based on the Ip mask
						// Applying the Ip mask and calculating the prefix
						int sourceDispatched = 0;
						int destinationDispatched = 0;
											
						
						string protocol(currentPacket->szProtocol);
			
						ListOfAs::iterator iter;
						As * sourceAs;
						As *destAs; 
						
						for(iter = cb_ctxt->sp->ptrListOfAs.begin(); iter != 	cb_ctxt->sp->ptrListOfAs.end(); iter++)
						{
							if((*iter)->FindFlow(sourceIp, destinationIp, protocol, atoi(currentPacket->szSourcePort), atoi(currentPacket->szDestPort)) == 1 && sourceDispatched == 0)
							{
								currentPacket->iAsSourceNumber = (*iter)->GetNumber();
								sourceDispatched = 1;
								sourceAs = *iter;
							}
	
							if((*iter)->FindFlow(destinationIp,sourceIp, protocol,  atoi(currentPacket->szDestPort), atoi(currentPacket->szSourcePort)) == 1 && destinationDispatched == 0)
							{
								currentPacket->iAsDestNumber = (*iter)->GetNumber();
								destAs = *iter;
								destinationDispatched = 1;
							}
							
							if(sourceDispatched == 1 && destinationDispatched == 1)
								break;	
						}
						
						if(!sourceDispatched)
						{
							// add the source ip prefix
							int selectedAsNumber = cb_ctxt->sp->SelectAsRandomlyBasedOnWeights();
							As* selectedAs = cb_ctxt->sp->GetAs(selectedAsNumber);
							if(selectedAs == NULL)
							{
								cout <<"Selected AS number: "<<selectedAsNumber<<endl;
								throw NspException("The selected AS is invalid.",__FILE__, 	__LINE__);
							}
			
							selectedAs->AddFlow(sourceIp, destinationIp, protocol, atoi(currentPacket->szSourcePort), atoi(currentPacket->szDestPort));
							currentPacket->iAsSourceNumber = selectedAsNumber;
							sourceDispatched = 1;

							sourceAs = selectedAs;
							selectedAs = NULL;
						}
	
						if(!destinationDispatched)
						{
							// add the destination ip prefix
							int selectedAsNumber = cb_ctxt->sp->SelectAsRandomlyBasedOnWeights();
							As* selectedAs = cb_ctxt->sp->GetAs(selectedAsNumber);
							if(selectedAs == NULL)
							{	
								cout << "Selected AS number: "<<selectedAsNumber<<endl;
								throw NspException("Invalid AS.",__FILE__, __LINE__);
							}
							selectedAs->AddFlow(destinationIp,sourceIp, protocol,  atoi(currentPacket->szDestPort), atoi(currentPacket->szSourcePort));
							destAs = selectedAs;
							selectedAs = NULL;
							currentPacket->iAsDestNumber = selectedAsNumber;
							destinationDispatched = 1;
						}
		
						if(sourceDispatched == 0 || destinationDispatched == 0)
						{
							throw NspException("Error occured while trying to dispatch 	packet.",__FILE__, __LINE__);
						}
	
						if(sourceDispatched && destinationDispatched && 	cb_ctxt->sp->ValidateCouple(currentPacket->iAsSourceNumber, currentPacket->iAsDestNumber))
						{
			
			
							// packet dispatched, so add it to the list, so it will be sent	
							// We will ignore packets with the same AS source and destination
							cb_ctxt->sp->numberOfPackets++;
	
							// Verify the packet's time stamp
							if(cb_ctxt->sp->maxTs.Compare(tmpTs) > 0)
	  						{
								cb_ctxt->sp->maxTs.Copy(tmpTs);
							}
						
							cb_ctxt->sp->IncrementTimeAxis(tmpTs);
							cb_ctxt->sp->PushPacket(currentPacket, currentPacket->iAsSourceNumber, currentPacket->iAsDestNumber);
							//currentPacket->LogInfos(log);
							#ifdef SHOW_PACKETS_DETAILS_WHEN_DISPATCHING
							currentPacket->ShowDetails();
							#endif
							currentPacket = NULL;
							break;
						}
	
						// Free the allocated memory
						// it could be an invalid packet: either not supported protocol or invalid packet details
						free(currentPacket);
					
					}
				break;
				default:
					free(currentPacket);
					break;
					
				};
			}
		} 
		
	}
}

catch(exception &e)
{
	cout << e.what() <<endl;
}

}

void SamplingPlatform::PushPacket(packet *pkt, int AsSource, int AsDest)
{
try
{
	if(pkt == NULL)
	{
		throw NspException("Invalid packet.",__FILE__,__LINE__);
	}else
	{
		
		ListOfPacketMatrix[AsSource][AsDest].push_back(pkt);
	}
}
catch(exception &e)
{
	cout << e.what()<<endl;
}
}


void SamplingPlatform::LoadBinaryTrafficFile(string &btf)
{
try
{
	cout<<"Reading and dispatching packets from the TcpDump binary file: "<<btf<<endl;
	Tl->StartTheInterfaceThreadF(btf, this);
}
catch(exception &e)
{
	cout << e.what() <<endl;
}
}

// Loads a list of binary files described in an xml file
void SamplingPlatform::LoadListOfBinaryTrafficFiles(string &btf)
{
try
{
	list<string> filesList;

	if(btf.length()==0)
		throw NspException("Invalid binary files description file.",__FILE__,__LINE__);
	if(Tl->IsReadable(btf))
	{
    		TiXmlDocument doc((char *) btf.c_str());
		bool bLoad=doc.LoadFile();
		if(bLoad)
		{
			// Extract the attributes of the transit Network
			TiXmlElement* pRoot=doc.FirstChildElement("BinaryFiles");
			if(pRoot==NULL)
				throw NspException("Error while parsing the xml file containing the description of binary files.",__FILE__,__LINE__);
			
    
			// Extracting the links
			TiXmlElement* pElemFile = pRoot->FirstChildElement("File");
			if(pElemFile == NULL)
				throw NspException("Error while parsing the xml file.",__FILE__,__LINE__);
			
			for(pElemFile;pElemFile;pElemFile = pElemFile->NextSiblingElement("File"))
			{
				// Extract the list of absolute paths
				filesList.push_back(pElemFile->Attribute("AbsolutePath"));
			}
	
			if(!filesList.empty())
				Tl->StartTheInterfaceThread(filesList, this);
			else throw NspException("The resulting list of binary files is empty.",__FILE__,__LINE__);

		} else   throw NspException("Error while parsing the xml file.",__FILE__,__LINE__);
	}else   throw NspException("Error while parsing the xml file.",__FILE__,__LINE__);

}

catch(exception &e)
{
	cout<<e.what()<<endl;
}
}

int SamplingPlatform::ChangeSamplingRate(string ipAdr, int p1, int p2)
{
try
{
	int ret = -1;
	ListOfTransitNetworks::iterator iter = ptrListOfTransitNetwork.begin();
	while(iter != ptrListOfTransitNetwork.end())
	{
		if((ret = (*iter)->ChangeSamplingRate(ipAdr, p1, p2)) != -1)
			return ret;
		iter++;
	}
	return ret;
}
catch(exception &e)
{
	cout << e.what()<<endl;
}
}
}



