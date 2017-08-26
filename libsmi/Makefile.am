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
# @(#) $Id: Makefile.am 1820 2014-10-13 15:06:41Z schoenw $
#

AUTOMAKE_OPTIONS	= no-dependencies foreign

EXTRA_DIST		= ANNOUNCE \
			  win/README.win win/config.h win/config.h.in \
			  win/GNUmakefile win/win.c win/win.h \
			  win/makefile win/config.nmake win/smi.def \
			  win/Makefile.mingw win/README.mingw \
			  smi.conf-example libsmi.m4 libsmi.pc

SUBDIRS			= lib

m4datadir		= $(datadir)/aclocal
m4data_DATA		= libsmi.m4

pkgconfigdir		= $(libdir)/pkgconfig
pkgconfig_DATA		= libsmi.pc
$(pkgconfig_DATA):	config.status

anoncvs:
	RSYNC_RSH=ssh rsync -rlt --delete $(HOME)/.cvs-repository/libsmi anoncvs@anoncvs:/var/lib/cvs

ChangeLog:
#	svn2cl --ignore-message-starting='***' -o $@
	svn2cl -i -a -o $@
