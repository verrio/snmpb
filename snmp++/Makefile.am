  ############################################################################
  ## 
  ##  Makefile.am  
  ##
  ##  SNMP++ v3.3
  ##  -----------------------------------------------
  ##  Copyright (c) 2001-2013 Jochen Katz, Frank Fock
  ##
  ##  This software is based on SNMP++2.6 from Hewlett Packard:
  ##  
  ##    Copyright (c) 1996
  ##    Hewlett-Packard Company
  ##  
  ##  ATTENTION: USE OF THIS SOFTWARE IS SUBJECT TO THE FOLLOWING TERMS.
  ##  Permission to use, copy, modify, distribute andor sell this software 
  ##  andor its documentation is hereby granted without fee. User agrees 
  ##  to display the above copyright notice and this license notice in all 
  ##  copies of the software and any documentation of the software. User 
  ##  agrees to assume all liability for the use of the software; 
  ##  Hewlett-Packard and Jochen Katz make no representations about the 
  ##  suitability of this software for any purpose. It is provided 
  ##  "AS-IS" without warranty of any kind, either express or implied. User 
  ##  hereby grants a royalty-free license to any and all derivatives based
  ##  upon this software code base. 
  ##  
  ##########################################################################*

AM_CPPFLAGS = -I$(abs_top_srcdir) -I$(abs_top_srcdir)/include $(PTHREAD_CFLAGS) @CLIBFLAGS@
SUBDIRS = include src consoleExamples
ACLOCAL_AMFLAGS = -I m4

EXTRA_DIST = snmp++-3.pc.in libsnmp.h CHANGES.snmp++v3 readme.txt README.v3 README.autoconf doc_config 

pkgconfigdir = $(libdir)/pkgconfig
pkgconfig_DATA = snmp++-3.pc

library_includedir = $(includedir)
library_include_HEADERS= libsnmp.h
