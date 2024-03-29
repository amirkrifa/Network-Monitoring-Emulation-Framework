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

#include "exception.h"

namespace AmDev
{
NspException::NspException(void) throw()
{
	
}
NspException::NspException(char * sCause,string sFile, int iLine) throw()
{
	char lineNumber[10];
	sprintf(lineNumber,"%i",iLine);
    	stringCause.append(sFile);
	stringCause.append("-");
	stringCause.append(lineNumber);
	stringCause.append(":");
	stringCause.append(sCause);
}
NspException::~NspException(void) throw()
{
}
const char * NspException::what() const throw()
{
	
	return stringCause.c_str();
}
}
