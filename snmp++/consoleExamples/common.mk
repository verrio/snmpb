  ############################################################################
  ## 
  ##  common.mk  
  ##
  ##  SNMP++v3.2.14
  ##  -----------------------------------------------
  ##  Copyright (c) 2001-2004 Jochen Katz, Frank Fock
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
  ##  Stuttgart, Germany, Tue Sep  7 21:25:32 CEST 2004 
  ##  
  ##########################################################################*

GET = snmpGet 
GETOBJS = snmpGet.o

SET = snmpSet 
SETOBJS = snmpSet.o

NEXT = snmpNext
NEXTOBJS = snmpNext.o

NEXTASYNC = snmpNextAsync
NEXTASYNCOBJS = snmpNextAsync.o

WALK = snmpWalk
WALKOBJS = snmpWalk.o

BULKWALK = snmpBulk
BULKWALKOBJS = snmpBulk.o

TRAPSEND = snmpTraps
TRAPSENDOBJS = snmpTraps.o

TRAPRECEIVE = receive_trap
TRAPRECEIVEOBJS = receive_trap.o

INFORM = snmpInform
INFORMOBJS = snmpInform.o 

PASSWD = snmpPasswd
PASSWDOBJS = snmpPasswd.o 

WALKTHREADS = snmpWalkThreads
WALKTHREADSOBJS = snmpWalkThreads.o

TESTAPP = test_app
TESTAPPOBJS = test_app.o

DISCOVER = snmpDiscover
DISCOVEROBJS = snmpDiscover.o

TARGETS =  $(GET) $(SET) $(NEXTASYNC) $(NEXT) $(WALK) \
	   $(BULKWALK) $(TRAPSEND) $(TRAPRECEIVE) $(INFORM) $(PASSWD) \
	   $(WALKTHREADS) $(DISCOVER)

#
# Installation directories
#
INSTBINPATH=/usr/local/bin

SNMPPLUSDIR = ..
SNMPLIBPATH = $(SNMPPLUSDIR)/lib
LIBDESDIR	= ../../libdes
LIBTOMCRYPTDIR	= ../../crypt

SNMPLIBS	= $(wildcard $(SNMPLIBPATH)/libsnmp++*)
LIBSNMP		= $(SNMPLIBPATH)/libsnmp++.a
LIBSNMPSH	= $(SNMPLIBPATH)/libsnmp++.so
LIBDES		= $(LIBDESDIR)/libdes.a
LIBTOMCRYPT	= $(LIBTOMCRYPTDIR)/libtomcrypt.a

HEADERS = $(wildcard $(SNMPPLUSDIR)/include/snmp_pp/*.h)

# verify that snmp++ lib is in ../lib
ifeq ($(SNMPLIBS),)
$(error Error: Need snmp++ library in $(SNMPLIBPATH))
endif

# Set crypto lib to use
ifndef CRYPTOLINKLIBS
ifneq ($(wildcard $(LIBTOMCRYPT)),)
USERTEXT	= INFO: Found libtomcrypt.
CRYPTOLINKLIBS	= -L$(LIBTOMCRYPTDIR) -ltomcrypt
else
ifneq ($(wildcard $(LIBDES)),)
USERTEXT	= INFO: Found libdes.
CRYPTOLINKLIBS	= -L$(LIBDESDIR) -ldes
else
USERTEXT	= INFO: No crypto libs found, SNMPv3 cannot be used.
CRYPTOLINKLIBS	=
endif
endif
endif

LINKLIBS	= -L$(SNMPLIBPATH) -lsnmp++ $(CRYPTOLINKLIBS)

all: checklib $(TARGETS) $(TESTAPP)

install: all
	install -d $(DESTDIR)$(INSTBINPATH)
	install $(TARGETS) $(DESTDIR)$(INSTBINPATH)

checklib:
	@echo $(USERTEXT)

%.o:	%.cpp
	$(CXX) $(CFLAGS) -o $@ -c $<

.c.o: 
	$(CC) $(CFLAGS) -c $<

.C.o: 
	$(CXX) $(CFLAGS) -c $<

.cpp.o: 
	$(CXX) $(CFLAGS) -c $<

%:	%.o $(SNMPLIBS)
	$(CXX) $(LDFLAGS) $< $(LINKLIBS) -o $@


#
# Dependencies:
#

$(GETOBJS): $(HEADERS)

$(SETOBJS): $(HEADERS)

$(NEXTOBJS): $(HEADERS)

$(NEXTASYNCOBJS): $(HEADERS)

$(WALKOBJS): $(HEADERS)

$(WALKTHREADSOBJS): $(HEADERS)

$(BULKWALKOBJS): $(HEADERS)

$(TRAPSENDOBJS): $(HEADERS)

$(TRAPRECEIVEOBJS): $(HEADERS)

$(INFORMOBJS): $(HEADERS)

$(PASSWDOBJS): $(HEADERS)

$(TESTAPPOBJS): $(HEADERS)

$(DISCOVEROBJS): $(HEADERS)

strip:	$(TARGETS) $(TESTAPP)
	-strip $(TARGETS) $(TESTAPP)

clean:
	-rm -f *.o *.a *~ core
	-rm -rf ptrepository cxx_repository/  SunWS_cache/

clobber: clean
	-rm -f $(TARGETS) $(TESTAPP) snmpv3_boot_counter

