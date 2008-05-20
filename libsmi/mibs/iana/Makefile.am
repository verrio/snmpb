#
# Makefile.am --
#
#      Template to generate the mibs/iana Makefile.in using automake.
#
# Copyright (c) 1999 Frank Strauss, Technical University of Braunschweig.
#
# See the file "COPYING" for information on usage and redistribution
# of this file, and for a DISCLAIMER OF ALL WARRANTIES.
#
# @(#) $Id: Makefile.am 6629 2007-03-05 14:59:30Z strauss $
#

ianadir			= $(mibdir)/iana
MIBS			= $(shell ls -1 $(srcdir) | egrep -v 'CVS|Makefile')
EXTRA_DIST		= $(MIBS)
iana_DATA		= $(MIBS)
MIBURLS			= http://www.iana.org/assignments/ianaaddressfamilynumbers-mib http://www.iana.org/assignments/ianacharset-mib http://www.iana.org/assignments/ianafinisher-mib http://www.iana.org/assignments/ianaiftype-mib http://www.iana.org/assignments/ianaiprouteprotocol-mib http://www.iana.org/assignments/ianalanguage-mib http://www.iana.org/assignments/ianamalloc-mib http://www.iana.org/assignments/ianaprinter-mib http://www.iana.org/assignments/ianatn3270etc-mib http://www.iana.org/assignments/ianagmplstc-mib http://www.iana.org/assignments/ianaippmmetricsregistry-mib http://www.iana.org/assignments/ianaitualarmtc-mib http://www.iana.org/assignments/ianamau-mib

download:
	mkdir tmp ; cd tmp ; for m in $(MIBURLS) ; do curl -s -O $$m ; b=`basename $$m` ; f=`head -5 $$b | grep DEFINITIONS | awk '{print $$1}'` ; cmp -s $$b ../$$f || cp -v $$b ../$$f ; done ; cd .. ; rm -rf tmp
