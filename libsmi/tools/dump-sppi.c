/*
 * dump-sppi.c --
 *
 *      Operations to dump SPPI module information.
 *
 * Copyright (c) 1999 Frank Strauss, Technical University of Braunschweig.
 *
 * See the file "COPYING" for information on usage and redistribution
 * of this file, and for a DISCLAIMER OF ALL WARRANTIES.
 *
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


#define  INDENT		4    /* indent factor */
#define  INDENTVALUE	16   /* column to start values, except multiline */
#define  INDENTTEXTS	 9   /* column to start multiline texts */
#define  INDENTMAX	72   /* max column to fill, break lines otherwise */

static char *convertType[] = {
    NULL,		 "INTEGER",	       NULL,	   "Integer32",
    NULL,		 "Bits",	       NULL,	   "BITS",
    NULL,		 "OctetString",	       NULL,	   "OCTET STRING",
    NULL,		 "ObjectIdentifier",   NULL,	   "OBJECT IDENTIFIER",
    NULL,		 "IpAddress",          NULL,	   "InetAddress",
    NULL,		 "Counter64",          NULL,	   "Unsigned64",
    NULL,		 "Counter32",          NULL,	   "Unsigned32",
    NULL,		 "Gauge32",            NULL,	   "Unsigned32",
    NULL, NULL, NULL, NULL };

static char *convertTypeMibToPib[] = {
    NULL,		 "SnmpAdminString",    NULL,	   "OCTET STRING",
    NULL, NULL, NULL, NULL };

static char *convertImport[] = {
    "SNMPv2-SMI", "Integer32",      "COPS-PR-SPPI", "Integer32",
    "SNMPv2-SMI", "Unsigned32",     "COPS-PR-SPPI", "Unsigned32",
    "SNMPv2-SMI", "TimeTicks",      "COPS-PR-SPPI", "TimeTicks",
    "SNMPv2-SMI", "IpAddress",      "INET-ADDRESS-MIB", "InetAddress",
    "SNMPv2-SMI", "MODULE-IDENTITY","COPS-PR-SPPI", "MODULE-IDENTITY",
    "SNMPv2-SMI", "MODULE-COMPLIANCE","COPS-PR-SPPI", "MODULE-COMPLIANCE",
    "SNMPv2-SMI", "OBJECT-TYPE",    "COPS-PR-SPPI", "OBJECT-TYPE",
    "SNMPv2-SMI", "OBJECT-IDENTITY","COPS-PR-SPPI", "OBJECT-IDENTITY",
    "SNMPv2-TC",  "TEXTUAL-CONVENTION","COPS-PR-SPPI", "TEXTUAL-CONVENTION",
    NULL, NULL, NULL, NULL };

static int current_column = 0;
static int silent = 0;
static int mibtopib = 0;


typedef struct Import {
    char          *module;
    char          *name;
    struct Import *nextPtr;
} Import;

static Import *importList = NULL;



static int invalidType(SmiBasetype basetype)
{
    return (basetype == SMI_BASETYPE_FLOAT32)
	|| (basetype == SMI_BASETYPE_FLOAT64)
	|| (basetype == SMI_BASETYPE_FLOAT128);
}



static char *getStatusString(SmiStatus status)
{
    return
	(status == SMI_STATUS_CURRENT)     ? "current" :
	(status == SMI_STATUS_DEPRECATED)  ? "deprecated" :
	(status == SMI_STATUS_OBSOLETE)    ? "obsolete" :
					     "<unknown>";
}



static char *getAccessString(SmiAccess access, int pibaccess)
{
    return
        (access == SMI_ACCESS_NOTIFY)	      ? "notify" :
        (access == SMI_ACCESS_INSTALL)	      ? "install" :
        (access == SMI_ACCESS_INSTALL_NOTIFY) ? "install-notify" :
        (access == SMI_ACCESS_REPORT_ONLY)    ? "report-only" :
        (access == SMI_ACCESS_NOT_ACCESSIBLE) ?
             (pibaccess == 1 ? "report-only" : "not-accessible") :
        mibtopib                              ? "notify" : 
					        "<unknown>";
}


static int isSmiOnlyType(char *type)
{
    return (!strcmp(type, "Counter32") ||
            !strcmp(type, "Counter64") ||
            !strcmp(type, "Gauge32"));
}


static char *getTimeString(time_t t)
{
    static char   *s = NULL;
    struct tm	  *tm;

    if (s) xfree(s);

    tm = gmtime(&t);
    smiAsprintf(&s, "%04d%02d%02d%02d%02dZ",
		tm->tm_year + 1900, tm->tm_mon + 1, tm->tm_mday,
		tm->tm_hour, tm->tm_min);
    return s;
}



static char *getTypeString(SmiBasetype basetype, SmiType *smiType)
{
    int         i;
    char        *typeModule, *typeName;

    typeName = smiType ? smiType->name : NULL;
    typeModule = smiType ? smiGetTypeModule(smiType)->name : NULL;

    if (typeName &&
	(basetype != SMI_BASETYPE_ENUM) &&
	(basetype != SMI_BASETYPE_BITS)) {
        if (mibtopib)
            for(i=0; convertTypeMibToPib[i+1]; i += 4) {
	        if ((!strcmp(typeName, convertTypeMibToPib[i+1])) &&
	            ((!typeModule) || (!convertTypeMibToPib[i]) ||
	             (!strcmp(typeModule, convertTypeMibToPib[i])))) {
	            return convertTypeMibToPib[i+3];
	        }
            }
	for(i=0; convertType[i+1]; i += 4) {
	    if ((!strcmp(typeName, convertType[i+1])) &&
		((!typeModule) || (!convertType[i]) ||
		 (!strcmp(typeModule, convertType[i])))) {
		return convertType[i+3];
	    }
	}
    }

    if ((!typeModule) || (!strlen(typeModule)) || (!typeName)) {
	if (basetype == SMI_BASETYPE_ENUM) {
	    return "INTEGER";
	}
	if (basetype == SMI_BASETYPE_BITS) {
            return "BITS";
	}
    }
	
    /* TODO: fully qualified if unambigous */

    return typeName;
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
	sprintf(append, " %u%s", parentNode->oid[parentNode->oidlen-1], s);

	/* retrieve the parent SmiNode */
	parentNode = smiGetParentNode(parentNode);

	if (!parentNode) {
	    sprintf(s, "%s", append);
	    return s;
	}
	
	/* found an imported or a local parent node? */
	if ((parentNode->name && strlen(parentNode->name)) &&
	    (smiIsImported(smiModule, NULL, parentNode->name) ||
	     (!importedParent &&
	      (smiGetNodeModule(parentNode) == smiModule)) ||
	     (parentNode->oidlen == 1))) {
	    sprintf(s, "%s%s", parentNode->name, append);
	    return s;
	}
	
    } while (parentNode);

    s[0] = 0;
    for (i=0; i < smiNode->oidlen; i++) {
	if (i) strcat(s, " ");
	sprintf(&s[strlen(s)], "%u", smiNode->oid[i]);
    }
    return s;
}



static char *getUppercaseString(char *s)
{
    static char *ss;

    ss = xstrdup(s);
    ss[0] = (char)toupper((int)ss[0]);
    return ss;
}



static int isObjectGroup(SmiNode *groupNode)
{
    SmiNode     *smiNode;
    SmiElement  *smiElement;
    
    for (smiElement = smiGetFirstElement(groupNode); smiElement;
	 smiElement = smiGetNextElement(smiElement)) {

	smiNode = smiGetElementNode(smiElement);
	
	if (smiNode->nodekind != SMI_NODEKIND_SCALAR
	    && smiNode->nodekind != SMI_NODEKIND_COLUMN) {
	    return 0;
	}
    }

    return 1;
}



static Import* addImport(char *module, char *name)
{
    Import **import, *newImport;
    int    i;

    for (i = 0; convertImport[i]; i += 4) {
	if (convertImport[i] && convertImport[i+1]
	    && !strcmp(module, convertImport[i])
	    && !strcmp(name, convertImport[i+1])) {
	    module = convertImport[i+2];
	    name = convertImport[i+3];
	    break;
	} else if (convertImport[i] && !convertImport[i+1]
		   && !strcmp(module, convertImport[i])) {
	    module = convertImport[i+2];
	    break;
	}
    }

    if (!module || !name) {
	return NULL;
    }
	    
    for (import = &importList; *import; import = &(*import)->nextPtr) {
	int c = strcmp((*import)->module, module);
	if (c < 0) continue;
	if (c == 0) {
	    int d = strcmp((*import)->name, name);
	    if (d < 0) continue;
	    if (d == 0) return *import;
	    if (d > 0) break;
	}
	if (c > 0) break;
    }

    newImport = xmalloc(sizeof(Import));
    if (! newImport) {
	return NULL;
    }
    newImport->module = module;
    newImport->name = name;
    newImport->nextPtr = *import;
    *import = newImport;
	
    return *import;
}



static void createImportList(SmiModule *smiModule)
{
    SmiNode     *smiNode;
    SmiType     *smiType;
    SmiNodekind kind = SMI_NODEKIND_SCALAR | SMI_NODEKIND_COLUMN;
    SmiImport   *smiImport;
    SmiModule	*smiModule2;

    if (mibtopib) {
        addImport("TUBS-SMI", "ibrmibtopib");
        if (smiGetFirstNode(smiModule, SMI_NODEKIND_COLUMN)) {
            addImport("COPS-PR-SPPI", "MODULE-COMPLIANCE");
            addImport("COPS-PR-SPPI", "OBJECT-GROUP");
            addImport("COPS-PR-SPPI", "OBJECT-IDENTITY");
            addImport("COPS-PR-SPPI-TC", "InstanceId");
        }
    }
    
    for(smiNode = smiGetFirstNode(smiModule, kind); smiNode;
	smiNode = smiGetNextNode(smiNode, kind)) {
	smiType = smiGetNodeType(smiNode);
	if (smiType && (smiType->decl == SMI_DECL_IMPLICIT_TYPE)) {
	    smiType = smiGetParentType(smiType);
	}
	if (smiType) {
	    smiModule2 = smiGetTypeModule(smiType);
	    if (smiModule2 && (smiModule2 != smiModule)) {
		if (strlen(smiModule2->name) && smiType->name &&
                    !isSmiOnlyType(smiType->name)) {
		    addImport(smiModule2->name, smiType->name);
		}
	    }
	}

	if (smiType && smiType->basetype == SMI_BASETYPE_INTEGER32) {
	    addImport("COPS-PR-SPPI", "Integer32");
	}

	if (smiType && smiType->basetype == SMI_BASETYPE_INTEGER64) {
	    addImport("COPS-PR-SPPI", "Integer64");
	}

	if (smiType && smiType->basetype == SMI_BASETYPE_UNSIGNED32) {
	    addImport("COPS-PR-SPPI", "Unsigned32");
	}

	if (smiType && smiType->basetype == SMI_BASETYPE_UNSIGNED64) {
	    addImport("COPS-PR-SPPI", "Unsigned64");
	}

	if ((smiNode->value.basetype == SMI_BASETYPE_OBJECTIDENTIFIER) &&
	    (!strcmp(smiNode->value.value.ptr, "zeroDotZero"))) {
	    addImport("SNMPv2-SMI", "zeroDotZero");
	}
    }

    smiNode = smiGetFirstNode(smiModule,
			      SMI_NODEKIND_SCALAR | SMI_NODEKIND_COLUMN
			      | SMI_NODEKIND_TABLE | SMI_NODEKIND_ROW);
    if (smiNode) {
	addImport("COPS-PR-SPPI", "OBJECT-TYPE");
    }

    smiNode = smiGetModuleIdentityNode(smiModule);
    if (smiNode) {
	if (strcmp("COPS-PR-SPPI", smiModule->name)) {
	    addImport("COPS-PR-SPPI", "MODULE-IDENTITY");
	}
    }
    
    for(smiNode = smiGetFirstNode(smiModule, SMI_NODEKIND_NODE);
	smiNode; smiNode = smiGetNextNode(smiNode, SMI_NODEKIND_NODE)) {
	if (smiNode->status != SMI_STATUS_UNKNOWN &&
	    smiNode != smiGetModuleIdentityNode(smiModule)) {
	    if (strcmp("COPS-PR-SPPI", smiModule->name)) {
		addImport("COPS-PR-SPPI", "OBJECT-IDENTITY");
	    }
	    break;
	}
    }

    smiNode = smiGetFirstNode(smiModule, SMI_NODEKIND_COMPLIANCE);
    if (smiNode) {
	if (strcmp("COPS-PR-SPPI", smiModule->name)) {
	    addImport("COPS-PR-SPPI", "MODULE-COMPLIANCE");
	}
    }

    for(smiNode = smiGetFirstNode(smiModule, SMI_NODEKIND_GROUP);
	smiNode;
	smiNode = smiGetNextNode(smiNode, SMI_NODEKIND_GROUP)) {
	if (strcmp("COPS-PR-SPPI", smiModule->name)) {
	    addImport("COPS-PR-SPPI", "OBJECT-GROUP");
	}
    }

    for(smiType = smiGetFirstType(smiModule);
	smiType; smiType = smiGetNextType(smiType)) {
	if (smiType->description) {
	    if (strcmp("COPS-PR-SPPI", smiModule->name)) {
		addImport("COPS-PR-SPPI", "TEXTUAL-CONVENTION");
	    }
	}
        if (smiType->decl == SMI_DECL_TEXTUALCONVENTION) {
            switch (smiType->basetype) {
	    case SMI_BASETYPE_INTEGER32:
		addImport("COPS-PR-SPPI", "Integer32");
		break;
	    case SMI_BASETYPE_INTEGER64:
		addImport("COPS-PR-SPPI", "Integer64");
		break;
	    case SMI_BASETYPE_UNSIGNED32:
		addImport("COPS-PR-SPPI", "Unsigned32");
		break;
	    case SMI_BASETYPE_UNSIGNED64:
		addImport("COPS-PR-SPPI", "Unsigned64");
		break;
	    default:
		break;
            }
        }
    }

    for (smiImport = smiGetFirstImport(smiModule); smiImport;
	 smiImport = smiGetNextImport(smiImport)) {
	if ((islower((int) smiImport->name[0]) ||
	    (smiImport->module && !strcmp(smiImport->module, "SNMPv2-SMI")) ||
	    (smiImport->module && !strcmp(smiImport->module, "SNMPv2-TC"))) &&
            !isSmiOnlyType(smiImport->name)) {
	    addImport(smiImport->module, smiImport->name);
	}
    }
}



static void freeImportList(void)
{
    Import *import, *freeme;

    for (import = importList; import; ) {
	freeme = import;
	import = import->nextPtr;
	xfree(freeme);
    }
    importList = NULL;
}



static void fprint(FILE *f, char *fmt, ...)
{
    va_list ap;
    char    *s;
    char    *p;
    
    va_start(ap, fmt);
    current_column += smiVasprintf(&s, fmt, ap);
    va_end(ap);
    fputs(s, f);
    if ((p = strrchr(s, '\n'))) {
	current_column = strlen(p) - 1;
    }
    free(s);
}



static void fprintSegment(FILE *f, int column, char *string,
			  int length, int comment)
{
    char *format;
    
    if (comment) {
	format = "-- %*c%s";
	/* if (column >= 3) column -= 3; */
    } else {
	format = "%*c%s";
    }
    
    fprint(f, format, column, ' ', string);
    if (length) {
	fprint(f, "%*c", length - strlen(string) - column, ' ');
    }
}



static void fprintWrapped(FILE *f, int column, char *string, int comment)
{
    if ((current_column + strlen(string)) > INDENTMAX) {
	putc('\n', f);
	current_column = 0;
	fprintSegment(f, column, "", 0, comment);
    }
    fprint(f, "%s", string);
}



static void fprintMultilineString(FILE *f, const char *s, const int comment)
{
    int i, len;
    
    fprintSegment(f, INDENTTEXTS - 1, "\"", 0, comment);
    if (s) {
	len = strlen(s);
	for (i=0; i < len; i++) {
	    putc(s[i], f);
	    current_column++;
	    if (s[i] == '\n') {
		current_column = 0;
		fprintSegment(f, INDENTTEXTS, "", 0, comment);
	    }
	}
    }
    putc('\"', f);
    current_column++;
}



static char *getValueString(SmiValue *valuePtr, SmiType *typePtr)
{
    static char    s[1024];
    char           ss[9];
    int		   n;
    unsigned int   i;
    SmiNamedNumber *nn;
    SmiNode        *nodePtr;
    
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
    case SMI_BASETYPE_ENUM:
	for (nn = smiGetFirstNamedNumber(typePtr); nn;
	     nn = smiGetNextNamedNumber(nn)) {
	    if (nn->value.value.unsigned32 == valuePtr->value.unsigned32)
		break;
	}
	if (nn) {
	    sprintf(s, "%s", nn->name);
	} else {
	    sprintf(s, "%ld", valuePtr->value.integer32);
	}
	break;
    case SMI_BASETYPE_OCTETSTRING:
	for (i = 0; i < valuePtr->len; i++) {
	    if (!isprint((int)valuePtr->value.ptr[i])) break;
	}
	if (i == valuePtr->len) {
	    sprintf(s, "\"%s\"", valuePtr->value.ptr);
	} else {
            sprintf(s, "'%*s'H", 2 * valuePtr->len, " ");
            for (i=0; i < valuePtr->len; i++) {
                sprintf(ss, "%02x", valuePtr->value.ptr[i]);
                strncpy(&s[1+2*i], ss, 2);
            }
	}
	break;
    case SMI_BASETYPE_BITS:
	sprintf(s, "{");
	for (i = 0, n = 0; i < valuePtr->len * 8; i++) {
	    if (valuePtr->value.ptr[i/8] & (1 << (7-(i%8)))) {
		for (nn = smiGetFirstNamedNumber(typePtr); nn;
		     nn = smiGetNextNamedNumber(nn)) {
		    if (nn->value.value.unsigned32 == i)
			break;
		}
		if (nn) {
		    if (n)
			sprintf(&s[strlen(s)], ", ");
		    n++;
		    sprintf(&s[strlen(s)], "%s", nn->name);
		}
	    }
	}
	sprintf(&s[strlen(s)], "}");
	break;
    case SMI_BASETYPE_FLOAT32:
    case SMI_BASETYPE_FLOAT64:
    case SMI_BASETYPE_FLOAT128:
    case SMI_BASETYPE_UNKNOWN:
    case SMI_BASETYPE_POINTER:
	break;
    case SMI_BASETYPE_OBJECTIDENTIFIER:
	nodePtr = smiGetNodeByOID(valuePtr->len, valuePtr->value.oid);
	if (nodePtr) {
	    sprintf(s, "%s", nodePtr->name);
	} else {
	    strcpy(s, "{");
	    for (i=0; i < valuePtr->len; i++) {
		if (i) strcat(s, " ");
		sprintf(&s[strlen(s)], "%u", valuePtr->value.oid[i]);
	    }
	    strcat(s, "}");
	}
	break;
    }

    return s;
}



static void fprintSubtype(FILE *f, SmiType *smiType, const int comment)
{
    SmiRange       *range;
    SmiNamedNumber *nn;
    char	   s[1024];
    int		   i;

    if ((smiType->basetype == SMI_BASETYPE_ENUM) ||
	(smiType->basetype == SMI_BASETYPE_BITS)) {
	for(i = 0, nn = smiGetFirstNamedNumber(smiType);
	    nn; i++, nn = smiGetNextNamedNumber(nn)) {
	    if (i) {
		fprint(f, ", ");
	    } else {
		if (comment) {
		    fprint(f, "\n");
		    fprintSegment(f, INDENT, "", INDENTVALUE, comment);
		    fprint(f, "  { ");
		} else {
		    fprint(f, " { ");
		}
	    }
	    sprintf(s, "%s(%s)", nn->name,
		    getValueString(&nn->value, smiType));
	    fprintWrapped(f, INDENTVALUE + INDENT, s, comment);
	}
	if (i) {
	    fprint(f, " }");
	}
    } else {
	for(i = 0, range = smiGetFirstRange(smiType);
	    range; i++, range = smiGetNextRange(range)) {
	    if (i) {
		fprint(f, " | ");
	    } else {
		if (smiType->basetype == SMI_BASETYPE_OCTETSTRING) {
		    fprint(f, " (SIZE(");
		} else {
		    fprint(f, " (");
		}
	    }	    
	    if (memcmp(&range->minValue, &range->maxValue,
		       sizeof(SmiValue))) {
		sprintf(s, "%s", getValueString(&range->minValue, smiType));
		sprintf(&s[strlen(s)], "..%s", 
			getValueString(&range->maxValue, smiType));
	    } else {
		sprintf(s, "%s", getValueString(&range->minValue, smiType));
	    }
	    fprintWrapped(f, INDENTVALUE + INDENT, s, 0);
	}
	if (i) {
	    if (smiType->basetype == SMI_BASETYPE_OCTETSTRING) {
		fprint(f, ")");
	    }
	    fprint(f, ")");
	}
    }
}



static void fprintIndex(FILE *f, SmiNode *indexNode, const int comment)
{
    SmiElement *smiElement;
    int        n, j;

    /* PIBs can contain both a PIB-INDEX | EXTENDS | AUGMENTS and an
       INDEX clause. The index list's first element is the PIB-INDEX
       (or EXTENDS/AUGMENTS) followed by the elements belonging to the
       INDEX clause. */
    
    for (n = 0, smiElement = smiGetFirstElement(indexNode); smiElement;
	 n++, smiElement = smiGetNextElement(smiElement));
    if (mibtopib) {
        SmiNode *smiParentNode = smiGetParentNode(indexNode);
        size_t len = strlen(smiParentNode->name);
        char *instanceId = xmalloc(len + 11);

        strcpy(instanceId, smiParentNode->name);
        if (len > 54)
          len = 54;
        strcpy(&instanceId[len], "InstanceId");
        fprintSegment(f, INDENT, "PIB-INDEX", INDENTVALUE, 0);
        fprint(f, "{ ");
        fprintWrapped(f, INDENTVALUE + 2, instanceId, 0);
        fprint(f, " }\n");
        fprintSegment(f, INDENT, "INDEX", INDENTVALUE, 0);
        fprint(f, "{ ");
        for (j = 0, smiElement = smiGetFirstElement(indexNode); smiElement;
	     j++, smiElement = smiGetNextElement(smiElement)) {
             if (j)
                 fprint(f, ", ");
             fprintWrapped(f, INDENTVALUE + 2,
                           smiGetElementNode(smiElement)->name, 0);
        }
        xfree(instanceId);
    }
    else
        for (j = 0, smiElement = smiGetFirstElement(indexNode); smiElement;
	     j++, smiElement = smiGetNextElement(smiElement)) {
            if (!j) {
                switch (indexNode->indexkind) {
                    case SMI_INDEX_INDEX:
                        fprintSegment(f, INDENT, "PIB-INDEX", INDENTVALUE,
                                      comment);
                        break;
                    case SMI_INDEX_AUGMENT:
                        fprintSegment(f, INDENT, "AUGMENTS", INDENTVALUE,
                                      comment);
                        break;
                    case SMI_INDEX_SPARSE:
                        fprintSegment(f, INDENT, "EXTENDS", INDENTVALUE,
                                      comment);
                        break;
		    case SMI_INDEX_UNKNOWN:
		    case SMI_INDEX_REORDER:
		    case SMI_INDEX_EXPAND:
                        fprintSegment(f, INDENT, "-- unsupported indexing --",
				      INDENTVALUE, comment);
			break;
                }
                fprint(f, "{ ");
            } else if (j == 1) {
                fprint(f, " }\n");
                fprintSegment(f, INDENT, "INDEX", INDENTVALUE, comment);
                fprint(f, "{ ");
            } else
	        fprint(f, ", ");
	    if (indexNode->implied && ((j+1) == n)) {
	        fprintWrapped(f, INDENTVALUE + 2, "IMPLIED ", 0);
	    }
	    fprintWrapped(f, INDENTVALUE + 2,
		          smiGetElementNode(smiElement)->name, 0);
	    /* TODO: non-local name if non-local */
        } /* TODO: empty? -> print error */
    fprint(f, " }\n");
}



static void fprintUniqueness(FILE *f, SmiNode *indexNode, const int comment)
{
    SmiElement *smiElement;
    int        j;

    smiElement = smiGetFirstUniquenessElement(indexNode);
    if (!smiElement)
        return;
    fprintSegment(f, INDENT, "UNIQUENESS", INDENTVALUE, comment);
    fprint(f, "{ ");
    for (j = 0; smiElement; j++,
         smiElement = smiGetNextElement(smiElement)) {
        if (j)
	    fprint(f, ", ");
	fprintWrapped(f, INDENTVALUE + 2,
		      smiGetElementNode(smiElement)->name, 0);
	/* TODO: non-local name if non-local */
    } /* TODO: empty? -> print error */
    fprint(f, " }\n");
}



static void fprintInstallErrors(FILE *f, SmiNode *indexNode, const int comment)
{
    SmiElement *smiElement;
    SmiNode    *smiNode;
    int        j;
    char       *id;

    smiElement = smiGetFirstElement(indexNode);
    if (!smiElement)
        return;
    fprintSegment(f, INDENT, "INSTALL-ERRORS", INDENTVALUE, comment);
    fprint(f, "{ ");
    for (j = 0; smiElement; j++,
         smiElement = smiGetNextElement(smiElement)) {
        smiNode = smiGetElementNode(smiElement);
        id = xmalloc(strlen(smiNode->name) + 10);
        sprintf(id, "%s (%ld)%s", smiNode->name,
                smiNode->oidlen ? (long)smiNode->oid : 0,
                smiGetNextElement(smiElement) ? ", " : "");
        fprintWrapped(f, 2+INDENTVALUE, id, 0);
        xfree(id);
	/* TODO: non-local name if non-local */
    } /* TODO: empty? -> print error */
    fprint(f, " }\n");
}



static void fprintImports(FILE *f)
{
    Import        *import;
    char	  *lastModulename = NULL;
    char	  *importedModulename, *importedDescriptor;
    int		  i;

    for(import = importList; import; import = import->nextPtr) {
	importedModulename = import->module;
	importedDescriptor = import->name;

	if (!strlen(importedModulename))
	    continue;
	
	for(i = 0; convertImport[i]; i += 4) {
	    if (convertImport[i] && convertImport[i+1]
		&& !strcmp(importedModulename, convertImport[i])
		&& !strcmp(importedDescriptor, convertImport[i+1])) {
		importedModulename = convertImport[i+2];
		importedDescriptor = convertImport[i+3];
		break;
	    } else if (convertImport[i] && !convertImport[i+1]
		       && !strcmp(importedModulename, convertImport[i])) {
		importedModulename = convertImport[i+2];
		break;
	    }
	}

	if (importedModulename && importedDescriptor &&
	    strlen(importedDescriptor)) {
	    if ((!lastModulename) ||
		strcmp(importedModulename, lastModulename)) {
		if (!lastModulename) {
		    fprint(f, "IMPORTS");
		} else {
		    fprint(f, "\n");
		    fprintSegment(f, 2 * INDENT, "", 0, 0);
		    fprint(f, "FROM %s", lastModulename);
		}
		fprint(f, "\n");
		fprintSegment(f, INDENT, "", 0, 0);
	    } else {
		fprint(f, ", ");
	    }
	    fprintWrapped(f, INDENT, importedDescriptor, 0);
	    lastModulename = importedModulename;
	}
    }
    if (lastModulename) {
	fprint(f, "\n");
	fprintSegment(f, 2 * INDENT, "", 0, 0);
	fprint(f, "FROM %s;\n\n", lastModulename);
    }
}



static void fprintModuleIdentity(FILE *f, SmiModule *smiModule)
{
    SmiRevision  *smiRevision;
    SmiNode      *smiNode, *smiNode2;
    SmiElement   *smiElement;
    char         *id;

    smiNode = smiGetModuleIdentityNode(smiModule);
    
    if (smiNode) {

        fprint(f, "%s MODULE-IDENTITY\n", smiNode->name);
        fprintSegment(f, INDENT, "SUBJECT-CATEGORIES", INDENTVALUE, 0);
        fprint(f, "{ ");
        smiElement = smiGetFirstElement(smiNode);
        if (smiElement && smiGetElementNode(smiElement))
        {
            for (; smiElement; smiElement = smiGetNextElement(smiElement)) {
                smiNode2 = smiGetElementNode(smiElement);
                id = xmalloc(strlen(smiNode2->name) + 10);
                if (smiNode2->oidlen)
                    sprintf(id, "%s (%ld)%s", smiNode2->name, (long)smiNode2->oid,
                            smiGetNextElement(smiElement) ? ", " : "");
                else
                    sprintf(id, "%s%s", smiNode2->name,
                            smiGetNextElement(smiElement) ? ", " : "");
                fprintWrapped(f, 2+INDENTVALUE, id, 0);
                xfree(id);
            }
            fprint(f, " }\n");
        } else {
        /* No SUBJECT-CATEGORIES entry was present, add one */
            fprint(f, "all } -- added by smidump\n");
        }
	fprintSegment(f, INDENT, "LAST-UPDATED", INDENTVALUE, 0);
	smiRevision = smiGetFirstRevision(smiModule);
	if (smiRevision)
	    fprint(f, "\"%s\"\n", getTimeString(smiRevision->date));
	else
	    fprint(f, "\"197001010000Z\"\n");
	fprintSegment(f, INDENT, "ORGANIZATION", INDENTVALUE, 0);
	fprint(f, "\n");
	fprintMultilineString(f, smiModule->organization, 0);
	fprint(f, "\n");
	fprintSegment(f, INDENT, "CONTACT-INFO", INDENTVALUE, 0);
	fprint(f, "\n");
	fprintMultilineString(f, smiModule->contactinfo, 0);
	fprint(f, "\n");
	fprintSegment(f, INDENT, "DESCRIPTION", INDENTVALUE, 0);
	fprint(f, "\n");
	if (smiModule->description) {
	    fprintMultilineString(f, smiModule->description, 0);
	} else {
	    fprintMultilineString(f, "...", 0);
	}
	fprint(f, "\n");

	for(; smiRevision;
	    smiRevision = smiGetNextRevision(smiRevision)) {
	    if (!smiRevision->description
		|| strcmp(smiRevision->description,
       "[Revision added by libsmi due to a LAST-UPDATED clause.]")) {
		fprintSegment(f, INDENT, "REVISION", INDENTVALUE, 0);
		fprint(f, "\"%s\"\n", getTimeString(smiRevision->date));
		fprintSegment(f, INDENT, "DESCRIPTION", INDENTVALUE,
			      0);
		fprint(f, "\n");
		if (smiRevision->description) {
		    fprintMultilineString(f, smiRevision->description,
					  0);
		} else {
		    fprintMultilineString(f, "...", 0);
		}
		fprint(f, "\n");
	    }
	}

	if (smiNode) {
	    fprintSegment(f, INDENT, "::= ", 0, 0);
            if (!mibtopib)
	        fprint(f, "{ %s }\n\n", getOidString(smiNode, 0));
            else
                fprint(f, "{ ibrmibtopib %d }\n\n",
                       smiNode->oid[smiNode->oidlen - 1]);
	}
	/* TODO: else error */

	fprint(f, "\n");
    }
}



static void fprintTypeDefinitions(FILE *f, SmiModule *smiModule)
{
    SmiType	 *smiType;
    int		 invalid;

    for(smiType = smiGetFirstType(smiModule);
	smiType; smiType = smiGetNextType(smiType)) {
	if (smiType->status == SMI_STATUS_UNKNOWN) {
	    invalid = invalidType(smiType->basetype);
	    if (invalid) {
		fprint(f, "-- %s ::=\n", smiType->name);
	    } else {
		fprint(f, "%s ::=\n", smiType->name);
	    }
	    fprintSegment(f, INDENT, "", 0, invalid);
	    fprint(f, "%s", getTypeString(smiType->basetype,
					  smiGetParentType(smiType)));
	    fprintSubtype(f, smiType, invalid);
	    fprint(f, "\n\n");
	}
    }
}



static void fprintTextualConventions(FILE *f, SmiModule *smiModule)
{
    SmiType	 *smiType;
    int		 invalid;
    
    for(smiType = smiGetFirstType(smiModule);
	smiType; smiType = smiGetNextType(smiType)) {
	if (smiType->status != SMI_STATUS_UNKNOWN) {
	    invalid = invalidType(smiType->basetype);
            fprint(f, "%s ::= TEXTUAL-CONVENTION\n", smiType->name);

	    if (smiType->format) {
		fprintSegment(f, INDENT, "DISPLAY-HINT", INDENTVALUE,
			      invalid);
		fprint(f, "\"%s\"\n", smiType->format);
	    }

	    fprintSegment(f, INDENT, "STATUS", INDENTVALUE,
			  invalid);
	    fprint(f, "%s\n", getStatusString(smiType->status));

	    fprintSegment(f, INDENT, "DESCRIPTION", INDENTVALUE,
			  invalid);
	    fprint(f, "\n");
	    if (smiType->description) {
		fprintMultilineString(f, smiType->description,
				      invalid);
	    } else {
		fprintMultilineString(f, "...", invalid);
	    }
	    fprint(f, "\n");

	    if (smiType->reference) {
		fprintSegment(f, INDENT, "REFERENCE", INDENTVALUE,
			      invalid);
		fprint(f, "\n");
		fprintMultilineString(f, smiType->reference,
				      invalid);
		fprint(f, "\n");
	    }
	    fprintSegment(f, INDENT, "SYNTAX", INDENTVALUE,
			  invalid);
	    fprint(f, "%s",
		   getTypeString(smiType->basetype,
				 smiGetParentType(smiType)));
	    fprintSubtype(f, smiType, invalid);
	    fprint(f, "\n\n");
	}
    }
}



static void fprintObjects(FILE *f, SmiModule *smiModule)
{
    SmiNode	 *smiNode, *rowNode, *colNode, *smiParentNode, *relatedNode;
    SmiType	 *smiType;
    SmiNodekind  nodekinds;
    int		 i, invalid, create, assignement, indentsequence, addinstanceid;
    
    nodekinds =  SMI_NODEKIND_NODE | SMI_NODEKIND_TABLE |
	SMI_NODEKIND_ROW | SMI_NODEKIND_COLUMN | SMI_NODEKIND_SCALAR;
            
    for(smiNode = smiGetFirstNode(smiModule, nodekinds);
	smiNode; smiNode = smiGetNextNode(smiNode, nodekinds)) {

	smiType = smiGetNodeType(smiNode);
	smiParentNode = smiGetParentNode(smiNode);
	
	create = smiParentNode ? smiParentNode->create : 0;
	
	invalid = !smiType ? 0 : invalidType(smiType->basetype);
	assignement = 0;

	if (invalid && silent
	    && (smiNode->nodekind == SMI_NODEKIND_SCALAR
		|| smiNode->nodekind == SMI_NODEKIND_COLUMN)) {
	    continue;
	}

	if (smiNode == smiGetModuleIdentityNode(smiModule)) {
	    continue;
	}
	
	if ((smiNode->nodekind == SMI_NODEKIND_NODE) &&
	    (smiNode->status == SMI_STATUS_UNKNOWN)) {
	    assignement = 1;
	    fprint(f, "%s OBJECT IDENTIFIER\n", smiNode->name);
	} else if (smiNode->nodekind == SMI_NODEKIND_NODE) {
	    fprint(f, "%s OBJECT-IDENTITY\n", smiNode->name);
	} else {
	    if (invalid) {
		fprint(f, "-- %s OBJECT-TYPE\n", smiNode->name);
	    } else {
		fprint(f, "%s OBJECT-TYPE\n", smiNode->name);
	    }
	}

	if ((smiNode->nodekind == SMI_NODEKIND_TABLE) ||
	    (smiNode->nodekind == SMI_NODEKIND_ROW) ||
	    (smiType)) {	    fprintSegment(f, INDENT, "SYNTAX", INDENTVALUE, invalid);
	    if (smiNode->nodekind == SMI_NODEKIND_TABLE) {
		fprint(f, "SEQUENCE OF ");
		rowNode = smiGetFirstChildNode(smiNode);
		if (rowNode) {
		    smiType = smiGetNodeType(rowNode);
		    if (smiType) {
			fprint(f, "%s\n", smiType->name);
		    } else {
			/* guess type name is uppercase row name */
			char *s = getUppercaseString(rowNode->name);
			fprint(f, "%s\n", s);
			xfree(s);
		    }
		    /* TODO: print non-local name qualified */
		} else {
		    fprint(f, "<unknown>\n");
		}
	    } else if (smiNode->nodekind == SMI_NODEKIND_ROW) {
		if (smiType) {
		    fprint(f, "%s\n", smiType->name);
		} else {
		    char *s = getUppercaseString(smiNode->name);
		    /* guess type name is uppercase row name */
		    fprint(f, "%s\n", s);
		    xfree(s);
		}
		/* TODO: print non-local name qualified */
	    } else if (smiType) {
		if (!smiType->name) {
		    /*
		     * an implicitly restricted type.
		     */
		    fprint(f, "%s", getTypeString(smiType->basetype,
						  smiGetParentType(smiType)));
		    fprintSubtype(f, smiType, invalid);
		    fprint(f, "\n");
		} else {
		    fprint(f, "%s\n",
			   getTypeString(smiType->basetype, smiType));
		}
	    }
	}

	if (! assignement && smiNode->nodekind == SMI_NODEKIND_TABLE) {
	    fprintSegment(f, INDENT, "PIB-ACCESS", INDENTVALUE, 0);
	    fprint(f, "%s\n", getAccessString(smiNode->access, 1));
	}

        if (! assignement && smiType && smiType->name &&
            !strcmp(smiType->name, "ReferenceId")) {
            relatedNode = smiGetRelatedNode(smiNode);
            if (relatedNode) {
                fprintSegment(f, INDENT, "PIB-REFERENCES", INDENTVALUE, 0);
                fprint(f, "{ %s }\n", relatedNode->name);
            }
        }

        if (! assignement && smiType && smiType->name &&
            !strcmp(smiType->name, "TagReferenceId")) {
            relatedNode = smiGetRelatedNode(smiNode);
            if (relatedNode) {
                fprintSegment(f, INDENT, "PIB-TAG", INDENTVALUE, 0);
                fprint(f, "{ %s }\n", relatedNode->name);
            }
        }

	if (! assignement) {
	    fprintSegment(f, INDENT, "STATUS", INDENTVALUE, invalid);
	    fprint(f, "%s\n", getStatusString(smiNode->status));
	}
	
	if (! assignement) {
	    fprintSegment(f, INDENT, "DESCRIPTION", INDENTVALUE, invalid);
	    fprint(f, "\n");
	    if (smiNode->description) {
		fprintMultilineString(f, smiNode->description, invalid);
	    } else {
		fprintMultilineString(f, "...", invalid);
	    }
	    fprint(f, "\n");
	}
        
        if (smiNode->nodekind == SMI_NODEKIND_TABLE)
            fprintInstallErrors(f, smiNode, invalid);
	    
	if (! assignement && smiNode->reference) {
	    fprintSegment(f, INDENT, "REFERENCE", INDENTVALUE, invalid);
	    fprint(f, "\n");
	    fprintMultilineString(f, smiNode->reference, invalid);
	    fprint(f, "\n");
	}

	relatedNode = smiGetRelatedNode(smiNode);
	switch (smiNode->indexkind) {
	case SMI_INDEX_INDEX:
	    fprintIndex(f, smiNode, invalid);
	    break;
	case SMI_INDEX_AUGMENT:
	    fprintSegment(f, INDENT, "AUGMENTS", INDENTVALUE, invalid);
	    fprint(f, "{ %s }\n", relatedNode->name);
	    break;
	case SMI_INDEX_SPARSE:
	    fprintSegment(f, INDENT, "EXTENDS", INDENTVALUE, invalid);
	    fprint(f, "{ %s }\n", relatedNode->name);
	    break;
	case SMI_INDEX_UNKNOWN:
	case SMI_INDEX_REORDER:
	case SMI_INDEX_EXPAND:
	    break;
	}
	
        if (smiNode->nodekind == SMI_NODEKIND_ROW)
            fprintUniqueness(f, smiNode, invalid);
        
	if (smiNode->value.basetype != SMI_BASETYPE_UNKNOWN) {
	    fprintSegment(f, INDENT, "DEFVAL", INDENTVALUE, invalid);
	    fprint(f, "{ %s }", getValueString(&smiNode->value, smiType));
	    fprint(f, "\n");
	}

	fprintSegment(f, INDENT, "::= ", 0, invalid);
	fprint(f, "{ %s }\n\n", getOidString(smiNode, 0));

	smiType = smiGetNodeType(smiNode);
        addinstanceid = 0;
	if (smiNode->nodekind == SMI_NODEKIND_ROW) {
            if (mibtopib)
                addinstanceid = 1;
	    if (smiType) {
		fprint(f, "%s ::= SEQUENCE {", smiType->name);
	    } else {
		/* guess type name is uppercase row name */
		char *s = getUppercaseString(smiNode->name);
		fprint(f, "%s ::= SEQUENCE {", s);
		xfree(s);
	    }
	    /* Find the last valid node in this sequence. We need it
	     * to suppress its trailing comma. Compute the longest
	     * column name so that we can adjust the indentation of
	     * the type names in the SEQUENCE definition. */
	    for(indentsequence = 0, colNode = smiGetFirstChildNode(smiNode);
		colNode;
		colNode = smiGetNextChildNode(colNode)) {
		int len = strlen(colNode->name);
		if (len > indentsequence) indentsequence = len;
		smiType = smiGetNodeType(colNode);
		if (smiType && !invalidType(smiType->basetype)) {
		    relatedNode = colNode;
		}
	    }
            if (mibtopib) {
                int len = strlen(smiParentNode->name) + 10;
                if (len > 64)
                  len = 64;
                indentsequence = len;
            }
	    if (relatedNode) relatedNode = smiGetNextChildNode(relatedNode);
	    indentsequence = (2*INDENT + indentsequence + 1) / INDENT * INDENT;
	    /* TODO: non-local name? */
	    for(i = 0, invalid = 0, colNode = smiGetFirstChildNode(smiNode);
		colNode;
		colNode = smiGetNextChildNode(colNode)) {
		if (! invalid || ! silent) {
		    if (i && (relatedNode != colNode)) {
			fprint(f, ",");
		    }
		    fprint(f, "\n");
		}
		
		smiType = smiGetNodeType(colNode);
		invalid = (smiType == NULL) || invalidType(smiType->basetype);

		if (! invalid || ! silent) {
		    fprintSegment(f, INDENT, colNode->name, indentsequence,
				  invalid);
		    if (smiType && smiType->decl == SMI_DECL_IMPLICIT_TYPE) {
			fprint(f, "%s", getTypeString(smiType->basetype,
					      smiGetParentType(smiType)));
		    } else if (smiType) {
			fprint(f, "%s", getTypeString(smiType->basetype,
					      smiGetNodeType(colNode)));
		    } else {
			fprint(f, "<unknown>");
		    }
		}
		i++;
	    }
            if (mibtopib) {
                size_t len = strlen(smiParentNode->name);
                int maxid;
                char *instanceId = xmalloc(len + 11);
                strcpy(instanceId, smiParentNode->name);
                if (len > 54)
                  len = 54;
                strcpy(&instanceId[len], "InstanceId");
                fprint(f, ",\n");
                fprintSegment(f, INDENT, instanceId, indentsequence, 0);
                fprint(f, "InstanceId\n}\n\n");
                
                fprint(f, "%s OBJECT-TYPE\n", instanceId);
	        fprintSegment(f, INDENT, "SYNTAX", INDENTVALUE, 0);
                fprint(f, "InstanceId\n");
                fprintSegment(f, INDENT, "STATUS", INDENTVALUE, 0);
                fprint(f, "current\n");
                fprintSegment(f, INDENT, "DESCRIPTION", INDENTVALUE, 0);
                fprint(f, "\n");
		fprintMultilineString(f, "Added by smidump for automatic " \
                                      "MIB to PIB conversion.", 0);
                fprint(f, "\n");
	        fprintSegment(f, INDENT, "::= ", 0, 0);
                for (maxid = 0, colNode = smiGetFirstChildNode(smiNode);
                     colNode; colNode = smiGetNextChildNode(colNode))
                     if (colNode->oidlen &&
                         (colNode->oid[colNode->oidlen - 1] > maxid))
                         maxid = colNode->oid[colNode->oidlen - 1];
	        fprint(f, "{ %s %d }\n\n",
                       smiGetFirstChildNode(smiParentNode)->name, 
                       (maxid + 1) > 128 ? (maxid + 1) : 128);
                xfree(instanceId);
            } else
 	        fprint(f, "\n}\n\n");
	}
    }
}



static void fprintGroups(FILE *f, SmiModule *smiModule)
{
    SmiNode	 *smiNode;
    SmiElement   *smiElement;
    int          j, objectGroup = 0;
    
    for (smiNode = smiGetFirstNode(smiModule, SMI_NODEKIND_GROUP);
	 smiNode; smiNode = smiGetNextNode(smiNode, SMI_NODEKIND_GROUP)) {

	objectGroup = isObjectGroup(smiNode);

	if (!objectGroup) {
	    fprint(f, "%s OBJECT IDENTIFIER\n", smiNode->name);
	    fprintSegment(f, INDENT, "::= ", 0, 0);
	    fprint(f, "{ %s }\n\n", getOidString(smiNode, 0));
	}

	if (!objectGroup) {
	    continue;
	}

	fprint(f, "%s %s\n", smiNode->name, "OBJECT-GROUP");
	fprintSegment(f, INDENT, "OBJECTS", INDENTVALUE, 0);
	fprint(f, "{ ");
	for (j = 0, smiElement = smiGetFirstElement(smiNode);
	     smiElement;
	     j++, smiElement = smiGetNextElement(smiElement)) {
	    if (j) {
		fprint(f, ", ");
	    }
	    fprintWrapped(f, INDENTVALUE + 2,
			  smiGetElementNode(smiElement)->name, 0);
	    /* TODO: non-local name if non-local */
	} /* TODO: empty? -> print error */
	fprint(f, " }\n");

	fprintSegment(f, INDENT, "STATUS", INDENTVALUE, 0);
	fprint(f, "%s\n", getStatusString(smiNode->status));

	fprintSegment(f, INDENT, "DESCRIPTION", INDENTVALUE, 0);
	fprint(f, "\n");
	if (smiNode->description) {
	    fprintMultilineString(f, smiNode->description, 0);
	} else {
	    fprintMultilineString(f, "...", 0);
	}
	fprint(f, "\n");

	if (smiNode->reference) {
	    fprintSegment(f, INDENT, "REFERENCE", INDENTVALUE, 0);
	    fprint(f, "\n");
	    fprintMultilineString(f, smiNode->reference, 0);
	    fprint(f, "\n");
	}

	fprintSegment(f, INDENT, "::= ", 0, 0);
	fprint(f, "{ %s }\n\n", getOidString(smiNode, 0));
    }
}



static void fprintModuleCompliances(FILE *f, SmiModule *smiModule)
{
    SmiNode	  *smiNode, *smiNode2;
    SmiModule     *smiModule2;
    SmiType	  *smiType;
    SmiOption	  *smiOption;
    SmiRefinement *smiRefinement;
    SmiElement    *smiElement;
    char	  *module;
    char	  *done = NULL; /* "+" separated list of module names */
    char	  s[1024];
    int		  j;

    for (smiNode = smiGetFirstNode(smiModule, SMI_NODEKIND_COMPLIANCE);
	 smiNode; smiNode = smiGetNextNode(smiNode, SMI_NODEKIND_COMPLIANCE)) {
	fprint(f, "%s MODULE-COMPLIANCE\n", smiNode->name);

	fprintSegment(f, INDENT, "STATUS", INDENTVALUE, 0);
	fprint(f, "%s\n", getStatusString(smiNode->status));

	fprintSegment(f, INDENT, "DESCRIPTION", INDENTVALUE, 0);
	fprint(f, "\n");
	if (smiNode->description) {
	    fprintMultilineString(f, smiNode->description, 0);
	} else {
	    fprintMultilineString(f, "...", 0);
	}
	fprint(f, "\n");

	if (smiNode->reference) {
	    fprintSegment(f, INDENT, "REFERENCE", INDENTVALUE, 0);
	    fprint(f, "\n");
	    fprintMultilineString(f, smiNode->reference, 0);
	    fprint(f, "\n");
	}

	/* `this module' always first */
	done = xstrdup("+");
	for (module = smiModule->name; module; ) {

	    fprint(f, "\n");
	    fprintSegment(f, INDENT, "MODULE", INDENTVALUE, 0);
	    if (strlen(module) && strcmp(smiModule->name, module)) {
		fprint(f, "%s\n", module);
	    } else {
		fprint(f, "-- this module\n");
	    }

	    for (j = 0, smiElement = smiGetFirstElement(smiNode);
		 smiElement;
		 smiElement = smiGetNextElement(smiElement)) {
		if (!strcmp(smiGetNodeModule(smiGetElementNode(smiElement))->name, module)) {
		    if (j) {
			fprint(f, ", ");
		    } else {
			fprint(f, "\n");
			fprintSegment(f, 2 * INDENT, "MANDATORY-GROUPS",
				      INDENTVALUE, 0);
			fprint(f, "{ ");
		    }
		    j++;
		    fprintWrapped(f, INDENTVALUE + 2,
				  smiGetElementNode(smiElement)->name,
				  0);
		}
	    }
	    if (j) {
		fprint(f, " }\n");
	    }

	    for(smiOption = smiGetFirstOption(smiNode); smiOption;
		smiOption = smiGetNextOption(smiOption)) {
		smiNode2 = smiGetOptionNode(smiOption);
		smiModule2 = smiGetNodeModule(smiNode2);
		if (!strcmp(smiModule2->name, module)) {
		    fprint(f, "\n");
		    fprintSegment(f, 2 * INDENT, "GROUP",
				  INDENTVALUE, 0);
		    fprint(f, "%s\n", smiNode2->name);
		    fprintSegment(f, 2 * INDENT, "DESCRIPTION",
				  INDENTVALUE, 0);
		    fprint(f, "\n");
		    if (smiOption->description) {
			fprintMultilineString(f, smiOption->description,
					      0);
		    } else {
			fprintMultilineString(f, "...", 0);
		    }
		    fprint(f, "\n");
		}
	    }

	    for(smiRefinement = smiGetFirstRefinement(smiNode);
		smiRefinement;
		smiRefinement = smiGetNextRefinement(smiRefinement)) {
		smiNode2 = smiGetRefinementNode(smiRefinement);
		smiModule2 = smiGetNodeModule(smiNode2);
		if (!strcmp(smiModule2->name, module)) {
		    fprint(f, "\n");
		    fprintSegment(f, 2 * INDENT, "OBJECT",
				  INDENTVALUE, 0);
		    fprint(f, "%s\n", smiNode2->name);

		    smiType = smiGetRefinementType(smiRefinement);
		    if (smiType) {
			fprintSegment(f, 2 * INDENT, "SYNTAX", INDENTVALUE,
				      0);
			fprint(f, "%s",
			       getTypeString(smiType->basetype,
					     smiGetParentType(smiType)));
			fprintSubtype(f, smiType, 0);
			fprint(f, "\n");
		    }

/*		    if ((smiRefinement->access == SMI_ACCESS_NOTIFY) ||
                        (smiRefinement->access >= SMI_REPORT_ONLY)) {*/
                    if (smiRefinement->access != SMI_ACCESS_UNKNOWN) {
			fprintSegment(f, 2 * INDENT, "PIB-MIN-ACCESS",
				      INDENTVALUE, 0);
			fprint(f, "%s\n",
			       getAccessString(smiRefinement->access, 0));
			/* we assume, that read-create does not appear in
			 * an OT refinement.
			 */
		    }
		    fprintSegment(f, 2 * INDENT, "DESCRIPTION",
				  INDENTVALUE, 0);
		    fprint(f, "\n");
		    if (smiRefinement->description) {
			fprintMultilineString(f,
					      smiRefinement->description,
					      0);
		    } else {
			fprintMultilineString(f, "...", 0);
		    }
		    fprint(f, "\n");
		}
	    }

	    /*
	     * search the next module name in the list of mandatory
	     * groups, optional groups and refinements.
	     */
	    done = xrealloc(done,
			    strlen(done)+strlen(module)+2*sizeof(char));
	    strcat(done, module);
	    strcat(done, "+");
	    module = NULL;
	    for (smiElement = smiGetFirstElement(smiNode);
		 smiElement;
		 smiElement = smiGetNextElement(smiElement)) {
		sprintf(s, "+%s+",
		    smiGetNodeModule(smiGetElementNode(smiElement))->name);
		if ((!strstr(done, s))) {
		    module =
		     smiGetNodeModule(smiGetElementNode(smiElement))->name;
		    break;
		}
	    }
	    if (!module) {
		; /* TODO: search in options list */
	    }
	    if (!module) {
		; /* TODO: search in refinements list */
	    }
	}

	fprint(f, "\n");
	fprintSegment(f, INDENT, "::= ", 0, 0);
	fprint(f, "{ %s }\n\n", getOidString(smiNode, 0));
    }
    xfree(done);
    
    if (mibtopib) {
        char *newCompliance = xmalloc(65);
        char *newGroup = xmalloc(65);
        char *newId = xmalloc(65);
        int len, maxid;
        
        for (maxid = 0, smiNode2 = smiGetFirstChildNode(smiNode);
             smiNode2; smiNode2 = smiGetNextChildNode(smiNode2))
             if (smiNode2->oidlen &&
                 (smiNode2->oid[smiNode2->oidlen - 1] > maxid))
                 maxid = smiNode2->oid[smiNode2->oidlen - 1];
        maxid++;

        smiNode = smiGetModuleIdentityNode(smiModule);
        if (!smiNode || !newCompliance || !newGroup)
            return;
        len = strlen(smiNode->name);
        
        memset(newId, 0, 65);
        strncpy(newId, smiNode->name, 46);
        strcat(newId + (len > 46 ? 46 : len), "MIBtoPIBCompliance");

        memset(newCompliance, 0, 65);
        strncpy(newCompliance, smiNode->name, 46);
        strcat(newCompliance + (len > 46 ? 46 : len), "MIBtoPIBModuleComp");

        memset(newGroup, 0, 65);
        strncpy(newGroup, smiNode->name, 51);
        strcat(newGroup + (len > 51 ? 51 : len), "MIBtoPIBGroup");

        fprintf(f, "-- The following three items were added in order " \
                "to create a RFC compliant\n-- SPPI module. They do not " \
                "provide any usable content.\n-- %s\n-- %s\n-- %s\n\n",
                newId, newCompliance, newGroup);
        
        fprint(f, "%s OBJECT-IDENTITY\n", newId);
        fprintSegment(f, INDENT, "STATUS", INDENTVALUE, 0);
        fprint(f, "current\n");
	fprintSegment(f, INDENT, "DESCRIPTION", INDENTVALUE, 0);
	fprint(f, "\n");
        fprintMultilineString(f, "Added by smidump for automatic " \
                              "MIB to PIB conversion.", 0);
	fprint(f, "\n");
	fprintSegment(f, INDENT, "::= ", 0, 0);
	fprint(f, "{ %s %d }\n\n", smiNode->name, (maxid > 128 ? maxid : 128));
        
        fprint(f, "%s MODULE-COMPLIANCE\n", newCompliance);
	fprintSegment(f, INDENT, "STATUS", INDENTVALUE, 0);
	fprint(f, "current\n");
	fprintSegment(f, INDENT, "DESCRIPTION", INDENTVALUE, 0);
	fprint(f, "\n");
        fprintMultilineString(f, "Added by smidump for automatic " \
                              "MIB to PIB conversion.", 0);
	fprint(f, "\n");
        
        fprintSegment(f, INDENT, "MODULE", INDENTVALUE, 0);
        fprint(f, "-- this module\n");
        
        fprintSegment(f, 2 * INDENT, "MANDATORY-GROUPS", INDENTVALUE, 0);
        fprint(f, "{ ");
        
	fprintWrapped(f, INDENTVALUE + 2, newGroup, 0);
        fprint(f, "}\n");
        
	fprintSegment(f, INDENT, "::= ", 0, 0);
	fprint(f, "{ %s 1 }\n\n", newId);

	fprint(f, "%s OBJECT-GROUP\n", newGroup);
	fprintSegment(f, INDENT, "OBJECTS", INDENTVALUE, 0);
	fprint(f, "{ ");
        for (len=0, smiNode2 = smiGetFirstNode(smiModule, SMI_NODEKIND_COLUMN);
            smiNode2; len = 1,
            smiNode2 = smiGetNextNode(smiNode2, SMI_NODEKIND_COLUMN)) {
            if (len)
                fprint(f, ", ");
	    fprintWrapped(f, INDENTVALUE + 2, smiNode2->name, 0);
        }
        for (smiNode2 = smiGetFirstNode(smiModule, SMI_NODEKIND_ROW);
             smiNode2; smiNode2 = smiGetNextNode(smiNode2, SMI_NODEKIND_ROW)) {
            SmiNode *smiParentNode = smiGetParentNode(smiNode2);
            size_t len = strlen(smiParentNode->name);
            char *instanceId = xmalloc(len + 11);

            strcpy(instanceId, smiParentNode->name);
            if (len > 54)
              len = 54;
            strcpy(&instanceId[len], "InstanceId");
            if (len)
                fprint(f, ", ");
	    fprintWrapped(f, INDENTVALUE + 2, instanceId, 0);
            xfree(instanceId);
        }

	fprint(f, " }\n");

	fprintSegment(f, INDENT, "STATUS", INDENTVALUE, 0);
	fprint(f, "current\n");

	fprintSegment(f, INDENT, "DESCRIPTION", INDENTVALUE, 0);
	fprint(f, "\n");
	fprintMultilineString(f, "Added by smidump for automatic" \
                              "MIB to PIB conversion.", 0);
	fprint(f, "\n");

	fprintSegment(f, INDENT, "::= ", 0, 0);
	fprint(f, "{ %s 2 }\n\n", newId);

        xfree(newCompliance);
        xfree(newGroup);
    }
}



static void doDumpSppi(FILE *f, SmiModule *smiModule)
{
    if (smiModule->language != SMI_LANGUAGE_SPPI) /* MIB to PIB conversion */
        mibtopib = 1;
    else
        mibtopib = 0;

    createImportList(smiModule);
    
    fprint(f, "--\n");
    fprint(f, "-- This SPPI module has been generated by smidump "
	   SMI_VERSION_STRING ". Do not edit.\n");
    fprint(f, "--\n\n");
    fprint(f, "%s%s PIB-DEFINITIONS ::= BEGIN\n\n", smiModule->name,
           mibtopib ? "-PIB" : "");
	
    fprintImports(f);
    fprintModuleIdentity(f, smiModule);
    fprintTypeDefinitions(f, smiModule);
    fprintTextualConventions(f, smiModule);
    fprintObjects(f, smiModule);
    fprintGroups(f, smiModule);
    fprintModuleCompliances(f, smiModule);
    
    fprint(f, "END -- end of module %s.\n", smiModule->name);

    freeImportList();
}


static void dumpSppi(int modc, SmiModule **modv, int flags, char *output)
{
    int  i;
    FILE *f = stdout;

    silent = (flags & SMIDUMP_FLAG_SILENT);

    if (output) {
	f = fopen(output, "w");
	if (!f) {
	    fprintf(stderr, "smidump: cannot open %s for writing: ", output);
	    perror(NULL);
	    exit(1);
	}
    }

    for (i = 0; i < modc; i++) {
	doDumpSppi(f, modv[i]);
    }

    if (fflush(f) || ferror(f)) {
	perror("smidump: write error");
	exit(1);
    }
    
    if (output) {
	fclose(f);
    }
}



void initSppi()
{
    static SmidumpDriver driver = {
	"sppi",
	dumpSppi,
	0,
	SMIDUMP_DRIVER_CANT_UNITE,
	"SPPI (RFC 3159)",
	NULL,
	NULL
    };
    
    smidumpRegisterDriver(&driver);
}
