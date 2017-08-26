#
# Makefile.am --
#
#      Template to generate the yang/iana Makefile.in using automake.
#
# Copyright (c) 2011 Juergen Schoenwaelder, Jacobs University Bremen
#
# See the file "COPYING" for information on usage and redistribution
# of this file, and for a DISCLAIMER OF ALL WARRANTIES.
#
# @(#) $Id: Makefile.am 1507 2002-12-22 16:12:23Z strauss $
#

ianadir			= $(yangdir)/iana
YANGS			= $(shell ls -1 $(srcdir) | egrep -v 'CVS|Makefile')
EXTRA_DIST              = $(YANGS)
iana_DATA		= $(YANGS)
IANA			= http://www.iana.org/assignments
YANGURLS		= $(IANA)/iana-if-type-yang/iana-if-type-yang

download:
	mkdir tmp ; cd tmp ; for m in $(YANGURLS) ; do curl -s -O $$m ; b=`basename $$m` ; f=`head -5 $$b | grep module | awk '{print $$2}'` ; cmp -s $$b ../$$f || cp -v $$b ../$$f ; done ; cd .. ; rm -rf tmp

