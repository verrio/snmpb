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


LIBDESDIR	= ../../libdes
LIBTOMCRYPTDIR	= ../../crypt

PP_INC		= ../include

CINCDIRS	= -I$(PP_INC) -I./ -I$(LIBDESDIR) -I$(LIBTOMCRYPTDIR)

# snmp++ lib headers
HEADERS		= $(wildcard $(PP_INC)/snmp_pp/*.h)

# snmp++ lib sources
PP_SRCS		= $(wildcard *.cpp)

#
#  Object Files produced
#
OBJS		= $(PP_SRCS:.cpp=.o)
OBJS_SHARED	= $(PP_SRCS:.cpp=_sh.o)

#
#  Libraries:  dependencies and produced
#
LIBPATH = ../lib
LIBSNMPPLUS_SHARED = $(LIBPATH)/libsnmp++.so
LIBSNMPPLUS = $(LIBPATH)/libsnmp++.a
LIBSNMPX11 = $(LIBPATH)/libsnmpx11.a

#
# Installation directories
#
ifndef INSTPREFIX
INSTPREFIX	= /usr/local
endif

INSTLIBPATH	= $(INSTPREFIX)/lib
INSTINCPATH	= $(INSTPREFIX)/include

#
#  Here for a quick sanity check upon completing a build...
#

.SUFFIXES: .cpp .C

%.o:	%.cpp
	$(CC) $(CFLAGS) -o $@ -c $<

%x11.o: %.cpp
	$(CC) -DSNMPX11 -I/usr/include/X11R5 $(CFLAGS) -c $< -o $@

%.o:	%.C
	$(CC) $(CFLAGS) -o $@ -c $<

%_sh.o:	%.cpp
	$(CC) $(SHARED) $(CFLAGS) -o $@ -c $<

%_sh.o:	%.C
	$(CC) $(SHARED) $(CFLAGS) -o $@ -c $<

#
#  Build rules
#
all:  $(LIBSNMPPLUS) $(LIBSNMPPLUS_SHARED) # $(LIBSNMPX11)

lib: $(LIBSNMPPLUS)

shlib: $(LIBSNMPPLUS_SHARED)

$(LIBSNMPPLUS): $(OBJS)
	ar -rv $(LIBSNMPPLUS) $(OBJS)	

$(LIBSNMPPLUS_SHARED): $(OBJS_SHARED)
	$(CC) $(SHARED) $(LDFLAGS) $(OBJS_SHARED) -o $@ 

$(LIBSNMPX11): $(OBJS:.o=x11.o)
	ar -rv $(LIBSNMPX11) $(OBJS:.o=x11.o)

clean:
	-rm -f core *.o *.rpo *~ a.out ../include/snmp_pp/*~

clobber: clean
	-rm -f $(LIBSNMPPLUS) $(LIBSNMPX11) $(LIBSNMPPLUS_SHARED)

install: all
	install -d $(DESTDIR)$(INSTLIBPATH)
	install -d $(DESTDIR)$(INSTINCPATH)/snmp_pp/
	install $(LIBSNMPPLUS) $(DESTDIR)$(INSTLIBPATH)
ifneq ($(wildcard $(LIBSNMPPLUS_SHARED)),)
	install $(LIBSNMPPLUS_SHARED) $(DESTDIR)$(INSTLIBPATH)
endif
	install $(HEADERS) $(DESTDIR)$(INSTINCPATH)/snmp_pp/

#
#  Dependency rules
#
$(P1OBJ): $(HEADERS)

$(OBJS): $(HEADERS)

#dependencies:	$(PP_SRCS) $(HEADERS)
#	$(CC) -MM $(PP_SRCS) $(CINCDIRS)      > dependencies
#
#ifneq ($(wildcard dependencies),)
#include dependencies
#endif
