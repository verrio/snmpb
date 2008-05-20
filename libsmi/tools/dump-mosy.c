/*
 * dump-mosy.c --
 *
 *      Operations to dump MIB modules in the MOSY output format.
 *
 * Copyright (c) 1999 Frank Strauss, Technical University of Braunschweig.
 * Copyright (c) 1999 J. Schoenwaelder, Technical University of Braunschweig.
 *
 * See the file "COPYING" for information on usage and redistribution
 * of this file, and for a DISCLAIMER OF ALL WARRANTIES.
 *
 * @(#) $Id: dump-mosy.c 8090 2008-04-18 12:56:29Z strauss $
 */

#include <config.h>

#include <stdio.h>
#include <string.h>

#include "smi.h"
#include "smidump.h"


static char *ignoreTypeRanges[] = {
    "TimeTicks",
    "Counter32",
    "Gauge32",
    "Counter64",
    NULL
};


static char *getStatusString(SmiStatus status)
{
    return
	(status == SMI_STATUS_CURRENT)     ? "current" :
	(status == SMI_STATUS_DEPRECATED)  ? "deprecated" :
	(status == SMI_STATUS_OBSOLETE)    ? "obsolete" :
	(status == SMI_STATUS_MANDATORY)   ? "mandatory" :
	(status == SMI_STATUS_OPTIONAL)    ? "optional" :
					     "<unknown>";
}


static char *getAccessString(SmiAccess access, int create)
{
    return
	(create && (access == SMI_ACCESS_READ_WRITE)) ? "read-create" :
	(access == SMI_ACCESS_NOT_ACCESSIBLE) ? "not-accessible" :
	(access == SMI_ACCESS_NOTIFY)	      ? "accessible-for-notify" :
	(access == SMI_ACCESS_READ_ONLY)      ? "read-only" :
	(access == SMI_ACCESS_READ_WRITE)     ? "read-write" :
						"<unknown>";
}


static char *getBasetypeString(SmiBasetype basetype)
{
    return
        (basetype == SMI_BASETYPE_UNKNOWN)           ? "<UNKNOWN>" :
        (basetype == SMI_BASETYPE_OCTETSTRING)       ? "OctetString" :
        (basetype == SMI_BASETYPE_OBJECTIDENTIFIER)  ? "ObjectID" :
        (basetype == SMI_BASETYPE_UNSIGNED32)        ? "Unsigned32" :
        (basetype == SMI_BASETYPE_INTEGER32)         ? "Integer32" :
        (basetype == SMI_BASETYPE_UNSIGNED64)        ? "Unsigned64" :
        (basetype == SMI_BASETYPE_INTEGER64)         ? "Integer64" :
        (basetype == SMI_BASETYPE_FLOAT32)           ? "Float32" :
        (basetype == SMI_BASETYPE_FLOAT64)           ? "Float64" :
        (basetype == SMI_BASETYPE_FLOAT128)          ? "Float128" :
        (basetype == SMI_BASETYPE_ENUM)              ? "INTEGER" :
        (basetype == SMI_BASETYPE_BITS)              ? "Bits" :
                                                   "<unknown>";
}



static char *getOidString(SmiNode *smiNode, int importedParent)
{
    SmiNode	 *parentNode;
    SmiModule	 *smiModule;
    static char	 s[200];
    char	 append[200];
    unsigned int i;
    
    append[0] = 0;

    parentNode = smiNode;
    smiModule = smiGetNodeModule(smiNode);
    
    do {

	if (parentNode->oidlen <= 1) {
	    break;
	}
	
	/* prepend the cut-off subidentifier to `append'. */
	strcpy(s, append);
	sprintf(append, ".%u%s", parentNode->oid[parentNode->oidlen-1], s);

	/* retrieve the parent SmiNode */
	parentNode = smiGetParentNode(parentNode);

	if (!parentNode) {
	    sprintf(s, "%s", append);
	    return s;
	}
	
	/* found an imported or a local parent node? */
	if ((parentNode->decl != SMI_DECL_IMPL_OBJECT) 
	    && ((parentNode->name && strlen(parentNode->name)) 
		&& (smiIsImported(smiModule, NULL, parentNode->name) 
		    || (!importedParent 
			&& (smiGetNodeModule(parentNode) == smiModule)) 
		    || (parentNode->oidlen == 1)))) {
	    sprintf(s, "%s%s", parentNode->name, append);
	    return s;
	}
	
    } while (parentNode);

    s[0] = 0;
    for (i=0; i < smiNode->oidlen; i++) {
	if (i) strcat(s, ".");
	sprintf(&s[strlen(s)], "%u", smiNode->oid[i]);
    }
    return s;
}



static char *getValueString(SmiValue *valuePtr)
{
    static char    s[1024];

    s[0] = 0;
    
    switch (valuePtr->basetype) {
    case SMI_BASETYPE_UNSIGNED32:
	sprintf(s, "%lu", valuePtr->value.unsigned32);
	break;
    case SMI_BASETYPE_INTEGER32:
	sprintf(s, "%ld", valuePtr->value.integer32);
	break;
    case SMI_BASETYPE_UNSIGNED64:
	sprintf(s, UINT64_FORMAT, valuePtr->value.unsigned64);
	break;
    case SMI_BASETYPE_INTEGER64:
	sprintf(s, INT64_FORMAT, valuePtr->value.integer64);
	break;
    case SMI_BASETYPE_FLOAT32:
    case SMI_BASETYPE_FLOAT64:
    case SMI_BASETYPE_FLOAT128:
	break;
    case SMI_BASETYPE_ENUM:
    case SMI_BASETYPE_OCTETSTRING:
    case SMI_BASETYPE_BITS:
    case SMI_BASETYPE_OBJECTIDENTIFIER:
	/* not required in MOSY format */
	break;
    case SMI_BASETYPE_UNKNOWN:
	break;
    case SMI_BASETYPE_POINTER:
	break;
    }

    return s;
}



static void printIndex(FILE *f, SmiNode *smiNode)
{
    char *indexname;
    int  i;
    SmiElement *smiElement;
    
    fprintf(f, "%%%-19s %-16s \"", "ei", smiNode->name);
    indexname = NULL;
    for (i = -1, smiElement = smiGetFirstElement(smiNode);
	 smiElement; smiElement = smiGetNextElement(smiElement), i++) {
	if (i > 0) fprintf(f, " ");
	if (indexname) {
	    fprintf(f, "%s", indexname);
	}
	indexname = smiGetElementNode(smiElement)->name;
    }
    if (indexname) {
	fprintf(f, "%s%s%s",
		(i > 0) ? " " : "",
		(smiNode->implied) ? "*" : "",
		indexname);
    }
    fprintf(f, "\"\n");
}



static void printAssignements(FILE *f, SmiModule *smiModule)
{
    int		 cnt = 0;
    SmiNode	 *smiNode;

    for (smiNode = smiGetFirstNode(smiModule, SMI_NODEKIND_NODE);
	 smiNode; smiNode = smiGetNextNode(smiNode, SMI_NODEKIND_NODE)) {

	cnt++;

	if (smiNode->status == SMI_STATUS_UNKNOWN &&
	    smiNode != smiGetModuleIdentityNode(smiModule)) {
	    fprintf(f, "%-20s %s\n", smiNode->name, getOidString(smiNode, 0));
	    fprintf(f, "%%n0 %-16s object-id\n", smiNode->name);
	}
    }

    if (cnt) {
	fprintf(f, "\n");
    }
}



static void printTypedefs(FILE *f, SmiModule *smiModule)
{
    int		   i;
    SmiType	   *smiType, *smiParentType;
    SmiNamedNumber *nn;
    char	   *type_name;
    
    for (i = 0, smiType = smiGetFirstType(smiModule);
	 smiType; smiType = smiGetNextType(smiType)) {
	
	smiParentType = smiGetParentType(smiType);
	type_name = smiParentType->name;
	
	if (smiParentType->decl == SMI_DECL_IMPLICIT_TYPE) {
	    smiParentType = smiGetParentType(smiParentType);
	    type_name = smiParentType->name;
	}
	if (smiParentType->basetype == SMI_BASETYPE_OBJECTIDENTIFIER) {
	    type_name = "ObjectID";
	}
	if (smiParentType->basetype == SMI_BASETYPE_ENUM) {
	    type_name = "INTEGER";
	}

	fprintf(f, "%%%-19s %-16s %-15s \"%s\"\n", "tc",
		smiType->name, type_name,
		smiType->format ? smiType->format : "");
	
	for (i = 0, nn = smiGetFirstNamedNumber(smiType);
	     nn ; i++, nn = smiGetNextNamedNumber(nn)) {
	    fprintf(f, "%%%-19s %-16s %-15s %s\n", "es",
		    smiType->name, nn->name,
		    getValueString(&nn->value));
	}
    }
}



static void printObjects(FILE *f, SmiModule *smiModule)
{
    int		   i, j, ignore, cnt = 0, aggregate, create;
    char	   *type_name;
    SmiNode	   *smiNode, *relatedNode;
    SmiType	   *smiType;
    SmiNamedNumber *smiNamedNumber;
    SmiRange       *smiRange;
    
    for (smiNode = smiGetFirstNode(smiModule, SMI_NODEKIND_ANY);
	 smiNode; smiNode = smiGetNextNode(smiNode, SMI_NODEKIND_ANY)) {

	if (smiNode->nodekind != SMI_NODEKIND_NODE
	    && smiNode->nodekind != SMI_NODEKIND_SCALAR
	    && smiNode->nodekind != SMI_NODEKIND_TABLE
	    && smiNode->nodekind != SMI_NODEKIND_ROW
	    && smiNode->nodekind != SMI_NODEKIND_COLUMN) {
	    continue;
	}
	
	cnt++;

	if (smiNode->nodekind == SMI_NODEKIND_NODE) {
	    if (smiNode->status != SMI_STATUS_UNKNOWN &&
		smiNode != smiGetModuleIdentityNode(smiModule)) {
		fprintf(f, "%-20s %s\n", smiNode->name,
			getOidString(smiNode, 0));
		fprintf(f, "%%n0 %-16s object-id\n", smiNode->name);
	    }
	    continue;
	}

	aggregate = smiNode->nodekind == SMI_NODEKIND_TABLE
	    || smiNode->nodekind == SMI_NODEKIND_ROW;

	type_name = NULL;
	smiType = smiGetNodeType(smiNode);
	if (!aggregate) {
	    if (! smiType) {
		continue;
	    }
	    type_name = getBasetypeString(smiType->basetype);
	    if (smiType && (smiType->decl != SMI_DECL_IMPLICIT_TYPE)) {
		type_name = smiType->name;
		if (!strcmp(type_name, "ObjectIdentifier")) {
		    type_name = "ObjectID";
		}
	    }
	
	    if (smiType && smiType->decl == SMI_DECL_IMPLICIT_TYPE) {
		type_name = smiGetParentType(smiType)->name;
		if (smiType->basetype == SMI_BASETYPE_OBJECTIDENTIFIER) {
		    type_name = "ObjectID";
		}
		if (smiType->basetype == SMI_BASETYPE_ENUM) {
		    type_name = "INTEGER";
		}
	    }
	} else {
	    type_name = "Aggregate";
	}

	if (smiNode->nodekind == SMI_NODEKIND_COLUMN) {
	    create = smiGetParentNode(smiNode)->create;
	} else {
	    create = 0;
	}
	
	fprintf(f, "%-20s %-16s ", smiNode->name, getOidString(smiNode, 0));
	fprintf(f, "%-15s %-15s %s\n", type_name,
		getAccessString(smiNode->access, create),
		getStatusString(smiNode->status));

	relatedNode = smiGetRelatedNode(smiNode);
	switch (smiNode->indexkind) {
	case SMI_INDEX_INDEX:
	case SMI_INDEX_REORDER:
	    printIndex(f, smiNode);
	    break;
	case SMI_INDEX_EXPAND:	/* TODO: we have to do more work here! */
	    break;
	case SMI_INDEX_AUGMENT:
	    if (relatedNode) {
		fprintf(f, "%%%-19s %-16s %s\n", "ea",
			smiNode->name, relatedNode->name);
	    }
	    break;
	case SMI_INDEX_SPARSE:
	    if (relatedNode) {
		printIndex(f, relatedNode);
	    }
	    break;
	case SMI_INDEX_UNKNOWN:
	    break;	    
	}

	if (smiType && smiType->decl == SMI_DECL_IMPLICIT_TYPE) {
	    for (i = 0, smiNamedNumber = smiGetFirstNamedNumber(smiType);
		smiNamedNumber;
		i++, smiNamedNumber = smiGetNextNamedNumber(smiNamedNumber)) {
		fprintf(f, "%%%-19s %-16s %-15s %s\n", "ev",
			smiNode->name, smiNamedNumber->name,
			getValueString(&smiNamedNumber->value));
	    }

	    for (ignore = 0, j = 0; ignoreTypeRanges[j]; j++) {
		if (strcmp(type_name, ignoreTypeRanges[j]) == 0) {
		    ignore++;
		    break;
		}
	    }

	    if (! ignore) {
		for (smiRange = smiGetFirstRange(smiType);
		     smiRange;
		     smiRange = smiGetNextRange(smiRange)) {
		    fprintf(f, "%%%-19s %-16s %-15s ", "er",
			    smiNode->name,
			    getValueString(&smiRange->minValue));
		    fprintf(f, "%s\n", getValueString(&smiRange->maxValue));
		}
	    }
	}
    }

    if (cnt) {
	fprintf(f, "\n");
    }
}



static void printNotifications(FILE *f, SmiModule *smiModule)
{
    int		 cnt = 0;
    SmiNode	 *smiNode;
    
    for (smiNode = smiGetFirstNode(smiModule, SMI_NODEKIND_NOTIFICATION);
	 smiNode; 
	 smiNode = smiGetNextNode(smiNode, SMI_NODEKIND_NOTIFICATION)) {

	cnt++;
	
	fprintf(f, "%-20s %s\n", smiNode->name, getOidString(smiNode, 0));
	fprintf(f, "%%n0 %-16s notification\n", smiNode->name);
    }

    if (cnt) {
	fprintf(f, "\n");
    }
}



static void printGroups(FILE *f, SmiModule *smiModule)
{
    SmiNode	*smiNode, *smiNodeMember;
    SmiElement  *smiElement;
    int		cnt = 0, objects, notifications;
    
    for (smiNode = smiGetFirstNode(smiModule, SMI_NODEKIND_GROUP);
	 smiNode; smiNode = smiGetNextNode(smiNode, SMI_NODEKIND_GROUP)) {

	cnt ++;

	for (objects = 0, notifications = 0,
	     smiElement = smiGetFirstElement(smiNode);
	     smiElement;
	     smiElement = smiGetNextElement(smiElement)) {

	    smiNodeMember = smiGetElementNode(smiElement);
	    
	    objects += 
		(smiNodeMember->nodekind == SMI_NODEKIND_SCALAR)
		|| (smiNodeMember->nodekind == SMI_NODEKIND_COLUMN);
	    notifications +=
		(smiNodeMember->nodekind == SMI_NODEKIND_NOTIFICATION);
	}

	fprintf(f, "%-20s %s\n", smiNode->name, getOidString(smiNode, 0));
	fprintf(f, "%%n0 %-16s %s\n", smiNode->name,
		(objects && ! notifications) ? "object-group" :
		(! objects && notifications) ? "notification-group" : "group");
    }

    if (cnt) {
	fprintf(f, "\n");
    }
}



static void printCompliances(FILE *f, SmiModule *smiModule)
{
    int		  cnt = 0;
    SmiNode	  *smiNode;
    
    for (smiNode = smiGetFirstNode(smiModule, SMI_NODEKIND_COMPLIANCE);
	 smiNode; smiNode = smiGetNextNode(smiNode, SMI_NODEKIND_COMPLIANCE)) {
	
	cnt++;

	fprintf(f, "%-20s %s\n", smiNode->name, getOidString(smiNode, 0));
	fprintf(f, "%%n0 %-16s module-compliance\n", smiNode->name);
    }
	    
    if (cnt) {
	fprintf(f, "\n");
    }
}



static void dumpMosy(int modc, SmiModule **modv, int flags, char *output)
{
    SmiNode	*smiNode;
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

	if (! (flags & SMIDUMP_FLAG_SILENT)) {
	    fprintf(f,
		    "-- automatically generated by smidump %s, do not edit!\n",
		    SMI_VERSION_STRING);
	    fprintf(f,
		    "\n-- object definitions compiled from %s\n\n",
		    modv[i]->name);
	}

	if (! (flags & SMIDUMP_FLAG_SILENT) && (flags & SMIDUMP_FLAG_ERROR)) {
	    fprintf(f, "-- WARNING: this output may be incorrect due to "
		    "significant parse errors\n\n");
	}
	
	smiNode = smiGetModuleIdentityNode(modv[i]);
	if (smiNode) {
	    SmiNode *parent = smiGetParentNode(smiNode);
	    fprintf(f, "%-20s %s\n", smiNode->name, getOidString(smiNode, !parent || parent->nodekind == SMI_NODEKIND_UNKNOWN ));
	    fprintf(f, "%%n0 %-16s module-identity\n", smiNode->name);
	    fprintf(f, "\n");
	}
	
	printAssignements(f, modv[i]);
	printTypedefs(f, modv[i]);
	printObjects(f, modv[i]);
	printNotifications(f, modv[i]);
	printGroups(f, modv[i]);
	printCompliances(f, modv[i]);
    }

    if (fflush(f) || ferror(f)) {
	perror("smidump: write error");
	exit(1);
    }

    if (output) {
	fclose(f);
    }
}



void initMosy()
{
    
    static SmidumpDriver driver = {
	"mosy",
	dumpMosy,
	SMI_FLAG_NODESCR,
	SMIDUMP_DRIVER_CANT_UNITE,
	"intermediate format generated by the mosy compiler",
	NULL,
	NULL
    };
    
    smidumpRegisterDriver(&driver);
}
