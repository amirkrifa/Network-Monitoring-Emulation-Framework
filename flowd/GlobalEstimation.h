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

#ifndef _GLOBALESTIMATION_H
#define _GLOBALESTIMATION_H

#include<pthread.h>
#include <stdio.h>
#include "store.h"
#include "addr.h"
#include "ClientController.h"


#define NOMBRE_MONITEURS 97
#define NOMBRE_AS 23
//////////////////////////////////	THREAD	////////////////////////////////////////////////////////
void Start_EstimationThread();
static void *Estimation(void * arg);

void Start_CompteurThread();
static void * fcompteur(void * arg);

////////////////////////////////////////// OutPut  //////////////////////////////

struct theResult{
int measuredFlowNumber;
u_int64_t estimatedPacketNumber;
double accuracy;
double relatifError;
int nombre_rapport;
};




struct perInterfaceResult{
int measuredFlows;
int exportedRapports;
};
/////////////////////////////////	les CHEMINS matrice reliant les AS	////////////////////////
typedef struct un_chemin{
struct odPair * chemin;
struct f_list * flow_list;
struct f_list * AddFlow;
struct f_list * PicFlow_End;
struct result_list *result;
struct theResult * resultat;
u_int64_t taille;
// struct un_chemin * next;
}route;


struct odPair{
char * interface;
struct odPair * next;
};



struct odPair * Add_path(char * r_p);
void remplir_topologie();
/////////////////////////////////	les moniteurs	//////////////////////////////////////////////////
struct monitor_observation{
int		used;
char *		agent_addr;
char *		src_addr;
char *		dst_addr;
u_int16_t		src_port;
u_int16_t		dst_port;
u_int8_t		protocol;
u_int32_t		src_as;
u_int32_t		dst_as;
u_int64_t	s_f;
double alpha_f;
int rapportNumber_f;
struct monitor_observation * next;
};

struct leResultat{
u_int64_t somme;
double variance;
};

typedef struct un_moniteur{
double sampling_rate;
char * adresse;
struct monitor_observation * les_observations;
}Couple;


Couple les_moniteurs[NOMBRE_MONITEURS];

void vider_les_moniteurs();
void remplir_les_moniteurs();
double samling_rate(char * s);

/////////////////////////////////////	flow list	///////////////////////////////////////////////////
struct f_list{
int used; 
char *		agent_addr;
char *		src_addr;
char *		dst_addr;
char *		gateway_addr;
u_int16_t		src_port;
u_int16_t		dst_port;
u_int64_t		flow_packets;
u_int64_t		flow_octets;
u_int8_t		protocol;
u_int32_t		src_as;
u_int32_t		dst_as;
struct f_list * next;
};



struct calcul_list{
u_int64_t estimated_byteNumber;
u_int64_t estimated_packetNumber;
double variance;
double sRate;
struct f_list * flow;
struct calcul_list * next;
};



struct result_list{
int rapportNumber;
double alpha_flow;
u_int64_t byteNumber;
u_int64_t packetNumber;
double flowVariance;
struct result_list * next;
};




//void estimate(struct calcul_list * liste);

void add_observation(struct f_list *fl, u_int64_t ff);



void Add_flow(struct flot_node * ff); //
void liberer(int i, int j);
void liberer_flow();



void Add_calcul(struct calcul_list ** calcul, struct f_list * flow);
void liberer_calcul(struct calcul_list **liste);

void Add_result(struct calcul_list * calcul);
void liberer_result();

#endif