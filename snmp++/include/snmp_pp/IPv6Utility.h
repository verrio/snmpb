/*_############################################################################
  _## 
  _##  IPv6Utility.h  
  _##
  _##  SNMP++v3.2.24
  _##  -----------------------------------------------
  _##  Copyright (c) 2001-2009 Jochen Katz, Frank Fock
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
  _##  Stuttgart, Germany, Fri May 29 22:35:14 CEST 2009 
  _##  
  _##########################################################################*/
/*
 * Copyright (C) 2004, 2005  Internet Systems Consortium, Inc. ("ISC")
 * Copyright (C) 1996-2001  Internet Software Consortium.
 *
 * Permission to use, copy, modify, and distribute this software for any
 * purpose with or without fee is hereby granted, provided that the above
 * copyright notice and this permission notice appear in all copies.
 *
 * THE SOFTWARE IS PROVIDED "AS IS" AND ISC DISCLAIMS ALL WARRANTIES WITH
 * REGARD TO THIS SOFTWARE INCLUDING ALL IMPLIED WARRANTIES OF MERCHANTABILITY
 * AND FITNESS.  IN NO EVENT SHALL ISC BE LIABLE FOR ANY SPECIAL, DIRECT,
 * INDIRECT, OR CONSEQUENTIAL DAMAGES OR ANY DAMAGES WHATSOEVER RESULTING FROM
 * LOSS OF USE, DATA OR PROFITS, WHETHER IN AN ACTION OF CONTRACT, NEGLIGENCE
 * OR OTHER TORTIOUS ACTION, ARISING OUT OF OR IN CONNECTION WITH THE USE OR
 * PERFORMANCE OF THIS SOFTWARE.
 */
/*===================================================================

  SNMP++ IPV6Utility.h

  DESCRIPTION:
  This module contains the Utility functions for IPV6 support functions 
  required for WIN32 environment

  Adapted and integrated into snmp++ by Ragavan Tetchinamourty

=====================================================================*/
// $Id: $

#ifndef _IPV6UTILITY_H
#define _IPV6UTILITY_H

#include "snmp_pp/config_snmp_pp.h"

#if defined(_MSC_VER) && defined(SNMP_PP_IPv6)

#define NS_INT16SZ	 2
#define NS_IN6ADDRSZ	16
#define EAFNOSUPPORT WSAEAFNOSUPPORT
#define ENOSPC 28


/*
 * WARNING: Don't even consider trying to compile this on a system where
 * sizeof(int) < 4.  sizeof(int) > 4 is fine; all the world's not a VAX.
 */

static const char *inet_ntop4(const unsigned char *src, char *dst,
			      size_t size);

#ifdef AF_INET6
static const char *inet_ntop6(const unsigned char *src, char *dst,
			      size_t size);
#endif

const char *
inet_ntop(int af, const void *src, char *dst, size_t size);


static int
snprintf (char *str, int n, char *fmt, ...);

//extern const char *inet_pton(int, const char*, void*);

#define NS_INT16SZ	 2
#define NS_INADDRSZ	 4
#define NS_IN6ADDRSZ	16

/*
 * WARNING: Don't even consider trying to compile this on a system where
 * sizeof(int) < 4.  sizeof(int) > 4 is fine; all the world's not a VAX.
 */

static int inet_pton4(const char *src, unsigned char *dst);
#ifdef AF_INET6
static int inet_pton6(const char *src, unsigned char *dst);
#endif

int inet_pton(int af, const char *src, void *dst);

#endif // defined(_MSC_VER) && defined(SNMP_PP_IPv6)

#endif // IPV6UTILITY_H
