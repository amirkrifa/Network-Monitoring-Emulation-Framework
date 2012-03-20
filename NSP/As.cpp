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

#include "As.h"
#include <fstream>
#include <iostream>
#include <string>
#include "TransitNetwork.h"
#include "CommonStructs.h"

namespace AmDev {

As::As(string & IpsListFile, int iNumber)
{
try 
{
	InitPrefixTabs();
	if(IpsListFile.length()==0)
		throw NspException("IpsListFile == null",__FILE__,__LINE__);
	ifstream IpsFile;
	IpsFile.open((char *)IpsListFile.c_str(),fstream::out);
	char szIp[16];
	iNumberOfIps=0;
	while(IpsFile.getline(szIp,100))
	{
		ListOfIps.push_back(string(szIp));
		iNumberOfIps++;
	}
	IpsFile.close();
	string sCommand("rm -f ");
	sCommand.append(IpsListFile);
	if(system(sCommand.c_str()))
		throw NspException("Error while executing system()",__FILE__,__LINE__);
	iAsNumber=iNumber;
	char id[10];
	sprintf(id, "%i", iAsNumber);
	string name;
	name.append("AS-");
	name.append(id);
	name.append(".txt");
	asLog.open(name.c_str(),ios::out|ios::app);
}
catch (exception &e)
{
	cout<<e.what()<<endl;
}
}

As::As(int iAsNumber, int iMaxIps, int iAsImportance,int minInterval, int maxInterval, char * ipClass, char * ipRangeStart, char * ipRangeEnd,int ipMask, int dispatchingMethod)
{
try 
{
	InitPrefixTabs();
	this->iAsMaxIps = iMaxIps;
	this->iAsNumber = iAsNumber;
	this->iAsImportance = iAsImportance;
	this->minInterval = minInterval;
	this->maxInterval = maxInterval;

	switch(dispatchingMethod)
	{
	case 1:
		// Dispatching based on the Ip mask	
		if(ipMask != 1 && ipMask != 2 && ipMask != 3 && ipMask != 4)
			throw NspException("Invalid ip mask.", __FILE__, __LINE__);
		else this->ipMask = ipMask;
		break;
	case 2:
		// Dispatching based on the Ip's Class
		if(strcmp(ipClass, "A") != 0 && strcmp(ipClass, "B") != 0 && strcmp(ipClass, "C") != 0 && strcmp(ipClass, "D") != 0)
			throw NspException("Invalid Ip class.", __FILE__, __LINE__);
		this->ipClass.assign(ipClass);
		this->dispatchingMethod = dispatchingMethod;
		break;
	case 3:
		// Dispatching based on the Ip Range
		if(ipClass == NULL || ipRangeStart == NULL || ipRangeEnd == NULL)
			throw NspException("Invalid AS Atrributes.", __FILE__, __LINE__);
		this->ipRangeStart.assign(ipRangeStart);
		this->ipRangeEnd.assign(ipRangeEnd);
		this->dispatchingMethod = dispatchingMethod;
		break;
	default:
		throw NspException("Invalid dispatching method.", __FILE__, __LINE__);
		break;
	};
        char id[10];
        sprintf(id, "%i", iAsNumber);
        string name;
        name.append("AS-");
        name.append(id);
        name.append(".txt");
        asLog.open(name.c_str(),ios::out|ios::app);
}
catch (exception &e)
{
	cout<<e.what()<<endl;
}
}

void As::InitPrefixTabs()
{
	numberOfPrefix = 0;
	
	for(int i = 0; i < 255;i++)
	{
		for(int j = 0; j < 255; j++)
		{
			tabPref1[i][j] = 0;
			tabPref2[i][j] = 0;
		}
	}

}

int64_t As::GetPrefix(string& pref, int length)
{
try
{	
	char tmp1[length * 3];
	memset(tmp1, '\0', length * 3);
	char tmp2[3];
	int64_t ret;

	switch(length)
	{
	case 1:
		sprintf(tmp2, "%i", GetIpPartNumber(pref, 0));
		strcpy(tmp1, tmp2);
		ret = (tmp1 == NULL?0:atoll(tmp1));
		return ret;
	case 2:
		sprintf(tmp2, "%i", GetIpPartNumber(pref, 0));
		strcpy(tmp1, tmp2);
		sprintf(tmp2, "%i", GetIpPartNumber(pref, 1));
		strcat(tmp1, tmp2);
		ret = (tmp1 == NULL?0:atoll(tmp1));
		return ret;		
	case 3:
		sprintf(tmp2, "%i", GetIpPartNumber(pref, 0));
		strcpy(tmp1, tmp2);
		sprintf(tmp2, "%i", GetIpPartNumber(pref, 1));
		strcat(tmp1, tmp2);
		sprintf(tmp2, "%i", GetIpPartNumber(pref, 2));
		strcat(tmp1, tmp2);
		ret = (tmp1 == NULL?0:atoll(tmp1));
		return ret;		
	case 4:
		sprintf(tmp2, "%i", GetIpPartNumber(pref, 0));
		strcpy(tmp1, tmp2);
		sprintf(tmp2, "%i", GetIpPartNumber(pref, 1));
		strcat(tmp1, tmp2);
		sprintf(tmp2, "%i", GetIpPartNumber(pref, 2));
		strcat(tmp1, tmp2);
		sprintf(tmp2, "%i", GetIpPartNumber(pref, 3));
		strcat(tmp1, tmp2);
		ret = (tmp1 == NULL?0:atoll(tmp1));
		return ret;		
	default:
		return -1;
	};
}

catch(exception &e)
{
	cout<<e.what()<<endl;
}
}

bool As::GenerateIpsListFile()
{
try
{
	ofstream of;
	char Number[10];
	sprintf(Number,"%i",iAsNumber);
	string sFileName(string("AS")+string(Number)+string(".txt"));
	of.open((char *)sFileName.c_str(),fstream::in|fstream::trunc);
	IpsList::iterator iter=ListOfIps.begin();
	while (iter!=ListOfIps.end())
	{ 
		of<<*iter<<endl;
		iter++;
	}
	of.close();
	return true;
}
catch(exception &e)
{
	cout<<e.what()<<endl;
}
}

string As::GetListOfIpsFilePath() const
{
 try
 {
	char Number[10];
	sprintf(Number,"%i",iAsNumber);
	string sFileName(string("AS")+string(Number)+string(".txt"));
	if(sFileName.length()==0)
		throw NspException("Error while looking for the path of the file containig the list of Ips.",__FILE__,__LINE__);
	return sFileName;
 }
 catch(exception &e)
 {
	 cout<<e.what()<<endl;
 }
}


As::~As()
{
	//asLog <<"numberOfPrefix: "<< numberOfPrefix<<endl;
	// Write each prefix and its number of generated packets
	for(map<string , unsigned int>::iterator iter =  prefMap.begin(); iter != prefMap.end();iter++)
	{
		if(iter->second != 0)
			asLog << (iter->first).c_str()<<" "<<iter->second<<endl; 
	}
	asLog.close();
	if(ListOfIps.size())
		ListOfIps.clear();
	if(ptrListTn.size())
		ptrListTn.clear();
	// Deleting the created file 
	try
	{
	if(OutboundTrafficFile.length())
	{
	string sCommand("rm -f ");
	sCommand.append(OutboundTrafficFile);
	if(system(sCommand.c_str()))
			throw NspException("Error encountred while executing System().",__FILE__,__LINE__);
	}
	
	
	}
	catch(exception &e)
	{
		cout<<e.what()<<endl;
	}

}

void As::ConnectToTransitNetwork(TransitNetwork * const ptrTransitNetwork)
{
try
{
	list<TransitNetwork *>::iterator TnIter=ptrListTn.begin();
	for(TnIter=ptrListTn.begin();TnIter!=ptrListTn.end();TnIter++)
	{
		if(*TnIter==ptrTransitNetwork)
			break;
	}
	if(TnIter==ptrListTn.end())
	ptrListTn.push_back(ptrTransitNetwork);
}
catch(exception &e)
{
	cout<<e.what()<<endl;
}
}

void As::ShowDetails()
{
	cout<<"  As Number: "<<iAsNumber<<endl;
	cout<<"  Connected to "<<ptrListTn.size()<<" Transit Networks"<<endl;
	list<TransitNetwork *>::iterator TnIter;
	int i=1;
	for(TnIter=ptrListTn.begin();TnIter!=ptrListTn.end();TnIter++)
	{
		cout<<"      Transit Network Number: "<<i<<endl;
		(*TnIter)->ShowDetails();
		i++;
	}
}
// Sends a given packet
void As::SendPacket(packet *pPacket)
{
try
{
	if(pPacket == NULL)
		throw NspException("The pointer pPacket to the packet to send is = null.",__FILE__,__LINE__);
  
	for(list<TransitNetwork *>::iterator iter = ptrListTn.begin();iter != ptrListTn.end();iter++)
	{
		if((*iter)->CouldYouForwrdTo(pPacket->iAsDestNumber))
		{
			(*iter)->ForwardPacketE(pPacket);
				break;
		}
	}
}
catch(exception &e)
{
	 cout<<e.what()<<endl;
}
}

// Returns the class of a given Ip Adr
string As::GetClass( string & Ip)
{
	int p1 = -1, p2 = -1, p3 = -1, p4 = -1;
	string remaining;
	int pos;
try
{
	

	pos = Ip.find_first_of('.');	
	p1 = atoi(Ip.substr(0, pos).c_str());
	remaining = Ip.substr(pos+1);	

	pos = remaining.find_first_of('.');
	p2 = atoi(remaining.substr(0, pos).c_str());
	remaining = remaining.substr(pos+1);

	pos = remaining.find_first_of('.');
	p3 = atoi(remaining.substr(0, pos).c_str());
	remaining = remaining.substr(pos+1);
	
	pos = remaining.find_first_of('.');
	p4 = atoi(remaining.substr(0, pos).c_str());
	remaining = remaining.substr(pos+1);
	if(p1 <= 0 || p2 < 0 || p3 < 0 || p4 < 0)
		throw NspException("Error occured while parsing Ip Adr.",__FILE__, __LINE__);
	
	if( p1 >=1 && p1 <= 127)
		return string("A");
	else if( p1 >= 128 && p1 <= 191)
		return string("B");		
	else if( p1 >= 192 && p1 <= 223)
		return string("C");		
	else if( p1 >= 224 && p1 <= 239)
		return string("D");		
	else if( p1 >= 240 && p1 <= 255)
		return string("E");		
	else return string("");
}
catch(exception &e)
{
	cout << e.what() << endl;
	cout << "IP: "<<Ip<<endl;
	cout<<"p1: "<<p1<<" p2: "<<p2<<" p3: "<<p3<<" p4: "<<p4<<endl;
	return string("");
}

}

// Returns an Ip part given its number
int As::GetIpPartNumber(string &Ip, int p)
{
try
{
	int part = -1;
	string remaining;
	int pos;

	
	pos = Ip.find_first_of('.');	
	part = atoi(Ip.substr(0, pos).c_str());
	remaining = Ip.substr(pos+1);	

	if(p == 0 && part >0 )
		return part;
	
	for(int i=0;i < p;i++)
	{
		pos = remaining.find_first_of('.');
		part = atoi(remaining.substr(0, pos).c_str());
		remaining = remaining.substr(pos+1);
	}
	if(part >= 0)
		return part;
	else throw NspException("Error occured while parsing Ip Adr.",__FILE__, __LINE__);
}

catch(exception &e)
{
	cout <<"Ip: "<<Ip<<" ,part: "<<p<<endl;
	cout << e.what() << endl;
}
}

// Apply a given mask to an Ip address and returns the result
string As::ApplyMask(string &ip, int mask)
{
try
{

	if(mask < 1 || mask > 4)
		throw NspException("Invalid mask.",__FILE__, __LINE__);
	char szTmp[16];
	string sTmp;
	switch(mask)
	{
	case 1:
		sprintf(szTmp, "%i", GetIpPartNumber(ip, 0));
		sTmp.append(szTmp);
		return sTmp;
	case 2:
		sprintf(szTmp, "%i.%i", GetIpPartNumber(ip, 0), GetIpPartNumber(ip, 1));
		sTmp.append(szTmp);
		return sTmp;
	case 3:
		sprintf(szTmp, "%i.%i.%i", GetIpPartNumber(ip, 0), GetIpPartNumber(ip, 1), GetIpPartNumber(ip, 2));
		sTmp.append(szTmp);
		return sTmp;
	case 4: 
		sprintf(szTmp, "%i.%i.%i.%i", GetIpPartNumber(ip, 0), GetIpPartNumber(ip, 1), GetIpPartNumber(ip, 2), GetIpPartNumber(ip, 3));
		sTmp.append(szTmp);
		return sTmp;
	default:
		throw NspException("Invalid mask.",__FILE__, __LINE__);
	};
}
catch(exception &e)
{
	cout << e.what() << endl;
}
}

// Verify if a given ip adr is in the AS range or not
bool As::IsItInTheRange(string &ipAdr)
{
	int ipRangeStart[4], ipRangeEnd[4], ip[4];

	for(int i=0; i < 4; i++)
	{
		ip[i] = As::GetIpPartNumber(ipAdr, i);
		ipRangeStart[i] = As::GetIpPartNumber(this->ipRangeStart, i);
		ipRangeEnd[i] = As::GetIpPartNumber(this->ipRangeEnd, i);
		
	}
	
	if( ip[0] > ipRangeStart[0] && ip[0] < ipRangeEnd[0])
		return true;
	else if(ip[0] == ipRangeStart[0])
	{
		if(ip[1] > ipRangeStart[1])
			return true;
		else if(ip[1] == ipRangeStart[1])
		{
			if(ip[2] > ipRangeStart[2])
				return true;
			else if(ip[2] == ipRangeStart[2])
			{
				if(ip[3] >= ipRangeStart[3])
					return true;
				else if(ip[3] < ipRangeStart[3])
					return false;
			} else return false;	
		} else return false;
	} 
	else if(ip[0] == ipRangeEnd[0])
	{
		if(ip[1] < ipRangeEnd[1])
			return true;
		else if(ip[1] == ipRangeEnd[1])
		{
			if(ip[2] < ipRangeEnd[2])
				return true;
			else if(ip[2] == ipRangeEnd[2])
			{
				if(ip[3] < ipRangeEnd[3])
					return true;
				else if(ip[3] <= ipRangeEnd[3])
					return true;
				else if(ip[3] > ipRangeEnd[3])
					return false;
			} else return false;	
		} else return false;
	
	}else return false;
}

bool As::VerifyIpAdr(string &ip, int method)
{

if(method == 1)
{
	if(GetClass(ip).compare(ipClass) == 0)
		return true;
	else return false;

} else if(method == 2 )
	{
		return IsItInTheRange(ip);			
	}
	
}

// Add an ip adr to the list
void As::AddIp(string &szIp)
{
    ListOfIps.push_back(string(szIp));
    iNumberOfIps++;
}


void As::AddPrefix(string &p, int srcdest)
{
try
{	
	string pref(p);
	if(srcdest == 0)
		prefMap[pref] = 1;
	else prefMap[pref] = 0;
	numberOfPrefix++;
	
}
catch (exception &e)
{
	cout << e.what()<<endl;
}
}

int As::FindPrefix(string &pref,int srcdest)
{

try
{	string prefx(pref);
	map<string, unsigned int>::iterator iter = prefMap.find(prefx);
	if(iter != prefMap.end())
	{
		if(srcdest == 0)
			iter->second++;

		return 1;
	}
	else
	{
	 	return 0;
	}	
	
}
catch (exception &e)
{
	cout << "Prefix: "<<pref<<endl;
	cout << e.what()<<endl;
}

}	

void As::AddFlow(string sourceIp, string destIp, string protocol, int sourcePort, int destPort)
{
	flowsMap[Flow::GetUniqueDescription(sourceIp, destIp, protocol, sourcePort, destPort)] = 1;
}


int As::FindFlow(string sourceIp, string destIp, string protocol, int sourcePort, int destPort)
{	map<string, unsigned int>::iterator iter = flowsMap.find(Flow::GetUniqueDescription(sourceIp, destIp, protocol, sourcePort, destPort));
	if(iter != flowsMap.end())
	{
		return 1;
	}
	else 
	{
		return 0;
	}
}

}
