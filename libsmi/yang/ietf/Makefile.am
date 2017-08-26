#
# Makefile.am --
#
#      Template to generate the yang/ietf Makefile.in using automake.
#
# Copyright (c) 2011 Juergen Schoenwaelder, Jacobs University Bremen
#
# See the file "COPYING" for information on usage and redistribution
# of this file, and for a DISCLAIMER OF ALL WARRANTIES.
#
# @(#) $Id: Makefile.am 1507 2002-12-22 16:12:23Z strauss $
#

ietfdir			= $(yangdir)/ietf
YANG			= $(shell ls -1 $(srcdir) | egrep -v 'CVS|Makefile')
EXTRA_DIST		= $(YANG)
ietf_DATA		= $(YANG)

