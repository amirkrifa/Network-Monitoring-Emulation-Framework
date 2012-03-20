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

#ifndef CONTROL_SERVER_H
#define CONTROL_SERVER_H

/* Maximum outstanding connection requests */
#define MAXPENDING 5    
#define MAX_CONTROL_MESSAGE_PARTS 3
#define CONTROL_BUFFER_SIZE 1024
#define CONTROL_SERVER_DEFAULT_PORT 26174

/* Structure used to pass parameters to the ControlServer thread. */
typedef struct csThreadParam
{
	
}csThreadParam;

/* Prototype for functions defined in ControlServer.c */
int GetStopsControlServer();
void SetStopsControlServer();
static void *ThreadProcessFunction(void *arg);
void HandleTCPClient(int clntSocket);
void DieWithError(char *errorMessage);
int StartControlServer();
static int get_control_packet_part(char *complete_packet,int part_number,char * resulting_part);
void SendEndMsg();
#endif
