#
# Makefile.am --
#
#      Template to generate the toplevel Makefile.in using automake.
#
# Copyright (c) 1999 Frank Strauss, Technical University of Braunschweig.
#
# See the file "COPYING" for information on usage and redistribution
# of this file, and for a DISCLAIMER OF ALL WARRANTIES.
#
# @(#) $Id: Makefile.am 1670 2004-08-09 14:06:04Z strauss $
#

AUTOMAKE_OPTIONS	= no-dependencies foreign

EXTRA_DIST		= ANNOUNCE \
			  win/Makefile.mingw win/README.mingw win/README.win \
			  win/config.h win/config.h.in \
			  win/makefile win/GNUmakefile win/win.c win/win.h \
			  smi.conf-example libsmi.m4 libsmi.pc

SUBDIRS			= lib tools mibs pibs test doc

m4datadir		= $(datadir)/aclocal
m4data_DATA		= libsmi.m4

pkgconfigdir		= $(libdir)/pkgconfig
pkgconfig_DATA		= libsmi.pc
$(pkgconfig_DATA):	config.status

anoncvs:
	RSYNC_RSH=ssh rsync -rlt --delete $(HOME)/.cvs-repository/libsmi anoncvs@anoncvs:/var/lib/cvs
