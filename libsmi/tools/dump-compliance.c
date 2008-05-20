/*
 * dump-compliances.c --
 *
 *      Operations to dump compliances in a human readable format.
 *
 * Copyright (c) 2005 J. Schoenwaelder, International University Bremen.
 *
 * See the file "COPYING" for information on usage and redistribution
 * of this file, and for a DISCLAIMER OF ALL WARRANTIES.
 *
 * @(#) $Id: dump-compliances.c 1571 2003-07-14 22:58:42Z schoenw $
 */

#include <config.h>

#include <stdio.h>
#include <string.h>

#include "smi.h"
#include "smidump.h"


static char *getFlags(SmiNode *smiNode)
{

    switch (smiNode->access) {
    case SMI_ACCESS_UNKNOWN:
	return "---";
    case SMI_ACCESS_NOT_ACCESSIBLE:
	return "---";
    case SMI_ACCESS_NOTIFY:
	return "--n";
    case SMI_ACCESS_EVENT_ONLY:
	return "--n";
    case SMI_ACCESS_READ_ONLY:
	return "r-n";
    case SMI_ACCESS_READ_WRITE:
	return "rwn";
    case SMI_ACCESS_NOT_IMPLEMENTED:
	return "---";
    case SMI_ACCESS_INSTALL:
	return "-i-";
    case SMI_ACCESS_INSTALL_NOTIFY:
	return "-in";
    case SMI_ACCESS_REPORT_ONLY:
	return "--r";
    }

    return "";
}



static char getStatusChar(SmiStatus status)
{
    switch (status) {
    case SMI_STATUS_UNKNOWN:
	return '+';
    case SMI_STATUS_CURRENT:
	return '+';
    case SMI_STATUS_DEPRECATED:
	return 'x';
    case SMI_STATUS_MANDATORY:
        return '+';
    case SMI_STATUS_OPTIONAL:
	return '+';
    case SMI_STATUS_OBSOLETE:
	return 'o';
    }

    return ' ';
}



static char *getTypeName(SmiNode *smiNode)
{
    char *type;
    SmiType *smiType, *parentType;

    smiType = smiGetNodeType(smiNode);

    if (!smiType || smiNode->nodekind == SMI_NODEKIND_TABLE)
	return NULL;
    
    if (smiType->decl == SMI_DECL_IMPLICIT_TYPE) {
	parentType = smiGetParentType(smiType);
	if (!parentType)
	    return NULL;
	smiType = parentType;
    }

    type = xstrdup(smiType->name);
    return type;
}



static void fprintGroup(FILE *f, SmiNode *smiNode, char c,
			int *typelen, int *namelen, int pass)
{
    SmiElement *smiElement;
    SmiNode *smiObject;
    SmiModule *smiModule;
    char *type_name;
    int tlen = 9, nlen = 9;

    switch (smiNode->nodekind) {
    case SMI_NODEKIND_GROUP:
	for (smiElement = smiGetFirstElement(smiNode);
	     smiElement;
	     smiElement = smiGetNextElement(smiElement)) {
	    smiObject = smiGetElementNode(smiElement);
	    smiModule = smiGetNodeModule(smiNode);
	    type_name = getTypeName(smiObject);
	    if (pass == 1) {
		if (type_name) {
		    int newlen = strlen(type_name);
		    tlen = (tlen < newlen) ? newlen : tlen;
		}
		if (smiObject->name) {
		    int newlen = strlen(smiObject->name);
		    nlen = (nlen < newlen) ? newlen : nlen;
		}
	    } else if (pass == 2) {
		fprintf(f, "  %c%c%s %-*s %-*s (%s)\n",
			getStatusChar(smiObject->status), c,
			getFlags(smiObject),
			*typelen, type_name ? type_name : "-",
			*namelen, smiObject->name, smiNode->name);
	    }
	    xfree(type_name);
	}
	break;
    case SMI_NODEKIND_SCALAR:
    case SMI_NODEKIND_COLUMN:
	smiObject = smiNode;
	type_name = getTypeName(smiObject);
	if (pass == 1) {
	    if (type_name) {
		int newlen = strlen(type_name);
		tlen = tlen < newlen ? newlen : tlen;
	    }
	    if (smiObject->name) {
		int newlen = strlen(smiObject->name);
		nlen = (nlen < newlen) ? newlen : nlen;
	    }
	} else if (pass == 2) {
	    fprintf(f, "  %c%c%s %-*s %s\n",
		    getStatusChar(smiObject->status), 'r',
		    getFlags(smiObject),
		    *typelen, type_name ? type_name : "-",
		    smiObject->name);
	}
	xfree(type_name);
	break;
    default:
	break;
    }

    if (pass == 1) {
	if (typelen) *typelen = tlen;
	if (namelen) *namelen = nlen;
    }
}



static void fprintCompliance(FILE *f, SmiNode *smiNode,
			     int *typelen, int *namelen, int pass)
{
    SmiElement *smiElement;
    SmiOption *smiOption;
    SmiRefinement *smiRefinement;
    int tlen = 0, nlen = 0;

    for (smiElement = smiGetFirstElement(smiNode);
	 smiElement;
	 smiElement = smiGetNextElement(smiElement)) {
	fprintGroup(f, smiGetElementNode(smiElement), 'm',
		    (pass == 1) ? &tlen : typelen,
		    (pass == 1) ? &nlen : namelen, pass);
	if (pass == 1) {
	    if (typelen) {
		*typelen = *typelen < tlen ? tlen : *typelen;
	    }
	    if (namelen) {
		*namelen = *namelen < nlen ? nlen : *namelen;
	    }
	}
    }

    for(smiOption = smiGetFirstOption(smiNode);
	smiOption;
	smiOption = smiGetNextOption(smiOption)) {
	fprintGroup(f, smiGetOptionNode(smiOption), 'c',
		    (pass == 1) ? &tlen : typelen,
		    (pass == 1) ? &nlen : namelen, pass);
	if (pass == 1) {
	    if (typelen) {
		*typelen = *typelen < tlen ? tlen : *typelen;
	    }
	    if (namelen) {
		*namelen = *namelen < nlen ? nlen : *namelen;
	    }
	}
    }

    for (smiRefinement = smiGetFirstRefinement(smiNode);
	 smiRefinement;
	 smiRefinement = smiGetNextRefinement(smiRefinement)) {
	fprintGroup(f, smiGetRefinementNode(smiRefinement), 'r',
		    (pass == 1) ? &tlen : typelen,
		    (pass == 1) ? &nlen : namelen, pass);
	if (pass == 1) {
	    if (typelen) {
		*typelen = *typelen < tlen ? tlen : *typelen;
	    }
	    if (namelen) {
		*namelen = *namelen < nlen ? nlen : *namelen;
	    }
	}
    }
}



static void fprintCompliances(FILE *f, SmiModule *smiModule)
{
    SmiNode *smiNode;
    int i, typelen = 0, namelen = 0;

    for (i = 0, smiNode = smiGetFirstNode(smiModule, SMI_NODEKIND_COMPLIANCE);
	 smiNode;
	 smiNode = smiGetNextNode(smiNode, SMI_NODEKIND_COMPLIANCE), i++) {
	fprintf(f, "%s%s:\n", i ? "\n" : "", smiNode->name);
	fprintCompliance(f, smiNode, &typelen, &namelen, 1);
	fprintCompliance(f, smiNode, &typelen, &namelen, 2);
    }
}



static void
dumpCompliances(int modc, SmiModule **modv, int flags, char *output)
{
    int     i;
    FILE    *f = stdout;
    
    if (output) {
	f = fopen(output, "w");
	if (!f) {
	    fprintf(stderr, "smidump: cannot open %s for writing: ", output);
	    perror(NULL);
	    exit(1);
	}
    }

    for (i = 0; i < modc; i++) {
	if (! (flags & SMIDUMP_FLAG_SILENT)) {
	    fprintf(f, "# %s compliances (generated by smidump "
		    SMI_VERSION_STRING ")\n\n", modv[i]->name);
	}

	if (! (flags & SMIDUMP_FLAG_SILENT) && (flags & SMIDUMP_FLAG_ERROR)) {
	    fprintf(f, "# WARNING: this output may be incorrect due to "
		    "significant parse errors\n\n");
	}
	
	fprintCompliances(f, modv[i]);
    }

    if (fflush(f) || ferror(f)) {
	perror("smidump: write error");
	exit(1);
    }

    if (output) {
	fclose(f);
    }
}



void initCompliances()
{
    static SmidumpDriver driver = {
	"compliances",
	dumpCompliances,
	SMI_FLAG_NODESCR,
	SMIDUMP_DRIVER_CANT_UNITE,
	"compliances with all included objects / notifications",
	/* opt, */ NULL,
	NULL
    };
    
    smidumpRegisterDriver(&driver);
}
