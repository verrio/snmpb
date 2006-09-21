#
# Makefile.am --
#
#      Template to generate the pibs Makefile.in using automake.
#
# Copyright (c) 2002 Frank Strauss, Technical University of Braunschweig.
#
# See the file "COPYING" for information on usage and redistribution
# of this file, and for a DISCLAIMER OF ALL WARRANTIES.
#
# @(#) $Id: Makefile.am 1508 2002-12-22 16:14:01Z strauss $
#

PIBDIRS			= ietf tubs site

PIBS			= $(shell ls -1d [a-z]*/* | egrep -v 'CVS|Makefile')
pib_DATA		= $(PIBS)
SUBDIRS			= $(PIBDIRS)

install-exec-local:
	for dir in $(PIBDIRS) ; do \
		$(mkinstalldirs) $(DESTDIR)$(pibdir)/$$dir ; \
	done
