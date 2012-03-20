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

#ifndef EMULATOR_INTERFACE_H
#define EMULATOR_INTERFACE_H
#include "common.h"
#include "softflowd.h"

#define MAX_EMULATOR_MESSAGE_PARTS 14

#undef USE_EMULATOR_UNIX_LISTENER
#define USE_EMULATOR_UDP_LISTENER

#ifdef USE_EMULATOR_UNIX_LISTENER
#undef USE_EMULATOR_UDP_LISTENER
#endif

#ifdef USE_EMULATOR_UDP_LISTENER
#undef USE_EMULATOR_UNIX_LISTENER
#endif

/* Structure used to pass parameters to the emulator thread. */
typedef struct structThreadParam
{
	struct FLOWTRACK *ft;
	int emulatorSocket;
}structThreadParam;


void add_monitor( char * ip);

/* Prototype for functions defined in EmulatorInterface.c */
void UpdateMonitorNumberOfFlows(char * ip, int x);
void ClearMonitorsStatList();
void WriteMonitorsStatToTraceFile();

static int accept_emulator(int lsock, struct FLOWTRACK *ft);
static void * EmulatorThreadProcess (void * arg);
int StartEmulatorInterfaceThread(int emulatorSocket,struct FLOWTRACK *ft);
int unix_emulator_listener(const char *path);
int udp_emulator_listener(int hostPort);

static int process_emulator_packet(struct FLOWTRACK *ft, char *pkt, int af, const unsigned int caplen, const u_int32_t len, const struct timeval *received_time);
static int get_emulator_packet_part(char *complete_packet,int part_number,char * resulting_part);
static int emulator_ipv4_to_flowrec(struct FLOW *flow, char *pkt, size_t caplen, size_t len, int *isfrag, int af, int * ndxx);
static int emulator_transport_to_flowrec(struct FLOW *flow, char *pkt, const size_t caplen, int isfrag, int protocol, int ndx);
void SetStopsEmulatorThread();
int GetStopsEmulatorThread();
/* Prototype for functions and variables shared from softflowd.c */
void flow_update_expiry(struct FLOWTRACK *ft, struct FLOW *flow);
const char * format_flow_brief(struct FLOW *flow);
extern int verbose_flag;

#endif
