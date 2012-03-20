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

#ifndef _TRAFIC_LOADER_
#define _TRAFIC_LOADER_
#include "tinyxml.h"
#include "tinystr.h"
#include <iostream>
#include "exception.h"
#include <pcap.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <sys/ioctl.h>
#include <sys/poll.h>
#include <sys/socket.h>
#include <sys/time.h>
#include <sys/un.h>

#include <netinet/in.h>
#include <netinet/in_systm.h>
#include <netinet/ip.h>
#include <netinet/ip6.h>
#include <netinet/ip_icmp.h>
#define __FAVOR_BSD
#include <netinet/tcp.h>
#include <netinet/udp.h>
#include <arpa/inet.h>

#include <unistd.h>
#include <stdlib.h>
#include <errno.h>
#include <fcntl.h>
#include <grp.h>
#include <netdb.h>
#include <limits.h>
#include <pwd.h>
#include <signal.h>
#include <stdio.h>
#include <string.h>
#include <syslog.h>
#include <time.h>
#include <string>
#include <map>
#include <list>
#include <fstream>
using namespace std;
namespace AmDev{

/* 
 * Capture length for libpcap: Must fit the link layer header, plus 
 * a maximally sized ip/ipv6 header and most of a TCP header
 */
#define LIBPCAP_SNAPLEN_V4		96
#define LIBPCAP_SNAPLEN_V6		160

#ifndef IP_OFFMASK
# define IP_OFFMASK		0x1fff	/* mask for fragmenting bits */
#endif
#ifndef IPV6_VERSION
#define IPV6_VERSION		0x60
#endif
#ifndef IPV6_VERSION_MASK
#define IPV6_VERSION_MASK	0xf0
#endif
#ifndef IPV6_FLOWINFO_MASK
#define IPV6_FLOWINFO_MASK	ntohl(0x0fffffff)
#endif
#ifndef IPV6_FLOWLABEL_MASK
#define IPV6_FLOWLABEL_MASK	ntohl(0x000fffff)
#endif

/* Return values from process_packet */
#define PP_OK		0
#define PP_BAD_PACKET	-2
#define PP_MALLOC_FAIL	-3


/* Describes a datalink header and how to extract v4/v6 frames from it */
struct DATALINK {
	int dlt;		/* BPF datalink type */
	int skiplen;		/* Number of bytes to skip datalink header */
	int ft_off;		/* Datalink frametype offset */
	int ft_len;		/* Datalink frametype length */
	int ft_is_be;		/* Set if frametype is big-endian */
	u_int32_t ft_mask;	/* Mask applied to frametype */
	u_int32_t ft_v4;	/* IPv4 frametype */
	u_int32_t ft_v6;	/* IPv6 frametype */
};
/* Datalink types that we know about */
static const struct DATALINK lt[] = {
	{ DLT_EN10MB,	14, 12,  2,  1, 0xffffffff,  0x0800,   0x86dd },
	{ DLT_PPP,	 5,  3,  2,  1, 0xffffffff,  0x0021,   0x0057 },
#ifdef DLT_LINUX_SLL
	{ DLT_LINUX_SLL,16, 14,  2,  1, 0xffffffff,  0x0800,   0x86dd },
#endif
	{ DLT_RAW,	 0,  0,  1,  1, 0x000000f0,  0x0040,   0x0060 },
	{ DLT_NULL,	 4,  0,  4,  0, 0xffffffff, AF_INET, AF_INET6 },
#ifdef DLT_LOOP
	{ DLT_LOOP,	 4,  0,  4,  1, 0xffffffff, AF_INET, AF_INET6 },
#endif
	{ -1,		-1, -1, -1, -1, 0x00000000,  0xffff,   0xffff },
};

class SamplingPlatform;
struct packet;
class TrafficLoader;
/* Context for libpcap callback functions */
struct CB_CTXT {
	SamplingPlatform * sp;
	int linktype;
	int fatal;
	int want_v6;
	bool stopCollecting;
	TrafficLoader * tl;
};


class TrafficLoader{
public:
	TrafficLoader(char *);
	virtual ~TrafficLoader();
	int GenerateIpsList();
	int ParseIpsBasedOnClass(string & IpsListFileName);
	inline string GetDumpFilePath()const {return sDumpFile;}
	bool IsReadable( const string & file ); 

	// Parsing th text file resulting from tcpdump -r API
	bool IsItValid(string & sDataRecord);
	string GetDataRecordProtocol(string & sDataRecord);
	long int GetPacketSizeFromTheDataRecord(string &sDataRecord);
	long int GetPacketTimeStampFromTheDataRecord(string &sDataRecord);
	long int GetPacketTimeStampSecondFromTheDataRecord(string &sDataRecord);
	string GetDataRecordSourceIp(string & sDataRecord);
	string GetDataRecordDestIp(string & sDataRecord);
	string GetDataRecordSourcePort(string & sDataRecord);
	string GetDataRecordDestPort(string & sDataRecord);
	bool IsFragment(string & sDataRecord);
	unsigned long int GetFragmentSeqNumber(string & sDataRecord);

	// pcap base API
	void StartTheInterfaceThreadF(string &absolutePath, SamplingPlatform *sp);
	void StartTheInterfaceThread(list<string> &ap, SamplingPlatform *sp);
	static void * LoadTcpDumpBinaryFile(void *);
	void SetupPacketCapture(struct pcap **pcap, int *linktype, char *capfile, char *bpf_prog, int need_v6);
	static int DatalinkCheck(int linktype, const u_int8_t *pkt, u_int32_t caplen, int *af);
	static int ProcessPacket(const u_int8_t *pkt, int af, const u_int32_t caplen, const u_int32_t len, const struct timeval *received_time, packet *);
	static int Ipv4ToUpLevel(const u_int8_t *pkt, size_t caplen, size_t len, int *isfrag, int af, packet *);
	static int Ipv6ToUpLevel(const u_int8_t *pkt, size_t caplen, size_t len, int *isfrag, int af, packet *);
	static int TransportToUpLevel(const u_int8_t *pkt, const size_t caplen, int isfrag, int protocol, int ndx, packet *);
	inline bool GetLoadingThreadStatus()	
	{
		return loadingThreadStatus;
	}
	inline void TraficLoadingEnded()
	{
		loadingThreadStatus = false;
	}
	inline void AddPacket()
	{
		numberOfPacketsLoaded++;
	}
	inline bool AnotherDataBinaryFile()
	{
		if(!mapBinaryFiles.empty())
			return !(currentFile == mapBinaryFiles.size() - 1);
		else return false;
	}
	string & GetTheNextBinaryFile()
	{
		currentFile++;
		return mapBinaryFiles[currentFile];
	}
	fstream log;
private:
	char * capFile;
	pcap_t *pcap;
	string sDumpFile;
	bool loadingThreadStatus;
	unsigned int numberOfPacketsLoaded;
	// If more than just one file
	map<int, string> mapBinaryFiles;
	int currentFile;
	
};
}
#endif
