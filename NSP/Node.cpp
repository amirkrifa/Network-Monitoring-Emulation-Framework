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

#include "Node.h"
#include "Link.h"
const int DEBUG =1;

namespace AmDev{

Node::Node(int iA):iNumberOfNetworkAdapters(iA)
{
}

Node::~Node()
{
	if(!ptrListOfLinks.empty())
		ptrListOfLinks.clear();
	if(!ListOfAdapters.empty())
		ptrListOfLinks.clear();
}

void Node::AddLink(Link * const ptrLink)
{
try
{
  	if(ptrLink==NULL)
    		throw NspException("Error encountred while adding the link.",__FILE__,__LINE__); 

  	if(ptrListOfLinks.size()<= iNumberOfNetworkAdapters)
  	{
    		ptrListOfLinks.push_back(ptrLink);
  	}else throw NspException(" Number of links is exceeding the number of adapters.",__FILE__,__LINE__); 
}

catch (exception &e)
{
	cout<<e.what()<<endl;;
}
}


void Node::AddAdapterAdr(string& Ip)
{
try
{
  	if(Ip.length()==0)
    		throw NspException("Error encountred while adding an adapter.",__FILE__,__LINE__);
  	if(ListOfAdapters.size()<= iNumberOfNetworkAdapters)
  	{
    		ListOfAdapters.push_back(Ip);
  	}else throw NspException("Exceeding the number of adapters.",__FILE__,__LINE__); 
}

catch (exception &e)
{
	cout<<e.what()<<endl;
}
}

void Node::ShowDetails()
{

}
// Verify if the given Ip @ is already assigned to one of its adapters
bool Node::IsItYourAdapter(string sAdapter)
{
try
{
  	if(sAdapter.length()==0)
    		throw NspException("Incorrect adapter.",__FILE__,__LINE__);
  	for(list<string>::iterator iter= ListOfAdapters.begin();iter!=ListOfAdapters.end();iter++)
  	{
    		if((*iter).compare(sAdapter)==0)
      		return true;
  	}
  	return false;
}

catch(exception &e)
{
	cout<<e.what()<<endl;
}
}


Link * Node::GetLink(string adapter)
{
	for(LinkList::iterator iter = ptrListOfLinks.begin(); iter!=ptrListOfLinks.end(); iter++)
	{
		if((*iter)->GetAdapter1().compare(adapter) == 0 || (*iter)->GetAdapter2().compare(adapter) == 0)
			return (*iter);
	}
	return NULL;
}
}


