/*
 * dump-ietf.c --
 *
 *      Operations to dump information in various ietf formats.
 *
 * Copyright (c) 2008 J. Schoenwaelder, Technical University of Braunschweig.
 *
 * See the file "COPYING" for information on usage and redistribution
 * of this file, and for a DISCLAIMER OF ALL WARRANTIES.
 *
 * @(#) $Id: dump-ietf.c 5758 2006-08-16 21:10:05Z schoenw $
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

static int xml = 0;

static int boilerplate = 0;
static int type_list = 0;

static int moduleLen = 0;
static int identifierLen = 0;

static int level = 0;

static char*
getStringTime(time_t t)
{
    static char   s[27];
    struct tm	  *tm;

    tm = gmtime(&t);
    sprintf(s, "%04d-%02d-%02d %02d:%02d",
	    tm->tm_year + 1900, tm->tm_mon + 1, tm->tm_mday,
	    tm->tm_hour, tm->tm_min);
    return s;
}


static void fprintParagraph(FILE *f, char *txt)
{
    if (txt) {
	if (xml) {
	    fprintf(f, "<t>\n%s\n</t>\n", txt);
	} else {
	    fprintf(f, "%s\n\n", txt);
	}
    }
}


static void fprintPreformatted(FILE *f, char *txt)
{
    if (txt) {
	if (xml) {
	    fprintf(f,
		    "<t>\n"
		    "  <figure>\n"
		    "    <artwork>\n"
		    "<![CDATA[%s]]>"
		    "    </artwork>\n"
		    "  </figure>\n"
		    "</t>\n", txt);
	} else {
	    fprintf(f, "%s\n\n", txt);
	}
    }
}


static void fprintComment(FILE *f, char *txt)
{
    if (txt) {
	if (xml) {
	    fprintf(f, "<!--\n%s\n-->\n", txt);
	} else {
	    fprintf(f, "%s\n\n", txt);
	}
    }
}


static void fprintSectionStart(FILE *f, char *txt)
{
    int i;
    
    level++;
    if (! txt) {
	txt = "UNKNOWN";
    }
    if (xml) {
	fprintf(f, "<section title=\"%s\">\n", txt);
    } else {
	for (i = 0; i < level; i++) {
	    fprintf(f, "*");
	}
	fprintf(f, " %s\n\n", txt);
    }
}


static void fprintSectionEnd(FILE *f)
{
    level--;
    if (xml) {
	fprintf(f, "</section>\n");
    }
}


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
	fprintParagraph(f,
		"This module does not define any management objects.  Instead, it\n"
		"defines a set of textual conventions which may be used by other MIB\n"
			"modules to define management objects.\n");
	fprintParagraph(f,
		"Meaningful security considerations can only be written in the MIB\n"
		"modules that define management objects.  This document has therefore\n"
		"no impact on the security of the Internet.\n");
	return;
    }

    if (rwobjs) {
	fprintComment(f,
		    "# if you have any read-write and/or read-create objects, please\n"
		    "# describe their specific sensitivity or vulnerability.\n"
		    "# RFC 2669 has a very good example.\n");
	fprintParagraph(f,
		"There are a number of management objects defined in this MIB module\n"
		"with a MAX-ACCESS clause of read-write and/or read-create.  Such\n"
		"objects may be considered sensitive or vulnerable in some network\n"
		"environments.  The support for SET operations in a non-secure\n"
		"environment without proper protection can have a negative effect on\n"
		"network operations.  These are the tables and objects and their\n"
		"sensitivity/vulnerability:");

	if (xml) fprintf(f, "<t><list style=\"symbols\">\n");
	for (i = 0; i < modc; i++) {
	    for (smiNode = smiGetFirstNode(modv[i], SMI_NODEKIND_ANY);
		 smiNode;
		 smiNode = smiGetNextNode(smiNode, SMI_NODEKIND_ANY)) {
		if (smiNode->access == SMI_ACCESS_READ_WRITE
		    && smiNode->name) {
		    if (xml) {
			fprintf(f, "<t>%s: [TBD] explain sensitivity</t>\n",
				smiNode->name);
		    } else {
			fprintf(f, "  %-*s # explain sensitivity\n",
				identifierLen, smiNode->name);
		    }
		}
	    }
	    fprintf(f, "\n");
	}
	if (xml) fprintf(f, "</list></t>\n");
    } else {
	fprintParagraph(f,
		"There are no management objects defined in this MIB module that have\n"
		"a MAX-ACCESS clause of read-write and/or read-create.  So, if this\n"
		"MIB module is implemented correctly, then there is no risk that an\n"
		"intruder can alter or create any management objects of this MIB\n"
		"module via direct SNMP SET operations.\n");
    }

    if (roobjs) {
	fprintComment(f,
		"# for all MIB modules you must evaluate whether any readable objects\n"
		"# are sensitive or vulnerable (for instance, if they might reveal\n"
		"# customer information or violate personal privacy laws such as\n"
  	        "# those of the European Union if exposed to unathorized parties)\n");
	fprintParagraph(f,
		"Some of the readable objects in this MIB module (i.e., objects with a\n"
		"MAX-ACCESS other than not-accessible) may be considered sensitive or\n"
		"vulnerable in some network environments.  It is thus important to\n"
		"control even GET and/or NOTIFY access to these objects and possibly\n"
		"to even encrypt the values of these objects when sending them over\n"
		"the network via SNMP.  These are the tables and objects and their\n"
		"sensitivity/vulnerability:\n");

	if (xml) fprintf(f, "<t><list style=\"symbols\">\n");
	for (i = 0; i < modc; i++) {
	    for (smiNode = smiGetFirstNode(modv[i], SMI_NODEKIND_ANY);
		 smiNode;
		 smiNode = smiGetNextNode(smiNode, SMI_NODEKIND_ANY)) {
		if ((smiNode->access == SMI_ACCESS_READ_WRITE
		     || smiNode->access == SMI_ACCESS_READ_ONLY
		     || smiNode->access == SMI_ACCESS_NOTIFY)
		    && smiNode->name) {
		    if (xml) {
			fprintf(f, "<t>%s: [TBD] explain sensitivity</t>\n",
			    smiNode->name);
		    } else {
			fprintf(f, "  %-*s # explain sensitivity\n",
				identifierLen, smiNode->name);
		    }
		}
	    }
	    fprintf(f, "\n");
	}
	if (xml) fprintf(f, "</list></t>\n");

	fprintParagraph(f,
		"SNMP versions prior to SNMPv3 did not include adequate security.\n"
		"Even if the network itself is secure (for example by using IPsec),\n"
		"even then, there is no control as to who on the secure network is\n"
		"allowed to access and GET/SET (read/change/create/delete) the objects\n"
		"in this MIB module.\n");
	fprintParagraph(f,
		"It is RECOMMENDED that implementers consider the security features as\n"
		"provided by the SNMPv3 framework (see [RFC3410], section 8),\n"
		"including full support for the SNMPv3 cryptographic mechanisms (for\n"
		"authentication and privacy).\n");
	fprintParagraph(f,
                "Further, deployment of SNMP versions prior to SNMPv3 is NOT\n"
		"RECOMMENDED.  Instead, it is RECOMMENDED to deploy SNMPv3 and to\n"
		"enable cryptographic security.  It is then a customer/operator\n"
		"responsibility to ensure that the SNMP entity giving access to an\n"
		"instance of this MIB module is properly configured to give access to\n"
		"the objects only to those principals (users) that have legitimate\n"
		"rights to indeed GET or SET (change/create/delete) them.\n");
    }
}



static void dumpBoilerplate(int modc, SmiModule **modv, int flags,
			    FILE *f)
{
    SmiNode   *smiNode;
    int	      i, len;

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
}



static void fprintSmiTcList(FILE *f, int modc, SmiModule **modv)
{
    SmiType     *smiType;
    SmiRevision *smiRevision;
    int	        i;
    char        *title;

    for (i = 0; i < modc; i++) {

	/* check whether there is at least one explicit type definition */
        for (smiType = smiGetFirstType(modv[i]);
	     smiType;
	     smiType = smiGetNextType(smiType)) {
	    if (smiType->status != SMI_STATUS_UNKNOWN) {
		break;
	    }
	}
	if (! smiType) {
	    continue;
	}

	smiRevision = smiGetFirstRevision(modv[i]);
	if (smiRevision) {
	    (void) smiAsprintf(&title, "%s (revision %s)",
			       modv[i]->name,
			       getStringTime(smiRevision->date));
	} else {
	    (void) smiAsprintf(&title, "%s", modv[i]->name);
	}
	fprintSectionStart(f, title);	
	smiFree(title);
	fprintParagraph(f, modv[i]->description);
	
        for (smiType = smiGetFirstType(modv[i]);
	     smiType;
	     smiType = smiGetNextType(smiType)) {

	    if (smiType->status == SMI_STATUS_UNKNOWN) {
		/* skip over ASN.1 type assignments */
		continue;
	    }

	    if (smiType->status == SMI_STATUS_CURRENT) {
		(void) smiAsprintf(&title, "%s", smiType->name);
	    } else {
		(void) smiAsprintf(&title, "%s (%s)", smiType->name, 
				   smiStatusAsString(smiType->status));
	    }
	    fprintSectionStart(f, title);
	    smiFree(title);
	    fprintPreformatted(f, smiType->description);
	    fprintSectionEnd(f);
	}
	fprintSectionEnd(f);
    }
}


#ifdef BACKEND_YANG
static void fprintYangTcs(FILE *f, YangNode *node)
{
    YangNode *childNode;
    char *title;

    for (childNode = yangGetFirstChildNode(node);
	 childNode;
	 childNode = yangGetNextChildNode(childNode)) {
	switch (childNode->nodeKind) {
	case YANG_DECL_TYPEDEF:
	case YANG_DECL_GROUPING:
	    if (childNode->status == YANG_STATUS_DEFAULT_CURRENT
		|| childNode->status == YANG_STATUS_CURRENT) {
		(void) smiAsprintf(&title, "%s", childNode->value);
	    } else {
		(void) smiAsprintf(&title, "%s (%s)", childNode->value, 
				   yangStatusAsString(childNode->status));
	    }
	    fprintSectionStart(f, title);
	    smiFree(title);
	    fprintPreformatted(f, childNode->description);
	    fprintSectionEnd(f);
	    break;
	default:
	    break;
	}
    }
}



static void fprintYangTcList(FILE *f, int modc, SmiModule **modv)
{
    YangNode	*node;
    int	        i;
    char        *title;

    for (i = 0; i < modc; i++) {
	node = yangGetModule(modv[i]->name);
	if (! node) continue;

	/* xxx search for the revision */
	(void) smiAsprintf(&title, "%s", modv[i]->name);
	fprintSectionStart(f, title);	
	smiFree(title);
	fprintParagraph(f, node->description);
	fprintYangTcs(f, node);
	fprintSectionEnd(f);
    }
}
#endif


static void dumpTcList(int modc, SmiModule **modv, int flags, FILE *f)
{
    int i;
    char *comment;

    if (flags & SMIDUMP_FLAG_UNITE) {

    	if (! (flags & SMIDUMP_FLAG_SILENT)) {
	    fprintComment(f,
		  "# united textual convention / typedef and grouping list (generated by smidump "
			  SMI_VERSION_STRING ")");
	}

	if (! (flags & SMIDUMP_FLAG_SILENT) && (flags & SMIDUMP_FLAG_ERROR)) {
	    fprintComment(f,
			  "# WARNING: this output may be incorrect due to "
			  "significant parse errors");
	}
#ifdef BACKEND_YANG
	if (modv[0]->language == SMI_LANGUAGE_YANG) {
	    fprintYangTcList(f, modc, modv);
	}
#endif
	fprintSmiTcList(f, modc, modv);

    } else {

	for (i = 0; i < modc; i++) {

	    if (! (flags & SMIDUMP_FLAG_SILENT)) {
		smiAsprintf(&comment,
		    "# %s textual convention / typedef and grouping list (generated by smidump "
			    SMI_VERSION_STRING ")", modv[i]->name);
		fprintComment(f, comment);
		smiFree(comment);
	    }

	    if (! (flags & SMIDUMP_FLAG_SILENT) && (flags & SMIDUMP_FLAG_ERROR)) {
		fprintComment(f,
			      "# WARNING: this output may be incorrect due to "
			      "significant parse errors");
	    }
	    
#ifdef BACKEND_YANG
	    if (modv[i]->language == SMI_LANGUAGE_YANG) {
		fprintYangTcList(f, 1, &(modv[i]));
	    }
#endif
	    fprintSmiTcList(f, 1, &(modv[i]));
	}
    }
}



static void dumpIetf(int modc, SmiModule **modv, int flags,
		     char *output)
{
    FILE *f = stdout;
    int i, cntSmi = 0, cntYang = 0;
    char *title = "SMIv2 Textual Conventions / YANG Type Definitions and Groupings";
    char *abbrev = "TCs / Typedefs and Groupings";
    char *filename = "textual-convention-typedef-grouping-list.txt";
    
    if (output) {
	f = fopen(output, "w");
	if (!f) {
	    fprintf(stderr, "smidump: cannot open %s for writing: ", output);
	    perror(NULL);
	    exit(1);
	}
    }

    for (i = 0; i < modc; i++) {
#ifdef BACKEND_YANG
	if (modv[i]->language == SMI_LANGUAGE_YANG) {
	    cntYang++;
	}
#endif
	if (modv[i]->language == SMI_LANGUAGE_SMIV1
	    || modv[i]->language == SMI_LANGUAGE_SMIV2) {
	    cntSmi++;
	}
    }

    if (cntSmi == 0) {
	title = "YANG Type Definitions and Groupings";
	abbrev = title;
	filename = "typedef-grouping-list.txt";
    }

    if (cntYang == 0) {
	title = "SMIv2 Textual Conventions";
	abbrev = title;
	filename = "textual-convention-list.txt";
    }

    if (xml) {
	fprintf(f,
		"<?xml version=\"1.0\" encoding=\"UTF-8\"?>\n"
		"\n"
		"<rfc category='info' ipr='pre5378Trust200902' docName='%s'>\n"
		"\n"
		"<?xml-stylesheet type='text/xsl' href='rfc2629.xslt'?>\n"
		"\n"
		"<?rfc toc='yes'?>\n"
		"<?rfc strict='yes'?>\n"
		"<?rfc compact='yes'?>\n"
		"\n",
		type_list ? filename
		          : boilerplate ? "boilerplate.txt" : "unknown.txt");

	fprintf(f,
		"  <front>\n"
		"    <title abbrev='%s'>List of %s</title>\n"
		"    <author initials='' surname='' fullname=''>\n"
		"      <organization/>\n"
		"    </author>\n"
		"    <date/>\n", abbrev, title);
	if (boilerplate) {
	    fprintf(f,
		    "    <abstract>\n"
		    "      <t>\n"
		    "        Security considerations boilerplate. This file\n"
		    "        has been generated by smidump. Do not edit.\n"
		    "     </t>\n"
		    "    </abstract>\n");
	}
	if (type_list) {
	    fprintf(f,
		    "    <abstract>\n"
		    "      <t>\n"
		    "        List of %s.\n"
		    "        This file has been generated by smidump. Do not edit.\n"
		    "     </t>\n"
		    "    </abstract>\n", title);
	}
	fprintf(f,
		"  </front>\n");
	fprintf(f, "  <middle>\n");
    }

    if (boilerplate) {
	dumpBoilerplate(modc, modv, flags, f);
	fprintf(f, "  </middle>\n");
    }
    if (type_list) {
	dumpTcList(modc, modv, flags, f);
	if (xml) {
	    fprintSectionStart(f, "Security Considerations");
	    fprintParagraph(f, "None.");
	    fprintSectionEnd(f);
	    fprintf(f, "  </middle>\n");
	}
    }

    if (xml) {
	fprintf(f,
		"  <back/>\n"
		"</rfc>\n");
    }

    if (fflush(f) || ferror(f)) {
	perror("smidump: write error");
	exit(1);
    }

    if (output) {
	fclose(f);
    }
}



void initIetf()
{
    static SmidumpDriverOption opt[] = {
	{ "type-list", OPT_FLAG, &type_list, 0,
	  "list of textual conventions or type definitions and groupings" },
	{ "boilerplate", OPT_FLAG, &boilerplate, 0,
	  "generate security considerations boilerplate from a data model" },
	{ "xml2rfc", OPT_FLAG, &xml, 0,
	  "generate output in xml2rfc format instead of plain text format" },
        { 0, OPT_END, 0, 0 }
    };
    
   
    static SmidumpDriver driver = {
	"ietf",
	dumpIetf,
	0,
	0,
	"generate output in various IETF documentation formats",
	opt,
	NULL
    };
    
    smidumpRegisterDriver(&driver);
}
