/*_############################################################################
  _## 
  _##  ctr64.cpp  
  _##
  _##  SNMP++v3.2.14
  _##  -----------------------------------------------
  _##  Copyright (c) 2001-2004 Jochen Katz, Frank Fock
  _##
  _##  This software is based on SNMP++2.6 from Hewlett Packard:
  _##  
  _##    Copyright (c) 1996
  _##    Hewlett-Packard Company
  _##  
  _##  ATTENTION: USE OF THIS SOFTWARE IS SUBJECT TO THE FOLLOWING TERMS.
  _##  Permission to use, copy, modify, distribute and/or sell this software 
  _##  and/or its documentation is hereby granted without fee. User agrees 
  _##  to display the above copyright notice and this license notice in all 
  _##  copies of the software and any documentation of the software. User 
  _##  agrees to assume all liability for the use of the software; 
  _##  Hewlett-Packard and Jochen Katz make no representations about the 
  _##  suitability of this software for any purpose. It is provided 
  _##  "AS-IS" without warranty of any kind, either express or implied. User 
  _##  hereby grants a royalty-free license to any and all derivatives based
  _##  upon this software code base. 
  _##  
  _##  Stuttgart, Germany, Tue Sep  7 21:25:32 CEST 2004 
  _##  
  _##########################################################################*/
/*===================================================================

  Copyright (c) 1999
  Hewlett-Packard Company

  ATTENTION: USE OF THIS SOFTWARE IS SUBJECT TO THE FOLLOWING TERMS.
  Permission to use, copy, modify, distribute and/or sell this software
  and/or its documentation is hereby granted without fee. User agrees
  to display the above copyright notice and this license notice in all
  copies of the software and any documentation of the software. User
  agrees to assume all liability for the use of the software; Hewlett-Packard
  makes no representations about the suitability of this software for any
  purpose. It is provided "AS-IS" without warranty of any kind,either express
  or implied. User hereby grants a royalty-free license to any and all
  derivatives based upon this software code base.


  C O U N T E R 6 4. C P P

  COUNTER64 CLASS IMPLEMENTATION

  DESIGN + AUTHOR:
  Peter E. Mellquist

  DESCRIPTION:
  Implementation for
  Counter64 ( 64 bit counter class).

  LANGUAGE:
  ANSI C++

  OPERATING SYSTEM(S):
  MS-Windows Win32
  BSD UNIX

=====================================================================*/
char counter64_cpp_version[]="@(#) SNMP++ $Id$";

#include "snmp_pp/ctr64.h"
#include <stdio.h>   // for pretty printing...

#ifdef SNMP_PP_NAMESPACE
namespace Snmp_pp {
#endif


#define MAX32 4294967295u

//------------------[ constructor with no value ]------------------------
Counter64::Counter64()
{
  smival.syntax = sNMP_SYNTAX_CNTR64;
  smival.value.hNumber.hipart = 0;
  smival.value.hNumber.lopart = 0;
}

//------------------[ constructor with values ]--------------------------
Counter64::Counter64(unsigned long hi, unsigned long lo)
{
  smival.syntax = sNMP_SYNTAX_CNTR64;
  smival.value.hNumber.hipart = hi;
  smival.value.hNumber.lopart = lo;
}

//------------------[ constructor with low value only ]------------------
Counter64::Counter64(unsigned long lo)
{
  smival.syntax = sNMP_SYNTAX_CNTR64;
  smival.value.hNumber.hipart = 0;
  smival.value.hNumber.lopart = lo;
}

//------------------[ copy constructor ]---------------------------------
Counter64::Counter64( const Counter64 &ctr64 )
{
  smival.syntax = sNMP_SYNTAX_CNTR64;
  smival.value.hNumber.hipart = ctr64.high();
  smival.value.hNumber.lopart = ctr64.low();
}

//------------------[ operator=( const Counter64 &ctr64) ]-------------
// assign a ctr64 to a ctr64
Counter64& Counter64::operator=(const Counter64 &ctr64)
{
  if (this == &ctr64) return *this;  // check for self assignment
  smival.value.hNumber.hipart = ctr64.high();
  smival.value.hNumber.lopart = ctr64.low();
  return *this;
}

//-------------------[ operator=( const unsigned long int i) ]---------
// assign a ul to a ctr64, clears the high part
// and assugns the low part
Counter64& Counter64::operator=( const unsigned long i)
{
   smival.value.hNumber.hipart = 0;
   smival.value.hNumber.lopart = i;
   return *this;
}


//-----------[ c64_to_ld( Counter64 c64) ]-----------------------------
// convert a Counter 64 to a long double
long double Counter64::c64_to_ld( const Counter64 &c64)
{
  long double ld = c64.high();
  ld *= (long double)MAX32 + 1.0l; // gotta be MAX32 + 1 to move it to next pos
  ld += c64.low();
  return ld;
}

//-----------[ c64_to_ld( ) ]------------------------------------------
long double Counter64::c64_to_ld() const
{
  long double ld = high();
  ld *= (long double)MAX32 + 1.0l; // gotta be MAX32 + 1 to move it to next pos
  ld += low();
  return ld;
}


//-----------[ ld_to_c64( long double ld) ]----------------------------
// convert a long double to a Counter64
Counter64 Counter64::ld_to_c64( const long double &ld)
{
  long double high = MAX32 + 1.0l; // look above
  unsigned long h = (unsigned long)(ld / high);
  return Counter64( h, (unsigned long)(ld - (h * high)));
}

//----------[ Counter64::operator+( const Counter64 &c) ]---------------
// add two Counter64s
Counter64 Counter64::operator+( const Counter64 &c) const
{
  long double ldsum = c64_to_ld() + c.c64_to_ld();
  return ld_to_c64( ldsum);
}

//------------[ Counter64::operator-( const Counter64 &c) ]-------------
// subtract two Counter64s
Counter64 Counter64::operator-( const Counter64 &c) const
{
  long double lddiff = c64_to_ld() - c.c64_to_ld();
  return ld_to_c64( lddiff);
}

//------------[ Counter64::operator*( const Counter64 &c) ]-------------
// multiply two Counter64s
Counter64 Counter64::operator*( const Counter64 &c) const
{
  long double ldmult = c64_to_ld() * c.c64_to_ld();
  return ld_to_c64( ldmult);
}

//------------[ Counter64::operator/( const Counter64 &c) ]--------------
// divide two Counter64s
Counter64 Counter64::operator/( const Counter64 &c) const
{
  long double lddiv = c64_to_ld() / c.c64_to_ld();
  return ld_to_c64( lddiv);
}

//-------[ overloaded equivlence test ]----------------------------------
bool operator==(const Counter64 &lhs, const Counter64 &rhs)
{
  return (( lhs.high() == rhs.high()) && ( lhs.low() == rhs.low()));
}

//-------[ overloaded not equal test ]-----------------------------------
bool operator!=(const Counter64 &lhs, const Counter64 &rhs)
{
  return (( lhs.high() != rhs.high()) || ( lhs.low() != rhs.low()));
}

//--------[ overloaded less than ]---------------------------------------
bool operator<(const Counter64 &lhs, const Counter64 &rhs)
{
  return ( (lhs.high() < rhs.high()) ||
	   ((lhs.high() == rhs.high()) && (lhs.low() < rhs.low())));
}

//---------[ overloaded less than or equal ]-----------------------------
bool operator<=(const Counter64 &lhs, const Counter64 &rhs)
{
  return ( (lhs.high() < rhs.high()) ||
	   ((lhs.high() == rhs.high()) && (lhs.low() <= rhs.low())));
}

//---------[ overloaded greater than ]-----------------------------------
bool operator>(const Counter64 &lhs, const Counter64 &rhs)
{
  return ( (lhs.high() > rhs.high()) ||
	   ((lhs.high() == rhs.high()) && (lhs.low() > rhs.low())));
}

//----------[ overloaded greater than or equal ]-------------------------
bool operator>=(const Counter64 &lhs, const Counter64 &rhs)
{
  return ( (lhs.high() > rhs.high()) ||
	   ((lhs.high() == rhs.high()) && (lhs.low() >= rhs.low())));
}

//----------[ return ASCII format ]-------------------------
// TODO:  Fix up to do real 64bit decimal value printing...
//        For now, print > 32-bit values in hex
// 26Nov2002 M.Evstiounin - this method is not thread safe!
const char *Counter64::get_printable() const
{
  char *buf = PP_CONST_CAST(char*, output_buffer);
  if ( high() != 0 )
    sprintf(buf, "0x%lX%08lX", high(), low());
  else
    sprintf(buf, "%lu", low());
  return buf;
}


//----------------[ general Value = operator ]---------------------
SnmpSyntax& Counter64::operator=(const SnmpSyntax &val)
{
  // protect against assignment from itself
  if (this == &val) return *this;

  smival.value.hNumber.lopart = 0;	// pessimsitic - assume no mapping
  smival.value.hNumber.hipart = 0;

  // try to make assignment valid
  if (val.valid())
  {
    switch (val.get_syntax())
    {
      case sNMP_SYNTAX_CNTR64:
	smival.value.hNumber.hipart =
		((Counter64 &)val).smival.value.hNumber.hipart;
	smival.value.hNumber.lopart =
		((Counter64 &)val).smival.value.hNumber.lopart;
	break;

      case sNMP_SYNTAX_CNTR32:
      case sNMP_SYNTAX_TIMETICKS:
      case sNMP_SYNTAX_GAUGE32:
   // case sNMP_SYNTAX_UINT32:		.. indistinguishable from GAUGE32
      case sNMP_SYNTAX_INT32:
	// take advantage of union...
	smival.value.hNumber.lopart = ((Counter64 &)val).smival.value.uNumber;
	smival.value.hNumber.hipart = 0;
	break;
    }
  }
  return *this;
}

// Return the space needed for serialization
int Counter64::get_asn1_length() const
{
  if (smival.value.hNumber.hipart == 0)
  {
    if (smival.value.hNumber.lopart < 0x80)
      return 3;
    else if (smival.value.hNumber.lopart < 0x8000)
      return 4;
    else if (smival.value.hNumber.lopart < 0x800000)
      return 5;
    else if (smival.value.hNumber.lopart < 0x80000000)
      return 6;
    return 7;
  }
  if (smival.value.hNumber.hipart < 0x80)
    return 7;
  else if (smival.value.hNumber.hipart < 0x8000)
    return 8;
  else if (smival.value.hNumber.hipart < 0x800000)
    return 9;
  else if (smival.value.hNumber.hipart < 0x80000000)
    return 10;
  return 11;
}

#ifdef SNMP_PP_NAMESPACE
}; // end of namespace Snmp_pp
#endif 
