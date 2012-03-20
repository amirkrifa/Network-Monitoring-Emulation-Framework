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

#include "SamplingPlatform.h"
#include "CommonStructs.h"

#include <string>
#include <list>
#include <unistd.h>
#include <sched.h>

using namespace std;
using namespace AmDev;


extern string SOFTFLOWD_HOST_NAME;
extern int SOFTFLOWD_HOST_PORT;

void Usage()
{
	fprintf(stderr, "Usage ./SPlatform  \n \
	-t [The xml topology file absolute path]\n \
	-a [The xml ASs configuration file absolute path]\n \
	-b [The binary TcpDump file absolute path]\n\
	-s [Softflowd server host name]\n\
	-p [Softflowd server port]\n\
	-g [Set this option if you want to show the generated topology before running the emulation]\n");
} 

int main(int argc, char** argv)
{

	int index;
	int c;
	string sTopologyFile;
	string sEdgeAsFile;
	string sBinaryFile;
	bool showGeneratedTopology = false;
	opterr = 0;

	while ((c = getopt (argc, argv, "t:a:b:s:p:g")) != -1)
	{
		switch (c)
		{
		case 't':
			if(optarg != NULL)
				sTopologyFile.assign(optarg);
			break;
		case 'a':
			if(optarg != NULL)
				sEdgeAsFile.assign(optarg);
			break;
		case 'b':
			if(optarg != NULL)
				sBinaryFile.assign(optarg);
			break;
		case 's':
			if(optarg != NULL)
				SOFTFLOWD_HOST_NAME.assign(optarg);
			break;
		case 'p':
			if(optarg != NULL)
				SOFTFLOWD_HOST_PORT = atoi(optarg);
			break;
		case 'g':
			showGeneratedTopology = true;
			break;
		case '?':
			if(optopt =='t' || optopt =='a' || optopt =='b' || optopt =='s' || optopt =='p')
				fprintf (stderr, "Incomplete option `-%c'.\n", optopt);
			 else if (isprint (optopt))
				fprintf (stderr, "Unknown option `-%c'.\n", optopt);
			else
				fprintf (stderr, "Unknown option character `\\x%x'.\n", optopt);
			return -1;
		default:
			return -1;
		}
	}
	if(sBinaryFile.length() == 0 || sEdgeAsFile.length() == 0 || sTopologyFile.length() == 0)
	{
		Usage();
		return -1;
	}
	{
	
		SamplingPlatform Sp;
		Sp.StartTrafficLoader("");
		Sp.AddEdgeAS(sEdgeAsFile);
		Sp.AddNewTransitNetwork(sTopologyFile);
		Sp.ConnectTransitNetworksToAss();
		Sp.ExploreTransitNetworks();
		if(showGeneratedTopology)
		{	
			Sp.ShowGeneratedTopology();
			{
				int x; cin >>x;
			}
		}
		Sp.LoadListOfBinaryTrafficFiles(sBinaryFile);
		sleep(1);
		Sp.StartSendingPackets();
	}
  	return 1;
}
