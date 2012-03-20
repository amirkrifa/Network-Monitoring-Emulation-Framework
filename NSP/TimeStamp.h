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

#ifndef TIME_STAMP_H
#define TIME_STAMP_H
#include <string>
#include <iostream>
#include <sys/time.h>

using namespace std;

class TimeStamp {
public:
	TimeStamp(unsigned int usecond, unsigned int second);
	TimeStamp(TimeStamp &);
	TimeStamp(const TimeStamp &);
	TimeStamp():usecond(0),second(0){}
	TimeStamp(timeval & t);
	void Increment();
	string GetTime();
	int Compare( TimeStamp & ts);
	unsigned int GetSecond(){return second;}
	unsigned int GetuSecond(){return usecond;}
	void SetuSecond(unsigned int usec){usecond = usec;}
	void SetSecond(unsigned int sec){second = sec;}
	void Copy(TimeStamp & ts);
	static void WaitBetween(TimeStamp & t1, TimeStamp &t2);
	static void WaitBetween(TimeStamp & t1, TimeStamp &t2, TimeStamp &loadingTime, TimeStamp & traceCT, TimeStamp & emulatingCT);
	void Init()
	{
		usecond = 0;
		second = 0;
	}
	static void GetTimeToWaitBetween(TimeStamp & t1, TimeStamp & t2, TimeStamp & t3);
	void SetToTheSystemTime();
private:
	unsigned int usecond;
	unsigned int second;
};
#endif
