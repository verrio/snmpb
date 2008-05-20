/*
 * dump-boilerplate.c --
 *
 *      Operations to dump security considerations boilerplates SMI modules.
 *
 * Copyright (c) 2008 J. Schoenwaelder, Technical University of Braunschweig.
 *
 * See the file "COPYING" for information on usage and redistribution
 * of this file, and for a DISCLAIMER OF ALL WARRANTIES.
 *
 * @(#) $Id: dump-identifiers.c 5758 2006-08-16 21:10:05Z schoenw $
 */

#include <config.h>

#include <stdlib.h>
#include <stdio.h>
#include <stdarg.h>
#include <string.h>
#include <ctype.h>
#include <time.h>

#include "smi.h"
#include "smidump.h"


static int moduleLen = 0;
static int identifierLen = 0;

static void fprintBoilerplate(FILE *f, int modc, SmiModule **modv)
{
    SmiNode   *smiNode;
    int	      i, roobjs = 0, rwobjs = 0;

    for (i = 0; i < modc; i++) {
	for (smiNode = smiGetFirstNode(modv[i], SMI_NODEKIND_ANY);
	     smiNode;
	     smiNode = smiGetNextNode(smiNode, SMI_NODEKIND_ANY)) {
	    if (!smiNode->name) continue;
	    if (smiNode->access == SMI_ACCESS_READ_WRITE) {
		rwobjs++;
	    }
	    if (smiNode->access == SMI_ACCESS_READ_WRITE
		|| smiNode->access == SMI_ACCESS_READ_ONLY
		|| smiNode->access == SMI_ACCESS_NOTIFY) {
		roobjs++;
	    }
	}
    }

    if (roobjs == 0 && rwobjs == 0) {
	fprintf(f,
		"This module does not define any management objects.  Instead, it\n"
		"defines a set of textual conventions which may be used by other MIB\n"
		"modules to define management objects.\n"
		"\n"
		"Meaningful security considerations can only be written in the MIB\n"
		"modules that define management objects.  This document has therefore\n"
		"no impact on the security of the Internet.\n");
	return;
    }

    if (rwobjs) {
	fprintf(f,
		"# if you have any read-write and/or read-create objects, please\n"
		"# describe their specific sensitivity or vulnerability.\n"
		"# RFC 2669 has a very good example.\n"
		"\n"
		"There are a number of management objects defined in this MIB module\n"
		"with a MAX-ACCESS clause of read-write and/or read-create.  Such\n"
		"objects may be considered sensitive or vulnerable in some network\n"
		"environments.  The support for SET operations in a non-secure\n"
		"environment without proper protection can have a negative effect on\n"
		"network operations.  These are the tables and objects and their\n"
		"sensitivity/vulnerability:\n"
		"\n");
	for (i = 0; i < modc; i++) {
	    for (smiNode = smiGetFirstNode(modv[i], SMI_NODEKIND_ANY);
		 smiNode;
		 smiNode = smiGetNextNode(smiNode, SMI_NODEKIND_ANY)) {
		if (smiNode->access == SMI_ACCESS_READ_WRITE
		    && smiNode->name) {
		    fprintf(f, "  %-*s # explain sensitivity\n",
			    identifierLen, smiNode->name);
		}
	    }
	    fprintf(f, "\n");
	}
    } else {
	fprintf(f,
		"There are no management objects defined in this MIB module that have\n"
		"a MAX-ACCESS clause of read-write and/or read-create.  So, if this\n"
		"MIB module is implemented correctly, then there is no risk that an\n"
		"intruder can alter or create any management objects of this MIB\n"
		"module via direct SNMP SET operations.\n"
		"\n");
    }

    if (roobjs) {
	fprintf(f,
		"# for all MIB modules you must evaluate whether any readable objects\n"
		"# are sensitive or vulnerable (for instance, if they might reveal\n"
		"# customer information or violate personal privacy laws such as\n"
		"# those of the European Union if exposed to unathorized parties)\n"
		"\n"
		"Some of the readable objects in this MIB module (i.e., objects with a\n"
		"MAX-ACCESS other than not-accessible) may be considered sensitive or\n"
		"vulnerable in some network environments.  It is thus important to\n"
		"control even GET and/or NOTIFY access to these objects and possibly\n"
		"to even encrypt the values of these objects when sending them over\n"
		"the network via SNMP.  These are the tables and objects and their\n"
		"sensitivity/vulnerability:\n"
		"\n");

	for (i = 0; i < modc; i++) {
	    for (smiNode = smiGetFirstNode(modv[i], SMI_NODEKIND_ANY);
		 smiNode;
		 smiNode = smiGetNextNode(smiNode, SMI_NODEKIND_ANY)) {
		if ((smiNode->access == SMI_ACCESS_READ_WRITE
		     || smiNode->access == SMI_ACCESS_READ_ONLY
		     || smiNode->access == SMI_ACCESS_NOTIFY)
		    && smiNode->name) {
		    fprintf(f, "  %-*s # explain sensitivity\n",
			    identifierLen, smiNode->name);
		}
	    }
	    fprintf(f, "\n");
	}

	fprintf(f,
		"SNMP versions prior to SNMPv3 did not include adequate security.\n"
		"Even if the network itself is secure (for example by using IPsec),\n"
		"even then, there is no control as to who on the secure network is\n"
		"allowed to access and GET/SET (read/change/create/delete) the objects\n"
		"in this MIB module.\n"
		"\n"
		"It is RECOMMENDED that implementers consider the security features as\n"
		"provided by the SNMPv3 framework (see [RFC3410], section 8),\n"
		"including full support for the SNMPv3 cryptographic mechanisms (for\n"
		"authentication and privacy).\n"
		"\n"
		"Further, deployment of SNMP versions prior to SNMPv3 is NOT\n"
		"RECOMMENDED.  Instead, it is RECOMMENDED to deploy SNMPv3 and to\n"
		"enable cryptographic security.  It is then a customer/operator\n"
		"responsibility to ensure that the SNMP entity giving access to an\n"
		"instance of this MIB module is properly configured to give access to\n"
		"the objects only to those principals (users) that have legitimate\n"
		"rights to indeed GET or SET (change/create/delete) them.\n"
		"\n");
    }
}



static void dumpBoilerplate(int modc, SmiModule **modv, int flags,
			    char *output)
{
    SmiNode   *smiNode;
    int	      i, len;
    FILE      *f = stdout;

    if (output) {
	f = fopen(output, "w");
	if (!f) {
	    fprintf(stderr, "smidump: cannot open %s for writing: ", output);
	    perror(NULL);
	    exit(1);
	}
    }

    for (moduleLen = 0, identifierLen = 0, i = 0; i < modc; i++) {
	for (smiNode = smiGetFirstNode(modv[i], SMI_NODEKIND_ANY);
	     smiNode;
	     smiNode = smiGetNextNode(smiNode, SMI_NODEKIND_ANY)) {
	    if (smiNode->name) {
		len = strlen(smiNode->name);
		if (len > identifierLen) identifierLen = len;
	    }
	}
    }

    if (flags & SMIDUMP_FLAG_UNITE) {

	if (! (flags & SMIDUMP_FLAG_SILENT)) {
	    fprintf(f, "# united security considerations boilerplate (generated by smidump "
		    SMI_VERSION_STRING ")\n\n");
	}

	if (! (flags & SMIDUMP_FLAG_SILENT) && (flags & SMIDUMP_FLAG_ERROR)) {
	    fprintf(f, "# WARNING: this output may be incorrect due to "
		    "significant parse errors\n\n");
	}

	fprintBoilerplate(f, modc, modv);

    } else {

	for (i = 0; i < modc; i++) {

	    if (! (flags & SMIDUMP_FLAG_SILENT)) {
		fprintf(f, "# %s security considerations boilerplate (generated by smidump "
			SMI_VERSION_STRING ")\n\n",
			modv[i]->name);
	    }

	    if (! (flags & SMIDUMP_FLAG_SILENT) && (flags & SMIDUMP_FLAG_ERROR)) {
		fprintf(f, "# WARNING: this output may be incorrect due to "
			"significant parse errors\n\n");
	    }
	    
	    fprintBoilerplate(f, 1, &(modv[i]));
	}
    }

    if (fflush(f) || ferror(f)) {
	perror("smidump: write error");
	exit(1);
    }

    if (output) {
	fclose(f);
    }
}



void initBoilerplate()
{
   
    static SmidumpDriver driver = {
	"boilerplate",
	dumpBoilerplate,
	SMI_FLAG_NODESCR,
	0,
	"generate security considerations boilerplate text",
	NULL,
	NULL
    };
    
    smidumpRegisterDriver(&driver);
}
