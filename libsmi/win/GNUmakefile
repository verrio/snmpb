#
# GNUmakefile --
#
#	GNU Makefile for w32 VC++ environment.
# 
# Copyright (c) 2000-2005 Yigal Hochberg, DMH Software.
# Copyright (c) 2000 E. Schoenfelder, Gaertner Datensysteme Braunschweig.
# Copyright (c) 2000 J. Schoenwaelder, Technical University of Braunschweig.
#
# See the file "COPYING" for information on usage and redistribution
# of this file, and for a DISCLAIMER OF ALL WARRANTIES.
#
# @(#) $Id: GNUmakefile 2409 2005-05-30 13:14:21Z strauss $

##
## Project directories -- these may need to be customized for your site
##
## ROOT --	top of the source tree.
## TMPDIR --	location for .obj files.
## TOOLS32 --	location of VC++ compiler installation.
##
ROOT	= ./..
TMPDIR	= .
TOOLS32	= C:\PROGRA~1\MICROS~2\VC98

##
## Installation paths: 
##
## MIBROOT  -- definition for default search path (unix-style)
## MIBDIR   -- same in windows style
## BINDIR   -- destination for the binaries (smidump.exe, ...)
## LIBDIR   -- destination for the library (smi.lib)
##
MIBROOT = c:/smi/mibs
INSTROOT = c:\smi
MIBDIR = $(INSTROOT)\mibs
BINDIR = $(INSTROOT)\bin
LIBDIR = $(INSTROOT)\lib

##
## This include is for smidump_SOURCES definition.
##
include ../tools/Makefile.am

##
## Visual C++ tools
##
PATH := $(TOOLS32)\bin;$(PATH)

cc32	= cl
implib	= lib
CC      = $(cc32)
CFLAGS  = -nologo -W3 -DYY_NO_UNISTD_H
CC_INCL = #-I$(TOOLS32)/include
AR      = $(implib)
ARFLAGS = -nologo
CP      = copy
RM      = del
MKDIR	= md

##
## Definitions:
##
#DEBUG = -W3 -Zi -Od /Yd
INCLUDES = -I$(ROOT)/lib -I$(ROOT)/win $(CC_INCL)
DEFINES = $(DEBUG)

##
## Library objects
##
LIBSMIOBJS = $(patsubst %.c, %.obj, $(notdir $(wildcard ../lib/*.c))) win.obj \
	     shhopt.obj

##
## Tool objects
##
SMIQUERYOBJS = smiquery.obj
SMILINTOBJS  = smilint.obj
SMIDUMPOBJS  = $(filter-out %.h, $(smidump_SOURCES:.c=.obj))

##
## Targets
##
all:		libs tools

libs:		smi.lib
tools:		smiquery.exe smilint.exe smidump.exe

smi.lib:	$(LIBSMIOBJS)
smiquery.exe:	$(SMIQUERYOBJS) smi.lib
smilint.exe:	$(SMILINTOBJS) smi.lib
smidump.exe:	$(SMIDUMPOBJS) smi.lib

clean:
	del *.lib
	del *.obj
	del *.exe
	del *.ilk
	del *.pdb
	del *.mdp

install:	all install-doc install-bin install-lib install-mibs

install-doc:
	$(MKDIR) $(INSTROOT)
	$(CP) ..\README $(INSTROOT)
	$(CP) ..\COPYING $(INSTROOT)
	$(CP) README.win $(INSTROOT)
	$(MKDIR) $(INSTROOT)\doc
	$(CP) ..\doc\*.txt $(INSTROOT)\doc
	$(CP) ..\doc\*.ps $(INSTROOT)\doc

install-bin:
	$(MKDIR) $(BINDIR)
	$(CP) *.exe $(BINDIR)

install-lib:
	$(MKDIR) $(LIBDIR)
	$(CP) *.lib $(LIBDIR)

install-mibs:
	$(MKDIR) $(MIBDIR)
	$(MKDIR) $(MIBDIR)\iana
	$(CP) ..\mibs\iana\*.* $(MIBDIR)\iana
	$(MKDIR) $(MIBDIR)\ietf
	$(CP) ..\mibs\ietf\*.* $(MIBDIR)\ietf
	$(MKDIR) $(MIBDIR)\irtf
	$(CP) ..\mibs\irtf\*.* $(MIBDIR)\irtf
	$(MKDIR) $(MIBDIR)\tubs
	$(CP) ..\mibs\tubs\*.* $(MIBDIR)\tubs
	$(MKDIR) $(MIBDIR)\site

##
## Generic rules
##
vpath %.c ../lib
vpath %.c ../tools

%.obj : %.c
	$(CC) $(CFLAGS) -c $(INCLUDES) $(DEFINES) -Fo$@ $<

%.exe :
	$(CC) $(CFLAGS) $(DEFINES) -o $(notdir $@) $^

%.lib :
	$(AR) $(ARFLAGS) -out:$(notdir $@) $^ 

##
## Special compilation rules for scanner files
##
scanner-objs = scanner-smi.obj scanner-sming.obj

$(scanner-objs) : %.obj : %.c
	$(CC) -c $(CFLAGS) -D__STDC__ $(INCLUDES) $(DEFINES) -Fo$@ $<

