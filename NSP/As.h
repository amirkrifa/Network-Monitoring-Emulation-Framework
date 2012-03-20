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

#ifndef _AS_H_
#define _SA_H_
#include "Network.h"
#include <string>
#include <list>
#include <ext/hash_map>
#include "exception.h"
#include <map>
#include <fstream>
using namespace std;

namespace AmDev{

class TransitNetwork;
struct packet;
typedef list<string> IpsList;
class Flow
{
public:
	int CompareTo(Flow * f)
	{
		if(f->sourceIp.compare(sourceIp) == 0 && f->destIp.compare(destIp) == 0 && f->protocol.compare(protocol) == 0 && f->sourcePort == sourcePort && f->destPort == destPort)
			return 1;
		else return 0;
	}
	
	string sourceIp;
	string destIp;
	int sourcePort;
	int destPort;
	string protocol;
	
	static string GetUniqueDescription(string sourceIp, string destIp, string protocol, int sourcePort, int destPort)
	{
		string tmp;
		tmp.append(sourceIp);
		tmp.append(" ");
		tmp.append(destIp);
		tmp.append(" ");
		char sp[20];
		char dp[20];
		sprintf(sp, "%i", sourcePort);
		sprintf(dp, "%i", destPort);
		tmp.append(sp);
		tmp.append(" ");
		tmp.append(dp);
		tmp.append(" ");
		tmp.append(protocol);
		return tmp;
	}
	
};

class As: public Network {
public:
	As(string &IpsListFile, int iNumber);
	As(int iAsNumber, int iMaxIps, int iAsImportance,int minInterval, int maxInterval, char * ipClass, char * ipRangeStart, char * ipRangeEnd,int ipMask, int dispatchingMethod);
	void SetOutboundTrafficFilePath(string & path);
	bool GenerateIpsListFile();
	string GetListOfIpsFilePath() const;
	int GetNumber() const {return iAsNumber;}
	void ConnectToTransitNetwork(TransitNetwork * const ptrTransitNetwork);
	void ShowDetails();
	void SendPacket(packet *);
	bool VerifyIpAdr(string &ip, int method);
	inline bool IsInOurClass(string &ip)
	{
		if(GetClass(ip).compare(ipClass) == 0)
			return true;
		return false;
	}
	void AddIp(string &szIp);
	inline int GetDispatchingMethod()
	{
		return dispatchingMethod;
	}
	void AddPrefix(string &pref,int srcdest);
	int FindPrefix(string &pref, int srcdest);
	inline int GetMask()
	{
		return ipMask;
	}
	inline int GetNumberOfPrefix()
	{
		return numberOfPrefix;
	}
	virtual ~As();
	
	void AddFlow(string sourceIp, string destIp, string protocol, int sourcePort, int destPort);
	int FindFlow(string sourceIp, string destIp, string protocol, int sourcePort, int destPort);
	// static methods
 	static string GetClass( string &Ip);
	static int GetIpPartNumber(string &Ip, int i);
	static string ApplyMask(string &ip, int mask);
	int IsItWithinInterval(int randomValue)
	{
		if(randomValue >= minInterval && randomValue <= maxInterval)
			return 1;	
		else return 0;
	}
private:
	bool IsItInTheRange(string &ipAdr);
	void InitPrefixTabs();
	int64_t GetPrefix(string &pref, int length);
private:
	string OutboundTrafficFile;
	IpsList ListOfIps;
	unsigned int iNumberOfIps;
	list<TransitNetwork *> ptrListTn;

	// Informations extracted from the xml file
	int iAsNumber;
	int iAsMaxIps;
	int dispatchingMethod;
	string ipClass;
	string ipRangeStart;
	string ipRangeEnd;

	int iAsImportance;
	int minInterval;
	int maxInterval;

	int ipMask;

	// Tables used to store prefix
	int tabPref1[255][255];
	int tabPref2[255][255];
	map<string , unsigned int> prefMap;
	map<string, unsigned int > flowsMap;
	
	int numberOfPrefix;
	fstream asLog;
};
}

#endif 

