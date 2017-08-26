/*
 * dump-yang.c --
 *
 *      Operations to dump MIB modules in the YANG output format.
 *
 * Copyright (c) 2007-2011 J. Schoenwaelder, Jacobs University Bremen
 *
 * See the file "COPYING" for information on usage and redistribution
 * of this file, and for a DISCLAIMER OF ALL WARRANTIES.
 *
 * @(#) $Id: dump-yang.c 13463 2009-03-16 07:54:49Z schoenw $
 */

#include <config.h>

#ifdef BACKEND_YANG

#include <ctype.h>
#include <stdio.h>
#include <string.h>

#include "smi.h"
#include "smi2yang.h"
#include "smidump.h"
#include "fprint.h"
#include "fortopat.h"

static int INDENT = 2;		/* indent factor */

static
void fprintString(FILE *f, int indent, YangNode* nodePtr)
{
    char *str;
    int i;
    int prev = 0;

    fprint(f, "%*c\"", indent + 1, ' ');

    for (i = 0; i < strlen(nodePtr->value); i++) {
	if (nodePtr->value[i] == '\n') {
	    str = xmalloc(i - prev + 1);
	    memcpy(str, &nodePtr->value[prev], i - prev);	    
	    str[i - prev] = 0;
	    if (!prev) {
		fprint(f, "%s", str);
	    } else {
		fprint(f, "\n");
		fprint(f, "%*c", indent + 2, ' ');
		fprint(f, "%s", str);
	    }
	    xfree(str);
	    prev = i + 1;
	}
    }
    if (i != prev) {
	str = xmalloc(i - prev + 1);
	memcpy(str, &nodePtr->value[prev], i - prev);
	str[i - prev] = 0;
	if (!prev) {
	    fprint(f, "%s", str);
	} else {
	    fprint(f, "\n");
	    fprint(f, "%*c", indent + 2, ' ');
	    fprint(f, "%s", str);
	}
	xfree(str);
    }
    
    fprint(f, "\"");
}

static void
fprintPattern(FILE *f, int indent, YangNode* nodePtr)
{
    char *str;
    int i;
    int prev = 0;
    
    for (i = 0; i < strlen(nodePtr->value); i++) {
	if (nodePtr->value[i] == '\n') {
	    str = xmalloc(i - prev + 1);
	    memcpy(str, &nodePtr->value[prev], i - prev);
	    str[i - prev] = 0;
	    if (!prev) {
		fprint(f, "'%s'", str);
	    } else {
		fprint(f, "\n");
		fprint(f, "%*c", indent-1, ' ');
		fprint(f, "+ '%s'", str);
	    }
	    xfree(str);
	    prev = i + 1;
	}
    }
    if (i != prev) {
	str = xmalloc(i - prev + 1);
	memcpy(str, &nodePtr->value[prev], i - prev);
	str[i - prev] = 0;
	if (!prev) {
	    fprint(f, "'%s'", str);
	} else {
	    fprint(f, "\n");
	    fprint(f, "%*c", indent-1, ' ');
	    fprint(f, "+ '%s'", str);
	}
	xfree(str);
    }
}

static void
fprintArgument(FILE *f, int indent, YangNode* nodePtr)
{
    switch (nodePtr->nodeKind) {
    case YANG_DECL_ORGANIZATION:
    case YANG_DECL_CONTACT:
    case YANG_DECL_DESCRIPTION:
    case YANG_DECL_REFERENCE:
	fprint(f, "\n");
	fprintString(f, indent, nodePtr);
	break;

    case YANG_DECL_PATTERN:
	fprint(f, " ");
	fprintPattern(f, indent + 7, nodePtr);
	break;

    case YANG_DECL_MODULE:
    case YANG_DECL_SUBMODULE:
    case YANG_DECL_TYPE:
    case YANG_DECL_REVISION:
    case YANG_DECL_IMPORT:
    case YANG_DECL_INCLUDE:
    case YANG_DECL_NOTIFICATION:
    case YANG_DECL_RPC:
    case YANG_DECL_USES:
    case YANG_DECL_ANYXML:
    case YANG_DECL_CHOICE:
    case YANG_DECL_LIST:
    case YANG_DECL_LEAF_LIST:
    case YANG_DECL_LEAF:
    case YANG_DECL_CONTAINER:
    case YANG_DECL_GROUPING:
    case YANG_DECL_BIT:
    case YANG_DECL_STATUS:
    case YANG_DECL_CONFIG:
    case YANG_DECL_MANDATORY:
    case YANG_DECL_TYPEDEF:
    case YANG_DECL_IF_FEATURE:
    case YANG_DECL_FEATURE:
    case YANG_DECL_BASE:
    case YANG_DECL_IDENTITY:
    case YANG_DECL_ARGUMENT:
    case YANG_DECL_EXTENSION:
    case YANG_DECL_BELONGS_TO:
    case YANG_DECL_YANG_VERSION:
	fprint(f, " %s", nodePtr->value);
	break;
	
    default: 
	fprint(f, " \"%s\"", nodePtr->value);
    }
}

static void
fprintYangNode(FILE *f, int indent, YangNode* nodePtr)
{
    YangNode *childPtr;
    int first = 1;
    
    switch (nodePtr->nodeKind) {
    case YANG_DECL_COMPLEX_TYPE:
    case YANG_DECL_INSTANCE:
    case YANG_DECL_INSTANCE_LIST:
    case YANG_DECL_INSTANCE_TYPE:
    case YANG_DECL_ABSTRACT:
    case YANG_DECL_EXTENDS:       
	fprintSegment(f, indent, nodePtr->extra, 0);
	fprint(f, " %s", nodePtr->value);
	break;
	
    case YANG_DECL_UNKNOWN_STATEMENT:
	fprintSegment(f, indent, nodePtr->value, 0);
	printf("%s", nodePtr->extra);
	if (nodePtr->extra) {
	    fprint(f, " \"%s\"", nodePtr->extra);
	}
	break;

    default: 
	if (nodePtr->nodeKind == YANG_DECL_LEAF
	    || nodePtr->nodeKind == YANG_DECL_LEAF_LIST
	    || nodePtr->nodeKind == YANG_DECL_LIST
	    || nodePtr->nodeKind == YANG_DECL_CONTAINER) {
	    fprintf(f, "\n");
	}
	if (indent) {
	    fprintSegment(f, indent, yangDeclAsString(nodePtr->nodeKind), 0);
	} else {
	    fprint(f, "%s", yangDeclAsString(nodePtr->nodeKind));
	}
	if (nodePtr->value) {
	    fprintArgument(f, indent, nodePtr);
	}
    }
    
    childPtr = yangGetFirstChildNode(nodePtr);
    first = 1;
    if (childPtr) {
        fprint(f, " {\n");
        for (; childPtr; childPtr = yangGetNextChildNode(childPtr)) {
	    if (first && !indent) {
		fprint(f, "\n");
	    }
	    first = 0;
	    
            fprintYangNode(f, indent + INDENT, childPtr);
	    
	    if (!indent && childPtr->nodeKind != YANG_DECL_NAMESPACE) {
		fprint(f, "\n");
	    }
        }
        if (indent) {
            fprintSegment(f, indent, "}\n", 0);
        } else {
            fprint(f, "}\n");
        }
    } else {
        fprint(f, ";\n");
    }
    
}

static void
fprintYangModule(FILE *f, SmiModule *smiModule, YangNode *yangModule, int flags)
{
    fprint(f, "/*\n");
    fprint(f, " * This YANG module has been generated by smidump "
	   SMI_VERSION_STRING ":\n");
    fprint(f, " *\n");
    fprint(f, " *      smidump -f yang");
    fprint(f, " %s\n", smiModule->name);
    fprint(f, " *\n");
    fprint(f, " * Do not edit. Edit the source file instead!\n");
    fprint(f, " */\n\n");
    if (! (flags & SMIDUMP_FLAG_SILENT) && (flags & SMIDUMP_FLAG_ERROR)) {
	fprintf(f, "/*\n * WARNING: this output may be incorrect due to "
		"significant parse errors\n */\n\n");
    }

    fprintYangNode(f, 0, yangModule);
}

static void
dumpYang(int modc, SmiModule **modv, int flags, char *output)
{
    SmiModule   *smiModule;
    YangNode    *yangModule;
    int		i;
    FILE	*f = stdout;

    if (output) {
        f = fopen(output, "w");
        if (!f) {
            fprintf(stderr, "smidump: cannot open %s for writing: ", output);
            perror(NULL);
            exit(1);
        }
    }

    for (i = 0; i < modc; i++) {        
        smiModule = modv[i];
	yangModule = NULL;
	if (smiModule->language == SMI_LANGUAGE_SMIV1
	    ||smiModule->language == SMI_LANGUAGE_SMIV2) {
	    yangModule = yangGetModuleFromSmiModule(smiModule, 0);
	}
	if (smiModule->language == SMI_LANGUAGE_YANG) {
	    yangModule = yangGetModule(smiModule->name);
	}
	if (yangModule) fprintYangModule(f, smiModule, yangModule, flags);
    }

    if (fflush(f) || ferror(f)) {
        perror("smidump: write error");
        exit(1);
    }

    if (output) {
        fclose(f);
    }
}



void initYang()
{
    static SmidumpDriverOption opt[] = {
	{ "indent", OPT_INT, &INDENT, 0,
	  "indentation (default 2)" },
        { 0, OPT_END, 0, 0 }
    };

    static SmidumpDriver driver = {
	"yang",
	dumpYang,
	0,
	SMIDUMP_DRIVER_CANT_UNITE | SMIDUMP_DRIVER_CANT_SPPI,
	"YANG (RFC 6020, RFC 6021)",
	opt,
	NULL
    };
    
    smidumpRegisterDriver(&driver);
}

#endif

