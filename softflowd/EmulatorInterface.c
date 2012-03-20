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


#include"EmulatorInterface.h" 
#include <pthread.h>
#include "sys-tree.h"
#include "convtime.h"
#include "treetype.h"
#include "SamplingModule.h"
#include "ControlServer.h"
#include <semaphore.h>
#include "softflowd.h"

int StopsEmulatorThread = 0;
extern sem_t flow_expiry_sem;
extern sem_t end_msg;


struct in_addr emulator_monitors_adrs[DEFAULT_MAXIMUM_NUMBER_OF_EMULATORS];
u_int32_t nbr_monitors = 0;

typedef struct MonitorStat
{
	char monitorAdr[50];
	unsigned int numberOfFlows;
	struct MonitorStat * next;
}MonitorStat; 

MonitorStat * statList = NULL;

int GetStopsEmulatorThread()
{
	sem_wait(&end_msg);
		int ret = StopsEmulatorThread;
	sem_post(&end_msg);
	return ret;
}

void SetStopsEmulatorThread()
{
	sem_wait(&end_msg);
		StopsEmulatorThread = 1;
	sem_post(&end_msg);
}

void add_monitor( char * ip)
{
	// Verify if the monitor have been already added
	int i = 0;
	
	for(; i < nbr_monitors;i++)
	{
		if(strcmp(inet_ntoa(emulator_monitors_adrs[i]), ip) == 0)
			break;
	}

	if(i >= nbr_monitors)
	{
		if(inet_pton(AF_INET, ip, (void*)&emulator_monitors_adrs[nbr_monitors]) == 0)
		{
			logit(LOG_ERR,"%s:%i Error occured while converting the source ip adr: %s to binary format", __FILE__, __LINE__, ip);
		}

	
		nbr_monitors++;
	}
}

void UpdateMonitorNumberOfFlows(char * ip, int x)
{
	MonitorStat * tmp = statList;
	while(tmp != NULL)
	{
		if(strcmp(tmp->monitorAdr, ip) == 0)
			break;
		tmp = tmp->next;
	}

	if(tmp != NULL)
	{
		// Monitor found
		tmp->numberOfFlows += x;
		tmp = NULL;
	}
}

void ClearMonitorsStatList()
{
	MonitorStat * tmp = statList;
	while(tmp != NULL)
	{
		MonitorStat * tmp2 = tmp;
		tmp = tmp->next;

		tmp2->next = NULL;
		free(tmp2);
	}
}

void WriteMonitorsStatToTraceFile()
{
	MonitorStat * tmp = statList;
	while(tmp != NULL)
	{
		char fileName[50];
		strcpy(fileName, tmp->monitorAdr);
		strcat(fileName,".txt");
		FILE * fp;
		fp = fopen(fileName, "w+");
		fprintf(fp,"%u\n", tmp->numberOfFlows);
		fclose(fp);	

		tmp = tmp->next;
	}
}
/* Return values from process_emulator_packet */
#define PP_OK		0
#define PP_BAD_PACKET	-2
#define PP_MALLOC_FAIL	-3

/*
 * Main emulator per-packet processing function. Take a packet (provided by 
 * the emulator) and attempt to find a matching flow. If no such flow exists, 
 * then create one. 
 *
 * Also marks flows for fast expiry, based on flow or packet attributes
 * (the actual expiry is performed elsewhere)
 */

static int process_emulator_packet(struct FLOWTRACK *ft, char *pkt, int af, const u_int32_t caplen, const u_int32_t len, const struct timeval *received_time)
{
	char tmpPacket[100];
	strcpy(tmpPacket,pkt);
	struct FLOW tmp, *flow;
	int frag;
	char tmpEm[16];
	int ndx;	
	ft->total_packets++;

	/* Convert the IP packet to a flow identity */
	memset(&tmp, 0, sizeof(tmp));
	switch (af) {
	case AF_INET:
		if (emulator_ipv4_to_flowrec(&tmp, tmpPacket, caplen, len, &frag, af, &ndx) == -1)
			goto bad;
		break;
	case AF_INET6:
			goto bad;
		break;
	default:
 bad: 
		logit(LOG_DEBUG,"Another bad packet.");
		ft->bad_packets++;
		
		
		
		return (PP_BAD_PACKET);
	}

	if (frag)
		ft->frag_packets++;

	/* Zero out bits of the flow that aren't relevant to tracking level */
	switch (ft->track_level) {
	case TRACK_IP_ONLY:
		tmp.protocol = 0;
		/* FALLTHROUGH */
	case TRACK_IP_PROTO:
		tmp.port[0] = tmp.port[1] = 0;
		tmp.tcp_flags[0] = tmp.tcp_flags[1] = 0;
		/* FALLTHROUGH */
	case TRACK_FULL:
		break;
	}

	/* If a matching flow does not exist, create and insert one */
	if ((flow = (struct FLOW *)FLOW_FIND(FLOWS, &ft->flows, &tmp)) == NULL) {
		/* Allocate and fill in the flow */
		if ((flow = malloc(sizeof(*flow))) == NULL) {
			logit(LOG_ERR, "%s:%i process_packet: flow malloc(%u) fail",__FILE__,__LINE__,
			    sizeof(*flow));

			

			return (PP_MALLOC_FAIL);
		}
		memcpy(flow, &tmp, sizeof(*flow));
		
		// Flow start time
		flow->flow_start.tv_sec = received_time->tv_sec;
		flow->flow_start.tv_usec = received_time->tv_usec;
		flow->flow_last.tv_sec = received_time->tv_sec;
		flow->flow_last.tv_usec = received_time->tv_usec;
		
		flow->flow_seq = ft->next_flow_seq++;
		FLOW_INSERT(FLOWS, &ft->flows, flow);

		/* Allocate and fill in the associated expiry event */
		if ((flow->expiry = malloc(sizeof(*flow->expiry))) == NULL) {
			logit(LOG_ERR, "%s:%i process_packet: expiry malloc(%u) fail",__FILE__,__LINE__,
			    sizeof(*flow->expiry));	
	
			

			return (PP_MALLOC_FAIL);
		}
		flow->expiry->flow = flow;
		flow->expiry->expires_at =  1;
		flow->expiry->reason = R_GENERAL;
		EXPIRY_INSERT(EXPIRIES, &ft->expiries, flow->expiry);

		ft->num_flows++;

		if (verbose_flag)
			logit(LOG_DEBUG, "ADD FLOW %s", format_flow_brief(flow));

		// Set the emulator monitor source adr
		char tmpPacket[100];
		strcpy(tmpPacket, pkt); 
		
		memset(tmpEm,'\0',16);
		if(get_emulator_packet_part(tmpPacket, 0, tmpEm) == -1)
		{
			logit(LOG_ERR,"%s:%i Error occured while parsing the packet: %s",__FILE__,__LINE__, pkt);
			return PP_BAD_PACKET;
		}else
		{
			
			// setting struct in_addr emulator_monitor_adr
			if(ndx == 0)
			{
				if(inet_pton(af, tmpEm, (void*)&flow->emulator_monitor_adr_0[0]) == 0)
				{
					/*Error occured while parsing the source adr*/
					logit(LOG_ERR,"%s:%i Error occured while converting the source ip adr: %s to binary format", __FILE__, __LINE__, tmpEm);
					return PP_BAD_PACKET;
				}
				
				flow->monitor_stat_0[0].octets = flow->octets[0];
				flow->monitor_stat_0[0].packets = flow->packets[0];					

			}else if (ndx ==1)
			{
				if(inet_pton(af, tmpEm, (void*)&flow->emulator_monitor_adr_1[0]) == 0)
				{
					/*Error occured while parsing the source adr*/
					logit(LOG_ERR,"%s:%i Error occured while converting the source ip adr: %s to binary format", __FILE__, __LINE__, tmpEm);
					return PP_BAD_PACKET;
				}

				flow->monitor_stat_1[0].octets = flow->octets[0];
				flow->monitor_stat_1[0].packets = flow->packets[0];
				
			}


			flow->number_of_monitors[ndx] = 1;
			flow->number_of_monitors[ndx^1] = 0;
			
			add_monitor(tmpEm);
			// TODO:
			//UpdateMonitorNumberOfFlows(inet_ntoa(flow->emulator_monitor_adr[0]), 1);
		}	
	
	} else if(flow->expiry->expires_at != 0 ){
			// Add a source Monitor adr if a new one appears
			memset(tmpEm,'\0',16);
			if(get_emulator_packet_part(tmpPacket, 0, tmpEm) == -1)
			{
				logit(LOG_ERR,"%s:%i Error occured while parsing the packet: %s",__FILE__,__LINE__, pkt);
				return PP_BAD_PACKET;
			}else
			{
				add_monitor(tmpEm);

				// Verify if it is a new emulator
				struct in_addr tmpAddr;
				if(inet_pton(af, tmpEm, (void*)&tmpAddr) == 0)
				{
					/*Error occured while parsing the source adr*/
					logit(LOG_ERR,"%s:%i Error occured while converting the source ip adr: %s to binary format", __FILE__, __LINE__, tmpEm);
					return PP_BAD_PACKET;
				}
				
				if(ndx == 0)
				{
					int k = 0;
					for(;k < flow->number_of_monitors[0]; k++)
					{
						if(memcmp(&flow->emulator_monitor_adr_0[k], &tmpAddr, sizeof(struct in_addr)) == 0)
							break;
					}
					if(k >= flow->number_of_monitors[0])
					{
						// A New 0 monitor
						flow->emulator_monitor_adr_0[flow->number_of_monitors[0]].s_addr = tmpAddr.s_addr;
						flow->monitor_stat_0[flow->number_of_monitors[0]].packets = tmp.packets[0];
						flow->monitor_stat_0[flow->number_of_monitors[0]].octets = tmp.octets[0];
						flow->number_of_monitors[0] ++; 
					}else 
					{
						// The monitor already exist
						flow->monitor_stat_0[k].packets += tmp.packets[0];
 						flow->monitor_stat_0[k].octets += tmp.octets[0];
					
					}
					
					
				}else if(ndx == 1)
				{
					int k = 0;
					for(;k < flow->number_of_monitors[1]; k++)
					{
						if(memcmp(&flow->emulator_monitor_adr_1[k], &tmpAddr, sizeof(struct in_addr)) == 0)
							break;
					}
					if(k >= flow->number_of_monitors[1])
					{
						// A New 0 monitor
						flow->emulator_monitor_adr_1[flow->number_of_monitors[1]].s_addr = tmpAddr.s_addr;
						flow->monitor_stat_1[flow->number_of_monitors[1]].packets = tmp.packets[1];
						flow->monitor_stat_1[flow->number_of_monitors[1]].octets = tmp.octets[1];
						flow->number_of_monitors[1] ++; 
					}else
					{
						// The monitor already exist
						flow->monitor_stat_1[k].packets += tmp.packets[1];
 						flow->monitor_stat_1[k].octets += tmp.octets[1];	
							 
					}
					
				
				}

				
			}

			/* Update flow statistics only if we received a new packet from the emulator */
			char alreadyReceived[1];
			if(get_emulator_packet_part(tmpPacket, 14, alreadyReceived) == -1)
			{
				logit(LOG_ERR,"%s:%i Error occured while parsing the packet: %s",__FILE__, __LINE__, pkt);
				return PP_BAD_PACKET;
			}	
		
			if(atoi(alreadyReceived) == 0)
			{
				flow->packets[0] += tmp.packets[0];
				flow->octets[0] += tmp.octets[0];
				flow->tcp_flags[0] |= tmp.tcp_flags[0];
				flow->packets[1] += tmp.packets[1];
				flow->octets[1] += tmp.octets[1];
				flow->tcp_flags[1] |= tmp.tcp_flags[1];
				
				// update the last time the flow was modified
				flow->flow_last.tv_sec = received_time->tv_sec;
				flow->flow_last.tv_usec = received_time->tv_usec;
				
				// Source As
				flow->as_src[ndx] = tmp.as_src[ndx];
				// Dest AS adr
				flow->as_dst[ndx ^ 1] = tmp.as_dst[ndx^1];				
				
			}

			flow_update_expiry(ft, flow);
		}
	

	
	
	return (PP_OK);
}

// Function called each time we receive a new packet from the Emulator
static int accept_emulator(int lsock, struct FLOWTRACK *ft)
{
	unsigned char buf[1024], *p;
	int fd = -1;
	int rlen = 0, fromlen = 0;
	
	#ifdef USE_EMULATOR_UNIX_LISTENER
		struct  sockaddr_un from;
		fromlen = sizeof(from);
	#endif
	#ifdef USE_EMULATOR_UDP_LISTENER
		struct  sockaddr_in from;
		fromlen = sizeof(from);
	#endif

	memset(buf,'\0',1024);

	#ifdef USE_EMULATOR_UNIX_LISTENER
        	rlen = recvfrom(lsock, buf, 1024, 0, (struct sockaddr *)&from, &fromlen);
	#endif
	#ifdef USE_EMULATOR_UDP_LISTENER
		rlen = recvfrom(lsock, buf, 1024, 0, (struct sockaddr *)&from, &fromlen);
	#endif
	
        if (rlen == -1) 
	{
		strerror((errno));
		return -1;
        } else 
	{
		buf[1024]='\0';
	}
	
	if(strcmp(buf, "end") == 0)
	{
		// The end of transmission
		return 1;
	}

	char tmp[16];

	int caplen = 0; 
	struct timeval ts;	
				
	/* length this packet (off wire) */
	memset(tmp,'\0',16);
	get_emulator_packet_part(buf,3,tmp);  
	int len = atoi(tmp);	
		
	int  af=0;
	struct CB_CTXT *cb_ctxt;

		
	/* We assume, we have already eliminated the data link header */
	/* Setting the packet time stamp */
	// Set the current system time 	
	
	gettimeofday(&ts, NULL);
			
	/* 
	* We assume also that the frame type is IPv4
	* For IPv4 please set af = AF_INET;
	* For IPv6 please set af = AF_INET6;
	*/
	af = AF_INET;
		
	// Sampling the comming packets
	// Look for the responsible node Ip addr
	memset(tmp,'\0',16);
	if(get_emulator_packet_part(buf, 0, tmp) == -1)
	{
		logit(LOG_ERR,"%s:%i Error occured while parsing the packet: %s",__FILE__,__LINE__,  buf);
		return -1;
	}

	if(RandomPeriodicNodeSampling(tmp) == 1)
	{	
		if (process_emulator_packet(ft, buf, af, caplen, len, &ts) == PP_MALLOC_FAIL);
	}

	close(fd);
	return 0;
}

/* The emulator's socket setup function */
/* listener based on an AF_UNIX socket*/
int unix_emulator_listener(const char *path)
{
	int     sock;
        struct  sockaddr_un servaddr; 

        if ((sock = socket(AF_UNIX, SOCK_DGRAM, 0)) < 0) {
                logit(LOG_ERR,"%s:%i server: socket.\n",__FILE__,__LINE__);
                exit(1);
        }

        bzero(&servaddr, sizeof(servaddr));
        servaddr.sun_family = AF_UNIX;
        strcpy(servaddr.sun_path, path);
        unlink(path);

        if (bind(sock, (struct sockaddr *)&servaddr, sizeof(servaddr)) < 0) 
	{
                close(sock);
                logit(LOG_ERR,"%s:%i server: bind.\n",__FILE__,__LINE__);
		unlink(path);
                exit(1);
        }
	
	return (sock);
}

/*Listener base on an UDP AF_INET socket */
int udp_emulator_listener(int hostPort)
{
	int     sock;
        struct  sockaddr_in servaddr; 
	
	if ((sock = socket(AF_INET, SOCK_DGRAM, 0)) < 0) {
                logit(LOG_ERR,"%s:%i server: socket.\n",__FILE__,__LINE__);
                exit(1);
        }

        bzero(&servaddr, sizeof(servaddr));
	servaddr.sin_family = AF_INET;
	servaddr.sin_addr.s_addr = htonl(INADDR_ANY);
	servaddr.sin_port = htons(hostPort);

	if (bind(sock, (struct sockaddr *)&servaddr, sizeof(servaddr)) < 0) 
	{
                close(sock);
                logit(LOG_ERR,"%s:%i server: bind.\n",__FILE__,__LINE__);
                exit(1);
        }
	
	return (sock);
}


/* function used by the thread, which will receive packets from the emulator */
static void * EmulatorThreadProcess (void * arg)
{
	structThreadParam * ptrParam = (structThreadParam * )arg;
	int ret = 0; 
	while(1)
	{
		if(GetStopsEmulatorThread())
			break;
		
		sem_wait(&flow_expiry_sem);
			ret = accept_emulator(ptrParam->emulatorSocket, ptrParam->ft);
		sem_post(&flow_expiry_sem);
		
		if(ret == 1)
		{
			SetStopsEmulatorThread();
			SendEndMsg();
			break;
		}
	}

	
	free(ptrParam);
	WriteMonitorsStatToTraceFile();
	logit(LOG_DEBUG,"The Emulator's thread is shutting down.\n");
	pthread_exit (0);
		
}

/* Function used to start the emulator's interface thread*/
int StartEmulatorInterfaceThread(int emulatorSocket,struct FLOWTRACK *ft)
{
	
	structThreadParam * ptrParam= (structThreadParam*)malloc(sizeof(structThreadParam));
	pthread_t interfaceThread;
    	if(ptrParam==NULL)
		;
	ptrParam->emulatorSocket=emulatorSocket;
	ptrParam->ft=ft;
	
	pthread_attr_t threadAttr;

	if (pthread_attr_init (&threadAttr) != 0) 
	{
		logit(LOG_ERR,"%s:%i pthread_attr_init error.\n",__FILE__,__LINE__);
		return -1;
	}

	if (pthread_attr_setdetachstate (&threadAttr,PTHREAD_CREATE_DETACHED) != 0) 
	{	
		logit(LOG_ERR,"%s:%i pthread_attr_setdetachstate error.\n",__FILE__,__LINE__);
		return -1;
	}
		
	if (pthread_create(&interfaceThread,&threadAttr,EmulatorThreadProcess, (void *)ptrParam) < 0) 
	{
		logit(LOG_ERR,"%s:%i pthread_create error.\n",__FILE__,__LINE__);
		return -1;
	}
	return 1;
}

/*
* Function used to extract the different parts of a packet received from the emulator
*/
static int get_emulator_packet_part(char *complete_packet,int part_number,char * resulting_part)
{
	
	if(part_number <= MAX_EMULATOR_MESSAGE_PARTS)
	{
		/* Looking for a field from the network layer */
		int i=0, offset=0;
		int error=0;
		char * sep_position=NULL,* prev_sep_position=NULL;

		if(part_number == 0)
		{
			sep_position=strchr(complete_packet,'#');
			if(strncpy(resulting_part,complete_packet,sep_position-complete_packet) == NULL)
			{
				logit(LOG_ERR,"%s:%i Error occured while parsing the received data from the emulator.\n",__FILE__,__LINE__);
				return -1;
			}
			else return 0;
		}
		
		prev_sep_position = complete_packet;
		sep_position = complete_packet;

		for(;i <= part_number;i++)
		{
			prev_sep_position = sep_position;
			if(i == MAX_EMULATOR_MESSAGE_PARTS )
			{
				/*The last part of the message*/
				
				if(strncpy(resulting_part, prev_sep_position+1, (complete_packet+ strlen(complete_packet))-prev_sep_position-1) == NULL)
				{
					logit(LOG_ERR,"%s:%i Error occured while parsing the received data from the emulator.\n",__FILE__,__LINE__);
						return -1;
				}
				else return 0;
			}else if((sep_position=strchr(prev_sep_position+1,'#'))==NULL)
			{
				error=1;	
				break;
			}
		}

		if(!error)
		{
			if(strncpy(resulting_part,prev_sep_position+1,sep_position-prev_sep_position-1)==NULL)
			{
				logit(LOG_ERR,"%s:%i Error occured while parsing the received data %s from the emulator; part: %i.\n", __FILE__, __LINE__, complete_packet, part_number);
				return -1;
			}
			else return 0;
		} else 
			{
				logit(LOG_ERR,"%s:%i Error occured while parsing the received data %s from the emulator; part: %i.\n", __FILE__, __LINE__, complete_packet, part_number);
				return -1;
			}
	} else
		{
			logit(LOG_ERR,"%s:%i Error occured while parsing the received data %s from the emulator; part: %i.\n", __FILE__, __LINE__, complete_packet, part_number);
			return -1;
		}
}

/* Convert an IPv4 emulator packet to a partial flow record (used for comparison) 
* Returns 0 on success and -1 on failure.
*/
static int emulator_ipv4_to_flowrec(struct FLOW *flow, char *pkt, size_t caplen, size_t len, int *isfrag, int af, int * ndxx)
{
	int ndx;
	/*Used to save the different packet's part */
	char tmp[16];
	/*Temporary packet*/
	char tmpPacket[100];
	strcpy(tmpPacket, pkt); 
	
	/*Setting the ip offset*/
	unsigned short int ip_off;
	
	/* We are supposing that we are using IPv4 packets */
	
	/* Prepare to store flow in canonical format */
	/*Setting the source and destination adr */
	struct	in_addr ip_src,ip_dst;
	memset(tmp,'\0',16);
	if(get_emulator_packet_part(tmpPacket, 1, tmp) == -1)
	{
		logit(LOG_ERR,"%s:%i Error occured while parsing the packet: %s",__FILE__,__LINE__, pkt);
		return -1;
	}
	
	if(inet_pton(af, tmp, (void*)&ip_src) == 0)
	{
		/*Error occured while parsing the source adr*/
		logit(LOG_ERR,"%s:%i Error occured while converting the source ip adr: %s to binary format", __FILE__, __LINE__, tmp);
		return -1;
	}

	memset(tmp,'\0',16);
	if(get_emulator_packet_part(tmpPacket, 2, tmp) == -1)
	{
		logit(LOG_ERR,"%s:%i Error occured while parsing the packet: %s", __FILE__, __LINE__, pkt);
		return -1;
	}
	if(inet_pton(af, tmp, (void*)&ip_dst) == 0)
	{
		/*Error occured while parsing the source adr*/
		logit(LOG_ERR,"%s:%i Error occured while converting the destination ip adr: %s to binary format", __FILE__, __LINE__, tmp);
		return -1;
	}
	
	
	u_char	ip_p;

	memset(tmp,'\0',16);
	if(get_emulator_packet_part(tmpPacket, 10, tmp) == -1)
	{
		logit(LOG_ERR,"%s:%i Error occured while parsing the packet: %s",__FILE__,__LINE__, pkt);
		return -1;
	}

	
	if(strcmp(tmp, "TCP") == 0)
	{
		// tcp packet
		ip_p = 6;
		
		*isfrag = 0;
		char tmpF[20];
		memset(tmpF, '\0', 20);
		if(get_emulator_packet_part(tmpPacket, 11, tmpF) == -1)
		{
			logit(LOG_ERR,"%s:%i Error occured while parsing the packet: %s",__FILE__,__LINE__, pkt);
			return -1;
		}
		*isfrag = atoi(tmpF);
		if(*isfrag == 1)
		{
			memset(tmpF, '\0', 20);
			if(get_emulator_packet_part(tmpPacket, 12, tmpF) == -1)
			{
				logit(LOG_ERR,"%s:%i Error occured while parsing the packet: %s",__FILE__,__LINE__, pkt);
				return -1;
			}
			ip_off = atol(tmpF);
		}
		
	} else if(strcmp(tmp, "UDP") == 0)
	{
		// Udp packet
		ip_p = 17;
		
	}else if(strcmp(tmp, "ICMP") == 0)
	{
		// Icmp packet
		ip_p = 1;
	}

	
	ndx = memcmp(&ip_src, &ip_dst, sizeof(ip_src)) > 0 ? 1 : 0;
	*ndxx = ndx;
	flow->af = af;
	flow->addr[ndx].v4 = ip_src;
	flow->addr[ndx ^ 1].v4 = ip_dst;
	flow->protocol = ip_p;
	flow->octets[ndx] = len;
	flow->packets[ndx] = 1;

	// Assigning the source and destination ASes
	// Source AS adr
	memset(tmp,'\0',16);
	if(get_emulator_packet_part(tmpPacket, 8, tmp) == -1)
	{
		logit(LOG_ERR,"%s:%i Error occured while parsing the packet: %s",__FILE__,__LINE__, pkt);
		return -1;
	}
	flow->as_src[ndx] = atoi(tmp);

	// Dest AS adr
	memset(tmp,'\0',16);
	if(get_emulator_packet_part(tmpPacket, 9, tmp) == -1)
	{
		logit(LOG_ERR,"%s:%i Error occured while parsing the packet: %s",__FILE__,__LINE__, pkt);
		return -1;
	}
	flow->as_dst[ndx ^ 1] = atoi(tmp);

	
	/* Don't try to examine higher level headers if not first fragment */
	if (*isfrag && (ntohs(ip_off)) != 0)
		return (0);
	
	/* We use the default Ip header length 20 (5*4) bytes*/
	return (emulator_transport_to_flowrec(flow, pkt, 
	    caplen, *isfrag, ip_p, ndx));
}


/* Fill in transport-layer (tcp/udp) portions of an emulator flow record 
* Return 0 on success and -1 on failure.
*/
static int emulator_transport_to_flowrec(struct FLOW *flow, char *pkt, const size_t caplen, int isfrag, int protocol, int ndx)
{
	u_short sport,dport,th_flags = 0;
	u_char icmp_code,icmp_type;
	
	char tmpPacket[100];
	strcpy(tmpPacket, pkt);

		
	char tmp[16];	/*Used for saving temporary the different parts of a packet*/
	
	memset(tmp, '\0', 16);
	if(get_emulator_packet_part(tmpPacket, 6, tmp) == -1)
	{
		logit(LOG_ERR,"%s:%i Error occured while parsing the packet: %s", __FILE__, __LINE__, pkt);
		return -1;
	}
	sport = htons(atoi(tmp));

	memset(tmp, '\0', 16);
	if(get_emulator_packet_part(tmpPacket, 7, tmp) == -1)
	{
		logit(LOG_ERR,"%s:%i Error occured while parsing the packet: %s", __FILE__, __LINE__, pkt);
		return -1;
	}
	dport = htons(atoi(tmp));

	switch (protocol) 
	{
	case IPPROTO_TCP:
		flow->port[ndx] = sport;
		flow->port[ndx ^ 1] = dport; 
	
		memset(tmp, '\0', 16);
		if(get_emulator_packet_part(tmpPacket, 13, tmp) == -1)
		{
			logit(LOG_ERR,"%s:%i Error occured while parsing the packet: %s", __FILE__, __LINE__, pkt);
			return -1;
		}
		flow->tcp_flags[ndx] |= (unsigned char)tmp[0];
		break;
	case IPPROTO_UDP:
		flow->port[ndx] = sport; 
		flow->port[ndx ^ 1] = dport;
		break;
	case IPPROTO_ICMP:
		flow->port[ndx] = 0;
		flow->port[ndx ^ 1] = htons(icmp_type * 256 +
		    icmp_code); 
		break;
	}
	return (0);
}
