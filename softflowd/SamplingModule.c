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

#include "SamplingModule.h"
#include "common.h"
#include <time.h>
#include <stdlib.h>


// The list of sampling nodes
SamplingNode * listNodes = NULL;



///////////////////////////////////////////////////////////////////
//        The general case used for the emulator
//////////////////////////////////////////////////////////////////

// Add a new Node 
// Not thread safe
SamplingNode * AddNode(char * ip)
{
	char tmpIp[16];
	memset(tmpIp, '\0', 16);
	strcpy(tmpIp, ip);
	if(listNodes == NULL)
	{
		listNodes = (SamplingNode * )malloc(sizeof(SamplingNode));
		listNodes->next = NULL;
		listNodes->samplingRateP1 = 100;
		listNodes->samplingRateP2 = 100;
		listNodes->samplingRateChanged = 1;
		listNodes->samplingRateAxis = 0;
		listNodes->samplingTab = NULL;
		memset(listNodes->ipAddr, '\0', 16);
		strcpy(listNodes->ipAddr, tmpIp);
		sem_init(&listNodes->sem, 0, 1);
		// Init the random numbers generator
		time_t seconds;
		time(&seconds);
		srand((unsigned int)seconds);
		return listNodes;
	}else 
	{
		SamplingNode* sn = listNodes;
		while(sn->next != NULL)
		{
			sn = sn->next;
		}

		// Adding the node
		sn->next = (SamplingNode * )malloc(sizeof(SamplingNode));
		sn = sn->next;
		sn->samplingRateP1 = 100;
		sn->samplingRateP2 = 100;
		sn->samplingRateChanged = 1;
		sn->samplingRateAxis = 0;
		sn->samplingTab = NULL;
		memset(sn->ipAddr, '\0', 16);
		strcpy(sn->ipAddr, tmpIp);
		sem_init(&sn->sem, 0, 1);
		sn->next = NULL;
		return sn;
	}
		
}

// Look for a sampling Node given its ip addr, if not found create a new one
SamplingNode * GetSamplingNode(char * ip)
{
	char tmpIp[16];
	memset(tmpIp, '\0', 16);
	strcpy(tmpIp, ip);
	if(listNodes == NULL)
	{
		// Adding a new node
		SamplingNode * tmpNode = AddNode(tmpIp);
		return tmpNode;	
	}
	else
	{
		SamplingNode * tmp = listNodes;
		while(tmp != NULL)
		{
			if(strcmp(tmp->ipAddr, tmpIp) == 0)
			{
				// Node found
				return tmp;
			}
			tmp = tmp->next;
		}
		if(tmp == NULL)
		{
			// Add a new Node 
			return AddNode(tmpIp);
		}
		return NULL;
	}
}

// Try to find a sampling node, returns NULL if not found
SamplingNode * FindSamplingNode(char * ip)
{
	char tmpIp[16];
	memset(tmpIp, '\0', 16);
	strcpy(tmpIp, ip);
	if(listNodes == NULL)
	{
		return NULL;	
	}
	else
	{
		SamplingNode * tmp = listNodes;
		while(tmp != NULL)
		{
			if(strcmp(tmp->ipAddr, tmpIp) == 0)
			{
				// Node found
				return tmp;
			}
			tmp = tmp->next;
		}
		return tmp;
	}
}


// Deletes the listNodes
void FreeListNodes()
{
	SamplingNode * tmpNode = NULL;
	if(listNodes != NULL)
	{
		while(listNodes != NULL)
		{
			tmpNode = listNodes;
			listNodes = listNodes->next;
			sem_destroy(&tmpNode->sem);
			free(tmpNode);
		}
	}
}


int ChangeSamplingRateInNode(int  newSamplingRateP1, int newSamplingRateP2, char * ip )
{
	char tmpIp[16];
	memset(tmpIp, '\0', 16);
	strcpy(tmpIp, ip);
	logit(LOG_DEBUG, "Changing the sampling rate of the monitor %s to %i/%i.", ip, newSamplingRateP1, newSamplingRateP2);	
	SamplingNode * sn = FindSamplingNode(tmpIp);
	if(sn != NULL)
	{
		sem_wait(&sn->sem);
			sn->samplingRateP1 = newSamplingRateP1;
			sn->samplingRateP2 = newSamplingRateP2;
			sn->samplingRateChanged = 1;
		sem_post(&sn->sem);
		sn = NULL;
		return 0;
	} else 
	{
		logit(LOG_DEBUG, "Error occured while trying to change the sampling rate: monitor %s was not found.", ip);	
		return 1;	
	}
}

// Should be included in a thread safe code block
// Used to generate random choices
void GenerateRandomChoicesForNode(SamplingNode * sn)
{

	if(sn->samplingRateChanged == 1)
	{
		// Delete the last random choices
		if(sn->samplingTab != NULL)
			free(sn->samplingTab);
		sn->samplingTab = (int *)malloc(sizeof(int) * sn->samplingRateP2);
		sn->samplingRateChanged = 0;
	}

	int i = 0;
	for(; i < sn->samplingRateP2; i++)
	{
		sn->samplingTab[i] = 0;
	}
	for(i = 0; i < sn->samplingRateP1; i++)
	{
		sn->samplingTab[rand() % sn->samplingRateP2 ] = 1;
	}
	
}

// a thread safe method that returns the sampling rate status of a given node
int GetNodeSamplingRateStatus(SamplingNode *sn)
{
	int tmp;
	sem_wait(&sn->sem);
		tmp = sn->samplingRateChanged;
	sem_post(&sn->sem);
	return tmp;
}

// A thread safe method used to init the sampling rate status of a given node
void InitNodeSamplingRateStatus(SamplingNode *sn)
{
	sem_wait(&sn->sem);
		sn->samplingRateAxis = 0;
	sem_post(&sn->sem);
}


/* Returns wether we should take the current packet or not. (decision / node)
*  0 : We have to escape the current packet.
*  1 : We have to consider the current packet.
*/
int RandomPeriodicNodeSampling(char * ip)
{
	char tmpIp[16];
	memset(tmpIp, '\0', 16);
	strcpy(tmpIp, ip);
	
	// Retrive the SamplingNode
	SamplingNode *sn = GetSamplingNode(ip);
	if(sn == NULL)
	{
		logit(LOG_DEBUG, "%s:%i Invalid Sampling Node.", __FILE__, __LINE__);	
		return 0;
	}
	//fprintf(stderr, "p1 %i p2 %i src %i sra %i ip %s\n", sn->samplingRateP1, sn->samplingRateP2, sn->samplingRateChanged, sn->samplingRateAxis, sn->ipAddr );

// Thread safe block
	if(GetNodeSamplingRateStatus(sn) == 1)
	{
		InitNodeSamplingRateStatus(sn);	
	}
// Thread safe block
	
// Thread safe block
	sem_wait(&sn->sem);
	
		if(sn->samplingRateP1 == 0)
		{
			// We dont take any packet
			sem_post(&sn->sem);	
			return 0;
		}else if(sn->samplingRateP1 == sn->samplingRateP2)
		{
			// We take all the packets
			sem_post(&sn->sem);	
			return 1;
		}else if(sn->samplingRateP1 < sn->samplingRateP2)
		{
			//We take samplingRateP1 packets over samplingRateP2
			if(sn->samplingRateAxis == 0)
			{
				// Generating new numbers
				GenerateRandomChoicesForNode(sn);
				int ret = sn->samplingTab[sn->samplingRateAxis];
				sn->samplingRateAxis ++;
				sem_post(&sn->sem);
				return ret;
			}else if(sn->samplingRateAxis < (sn->samplingRateP2 - 1))
			{
				int ret = sn->samplingTab[sn->samplingRateAxis];
				sn->samplingRateAxis++;
				sem_post(&sn->sem);	
				return ret;
			
			} else if(sn->samplingRateAxis == (sn->samplingRateP2 -1))
			{
				int ret = sn->samplingTab[sn->samplingRateAxis];	
				sn->samplingRateAxis = 0;	
				sem_post(&sn->sem);	
				return ret;
			}
		} else if(sn->samplingRateP1 > sn->samplingRateP2)
		{
			logit(LOG_DEBUG, "Invalid Sampling Rate.");
			sem_post(&sn->sem);	
			return 0;
		}
// Thread safe block


	
}
