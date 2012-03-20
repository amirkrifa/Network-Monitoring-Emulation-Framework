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

#include <stdio.h>
#include <stdlib.h>
#include <semaphore.h>
#include <pthread.h>
#include <string.h>


#include "GlobalEstimation.h"
#include "store.h"
#include "addr.h"
#include "ClientController.h"

float conf_init = 0.005;
u_int64_t sizeval = 0;
int thrd = 0;
int overhead = 0;

int Emulation = 1;
extern int stepSize;
extern int threshold;
sem_t semFlows;
int compteur2 = 0;
int compteur = 0;
pthread_t EstimationThread;
pthread_t CompteurThread;
extern struct un_chemin chemin[NOMBRE_AS][NOMBRE_AS];
// extern struct f_list * estimation_flow_list;
// extern struct f_list * AddFlow;
// struct f_list * PicFlow_End;
u_int64_t taille = 0;



void liberer_observation(struct monitor_observation ** liste)
{
	struct monitor_observation * pCourant;
	if(*liste != NULL)
	{
		pCourant = *liste;
		while(pCourant->next != NULL)
		{
			*liste = pCourant->next;
			free(pCourant);
			pCourant = *liste;
		}
		free(*liste);
	}
}

void liberer_moniteurs()
{
	int k = 0;
	for(k = 0; k < NOMBRE_MONITEURS; k++)
	{
		liberer_observation(&les_moniteurs[k].les_observations);
		les_moniteurs[k].les_observations = NULL;
	}
}


Add_observation(struct f_list *flow, u_int64_t n_packets, double alfa, int rapports)
{
	int find = 0;
	int i = 0;
	int indice = 0;
	do
	{
		if (strcmp(les_moniteurs[i].adresse, flow->agent_addr) == 0)
		{
			find = 1;
			indice = i;
		}
		if(find == 0)
		{
			i++;
		}
	}while((find == 0));// && (i < NOMBRE_MONITEURS));

	if(find == 0)
	{
 		printf("Add_observation: strcmp n a pas trouve l interface ");
	}

	struct monitor_observation *pCourant;
	struct monitor_observation *pNouveau;
	pNouveau = (struct monitor_observation *)malloc(sizeof(struct monitor_observation));
	
	pNouveau->used = 0;
	pNouveau->protocol = flow->protocol;
	pNouveau->agent_addr = flow->agent_addr;
	pNouveau->src_addr = flow->src_addr;
	pNouveau->dst_addr = flow->dst_addr;
	pNouveau->src_port = flow->src_port;
	pNouveau->dst_port = flow->dst_port;

	pNouveau->src_as = flow->src_as;
	pNouveau->dst_as = flow->dst_as;

	pNouveau->s_f = n_packets;
	pNouveau->alpha_f = alfa;
	pNouveau->rapportNumber_f = rapports;
	pNouveau->next = NULL;

	if(les_moniteurs[indice].les_observations != NULL)
	{
		pCourant = les_moniteurs[i].les_observations;
		while(pCourant->next != NULL)
			pCourant = pCourant->next;
		pCourant->next=pNouveau;
	}
	else
	{
		les_moniteurs[i].les_observations = pNouveau;
	}
}

void vider_les_moniteurs()
{
	int l = 0;
	for(l = 0; l < NOMBRE_MONITEURS; l++)
	{
		free(les_moniteurs[l].adresse);
		les_moniteurs[l].adresse = NULL;
	}
}

double samling_rate(char * s)
{
	int find = 0;
	double p = 0;
	int i = 0;
	do
	{
		if (strcmp(les_moniteurs[i].adresse, s) == 0)
		{
			find = 1;
			p = les_moniteurs[i].sampling_rate;
		}
		i++;
	}while((find == 0));// && (i < NOMBRE_MONITEURS));

	if(find == 0)
	{
		printf("sampling rate: strcmp n a pas trouve l interface ");
	}
	return p;
}



void Add_flow(struct flot_node * ff) //struct flot_node * ff
 {
	u_int64_t overhead_threshold = threshold;
	struct f_list *pCourant;
	struct f_list *pNouveau;
	int as1 = 0;
	int as2 = 0;
	as1 = ff->src_as - 1;
	as2 = ff->dst_as - 1;

	if(as1 < 0 || as1 >= NOMBRE_AS || as2 < 0 || as2 >= NOMBRE_AS || ff == NULL)
	{
		printf("^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^le noeud est nul ou as1 est faux ou as2 est faux \n");
	}
	else
	{
	
		pNouveau = (struct f_list *)malloc(sizeof(struct f_list));
		pNouveau->protocol = ff->protocol;
		pNouveau->agent_addr = ff->agent_addr;
		pNouveau->src_addr = ff->src_addr;
		pNouveau->dst_addr = ff->dst_addr;
		pNouveau->gateway_addr = ff->gateway_addr;
		pNouveau->src_port = ff->src_port;
		pNouveau->dst_port = ff->dst_port;

		pNouveau->src_as = ff->src_as;
		pNouveau->dst_as = ff->dst_as;

		

		pNouveau->flow_octets = ff->flow_octets;
		pNouveau->flow_packets = ff->flow_packets;
	
		// 	pNouveau->src_addr.af = AF_INET;
		// 	pNouveau->dst_addr.af = AF_INET;
		// 	pNouveau->gateway_addr.af = AF_INET;
		pNouveau->used = 0;
 		pNouveau->next = NULL;
		
	
		if(chemin[as1][as2].AddFlow != NULL)
		{
			chemin[as1][as2].AddFlow->next = pNouveau;
		}
		else
		{
			chemin[as1][as2].flow_list = pNouveau;
			if(thrd == 0)
			{
				thrd = 1;
  				Start_EstimationThread();
 				Start_CompteurThread();
 				fprintf(stderr," &&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&& \n");
			}
		}
	
		chemin[as1][as2].AddFlow = pNouveau;
		chemin[as1][as2].taille++;
		taille++;

		if(taille >= overhead_threshold)
		{
			overhead = 1;
		}
	}

 }

void liberer_flow()
{
	int i = 0;
	int j = 0;
	for(i = 0; i < NOMBRE_AS; i++)
	{
		for(j = 0; j < NOMBRE_AS; j++)
		{
			liberer(i, j);
		}
	}
}

void liberer(int i, int j)
{
	struct f_list * pCourant;
	if(chemin[i][j].flow_list != NULL && chemin[i][j].PicFlow_End != NULL)
	{
		pCourant = chemin[i][j].flow_list;
		while(pCourant != chemin[i][j].PicFlow_End && pCourant->next != NULL)
		{
			chemin[i][j].flow_list = pCourant->next;

			pCourant->next = NULL;
			free(pCourant->agent_addr);
			free(pCourant->src_addr);
			free(pCourant->dst_addr);
			free(pCourant->gateway_addr);
			free(pCourant);
			chemin[i][j].taille--;
			taille--;

			pCourant = chemin[i][j].flow_list;
		}

		if(chemin[i][j].PicFlow_End->next != NULL)
		{

			chemin[i][j].flow_list = chemin[i][j].PicFlow_End->next;
		}
		else
		{
			chemin[i][j].AddFlow = NULL;
			chemin[i][j].flow_list = NULL;
			chemin[i][j].PicFlow_End = NULL;
		}



		pCourant->next = NULL;
		free(pCourant->agent_addr);
		free(pCourant->src_addr);
		free(pCourant->dst_addr);
		free(pCourant->gateway_addr);
		free(pCourant);
		pCourant = NULL;
		chemin[i][j].taille--;
		taille--;
	}
}

struct odPair * Add_path(char * received_path)
{
	struct odPair * pathh = NULL;
	struct odPair * nouveau = NULL;
	struct odPair * pCourant = NULL;
	char * fin_chaine;
// 	char * received_path = NULL;
	char * current_interface = NULL;
	char * sep_position = NULL;
	char * prev_sep_position = NULL;
	int tail = 0;

// 	current_interface = (char *)malloc(sizeof("255.255.255.255"));
	prev_sep_position = received_path;
	sep_position = received_path -1;
	fin_chaine = strchr(received_path,'\0');


	do
	{
		prev_sep_position = sep_position + 1;
		sep_position = strchr(prev_sep_position,'#');
		tail = sep_position-prev_sep_position + 1;
		current_interface = (char *)malloc(tail * sizeof(char));
		memset(current_interface, '\0', tail);
	

		strncpy(current_interface,prev_sep_position,sep_position-prev_sep_position);
		nouveau = (struct odPair *)malloc(sizeof(struct odPair));
		nouveau->interface = current_interface;
		nouveau->next = NULL;

		if(pathh != NULL)
		{
			pCourant = pathh;
			while(pCourant->next != NULL)
				pCourant = pCourant->next;
			pCourant->next=nouveau;
		}
		else
		{
			pathh = nouveau;
		}


	}while(sep_position < fin_chaine-1);


	return pathh;
}

void Add_result(struct calcul_list * calcul)
{
	int nRapport = 0;
	struct result_list *pCourant;
	struct result_list *pNouveau;
	pNouveau = (struct result_list *)malloc(sizeof(struct result_list));
	int l = 0;
	struct calcul_list * cl = NULL;
	struct calcul_list * cl2 = NULL;
	double packet = 0;
	double varianc = 0;
	u_int64_t byte = 0;
	u_int64_t gPacket = 0;

  	FILE * chemintxt;


	int as1 = 0;
	int as2 = 0;
	double alpha = 0.0;

	int remotePortt = 2618;
	char *remoteServerr;
	int pp;
	char *received_path;
	struct odPair * cc;

	received_path = (char *)malloc(NOMBRE_AS * sizeof("255.255.255.255"));
	memset(received_path, '\0', NOMBRE_AS * sizeof("255.255.255.255"));

 	remoteServerr = (char *)malloc(sizeof("138.96.196.16"));//sim1
 	strcpy(remoteServerr, "138.96.196.16");
//  	remoteServerr = (char *)malloc(sizeof("127.0.0.1"));//localhost
//  	strcpy(remoteServerr, "127.0.0.1");//localhost


	cl = calcul;

 	as1 = cl->flow->src_as;
	as2 = cl->flow->dst_as;

// 	fprintf(stderr, "AS1 = %d et AS2 = %d \n", as1, as2);
	
	if (calcul == NULL || as1 < 1 || as1 > NOMBRE_AS ||  as2 < 1 || as2 > NOMBRE_AS)
	{
		fprintf(stderr, "la liste calcul est nulle ou l'une des ASs est nulle");
	}
	else
	{
		// Si le chemin est NULL on demande le path et on le sauvegarde
		if(chemin[as1-1][as2-1].chemin == NULL)
		{
			pp = SendPathRequestPacket(remoteServerr, remotePortt, as1, as2, received_path);
			chemin[as1-1][as2-1].chemin = Add_path(received_path);

			chemintxt = fopen("chemin.txt", "a");
			fprintf(chemintxt, "%d \t %d \t %s \n", as1, as2, received_path);
			fclose(chemintxt);
		}
		// Calcul de alpha

		cc = chemin[as1-1][as2-1].chemin;


		do
		{
			int find = 0;
			double p = 0;
			int indice = 0;
			do
			{
				if (strcmp(les_moniteurs[indice].adresse, cc->interface) == 0)
				{
					find = 1;
					p = les_moniteurs[indice].sampling_rate;
					alpha += (p / (1 - p));
				
//   					
				}
				indice++;
			}while((find == 0) && (indice < NOMBRE_MONITEURS));
			
			if(find == 0)
			{
				fprintf(stderr, "find == 0 pour le mon %s \n", cc->interface);
			}

			cc = cc->next;
		}while(cc != NULL);

		cl = calcul;
	
		do 
		{
			packet += cl->estimated_packetNumber * (cl->sRate / (1 - cl->sRate));
			nRapport++;
			cl = cl->next;
		}while(cl != NULL);
		gPacket = packet / alpha;
		varianc = packet / (alpha * alpha);

// 		fprintf(stderr, "Le nombre de paquets : %llu  \n", gPacket);
// 		fprintf(stderr, "Variance : %f  num %f et denom %f\n", varianc, gPacket, alpha);

		cl2 = calcul;
		do
		{
			Add_observation(cl2->flow, gPacket, alpha, 1);
			cl2 = cl2->next;
		}while(cl2 != NULL);

		// Remplissage de la liste result_list

		pNouveau->packetNumber = gPacket;
		pNouveau->flowVariance = varianc;
		pNouveau->alpha_flow = alpha;
		pNouveau->rapportNumber = nRapport;
		pNouveau->byteNumber = 0;
		pNouveau->next=NULL;

		if(chemin[as1-1][as2-1].result != NULL)
		{
			pCourant = chemin[as1-1][as2-1].result;
			while(pCourant->next != NULL)
				pCourant = pCourant->next;
			pCourant->next=pNouveau;
		}
		else
		{
			chemin[as1-1][as2-1].result = pNouveau;
		}
	}
	free(received_path);
	received_path = NULL;
}

void liberer_result()
{
	int ligne = 0;
	int colonne = 0;
	for(ligne = 0; ligne < NOMBRE_AS; ligne++)
	{
		for(colonne = 0; colonne < NOMBRE_AS; colonne++)
		{
			struct result_list * pCourant;
			if(chemin[ligne][colonne].result != NULL)
			{
				pCourant = chemin[ligne][colonne].result;
				while(pCourant->next != NULL)
				{
					chemin[ligne][colonne].result = pCourant->next;
					free(pCourant);
					pCourant = chemin[ligne][colonne].result;
				}
				free(chemin[ligne][colonne].result);
				chemin[ligne][colonne].result = NULL;
				pCourant = NULL;
			}
			if(chemin[ligne][colonne].resultat != NULL)
			{
				free(chemin[ligne][colonne].resultat);
				chemin[ligne][colonne].resultat = NULL;
			}
			
		}
	}
}




void Add_calcul(struct calcul_list **Tete, struct f_list *flow)
{
	u_int64_t octet = 0;
	u_int64_t packet = 0;
	double p;
	p = samling_rate(flow->agent_addr);
// 	fprintf(stderr, "Taux d echantillonnage %f du moniteur %s  \n", p, flow->agent_addr);

	double varianc;
	struct calcul_list *pC;
	struct calcul_list *pNouveau;
	pNouveau = (struct calcul_list *)malloc(sizeof(struct calcul_list));
	
	octet = (flow->flow_octets) / p;
	packet = (flow->flow_packets) / p;
	varianc = (packet * (1-p)) / p;
	
	pNouveau->estimated_byteNumber = octet;
	pNouveau->estimated_packetNumber = packet;
	pNouveau->variance = varianc;
	pNouveau->sRate = p;
	pNouveau->next=NULL;
	pNouveau->flow = flow;
	if(*Tete != NULL)
	{
		pC = *Tete;
		while(pC->next != NULL)
			pC = pC->next;
		pC->next=pNouveau;
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



struct perInterfaceResult * interfaceOutput(int interfaceId)
{
	int perInterfaceMF = 0;
	int perInterfaceER = 0;
	struct monitor_observation * mo = NULL;
	struct perInterfaceResult * perInterfaceRes = NULL;

	if(les_moniteurs[interfaceId].les_observations != NULL)
	{
		mo = les_moniteurs[interfaceId].les_observations;
		do
		{
			perInterfaceMF++;
			perInterfaceER += mo->rapportNumber_f;
			mo = mo->next;
		}while(mo != NULL);
	}

	perInterfaceRes = (struct perInterfaceResult *)malloc(sizeof(struct perInterfaceResult));

	perInterfaceRes->measuredFlows = perInterfaceMF;
	perInterfaceRes->exportedRapports = perInterfaceER;

	return perInterfaceRes;
}



struct theResult * output()
{
	struct result_list *aa;
	struct theResult *theRes;
	struct theResult *globalRes;
	u_int64_t som = 0;
	int ir = 0;
	double accurac = 0;
	double acc = 0;
	double normaliser = 0;
	int odRapportNumber = 0;

	u_int64_t global_som = 0;
	int global_odRapportNumber = 0;
	int global_ir = 0;
	double global_acc = 0;
	double global_normaliser = 0;
	int ligne = 0;
	int colonne = 0;

	for(ligne = 0; ligne < NOMBRE_AS; ligne++)
	{
		for(colonne = 0; colonne < NOMBRE_AS; colonne++)
		{

			som = 0;
			ir = 0;
			accurac = 0;
			acc = 0;
			normaliser = 0;
			odRapportNumber = 0;


			theRes = (struct theResult *)malloc(sizeof(struct theResult));
			if(chemin[ligne][colonne].result != NULL)
			{
				aa = chemin[ligne][colonne].result;
				do
 				{	
					acc = acc + aa->flowVariance;
					ir++;
					odRapportNumber += aa->rapportNumber;
					som = som + aa->packetNumber;
					aa = aa->next;
				}while(aa != NULL);

				if(som != 0)
				{
					normaliser = acc / (som * som);
				}
				else
				{
					normaliser = 0;
				}
			}

			global_ir += ir;
			global_som += som;
			global_acc += acc;
			global_normaliser += normaliser;
			global_odRapportNumber += odRapportNumber;

			theRes->measuredFlowNumber = ir;
			theRes->estimatedPacketNumber = som;
			theRes->accuracy = acc;
			theRes->relatifError = normaliser;
			theRes->nombre_rapport = odRapportNumber;
			chemin[ligne][colonne].resultat = theRes;
		}
	}

	globalRes = (struct theResult *)malloc(sizeof(struct theResult));
	globalRes->measuredFlowNumber = global_ir;
	globalRes->estimatedPacketNumber = global_som;
	globalRes->accuracy = global_acc;
	globalRes->relatifError = global_normaliser;
	globalRes->nombre_rapport = global_odRapportNumber;

	return globalRes;
}

double gen_random()
{
double theStep;
int aleatoire;

aleatoire = rand() % 100;

theStep = (aleatoire / 100.0) + 1.5;

return theStep;
}

void reconfigurer(char * rs, int loverhead, int lcompteur)
{
	int ligne = 0;
	int colonne = 0;
	char *remotServer;
	char *monitorAdr;
	int remotePort = 2618;
	int p11 = 0;
	int p22 = 0;
	double delta_pj;
	double stepp;
	double stepp_des;
	double interm, ovh;
	double sigma = 2;
	int target_overhead = 0;



	double taux;
	double minSR = 0.0005;
	double maxSR = 0.999;
	int l = 0;

	double moyenne = 0;
	double laSomme = 0;

	int indice_min = 0;
	double min = 0;
	double val = 0;
	double pj = 0;
	double somm = 0;
	int overhead_fixe;

	u_int64_t packetNumber = 0;
	double valsji = 0;
	int rr;

	double les_deltaVar[NOMBRE_MONITEURS];
	
	remotServer = rs;
	
// 	stepp = gen_random();
// 	stepp_des = gen_random();
	

	target_overhead = threshold;
	ovh = sizeval / (lcompteur * 1.0);

	interm = 1 + (sigma * fabs((target_overhead - ovh) / target_overhead));
	if (interm < 3)
	{
		stepp = interm;
		stepp_des = interm;
	}
	else
	{
		stepp = 3;
		stepp_des = 3;
	}

// 	stepp = 1.288; // aller du min au max dans 30 min
// 	stepp_des = 1.432; // 3/2 of 0.288 pour balayer toute l echelle
	overhead_fixe = loverhead;
	
	if(overhead_fixe == 0)
	{	
		for (l = 0 ; l < NOMBRE_MONITEURS ; l++)
		{
			if(les_moniteurs[l].sampling_rate > (maxSR / stepp))
			{
				taux = maxSR;
			}
			else
			{
				taux = les_moniteurs[l].sampling_rate * stepp;
			}
	
			les_moniteurs[l].sampling_rate = taux;
	
			p11 = taux * 10000;
			p22 = 10000;
			monitorAdr = les_moniteurs[l].adresse;
			rr = SendControlPacket(remotServer, remotePort, p11, p22, monitorAdr );
		}
	}


	
	if(overhead_fixe == 1)
	{	
		struct monitor_observation * mo = NULL;
		struct monitor_observation * mo2 = NULL;

		for(l=0; l < NOMBRE_MONITEURS; l++)
		{	
			if(les_moniteurs[l].les_observations != NULL)
			{
				mo = les_moniteurs[l].les_observations;
					do
					{
						if(mo->used != 1)
						{
							somm = 0;
							ligne = mo->src_as;
							colonne = mo->dst_as;
							mo2 = mo;
							do
							{
								if(mo2->src_as == ligne && mo2->dst_as == colonne)
								{
									mo2->used = 1;
									somm += ( mo2->s_f / (mo2->alpha_f * mo2->alpha_f));
								}
								mo2 = mo2->next;
							}while(mo2 != NULL);
							pj = les_moniteurs[l].sampling_rate;
							packetNumber = chemin[ligne - 1][colonne - 1].resultat->estimatedPacketNumber;
							val = pj / ((1 - pj) * (1 - pj));// absolute value ;)
							valsji = val * (somm / (packetNumber * packetNumber));
						}
				
// 						fprintf(stderr, "&&&&&&&&&&&&&&&&&&&&&&&&&&&   Reconfigurerrrrrrrrrrrrrrrrrrrrrrrrrrrrrrrrrrrrrrrrrrrr \n");	
						mo = mo->next;
					}while(mo != NULL);
					les_deltaVar[l] += valsji;	
			}
			else
			{
				les_deltaVar[l] = 0;
			}
		}

		moyenne = 0;
		laSomme = 0;
		for (l = 0 ; l < NOMBRE_MONITEURS ; l++)
		{
			laSomme += les_deltaVar[l];
		}
		moyenne = laSomme / NOMBRE_MONITEURS;

		for (l = 0 ; l < NOMBRE_MONITEURS ; l++)
		{
			if(les_deltaVar[l] < moyenne)
			{
				if(les_moniteurs[l].sampling_rate < (stepp_des * minSR))
				{
					taux = minSR;
				}
				else
				{
					taux = les_moniteurs[l].sampling_rate / stepp_des;
				}	
	
				les_moniteurs[l].sampling_rate = taux;
	
				monitorAdr = les_moniteurs[l].adresse;
	
				p11 = taux * 10000;
				p22 = 10000;
		
				rr = SendControlPacket(remotServer, remotePort, p11, p22, monitorAdr );
			}
		}

		

		/*min = les_deltaVar[0];
		indice_min = 0;

		for (l = 0 ; l < NOMBRE_MONITEURS ; l++)
		{
			if(les_deltaVar[l] < min && les_moniteurs[l].sampling_rate > minSR && les_deltaVar[l] != 0)
			{
				min = les_deltaVar[l];
				indice_min = l;
			}
			if(les_moniteurs[indice_min].sampling_rate == minSR)
			{
				min = les_deltaVar[l];
				indice_min = l;
			}
		}

	
		if(les_moniteurs[indice_min].sampling_rate < (stepp_des * minSR))
		{
			taux = minSR;
		}
		else
		{
			taux = les_moniteurs[indice_min].sampling_rate / stepp_des;
		}	
	
		les_moniteurs[indice_min].sampling_rate = taux;
	
		monitorAdr = les_moniteurs[indice_min].adresse;
	
		p11 = taux * 10000;
		p22 = 10000;
		
		rr = SendControlPacket(remotServer, remotePort, p11, p22, monitorAdr );*/
	}
	
}
//////////////////////////////////////////////////////






void Start_CompteurThread()
{
	

	

	pthread_attr_t threadAttr;

	if (pthread_attr_init (&threadAttr) != 0)
	{
		fprintf(stderr, "Pthread ATTr INIT ERROR");
		
	}
 	



	if(pthread_attr_setdetachstate (&threadAttr,PTHREAD_CREATE_DETACHED) != 0)
	{
		fprintf(stderr, "pthread_attr_setdetachstate error");
	}

	

	if (pthread_create (&CompteurThread,&threadAttr,fcompteur, NULL) < 0) 
		{
			fprintf(stderr, "THread NON cree ^^^^^^^^ \n");
		}

	

}



static void * fcompteur(void * arg)
{

	while (Emulation)
    	{
		compteur ++ ;
		compteur2 ++;
 		sleep(1);
	}
}


// typedef struct ThreadParam
// {
// 	struct un_chemin * list[NOMBRE_AS][NOMBRE_AS];
// 
// }ThreadParam;


void Start_EstimationThread()
{
// 	int th = 0;
	
// 	ThreadParam * t = (ThreadParam * )malloc(sizeof(ThreadParam));
// 	for(th = 0; th < NOMBRE_AS; th++)
// 	{
// 		t->list = &chemin[0][0];
// 	}
// 	t->list = &estimation_flow_list;
	

	pthread_attr_t threadAttr;

	if (pthread_attr_init (&threadAttr) != 0)
	{
		fprintf(stderr, "Pthread ATTr INIT ERROR");
		
	}
 	



	if(pthread_attr_setdetachstate (&threadAttr,PTHREAD_CREATE_DETACHED) != 0)
	{
		fprintf(stderr, "pthread_attr_setdetachstate error");
	}

	

	if (pthread_create (&EstimationThread,&threadAttr,Estimation, NULL) < 0) //(void *) t) < 0) 
		{
			fprintf(stderr, "THread NON cree ^^^^^^^^ \n");
		}

	
// 	t = NULL;
}










static void *Estimation(void * arg)
{
	float debit = 0;
	int local_overhead = 0;
	int ligne = 0;
	int colonne = 0;
	int k = 0;
	int oh = threshold;
	struct theResult * the_result;
	struct perInterfaceResult * perInterface_result;
	char * remoteServer;
	u_int64_t ab = 0;
	FILE *chemin2;
	FILE *od;
	FILE *f2;
	FILE *sr;
	FILE *f;
	FILE *odconv;
	FILE *srconv;
	int local_compteur = 0;
	int local_compteur2 = 0;
	int commande;
	int tps = 0;
	int test = 0;

// 	ThreadParam * t = (ThreadParam *)arg;
	
	
// 	struct result_list * result = NULL;

	f2 = fopen("output2.txt", "w");
	fclose(f2);

	od = fopen("parOD.txt", "w");
	fclose(od);

	sr = fopen("samplingRate.txt", "w");
	fclose(sr);

	odconv = fopen("parOD_conv.txt", "w");
	fclose(odconv);

	srconv = fopen("samplingRate_conv.txt", "w");
	fclose(srconv);

	chemin2 = fopen("chemin.txt", "w");
	fclose(chemin2);

	f = fopen("output.txt", "w");

	fprintf(f, "TTTTTTTTTTTTTTTTTTTle step est %d et le seuil est %d \n", stepSize, threshold);
	fclose(f);


 	remoteServer = (char *)malloc(sizeof("138.96.196.16"));//sim1
 	strcpy(remoteServer, "138.96.196.16");
//  	remoteServer = (char *)malloc(sizeof("127.0.0.1"));//localhost
//  	strcpy(remoteServer, "127.0.0.1");

   	while (Emulation)
    	{
		if(compteur != tps)
		{
			tps = compteur;
 			commande = system("clear");
			fprintf(stderr,"\n\n\n\n\n\n\n\n\n\n\n\n\n\n\n\n\n\n\n\n\n\n\n\n\n\n\n\n\n\n\n\n\n\n\n\n\n\n\n\n\n\n\n\n\n\n\n\n\n\n\n\n\n\n\n\n\n\n\n\n\n\n");
// 			exec("clear");
			fprintf(stderr, "Counter: %i \t The buffer size %llu \n", compteur, taille);
		}
		if(compteur >= 60 || overhead == 1)
		{
			local_overhead = overhead;
			local_compteur = compteur;
			local_compteur2 = compteur2;

			ab = 0;

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
			struct f_list * pCourant = NULL;
			struct f_list * pCourant2 = NULL;

			for(ligne = 0; ligne < NOMBRE_AS; ligne++)
			{
				for(colonne = 0; colonne < NOMBRE_AS; colonne++)
				{
					chemin[ligne][colonne].PicFlow_End = chemin[ligne][colonne].AddFlow;
				}
			}
			sizeval = taille;

			for(ligne = 0; ligne < NOMBRE_AS; ligne++)
			{
				for(colonne = 0; colonne < NOMBRE_AS; colonne++)
				{
					if(chemin[ligne][colonne].PicFlow_End != NULL)
					{
						test = 0;
						do
						{
							if(test == 0)
							{
								pCourant = chemin[ligne][colonne].flow_list;
								test = 1;
							}
							else
							{
								pCourant = pCourant->next;
							}

							if(pCourant->used != 1)
							{
								struct calcul_list * calcul = NULL;
						
								Add_calcul(&calcul, pCourant);

								if(pCourant->next != NULL && pCourant != chemin[ligne][colonne].PicFlow_End)
								{
									pCourant2 = pCourant;
						
									do 
									{
										pCourant2 = pCourant2->next;
										if((strcmp(pCourant2->src_addr, pCourant->src_addr) == 0) && (strcmp(pCourant2->dst_addr, pCourant->dst_addr) == 0) && (pCourant2->src_port == pCourant->src_port) && (pCourant2->dst_port == pCourant->dst_port) && (pCourant2->protocol == pCourant->protocol))
										{
											pCourant2->used = 1;
											Add_calcul(&calcul, pCourant2);	
										}
// 										fprintf(stderr, "z");
								
									}while(pCourant2 != chemin[ligne][colonne].PicFlow_End && pCourant2->next != NULL);

// 									fprintf(stderr, "g");
						// calcul contient une liste pr un seul flow et Add_result va combiner ts les resultats de ce flow et ajouter un noeud result ;)
									if(calcul != NULL)
									{
										Add_result(calcul);	
										liberer_calcul(&calcul);
										calcul = NULL;
									}
								}
							}

							pCourant->used = 1;
							ab++;

// 							fprintf(stderr, "o");
						}while(pCourant != chemin[ligne][colonne].PicFlow_End && pCourant != NULL);
// 						fprintf(stderr, "p");
					}
				}
			}
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
			if(sizeval != 0)
			{
					the_result = output();
					reconfigurer(remoteServer, local_overhead, local_compteur);
// 					reconfigurer(remoteServer, local_overhead);
					f2 = fopen("output2.txt", "a");
					od = fopen("parOD.txt", "a");
					sr = fopen("samplingRate.txt", "a");

					if(local_overhead ==1 && local_compteur > 56)
					{
						odconv = fopen("parOD_conv.txt", "a");
						srconv = fopen("samplingRate_conv.txt", "a");
					}

					debit = sizeval / local_compteur;
	
					fprintf(f2, "%d \t %d \t %d \t %llu == %llu \t %f \t %d \t %llu \t %d \t %f \n", local_compteur2, local_compteur, oh, ab, sizeval, debit, the_result->measuredFlowNumber, the_result->estimatedPacketNumber, the_result->nombre_rapport, the_result->relatifError);

					free(the_result);
					the_result = NULL;
	
					for(k = 0; k < NOMBRE_MONITEURS; k++)
					{
						perInterface_result = interfaceOutput(k);

						fprintf(sr, " %d \t %d  \t %s \t %f \t %d \t %d \n", local_compteur2, k+1, les_moniteurs[k].adresse, les_moniteurs[k].sampling_rate, perInterface_result->measuredFlows, perInterface_result->exportedRapports);

						if(local_overhead ==1 && local_compteur > 56)
						{
							fprintf(srconv, " %d \t %d  \t %s \t %f \t %d \t %d \n", local_compteur2, k+1, les_moniteurs[k].adresse, les_moniteurs[k].sampling_rate, perInterface_result->measuredFlows, perInterface_result->exportedRapports);
						}

						free(perInterface_result);
						perInterface_result = NULL;
					}

					for(ligne = 0; ligne < NOMBRE_AS; ligne++)
					{
						for(colonne = 0; colonne < NOMBRE_AS; colonne++)
						{
							fprintf(od, " %d \t %d \t %d \t %d \t %llu \t %d \t %f \t %f \n", local_compteur2, (ligne + 1), (colonne + 1), chemin[ligne][colonne].resultat->measuredFlowNumber, chemin[ligne][colonne].resultat->estimatedPacketNumber, chemin[ligne][colonne].resultat->nombre_rapport, chemin[ligne][colonne].resultat->accuracy, chemin[ligne][colonne].resultat->relatifError);

							if(local_overhead ==1 && local_compteur > 56)
							{
								fprintf(odconv, " %d \t %d \t %d \t %d \t %llu \t %d \t %f \t %f \n", local_compteur2, (ligne + 1), (colonne + 1), chemin[ligne][colonne].resultat->measuredFlowNumber, chemin[ligne][colonne].resultat->estimatedPacketNumber, chemin[ligne][colonne].resultat->nombre_rapport, chemin[ligne][colonne].resultat->accuracy, chemin[ligne][colonne].resultat->relatifError);
							}
					
						}
					}
			
					if(local_overhead ==1 && local_compteur > 56)
					{
						fclose(odconv);
						fclose(srconv);
					}	

					fclose(f2);
					fclose(od);
					fclose(sr);

					

					liberer_result();



				sem_wait (&semFlows);
				liberer_flow();
				sem_post (&semFlows);
				liberer_moniteurs();
				
				overhead = 0;
				local_overhead = 0;
				sizeval = 0;
			}
			
			else
			{
				fprintf(stderr, "We have not received NetFlow records during the last minute \n");
			}
// 			sem_post (&semFlows);
			compteur = 0;
			local_compteur = 0;
			local_compteur2 = 0;
		}



     	}

free(remoteServer);
remoteServer = NULL;

// free(t);
// t = NULL;
}





void remplir_topologie()
{
	int x = 0;
	int y = 0;

	for(x = 0; x < NOMBRE_AS; x++)
	{
		for(y = 0; y < NOMBRE_AS; y++)
		{
			chemin[x][y].chemin = NULL;
			chemin[x][y].flow_list = NULL;
			chemin[x][y].AddFlow = NULL;
			chemin[x][y].PicFlow_End = NULL;
			chemin[x][y].result = NULL;
			chemin[x][y].resultat = NULL;
			chemin[x][y].taille = 0;
		}
	}
}


void remplir_les_moniteurs()
{
int i = 0;

char *interfaces[] = {"10.0.0.127" , "10.0.0.169" , "10.0.1.1" , "10.0.0.138" , "10.0.0.126" , "10.0.0.182" , "10.0.0.90" , "10.0.0.69" , "10.0.0.170" , "10.0.2.1" , "10.0.0.166" ,
"10.0.0.137" , "10.0.3.1" , "10.0.0.102" , "10.0.0.22" , "10.0.0.117" , "10.0.0.181" , "10.0.4.1" , "10.0.0.25" , "10.0.0.17" , "10.0.0.21" , "10.0.5.1" , "10.0.0.89" , "10.0.0.18" ,
"10.0.0.85" , "10.0.0.50" , "10.0.0.10" , "10.0.0.30" , "10.0.6.1" , "10.0.0.29" , "10.0.0.33" , "10.0.0.2" , "10.0.7.1" , "10.0.0.70" , "10.0.0.78" , "10.0.8.1" , "10.0.0.26" , 
"10.0.0.86" , "10.0.9.1" , "10.0.0.185" , "10.0.0.77" , "10.0.0.9" , "10.0.10.1" , "10.0.0.101" , "10.0.0.165" , "10.0.0.38" , "10.0.0.129" , "10.0.0.49" , "10.0.11.1" , "10.0.0.113" , 
"10.0.0.66" , "10.0.0.125" , "10.0.12.1" , "10.0.0.65" , "10.0.0.58" , "10.0.0.82" , "10.0.0.61" , "10.0.0.130" , "10.0.13.1" , "10.0.0.186" , "10.0.0.62" , "10.0.0.154" , "10.0.0.142",
"10.0.0.34" , "10.0.14.1" , "10.0.0.177" , "10.0.0.13" , "10.0.0.110" , "10.0.0.1" , "10.0.0.57" , "10.0.15.1" , "10.0.0.46" , "10.0.0.114" , "10.0.16.1" , "10.0.0.45" , "10.0.0.37" ,
"10.0.0.41" , "10.0.17.1" , "10.0.0.42" , "10.0.0.106" , "10.0.18.1" , "10.0.0.105" , "10.0.0.146" , "10.0.0.178" , "10.0.19.1" , "10.0.0.145" , "10.0.0.73" , "10.0.20.1", "10.0.0.74" , "10.0.0.14" , "10.0.21.1" , "10.0.0.141" , "10.0.0.81" , "10.0.22.1" , "10.0.0.153" , "10.0.0.118" , "10.0.23.1"};

	for(i = 0; i < NOMBRE_MONITEURS; i++)
	{
		les_moniteurs[i].sampling_rate = conf_init;
		les_moniteurs[i].adresse = (char *)malloc(sizeof("255.255.255.255"));
		strcpy(les_moniteurs[i].adresse, interfaces[i]);
// 		les_moniteurs[0].adresse = m0;
		les_moniteurs[i].les_observations = NULL;
// // // memset(current_interface, '\0', tail);
	}



}

