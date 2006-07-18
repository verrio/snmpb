/*_############################################################################
  _## 
  _##  collect.h  
  _##
  _##  SNMP++v3.2.21
  _##  -----------------------------------------------
  _##  Copyright (c) 2001-2006 Jochen Katz, Frank Fock
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
  _##  Stuttgart, Germany, Fri Jun 16 17:48:57 CEST 2006 
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
  purpose. It is provided "AS-IS without warranty of any kind,either express
  or implied. User hereby grants a royalty-free license to any and all
  derivatives based upon this software code base.


  SNMP++ C O L L E C T . H

  COLLECTION CLASS DEFINITION

  DESIGN + AUTHOR:
  Peter E Mellquist

  LANGUAGE:
  ANSI C++

  OPERATING SYSTEMS:
  Win 32
  BSD UNIX

  DESCRIPTION:
  Simple Collection classes for SNMP++ classes.

=====================================================================*/
// $Id$

#ifndef _COLLECTION_H_
#define _COLLECTION_H_

#include "snmp_pp/config_snmp_pp.h"

#ifdef SNMP_PP_NAMESPACE
namespace Snmp_pp {
#endif

#ifndef TRUE
#define TRUE 1
#endif

#ifndef FALSE
#define FALSE 0
#endif

#define MAXT 25     // elements per block

#ifdef SNMP_PP_NAMESPACE
}; // end of namespace Snmp_pp
#endif 

// If you have problems with the collection code:
// 1. Send a mail to katz@agentpp.com with details about the used
//    compile flags, compiler (for example g++ -dumpspecs),... 
//    so we can change the default behaviour for your system
// 2. comment in the define _OLD_TEMPLATE_COLLECTION in
//    config_snmp_pp.h
#ifdef _OLD_TEMPLATE_COLLECTION

#include "snmp_pp/collect2.h"

#else

#include "snmp_pp/collect1.h"

#endif

#endif  // _COLLECTION_H_

