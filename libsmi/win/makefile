
#
# makefile --
#
#       Makefile for VC++ (tested with version 6.0) for compiling
#	libsmi and associated tools natively under Win32.
#
# Copyright (c) 2000-2003 Yigal Hochberg, DMH Software.
# Copyright (c) 2000 E. Schoenfelder, Gaertner Datensysteme Braunschweig.
# Copyright (c) 2000 J. Schoenwaelder, Technical University of Braunschweig.
#
# See the file "COPYING" for information on usage and redistribution
# of this file, and for a DISCLAIMER OF ALL WARRANTIES.
#
# @(#) $Id: makefile 3226 2006-02-27 21:48:23Z schoenw $

##
## Project directories -- these may need to be customized for your site
##
## ROOT --	top of the source tree.
## TMPDIR --	location for .obj files.
## TOOLS32 --	location of VC++ compiler installation.
##
ROOT	= .\..
TMPDIR	= .
#TOOLS32	= C:\MSDEV
TOOLS32	= C:\PROGRA~1\MICROS~2\VC98

##
## Installation paths: 
##
## MIBROOT  -- definition for default search path (unix-style)
## MIBDIR   -- same in windows style
## PIBDIR   -- same in windows style
## BINDIR   -- destination for the binaries (smidump.exe, ...)
## LIBDIR   -- destination for the library (smi.lib)
##
MIBROOT = c:/smi/mibs
PIBROOT = c:/smi/pibs
INSTROOT = c:\smi
MIBDIR = $(INSTROOT)\mibs
PIBDIR = $(INSTROOT)\pibs
BINDIR = $(INSTROOT)\bin
LIBDIR = $(INSTROOT)\lib

##
## Visual C++ tools
##
PATH=$(TOOLS32)\bin;$(PATH)

cc32	= cl
CP      = copy
RM      = del
MKDIR	= md

##
## Definitions:
##
# DEBUG = -W3 -Zi -Od /Yd
INCLUDES = -I$(ROOT)\lib -I$(ROOT)\win -I$(TOOLS32)\include
DEFINES = $(DEBUG)

!include <win32.mak>

## 
## Global makefile settings
##
LIBSMIOBJS =					\
	$(TMPDIR)\win.obj			\
	$(TMPDIR)\shhopt.obj			\
	$(TMPDIR)\data.obj			\
	$(TMPDIR)\check.obj			\
	$(TMPDIR)\error.obj			\
	$(TMPDIR)\util.obj			\
	$(TMPDIR)\smi.obj			\
	$(TMPDIR)\parser-smi.obj		\
	$(TMPDIR)\scanner-smi.obj		\
	$(TMPDIR)\parser-sming.obj		\
	$(TMPDIR)\scanner-sming.obj		\
	$(TMPDIR)\snprintf.obj

SMIQUERYOBJS =					\
	$(TMPDIR)\smiquery.obj

SMILINTOBJS =					\
	$(TMPDIR)\smilint.obj

SMIDUMPOBJS =					\
	$(TMPDIR)\smidump.obj			\
	$(TMPDIR)\dump-sming.obj		\
	$(TMPDIR)\dump-smi.obj			\
	$(TMPDIR)\dump-imports.obj		\
	$(TMPDIR)\dump-types.obj		\
	$(TMPDIR)\dump-tree.obj			\
	$(TMPDIR)\dump-mosy.obj			\
	$(TMPDIR)\dump-corba.obj		\
	$(TMPDIR)\dump-netsnmp.obj		\
	$(TMPDIR)\dump-jax.obj			\
	$(TMPDIR)\dump-xml.obj			\
	$(TMPDIR)\dump-cm.obj			\
	$(TMPDIR)\dump-metrics.obj		\
	$(TMPDIR)\dump-identifiers.obj		\
	$(TMPDIR)\dump-python.obj		\
	$(TMPDIR)\dump-perl.obj			\
	$(TMPDIR)\dump-scli.obj			\
	$(TMPDIR)\dump-xsd.obj			\
	$(TMPDIR)\dump-sppi.obj			\
	$(TMPDIR)\dump-sizes.obj		\
	$(TMPDIR)\dump-svg.obj			\
	$(TMPDIR)\dump-compliance.obj		\
	$(TMPDIR)\rea.obj

SMIDIFFOBJS =					\
	$(TMPDIR)\smidiff.obj

SMILIB = smi.lib

##
## Implicit Targets
##
.c.obj:
	$(cc32) $(cdebug) $(cflags) $(INCLUDES) \
		$(DEFINES) -Fo$(TMPDIR)\ $<

{$(ROOT)\lib}.c{$(TMPDIR)}.obj:
	$(cc32) $(cdebug) $(cflags) $(INCLUDES) \
		$(DEFINES) -Fo$(TMPDIR)\ $<

{$(ROOT)\tools}.c{$(TMPDIR)}.obj:
	$(cc32) $(cdebug) $(cflags) $(INCLUDES) \
		$(DEFINES) -Fo$(TMPDIR)\ $<

##
## Targets
##
all:	libs tools

libs:	$(SMILIB)

# special compilation rules
# (flex-src avoids missing unistd.h if _Win32 defined)
#
$(TMPDIR)\scanner-smi.obj : $(ROOT)\lib\scanner-smi.c
	$(cc32) $(cdebug) $(cflags) -D_Win32 -D__STDC__ $(INCLUDES) \
		$(DEFINES) -Fo$(TMPDIR)\ $?

$(TMPDIR)\scanner-sming.obj : $(ROOT)\lib\scanner-sming.c
	$(cc32) $(cdebug) $(cflags) -D_Win32 -D__STDC__ $(INCLUDES) \
		$(DEFINES) -Fo$(TMPDIR)\ $?


$(SMILIB):	$(LIBSMIOBJS)
	$(implib) /out:smi.lib $(LIBSMIOBJS)

tools:	smiquery.exe smilint.exe smidump.exe smidiff.exe

smiquery.exe:	$(SMIQUERYOBJS) $(SMILIB)
	$(cc32) $(DEFINES) /osmiquery.exe $(SMIQUERYOBJS) $(SMILIB)

smilint.exe:	$(SMILINTOBJS) $(SMILIB)
	$(cc32) $(DEFINES) /osmilint.exe $(SMILINTOBJS) $(SMILIB)

smidump.exe:	$(SMIDUMPOBJS) $(SMILIB)
	$(cc32) $(DEFINES) \
		/osmidump.exe $(SMIDUMPOBJS) $(SMILIB)

smidiff.exe:	$(SMIDIFFOBJS) $(SMILIB)
	$(cc32) $(DEFINES) \
		/osmidiff.exe $(SMIDIFFOBJS) $(SMILIB)

clean:
	del *.lib
	del *.obj
	del *.exe
	del *.ilk
	del *.pdb
	del *.mdp

install:	all install-doc install-bin install-lib install-mibs install-pibs

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


install-pibs:
	$(MKDIR) $(PIBDIR)
	$(MKDIR) $(PIBDIR)\ietf
	$(CP) ..\pibs\ietf\*.* $(PIBDIR)\ietf
	$(MKDIR) $(PIBDIR)\tubs
	$(CP) ..\pibs\tubs\*.* $(PIBDIR)\tubs
	$(MKDIR) $(PIBDIR)\site


# EOF: makefile
