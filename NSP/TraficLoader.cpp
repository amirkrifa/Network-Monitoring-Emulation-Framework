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

#include "TraficLoader.h"
#include <fstream>
// Just for the call back function
#include "SamplingPlatform.h"
#include "CommonStructs.h"

namespace AmDev{

typedef struct TlThreadParam
{
	SamplingPlatform * sp;
	TrafficLoader * tl;
	char absolutePath[100];
	
} TlThreadParam;

TrafficLoader::TrafficLoader(char * szFn)
{
try
{
	loadingThreadStatus = true;
	numberOfPacketsLoaded = 0;
	currentFile = 0;
	log.open("./trace.txt",ios::out|ios::app);
  	//if(IsReadable(string(szFn)))
    		sDumpFile.append(szFn);
}

catch(exception &e)
{
	cout<<e.what()<<endl;
}
}

TrafficLoader::~TrafficLoader()
{
	log.close();
	// Clean the map of binary files
	if(!mapBinaryFiles.empty())
	{
		mapBinaryFiles.clear();
	}
}

int TrafficLoader::GenerateIpsList()
{
try
{
  	string sCommand;
  	sCommand.append("./Parse ");
  	sCommand.append((char *)sDumpFile.c_str());
  	sCommand.append(" -a GenerateIpList");
  	if(system((char*)sCommand.c_str()))
	  	throw NspException(" Error occured while executing the system() function.",__FILE__,__LINE__);
}

catch(exception &e)
{
	cout<<e.what()<<endl;
}
}

int TrafficLoader::ParseIpsBasedOnClass(string & IpsListFileName)
{
try
{
  	if(IsReadable(IpsListFileName))
  	{
   	 	string sCommand;
    	 	sCommand.append("./Parse ");
    		sCommand.append((char *)IpsListFileName.c_str());
    		sCommand.append(" -a ClassifyIps");
    		return system((char*)sCommand.c_str());
  	}else {
    		throw NspException(" Error occured while trying to read a file.",__FILE__,__LINE__);
		}
}

catch(exception &e)
{
	cout<<e.what()<<endl;
}
}

bool TrafficLoader::IsReadable( const string & file ) 
{  
try
{
	if(file.length()==0) throw NspException("Invalid file path.",__FILE__,__LINE__);
	ifstream fichier(file.c_str()); 
    	return !fichier.fail(); 
}

catch(exception &e)
{
	cout <<"File: "<<file<<endl;
	cout<<e.what()<<endl;
}
}

bool TrafficLoader::IsItValid(string & sDataRecord)
{
try
{
  	string tmp;
  	if(sDataRecord.length() == 0)
    		throw NspException("Invalid data record.",__FILE__,__LINE__);
  	size_t pos = sDataRecord.find_last_of(':');
	tmp = sDataRecord.substr(pos+2, 1);
	if(tmp.length() == 0)
		throw NspException("Error while parsing the data record.", __FILE__, __LINE__);
	if(tmp.compare(string("F")) == 0 || tmp.compare(string("I")) == 0 || tmp.compare(string("U")) == 0)
		return true;
	else return false;
}

catch(exception &e)
{
	cout<<e.what()<<endl;
	cout<<__FILE__<<":"<<__LINE__<<" Data record: "<<sDataRecord.c_str()<<endl;
	return false;
}
}

// Return The data record Data
string TrafficLoader::GetDataRecordProtocol(string & sDataRecord)
{
try
{
  	string tmp;
  	if(sDataRecord.length()==0)
    		throw NspException("Invalid data record.",__FILE__,__LINE__);
  	size_t pos = sDataRecord.find_last_of(':');
	tmp = sDataRecord.substr(pos+2);
	pos = tmp.find_first_of(' ');
	string testUdp = tmp.substr(0,1);
	if(pos!=string::npos)
    	{
		tmp = tmp.substr(0, pos);
		if(testUdp.compare("U") == 0)
		{
			pos = tmp.find_last_of(',');
			tmp = tmp.substr(0, pos);
		}
		if(tmp.length()==0) throw NspException("GetDataRecordDestIp: error in the parsing result.",__FILE__,__LINE__);
		if(testUdp.compare("U") == 0 || testUdp.compare("F") == 0 || testUdp.compare("I") == 0 )
			{return tmp;}
		else {return string("NotFound");}
    	}
  	else throw NspException("Invalid data record.",__FILE__,__LINE__);
}

catch(exception &e)
{
	cout<<e.what()<<endl;
	cout<<__FILE__<<":"<<__LINE__<<" Data record: "<<sDataRecord.c_str()<<endl;
}
}
// Return th e packet size from the data record
long int TrafficLoader::GetPacketSizeFromTheDataRecord(string &sDataRecord)
{
try
{
  	if(sDataRecord.length()==0)
    		throw NspException("Invalid data record.",__FILE__,__LINE__);
  	size_t pos=sDataRecord.find_last_of(' ');
  	if(pos!=string::npos)
  	{
		long int tmp=-1;
		tmp=atol(sDataRecord.substr(pos+1).c_str());
		if(tmp<0) throw NspException("GetPacketSizeFromTheDataRecord: error in the parsing result.",__FILE__,__LINE__);
		return tmp;
  	}else   throw NspException("Invalid data record.",__FILE__,__LINE__);
}
catch(exception &e)
{
	cout<<e.what()<<endl;
	cout<<__FILE__<<":"<<__LINE__<<" Data record: "<<sDataRecord.c_str()<<endl;
}
}
// Returns the data record time stamp (usec)
long int TrafficLoader::GetPacketTimeStampFromTheDataRecord(string &sDataRecord)
{
try
{	
	long int tmp=-1;
  	if(sDataRecord.length()==0)
    		throw NspException("Invalid data record.",__FILE__,__LINE__);
  	size_t pos1=sDataRecord.find_first_of(' ');
	string strTmp;

	strTmp = sDataRecord.substr(0,pos1);
	size_t pos2 = strTmp.find_last_of('.');
	if(pos1 != string::npos && pos2 != string::npos)
	{
		tmp=atol(strTmp.substr(pos2+1).c_str());
		if(tmp<0) throw NspException("GetPacketTimeStampFromTheDataRecord: error in the parsing result.",__FILE__,__LINE__);
		return tmp;
    	}
  	else throw NspException("Invalid data record.",__FILE__,__LINE__);
}
catch(exception &e)
{
	cout<<e.what()<<endl;
	cout<<__FILE__<<":"<<__LINE__<<" Data record: "<<sDataRecord.c_str()<<endl;
}
}

// Returns the data record time stamp (sec)
long int TrafficLoader::GetPacketTimeStampSecondFromTheDataRecord(string &sDataRecord)
{
try
{
  	long int tmp=-1;
	string strTmp;
  	if(sDataRecord.length()==0)
    		throw NspException("Invalid data record.",__FILE__,__LINE__);
  	strTmp = sDataRecord.substr(0,sDataRecord.find_first_of(' '));
	strTmp = strTmp.substr(0,strTmp.find_last_of('.'));
	size_t pos = strTmp.find_last_of(':');
  	if(pos!=string::npos)
    	{	
		tmp=atol(strTmp.substr(pos+1).c_str());
		if(tmp<0) throw NspException("GetPacketTimeStampSecondFromTheDataRecord: error in the parsing result.",__FILE__,__LINE__);
		return tmp;
    	}
  	else throw NspException("Invalid data record.",__FILE__,__LINE__);
}
catch(exception &e)
{
	cout<<e.what()<<endl;
	cout<<__FILE__<<":"<<__LINE__<<" Data record: "<<sDataRecord.c_str()<<endl;
}
}

// Returns the data record destination Ip @
string TrafficLoader::GetDataRecordSourceIp(string & sDataRecord)
{
try
{
  	string tmp;
  	if(sDataRecord.length()==0)
    		throw NspException("Invalid data record.",__FILE__,__LINE__);
  	size_t pos=sDataRecord.find_first_of('>');
	tmp = sDataRecord.substr(0, pos);
	pos = tmp.find_last_of(' ');
	tmp = tmp.substr(0, pos-1);
	pos = tmp.find_last_of('.');
	tmp = tmp.substr(0, pos);
	size_t pos2 = tmp.find_last_of(' ');
	
  	if(pos!=string::npos)
    	{
		tmp=tmp.substr(pos2+1);
		if(tmp.length()==0) throw NspException("GetDataRecordSourceIp: error in the parsing result.",__FILE__,__LINE__);
		return tmp;
    	}
  	else  throw NspException("Invalid data record.",__FILE__,__LINE__);

}

catch(exception &e)
{
	cout<<e.what()<<endl;
	cout<<__FILE__<<":"<<__LINE__<<" Data record: "<<sDataRecord.c_str()<<endl;
}
}

// Returns the data record destination Ip @
string TrafficLoader::GetDataRecordDestIp(string & sDataRecord)
{
try
{
  	string tmp;
  	if(sDataRecord.length()==0)
    		throw NspException("Invalid data record.",__FILE__,__LINE__);
  	size_t pos = sDataRecord.find_last_of(':');
	tmp = sDataRecord.substr(0, pos-1);
	pos = tmp.find_last_of('.');
	tmp = tmp.substr(0, pos);
	pos = tmp.find_last_of('>');
	
  	if(pos!=string::npos)
    	{
		tmp = tmp.substr(pos+2);
		if(tmp.length()==0) throw NspException("GetDataRecordDestIp: error in the parsing result.",__FILE__,__LINE__);
		return tmp;
    	}
  	else throw NspException("Invalid data record.",__FILE__,__LINE__);
}

catch(exception &e)
{
	cout<<e.what()<<endl;
	cout<<__FILE__<<":"<<__LINE__<<" Data record: "<<sDataRecord.c_str()<<endl;
}
}

// Returns the data record destination port
string TrafficLoader::GetDataRecordSourcePort(string & sDataRecord)
{
try
{
	string tmp;
  	if(sDataRecord.length() == 0)
    		throw NspException("Invalid data record.",__FILE__,__LINE__);
  	size_t pos=sDataRecord.find_first_of('>');
	tmp = sDataRecord.substr(0, pos-1);
	pos = tmp.find_last_of('.');
	
  	if(pos!=string::npos)
    	{
		tmp=tmp.substr(pos+1);
		if(tmp.length()==0) throw NspException("GetDataRecordSourceIp: error in the parsing result.",__FILE__,__LINE__);
		return tmp;
    	}
  	else  throw NspException("Invalid data record.",__FILE__,__LINE__);
}

catch(exception &e)
{
	cout<<e.what()<<endl;
	cout<<__FILE__<<":"<<__LINE__<<" Data record: "<<sDataRecord.c_str()<<endl;
}
}

// Returns the data record source port
string TrafficLoader::GetDataRecordDestPort(string & sDataRecord)
{
try
{
  	string tmp;
  	if(sDataRecord.length()==0)
    		throw NspException("Invalid data record.",__FILE__,__LINE__);
  	size_t pos=sDataRecord.find_last_of(':');
	tmp = sDataRecord.substr(0, pos);
	pos = tmp.find_last_of('.');
  	if(pos!=string::npos)
    	{
		
		tmp = tmp.substr(pos+1);
		if(tmp.length()==0) throw NspException("GetDataRecordDestIp: error in the parsing result.",__FILE__,__LINE__);
		return tmp;
    	}
  	else throw NspException("Invalid data record.",__FILE__,__LINE__);
}

catch(exception &e)
{
	cout<<e.what()<<endl;
	cout<<__FILE__<<":"<<__LINE__<<" Data record: "<<sDataRecord.c_str()<<endl;
}
}

// verify if the given data record represents a fragment
bool TrafficLoader::IsFragment(string & sDataRecord)
{
try
{
	if(GetDataRecordProtocol(sDataRecord).compare("Flags") == 0)
	{
  		string tmp;
  		if(sDataRecord.length()==0)
    			throw NspException("Invalid data record.",__FILE__,__LINE__);
  		size_t pos=sDataRecord.find_first_of('[');
		tmp = sDataRecord.substr(pos+1);
		pos = tmp.find_first_of('.');
		tmp = tmp.substr(0, 1);
  		
		if(tmp.compare("F") == 0)
		{
			return true;
		} else return false;
	} else return false;
}

catch(exception &e)
{
	cout<<e.what()<<endl;
	cout<<__FILE__<<":"<<__LINE__<<" Data record: "<<sDataRecord.c_str()<<endl;
}
}

// Returns the sequence number of a fragemnt
unsigned long int TrafficLoader::GetFragmentSeqNumber(string & sDataRecord)
{
try
{
	if(IsFragment(sDataRecord))
	{
  		string tmp;
  		if(sDataRecord.length()==0)
    			throw NspException("Invalid data record.",__FILE__,__LINE__);
  		size_t pos=sDataRecord.find_first_of(',');
		tmp = sDataRecord.substr(pos+2); //s
		tmp = tmp.substr(tmp.find_first_of(' ')+1);
		pos = tmp.find_first_of(',');
		tmp = tmp.substr(0, pos);
		unsigned long int seq = atol(tmp.c_str());
		if(tmp.length() == 0 )
			{throw NspException("Error occured while parsing the data record.",__FILE__,__LINE__);}
		else
			return seq;
  		
	} 
}

catch(exception &e)
{
	cout<<e.what()<<endl;
	cout<<__FILE__<<":"<<__LINE__<<" Data record: "<<sDataRecord.c_str()<<endl;
}
}

// Pcap base API
// runs the interface thread.

void TrafficLoader::StartTheInterfaceThreadF(string &ap, SamplingPlatform *sp)
{
	
	try
	{
		cout << "Playing the binary file: "<<ap<<endl;	
		TlThreadParam * ptrParam= (TlThreadParam*)malloc(sizeof(TlThreadParam));
	    	if(ptrParam==NULL)
			throw NspException("Memory allocation problem.",__FILE__,__LINE__);
		ptrParam->sp = sp;
		strcpy(ptrParam->absolutePath, ap.c_str());
		ptrParam->tl = this;

		pthread_t loadingThread;
		pthread_attr_t threadAttr;

		if (pthread_attr_init (&threadAttr) != 0) 
		{
			throw NspException("pthread_attr_init error.",__FILE__,__LINE__);
		}

		if (pthread_attr_setdetachstate (&threadAttr,PTHREAD_CREATE_DETACHED) != 0) 
		{
			throw NspException("pthread_attr_setdetachstate error.",__FILE__,__LINE__);
		}
		
		if (pthread_create (&loadingThread, &threadAttr, LoadTcpDumpBinaryFile, (void *)ptrParam) < 0) 
		{
			throw NspException("pthread_create error.",__FILE__,__LINE__);
		}
	}
	catch(exception &e)
	{
		cout<<e.what()<<endl;
	}
}

void TrafficLoader::StartTheInterfaceThread(list<string> &ap, SamplingPlatform *sp)
{
	
	try
	{
		list<string>::iterator iter = ap.begin(); 
		string firstFile;
		firstFile.assign(*iter);
		currentFile = 0;
		int i = currentFile;
		while(iter != ap.end())
		{
			mapBinaryFiles[i] = *iter;
			iter++;
			i++;
		}

		cout << "Playing the binary file: "<<firstFile<<endl;	

		TlThreadParam * ptrParam= (TlThreadParam*)malloc(sizeof(TlThreadParam));
	    	if(ptrParam==NULL)
			throw NspException("Memory allocation problem.",__FILE__,__LINE__);
		ptrParam->sp = sp;
		strcpy(ptrParam->absolutePath, firstFile.c_str());
		ptrParam->tl = this;

		pthread_t loadingThread;
		pthread_attr_t threadAttr;

		if (pthread_attr_init (&threadAttr) != 0) 
		{
			throw NspException("pthread_attr_init error.",__FILE__,__LINE__);
		}

		if (pthread_attr_setdetachstate (&threadAttr,PTHREAD_CREATE_DETACHED) != 0) 
		{
			throw NspException("pthread_attr_setdetachstate error.",__FILE__,__LINE__);
		}
		
		if (pthread_create (&loadingThread, &threadAttr, LoadTcpDumpBinaryFile, (void *)ptrParam) < 0) 
		{
			throw NspException("pthread_create error.",__FILE__,__LINE__);
		}
	}
	catch(exception &e)
	{
		cout<<e.what()<<endl;
	}
}

void * TrafficLoader::LoadTcpDumpBinaryFile(void * arg)
{
try
{
	TlThreadParam * ptrParam= (TlThreadParam*)arg;
	
	if(!ptrParam->tl->IsReadable(ptrParam->absolutePath))
		throw NspException("Error occured while trying to open the tcpdump binary file.",__FILE__,__LINE__);
	int r;
	int max_packets = MAX_TL_LOOP_PACKETS;
	
	// Setup the pcap functions
	struct CB_CTXT cb_ctxt;
	int linktype;
	int need_v6 = 0;
	pcap_t *pcap = NULL;
	char capfile[100];
	strcpy(capfile, ptrParam->absolutePath);
	char *bpf_prog = NULL;
	
	ptrParam->tl->SetupPacketCapture(&pcap, &linktype, capfile, bpf_prog, 0);
	
	memset(&cb_ctxt, '\0', sizeof(cb_ctxt));
	cb_ctxt.linktype = linktype;
	cb_ctxt.want_v6 = need_v6;
	cb_ctxt.sp = ptrParam->sp;
	cb_ctxt.fatal = 0;
	cb_ctxt.stopCollecting = false;
	cb_ctxt.tl = ptrParam->tl;

	while(!cb_ctxt.stopCollecting)
	{
		if(cb_ctxt.sp->DataNeeded())
		{
			
			cb_ctxt.sp->GetSendingSem();
			cb_ctxt.sp->SetLoadingStartTime();
			cb_ctxt.sp->SetLoading(true);	
			while(cb_ctxt.sp->DataNeeded())
			{
				r = pcap_dispatch(pcap, max_packets, SamplingPlatform::PerPacketCallBackFunction, (u_char *)&cb_ctxt);
				if (r == -1)
				{
					throw NspException("Exiting on pcap_dispatch.",__FILE__, __LINE__);
					pcap_geterr(pcap);
				} else if (r == 0)
				{
					//Shutting down after pcap EOF
					cout <<"pcap EOF received, shutting down!"<<endl;
					break;
				}
			}
			cb_ctxt.sp->SetLoadingEndTime();
			cb_ctxt.sp->ReleaseSendingSem();
			if (r == 0 && !ptrParam->tl->AnotherDataBinaryFile()) 
			{
				ptrParam->tl->TraficLoadingEnded();
				break;
			}else if(r == 0)
			{
				// Start The next file
				ptrParam->tl->StartTheInterfaceThreadF(ptrParam->tl->GetTheNextBinaryFile(), cb_ctxt.sp);
				usleep(1);
				break;
			}
		}
	}

	if(ptrParam != NULL)
		free(ptrParam);
	pthread_exit (0);
}
catch(exception &e)
{
	cout << e.what()<<endl;
}
}

void TrafficLoader::SetupPacketCapture(struct pcap **pcap, int *linktype, char *capfile, char *bpf_prog, int need_v6)
{
try
{
	char ebuf[PCAP_ERRBUF_SIZE];
	struct bpf_program prog_c;
	u_int32_t bpf_mask, bpf_net;

	/* Open pcap */
	 if(capfile != NULL) 
	{
		if ((*pcap = pcap_open_offline(capfile, ebuf)) == NULL) 
		{
			throw NspException("pcap_open_offline.",__FILE__, __LINE__);
		}
		bpf_net = bpf_mask = 0;
	}

	*linktype = pcap_datalink(*pcap);
	if (DatalinkCheck(*linktype, NULL, 0, NULL) == -1) 
	{
		throw NspException("Unsupported datalink type.",__FILE__, __LINE__);
	}

	/* Attach BPF filter, if specified */
	if (bpf_prog != NULL) 
	{
		if (pcap_compile(*pcap, &prog_c, bpf_prog, 1, bpf_mask) == -1) 
		{
			throw NspException("pcap_compile.",__FILE__,__LINE__);
		}
		if (pcap_setfilter(*pcap, &prog_c) == -1) 
		{
			throw NspException("pcap_setfilter.",__FILE__, __LINE__);
		}
	}

}

catch(exception &e)
{
	cout<<e.what()<<endl;
}
}



/*
 * Main per-packet processing function. Take a packet (provided by 
 * libpcap) and attempt to find a matching flow. If no such flow exists, 
 * then create one. 
 *
 * Also marks flows for fast expiry, based on flow or packet attributes
 * (the actual expiry is performed elsewhere)
 */

int TrafficLoader::ProcessPacket(const u_int8_t *pkt, int af, const u_int32_t caplen, const u_int32_t len, const struct timeval *received_time, packet * currentPacket)
{
try
{
	int frag;

	/* Convert the IP packet to a flow identity */
	switch (af) {
	case AF_INET:
		currentPacket->ipv4 = true;
		currentPacket->ipv6 = false;
		if (Ipv4ToUpLevel(pkt, caplen, len, &frag, af, currentPacket) == -1)
			goto bad;
		break;
	case AF_INET6:
		currentPacket->ipv4 = false;
		currentPacket->ipv6 = true;

		if (Ipv6ToUpLevel(pkt, caplen, len, &frag, af, currentPacket) == -1)
			goto bad;
		break;
	default:
 bad: 
		return (PP_BAD_PACKET);
	}

	
	if (frag)
	{
		currentPacket->fragment = true;
	}	

	return (PP_OK);
}

catch(exception &e)
{
	cout <<e.what()<<endl;	
}
}



/*
 * Figure out how many bytes to skip from front of packet to get past 
 * datalink headers. If pkt is specified, also check whether determine
 * whether or not it is one that we are interested in (IPv4 or IPv6 for now)
 *
 * Returns number of bytes to skip or -1 to indicate that entire 
 * packet should be skipped
 */
int TrafficLoader:: DatalinkCheck(int linktype, const u_int8_t *pkt, u_int32_t caplen, int *af)
{
	int i, j;
	u_int32_t frametype;
	static const struct DATALINK *dl = NULL;

	/* Try to cache last used linktype */
	if (dl == NULL || dl->dlt != linktype) {
		for (i = 0; lt[i].dlt != linktype && lt[i].dlt != -1; i++)
			;
		dl = &lt[i];
	}
	if (dl->dlt == -1 || pkt == NULL)
	{
		return (dl->dlt);
	}
	
	if (caplen <= dl->skiplen )
	{
		// Skipping the entire packet
		cout<<__FILE__<<":"<<__LINE__<<endl;
		return (-1);
	}

	/* Suck out the frametype */
	frametype = 0;
	if (dl->ft_is_be) {
		for (j = 0; j < dl->ft_len; j++) {
			frametype <<= 8;
			frametype |= pkt[j + dl->ft_off];
		}
	} else {
		for (j = dl->ft_len - 1; j >= 0 ; j--) {
			frametype <<= 8;
			frametype |= pkt[j + dl->ft_off];
		}
	}
	frametype &= dl->ft_mask;

	if (frametype == dl->ft_v4)
		*af = AF_INET;
	else if (frametype == dl->ft_v6)
		*af = AF_INET6;
	else
		{cout<<__FILE__<<":"<<__LINE__<<endl;return (-1);}
	
	return (dl->skiplen);
}




/* Convert a IPv4 packet to a partial flow record (used for comparison) */
int TrafficLoader:: Ipv4ToUpLevel(const u_int8_t *pkt, size_t caplen, size_t len, int *isfrag, int af, packet * currentPacket)
{
try
{
	const struct ip *ip = (const struct ip *)pkt;
	int ndx;

	if (caplen < 20 || caplen < ip->ip_hl * 4)
		return (-1);	/* Runt packet */
	if (ip->ip_v != 4)
		return (-1);	/* Unsupported IP version */
	
	currentPacket->lLength = ntohs(ip->ip_len);

	strcpy(currentPacket->szIpSource, inet_ntoa(ip->ip_src));
	strcpy(currentPacket->szIpDest, inet_ntoa(ip->ip_dst));
	
	*isfrag = (ntohs(ip->ip_off) & (IP_OFFMASK|IP_MF)) ? 1 : 0;

	if (*isfrag && (ntohs(ip->ip_off) & IP_OFFMASK) != 0)
	{
		currentPacket->fragment = true;
		currentPacket->fragmentSeqNumber = ntohs(ip->ip_off);
	}
	
	return (TransportToUpLevel(pkt + (ip->ip_hl * 4), 
	    caplen - (ip->ip_hl * 4), *isfrag, ip->ip_p, ndx, currentPacket));
}
catch(exception &e)
{
	cout<<e.what()<<endl;
}
}

/* Convert a IPv6 packet to a partial flow record (used for comparison) */
int TrafficLoader::Ipv6ToUpLevel(const u_int8_t *pkt, size_t caplen, size_t len, int *isfrag, int af, packet * currentPacket)
{
try
{
	const struct ip6_hdr *ip6 = (const struct ip6_hdr *)pkt;
	const struct ip6_ext *eh6;
	const struct ip6_frag *fh6;
	int ndx, nxt;

	if (caplen < sizeof(*ip6))
	{
		throw NspException("Invalid ipv6 packet.",__FILE__, __LINE__);
	}

	if ((ip6->ip6_vfc & IPV6_VERSION_MASK) != IPV6_VERSION)
	{
		throw NspException("Unsupported Ipv6 version.",__FILE__, __LINE__);
	}	

	ndx = memcmp(&ip6->ip6_src, &ip6->ip6_dst,
	    sizeof(ip6->ip6_src)) > 0 ? 1 : 0;
	
	
	*isfrag = 0;
	nxt = ip6->ip6_nxt;
	pkt += sizeof(*ip6);
	caplen -= sizeof(*ip6);

	/* Now loop through headers, looking for transport header */
	for (;;) {
		eh6 = (const struct ip6_ext *)pkt;
		if (nxt == IPPROTO_HOPOPTS || 
		    nxt == IPPROTO_ROUTING || 
		    nxt == IPPROTO_DSTOPTS) {
			if (caplen < sizeof(*eh6) ||
			    caplen < (eh6->ip6e_len + 1) << 3)
				return (1); /* Runt */
			nxt = eh6->ip6e_nxt;
			pkt += (eh6->ip6e_len + 1) << 3;
			caplen -= (eh6->ip6e_len + 1) << 3;
		} else if (nxt == IPPROTO_FRAGMENT) {
			*isfrag = 1;
			fh6 = (const struct ip6_frag *)eh6;
			if (caplen < sizeof(*fh6))
				return (1); /* Runt */
			/*
			 * Don't try to examine higher level headers if 
			 * not first fragment
			 */
			if ((fh6->ip6f_offlg & IP6F_OFF_MASK) != 0)
				return (0);
			nxt = fh6->ip6f_nxt;
			pkt += sizeof(*fh6);
			caplen -= sizeof(*fh6);
		} else 
			break;
	}

	return (TransportToUpLevel(pkt, caplen, *isfrag, nxt, ndx, currentPacket));
}
catch(exception &e)
{
	cout <<e.what()<<endl;
}
}


/* Fill in transport-layer (tcp/udp) portions of packet record */
int TrafficLoader::TransportToUpLevel(const u_int8_t *pkt, const size_t caplen, int isfrag, int protocol, int ndx, packet * currentPacket)
{
try
{
	const struct tcphdr *tcp = (const struct tcphdr *)pkt;
	const struct udphdr *udp = (const struct udphdr *)pkt;
	const struct icmp *icmp = (const struct icmp *)pkt;

	char tmp[20];
	switch (protocol) {
	case IPPROTO_TCP:
		/* Check for runt packet, but don't error out on short frags */
		strcpy(currentPacket->szProtocol, "TCP");
		memset(tmp, '\0', 20);
		sprintf(tmp,"%ld", tcp->th_sport);
		strcpy(currentPacket->szSourcePort, tmp);
		memset(tmp, '\0', 20);
		sprintf(tmp,"%ld", tcp->th_dport);
		strcpy(currentPacket->szDestPort, tmp);
		currentPacket->tcpFlags = tcp->th_flags;
		
		if (caplen < sizeof(*tcp))
			return (isfrag ? 0 : 1);
		break;
	case IPPROTO_UDP:
		
		strcpy(currentPacket->szProtocol, "UDP");
		memset(tmp, '\0', 20);
		sprintf(tmp,"%ld", udp->uh_sport);
		strcpy(currentPacket->szSourcePort, tmp);
		memset(tmp, '\0', 20);
		sprintf(tmp,"%ld", udp->uh_dport);
		strcpy(currentPacket->szDestPort, tmp);
		
		/* Check for runt packet, but don't error out on short frags */
		if (caplen < sizeof(*udp))
			return (isfrag ? 0 : 1);
		break;
	case IPPROTO_ICMP:
		/*
		 * Encode ICMP type * 256 + code into dest port like
		 * Cisco routers
		 */
		strcpy(currentPacket->szProtocol, "ICMP");
		break;
	default:
		memset(tmp, '\0', 20);
		sprintf(tmp, "%i", protocol);
		strcpy(currentPacket->szProtocol, "UNKNOWN: ");
		strcat(currentPacket->szProtocol, tmp);
	}
	return (0);
}
catch(exception &e)
{
	cout << e.what() << endl;
}
}


}
