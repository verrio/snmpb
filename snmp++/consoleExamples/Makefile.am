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

bin_PROGRAMS =  snmpGet snmpSet snmpNext snmpNextAsync snmpWalk snmpBulk \
                snmpTraps receive_trap snmpInform snmpPasswd snmpWalkThreads \
                snmpDiscover

noinst_PROGRAMS =	test_app

snmpGet_SOURCES =	snmpGet.cpp
snmpGet_LDADD =		$(abs_top_builddir)/src/libsnmp++.la @LINKFLAGS@

snmpSet_SOURCES =	snmpSet.cpp
snmpSet_LDADD =		$(abs_top_builddir)/src/libsnmp++.la @LINKFLAGS@

snmpNext_SOURCES =	snmpNext.cpp
snmpNext_LDADD =	$(abs_top_builddir)/src/libsnmp++.la @LINKFLAGS@

snmpNextAsync_SOURCES =	snmpNextAsync.cpp
snmpNextAsync_LDADD =	$(abs_top_builddir)/src/libsnmp++.la @LINKFLAGS@

snmpWalk_SOURCES =	snmpWalk.cpp
snmpWalk_LDADD =	$(abs_top_builddir)/src/libsnmp++.la @LINKFLAGS@

snmpBulk_SOURCES =	snmpBulk.cpp
snmpBulk_LDADD =	$(abs_top_builddir)/src/libsnmp++.la @LINKFLAGS@

snmpTraps_SOURCES =	snmpTraps.cpp
snmpTraps_LDADD =	$(abs_top_builddir)/src/libsnmp++.la @LINKFLAGS@

receive_trap_SOURCES =	receive_trap.cpp
receive_trap_LDADD =	$(abs_top_builddir)/src/libsnmp++.la @LINKFLAGS@

snmpInform_SOURCES =	snmpInform.cpp
snmpInform_LDADD =	$(abs_top_builddir)/src/libsnmp++.la @LINKFLAGS@

snmpPasswd_SOURCES =	snmpPasswd.cpp
snmpPasswd_LDADD =	$(abs_top_builddir)/src/libsnmp++.la @LINKFLAGS@

snmpWalkThreads_SOURCES =	snmpWalkThreads.cpp
snmpWalkThreads_LDADD =	$(abs_top_builddir)/src/libsnmp++.la @LINKFLAGS@

snmpDiscover_SOURCES =	snmpDiscover.cpp
snmpDiscover_LDADD =	$(abs_top_builddir)/src/libsnmp++.la @LINKFLAGS@

test_app_SOURCES =	test_app.cpp
test_app_LDADD =	$(abs_top_builddir)/src/libsnmp++.la @LINKFLAGS@

EXTRA_DIST = Makefile.dunix Makefile.bcc Makefile.linux Makefile.solaris common.mk