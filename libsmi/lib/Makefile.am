#
# Makefile.am --
#
#      Template to generate libsmi Makefile.in using automake.
#
# Copyright (c) 1999 Frank Strauss, Technical University of Braunschweig.
#
# See the file "COPYING" for information on usage and redistribution
# of this file, and for a DISCLAIMER OF ALL WARRANTIES.
#
# @(#) $Id: Makefile.am 1817 2014-10-13 13:20:45Z schoenw $
#

EXTRA_DIST		= parser-yang.y parser-sming.y parser-smi.y \
			  scanner-yang.l scanner-sming.l scanner-smi.l \
			  common.h errormacros.h error.h util.h snprintf.h \
			  smi.h smi-data.h smi-check.h \
			  yang.h yang-data.h yang-check.h \
			  yang-complex-types.h \
			  smi2yang.h \
			  scanner-smi.h parser-smi.h parser-smi.tab.h \
			  scanner-sming.h parser-sming.h parser-sming.tab.h \
			  scanner-yang.h parser-yang.h parser-yang.tab.h \
			  $(man_MANS)
include_HEADERS		= smi.h yang.h
CLEANFILES		= parser-smi.output parser-sming.output \
			  parser-yang.output
MAINTAINERCLEANFILES	= parser-smi.c parser-sming.c parser-yang.c \
			  scanner-smi.c scanner-sming.c scanner-yang.c \
			  parser-smi.tab.h parser-sming.tab.h parser-yang.tab.h \
			  errormacros.h
man_MANS		= libsmi.3 smi_config.3 smi_module.3 smi_macro.3 \
			  smi_node.3 smi_type.3 smi_render.3 smi_util.3 \
			  smi_class.3 smi_attribute.3 smi_identity.3 \
			  smi_event.3 \
			  yang_node.3
lib_LTLIBRARIES		= libsmi.la
libsmi_la_SOURCES	= common.c error.c util.c snprintf.c \
			  smi.c smi-data.c smi-check.c \
			  yang.c yang-data.c yang-check.c \
			  yang-complex-types.c \
			  smi2yang.c \
		  	  parser-smi.c scanner-smi.c \
		  	  parser-sming.c scanner-sming.c \
		  	  parser-yang.c scanner-yang.c
libsmi_la_LDFLAGS	= -version-info @VERSION_LIBTOOL@

parser-smi.tab.h: parser-smi.y scanner-smi.h parser-smi.h error.h
	$(BISON) --defines=parser-smi.tab.h -t -psmi -o parser-smi.c parser-smi.y

parser-sming.tab.h: parser-sming.y scanner-sming.h parser-sming.h error.h
	$(BISON) --defines=parser-sming.tab.h -t -psming -o parser-sming.c parser-sming.y

parser-yang.tab.h: parser-yang.y scanner-yang.h parser-yang.h error.h
	$(BISON) --defines=parser-yang.tab.h -t -pyang -o parser-yang.c parser-yang.y

scanner-smi.c: scanner-smi.l scanner-smi.h parser-smi.tab.h
	$(FLEX) -Cfe -Psmi -t -o scanner-smi.c scanner-smi.l > scanner-smi.c

scanner-sming.c: scanner-sming.l scanner-sming.h parser-sming.tab.h
	$(FLEX) -Cfe -Psming -t -o scanner-sming.c scanner-sming.l > scanner-sming.c

scanner-yang.c: scanner-yang.l scanner-yang.h parser-yang.tab.h
	$(FLEX) -Cfe -Pyang -t -o scanner-yang.c scanner-yang.l > scanner-yang.c

common.c smi.c smi-data.c yang.c yang-data.c: common.h

parser-smi.c: parser-smi.tab.h
parser-sming.c: parser-sming.tab.h
parser-yang.tab.h: parser-yang.tab.h

common.h: util.h error.h

error.h: errormacros.h

errormacros.h: error.c
	cat error.c | grep ERR_ | \
	sed -e 's/^.*\(ERR_[a-zA-Z0-9_]*\).*$$/\1/' | \
	awk '{printf "#define %-50s %d\n", $$1, NR-1}' > errormacros.h

