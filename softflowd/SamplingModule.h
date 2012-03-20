/*

  Copyright (C) 2010  INRIA, Plan�te Team

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

#ifndef SAMPLING_MODULE_H
#define SAMPLING_MODULE_H
#include <semaphore.h>
/* Prototypes for functions defined in SamplingModule.c */
// General case 

typedef struct SamplingNode
{
	int samplingRateP1;
	int samplingRateP2;
	int samplingRateChanged;
	int samplingRateAxis;
	char ipAddr[16];
	int * samplingTab;
	sem_t sem;
	struct SamplingNode * next;

}SamplingNode;

int ChangeSamplingRateInNode(int  newSamplingRateP1, int newSamplingRateP2, char * ip );
int RandomPeriodicNodeSampling(char * ip);
void FreeListNodes();
#endif
