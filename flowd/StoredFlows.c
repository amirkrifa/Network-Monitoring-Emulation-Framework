#include <stdlib.h>
#include "StoredFlows.h"

void Add_Flow(struct Flows_list **Tete, struct Flow *flow)
 {
 
 	struct Flows_list *pCourant;
 	struct Flows_list *pNouveau;
 	pNouveau = (struct Flows_list *)malloc(sizeof(struct Flows_list));
 	pNouveau->sflow = flow;
	pNouveau->used = 0;
 	pNouveau->pNext=NULL;
 	if(*Tete != NULL)
 	{
 		pCourant = *Tete;
 		while(pCourant->pNext != NULL)
 			pCourant = pCourant->pNext;
 		pCourant->pNext=pNouveau;
 	}
 	else
 		*Tete = pNouveau;
 	
 	
 		
 
 }


void liberer(struct Flows_list **liste)
{
	struct Flows_list * pCourant;
	if(*liste != NULL)
	{
		pCourant = *liste;
		while(pCourant->pNext != NULL)
		{
			*liste = pCourant->pNext;
			free(pCourant);
			pCourant = *liste;
		}
	}
	free(*liste);
}


void Add_result(struct result_list **Tete, struct calcul_list * calcul)
{
	struct result_list *pCourant;
	struct result_list *pNouveau;
	pNouveau = (struct result_list *)malloc(sizeof(struct result_list));
	
	struct calcul_list * cl;
	float trans1 = 0;
	float trans2 = 0;
	float packet = 0;
	float varianc = 0;
	float byte = 0;
	float gPacket = 0;
	if (calcul == NULL)
	{
		printf("la liste calcul est nulle");
	}
	else
	{
		cl = calcul;
		while(cl-> next != NULL)
		{
			trans1 += 1 / cl->variance;
			trans2 += (cl->sRate / (1 - cl->sRate));
			cl = cl->next;	
		}
		trans1 += 1 / cl->variance;
		trans2 += ((cl->sRate) / (1 - (cl->sRate)));
		
		cl = calcul;

		while(cl->next != NULL)
		{
			packet += ((cl->estimated_packetNumber) / (cl->variance));
		}
		packet += ((cl->estimated_packetNumber) / (cl->variance));
		gPacket = packet / trans1;
		varianc = gPacket / trans2;
	}



	pNouveau->packetNumber = gPacket;
	pNouveau->flowVariance = varianc;
	pNouveau->byteNumber = 0;
	pNouveau->next=NULL;
	if(*Tete != NULL)
	{
		pCourant = *Tete;
		while(pCourant->next != NULL)
			pCourant = pCourant->next;
		pCourant->next=pNouveau;
	}
	else
		*Tete = pNouveau;
}

void liberer_result(struct result_list ** liste)
{
	struct result_list * pCourant;
	if(*liste != NULL)
	{
		pCourant = *liste;
		while(pCourant->next != NULL)
		{
			*liste = pCourant->next;
			free(pCourant);
			pCourant = *liste;
		}
	}
	free(*liste);
}




void Add_calcul(struct calcul_list **Tete, struct Flow *flow)
{
	int octet = 0;
	int packet = 0;
	float p = 0.01;
	float varianc;
	struct calcul_list *pCourant;
	struct calcul_list *pNouveau;
	pNouveau = (struct calcul_list *)malloc(sizeof(struct calcul_list));
	octet = flow->flow_octets / p;
	packet = flow->flow_packets / p;
	varianc = (packet * (1-p)) / p;

	pNouveau->estimated_byteNumber = octet;
	pNouveau->estimated_packetNumber = packet;
	pNouveau->variance = varianc;
	pNouveau->sRate = p;
	pNouveau->next=NULL;

	if(*Tete != NULL)
	{
		pCourant = *Tete;
		while(pCourant->next != NULL)
			pCourant = pCourant->next;
		pCourant->next=pNouveau;
	}
	else
		*Tete = pNouveau;
}



void liberer_calcul(struct calcul_list ** liste)
{
	struct calcul_list * pCourant;
	if(*liste != NULL)
	{
		pCourant = *liste;
		while(pCourant->next != NULL)
		{
			*liste = pCourant->next;
			free(pCourant);
			pCourant = *liste;
		}
	}
	free(*liste);
}
