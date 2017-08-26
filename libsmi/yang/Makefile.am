#
# Makefile.am --
#
#      Template to generate the yang Makefile.in using automake.
#
# Copyright (c) 2011 Juergen Schoenwaelder, Jacobs University Bremen
#
# See the file "COPYING" for information on usage and redistribution
# of this file, and for a DISCLAIMER OF ALL WARRANTIES.
#
# @(#) $Id: Makefile.am 1508 2002-12-22 16:14:01Z strauss $
#

YANGDIRS		= ietf iana site

YANG			= $(shell ls -1d [a-z]*/* | egrep -v 'CVS|Makefile')
yang_DATA		= $(YANG)
SUBDIRS			= $(YANGDIRS)

install-exec-local:
	for dir in $(YANGDIRS) ; do \
		$(mkinstalldirs) $(DESTDIR)$(yangdir)/$$dir ; \
	done
