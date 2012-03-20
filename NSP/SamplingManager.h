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

#ifndef SAMPLING_MANAGER_H
#define SAMPLING_MANAGER_H
#include <semaphore.h>
#include <list>
using namespace std;
namespace AmDev
{
class SamplingManager;
class SamplingPlatform;
class SamplingManager;
typedef struct csThreadParam
{
	SamplingManager * sm;	
}csThreadParam;

#define MAXPENDING 5 
#define CONTROL_BUFFER_SIZE 1024
#define MAX_CONTROL_MESSAGE_PARTS 3

class SamplingManager
{

public:
	SamplingManager(SamplingPlatform * sp);
	void SetStopsControlServer();
	int GetStopsControlServer();
	int StartControlServer();
	void SendEndMsg();
	static void  * ThreadProcessFunction(void *arg);
	void HandleTCPClient(int clntSocket);
	static int get_control_packet_part(char *complete_packet,int part_number,char * resulting_part);
	~SamplingManager();
private:
	SamplingPlatform * sp;	
	sem_t stop_control_server;
};
}
#endif
