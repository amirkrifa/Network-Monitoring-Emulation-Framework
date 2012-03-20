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

#ifndef _NODE_H_
#define _NODE_H_
#include <iostream>
#include <string>
#include <list>
#include "exception.h"

using namespace std;

namespace AmDev{
class Link;
typedef list<Link *> LinkList;

class Node{
public:
	Node( int iA);
	virtual ~Node();
	virtual void AddLink(Link *const );
	virtual void ShowDetails();
	void AddAdapterAdr(string &Ip);
	bool IsItYourAdapter(string sAdapter);
	inline string GetMonitoringAdapter()
	{
		if(ListOfAdapters.size() > 0)
		{
			return *(ListOfAdapters.begin());
			
		}
	}

	Link * GetLink(string adapter);
protected:
	int iNumberOfNetworkAdapters;
	LinkList ptrListOfLinks;
	list<string> ListOfAdapters;
	
};
}
#endif
