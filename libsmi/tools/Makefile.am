#
# Makefile.am --
#
#      Template to generate the tools Makefile.in using automake.
#
# Copyright (c) 1999 Frank Strauss, Technical University of Braunschweig.
#
# See the file "COPYING" for information on usage and redistribution
# of this file, and for a DISCLAIMER OF ALL WARRANTIES.
#
# @(#) $Id: Makefile.am 8090 2008-04-18 12:56:29Z strauss $
#

INCLUDES		= -I../lib

EXTRA_DIST		= smidump.h shhopt.h \
			  $(man_MANS) \
			  dump-fig.c \
			  dump-svg-script.js

bin_PROGRAMS		= smiquery smilint smidump smidiff smixlate

bin_SCRIPTS		= smistrip smicache

man_MANS		= smiquery.1 smilint.1 smidump.1 smidiff.1 \
			  smistrip.1 smicache.1 smixlate.1

smiquery_SOURCES	= smiquery.c shhopt.c
smiquery_LDADD		= ../lib/libsmi.la

smilint_SOURCES		= smilint.c shhopt.c
smilint_LDADD		= ../lib/libsmi.la

smixlate_SOURCES	= smixlate.c shhopt.c dstring.h dstring.c
smixlate_LDADD		= ../lib/libsmi.la

dump-svg-script.h: dump-svg-script.js
	(echo "const char *code =";cat dump-svg-script.js | sed -e 's/\\/&&/g;s/"/\\"/g;s/^/"/;s/$$/\\n"/'; echo ";") > dump-svg-script.h

dump-svg.o: dump-svg-script.h

smidump_SOURCES		= smidump.c smidump.h shhopt.c \
			  dump-sming.c dump-smi.c dump-yang.c \
			  dump-imports.c dump-types.c dump-tree.c \
			  dump-mosy.c dump-corba.c dump-netsnmp.c \
			  dump-jax.c dump-xml.c dump-identifiers.c \
			  dump-metrics.c dump-cm.c dump-python.c \
			  dump-perl.c dump-scli.c dump-xsd.c dump-sppi.c \
			  dump-sizes.c dump-svg-script.h dump-svg.c \
			  rea.h rea.c dump-compliance.c dump-boilerplate.c \
			  dstring.h dstring.c \
			  fortopat.h fortopat.c fprint.h fprint.c

smidump_LDADD		= ../lib/libsmi.la -lm

smidiff_SOURCES		= smidiff.c shhopt.c
smidiff_LDADD		= ../lib/libsmi.la

MOSTLYCLEANFILES	= dump-svg-script.h
