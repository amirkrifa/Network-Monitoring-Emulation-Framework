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

#ifndef _LINK_H_
#define _LINK_H_
#include <string>
#include <list>
#include <map>
#include "exception.h"
using namespace std;
namespace AmDev{
struct packet;
class TransitNetwork;
class Link{
public:
	Link( TransitNetwork* tn,unsigned int Cost,unsigned int Bandwidth,unsigned int Delay,string sAd1,string sAd2,int iNbrDst, map<string,string> mDests);

	inline void SetCost(unsigned int uCost){this->uCost=uCost;}
	inline void SetBandwidth(unsigned int uBandwidth){this->uBandwidth=uBandwidth;}
	inline void SetDelay(unsigned int uDelay){this->uDelay=uDelay;}
	inline unsigned int GetCost() const {return uCost;}  
	inline unsigned int GetBandwidth() const {return uBandwidth;}  
	inline unsigned int GetDelay() const {return uDelay;}
	inline void SetAdapter1(string & ip){sAdapter1.assign(ip);}
	inline void SetAdapter2(string & ip){sAdapter2.assign(ip);}
	inline string  GetAdapter1() const {return sAdapter1;}
	inline string  GetAdapter2() const {return sAdapter2;}
	bool  IsItAFinalDest(string & sFinalDest);
	bool IsItOneOfTheAdapters(string &sAd);
	string GetListOfDestinations();
	void GetNextInterfaceTowrd(string & sDest, string &res);
	string GetTheOppositeAdapter(string & ad);
	
	map<string,string> mFinalDest;
	virtual ~Link();
private:
	string sAdapter1;
	string sAdapter2;
	unsigned int uCost;
	unsigned int uBandwidth;
	unsigned int uDelay;
	int iNumberOfDest;
	TransitNetwork * ptrTransitNetwork;
};
}
#endif
