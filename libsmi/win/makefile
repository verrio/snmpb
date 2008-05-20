#
# makefile --
#
#       Makefile for VC++ (tested with version 6.0) for compiling
#	libsmi and associated tools natively under Win32.
#
# Copyright (c) 2000-2008 Yigal Hochberg, DMH Software.
# Copyright (c) 2000 E. Schoenfelder, Gaertner Datensysteme Braunschweig.
# Copyright (c) 2000 J. Schoenwaelder, Technical University of Braunschweig.
#
# See the file "COPYING" for information on usage and redistribution
# of this file, and for a DISCLAIMER OF ALL WARRANTIES.
#
# @(#) $Id: makefile 8084 2008-04-18 07:51:26Z schoenw $

!include "config.nmake"
!include <win32.mak>


!IFDEF ENABLE_LIBSMI_DLL
CVARSDLL=-DWIN32 -DNULL=0 -D_MT -D_DLL /MD
!ENDIF



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
	$(TMPDIR)\smiquery.obj \

SMILINTOBJS =					\
	$(TMPDIR)\smilint.obj			\

SMIXLATEOBJS =					\
	$(TMPDIR)\smixlate.obj			\
	$(TMPDIR)\dstring.obj			\

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
	$(TMPDIR)\dump-boilerplate.obj		\
	$(TMPDIR)\dump-yang.obj			\
	$(TMPDIR)\fprint.obj			\
	$(TMPDIR)\rea.obj			\
	$(TMPDIR)\fortopat.obj			

SMIDIFFOBJS =					\
	$(TMPDIR)\smidiff.obj			\

SMILIB = smi.lib
SMIDLL = smi.dll

##
## Implicit Targets
##
.c.obj:
	$(cc32) $(cdebug) $(cflags) $(INCLUDES) \
		$(DEFINES) -Fo$(TMPDIR)\ $<

{$(ROOT)\lib}.c{$(TMPDIR)}.obj:
	$(cc32) $(cdebug) $(cflags) $(CVARSDLL) $(INCLUDES) \
		-DYYMALLOC=malloc -DYYFREE=free \
		$(DEFINES) -Fo$(TMPDIR)\ $<

{$(ROOT)\tools}.c{$(TMPDIR)}.obj:
	$(cc32) $(cdebug) $(cflags) $(CVARSDLL) $(INCLUDES) \
		$(DEFINES) -Fo$(TMPDIR)\ $<

##
## Targets
##
all:	libs tools

!IFDEF ENABLE_LIBSMI_DLL
libs:	$(SMILIB) $(SMIDLL)
!ELSE
libs:	$(SMILIB)
!ENDIF


# special compilation rules
# (flex-src avoids missing unistd.h if _Win32 defined)
#
$(TMPDIR)\scanner-smi.obj : $(ROOT)\lib\scanner-smi.c
	$(cc32) $(cdebug) $(cflags) $(CVARSDLL) -D_Win32 -D__STDC__ $(INCLUDES) \
		-DYY_NEVER_INTERACTIVE \
		$(DEFINES) -Fo$(TMPDIR)\ $?

$(TMPDIR)\scanner-sming.obj : $(ROOT)\lib\scanner-sming.c
	$(cc32) $(cdebug) $(cflags) $(CVARSDLL) -D_Win32 -D__STDC__ $(INCLUDES) \
		-DYY_NEVER_INTERACTIVE \
		$(DEFINES) -Fo$(TMPDIR)\ $?

$(SMIDLL): $(SMILIB)
        $(link)  $(dlllflags) $(conlibsdll) /NODEFAULTLIB /machine:i386 msvcrt.lib shell32.lib version.lib /DEBUG /DEF:smi.def /OUT:smi.dll  /IMPLIB:smi.lib   $(LIBSMIOBJS)
!IF "$(MSVC_VARIANT)" == "MSVC2005" || "$(MSVC_VARIANT)" == "MSVC2005EE" || "$(MSVC_VARIANT)" == "DOTNET20"
	mt.exe -nologo -manifest "smi.dll.manifest" -outputresource:smi.dll;1
!ENDIF

$(SMILIB):	$(LIBSMIOBJS)
	$(implib) /out:smi.lib $(LIBSMIOBJS)

tools:	smiquery.exe smilint.exe smixlate.exe smidump.exe smidiff.exe

smiquery.exe:	$(SMIQUERYOBJS) $(SMILIB)
	$(cc32) $(DEFINES) /osmiquery.exe $(SMIQUERYOBJS) $(SMILIB)
!IF "$(MSVC_VARIANT)" == "MSVC2005" || "$(MSVC_VARIANT)" == "MSVC2005EE" || "$(MSVC_VARIANT)" == "DOTNET20"
	mt.exe -nologo -manifest "smiquery.exe.manifest" -outputresource:smiquery.exe;1
!ENDIF

smilint.exe:	$(SMILINTOBJS) $(SMILIB)
	$(cc32) $(DEFINES) /osmilint.exe $(SMILINTOBJS) $(SMILIB)
!IF "$(MSVC_VARIANT)" == "MSVC2005" || "$(MSVC_VARIANT)" == "MSVC2005EE" || "$(MSVC_VARIANT)" == "DOTNET20"
	mt.exe -nologo -manifest "smilint.exe.manifest" -outputresource:smilint.exe;1
!ENDIF

smidump.exe:	$(SMIDUMPOBJS) $(SMILIB)
	$(cc32) $(DEFINES) \
		/osmidump.exe $(SMIDUMPOBJS) $(SMILIB)
!IF "$(MSVC_VARIANT)" == "MSVC2005" || "$(MSVC_VARIANT)" == "MSVC2005EE" || "$(MSVC_VARIANT)" == "DOTNET20"
	mt.exe -nologo -manifest "smidump.exe.manifest" -outputresource:smidump.exe;1
!ENDIF

smidiff.exe:	$(SMIDIFFOBJS) $(SMILIB)
	$(cc32) $(DEFINES) \
		/osmidiff.exe $(SMIDIFFOBJS) $(SMILIB)
!IF "$(MSVC_VARIANT)" == "MSVC2005" || "$(MSVC_VARIANT)" == "MSVC2005EE" || "$(MSVC_VARIANT)" == "DOTNET20"
	mt.exe -nologo -manifest "smidiff.exe.manifest" -outputresource:smidiff.exe;1
!ENDIF

smixlate.exe:	$(SMIXLATEOBJS) $(SMILIB)
	$(cc32) $(DEFINES) \
		/osmixlate.exe $(SMIXLATEOBJS) $(SMILIB)
!IF "$(MSVC_VARIANT)" == "MSVC2005" || "$(MSVC_VARIANT)" == "MSVC2005EE" || "$(MSVC_VARIANT)" == "DOTNET20"
	mt.exe -nologo -manifest "smixlate.exe.manifest" -outputresource:smixlate.exe;1
!ENDIF

clean:
	rm -f *.lib
	rm -f *.obj
	rm -f *.exe
	rm -f *.ilk
	rm -f *.pdb
	rm -f *.mdp
	rm -f *.dll
	rm -f *.exp

install:	all install-doc install-bin install-lib install-mibs install-pibs

install-doc:
	$(MKDIR) $(INSTROOT)
	$(CP) ..\README $(INSTROOT)
	$(CP) ..\COPYING $(INSTROOT)
	$(CP) README.win $(INSTROOT)
	$(MKDIR) $(INSTROOT)\doc
	$(CP) ..\doc\*.txt $(INSTROOT)\doc
	-$(CP) ..\doc\*.ps $(INSTROOT)\doc

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

# !include "local.nmake"

#
# Local Variables:
# mode: makefile
# make: "nmake"
# eval: (set (make-local-variable 'this-file) (file-name-nondirectory buffer-file-name))
# eval: (set (make-local-variable 'compile-command) (concat make " -k -f " this-file))
# End:
#


# EOF: makefile
