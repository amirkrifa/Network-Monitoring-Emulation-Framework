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

#include "Link.h"
#include "CommonStructs.h"
#include <iostream>
#include "TransitNetwork.h"


namespace AmDev{

Link::Link(TransitNetwork* ptn, unsigned int Cost,unsigned int Bandwidth,unsigned int Delay,string sAd1,string sAd2,int iNbrDst,map<string,string> mDests )
{
try
{
	ptrTransitNetwork = ptn;
	
	if(iNbrDst <= 0)
		throw NspException("Invalid number of destinations.",__FILE__,__LINE__);
	else iNumberOfDest = iNbrDst;

	if( Cost < 0) 
		throw NspException("Invalid link cost.",__FILE__,__LINE__);
	else uCost = Cost;

	if( Bandwidth < 0) 
		throw NspException("Invalid link bandwidth.",__FILE__,__LINE__);		
	else uBandwidth = Bandwidth;

	if( Delay < 0) 
		throw NspException("Invalid link Delay.",__FILE__,__LINE__);		
	else uDelay = Delay;

	if( sAd1.length() == 0) 
		throw NspException("Invalid link source adapter.",__FILE__,__LINE__);		
	else sAdapter1.assign(sAd1);
	
	if( sAd2.length() == 0) 
		throw NspException("Invalid link destination adapter.",__FILE__,__LINE__);		
	else sAdapter2.assign(sAd2);

	if(mDests.empty())
		throw NspException("Invalid final destinations list.",__FILE__,__LINE__);

	for(map<string,string>::iterator iter = mDests.begin();iter != mDests.end();iter++)
	{
		mFinalDest.insert(pair<string,string>(iter->first,iter->second));
	}
}

catch(exception & e)
{
	cout << e.what() <<endl;
}
}

Link::~Link()
{
}

bool  Link::IsItAFinalDest(string &sFinalDest) 
{
try 
{
 if(mFinalDest.empty()) 
	throw NspException(" No Final destination.",__FILE__,__LINE__); 
 for(map<string,string>::iterator iter=mFinalDest.begin();iter!=mFinalDest.end();iter++)
  {
    if((*iter).first.compare(sFinalDest)==0)
      return true;
  }
  return false;
}
catch(exception &e)
{
	cout <<e.what()<<endl;
}
}

bool Link::IsItOneOfTheAdapters(string& sAd)
{
  return (GetAdapter1().compare(sAd)==0 || GetAdapter2().compare(sAd)==0);
}

string Link::GetListOfDestinations()
{
try
{
  string sRes;
  for(map<string,string>::iterator iter=mFinalDest.begin();iter!=mFinalDest.end();iter++)
  {
    sRes.append(iter->first);
    if(iter!=mFinalDest.end())
      sRes.append("--");
  }
  if(sRes.length()==0)
	  throw NspException("List of destinations sRes==null.",__FILE__,__LINE__);
  return sRes;
}
catch(exception &e)
{
	cout<<e.what()<<endl;
}
}

void Link::GetNextInterfaceTowrd(string &sDest, string & res)
{
try
{
	if(sDest.length()==0)
		throw NspException("Incorrect destination in sDest.",__FILE__,__LINE__);
	if(mFinalDest.empty())
		throw NspException("Incorrect Link exit interfaces.",__FILE__,__LINE__);
	res.assign("");
	for(map<string,string>::iterator iter=mFinalDest.begin();iter!=mFinalDest.end();iter++)
	{
		if(iter->first.compare(sDest)==0)
			res.append(iter->second);
	}
	if(res.length() == 0)
		throw NspException("No next interface found.",__FILE__,__LINE__);
	
}
catch(exception &e)
{
	cout<<e.what()<<endl;
}
}

string Link::GetTheOppositeAdapter(string &ad)
{
try
{
	if(ad.compare(sAdapter1) == 0)
		return sAdapter2;
	else if(ad.compare(sAdapter2) == 0)
		return sAdapter1;
	else {cout <<"Adapter: "<<ad<<endl;throw NspException("Invalid input adapter.",__FILE__,__LINE__);}
}

catch(exception &e)
{
	cout<<e.what()<<endl;
}
}

}
