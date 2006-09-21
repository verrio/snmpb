#
# Makefile.am --
#
#      Template to generate the mibs/ietf Makefile.in using automake.
#
# Copyright (c) 1999 Frank Strauss, Technical University of Braunschweig.
#
# See the file "COPYING" for information on usage and redistribution
# of this file, and for a DISCLAIMER OF ALL WARRANTIES.
#
# @(#) $Id: Makefile.am 1606 2003-12-01 12:24:15Z strauss $
#

ietfdir			= $(mibdir)/ietf
MIBS			= $(shell ls -1 $(srcdir) | egrep -v 'CVS|Makefile')
EXTRA_DIST              = $(MIBS)
ietf_DATA		= $(MIBS)
