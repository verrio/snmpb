/*
 * dump-tree.c --
 *
 *      Operations to dump the OID tree in a human readable format.
 *
 * Copyright (c) 1999 Frank Strauss, Technical University of Braunschweig.
 * Copyright (c) 1999 J. Schoenwaelder, Technical University of Braunschweig.
 * Copyright (c) 2002 J. Schoenwaelder, University of Osnabrueck.
 *
 * See the file "COPYING" for information on usage and redistribution
 * of this file, and for a DISCLAIMER OF ALL WARRANTIES.
 *
 * @(#) $Id: dump-tree.c 8090 2008-04-18 12:56:29Z strauss $
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
static int compact = 0;

static char *getFlags(SmiNode *smiNode)
{

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
	    fprintf(f, indexname);
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
	    fprintf(f, objectname);
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
	 * interact with the semantics of ignoreconformance since we
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
    SmiNodekind lastNodeKind = SMI_NODEKIND_UNKNOWN;
    SmiType     *type;
    int         i = 0, cnt, prefixlen;
    size_t      newtypefieldlen = 9;
    char        c = 0;
    char	*type_name;

    if (smiNode) {
	prefixlen = strlen(prefix);
	switch (smiNode->nodekind) {
	case SMI_NODEKIND_SCALAR:
	case SMI_NODEKIND_COLUMN:
	    if (prefixlen > 0) {
	        c = prefix[prefixlen-1];
		prefix[prefixlen-1] = getStatusChar(smiNode->status);
	    }
	    type_name = getTypeName(smiNode);
	    if (type_name) {
		fprintf(f, "%s-- %s %-*s %s(%u)\n",
			prefix,
			getFlags(smiNode),
			typefieldlen,
			type_name,
			smiNode->name,
			smiNode->oid[smiNode->oidlen-1]);
		xfree(type_name);
	    }
	    if (prefixlen > 0 && c) {
		prefix[prefixlen-1] = c;
	    }
	    break;
	case SMI_NODEKIND_ROW:
	    if (prefixlen > 0) {
		c = prefix[prefixlen-1];
		prefix[prefixlen-1] = getStatusChar(smiNode->status);
	    }
	    fprintf(f, "%s--%s(%u) [", prefix,
		    smiNode->name,
		    smiNode->oid[smiNode->oidlen-1]);
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
		prefix[prefixlen-1] = getStatusChar(smiNode->status);
	    }
	    fprintf(f, "%s--%s(%u) [", prefix,
		    smiNode->name,
		    smiNode->oid[smiNode->oidlen-1]);
	    fprintObjects(f, smiNode);
	    fprintf(f, "]\n");
	    if (prefixlen > 0 && c) {
		prefix[prefixlen-1] = c;
	    }
	    break;
	default:
	    if (prefixlen > 0) {
		c = prefix[prefixlen-1];
		prefix[prefixlen-1] = getStatusChar(smiNode->status);
	    }
	    if (smiNode->oid)
		if (prefixlen > 0) {
		    fprintf(f, "%s--%s(%u)\n", prefix,
			    smiNode->name ? smiNode->name : " ",
			    smiNode->oid[smiNode->oidlen-1]);
		} else {
		    unsigned int j;
		    fprintf(f, "%s--%s(", prefix,
			    smiNode->name ? smiNode->name : " ");
		    for (j = 0; j < smiNode->oidlen; j++) {
			fprintf(f, "%s%u", j ? "." : "", smiNode->oid[j]);
		    }
		    fprintf(f, ")\n");
		}
	    else
		fprintf(f, "%s--%s(?)\n", prefix,
			smiNode->name ? smiNode->name : " ");
	    if (prefixlen > 0 && c) {
		prefix[prefixlen-1] = c;
	    }
	}
	for (childNode = smiGetFirstChildNode(smiNode), cnt = 0;
	     childNode;
	     childNode = smiGetNextChildNode(childNode)) {
	    if (! pruneSubTree(childNode)) {
		type = smiGetNodeType(childNode);
		if (type) {
		    type_name = getTypeName(childNode);
		    if (type_name) {
			if (strlen(type_name) > newtypefieldlen) {
			    newtypefieldlen = strlen(type_name);
			}
			xfree(type_name);
		    }
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
	    if (! compact &&
		((childNode->nodekind != SMI_NODEKIND_COLUMN
		  && childNode->nodekind != SMI_NODEKIND_SCALAR)
		 || (lastNodeKind != childNode->nodekind))) {
		fprintf(f, "%s  |\n", prefix);
	    }
	    newprefix = xmalloc(strlen(prefix)+10);
	    strcpy(newprefix, prefix);
	    if (cnt == 1 || cnt == i) {
		strcat(newprefix, "   ");
	    } else {
		strcat(newprefix, "  |");
	    }
	    fprintSubTree(f, childNode, newprefix, newtypefieldlen);
	    xfree(newprefix);
	    lastNodeKind = childNode->nodekind;
	}
    }
}



static void fprintTree(FILE *f)
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



static void dumpTree(int modc, SmiModule **modv, int flags, char *output)
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
	fprintTree(f);
	
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
	    fprintTree(f);
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



void initTree()
{
    static SmidumpDriverOption opt[] = {
	{ "no-conformance", OPT_FLAG, &ignoreconformance, 0,
	  "do not show conformance nodes"},
	{ "no-leafs", OPT_FLAG, &ignoreleafs, 0,
	  "do not show leaf nodes"},
	{ "full-root", OPT_FLAG, &full, 0,
	  "generate the full path to the root"},
	{ "compact", OPT_FLAG, &compact, 0,
	  "generate a more compact representation"},
        { 0, OPT_END, 0, 0 }
    };
    
    static SmidumpDriver driver = {
	"tree",
	dumpTree,
	SMI_FLAG_NODESCR,
	0,
	"structure of the OID tree",
	opt,
	NULL
    };
    
    smidumpRegisterDriver(&driver);
}
