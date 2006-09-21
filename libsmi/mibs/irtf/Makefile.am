#
# Makefile.am --
#
#      Template to generate the mibs/irtf Makefile.in using automake.
#
# Copyright (c) 1999 Frank Strauss, Technical University of Braunschweig.
#
# See the file "COPYING" for information on usage and redistribution
# of this file, and for a DISCLAIMER OF ALL WARRANTIES.
#
# @(#) $Id: Makefile.am 1607 2003-12-01 19:01:38Z strauss $
#

irtfdir			= $(mibdir)/irtf
MIBS			= $(shell ls -1 $(srcdir) | egrep -v 'CVS|Makefile')
EXTRA_DIST              = $(MIBS)
irtf_DATA		= $(MIBS)
