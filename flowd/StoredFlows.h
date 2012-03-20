#ifndef _STOREDFLOWS_H
#define _STOREDFLOWS_H


#include <stdio.h>
#include "store.h"
#include "addr.h"



struct Flow {

	struct xaddr		agent_addr;
	struct xaddr		src_addr;
	struct xaddr		dst_addr;
	struct xaddr		gateway_addr;
	u_int16_t		src_port;
	u_int16_t		dst_port;
	u_int64_t		flow_packets;
	u_int64_t		flow_octets;
	u_int8_t		protocol;
};



struct Flows_list {
int used;
struct Flow *sflow; 
struct Flows_list *pNext;
};


struct calcul_list{
u_int16_t estimated_byteNumber;
u_int16_t estimated_packetNumber;
float variance;
float sRate;
struct calcul_list * next;
};



struct result_list{
int id;
float byteNumber;
float packetNumber;
float flowVariance;
struct result_list * next;
};




//void estimate(struct calcul_list * liste);




void Add_Flow(struct Flows_list ** flows_list, struct Flow *flow);
void liberer(struct Flows_list **liste);



void Add_calcul(struct calcul_list ** calcul, struct Flow * flow);
void liberer_calcul(struct calcul_list **liste);

void Add_result(struct result_list ** result, struct calcul_list * calcul);
void liberer_result(struct result_list **liste);

#endif