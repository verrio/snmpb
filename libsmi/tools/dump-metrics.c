/*
 * dump-metrics.c --
 *
 *      Operations to compute and dump some MIB metrics.
 *
 * Copyright (c) 2000 Frank Strauss, Technical University of Braunschweig.
 * Copyright (c) 2000 J. Schoenwaelder, Technical University of Braunschweig.
 * Copyright (c) 2002 J. Schoenwaelder, University of Osnabrueck.
 * Copyright (c) 2004 J. Schoenwaelder, International University Bremen.
 *
 * See the file "COPYING" for information on usage and redistribution
 * of this file, and for a DISCLAIMER OF ALL WARRANTIES.
 *
 * @(#) $Id: dump-metrics.c 8090 2008-04-18 12:56:29Z strauss $
 */

/*
  # revisions
  # imports
  # row creations:
  # count node references in notification definitions
 */

#include <config.h>

#include <stdio.h>
#include <string.h>

#include "smi.h"
#include "smidump.h"


static int raw = 0;

static int silent = 0;


typedef struct BasetypeCounter {
    unsigned long total;
    unsigned long unknown;
    unsigned long integer32;
    unsigned long octetstring;
    unsigned long objectidentifier;
    unsigned long unsigned32;
    unsigned long integer64;
    unsigned long unsigned64;
    unsigned long float32;
    unsigned long float64;
    unsigned long float128;
    unsigned long enums;
    unsigned long bits;
    unsigned long pointer;
} BasetypeCounter;


typedef struct StatusCounter {
    unsigned long total;
    unsigned long current;
    unsigned long deprecated;
    unsigned long obsolete;
} StatusCounter;



typedef struct AccessCounter {
    unsigned long total;
    unsigned long noaccess;
    unsigned long notify;
    unsigned long readonly;
    unsigned long readwrite;
} AccessCounter;



typedef struct IndexCounter {
    unsigned long total;
    unsigned long index;
    unsigned long augment;
    unsigned long reorder;
    unsigned long sparse;
    unsigned long expand;
} IndexCounter;



typedef struct IndexLenCounter {
    unsigned long total;
    unsigned long length[11];
} IndexLenCounter;



typedef struct TableLenCounter {
    unsigned long total;
    unsigned long length[81];
} TableLenCounter;



typedef struct ScalarLenCounter {
    unsigned long total;
    unsigned long length[81];
} ScalarLenCounter;



typedef struct IndexComplexityCounter {
    unsigned long total;
    unsigned long complexity[100];
} IndexComplexityCounter;



typedef struct LengthCounter {
    unsigned long total;
    unsigned long descr;
    unsigned long descr_len;
    unsigned long reference;
    unsigned long reference_len;
    unsigned long units;
    unsigned long units_len;
    unsigned long format;
    unsigned long format_len;
} LengthCounter;



typedef struct RowStatusCounter {
    unsigned long basetables;
    unsigned long rowstatus;
    unsigned long storagetype;
} RowStatusCounter;



typedef struct Metrics {
    BasetypeCounter basetypesColumns;
    BasetypeCounter basetypesScalars;
    BasetypeCounter basetypesAll;
    StatusCounter   statusTypes;
    StatusCounter   statusTables;
    StatusCounter   statusColumns;
    StatusCounter   statusScalars;
    StatusCounter   statusNotifications;
    StatusCounter   statusGroups;
    StatusCounter   statusCompliances;
    StatusCounter   statusAll;
    AccessCounter   accessColumns;
    AccessCounter   accessScalars;
    AccessCounter   accessAll;
    IndexCounter    indexTables;
    IndexLenCounter indexLenTables;
    IndexComplexityCounter indexComplexity;
    TableLenCounter tableLength;
    ScalarLenCounter scalarLength;
    LengthCounter   lengthTypes;
    LengthCounter   lengthTables;
    LengthCounter   lengthRows;
    LengthCounter   lengthColumns;
    LengthCounter   lengthScalars;
    LengthCounter   lengthNotifications;
    LengthCounter   lengthAll;
} Metrics;



typedef struct UsageCounter {
    char     *module;
    char     *name;
    unsigned count;
    struct UsageCounter *nextPtr;
} UsageCounter;


static UsageCounter *typeList = NULL;
static UsageCounter *extTypeList = NULL;
static UsageCounter *extNodeList = NULL;
static UsageCounter *extModuleList = NULL;
static UsageCounter *indexComplexityList = NULL;

#define INCR_NODE 0x01
#define INCR_TYPE 0x02


static char*
getDateString(time_t t)
{
    static char   *s = NULL;
    struct tm	  *tm;

    if (s) xfree(s);
    
    tm = gmtime(&t);
    smiAsprintf(&s, "%04d-%02d-%02d",
		tm->tm_year + 1900, tm->tm_mon + 1, tm->tm_mday);
    return s;
}



static char*
language(SmiLanguage language)
{
    return
	(language == SMI_LANGUAGE_UNKNOWN)    ? "-" :
	(language == SMI_LANGUAGE_SMIV1)      ? "SMIv1" :
	(language == SMI_LANGUAGE_SMIV2)      ? "SMIv2" :
	(language == SMI_LANGUAGE_SMING)      ? "SMIng" :
						"-";
}



static int
calcSize(SmiModule *smiModule)
{
    SmiNode *smiNode;
    SmiType *smiType;
    int size = 0;
    
    for (smiType = smiGetFirstType(smiModule);
	 smiType;
	 smiType = smiGetNextType(smiType)) {
	if (smiType->name) {
	    size++;
	}
    }
    
    for (smiNode = smiGetFirstNode(smiModule, SMI_NODEKIND_ANY);
	 smiNode;
	 smiNode = smiGetNextNode(smiNode, SMI_NODEKIND_ANY)) {

	switch (smiNode->nodekind) {
	case SMI_NODEKIND_SCALAR:
	case SMI_NODEKIND_COLUMN:
	case SMI_NODEKIND_NOTIFICATION:
	    size++;
	    break;
	default:
	    break;
	}
    }

    return size;
}



typedef void	(*ForEachIndexFunc)	(FILE *f, SmiNode *groupNode, SmiNode *smiNode, void *data);

static void
foreachIndexDo(FILE *f, SmiNode *smiNode, ForEachIndexFunc func, void *data)
{
    SmiNode *indexNode = NULL, *iNode;
    SmiElement *smiElement;
    
    switch (smiNode->indexkind) {
    case SMI_INDEX_INDEX:
    case SMI_INDEX_REORDER:
	indexNode = smiNode;
	break;
    case SMI_INDEX_EXPAND:	/* TODO: we have to do more work here! */
	break;
    case SMI_INDEX_AUGMENT:
    case SMI_INDEX_SPARSE:
	indexNode = smiGetRelatedNode(smiNode);
	break;
    case SMI_INDEX_UNKNOWN:
	break;
    }
    if (indexNode) {
	for (smiElement = smiGetFirstElement(indexNode);
	     smiElement; smiElement = smiGetNextElement(smiElement)) {
	    iNode = smiGetElementNode(smiElement);
	    if (iNode) {
		(func) (f, smiNode, iNode, data);
	    }
	}
    }
}



static UsageCounter*
incrUsageCounter(UsageCounter *uCntList, char *module, char *name, int incr)
{
    UsageCounter *uCnt;

    for (uCnt = uCntList; uCnt; uCnt = uCnt->nextPtr) {
	if (strcmp(uCnt->module, module) == 0
	    && (! name || strcmp(uCnt->name, name) == 0)) {
	    break;
	}
    }

    if (! uCnt) {
	uCnt = (UsageCounter *) xmalloc(sizeof(UsageCounter));
	uCnt->module = xstrdup(module);
	uCnt->name = name ? xstrdup(name) : NULL;
	uCnt->count = 0;
	uCnt->nextPtr = uCntList;
	uCntList = uCnt;
    }

    uCnt->count += incr;
    return uCntList;
}



static void
incrTypeAndNodeUsageCounter(SmiModule *smiModule, SmiNode *smiNode, int flags)
{
    SmiType *smiType;
    char *extModule;

    /*
     * First check whether the node is external. If yes, increment the
     * external node counter and we are done.
     */

    extModule = smiGetNodeModule(smiNode)->name;
    if (extModule
	&& strcmp(extModule, smiModule->name) != 0) {
	if (flags & INCR_NODE) {
	    extNodeList = incrUsageCounter(extNodeList,
					   extModule, smiNode->name, 1);
	    extModuleList = incrUsageCounter(extModuleList, extModule, NULL, 1);
	}
	return;
    }
    
    /*
     * Next, check whether the type of the node is external. If yes,
     * increment the external type counter and we are done. Do not
     * count base types (that is types that have no parent type).
     */

    smiType = smiGetNodeType(smiNode);
    if (! smiType) {
	return;
    }

    if (smiType->name && smiGetParentType(smiType)) {
	char *extModule = smiGetTypeModule(smiType)->name;
	if (extModule /* && *extModule */
	    && strcmp(extModule, smiModule->name) != 0) {
	    if (flags & INCR_TYPE) {
		extTypeList = incrUsageCounter(extTypeList,
					       extModule, smiType->name, 1);
		extModuleList = incrUsageCounter(extModuleList, extModule, NULL, 1);
	    }
	}
    }

    /*
     * Finally, count the type name (whether external or not does not
     * matter here nor does it matter whether it is a base type or
     * not).
     */

    if (! smiType->name && smiGetParentType(smiType)) {
	smiType = smiGetParentType(smiType);
    }
    typeList = incrUsageCounter(typeList, smiGetTypeModule(smiType)->name,
				smiType->name, 1);
}



static void
incrIndexComplexityCounter(SmiModule *smiModule, SmiNode *smiNode, int complexity)
{
    indexComplexityList = incrUsageCounter(indexComplexityList,
					   smiModule->name, smiNode->name, complexity);
}



static int
cmp(const void *va, const void *vb)
{
    UsageCounter **a = (UsageCounter **) va;
    UsageCounter **b = (UsageCounter **) vb;

    if ((*a)->count > (*b)->count) return -1;
    if ((*a)->count < (*b)->count) return 1;

    if ((*a)->module && (*b)->module) {
	int x = strcmp((*a)->module,  (*b)->module);
	if (x) return x;
    }

    if ((*a)->name && (*b)->name) {
	int x = strcmp((*a)->name,  (*b)->name);
	if (x) return x;
    }

    return 0;
}



static int
fprintRevisions(FILE *f, int modLen, SmiRevision *smiRevision,
		SmiModule *smiModule, int size)
{
    int n = 0;
    
    if (smiRevision) {
	n = fprintRevisions(f, modLen,
			    smiGetNextRevision(smiRevision), smiModule, -1);
	fprintf(f, "%-*s %7s  ", modLen, smiModule->name,
		(size >= 0) ? language(smiModule->language) : "-");
 	if (size >= 0) {
	    fprintf(f, "%4d", size);
	} else {
	    fprintf(f, "   -");
	}
	fprintf(f, "   %3d    %s\n", n, getDateString(smiRevision->date));
	n++;
    }

    if (!smiRevision && size >= 0) {
	fprintf(f, "%-*s %7s  ", modLen, smiModule->name,
		language(smiModule->language));
	fprintf(f, "%4d", size);
	fprintf(f, "     -    ----------\n");
    }
    
    return n;
}



static void
fprintRevision(FILE *f, int modc, SmiModule **modv)
{
    int i;
    int modLen = 8;

    for (i = 0; i < modc; i++) {
	if (modLen < strlen(modv[i]->name)) {
	    modLen = strlen(modv[i]->name);
	}
    }

    fprintf(f, "%-*s LANGUAGE SIZE REVISION DATE\n", modLen, "MODULE");

    for (i = 0; i < modc; i++) {
	fprintRevisions(f, modLen, smiGetFirstRevision(modv[i]),
			modv[i], calcSize(modv[i]));
    }
    fprintf(f, "\n");
 
}



static void
fprintTypeUsage(FILE *f, UsageCounter *typeUsageList)
{
    UsageCounter *uCnt;
    int modLen = 8, nameLen = 8;
    unsigned total = 0;
    int i, cnt = 0;
    UsageCounter **sortCnt;

    /* should be sorted */

    for (uCnt = typeUsageList, cnt = 0; uCnt; uCnt = uCnt->nextPtr, cnt++) {
	if (modLen < strlen(uCnt->module)) {
	    modLen = strlen(uCnt->module);
	}
	if (nameLen < strlen(uCnt->name)) {
	    nameLen = strlen(uCnt->name);
	}
	total += uCnt->count;
    }

    if (cnt == 0) {
	return;
    }

    /* create an array for a quick qsort */

    sortCnt = (UsageCounter **) xmalloc(cnt * sizeof(UsageCounter *));
    memset(sortCnt, 0, cnt * sizeof(UsageCounter *));
    for (uCnt = typeUsageList, i = 0; uCnt; uCnt = uCnt->nextPtr, i++) {
	sortCnt[i] = uCnt;
    }
    qsort(sortCnt, cnt, sizeof(UsageCounter *), cmp);
    
    if (! silent) {
	fputs(
"# The following table shows the distribution of the number of references\n"
"# to defined types (including base types) in the set of loaded MIB\n"
"# modules.\n"
"\n", f);
    }
    fprintf(f, "%-*s %-*s   USAGE\n", modLen, "MODULE", nameLen, "TYPE");

    for (i = 0; i < cnt; i++) {
	fprintf(f, "%-*s %-*s ",
		modLen, sortCnt[i]->module, nameLen, sortCnt[i]->name);
	if (raw) {
	    fprintf(f, "%8u\n", sortCnt[i]->count);
	} else {
	    fprintf(f, "%6.1f%%\n", (double) sortCnt[i]->count * 100 / total);
	}
    }

    xfree(sortCnt);
}



static void
fprintExtTypeUsage(FILE *f, UsageCounter *typeUsageList)
{
    UsageCounter *uCnt;
    int modLen = 8, nameLen = 8;
    unsigned total = 0;
    int i, cnt = 0;
    UsageCounter **sortCnt;

    /* should be sorted */

    for (uCnt = typeUsageList, cnt = 0; uCnt; uCnt = uCnt->nextPtr, cnt++) {
	if (modLen < strlen(uCnt->module)) {
	    modLen = strlen(uCnt->module);
	}
	if (nameLen < strlen(uCnt->name)) {
	    nameLen = strlen(uCnt->name);
	}
	total += uCnt->count;
    }

    if (cnt == 0) {
	return;
    }

    /* create an array for a quick qsort */

    sortCnt = (UsageCounter **) xmalloc(cnt * sizeof(UsageCounter *));
    memset(sortCnt, 0, cnt * sizeof(UsageCounter *));
    for (uCnt = typeUsageList, i = 0; uCnt; uCnt = uCnt->nextPtr, i++) {
	sortCnt[i] = uCnt;
    }
    qsort(sortCnt, cnt, sizeof(UsageCounter *), cmp);
    
    if (! silent) {
	fputs(
"# The following table shows the distribution of the number of references\n"
"# to externally defined types (excluding base types) in the set of loaded\n"
"# MIB modules.\n"
"\n", f);
    }
    fprintf(f, "%-*s %-*s EXT-USAGE\n", modLen, "MODULE", nameLen, "TYPE");

    for (i = 0; i < cnt; i++) {
	fprintf(f, "%-*s %-*s ",
		modLen, sortCnt[i]->module, nameLen, sortCnt[i]->name);
	if (raw) {
	    fprintf(f, "%8u\n", sortCnt[i]->count);
	} else {
	    fprintf(f, "%6.1f%%\n", (double) sortCnt[i]->count * 100 / total);
	}
    }

    xfree(sortCnt);
}



static void
fprintExtNodeUsage(FILE *f, UsageCounter *typeUsageList)
{
    UsageCounter *uCnt;
    int modLen = 8, nameLen = 8;
    unsigned total = 0;
    int i, cnt = 0;
    UsageCounter **sortCnt;

    /* should be sorted */

    for (uCnt = typeUsageList, cnt = 0; uCnt; uCnt = uCnt->nextPtr, cnt++) {
	if (modLen < strlen(uCnt->module)) {
	    modLen = strlen(uCnt->module);
	}
	if (nameLen < strlen(uCnt->name)) {
	    nameLen = strlen(uCnt->name);
	}
	total += uCnt->count;
    }

    if (cnt == 0) {
	return;
    }

    /* create an array for a quick qsort */

    sortCnt = (UsageCounter **) xmalloc(cnt * sizeof(UsageCounter *));
    memset(sortCnt, 0, cnt * sizeof(UsageCounter *));
    for (uCnt = typeUsageList, i = 0; uCnt; uCnt = uCnt->nextPtr, i++) {
	sortCnt[i] = uCnt;
    }
    qsort(sortCnt, cnt, sizeof(UsageCounter *), cmp);
    
    if (! silent) {
	fputs(
"# The following table shows the distribution of the number of references\n"
"# to externally defined nodes in the set of loaded MIB modules.\n"
"\n", f);
    }
    fprintf(f, "%-*s %-*s EXT-USAGE\n", modLen, "MODULE", nameLen, "NODE");

    for (i = 0; i < cnt; i++) {
	fprintf(f, "%-*s %-*s ",
		modLen, sortCnt[i]->module, nameLen, sortCnt[i]->name);
	if (raw) {
	    fprintf(f, "%8u\n", sortCnt[i]->count);
	} else {
	    fprintf(f, "%6.1f%%\n", (double) sortCnt[i]->count * 100 / total);
	}
    }

    xfree(sortCnt);
}



static void
fprintModuleUsage(FILE *f, UsageCounter *modUsageList)
{
    UsageCounter *uCnt;
    int modLen = 8;
    unsigned total = 0;
    int i, cnt = 0;
    UsageCounter **sortCnt;

    /* should be sorted */

    for (uCnt = modUsageList, cnt = 0; uCnt; uCnt = uCnt->nextPtr, cnt++) {
	if (modLen < strlen(uCnt->module)) {
	    modLen = strlen(uCnt->module);
	}
	total += uCnt->count;
    }

    if (cnt == 0) {
	return;
    }

    /* create an array for a quick qsort */

    sortCnt = (UsageCounter **) xmalloc(cnt * sizeof(UsageCounter *));
    memset(sortCnt, 0, cnt * sizeof(UsageCounter *));
    for (uCnt = modUsageList, i = 0; uCnt; uCnt = uCnt->nextPtr, i++) {
	sortCnt[i] = uCnt;
    }
    qsort(sortCnt, cnt, sizeof(UsageCounter *), cmp);

    if (! silent) {
	fputs(
"# The following table shows the distribution of the number of references\n"
"# to externally defined items (such as types or objects) accumulated by\n"
"# the defining MIB module in the set of loaded MIB modules.\n"
"\n", f);
    }
    fprintf(f, "%-*s EXT-USAGE\n", modLen, "MODULE");

    for (i = 0; i < cnt; i++) {
	fprintf(f, "%-*s ", modLen, sortCnt[i]->module);
	if (raw) {
	    fprintf(f, "%8u\n", sortCnt[i]->count);
	} else {
	    fprintf(f, "%6.1f%%\n", (double) sortCnt[i]->count * 100 / total);
	}
    }

    xfree(sortCnt);
}



static void
fprintIndexComplexity(FILE *f, UsageCounter *modUsageList)
{
    UsageCounter *uCnt;
    int modLen = 8;
    int nameLen = 8;
    unsigned total = 0;
    int i, cnt = 0;
    UsageCounter **sortCnt;

    /* should be sorted */

    for (uCnt = modUsageList, cnt = 0; uCnt; uCnt = uCnt->nextPtr, cnt++) {
	if (modLen < strlen(uCnt->module)) {
	    modLen = strlen(uCnt->module);
	}
	if (nameLen < strlen(uCnt->name)) {
	    nameLen = strlen(uCnt->name);
	}
	total += uCnt->count;
    }

    if (cnt == 0) {
	return;
    }

    /* create an array for a quick qsort */

    sortCnt = (UsageCounter **) xmalloc(cnt * sizeof(UsageCounter *));
    memset(sortCnt, 0, cnt * sizeof(UsageCounter *));
    for (uCnt = modUsageList, i = 0; uCnt; uCnt = uCnt->nextPtr, i++) {
	sortCnt[i] = uCnt;
    }
    qsort(sortCnt, cnt, sizeof(UsageCounter *), cmp);

    if (! silent) {
	fputs(
"# The following table shows the distribution of the index complexity\n"
"# in the set of loaded MIB modules.\n"
"\n", f);
    }
    fprintf(f, "%-*s %-*s COMPLEXITY\n", modLen, "MODULE", nameLen, "TABLE");

    for (i = 0; i < cnt; i++) {
	fprintf(f, "%-*s %-*s ", modLen, sortCnt[i]->module, nameLen, sortCnt[i]->name);
	if (raw) {
	    fprintf(f, "%8u\n", sortCnt[i]->count);
	} else {
	    fprintf(f, "%6.1f%%\n", (double) sortCnt[i]->count);
	}
    }

    xfree(sortCnt);
}



static void
freeUsageCounter(UsageCounter *usageCounterList)
{
    UsageCounter *uCnt, *p;
    
    for (uCnt = usageCounterList; uCnt; ) {
	p = uCnt, uCnt = uCnt->nextPtr;
	xfree(p->module);
	xfree(p->name);
	xfree(p);
    }
}



static void
incrBasetypeCounter(BasetypeCounter *basetypeCounter, SmiNode *smiNode)
{
    SmiType *smiType;

    smiType = smiGetNodeType(smiNode);
    if (smiType) {
	basetypeCounter->total++;
	switch (smiType->basetype) {
	case SMI_BASETYPE_UNKNOWN:
	    basetypeCounter->unknown++;
	    break;
	case SMI_BASETYPE_INTEGER32:
	    basetypeCounter->integer32++;
	    break;
	case SMI_BASETYPE_OCTETSTRING:
	    basetypeCounter->octetstring++;
	    break;
	case SMI_BASETYPE_OBJECTIDENTIFIER:
	    basetypeCounter->objectidentifier++;
	    break;
	case SMI_BASETYPE_UNSIGNED32:
	    basetypeCounter->unsigned32++;
	    break;
	case SMI_BASETYPE_INTEGER64:
	    basetypeCounter->integer64++;
	    break;
	case SMI_BASETYPE_UNSIGNED64:
	    basetypeCounter->unsigned64++;
	    break;
	case SMI_BASETYPE_FLOAT32:
	    basetypeCounter->float32++;
	    break;
	case SMI_BASETYPE_FLOAT64:
	    basetypeCounter->float64++;
	    break;
	case SMI_BASETYPE_FLOAT128:
	    basetypeCounter->float128++;
	    break;
	case SMI_BASETYPE_ENUM:
	    basetypeCounter->enums++;
	    break;
	case SMI_BASETYPE_BITS:
	    basetypeCounter->bits++;
	    break;
	case SMI_BASETYPE_POINTER:
	    basetypeCounter->pointer++;
	    break;
	}
    }
}



static void
incrStatusCounter(StatusCounter *cnt, SmiStatus smiStatus)
{
    cnt->total++;
    switch (smiStatus) {
    case SMI_STATUS_CURRENT:
    case SMI_STATUS_MANDATORY:
    case SMI_STATUS_OPTIONAL:
	cnt->current++;
	break;
    case SMI_STATUS_DEPRECATED:
	cnt->deprecated++;
	break;
    case SMI_STATUS_OBSOLETE:
	cnt->obsolete++;
	break;
    case SMI_STATUS_UNKNOWN:
	break;
    }
}



static void
incrAccessCounter(AccessCounter *cnt, SmiAccess smiAccess)
{
    cnt->total++;
    switch (smiAccess) {
    case SMI_ACCESS_NOT_ACCESSIBLE:
	cnt->noaccess++;
	break;
    case SMI_ACCESS_NOTIFY:
	cnt->notify++;
	break;
    case SMI_ACCESS_READ_ONLY:
	cnt->readonly++;
	break;
    case SMI_ACCESS_READ_WRITE:
	cnt->readwrite++;
	break;
    case SMI_ACCESS_INSTALL:
    case SMI_ACCESS_INSTALL_NOTIFY:
    case SMI_ACCESS_REPORT_ONLY:
    case SMI_ACCESS_UNKNOWN:
    case SMI_ACCESS_NOT_IMPLEMENTED:
    case SMI_ACCESS_EVENT_ONLY:
	break;
    }
}



static void
incrIndexCounter(IndexCounter *cnt, SmiIndexkind indexkind)
{
    cnt->total++;
    switch (indexkind) {
    case SMI_INDEX_INDEX:
	cnt->index++;
	break;
    case SMI_INDEX_AUGMENT:
	cnt->augment++;
	break;
    case SMI_INDEX_REORDER:
	cnt->reorder++;
	break;
    case SMI_INDEX_SPARSE:
	cnt->sparse++;
	break;
    case SMI_INDEX_EXPAND:
	cnt->expand++;
	break;
    case SMI_INDEX_UNKNOWN:
	break;
    }
}



static void
incrIndexLenCounter(IndexLenCounter *cnt, int len)
{
    cnt->total++;
    if (len < sizeof(cnt->length)/sizeof(cnt->length[0])) {
	cnt->length[len]++;
    } else {
	fprintf(stderr, "smidump: index len overflow: %d\n", len);
    }
}



static void
incrTableLenCounter(TableLenCounter *cnt, int len)
{
    cnt->total++;
    if (len < sizeof(cnt->length)/sizeof(cnt->length[0])) {
	cnt->length[len]++;
    } else {
	fprintf(stderr, "smidump: table len overflow: %d\n", len);
    }
}



static void
incrIndexComplexityMetric(IndexComplexityCounter *cnt, int cmplx)
{
    cnt->total++;
    if (cmplx < sizeof(cnt->complexity)/sizeof(cnt->complexity[0])) {
	cnt->complexity[cmplx]++;
    } else {
	fprintf(stderr, "smidump: index complexity overflow: %d\n", cmplx);
    }
}



static void
incrLengthCounter(LengthCounter *cnt, char *description, char *reference,
		  char *units, char *format)
{
    cnt->total++;
    if (description) {
	cnt->descr++;
	cnt->descr_len += strlen(description);
    }

    if (reference) {
	cnt->reference++;
	cnt->reference_len += strlen(reference);
    }
    if (units) {
	cnt->units++;
	cnt->units_len += strlen(units);
    }
    if (format) {
	cnt->format++;
	cnt->format_len += strlen(format);
    }
}



static void
incrRowStatusCounter(SmiNode *rowNode)
{
    SmiNode *smiNode;
    SmiType *smiType;
    SmiModule *smiModule;

    for (smiNode = smiGetFirstChildNode(rowNode);
	 smiNode;
	 smiNode = smiGetNextChildNode(smiNode)) {
	smiType = smiGetNodeType(smiNode);
	if (smiType && smiType->name) {
	    smiModule = smiGetTypeModule(smiType);
	    if (smiModule && smiModule->name
		&& strcmp(smiType->name, "RowStatus") == 0
		&& strcmp(smiModule->name, "SNMPv2-TC") == 0) {
		break;
	    }
	}
    }

    if (smiNode) {
#if 0
	fprintf(stderr, "** %s\t%s\t%s\n", rowNode->name,
		smiNode->name, smiType->name);
	/* xxx count rows indexed by ifIndex, InterfaceIndex, InterfaceIndexOrZero, ... */
#endif
    }
}



static void
count(FILE *f, SmiNode *row, SmiNode *col, void *data)
{
    int *cnt = (int *) data;

    (*cnt)++;
}



static void
complexity(FILE *f, SmiNode *row, SmiNode *col, void *data)
{
    int *cmplx = (int *) data;
    SmiType *smiType;
    unsigned long min, max;

    smiType = smiGetNodeType(col);
    if (! smiType) {
	return;
    }

    switch (smiType->basetype) {
    case SMI_BASETYPE_INTEGER32:
    case SMI_BASETYPE_UNSIGNED32:
    case SMI_BASETYPE_ENUM:
	*cmplx += 1;
	break;
    case SMI_BASETYPE_OCTETSTRING:
    case SMI_BASETYPE_OBJECTIDENTIFIER:
    case SMI_BASETYPE_BITS:
	*cmplx += 2;
	min = smiGetMinSize(smiType);
	max = smiGetMaxSize(smiType);
	if (min != max) {
	    *cmplx += 1;
	}
	break;
    default:				/* ignore everything else */
	break;
    }
}



static void
yadayada(FILE *f, SmiNode *row, SmiNode *col, void *data)
{
    SmiModule *smiModule = (SmiModule *) data;
    int flags = 0;

    if (col->access == SMI_ACCESS_NOT_ACCESSIBLE) {
	flags |= INCR_TYPE;
    }
    flags |= INCR_NODE;

    incrTypeAndNodeUsageCounter(smiModule, col, flags);
}



static void
addMetrics(Metrics *metrics, SmiModule *smiModule)
{
    SmiNode *smiNode;
    SmiType *smiType;
    size_t len;

    for (smiNode = smiGetFirstNode(smiModule, SMI_NODEKIND_ANY);
	 smiNode;
	 smiNode = smiGetNextNode(smiNode, SMI_NODEKIND_ANY)) {
	len = smiNode->description ? strlen(smiNode->description) : 0;
	switch (smiNode->nodekind) {
	case SMI_NODEKIND_TABLE:
	    incrStatusCounter(&metrics->statusTables, smiNode->status);
	    incrStatusCounter(&metrics->statusAll, smiNode->status);
	    incrLengthCounter(&metrics->lengthTables,
			      smiNode->description, smiNode->reference,
			      smiNode->units, smiNode->format);
	    incrLengthCounter(&metrics->lengthAll,
			      smiNode->description, smiNode->reference,
			      smiNode->units, smiNode->format);
	    break;
	case SMI_NODEKIND_ROW:
	    incrIndexCounter(&metrics->indexTables, smiNode->indexkind);
	    incrLengthCounter(&metrics->lengthRows,
			      smiNode->description, smiNode->reference,
			      smiNode->units, smiNode->format);
	    incrLengthCounter(&metrics->lengthAll,
			      smiNode->description, smiNode->reference,
			      smiNode->units, smiNode->format);
	    incrRowStatusCounter(smiNode);
	    {
		int cnt = 0;
		foreachIndexDo(NULL, smiNode, count, &cnt);
		incrIndexLenCounter(&metrics->indexLenTables, cnt);
		foreachIndexDo(NULL, smiNode, yadayada, smiModule);
	    }
	    {
		int cmplx = 0;
		foreachIndexDo(NULL, smiNode, complexity, &cmplx);
		incrIndexComplexityCounter(smiModule, smiNode, cmplx);
		incrIndexComplexityMetric(&metrics->indexComplexity, cmplx);
	    }
	    /* count the childs ... */
	    {
		    SmiModule *smiModule = smiGetModule("SNMPv2-TC");
		    SmiNode *childNode;
		    SmiType *rowStatus = smiGetType(smiModule, "RowStatus");
		    SmiType *storageType = smiGetType(smiModule, "StorageType");
		    /* include index elements not in table */
		    int n = 0;
		    for (childNode = smiGetFirstChildNode(smiNode);
			 childNode;
			 childNode = smiGetNextChildNode(childNode)) {
			    n++;
			    if (rowStatus == smiGetNodeType(childNode)) {
				    fprintf(stderr, "**** GEEEEEE - ROWSTATUS\n");
			    }
			    if (storageType == smiGetNodeType(childNode)) {
				    fprintf(stderr, "**** GEEEEEE - STORAGETYPE\n");
			    }
		    }
		    incrTableLenCounter(&metrics->tableLength, n);
	    }
	    break;
	case SMI_NODEKIND_COLUMN:
	    incrBasetypeCounter(&metrics->basetypesColumns, smiNode);
	    incrBasetypeCounter(&metrics->basetypesAll, smiNode);
	    incrStatusCounter(&metrics->statusColumns, smiNode->status);
	    incrStatusCounter(&metrics->statusAll, smiNode->status);
	    incrAccessCounter(&metrics->accessColumns, smiNode->access);
	    incrAccessCounter(&metrics->accessAll, smiNode->access);
	    incrLengthCounter(&metrics->lengthColumns,
			      smiNode->description, smiNode->reference,
			      smiNode->units, smiNode->format);
	    incrLengthCounter(&metrics->lengthAll,
			      smiNode->description, smiNode->reference,
			      smiNode->units, smiNode->format);
	    incrTypeAndNodeUsageCounter(smiModule, smiNode, INCR_TYPE);
	    break;
	case SMI_NODEKIND_SCALAR:
	    incrBasetypeCounter(&metrics->basetypesScalars, smiNode);
	    incrBasetypeCounter(&metrics->basetypesAll, smiNode);
	    incrStatusCounter(&metrics->statusScalars, smiNode->status);
	    incrStatusCounter(&metrics->statusAll, smiNode->status);
	    incrAccessCounter(&metrics->accessScalars, smiNode->access);
	    incrAccessCounter(&metrics->accessAll, smiNode->access);
	    incrLengthCounter(&metrics->lengthScalars,
			      smiNode->description, smiNode->reference,
			      smiNode->units, smiNode->format);
	    incrLengthCounter(&metrics->lengthAll,
			      smiNode->description, smiNode->reference,
			      smiNode->units, smiNode->format);
	    incrTypeAndNodeUsageCounter(smiModule, smiNode, INCR_TYPE);
	    break;
	case SMI_NODEKIND_NOTIFICATION:
	    incrStatusCounter(&metrics->statusNotifications, smiNode->status);
	    incrStatusCounter(&metrics->statusAll, smiNode->status);
	    incrLengthCounter(&metrics->lengthNotifications,
			      smiNode->description, smiNode->reference,
			      smiNode->units, smiNode->format);
	    incrLengthCounter(&metrics->lengthAll,
			      smiNode->description, smiNode->reference,
			      smiNode->units, smiNode->format);
	    break;
	case SMI_NODEKIND_GROUP:
	    incrStatusCounter(&metrics->statusGroups, smiNode->status);
	    incrStatusCounter(&metrics->statusAll, smiNode->status);
	    break;
	case SMI_NODEKIND_COMPLIANCE:
	    incrStatusCounter(&metrics->statusCompliances, smiNode->status);
	    incrStatusCounter(&metrics->statusAll, smiNode->status);
	    break;
	}
    }

    for (smiType = smiGetFirstType(smiModule);
	 smiType;
	 smiType = smiGetNextType(smiType)) {

	/*
	 * Ignore all types with empty descriptions coming from the
	 * "SNMPv2-SMI" module since they are not really defined
	 * types but part of the language itself.
	 */

	if (! smiType->description) {
	    SmiModule *m = smiGetTypeModule(smiType);
	    if (m && strcmp(m->name, "SNMPv2-SMI") == 0) {
		continue;
	    }
	}
    
	incrStatusCounter(&metrics->statusTypes, smiType->status);
	incrStatusCounter(&metrics->statusAll, smiType->status);
	incrLengthCounter(&metrics->lengthTypes,
			  smiType->description, smiType->reference,
			  smiType->units, smiType->format);
	incrLengthCounter(&metrics->lengthAll,
			  smiType->description, smiType->reference,
			  smiType->units, smiType->format);
    }
}



static void
fprintBasetypeCounter(FILE *f, BasetypeCounter *cnt, const char *s)
{
    if (!s && ! cnt) {
	if (! silent) {
	    fputs(
"# The following table shows the basetype usage distribution in the\n"
"# set of loaded MIB modules.\n"
"\n", f);
	}
	fprintf(f, "%-10s Int32 Uns32 Int64 Uns64 OctSt ObjId Enums  Bits Flo32 Flo64 Flo128\n",
		"CATEGORY");
	return;
    }

    if (raw) {
	fprintf(f, "%-10s %5lu %5lu %5lu %5lu %5lu %5lu %5lu %5lu %5lu %5lu %5lu\n", s,
		cnt->integer32, cnt->unsigned32,
		cnt->integer64, cnt->unsigned64,
		cnt->octetstring, cnt->objectidentifier,
		cnt->enums, cnt->bits,
		cnt->float32, cnt->float64, cnt->float128);
    } else {
	fprintf(f, "%-10s %4.1f%% %4.1f%% %4.1f%% %4.1f%% %4.1f%% %4.1f%% %4.1f%% %4.1f%% %4.1f%% %4.1f%% %4.1f%%\n", s,
		cnt->total ? (double) cnt->integer32 * 100 / cnt->total : 0,
		cnt->total ? (double) cnt->unsigned32 * 100 / cnt->total : 0,
		cnt->total ? (double) cnt->integer64 * 100 / cnt->total : 0,
		cnt->total ? (double) cnt->unsigned64 * 100 / cnt->total : 0,
		cnt->total ? (double) cnt->octetstring * 100 / cnt->total : 0,
		cnt->total ? (double) cnt->objectidentifier * 100 / cnt->total : 0,
		cnt->total ? (double) cnt->enums * 100 / cnt->total : 0,
		cnt->total ? (double) cnt->bits * 100 / cnt->total : 0,
		cnt->total ? (double) cnt->float32 * 100 / cnt->total : 0,
		cnt->total ? (double) cnt->float64 * 100 / cnt->total : 0,
		cnt->total ? (double) cnt->float128 * 100 / cnt->total : 0);
    }
}




static void
fprintStatusCounter(FILE *f, StatusCounter *cnt, char *s)
{
    if (!s || !cnt) {
	if (! silent) {
	    fputs(
"# The following table shows the status distribution of various\n"
"# definitions contained in the set of loaded MIB modules.\n"
"\n", f);
	}
	fprintf(f, "%-14s %8s %8s %11s %9s\n", "CATEGORY",
		"TOTAL", "CURRENT", "DEPRECATED", "OBSOLETE");
	return;
    }

    if (raw) {
	fprintf(f, "%-14s %8lu %8lu %11lu %9lu\n", s,
		cnt->total, cnt->current, cnt->deprecated, cnt->obsolete);
    } else {
	fprintf(f, "%-14s %8lu %7.1f%% %10.1f%% %8.1f%%\n", s,
		cnt->total,
		cnt->total ? (double) cnt->current * 100 / cnt->total : 0,
		cnt->total ? (double) cnt->deprecated * 100 / cnt->total : 0,
		cnt->total ? (double) cnt->obsolete * 100 / cnt->total : 0);
    }
}



static void
fprintAccessCounter(FILE *f, AccessCounter *cnt, char *s)
{
    if (!s || !cnt) {
	if (! silent) {
	    fputs(
"# The following table shows the access mode distribution of all scalar\n"
"# or column definitions contained in the set of loaded MIB modules.\n"
"\n", f);
	}
	fprintf(f, "%-14s %8s %10s %9s %7s %8s\n", "CATEGORY",
		"TOTAL", "READWRITE", "READONLY", "NOTIFY", "NOACCES");
	return;
    }

    if (raw) {
	fprintf(f, "%-14s %8lu %10lu %9lu %7lu %8lu\n", s,
		cnt->total, cnt->readwrite, cnt->readonly,
		cnt->notify, cnt->noaccess);
    } else {
	fprintf(f, "%-14s %8lu %9.1f%% %8.1f%% %6.1f%% %7.1f%%\n", s,
		cnt->total,
		cnt->total ? (double) cnt->readwrite * 100 / cnt->total : 0,
		cnt->total ? (double) cnt->readonly * 100 / cnt->total : 0,
		cnt->total ? (double) cnt->notify * 100 / cnt->total : 0,
		cnt->total ? (double) cnt->noaccess * 100 / cnt->total : 0);
    }
}



static void
fprintIndexCounter(FILE *f, IndexCounter *cnt, char *s)
{
    if (! s || ! cnt) {
	if (! silent) {
	    fputs(
"# The following table shows the table index kind distribution of\n"
"# table definitions contained in the set of loaded MIB modules.\n"
"\n", f);
	fprintf(f, "%-14s %8s %8s %8s %8s %8s %8s\n", "CATEGORY",
		"TOTAL", "INDEX", "AUGMENT", "REORDER", "SPARSE", "EXPAND");
	}
	return;
    }
    
    if (raw) {
	fprintf(f, "%-14s %8lu %8lu %8lu %8lu %8lu %8lu\n", s,
		cnt->total, cnt->index, cnt->augment,
		cnt->reorder, cnt->sparse, cnt->expand);
    } else {
	fprintf(f, "%-14s %8lu %7.1f%% %7.1f%% %7.1f%% %7.1f%% %7.1f%%\n", s,
		cnt->total,
		cnt->total ? (double) cnt->index * 100 / cnt->total : 0,
		cnt->total ? (double) cnt->augment * 100 / cnt->total : 0,
		cnt->total ? (double) cnt->reorder * 100 / cnt->total : 0,
		cnt->total ? (double) cnt->sparse * 100 / cnt->total : 0,
		cnt->total ? (double) cnt->expand * 100 / cnt->total : 0);
    }
}



static void
fprintIndexLenCounter(FILE *f, IndexLenCounter *cnt, char *s)
{
    int i;
    int n = sizeof(cnt->length)/sizeof(cnt->length[0]);
    char buf[42];
    
    if (! s || ! cnt) {
	if (! silent) {
	    fputs(
"# The following table shows the table index length distribution of\n"
"# table definitions contained in the set of loaded MIB modules.\n"
"\n", f);
	}
	fprintf(f, "%-10s %6s ", "CATEGORY", "TOTAL");
	for (i = 1; i < n; i++) {
	    sprintf(buf, "[%d]", i);
	    fprintf(f, " %5s", buf);
	}
	fprintf(f, "\n");
	return;
    }

    fprintf(f, "%-10s %6lu ", s, cnt->total);
    if (raw) {
	for (i = 1; i < n; i++) {
	    fprintf(f, " %5lu", cnt->length[i]);
	}
    } else {
	for (i = 1; i < n; i++) {
	    fprintf(f, " %4.1f%%", (double) cnt->length[i] * 100 / cnt->total);
	}
    }
    
    fprintf(f, "\n");
}



static void
fprintTableLenCounter(FILE *f, TableLenCounter *cnt, char *s)
{
    int i;
    int n = sizeof(cnt->length)/sizeof(cnt->length[0]);
    char buf[42];
    
    if (! s || ! cnt) {
	if (! silent) {
	    fputs(
"# The following table shows the table length distribution of\n"
"# table definitions contained in the set of loaded MIB modules.\n"
"\n", f);
	}
	fprintf(f, "%-10s %6s ", "CATEGORY", "TOTAL");
	for (i = 1; i < n; i++) {
	    sprintf(buf, "[%d]", i);
	    fprintf(f, " %5s", buf);
	}
	fprintf(f, "\n");
	return;
    }

    fprintf(f, "%-10s %6lu ", s, cnt->total);
    if (raw) {
	for (i = 1; i < n; i++) {
	    fprintf(f, " %5lu", cnt->length[i]);
	}
    } else {
	for (i = 1; i < n; i++) {
	    fprintf(f, " %4.1f%%", (double) cnt->length[i] * 100 / cnt->total);
	}
    }
    
    fprintf(f, "\n");
}



static void
fprintLengthCounter(FILE *f, LengthCounter *cnt, char *s)
{
    if (! s) {
	if (! silent) {
	    fputs(
"# The following table shows the text clause usage distribution of all\n"
"# definitions contained in the set of loaded MIB modules.\n"
"\n", f);
	}
	fprintf(f, "%-14s %8s %12s %10s %8s %8s\n", "CATEGORY",
		"TOTAL", "DESCRIPTION", "REFERENCE", "UNIT", "FORMAT");
	return;
    }
    
    if (raw) {
	fprintf(f, "%-14s %8lu %12lu %10lu %8lu %8lu\n", s,
		cnt->total, cnt->descr, cnt->reference,
		cnt->units, cnt->format);
    } else {
	fprintf(f, "%-14s %8lu %11.1f%% %9.1f%% %7.1f%% %7.1f%%\n", s,
		cnt->total,
		cnt->total ? (double) cnt->descr * 100 / cnt->total : 0,
		cnt->total ? (double) cnt->reference * 100 / cnt->total : 0,
		cnt->total ? (double) cnt->units * 100 / cnt->total : 0,
		cnt->total ? (double) cnt->format * 100 / cnt->total : 0);
    }
}



static void
fprintLengthCounter2(FILE *f, LengthCounter *cnt, char *s)
{
    if (! s) {
	if (! silent) {
	    fprintf(f,
"# The following table shows the %s text length distribution (in\n"
"# bytes) of all definitions contained in the set of loaded MIB modules.\n"
"\n", raw ? "total" : "average");
	}
	fprintf(f, "%-14s %8s %12s %10s %8s %8s\n", "CATEGORY",
		"TOTAL", "DESCRIPTION", "REFERENCE", "UNIT", "FORMAT");
	return;
    }
    
    if (raw) {
	fprintf(f, "%-14s %8lu %12lu %10lu %8lu %8lu\n", s,
		cnt->total, cnt->descr_len, cnt->reference_len,
		cnt->units_len, cnt->format_len);
    } else {
	fprintf(f, "%-14s %8lu %12.1f %10.1f %8.1f %8.1f\n", s,
		cnt->total,
		cnt->descr ? (double) cnt->descr_len / cnt->descr : 0,
		cnt->reference ? (double) cnt->reference_len / cnt->reference : 0,
		cnt->units ? (double) cnt->units_len / cnt->units : 0,
		cnt->format ? (double) cnt->format_len / cnt->format : 0);
    }
}



static void
fprintfComplexity(FILE *f, Metrics *metrics)
{
    unsigned long cmplx = 0, fctrs = 0;
    unsigned long total_cmplx = 0, total_fctrs = 0;

    if (! silent) {
	fputs(
"# The following table shows the complexity metrics of the set of loaded\n"
"# MIB modules.\n"
"\n", f);
    }
    fprintf(f, "%-14s %8s %8s %8s %8s\n", "CATEGORY", "TOTAL",
	    "RAW", "WEIGHT", "COMPLEXITY");
    
    cmplx = metrics->accessScalars.readonly  * 1;
    fctrs = metrics->accessScalars.readonly;
    fprintf(f, "%-14s %8lu %8lu\n", "Scalars (ro):", fctrs, cmplx);
    total_cmplx += cmplx;
    total_fctrs += fctrs;
	    
    cmplx = metrics->accessScalars.readwrite * 2;
    fctrs = metrics->accessScalars.readwrite;
    fprintf(f, "%-14s %8lu %8lu\n", "Scalars (rw):", fctrs, cmplx);
    total_cmplx += cmplx;
    total_fctrs += fctrs;

    cmplx = metrics->accessColumns.readonly * 2;
    fctrs = metrics->accessColumns.readonly;
    fprintf(f, "%-14s %8lu %8lu\n", "Columns (ro):", fctrs, cmplx);
    total_cmplx += cmplx;
    total_fctrs += fctrs;

    cmplx = metrics->accessColumns.readwrite * 3;
    fctrs = metrics->accessColumns.readwrite;
    fprintf(f, "%-14s %8lu %8lu\n", "Columns (rw):", fctrs, cmplx);
    total_cmplx += cmplx;
    total_fctrs += fctrs;

    /* readcreate tables ? */

    /* table index complexity ? */

    {
	int i;
	cmplx = 0;
	for (i = 0; i < 100; i++) {
	    cmplx += 3 * i * metrics->indexComplexity.complexity[i];
	}
	fprintf(f, "%-14s %8lu %8lu\n", "Indexes:", metrics->indexComplexity.total, cmplx);
    }

    fprintf(f, "%-14s %8lu %8lu\n", "Summary:", total_fctrs, total_cmplx);
}



static void
fprintMetrics(FILE *f, Metrics *metrics)
{
    fprintStatusCounter(f, NULL, NULL);
    fprintStatusCounter(f, &metrics->statusTypes, "Types:");
    fprintStatusCounter(f, &metrics->statusTables, "Tables:");
    fprintStatusCounter(f, &metrics->statusColumns, "Columns:");
    fprintStatusCounter(f, &metrics->statusScalars, "Scalars:");
    fprintStatusCounter(f, &metrics->statusNotifications, "Notifications:");
    fprintStatusCounter(f, &metrics->statusGroups, "Groups:");
    fprintStatusCounter(f, &metrics->statusCompliances, "Compliances:");
    fprintStatusCounter(f, &metrics->statusAll, "Summary:");
    fprintf(f, "\n");
    fprintAccessCounter(f, NULL, NULL);
    fprintAccessCounter(f, &metrics->accessColumns, "Columns:");
    fprintAccessCounter(f, &metrics->accessScalars, "Scalars:");
    fprintAccessCounter(f, &metrics->accessAll, "Summary:");
    fprintf(f, "\n");
    fprintIndexCounter(f, NULL, NULL);
    fprintIndexCounter(f, &metrics->indexTables, "Tables:");
    fprintf(f, "\n");
    fprintIndexLenCounter(f, NULL, NULL);
    fprintIndexLenCounter(f, &metrics->indexLenTables, "Tables:");
    fprintf(f, "\n");
    fprintTableLenCounter(f, NULL, NULL);
    fprintTableLenCounter(f, &metrics->tableLength, "Tables:");
    fprintf(f, "\n");
    fprintLengthCounter(f, NULL, NULL);
    fprintLengthCounter(f, &metrics->lengthTypes, "Types:");
    fprintLengthCounter(f, &metrics->lengthTables, "Tables:");
    fprintLengthCounter(f, &metrics->lengthColumns, "Columns:");
    fprintLengthCounter(f, &metrics->lengthScalars, "Scalars:");
    fprintLengthCounter(f, &metrics->lengthNotifications, "Notifications:");
    fprintLengthCounter(f, &metrics->lengthAll, "Summary:");
    fprintf(f, "\n");
    fprintLengthCounter2(f, NULL, NULL);
    fprintLengthCounter2(f, &metrics->lengthTypes, "Types:");
    fprintLengthCounter2(f, &metrics->lengthTables, "Tables:");
    fprintLengthCounter2(f, &metrics->lengthColumns, "Columns:");
    fprintLengthCounter2(f, &metrics->lengthScalars, "Scalars:");
    fprintLengthCounter2(f, &metrics->lengthNotifications, "Notifications:");
    fprintLengthCounter2(f, &metrics->lengthAll, "Summary:");
    fprintf(f, "\n");
    fprintBasetypeCounter(f, NULL, NULL);
    fprintBasetypeCounter(f, &metrics->basetypesColumns, "Columns:");
    fprintBasetypeCounter(f, &metrics->basetypesScalars, "Scalars:");
    fprintBasetypeCounter(f, &metrics->basetypesAll, "Summary:");
    fprintf(f, "\n");
    fprintfComplexity(f, metrics);
    fprintf(f, "\n");
    fprintTypeUsage(f, typeList);
    freeUsageCounter(typeList), typeList = NULL;
    fprintf(f, "\n");
    fprintExtTypeUsage(f, extTypeList);
    freeUsageCounter(extTypeList), extTypeList = NULL;
    fprintf(f, "\n");
    fprintExtNodeUsage(f, extNodeList);
    freeUsageCounter(extNodeList), extNodeList = NULL;
    fprintf(f, "\n");
    fprintModuleUsage(f, extModuleList);
    freeUsageCounter(extModuleList), extModuleList = NULL;
    fprintf(f, "\n");
    fprintIndexComplexity(f, indexComplexityList);
    freeUsageCounter(indexComplexityList), indexComplexityList = NULL;
    fprintf(f, "\n");
}



static void
dumpMetrics(int modc, SmiModule **modv, int flags, char *output)
{
    Metrics   metrics;
    int       i;
    FILE      *f = stdout;

    silent = (flags & SMIDUMP_FLAG_SILENT);

    if (output) {
	f = fopen(output, "w");
	if (!f) {
	    fprintf(stderr, "smidump: cannot open %s for writing: ", output);
	    perror(NULL);
	    exit(1);
	}
    }

    if (flags & SMIDUMP_FLAG_UNITE) {
	if (! silent) {
	    int pos = 8888;
	    fprintf(f, "# united module metrics [%d modules] "
		    "(generated by smidump " SMI_VERSION_STRING ")\n", modc);
	    fprintf(f, "#\n# smidump -u -f metrics");
	    if (raw) fprintf(f, " --metrics-raw");
	    for (i = 0; i < modc; i++) {
		int len = strlen(modv[i]->name);
		if (pos + len > 70) {
		    fprintf(f, " \\\n#\t"), pos = 8;
		}
		fprintf(f, "%s ", modv[i]->name);
		pos += len + 1;
	    }
	    fprintf(f, "%s\n", (pos == 8) ? "" : "\n");
	}

	fprintRevision(f, modc, modv);
	    
	for (i = 0; i < modc; i++) {
	    memset(&metrics, 0, sizeof(Metrics));
	}
	for (i = 0; i < modc; i++) {
	    addMetrics(&metrics, modv[i]);
	}
	fprintMetrics(f, &metrics);
    } else {
	for (i = 0; i < modc; i++) {
	    if (! silent) {
		fprintf(f, "# %s module metrics (generated by smidump "
			SMI_VERSION_STRING ")\n\n", modv[i]->name);
	    }

	    fprintRevision(f, 1, modv+i);
    
	    memset(&metrics, 0, sizeof(Metrics));
	    addMetrics(&metrics, modv[i]);
	    fprintMetrics(f, &metrics);
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



void
initMetrics()
{
    static SmidumpDriverOption opt[] = {
	{ "raw", OPT_FLAG, &raw, 0,
	  "generate raw statistics (no percentages)"},
        { 0, OPT_END, 0, 0 }
    };

    static SmidumpDriver driver = {
	"metrics",
	dumpMetrics,
	0,
	0,
	"metrics characterizing MIB modules",
	opt,
	NULL
    };
    
    smidumpRegisterDriver(&driver);
}
