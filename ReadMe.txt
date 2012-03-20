MonLab: an Emulation Platform for Network Wide Traffic Sampling and Monitoring 

Project Web Site: http://planete.inria.fr/MonLab/


Introduction
-------------

It is of utmost importance for the network research community to have access to tools and testbeds 
to explore future directions for Internet traffic monitoring and engineering. 

MonLab is an emulation platform for network wide traffic monitoring that provides an answer to the 
limitations of the existing experimental solutions in terms of network wide sampling and monitoring at the flow level. 

MonLab presents a new approach for the emulation of Internet traffic and for its monitoring across
the different routers. Through MonLab, we put at the disposal of users a real traffic emulation 
service coupled to a set of libraries and tools capable of Cisco NetFlow data export and collection, 
the overall destined to run advanced applications for network wide traffic monitoring and optimization.


MonLab Main Components
-----------------------

NSP(Networking Sampling Platform)
---------------------------------

NSP provides means to describe a given network topology. The emulated network is fed with a real 
traffic captured on some high speed link in a backbone transit network, before being dispatched 
and played over the emulated topology. We provide a highly flexible configuration methodology via 
XML files through which users can describe their emulated network. 

In particular,  

* AsConfiguration.xml: describes the different stub ASes and their associated weights, 

* TopologyFile.xml: describes the list of routers (interfaces, IP addresses) and the set of 
characteristics of the different links as well as the list of monitors deployed inside routers. 

* BinaryFiles.xml: describes the list of TcpDump files to be replayed.


Once the emulated topology is available, NSP looks for IP prefixes within the available TcpDump 
traces and dispatches them over the stub ASes according to their weights. The packets of these 
traces are then played over the routers of the topology respecting their timestamps to reproduce 
the network wide packet level traffic we are looking for.


Building NSP: You’ll need just to run make.


Softflowd
----------
Softflowd listens promiscuously on a network interface and semi-statefully
tracks network flows. We’ve enhanced softflowd first version(http://www.mindrot.org/projects/softflowd/) 
so it can be connected an run either on top of a real router or an emulated one within our Network Sampling Platform (NSP). 
Flows collected within softflowd can be reported using NetFlow version 1, 5 
or 9 datagrams. SoftFlowd is fully IPv6 capable: it can track IPv6 flows and 
export to IPv6 hosts.


More details about softflowd's function and usage may be found in the
supplied manpages, which you can view prior to installation using

/usr/bin/nroff -c -mandoc softflowd.8 | less
/usr/bin/nroff -c -mandoc softflowctl.8 | less

Building softflowd should be as simple as typing:

./configure
make
make install


Flowd
------
Is a secure NetFlow collector. It offers the following features:

* Understanding NetFlow protocol v.1, v.5, v.7 and v.9 (including Ipv6 flows).
* Supports both Ipv4 and Ipv6 transport of flows.
* Supports filtering and tagging of flows, using a packet filter-like syntax.
* Stores recorded flow data in a compact binary format.

Flowd works with any standard NetFlow exporter, including hardware devices or software 
tracking agents, such as Softflowd. Note that original version of Flowd does not include 
support for performing data analysis and as we have done for Softflowd, we’ve enhanced 
Flowd with new modules for data analysis and remote control of the sampling rate of emulated 
routers within NSP.

Building Flowd should be as simple as typing:

./configure
make
make install


Usage Scenario
--------------

Note that MonLab components can run either within the same machine or in three different ones. 
Such choice depends mainly from the machines available resources and the scenario you’d like to run. 

Once you’ve specified the list of AS(s) to connect and their characteristics within the 
AsConfiguration.xml file. And specified the characteristics of the transit network which 
should connect the later AS(s), you cam start MonLab components as follows:

You first start the Flowd service; let’s say within the machine paierie.inria.fr. You have 
also to specify both the port number on which Flowd will listen for the incoming NetFlow 
reports from SoftFlowd (12345 in this case) and the trusted IP(s) (sim2.inria.fr in this case)
from which Flowd is supposed to receive NetFlow reports (Where Softflowd services are running). 
All the later parameters should be specified within the flowd.conf file. 

Then, you can start Flowd by simply typing

./flowd -d -g -f ./flowd.conf 


Once done, you can start the Softflowd service at the second machine (sim2.inria.fr). For that 
you should also specify the Ip:PortNumber where Flowd is running (Softflowd sends the NetFlow 
reports to that address), the port number at which Softflowd should listen for incoming sampled 
packets from NSP (You should use –e and in this case the port number is 2618) and finally the port 
number at which SoftFlowd should listen for incoming sampling rates changing requests ( sent from 
Flowd once some decisions are taken), for that you have to use (-s portNumber, 2617 in this case)  

Running SoftFlowd: ./softflowd -D -d -n paierie.inria.fr:12345 -e 2618 -s 2617  


And as a last step, you have to start the NSP emulator. For that you have to specify the path to the 
TopologyFile.xml file, to the AsConfiguration.xml file and finally to the BinaryFiles.xml one. You 
have also to specify the Ip address and port number at which Softflowd is running (sim2.inria.fr:2618 in this case).
 
Runnig the NSP: ./SPlatform -t ./TopologyFile.xml -a ./AsConfiguration.xml -b ./BinaryFiles.xml -s sim2.inria.fr -p 2618 


An exhaustive list of the features proposed by the MonLab package is dressed in: http://planete.inria.fr/MonLab/


Reporting Bugs
--------------

Please refer to http://planete.inria.fr/MonLab/ for the mailing list to report bugs in any component 
of MonLab. 


Contributing
------------

MonLab has an extensive TODO list of interesting features that are waiting to be implemented. If you 
are interested in helping, please contact us.


