#
# Makefile.am --
#
#      Template to generate test Makefile.in using automake.
#
# Copyright (c) 1999 Frank Strauss, Technical University of Braunschweig.
#
# See the file "COPYING" for information on usage and redistribution
# of this file, and for a DISCLAIMER OF ALL WARRANTIES.
#
# @(#) $Id: Makefile.am 1794 2013-06-06 09:10:04Z schoenw $
#

TESTMIBTOPDIR		= ../../mibs
TESTMIBDIR		= $(TESTMIBTOPDIR)/ietf:$(TESTMIBTOPDIR)/iana:$(TESTMIBTOPDIR)/irtf:$(TESTMIBTOPDIR)/site:$(TESTMIBTOPDIR)/tubs
TESTMIBS		= SNMPv2-MIB IF-MIB MAU-MIB RMON2-MIB BRIDGE-MIB

# smilint-smiv2.test has to be the first test. It initializes the
# sync-dumps script that is extended by subsequent tests.
TESTS			= smilint-smiv2.test \
			  smidump-smiv1.test \
			  smidump-smiv2.test \
			  smidump-yang.test \
			  smidump-mosy.test \
			  smidump-tree.test \
			  smidump-types.test \
			  smidump-imports.test \
			  smidump-identifiers.test \
			  smidump-metrics.test \
			  smidump-xml.test \
			  smidump-python.test \
			  smidump-cm.test \
			  smidump-corba.test \
			  smidump-jax.test \
			  smidump-netsnmp.test \
			  smidump-orig-smiv2.test \
			  smidump-smiv2-smiv2.test \
			  parser.test \
			  smidiff.test
# nobody uses SMIng at the moment and smidump-sming.test fails => disabled

TESTS_ENVIRONMENT	= SMIPATH="$(TESTMIBDIR)" TESTMIBS="$(TESTMIBS)"

EXTRA_DIST		= $(TESTS) smidump-sming.test

SMIDIFFTESTS            = $(shell ls -1d smidiff/* | egrep -v 'CVS|Makefile')

SUBDIRS                 = mibs dumps smidiff

clean-local:
	rm -rf *.out smidiff/*.diffdiff smidiff/*.result sync-dumps

