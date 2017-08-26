/*
 * dump-identifiers.c --
 *
 *      Operations to dump flat identifier lists for SMI modules.
 *
 * Copyright (c) 2000 Frank Strauss, Technical University of Braunschweig.
 * Copyright (c) 2000 J. Schoenwaelder, Technical University of Braunschweig.
 *
 * See the file "COPYING" for information on usage and redistribution
 * of this file, and for a DISCLAIMER OF ALL WARRANTIES.
 *
 * @(#) $Id: dump-identifiers.c 1772 2012-04-01 12:15:23Z schoenw $
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

/*
 * TODO:
 *
 *  - yang does not provide line numbers :-(
 *
 *  - mixed SMI and YANG modules not handled
 *
 */


static int moduleLen = 0;
static int identifierLen = 0;

static int showlines = 0;
static int showpath = 0;
static int ctagfmt = 0;

static void fprintSmiNodeIdentifiers(FILE *f, int modc, SmiModule **modv)
{
    SmiNode      *smiNode;
    unsigned int j;
    int          i;

    for (i = 0; i < modc; i++) {
	for (smiNode = smiGetFirstNode(modv[i], SMI_NODEKIND_ANY);
	     smiNode;
	     smiNode = smiGetNextNode(smiNode, SMI_NODEKIND_ANY)) {
	    if (smiNode->name) {
		if (ctagfmt) {
		    fprintf(f, "%*s", -identifierLen, smiNode->name);
		    fprintf(f, " %d", smiGetNodeLine(smiNode));
		    fprintf(f, " %*s", -moduleLen, modv[i]->path);
		    fprintf(f, " %s OBJECT-TYPE -- %s\n", smiNode->name,
			    smiNodekindAsString(smiNode->nodekind));
		} else {
		    fprintf(f, "%*s",
			    -moduleLen, showpath ? modv[i]->path : modv[i]->name);
		    if (showlines) {
			    fprintf(f, ":%d:", smiGetNodeLine(smiNode));
		    }
		    fprintf(f, " %*s %-12s ", -identifierLen, smiNode->name,
			    smiNodekindAsString(smiNode->nodekind));
		    for (j = 0; j < smiNode->oidlen; j++) {
			    fprintf(f, j ? ".%u" : "%u", smiNode->oid[j]);
		    }
		    fprintf(f, "\n");
		}
	    }
	}
    }
}



static void fprintSmiTypeIdentifiers(FILE *f, int modc, SmiModule **modv)
{
    SmiType   *smiType;
    int	      i;

    for (i = 0; i < modc; i++) {
	for (smiType = smiGetFirstType(modv[i]);
	     smiType;
	     smiType = smiGetNextType(smiType)) {
	    if (smiType->name) {
		if (ctagfmt) {
		    fprintf(f, "%*s", -identifierLen, smiType->name);
		    fprintf(f, " %d", smiGetTypeLine(smiType));
		    fprintf(f, " %*s", -moduleLen, modv[i]->path);
		    fprintf(f, " %s TEXTUAL-CONVENTION\n", smiType->name);
		} else {
		fprintf(f, "%*s",
			-moduleLen, showpath ? modv[i]->path : modv[i]->name);
		if (showlines) {
		    fprintf(f, ":%d:", smiGetTypeLine(smiType));
		}
		fprintf(f, " %*s %-12s\n", -identifierLen, smiType->name,
			"type");
		    }
	    }
	}
    }
}


#ifdef BACKEND_YANG

static void fprintYangAll(FILE *f, const char *module, YangNode *node)
{
    YangNode *childNode;

    for (childNode = yangGetFirstChildNode(node);
	 childNode;
	 childNode = yangGetNextChildNode(childNode)) {
	switch (childNode->nodeKind) {
	case YANG_DECL_MODULE:
	case YANG_DECL_SUBMODULE:
	case YANG_DECL_CONTAINER:
	case YANG_DECL_LEAF:
	case YANG_DECL_LEAF_LIST:
	case YANG_DECL_LIST:
	case YANG_DECL_GROUPING:
	case YANG_DECL_RPC:
	case YANG_DECL_TYPEDEF:
	case YANG_DECL_NOTIFICATION:
	case YANG_DECL_FEATURE:
	case YANG_DECL_IDENTITY:
	case YANG_DECL_DEVIATION:
	    if (! f) {
		int len;
		len = strlen(childNode->value);
		if (len > identifierLen) identifierLen = len;
	    } else {
		if (ctagfmt) {
		    fprintf(f, "%*s", -identifierLen, childNode->value);
		    fprintf(f, " %d", 0);
		    fprintf(f, " %*s", -moduleLen, module);
		    fprintf(f, " %s %s\n", childNode->value,
			    yangDeclAsString(childNode->nodeKind));
		} else {
		    fprintf(f, "%*s", -moduleLen, module);
		    if (showlines) {
			fprintf(f, ":%d:", 0);
		    }
		    fprintf(f, " %*s %s\n", -identifierLen, childNode->value,
			    yangDeclAsString(childNode->nodeKind), childNode);
		}
	    }
	    break;
	default:
	    break;
	}
	fprintYangAll(f, module, childNode);
    }
}

static void fprintYangIdentifiers(FILE *f, int modc, SmiModule **modv)
{
    YangNode *typeNode;
    int	      i;

    for (i = 0; i < modc; i++) {
	fprintYangAll(f, (showpath || ctagfmt) ? modv[i]->path : modv[i]->name,
		      yangGetModule(modv[i]->name));
    }
}
#endif


static void dumpIdentifiers(int modc, SmiModule **modv, int flags,
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
	if (showpath) {
	    len = strlen(modv[i]->path);
	} else {
	    len = strlen(modv[i]->name);
	}
	if (len > moduleLen) moduleLen = len;
#ifdef BACKEND_YANG
	if (modv[i]->language == SMI_LANGUAGE_YANG) {
	    fprintYangAll(NULL, NULL, yangGetModule(modv[i]->name));
	}
#endif
	if (modv[i]->language == SMI_LANGUAGE_SMIV1
	    || modv[i]->language == SMI_LANGUAGE_SMIV2) {
	    for (smiNode = smiGetFirstNode(modv[i], SMI_NODEKIND_ANY);
		 smiNode;
		 smiNode = smiGetNextNode(smiNode, SMI_NODEKIND_ANY)) {
		if (smiNode->name) {
		    len = strlen(smiNode->name);
		    if (len > identifierLen) identifierLen = len;
		}
	    }
	}
    }

    if (flags & SMIDUMP_FLAG_UNITE) {
	
	if (! (flags & SMIDUMP_FLAG_SILENT)) {
	    fprintf(f, "# united list of identifiers (generated by smidump "
		    SMI_VERSION_STRING ")\n\n");
	}

	if (! (flags & SMIDUMP_FLAG_SILENT) && (flags & SMIDUMP_FLAG_ERROR)) {
	    fprintf(f, "# WARNING: this output may be incorrect due to "
		    "significant parse errors\n\n");
	}

#ifdef BACKEND_YANG
	if (modv[0]->language == SMI_LANGUAGE_YANG) {
	    fprintYangIdentifiers(f, modc, modv);
	}
#endif
	if (modv[0]->language == SMI_LANGUAGE_SMIV1
	    || modv[0]->language == SMI_LANGUAGE_SMIV2) {
	    fprintSmiTypeIdentifiers(f, modc, modv);
	    fprintSmiNodeIdentifiers(f, modc, modv);
	}

    } else {

	for (i = 0; i < modc; i++) {

	    if (! (flags & SMIDUMP_FLAG_SILENT)) {
		fprintf(f, "# %s list of identifiers (generated by smidump "
			SMI_VERSION_STRING ")\n\n",
			modv[i]->name);
	    }

	    if (! (flags & SMIDUMP_FLAG_SILENT) && (flags & SMIDUMP_FLAG_ERROR)) {
		fprintf(f, "# WARNING: this output may be incorrect due to "
			"significant parse errors\n\n");
	    }
#ifdef BACKEND_YANG
	    if (modv[i]->language == SMI_LANGUAGE_YANG) {
		fprintYangIdentifiers(f, 1, &(modv[i]));
	    }
#endif
	    if (modv[i]->language == SMI_LANGUAGE_SMIV1
		|| modv[i]->language == SMI_LANGUAGE_SMIV2) {
		fprintSmiTypeIdentifiers(f, 1, &(modv[i]));
		fprintSmiNodeIdentifiers(f, 1, &(modv[i]));
	    }
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



void initIdentifiers()
{
    
    static SmidumpDriverOption opt[] = {
	{ "lines", OPT_FLAG, &showlines, 0,
	  "show line numbers"},
	{ "path", OPT_FLAG, &showpath, 0,
	  "show file path instead of module name"},
	{ "ctag", OPT_FLAG, &ctagfmt, 0,
	  "show symbols in [g]ctag format"},
        { 0, OPT_END, 0, 0 }
    };

    static SmidumpDriver driver = {
	"identifiers",
	dumpIdentifiers,
	SMI_FLAG_NODESCR,
	0,
	"list of all identifiers",
	opt,
	NULL
    };
    
    smidumpRegisterDriver(&driver);
}
