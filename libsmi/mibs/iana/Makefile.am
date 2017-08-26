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
# @(#) $Id: Makefile.am 1810 2014-08-15 07:32:45Z schoenw $
#

ianadir			= $(mibdir)/iana
MIBS			= $(shell ls -1 $(srcdir) | egrep -v 'CVS|Makefile')
EXTRA_DIST		= $(MIBS)
iana_DATA		= $(MIBS)
IANA			= http://www.iana.org/assignments
MIBURLS			= $(IANA)/ianaaddressfamilynumbers-mib/ianaaddressfamilynumbers-mib \
			  $(IANA)/ianacharset-mib/ianacharset-mib \
			  $(IANA)/ianafinisher-mib/ianafinisher-mib \
			  $(IANA)/ianaiftype-mib/ianaiftype-mib \
			  $(IANA)/ianaiprouteprotocol-mib/ianaiprouteprotocol-mib \
			  $(IANA)/ianalanguage-mib/ianalanguage-mib \
			  $(IANA)/ianamalloc-mib/ianamalloc-mib \
			  $(IANA)/ianaprinter-mib/ianaprinter-mib \
			  $(IANA)/ianatn3270etc-mib/ianatn3270etc-mib \
			  $(IANA)/ianagmplstc-mib/ianagmplstc-mib \
			  $(IANA)/ianaippmmetricsregistry-mib/ianaippmmetricsregistry-mib \
			  $(IANA)/ianaitualarmtc-mib/ianaitualarmtc-mib \
			  $(IANA)/ianamau-mib/ianamau-mib \
			  $(IANA)/ianabfdtcstd-mib/ianabfdtcstd-mib

download:
	mkdir tmp ; cd tmp ; for m in $(MIBURLS) ; do curl -s -O $$m ; b=`basename $$m` ; f=`head -5 $$b | grep DEFINITIONS | awk '{print $$1}'` ; cmp -s $$b ../$$f || cp -v $$b ../$$f ; done ; cd .. ; rm -rf tmp
