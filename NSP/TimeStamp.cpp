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

#include "TimeStamp.h"
#include "exception.h"
#include "CommonStructs.h"
#define _BSD_SOURCE
#include <unistd.h>

// In average the scheduler takes 10 ms to switch between contexts
#define TIME_CONSUMED_BY_THE_SCHEDULER 0.001 

TimeStamp::TimeStamp(unsigned int usecond, unsigned int second)
{
	this->usecond = usecond;
	this->second = second;
}

TimeStamp::TimeStamp(TimeStamp & ts)
{
	second = ts.second;
	usecond = ts.usecond;
}

TimeStamp::TimeStamp(const TimeStamp & ts)
{
	second = ts.second;
	usecond = ts.usecond;
}

TimeStamp::TimeStamp(timeval & t)
{
	second = t.tv_sec;
	usecond = t.tv_usec;
}

void TimeStamp::Increment()
{
	usecond++;
	if(usecond == 999999)
	{
		usecond = 0;
		second++;
	}
}

// Copys a time stamp
void TimeStamp::Copy(TimeStamp & ts)
{
	second = ts.second;
	usecond = ts.usecond;
}

// Compare two TimeStamps
// returns -1 if ts is smaller than this, 0 if equal and +1 if ts is bigger than this
int TimeStamp::Compare( TimeStamp & ts)
{
	if(ts.GetSecond() < second)
		return -1;
	else if( ts.GetSecond() == second)
	{
		if(ts.GetuSecond() < usecond)
			return -1;
		else if(ts.GetuSecond() > usecond)
			return +1;
		else if(ts.GetuSecond() == usecond)
			return 0;
	} else if(ts.GetSecond() > second)
		return +1;
}
string TimeStamp::GetTime()
{
	string res;
	char tmp[10];

	sprintf(tmp, "%li", second);
	res.append(tmp);
	res.append(":");

	sprintf(tmp, "%li", usecond);
	res.append(tmp);

	return res;
}

void TimeStamp::WaitBetween(TimeStamp & t1, TimeStamp &t2)
{
	if(t1.Compare(t2) > 0)
	{
		timeval x, y, result;
		x.tv_sec = t1.GetSecond();
		x.tv_usec = t1.GetuSecond();

		y.tv_sec = t2.GetSecond();
		y.tv_usec = t2.GetuSecond();
 
		if(x.tv_sec == y.tv_sec)
		{
			result.tv_sec = 0;
			result.tv_usec = y.tv_usec - x.tv_usec;
		} else if(x.tv_sec < y.tv_sec)
		{
			unsigned int tmp;
			tmp = (y.tv_sec * 1000000 + y.tv_usec) - (x.tv_sec * 1000000 + x.tv_usec );
			result.tv_usec = tmp % 1000000;
			result.tv_sec = tmp / 1000000;
		}
		// Perform sleep
		#ifdef SHOW_TIME_TO_WAIT_BETWEEN_TWO_PACKETS 
		cout << "Waiting from: "<<t1.GetTime()<<" to "<<t2.GetTime()<<" : "<< (result.tv_sec > 0?result.tv_sec:0) <<" second and "<<(result.tv_usec > 0?result.tv_usec:0)<<" usecond"<<endl;
		#endif
		if(result.tv_usec > TIME_CONSUMED_BY_THE_SCHEDULER * 1000000 )
			usleep(result.tv_usec - TIME_CONSUMED_BY_THE_SCHEDULER * 1000000);
		else if(result.tv_usec >0)
			{usleep(1);}

		if(result.tv_sec > 0)
			sleep(result.tv_sec - TIME_CONSUMED_BY_THE_SCHEDULER);
	} else if(t1.Compare(t2) <0)
			cout<<"Invalid TimeStamp."<<__FILE__<<":"<< __LINE__;
	
}

void TimeStamp::WaitBetween(TimeStamp & t1, TimeStamp &t2, TimeStamp &loadingTime, TimeStamp & traceCT, TimeStamp & emulatingCT)
{
	if(t1.Compare(t2) > 0)
	{
		timeval x, y, result;
		x.tv_sec = t1.GetSecond();
		x.tv_usec = t1.GetuSecond();

		y.tv_sec = t2.GetSecond();
		y.tv_usec = t2.GetuSecond();
 
		if(x.tv_sec == y.tv_sec)
		{
			result.tv_sec = 0;
			result.tv_usec = y.tv_usec - x.tv_usec;
		} else if(x.tv_sec < y.tv_sec)
		{
			unsigned int tmp;
			tmp = (y.tv_sec * 1000000 + y.tv_usec) - (x.tv_sec * 1000000 + x.tv_usec );
			result.tv_usec = tmp % 1000000;
			result.tv_sec = tmp / 1000000;
		}
		TimeStamp tmp(result);

		if(tmp.Compare(loadingTime) > 0)
		{
			// Dont wait any more
			#ifdef SHOW_TIME_TO_WAIT_BETWEEN_TWO_PACKETS 
			cout << "Loading Time "<<loadingTime.GetTime()<<" exceeded Time to wait "<<tmp.GetTime()<<endl;
			#endif
		}else
		{
			// Verifay if traceCt > emulatingCt then Wait else do not wait
			if(emulatingCT.Compare(traceCT) > 0)
				WaitBetween(loadingTime, tmp);
		}
	
	} else if(t1.Compare(t2) < 0)
			cout<<"Invalid TimeStamp."<<__FILE__<<":"<< __LINE__;

}

void TimeStamp::GetTimeToWaitBetween(TimeStamp & t1, TimeStamp &t2, TimeStamp &t3)
{
	if(t1.Compare(t2) > 0)
	{
		
		timeval x, y, result;
		x.tv_sec = t1.GetSecond();
		x.tv_usec = t1.GetuSecond();

		y.tv_sec = t2.GetSecond();
		y.tv_usec = t2.GetuSecond();
 
		if(x.tv_sec == y.tv_sec)
		{
			result.tv_sec = 0;
			result.tv_usec = y.tv_usec - x.tv_usec;
		} else if(x.tv_sec < y.tv_sec)
		{
			unsigned int tmp;
			tmp = (y.tv_sec * 1000000 + y.tv_usec) - (x.tv_sec * 1000000 + x.tv_usec );
			result.tv_usec = tmp % 1000000;
			result.tv_sec = tmp / 1000000;
		}
		t3.SetuSecond(result.tv_usec);
		t3.SetSecond(result.tv_sec);
		
	} else if (t1.Compare(t2) < 0)
			cout<<"Invalid TimeStamp."<<__FILE__<<":"<< __LINE__<<endl;
}
void TimeStamp::SetToTheSystemTime()
{
	struct timeval ts;
	gettimeofday(&ts, NULL);
	usecond = ts.tv_usec;
	second = ts.tv_sec;
}
