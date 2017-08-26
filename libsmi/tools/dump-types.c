/*
 * dump-types.c --
 *
 *      Operations to dump the type hierarchy in a human readable format.
 *
 * Copyright (c) 1999 Frank Strauss, Technical University of Braunschweig.
 * Copyright (c) 1999 J. Schoenwaelder, Technical University of Braunschweig.
 *
 * See the file "COPYING" for information on usage and redistribution
 * of this file, and for a DISCLAIMER OF ALL WARRANTIES.
 *
 * @(#) $Id: dump-types.c 1772 2012-04-01 12:15:23Z schoenw $
 */

/*
 * Decide how we want to name the implicitely defined types.
 */

#include <config.h>

#include <stdio.h>
#include <string.h>
#include <ctype.h>

#include "smi.h"
#include "smidump.h"


static int noimplicit = 0;


typedef struct BaseTypeCount {
    SmiBasetype basetype;
    int         counter;
} BaseTypeCount;


static BaseTypeCount basetypes[] = {
    { SMI_BASETYPE_INTEGER32, 0 },
    { SMI_BASETYPE_OCTETSTRING, 0 },
    { SMI_BASETYPE_OBJECTIDENTIFIER, 0 },
    { SMI_BASETYPE_UNSIGNED32, 0 },
    { SMI_BASETYPE_INTEGER64, 0 },
    { SMI_BASETYPE_UNSIGNED64, 0 },
    { SMI_BASETYPE_FLOAT32, 0 },
    { SMI_BASETYPE_FLOAT64, 0 },
    { SMI_BASETYPE_FLOAT128, 0 },
    { SMI_BASETYPE_ENUM, 0 },
    { SMI_BASETYPE_BITS, 0 },
    { SMI_BASETYPE_UNKNOWN, 0 }
};


typedef struct TypeNode {
    SmiType	*smiType;
    SmiModule	*smiModule;
    SmiBasetype smiBasetype;
    struct TypeNode *nextNodePtr;
    struct TypeNode *childNodePtr;
} TypeNode;


static TypeNode typeRoot = {
    NULL, NULL, SMI_BASETYPE_UNKNOWN, NULL, NULL
};


static int pmodc = 0;
static SmiModule **pmodv = NULL;


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



static char *getFlags(SmiModule *smiModule, SmiType *smiType)
{
    static char flags[4];
    int		i;
    
    memset(flags, 0, sizeof(flags));
    strcpy(flags, "---");

    switch (smiType->decl) {
    case SMI_DECL_IMPLICIT_TYPE:
	flags[0] = 'i';
	break;
    case SMI_DECL_TYPEASSIGNMENT:
	flags[0] = 'a';
	break;
    case SMI_DECL_TEXTUALCONVENTION:
    case SMI_DECL_TYPEDEF:
	flags[0] = 't';
	break;
    default:
	break;
    }

    for (i = 0; i < pmodc; i++) {
	if (strcmp(pmodv[i]->name, smiModule->name) == 0) {
	    break;
	}
    }

    if ((!smiModule) || (strlen(smiModule->name) == 0)) {
	flags[2] = 'b';
#if 0	/* xxx fixme */
    } else if (!moduleList) {
	flags[2] = '-';
    } else if (mPtr) {
	flags[2] = 'l';
#endif
    } else {
	flags[2] = 'i';
    }

    return flags;
}



static void initBaseTypeCount()
{
    int i;

    for (i = 0; i < sizeof(basetypes)/sizeof(BaseTypeCount); i++) {
	basetypes[i].counter = 0;
    }
}



static void incrBaseTypeCount(SmiBasetype basetype)
{
    int i;

    for (i = 0; i < sizeof(basetypes)/sizeof(BaseTypeCount); i++) {
	if (basetypes[i].basetype == basetype) {
	    basetypes[i].counter++;
	}
    }
}



static int getBaseTypeCount(SmiBasetype basetype)
{
    int i;

    for (i = 0; i < sizeof(basetypes)/sizeof(BaseTypeCount); i++) {
	if (basetypes[i].basetype == basetype) {
	    return basetypes[i].counter;
	}
    }

    return -1;
}



static void addToTypeTree(TypeNode *root,
			  SmiModule *smiModule, SmiType *smiType)
{
    TypeNode  *newType, **typePtrPtr;
    SmiType   *smiParentType;
    SmiModule *smiParentModule;

    if (! root) {
	return;
    }

    smiParentType = smiGetParentType(smiType);
    smiParentModule = smiParentType ? smiGetTypeModule(smiParentType) : NULL;

    if ((root->smiModule == smiParentModule && root->smiType == smiParentType)
	|| (! root->smiModule && ! root->smiType
	    && root->smiBasetype == smiType->basetype)) {

	 newType = xmalloc(sizeof(TypeNode));
	 newType->smiModule = smiModule;
	 newType->smiType = smiType;
	 newType->smiBasetype = smiType->basetype;
	 newType->childNodePtr = NULL;
	 newType->nextNodePtr = NULL;

	 for (typePtrPtr = &(root->childNodePtr);
	      *typePtrPtr; typePtrPtr = &((*typePtrPtr)->nextNodePtr)) ;
	 *typePtrPtr = newType;
	 return;
     }

     if (root->nextNodePtr) {
	 addToTypeTree(root->nextNodePtr, smiModule, smiType);
     }
     
     if (root->childNodePtr) {
	 addToTypeTree(root->childNodePtr, smiModule, smiType);
     }
}



static void freeTypeTree(TypeNode *root)
{
    if (root->childNodePtr) {
	if (root->childNodePtr->smiModule) {
	    freeTypeTree(root->childNodePtr);
	    root->childNodePtr = NULL;
	} else {
	    freeTypeTree(root->childNodePtr);
	}
    }
    
    if (root->nextNodePtr) {
	if (root->nextNodePtr->smiModule) {
	    freeTypeTree(root->nextNodePtr);
	    root->nextNodePtr = NULL;
	} else {
	    freeTypeTree(root->nextNodePtr);
	}
    }
    
    if (root->smiModule) {
	xfree(root);
    }
}



static TypeNode *findInTypeTree(TypeNode *root,
				SmiModule *smiModule, SmiType *smiType)
{
    TypeNode *result = NULL;
    
    if (root->smiModule && root->smiModule == smiModule
	&& smiType->name
	&& root->smiType && root->smiType == smiType) {
	result = root;
    }
    
    if (!result && root->childNodePtr) {
	result = findInTypeTree(root->childNodePtr, smiModule, smiType);
    }
    
    if (!result && root->nextNodePtr) {
	result = findInTypeTree(root->nextNodePtr, smiModule, smiType);
    }
    
    return result;
}



static char* getTypeName(TypeNode *typeNode)
{
    SmiRefinement *smiRefinement;
    SmiType *smiType;
    char *name = "?";

    if (typeNode->smiType->name) {
#if 1
	return xstrdup(typeNode->smiType->name);
#else
	char *s;
	s = xmalloc(strlen(typeNode->smiType->name)+
		    strlen(typeNode->smiModule ? typeNode->smiModule->name : "") +3);
	sprintf(s, "%s::%s", typeNode->smiModule ? typeNode->smiModule->name : "",
		typeNode->smiType->name);
	return s;
#endif
	
    } else if (typeNode->smiModule) {
	SmiNode *smiNode;

	for (smiNode = smiGetFirstNode(typeNode->smiModule, SMI_NODEKIND_ANY);
	     smiNode;
	     smiNode = smiGetNextNode(smiNode, SMI_NODEKIND_ANY)) {
	    if (smiGetNodeType(smiNode) == typeNode->smiType) {
		name = xmalloc(strlen(smiNode->name) + 3);
		sprintf(name, "(%s)", smiNode->name);
		return name;
	    }
	}

	for (smiNode = smiGetFirstNode(typeNode->smiModule,
				       SMI_NODEKIND_COMPLIANCE);
	     smiNode;
	     smiNode = smiGetNextNode(smiNode, SMI_NODEKIND_COMPLIANCE)) {
	    for(smiRefinement = smiGetFirstRefinement(smiNode);
		smiRefinement;
		smiRefinement = smiGetNextRefinement(smiRefinement)) {
		smiType = smiGetRefinementType(smiRefinement);
		if (smiType == typeNode->smiType) {
		    smiNode = smiGetRefinementNode(smiRefinement);
		    name = xmalloc(strlen(smiNode->name) + 3);
		    sprintf(name, "(%s)", smiNode->name);
		    return name;
		}
		smiType = smiGetRefinementWriteType(smiRefinement);
		if (smiType == typeNode->smiType) {
		    smiNode = smiGetRefinementNode(smiRefinement);
		    name = xmalloc(strlen(smiNode->name) + 3);
		    sprintf(name, "(%s)", smiNode->name);
		    return name;
		}
	    }
	}
    }

    return xstrdup(name);
}



static void fprintRestrictions(FILE *f, SmiType *smiType)
{
    SmiNamedNumber *nn;
    SmiRange       *range;
    char           s1[40], s2[40];
    int            i;
    
    if ((smiType->basetype == SMI_BASETYPE_ENUM) ||
	(smiType->basetype == SMI_BASETYPE_BITS)) {
	for(i = 0, nn = smiGetFirstNamedNumber(smiType);
	    nn ; nn = smiGetNextNamedNumber(nn), i++) {
	    fprintf(f, "%s%s(%ld)", (i == 0) ? " {" : ", ",
		   nn->name, nn->value.value.integer32);
	}
	if (i) fprintf(f, "}");
    } else {
	for(i = 0, range = smiGetFirstRange(smiType);
	    range ; range = smiGetNextRange(range), i++) {
	    strcpy(s1, smiValueAsString(&range->minValue, smiType,
		       SMI_LANGUAGE_SMIV2));
	    strcpy(s2, smiValueAsString(&range->maxValue, smiType,
		       SMI_LANGUAGE_SMIV2));
	    fprintf(f, "%s%s", (i == 0) ? " [" : ", ", s1);
	    if (strcmp(s1, s2)) fprintf(f, "..%s", s2);
	}
	if (i) fprintf(f, "]");
    }
}



static void fprintTypeTree(FILE *f, TypeNode *root, char *prefix)
{
    TypeNode *typeNode, *nextNode;
    char *name;
    int namelen = -1;

    if (root->smiModule) {
	fprintf(f, "%s  |\n", prefix);
    }

    for (typeNode = root; typeNode; typeNode = typeNode->nextNodePtr) {
	if (typeNode->smiType) {
	    int len;
	    name = getTypeName(typeNode);
	    len = strlen(name);
	    if (len > namelen) namelen = len;
	    xfree(name);
	}
    }
    
    for (typeNode = root; typeNode; typeNode = typeNode->nextNodePtr) {
 	if (typeNode != &typeRoot) {
	    char c = '+', *flags;
	    if (typeNode->smiType) {
		name = getTypeName(typeNode);
		c = getStatusChar(typeNode->smiType->status);
		if (getBaseTypeCount(typeNode->smiBasetype)) {
		    flags = getFlags(typeNode->smiModule, typeNode->smiType);
		    if (flags && *flags) {
			fprintf(f, "%s  %c-- %s %-*s", prefix, c, flags,
				namelen, name);
		    } else {
			fprintf(f, "%s  %c--%-*s", prefix, c,
				namelen, name);
		    }
		    fprintRestrictions(f, typeNode->smiType);
		    if (typeNode->smiType->format) {
			fprintf(f, "\t\"%s\"", typeNode->smiType->format);
		    }
		    fprintf(f, "\n");
		}
		xfree(name);
	    } else {
		fprintf(f, "xxxx\n");
	    }
 	}
 	if (typeNode->childNodePtr) {
 	    char *newprefix;
 	    newprefix = xmalloc(strlen(prefix)+10);
 	    strcpy(newprefix, prefix);
 	    if (typeNode != &typeRoot) {
		for (nextNode = typeNode->nextNodePtr;
		     nextNode; nextNode = nextNode->nextNodePtr) {
		    if (getBaseTypeCount(nextNode->smiBasetype)) {
			break;
		    }
		}
 		if (nextNode) {
  		    strcat(newprefix, "  |");
 		} else {
 		    strcat(newprefix, "   ");
 		}
 	    }
 	    fprintTypeTree(f, typeNode->childNodePtr, newprefix);
 	    xfree(newprefix);
	    for (nextNode = typeNode->nextNodePtr;
		 nextNode; nextNode = nextNode->nextNodePtr) {
		if (getBaseTypeCount(nextNode->smiBasetype)) {
		    break;
		}
	    }
	    if (nextNode) {
		fprintf(f, "%s  |\n", prefix);
	    }
 	}
    }
}



static void addType(SmiType *smiType)
{
    SmiModule *smiModule;
    SmiType   *smiParentType;

    smiModule = smiGetTypeModule(smiType);
    if (! smiModule) {
	return;
    }
    
    if (findInTypeTree(&typeRoot, smiModule, smiType)) {
	return;
    }

    smiParentType = smiGetParentType(smiType);
    if (smiParentType && smiParentType->name) {
 	if (smiParentType) {
 	    addType(smiParentType);
 	}
    }

    addToTypeTree(&typeRoot, smiModule, smiType);
    incrBaseTypeCount(smiType->basetype);
}



static void dumpTypes(int modc, SmiModule **modv, int flags, char *output)
{
    SmiNode	  *smiNode;
    SmiType       *smiType;
    SmiRefinement *smiRefinement;
    int		  i;
    FILE          *f = stdout;
    const unsigned int nodekind = SMI_NODEKIND_COLUMN | SMI_NODEKIND_SCALAR;
    
    if (output) {
	f = fopen(output, "w");
	if (!f) {
	    fprintf(stderr, "smidump: cannot open %s for writing: ", output);
	    perror(NULL);
	    exit(1);
	}
    }

    pmodc = modc;
    pmodv = modv;
    
    initBaseTypeCount();

    for (i = 0; i < modc; i++) {

	if (! (flags & SMIDUMP_FLAG_UNITE)) {
	    initBaseTypeCount();
	}

	for (smiType = smiGetFirstType(modv[i]);
	     smiType;
	     smiType = smiGetNextType(smiType)) {
	    addType(smiType);
	}

	for (smiNode = smiGetFirstNode(modv[i], nodekind);
	     smiNode;
	     smiNode = smiGetNextNode(smiNode, nodekind)) {
	    smiType = smiGetNodeType(smiNode);
	    if (smiType) {
		if (!noimplicit && smiType->decl == SMI_DECL_IMPLICIT_TYPE) {
		    addType(smiType);
		}
		incrBaseTypeCount(smiType->basetype);
	    }
	}
	
	for (smiNode = smiGetFirstNode(modv[i], SMI_NODEKIND_COMPLIANCE);
	     smiNode;
	     smiNode = smiGetNextNode(smiNode, SMI_NODEKIND_COMPLIANCE)) {
	    for (smiRefinement = smiGetFirstRefinement(smiNode);
		 smiRefinement;
		 smiRefinement = smiGetNextRefinement(smiRefinement)) {
		smiType = smiGetRefinementType(smiRefinement);
		if (smiType) {
		    if (!noimplicit
			&& smiType->decl == SMI_DECL_IMPLICIT_TYPE) {
			addType(smiType);
		    }
		    incrBaseTypeCount(smiType->basetype);
		}
		smiType = smiGetRefinementWriteType(smiRefinement);
		if (smiType) {
		    if (!noimplicit
			&& smiType->decl == SMI_DECL_IMPLICIT_TYPE) {
			addType(smiType);
		    }
		    incrBaseTypeCount(smiType->basetype);
		}
	    }
	}

	if (! (flags & SMIDUMP_FLAG_UNITE)) {
	    if (! (flags & SMIDUMP_FLAG_SILENT)) {
		fprintf(f, "# %s type derivation tree (generated by smidump "
			SMI_VERSION_STRING ")\n\n", modv[i]->name);
	    }
	    if (! (flags & SMIDUMP_FLAG_SILENT) && (flags & SMIDUMP_FLAG_ERROR)) {
		fprintf(f, "# WARNING: this output may be incorrect due to "
			"significant parse errors\n\n");
	    }
	    fprintTypeTree(f, &typeRoot, "");
	    freeTypeTree(&typeRoot);
	}
    }

    if (flags & SMIDUMP_FLAG_UNITE) {
	if (! (flags & SMIDUMP_FLAG_SILENT)) {
	    fprintf(f, "# united type derivation tree (generated by smidump "
		   SMI_VERSION_STRING ")\n\n");
	}
	if (! (flags & SMIDUMP_FLAG_SILENT) && (flags & SMIDUMP_FLAG_ERROR)) {
	    fprintf(f, "# WARNING: this output may be incorrect due to "
		    "significant parse errors\n\n");
	}
	fprintTypeTree(f, &typeRoot, "");
	freeTypeTree(&typeRoot);
    }

    if (fflush(f) || ferror(f)) {
	perror("smidump: write error");
	exit(1);
    }

    if (output) {
	fclose(f);
    }
}



void initTypes()
{
    static SmidumpDriverOption opt[] = {
	{ "no-implicit", OPT_FLAG, &noimplicit, 0,
	  "ignore implicit type definitions"},
        { 0, OPT_END, 0, 0 }
    };
    
    static SmidumpDriver driver = {
	"types",
	dumpTypes,
	SMI_FLAG_NODESCR,
	SMIDUMP_DRIVER_CANT_YANG,
	"recursive list of all derived types",
	opt,
	NULL
    };
    
    smidumpRegisterDriver(&driver);
}
