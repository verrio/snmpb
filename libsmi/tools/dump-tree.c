/*
 * dump-tree.c --
 *
 *      Operations to dump the OID tree in a human readable format.
 *
 * Copyright (c) 1999 Frank Strauss, Technical University of Braunschweig
 * Copyright (c) 1999 J. Schoenwaelder, Technical University of Braunschweig
 * Copyright (c) 2002 J. Schoenwaelder, University of Osnabrueck
 * Copyright (c) 2011 J. Schoenwaelder, Jacobs University Bremen
 *
 * See the file "COPYING" for information on usage and redistribution
 * of this file, and for a DISCLAIMER OF ALL WARRANTIES.
 *
 * @(#) $Id: dump-tree.c 1813 2014-10-13 12:53:06Z schoenw $
 */

#include <config.h>

#include <stdio.h>
#include <string.h>

#include "smi.h"
#include "smidump.h"


static int pmodc = 0;
static SmiModule **pmodv = NULL;

static int ignoreconformance = 0;
static int ignoreleafs = 0;
static int full = 0;
static int noids = 0;

static char *getSmiFlags(SmiNode *smiNode)
{
    SmiNode *parentNode = smiGetParentNode(smiNode);

#if 0
    switch (smiNode->access) {
    case SMI_ACCESS_UNKNOWN:
	return "---";
    case SMI_ACCESS_NOT_ACCESSIBLE:
	return "---";
    case SMI_ACCESS_EVENT_ONLY:
	return "---";
    case SMI_ACCESS_NOTIFY:
	return "--n";
    case SMI_ACCESS_READ_ONLY:
	return "r-n";
    case SMI_ACCESS_READ_WRITE:
	return (parentNode && parentNode->create) ? "rcn" : "rwn";
    case SMI_ACCESS_NOT_IMPLEMENTED:
	return "---";
    case SMI_ACCESS_INSTALL:
	return "-i-";
    case SMI_ACCESS_INSTALL_NOTIFY:
	return "-in";
    case SMI_ACCESS_REPORT_ONLY:
	return "--r";
    }
#else
    switch (smiNode->access) {
    case SMI_ACCESS_UNKNOWN:
	return "---";
    case SMI_ACCESS_NOT_ACCESSIBLE:
	return "---";
    case SMI_ACCESS_EVENT_ONLY:
	return "---";
    case SMI_ACCESS_NOTIFY:
	return "-n-";
    case SMI_ACCESS_READ_ONLY:
	return "-r-";
    case SMI_ACCESS_READ_WRITE:
	return (parentNode && parentNode->create) ? "-rc" : "-rw";
    case SMI_ACCESS_NOT_IMPLEMENTED:
	return "---";
    case SMI_ACCESS_INSTALL:
	return "-i-";
    case SMI_ACCESS_INSTALL_NOTIFY:
	return "-in";
    case SMI_ACCESS_REPORT_ONLY:
	return "--r";
    }
#endif

    return "";
}


#ifdef BACKEND_YANG
static char *getYangFlags(YangNode *yangNode)
{
    switch (yangNode->config) {
    case YANG_CONFIG_DEFAULT:
	return "--";
    case YANG_CONFIG_FALSE:
	return "ro";
    case YANG_CONFIG_TRUE:
	return "rw";
    }

    return "";
}
#endif


static char getSmiStatusChar(SmiStatus status)
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


#ifdef BACKEND_YANG
static char getYangStatusChar(YangStatus status)
{
    switch (status) {
    case YANG_STATUS_DEFAULT_CURRENT:
	return '+';
    case YANG_STATUS_CURRENT:
	return '+';
    case YANG_STATUS_DEPRECATED:
	return 'x';
    case YANG_STATUS_OBSOLETE:
	return 'o';
    }

    return ' ';
}
#endif


static char *getSmiTypeName(SmiNode *smiNode)
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

    type = smiStrdup(smiType->name);
    return type;
}


#ifdef BACKEND_YANG
static char *getYangTypeName(YangNode *yangNode)
{
    char *type;
    YangNode *typeNode = NULL;
    
    for (typeNode = yangGetFirstChildNode(yangNode);
	 typeNode; typeNode = yangGetNextChildNode(typeNode)) {
	if (typeNode->nodeKind == YANG_DECL_TYPE) break;
    }

    if (! typeNode) {
	return NULL;
    }

    type = smiStrdup(typeNode->value);
    return type;
}
#endif


static char *getSmiLabel(SmiNode *smiNode)
{
    char *label;

    if (noids) {
	label = smiStrdup(smiNode->name);
    } else {
	
	if (smiNode->oid) {
	    smiAsprintf(&label, "%s(%u)",
			smiNode->name,
			smiNode->oid[smiNode->oidlen-1]);
	} else {
	    smiAsprintf(&label, "%s(?)", smiNode->name);
	}
    }

    return label;
}



static void fprintIndex(FILE *f, SmiNode *smiNode)
{
    char *indexname;
    int  i;
    SmiElement *smiElement;
    
    indexname = NULL;
    for (i = -1, smiElement = smiGetFirstElement(smiNode);
	 smiElement; smiElement = smiGetNextElement(smiElement), i++) {
	if (i > 0) fprintf(f, ",");
	if (indexname) {
	    fprintf(f, "%s", indexname);
	}
	indexname = smiGetElementNode(smiElement)->name;
    }
    if (indexname) {
	fprintf(f, "%s%s%s",
		(i > 0) ? "," : "",
		(smiNode->implied) ? "*" : "",
		indexname);
    }
}



static void fprintObjects(FILE *f, SmiNode *smiNode)
{
    char *objectname;
    int  i;
    SmiElement *smiElement;

    objectname = NULL;
    for (i = -1, smiElement = smiGetFirstElement(smiNode);
	 smiElement;
	 smiElement = smiGetNextElement(smiElement), i++) {
	if (i > 0) fprintf(f, ",");
	if (objectname) {
	    fprintf(f, "%s", objectname);
	}
	objectname = smiGetElementNode(smiElement)->name;
    }
    if (objectname) {
	fprintf(f, "%s%s", (i > 0) ? "," : "", objectname);
    }
}



static int isPartOfLoadedModules(SmiNode *smiNode)
{
    SmiModule *smiModule;
    int i;
    
    smiModule = smiGetNodeModule(smiNode);

    for (i = 0; i < pmodc; i++) {
	if (strcmp(pmodv[i]->name, smiModule->name) == 0) {
	    return 1;
	}
    }
    return 0;
}

/*
 * The following function pruneSubTree() is tricky. There are some
 * interactions between the supported options. See the detailed
 * comments below. Good examples to test the implemented behaviour
 * are:
 *
 * smidump -u -f tree --tree-no-leaf IF-MIB ETHER-CHIPSET-MIB
 *
 * (And the example above does _not_ work in combination with
 * --tree-no-conformance so the code below is still broken.)
 */

static int pruneSubTree(SmiNode *smiNode)
{
    SmiNode   *childNode;

    const int confmask = (SMI_NODEKIND_GROUP | SMI_NODEKIND_COMPLIANCE);
    const int leafmask = (SMI_NODEKIND_GROUP | SMI_NODEKIND_COMPLIANCE
			  | SMI_NODEKIND_COLUMN | SMI_NODEKIND_SCALAR
			  | SMI_NODEKIND_ROW | SMI_NODEKIND_NOTIFICATION);

    if (! smiNode) {
	return 1;
    }

    /*
     * First, prune all nodes which the user has told us to ignore.
     * In the case of ignoreleafs, we have to special case nodes with
     * an unknown status (which actually represent OBJECT-IDENTITY
     * definitions). More special case code is needed to exclude
     * module identity nodes.
     */

    if (ignoreconformance && (smiNode->nodekind & confmask)) {
	return 1;
    }

    if (ignoreleafs) {
	if (smiNode->nodekind & leafmask) {
	    return 1;
	}
	if (smiNode->nodekind == SMI_NODEKIND_NODE
	    && smiNode->status != SMI_STATUS_UNKNOWN) {
	    SmiModule *smiModule = smiGetNodeModule(smiNode);
	    if (smiModule && smiNode != smiGetModuleIdentityNode(smiModule)) {
		return 1;
	    }
	}
    }

    /*
     * Next, generally do not prune nodes that belong to the set of
     * modules we are looking at.
     */

    if (isPartOfLoadedModules(smiNode)) {
	if (!ignoreconformance || !smiGetFirstChildNode(smiNode)) {
	    return 0;
	}
    }

    /*
     * Finally, prune all nodes where all child nodes are pruned.
     */

    for (childNode = smiGetFirstChildNode(smiNode);
	 childNode;
	 childNode = smiGetNextChildNode(childNode)) {

	/*
	 * In the case of ignoreleafs, we have to peek at the child
	 * nodes. Otherwise, we would prune too much. we still want to
	 * see the path to the leafs we have pruned away. This also
	 * interacts with the semantics of ignoreconformance since we
	 * still want in combination with ignoreleafs to see the path
	 * to the pruned conformance leafs.
	 */
	
	if (ignoreleafs && (childNode->nodekind & leafmask)) {
	    if (isPartOfLoadedModules(childNode)) {
		if (ignoreconformance && (childNode->nodekind & confmask)) {
		    return 1;
		}
		return 0;
	    }
	}
 
	if (! pruneSubTree(childNode)) {
	    return 0;
	}
    }

    return 1;
}



static void fprintSubTree(FILE *f, SmiNode *smiNode,
			  char *prefix, size_t typefieldlen)
{
    SmiNode     *childNode, *indexNode;
    SmiType     *type;
    int         i = 0, cnt, prefixlen;
    size_t      newtypefieldlen = 9;
    char        c = 0;
    char	*type_name;
    char        *label;

    if (smiNode) {
	prefixlen = strlen(prefix);
	switch (smiNode->nodekind) {
	case SMI_NODEKIND_SCALAR:
	case SMI_NODEKIND_COLUMN:
	    if (prefixlen > 0) {
	        c = prefix[prefixlen-1];
		prefix[prefixlen-1] = getSmiStatusChar(smiNode->status);
	    }
	    type_name = getSmiTypeName(smiNode);
	    if (type_name) {
		label = getSmiLabel(smiNode);
		fprintf(f, "%s-%s %-*s %s\n",
			prefix,
			getSmiFlags(smiNode),
			(int) typefieldlen,
			label,
			type_name);
		smiFree(type_name);
		smiFree(label);
	    }
	    if (prefixlen > 0 && c) {
		prefix[prefixlen-1] = c;
	    }
	    break;
	case SMI_NODEKIND_ROW:
	    if (prefixlen > 0) {
		c = prefix[prefixlen-1];
		prefix[prefixlen-1] = getSmiStatusChar(smiNode->status);
	    }
	    label = getSmiLabel(smiNode);
	    fprintf(f, "%s---- %s [", prefix, label);
	    smiFree(label);
	    switch (smiNode->indexkind) {
	    case SMI_INDEX_INDEX:
	    case SMI_INDEX_REORDER:
		fprintIndex(f, smiNode);
		break;
	    case SMI_INDEX_EXPAND:  /* TODO: we have to do more work here! */
		break;
	    case SMI_INDEX_AUGMENT:
	    case SMI_INDEX_SPARSE:
		indexNode = smiGetRelatedNode(smiNode);
		if (indexNode) {
		    fprintIndex(f, indexNode);
		}
		break;
	    case SMI_INDEX_UNKNOWN:
		break;	    
	    }
	    fprintf(f, "]\n");
	    if (prefixlen > 0 && c) {
		prefix[prefixlen-1] = c;
	    }
	    break;
	case SMI_NODEKIND_NOTIFICATION:
	    if (prefixlen > 0) {
		c = prefix[prefixlen-1];
		prefix[prefixlen-1] = getSmiStatusChar(smiNode->status);
	    }
	    label = getSmiLabel(smiNode);
	    fprintf(f, "%s---- %s {", prefix, label);
	    smiFree(label);
	    fprintObjects(f, smiNode);
	    fprintf(f, "}\n");
	    if (prefixlen > 0 && c) {
		prefix[prefixlen-1] = c;
	    }
	    break;
	default:
	    if (prefixlen > 0) {
		c = prefix[prefixlen-1];
		prefix[prefixlen-1] = getSmiStatusChar(smiNode->status);
	    }
	    if (smiNode->oid) {
		if (prefixlen > 0) {
		    label = getSmiLabel(smiNode);
		    fprintf(f, "%s---- %s\n", prefix, label);
		    smiFree(label);
		} else {
		    unsigned int j;
		    fprintf(f, "%s---- %s", prefix,
			    smiNode->name ? smiNode->name : " ");
		    if (! noids) {
			fprintf(f, "(");
			for (j = 0; j < smiNode->oidlen; j++) {
			    fprintf(f, "%s%u", j ? "." : "", smiNode->oid[j]);
			}
			fprintf(f, ")");
		    }
		    fprintf(f, "\n");
		}
	    } else {
		label = getSmiLabel(smiNode);
		fprintf(f, "%s-- %s\n", prefix, label);
		smiFree(label);
	    }
	    if (prefixlen > 0 && c) {
		prefix[prefixlen-1] = c;
	    }
	}
	for (childNode = smiGetFirstChildNode(smiNode), cnt = 0;
	     childNode;
	     childNode = smiGetNextChildNode(childNode)) {
	    if (! pruneSubTree(childNode)) {
		int len = 0;
		label = getSmiLabel(childNode);
		len = strlen(label);
		smiFree(label);
		if (len > newtypefieldlen) {
		    newtypefieldlen = len;
		}
		cnt++;
	    }
	}
	for (childNode = smiGetFirstChildNode(smiNode);
	     childNode;
	     childNode = smiGetNextChildNode(childNode)) {
	    char *newprefix;
	    if (pruneSubTree(childNode)) {
		continue;
	    }
	    i++;
	    newprefix = smiMalloc(strlen(prefix)+10);
	    strcpy(newprefix, prefix);
	    if (cnt == 1 || cnt == i) {
		strcat(newprefix, "   ");
	    } else {
		strcat(newprefix, "  |");
	    }
	    fprintSubTree(f, childNode, newprefix, newtypefieldlen);
	    smiFree(newprefix);
	}
    }
}



static void fprintSmiTree(FILE *f)
{
    SmiNode *smiNode;
    SmiNode *childNode;
    SmiNode *nextNode;
    int cnt;
    
    smiNode = smiGetNode(NULL, "iso");

    if (! full) {
	do {
	    for (childNode = smiGetFirstChildNode(smiNode), cnt = 0, nextNode = NULL;
		 childNode;
		 childNode = smiGetNextChildNode(childNode)) {
		if (! pruneSubTree(childNode)) {
		    cnt++;
		    if (! nextNode) {
			nextNode = childNode;
		    }
		}
	    }
	    if (cnt == 1) {
		smiNode = nextNode;
	    }
	} while (cnt == 1);
    }
    
    if (smiNode) {
	fprintSubTree(f, smiNode, "", 0);
    }
}



static void dumpOidTree(FILE *f, int modc, SmiModule **modv, int flags)
{
    int i;
    
    if (flags & SMIDUMP_FLAG_UNITE) {
	
	pmodc = modc;
	pmodv = modv;
	
	if (! (flags & SMIDUMP_FLAG_SILENT)) {
	    fprintf(f, "# united registration tree (generated by smidump "
		    SMI_VERSION_STRING ")\n\n");
	}
	if (! (flags & SMIDUMP_FLAG_SILENT) && (flags & SMIDUMP_FLAG_ERROR)) {
	    fprintf(f, "# WARNING: this output may be incorrect due to "
		    "significant parse errors\n\n");
	}
	fprintSmiTree(f);
	
    } else {

	for (i = 0; i < modc; i++) {

	    pmodc = 1;
	    pmodv = &(modv[i]);
	
	    if (! (flags & SMIDUMP_FLAG_SILENT)) {
		fprintf(f, "# %s registration tree (generated by smidump "
			SMI_VERSION_STRING ")\n\n", modv[i]->name);
	    }
	    if (! (flags & SMIDUMP_FLAG_SILENT) && (flags & SMIDUMP_FLAG_ERROR)) {
		fprintf(f, "# WARNING: this output may be incorrect due to "
			"significant parse errors\n\n");
	    }
	    fprintSmiTree(f);
	}
    }
}


#ifdef BACKEND_YANG
static void fprintYangTree(FILE *f, YangNode *node, char *prefix, int flags)
{
    int i = 0, cnt = 0, c = 0;
    YangNode *childNode;
    size_t prefixlen = 0;
    char *type_name;

    for (childNode = yangGetFirstChildNode(node), cnt = 0;
	 childNode; childNode = yangGetNextChildNode(childNode)) {
	switch (childNode->nodeKind) {
	case YANG_DECL_CONTAINER:
	case YANG_DECL_LIST:
	case YANG_DECL_LEAF_LIST:
	case YANG_DECL_LEAF:
	    cnt++;
	    break;
	default:
	    break;
	}
    }

    if (prefix) {
	prefixlen = strlen(prefix);
    }

    for (childNode = yangGetFirstChildNode(node);
	 childNode; childNode = yangGetNextChildNode(childNode)) {
	char *newprefix;

	switch (childNode->nodeKind) {
	case YANG_DECL_CONTAINER:
	case YANG_DECL_LIST:
	case YANG_DECL_LEAF_LIST:
	case YANG_DECL_LEAF:
	    break;
	default:
	    continue;
	}
#ifdef DEBUG
	fprintf(f, "** processing child %s **\n", childNode->value);
#endif

	if (prefixlen > 0) {
	    c = prefix[prefixlen-1];
	    prefix[prefixlen-1] = getYangStatusChar(childNode->status);
	}

	switch (childNode->nodeKind) {
	case YANG_DECL_CONTAINER:
	case YANG_DECL_LIST:
	    fprintf(f, "%s--%s %s (%s)\n",
		    prefix,
		    getYangFlags(childNode),
		    childNode->value,
		    yangDeclAsString(childNode->nodeKind));
	    break;
	case YANG_DECL_LEAF_LIST:
	case YANG_DECL_LEAF:
	    type_name = getYangTypeName(childNode);
	    fprintf(f, "%s--%s %s %s (%s)\n",
		    prefix,
		    getYangFlags(childNode),
		    childNode->value,
		    type_name ? type_name : "",
		    yangDeclAsString(childNode->nodeKind));
	    smiFree(type_name);
	    break;
	default:
	    break;
	}
	
	i++;
	if (prefixlen > 0 && c) {
	    prefix[prefixlen-1] = c;
	    c = 0;
	}
	newprefix = smiMalloc(strlen(prefix)+10);
	strcpy(newprefix, prefix);
	if (cnt == 1 || cnt == i) {
	    strcat(newprefix, "   ");
	} else {
	    strcat(newprefix, "  |");
	}
	fprintYangTree(f, childNode, newprefix, flags);
#ifdef DEBUG
	fprintf(f, "** iterating on %s **\n", childNode->value);
#endif
	smiFree(newprefix);
    }
}



static void dumpYangTree(FILE *f, int modc, SmiModule **modv, int flags)
{
    int i;
    
    if (flags & SMIDUMP_FLAG_UNITE) {
	fprintf(stderr, "smidump: tree format does not support united YANG output:"
		" ignoring -u\n");
	flags = (flags & ~SMIDUMP_FLAG_UNITE);
    }

    for (i = 0; i < modc; i++) {

	if (! (flags & SMIDUMP_FLAG_SILENT)) {
	    fprintf(f, "# %s YANG naming tree (generated by smidump "
		    SMI_VERSION_STRING ")\n\n", modv[i]->name);
	}
	if (! (flags & SMIDUMP_FLAG_SILENT) && (flags & SMIDUMP_FLAG_ERROR)) {
	    fprintf(f, "# WARNING: this output may be incorrect due to "
		    "significant parse errors\n\n");
	}

	fprintYangTree(f, yangGetModule(modv[i]->name), "", flags);
    }
}
#endif


static void dumpTree(int modc, SmiModule **modv, int flags, char *output)
{
    int i, cntSmi = 0;
#ifdef BACKEND_YANG
    int cntYang = 0;
#endif
    FILE *f = stdout;
    
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
	if (modv[i]->language == SMI_LANGUAGE_YANG) cntYang++;
#endif
	if (modv[i]->language == SMI_LANGUAGE_SMIV1
	    || modv[i]->language == SMI_LANGUAGE_SMIV2) cntSmi++;
    }
    if (cntSmi) {
	dumpOidTree(f, modc, modv, flags);
    }
#ifdef BACKEND_YANG
    if (cntYang) {
	dumpYangTree(f, modc, modv, flags);
    }
#endif

    if (fflush(f) || ferror(f)) {
	perror("smidump: write error");
	exit(1);
    }

    if (output) {
	fclose(f);
    }
}



void initTree()
{
    static SmidumpDriverOption opt[] = {
	{ "no-conformance", OPT_FLAG, &ignoreconformance, 0,
	  "do not show conformance nodes"},
	{ "no-leafs", OPT_FLAG, &ignoreleafs, 0,
	  "do not show leaf nodes"},
	{ "full-root", OPT_FLAG, &full, 0,
	  "generate the full path to the root"},
	{ "no-ids", OPT_FLAG, &noids, 0,
	  "do not show (sub-)identifiers"},
        { 0, OPT_END, 0, 0 }
    };
    
    static SmidumpDriver driver = {
	"tree",
	dumpTree,
	SMI_FLAG_NODESCR,
	0,
	"structure of the naming tree",
	opt,
	NULL
    };
    
    smidumpRegisterDriver(&driver);
}
