/*
 * dump-sming.c --
 *
 *      Operations to dump SMIng module information.
 *
 * Copyright (c) 1999 Frank Strauss, Technical University of Braunschweig.
 *
 * See the file "COPYING" for information on usage and redistribution
 * of this file, and for a DISCLAIMER OF ALL WARRANTIES.
 *
 * @(#) $Id: dump-sming.c 8090 2008-04-18 12:56:29Z strauss $
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



#define  INDENT		2    /* indent factor */
#define  INDENTVALUE	20   /* column to start values, except multiline */
#define  INDENTTEXTS	4    /* column to start multiline texts */
#define  INDENTMAX	64   /* max column to fill, break lines otherwise */



static char *excludeType[] = {
    "ObjectSyntax",
    "SimpleSyntax",
    "ApplicationSyntax",
    NULL };
    
static char *convertType[] = {
    "INTEGER",             "Integer32",
    "OCTET STRING",        "OctetString",
    "OBJECT IDENTIFIER",   "Pointer",
    
    "Gauge",               "Gauge32",
    "Counter",             "Counter32",
    "NetworkAddress",      "IpAddress", /* ??? */
    
    NULL, NULL };

static char *convertImport[] = {
    "SNMPv2-SMI",   "MODULE-IDENTITY",    NULL, NULL,
    "SNMPv2-SMI",   "OBJECT-IDENTITY",    NULL, NULL,
    "SNMPv2-SMI",   "OBJECT-TYPE",        NULL, NULL,
    "SNMPv2-SMI",   "NOTIFICATION-TYPE",  NULL, NULL,
    "SNMPv2-SMI",   "ObjectName",         NULL, NULL,
    "SNMPv2-SMI",   "NotificationName",   NULL, NULL,
    "SNMPv2-SMI",   "ObjectSyntax",       NULL, NULL,
    "SNMPv2-SMI",   "SimpleSyntax",       NULL, NULL,
    "SNMPv2-SMI",   "Integer32",          NULL, NULL,
    "SNMPv2-SMI",   "Unsigned32",         NULL, NULL,
    "SNMPv2-SMI",   "ApplicationSyntax",  NULL, NULL,
    "SNMPv2-SMI",   "IpAddress",          "IRTF-NMRG-SMING", "IpAddress",
    "SNMPv2-SMI",   "Counter32",          "IRTF-NMRG-SMING", "Counter32",
    "SNMPv2-SMI",   "TimeTicks",          "IRTF-NMRG-SMING", "TimeTicks",
    "SNMPv2-SMI",   "Opaque",             "IRTF-NMRG-SMING", "Opaque",
    "SNMPv2-SMI",   "Counter64",          "IRTF-NMRG-SMING", "Counter64",
    "SNMPv2-SMI",   "Gauge32",            "IRTF-NMRG-SMING", "Gauge32",
    "SNMPv2-SMI",   "mib-2",              "IRTF-NMRG-SMING-SNMP", "mib-2",

    "SNMPv2-TC",   "TimeStamp",           "IRTF-NMRG-SMING", "TimeStamp",
    "SNMPv2-TC",   "TimeInterval",        "IRTF-NMRG-SMING", "TimeInterval",
    "SNMPv2-TC",   "DateAndTime",         "IRTF-NMRG-SMING", "DateAndTime",
    "SNMPv2-TC",   "TruthValue",          "IRTF-NMRG-SMING", "TruthValue",
    "SNMPv2-TC",   "PhysAddress",         "IRTF-NMRG-SMING", "PhysAddress",
    "SNMPv2-TC",   "MacAddress",          "IRTF-NMRG-SMING", "MacAddress",
    "SNMPv2-TC",   "DisplayString",       "IRTF-NMRG-SMING", "DisplayString255",
    "SNMPv2-TC",   "TestAndIncr",         "IRTF-NMRG-SMING-SNMP", "TestAndIncr",
    "SNMPv2-TC",   "AutonomousType",      "IRTF-NMRG-SMING-SNMP", "AutonomousType",
    "SNMPv2-TC",   "VariablePointer",     "IRTF-NMRG-SMING-SNMP", "VariablePointer",
    "SNMPv2-TC",   "RowPointer",          "IRTF-NMRG-SMING-SNMP", "RowPointer",
    "SNMPv2-TC",   "RowStatus",           "IRTF-NMRG-SMING-SNMP", "RowStatus",
    "SNMPv2-TC",   "StorageType",         "IRTF-NMRG-SMING-SNMP", "StorageType",
    "SNMPv2-TC",   "TDomain",             "IRTF-NMRG-SMING-SNMP", "TDomain",
    "SNMPv2-TC",   "TAddress",            "IRTF-NMRG-SMING-SNMP", "TAddress",
    "SNMPv2-SMI",   NULL,                 "IRTF-NMRG-SMING", NULL,
    "SNMPv2-TC",    "TEXTUAL-CONVENTION", NULL, NULL,
    "SNMPv2-TC",    NULL,                 "IRTF-NMRG-SMING-TYPES", NULL,
    "SNMPv2-CONF",  "OBJECT-GROUP",       NULL, NULL,
    "SNMPv2-CONF",  "NOTIFICATION-GROUP", NULL, NULL,
    "SNMPv2-CONF",  "MODULE-COMPLIANCE",  NULL, NULL,
    "SNMPv2-CONF",  "AGENT-CAPABILITIES", NULL, NULL,
 
    "RFC1155-SMI",  "OBJECT-TYPE",        NULL, NULL,
    "RFC1155-SMI",  "ObjectName",         NULL, NULL,
    "RFC1155-SMI",  "ObjectSyntax",       NULL, NULL,
    "RFC1155-SMI",  "SimpleSyntax",       NULL, NULL,
    "RFC1155-SMI",  "ApplicationSyntax",  NULL, NULL,
    "RFC1155-SMI",  "Gauge",              "IRTF-NMRG-SMING-TYPES", "Gauge32",
    "RFC1155-SMI",  "Counter",            "IRTF-NMRG-SMING-TYPES", "Counter32",
    "RFC1155-SMI",  "TimeTicks",          "IRTF-NMRG-SMING-TYPES", "TimeTicks",
    "RFC1155-SMI",  "IpAddress",          "IRTF-NMRG-SMING-TYPES", "IpAddress",
    "RFC1155-SMI",  "NetworkAddress",     NULL, NULL, /* ??? */
    "RFC1155-SMI",  "Opaque",             "IRTF-NMRG-SMING", "Opaque",
    "RFC1155-SMI",  NULL,                 "IRTF-NMRG-SMING", NULL,
    "RFC1158-MIB",  "DisplayString",      "IRTF-NMRG-SMING", "DisplayString255",
    "RFC-1212",     "OBJECT-TYPE",        NULL, NULL,
    "RFC1213-MIB",  "mib-2",              "IRTF-NMRG-SMING-SNMP", "mib-2",
    "RFC1213-MIB",  "system",             "SNMPv2-MIB", "system",
    "RFC1213-MIB",  "interfaces",         "IF-MIB", "interfaces",
/*  "RFC1213-MIB",  "at",                 "RFC1213-MIB", "at", */
    "RFC1213-MIB",  "ip",                 "IP-MIB", "ip",
    "RFC1213-MIB",  "icmp",               "IP-MIB", "icmp",
    "RFC1213-MIB",  "tcp",                "TCP-MIB", "tcp",
    "RFC1213-MIB",  "udp",                "UDP-MIB", "udp",
/*  "RFC1213-MIB",  "egp",                "RFC1213-MIB", "egp", */
    "RFC1213-MIB",  "transmission",       "SNMPv2-SMI", "transmission",
    "RFC1213-MIB",  "snmp",               "SNMPv2-MIB", "snmp",
    "RFC1213-MIB",  "sysDescr",           "SNMPv2-MIB", "sysDescr",
    "RFC1213-MIB",  "sysObjectID",        "SNMPv2-MIB", "sysObjectID",
    "RFC1213-MIB",  "sysUpTime",          "SNMPv2-MIB", "sysUpTime",
    "RFC1213-MIB",  "ifIndex",            "IF-MIB", "ifIndex",
/* TODO ...many more objects from RFC1213-MIB.. */    
    "RFC1213-MIB",  "DisplayString",      "IRTF-NMRG-SMING", "DisplayString255",
    "RFC1213-MIB",  "PhysAddress",	  "IRTF-NMRG-SMING", "PhysAddress",
    "RFC-1215",     "TRAP-TYPE",          NULL, NULL,                          



    
    /* TODO: how to convert more SMIv1 information? */

    NULL, NULL, NULL, NULL };

static int current_column = 0;
static int silent = 0;


/*
 * Structure used to build a list of imported types.
 */


typedef struct Import {
    char          *module;
    char          *name;
    struct Import *nextPtr;
} Import;

static Import *importList = NULL;



static char *getStringStatus(SmiStatus status)
{
    return
	(status == SMI_STATUS_CURRENT)     ? "current" :
	(status == SMI_STATUS_DEPRECATED)  ? "deprecated" :
	(status == SMI_STATUS_OBSOLETE)    ? "obsolete" :
	(status == SMI_STATUS_MANDATORY)   ? "current" :
	(status == SMI_STATUS_OPTIONAL)    ? "current" :
					     "<unknown>";
}



static char *getAccessString(SmiAccess access)
{
    return
	(access == SMI_ACCESS_NOT_ACCESSIBLE) ? "noaccess" :
	(access == SMI_ACCESS_NOTIFY)	      ? "notifyonly" :
	(access == SMI_ACCESS_READ_ONLY)      ? "readonly" :
	(access == SMI_ACCESS_READ_WRITE)     ? "readwrite" :
						"<unknown>";
}



static char *getStringBasetype(SmiBasetype basetype)
{
    return
        (basetype == SMI_BASETYPE_UNKNOWN)           ? "<unknown>" :
        (basetype == SMI_BASETYPE_OCTETSTRING)       ? "OctetString" :
        (basetype == SMI_BASETYPE_OBJECTIDENTIFIER)  ? "ObjectIdentifier" :
        (basetype == SMI_BASETYPE_UNSIGNED32)        ? "Unsigned32" :
        (basetype == SMI_BASETYPE_INTEGER32)         ? "Integer32" :
        (basetype == SMI_BASETYPE_UNSIGNED64)        ? "Unsigned64" :
        (basetype == SMI_BASETYPE_INTEGER64)         ? "Integer64" :
        (basetype == SMI_BASETYPE_FLOAT32)           ? "Float32" :
        (basetype == SMI_BASETYPE_FLOAT64)           ? "Float64" :
        (basetype == SMI_BASETYPE_FLOAT128)          ? "Float128" :
        (basetype == SMI_BASETYPE_ENUM)              ? "Enumeration" :
        (basetype == SMI_BASETYPE_BITS)              ? "Bits" :
                                                   "<unknown>";
}



static char *getStringTime(time_t t)
{
    static char   s[27];
    struct tm	  *tm;

    tm = gmtime(&t);
    sprintf(s, "%04d-%02d-%02d %02d:%02d",
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
    
    if ((!typeModule) && (typeName) &&
	(basetype != SMI_BASETYPE_ENUM) &&
	(basetype != SMI_BASETYPE_BITS)) {
	for(i=0; convertType[i]; i += 2) {
	    if (!strcmp(typeName, convertType[i])) {
		return convertType[i+1];
	    }
	}
    }

    if ((!typeModule) || (!strlen(typeModule)) || (!typeName)) {
	if (basetype == SMI_BASETYPE_ENUM) {
	    return "Enumeration";
	}
	if (basetype == SMI_BASETYPE_BITS) {
	    return "Bits";
	}
    }
	
    if (!typeName) {
	return getStringBasetype(basetype);
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
	sprintf(append, ".%u%s", parentNode->oid[parentNode->oidlen-1], s);

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
	if (i) strcat(s, ".");
	sprintf(&s[strlen(s)], "%u", smiNode->oid[i]);
    }
    return s;
}



static Import* addImport(char *module, char *name)
{
    Import **import, *newImport;
    int i;
    
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
    SmiModule   *smiTypeModule;
    SmiNodekind kind = SMI_NODEKIND_SCALAR | SMI_NODEKIND_COLUMN;
    SmiImport   *smiImport;
    
    for (smiNode = smiGetFirstNode(smiModule, kind);
	 smiNode;
	 smiNode = smiGetNextNode(smiNode, kind)) {

	smiType = smiGetNodeType(smiNode);
	if (smiType && (smiType->decl == SMI_DECL_IMPLICIT_TYPE)) {
	    smiType = smiGetParentType(smiType);
	}
	if (smiType) {
	    smiTypeModule = smiGetTypeModule(smiType);
	    if (smiTypeModule &&
		strcmp(smiTypeModule->name, smiModule->name)) {
		if (strlen(smiTypeModule->name)) {
		    addImport(smiTypeModule->name, smiType->name);
		}
	    }
	    if (smiType->basetype == SMI_BASETYPE_INTEGER32) {
		addImport("SNMPv2-SMI", "Integer32");
	    }
	}
    }

    for (smiImport = smiGetFirstImport(smiModule); smiImport;
	 smiImport = smiGetNextImport(smiImport)) {
	if (islower((int) smiImport->name[0]) ||
	    (smiImport->module && !strcmp(smiImport->module, "SNMPv2-SMI")) ||
	    (smiImport->module && !strcmp(smiImport->module, "SNMPv2-TC"))) {
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
    char    s[200];
    char    *p;
    
    va_start(ap, fmt);
#ifdef HAVE_VSNPRINTF
    current_column += vsnprintf(s, sizeof(s), fmt, ap);
#else
    current_column += vsprintf(s, fmt, ap);	/* buffer overwrite */
#endif
    va_end(ap);

    fputs(s, f);

    if ((p = strrchr(s, '\n'))) {
	current_column = strlen(p) - 1;
    }
}



static void fprintSegment(FILE *f, int column, char *string, int length)
{
    fprint(f, "%*c%s", column, ' ', string);
    if (length) {
	fprint(f, "%*c", length - strlen(string) - column, ' ');
    }
}



static void fprintWrapped(FILE *f, int column, char *string)
{
    if ((current_column + strlen(string)) > INDENTMAX) {
	putc('\n', f);
	current_column = 0;
	fprintSegment(f, column, "", 0);
    }
    fprint(f, "%s", string);
}



static void fprintMultilineString(FILE *f, int column, const char *s)
{
    int i, len;
    
    fprintSegment(f, column - 1 + INDENTTEXTS, "\"", 0);
    if (s) {
	len = strlen(s);
	for (i=0; i < len; i++) {
	    putc(s[i], f);
	    current_column++;
	    if (s[i] == '\n') {
		current_column = 0;
		fprintSegment(f, column + INDENTTEXTS, "", 0);
	    }
	}
    }
    putc('\"', f);
    current_column++;
}



static char *getValueString(SmiValue *valuePtr, SmiType *typePtr)
{
    static char    s[100];
    char           ss[9];
    int		   n;
    unsigned int   i;
    SmiNamedNumber *nn;
    
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
    sprintf(s, "%G", valuePtr->value.float32);
    break;
    case SMI_BASETYPE_FLOAT64:
    sprintf(s, "%lG", valuePtr->value.float64);
    break;
    case SMI_BASETYPE_FLOAT128:
    sprintf(s, "%LG", valuePtr->value.float128);
    break;
	break;
    case SMI_BASETYPE_ENUM:
	for (nn = smiGetFirstNamedNumber(typePtr); nn;
	     nn = smiGetNextNamedNumber(nn)) {
	    if (nn->value.value.integer32 == valuePtr->value.integer32)
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
            sprintf(s, "0x%*s", 2 * valuePtr->len, "");
            for (i=0; i < valuePtr->len; i++) {
                sprintf(ss, "%02x", valuePtr->value.ptr[i]);
                strncpy(&s[2+2*i], ss, 2);
            }
	}
	break;
    case SMI_BASETYPE_BITS:
	sprintf(s, "(");
	for (i = 0, n = 0; i < valuePtr->len * 8; i++) {
	    if (valuePtr->value.ptr[i/8] & (1 << (7-(i%8)))) {
		if (n)
		    sprintf(&s[strlen(s)], ", ");
		n++;
		for (nn = smiGetFirstNamedNumber(typePtr); nn;
		     nn = smiGetNextNamedNumber(nn)) {
		    /* if (nn->value.value.unsigned64 == ((i/8)*8 + (7-(i%8)))) */
		    if (nn->value.value.unsigned64 == i)
			break;
		}
		if (nn) {
		    sprintf(&s[strlen(s)], "%s", nn->name);
		} else {
		    sprintf(&s[strlen(s)], "%d", i);
		}
	    }
	}
	sprintf(&s[strlen(s)], ")");
	break;
    case SMI_BASETYPE_UNKNOWN:
	break;
    case SMI_BASETYPE_OBJECTIDENTIFIER:
	/*nodePtr = smiGetNodeByOID(valuePtr->len, valuePtr->value.oid);
	if (nodePtr) {
	    sprintf(s, "%s", nodePtr->name);
	} else {
	    strcpy(s, "");
	    for (i=0; i < valuePtr->len; i++) {
		if (i) strcat(s, ".");
		sprintf(&s[strlen(s)], "%u", valuePtr->value.oid[i]);
	    }
	}*/
	sprintf(s, "%s", typePtr->value.value.ptr);
	break;
	case SMI_BASETYPE_POINTER:
	/*nodePtr = smiGetNodeByOID(valuePtr->len, valuePtr->value.oid);
	if (nodePtr) {
	    sprintf(s, "%s", nodePtr->name);
	} else {
	    strcpy(s, "");
	    for (i=0; i < valuePtr->len; i++) {
		if (i) strcat(s, ".");
		sprintf(&s[strlen(s)], "%u", valuePtr->value.oid[i]);
	    }
	}*/
	sprintf(s, "%s", valuePtr->value.ptr);
	break;
	default:
	sprintf(s, "%s", "");
	break;
    }

    return s;
}



static void fprintSubtype(FILE *f, SmiType *smiType)
{
    SmiRange       *range;
    SmiNamedNumber *nn;
    char	   s[100];
    int		   i;

    if ((smiType->basetype == SMI_BASETYPE_ENUM) ||
	(smiType->basetype == SMI_BASETYPE_BITS)) {
	for(i = 0, nn = smiGetFirstNamedNumber(smiType);
	    nn ; i++, nn = smiGetNextNamedNumber(nn)) {
	    if (i) {
		fprint(f, ", ");
	    } else {
		fprint(f, " (");
	    }
	    sprintf(s, "%s(%s)", nn->name,
		    getValueString(&nn->value, smiType));
	    fprintWrapped(f, INDENTVALUE + INDENT, s);
	}
	if (i) {
	    fprint(f, ")");
	}
	} else if(smiType->basetype == SMI_BASETYPE_POINTER) {
		nn = smiGetFirstNamedNumber(smiType);
		if(nn)
			fprint(f, " (%s)",nn->name);
    } else {
	for(i = 0, range = smiGetFirstRange(smiType);
	    range ; i++, range = smiGetNextRange(range)) {
	    if (i) {
		fprint(f, " | ");
	    } else {
		fprint(f, " (");
	    }	    
	    if (memcmp(&range->minValue, &range->maxValue,
		       sizeof(SmiValue))) {
		sprintf(s, "%s", getValueString(&range->minValue, smiType));
		sprintf(&s[strlen(s)], "..%s", 
			getValueString(&range->maxValue, smiType));
	    } else {
		sprintf(s, "%s", getValueString(&range->minValue, smiType));
	    }
	    fprintWrapped(f, INDENTVALUE + INDENT, s);
	}
	if (i) {
	    fprint(f, ")");
	}
    }
}

static void fprintAttributeSubtype(FILE *f, SmiAttribute *smiAttribute)
{
    SmiRange       *range;
    SmiNamedNumber *nn;
    char	   s[100];
    int		   i;

    if ((smiAttribute->basetype == SMI_BASETYPE_ENUM) ||
	(smiAttribute->basetype == SMI_BASETYPE_BITS)) {
	for(i = 0, nn = smiGetAttributeFirstNamedNumber(smiAttribute);
	    nn ; i++, nn = smiGetAttributeNextNamedNumber(nn)) {
	    if (i) {
		fprint(f, ", ");
	    } else {
		fprint(f, " (");
	    }
	    sprintf(s, "%s(%s)", nn->name,
		    getValueString(&nn->value, smiGetAttributeParentType(smiAttribute)));
	    fprintWrapped(f, INDENTVALUE + INDENT, s);
	}
	if (i) {
	    fprint(f, ")");
	}
	} else if(smiAttribute->basetype == SMI_BASETYPE_POINTER) {
		nn = smiGetAttributeFirstNamedNumber(smiAttribute);
		if(nn)
			fprint(f, " (%s)",nn->name);
    } else {
	for(i = 0, range = smiGetAttributeFirstRange(smiAttribute);
	    range ; i++, range = smiGetAttributeNextRange(range)) {
	    if (i) {
		fprint(f, " | ");
	    } else {
		fprint(f, " (");
	    }	    
	    if (memcmp(&range->minValue, &range->maxValue,
		       sizeof(SmiValue))) {
		sprintf(s, "%s", getValueString(&range->minValue, smiGetAttributeParentType(smiAttribute)));
		sprintf(&s[strlen(s)], "..%s", 
			getValueString(&range->maxValue, smiGetAttributeParentType(smiAttribute)));
	    } else {
		sprintf(s, "%s", getValueString(&range->minValue, smiGetAttributeParentType(smiAttribute)));
	    }
	    fprintWrapped(f, INDENTVALUE + INDENT, s);
	}
	if (i) {
	    fprint(f, ")");
	}
    }
}



static void fprintImports(FILE *f, SmiModule *smiModule)
{
    SmiImport *import;
    char   *lastModuleName = NULL;
    int    pos = 0, len, maxlen = 0;

    createImportList(smiModule);

    for (import = smiGetFirstImport(smiModule); import; import = smiGetNextImport(import)) {
	len = strlen(import->module);
	maxlen = (len > maxlen) ? len : maxlen;
    }

    for (import = smiGetFirstImport(smiModule); import; import = smiGetNextImport(import)) {
	int yaba = !lastModuleName || strcmp(import->module, lastModuleName);
	if (yaba) {
	    if (lastModuleName) {
		fprint(f, ");\n");
	    }
	    fprintSegment(f, INDENT, "", 0);
	    fprint(f, "import %-*s (", maxlen, import->module);
	    pos = INDENT + 12 + maxlen;
	} else {
	    fprint(f, ", ");
	}
	len = strlen(import->name);
	if (len + pos > INDENTMAX) {
	    fprint(f, "\n");
	    fprintSegment(f, INDENT, "", 0);
	    fprintSegment(f, INDENT, "", 0);
	    fprint(f, "     %-*s  ", maxlen, "");
	    pos = INDENT + 12 + maxlen;
	}
	fprint(f, "%s", import->name);
	pos += len;
	lastModuleName = import->module;
    }

    if (lastModuleName) {
	fprintf(f, ");\n");
    }

    fprint(f, "\n");

    freeImportList();
}




static void fprintRevisions(FILE *f, SmiModule *smiModule)
{
    int i;
    SmiRevision *smiRevision;
    
    for(i = 0, smiRevision = smiGetFirstRevision(smiModule);
	smiRevision; smiRevision = smiGetNextRevision(smiRevision)) {
	fprintSegment(f, INDENT, "revision {\n", 0);
	fprintSegment(f, 2 * INDENT, "date", INDENTVALUE);
	fprint(f, "\"%s\";\n", getStringTime(smiRevision->date));
	fprintSegment(f, 2 * INDENT, "description", INDENTVALUE);
	fprint(f, "\n");
	fprintMultilineString(f, 2 * INDENT, smiRevision->description);
	fprint(f, ";\n");
        fprintSegment(f, INDENT, "};\n", 0);
	i++;
    }
    if (i) {
	fprint(f, "\n");
    }
}



static void fprintTypedefs(FILE *f, SmiModule *smiModule)
{
    int		 i, j;
    SmiType	 *smiType;
    
    for(i = 0, smiType = smiGetFirstType(smiModule);
	smiType; smiType = smiGetNextType(smiType)) {
	
	if ((!(strcmp(smiModule->name, "SNMPv2-SMI"))) ||
	    (!(strcmp(smiModule->name, "RFC1155-SMI")))) {
	    for(j=0; excludeType[j]; j++) {
		if (!strcmp(smiType->name, excludeType[j])) break;
	    }
	    if (excludeType[j]) break;
	}
	    
	if (!i && !silent) {
	    fprint(f, "//\n// TYPE DEFINITIONS\n//\n\n");
	}
	fprintSegment(f, INDENT, "", 0);
	fprint(f, "typedef %s {\n", smiType->name);

	fprintSegment(f, 2 * INDENT, "type", INDENTVALUE);
	fprint(f, "%s", getTypeString(smiType->basetype,
				      smiGetParentType(smiType)));
	fprintSubtype(f, smiType);
	fprint(f, ";\n");

	if (smiType->value.basetype != SMI_BASETYPE_UNKNOWN) {
	    fprintSegment(f, 2 * INDENT, "default", INDENTVALUE);
	    fprint(f, "%s", getValueString(&smiType->value, smiType));
	    fprint(f, ";\n");
	}
	
	if (smiType->format) {
	    fprintSegment(f, 2 * INDENT, "format", INDENTVALUE);
	    fprint(f, "\"%s\";\n", smiType->format);
	}
	if (smiType->units) {
	    fprintSegment(f, 2 * INDENT, "units", INDENTVALUE);
	    fprint(f, "\"%s\";\n", smiType->units);
	}
	if ((smiType->status != SMI_STATUS_UNKNOWN) &&
	    (smiType->status != SMI_STATUS_MANDATORY) &&
	    (smiType->status != SMI_STATUS_OPTIONAL)) {
	    fprintSegment(f, 2 * INDENT, "status", INDENTVALUE);
	    fprint(f, "%s;\n", getStringStatus(smiType->status));
	}
	fprintSegment(f, 2 * INDENT, "description", INDENTVALUE);
	fprint(f, "\n");
	fprintMultilineString(f, 2 * INDENT, smiType->description);
	fprint(f, ";\n");
	if (smiType->reference) {
	    fprintSegment(f, 2 * INDENT, "reference", INDENTVALUE);
	    fprint(f, "\n");
	    fprintMultilineString(f, 2 * INDENT, smiType->reference);
	    fprint(f, ";\n");
	}
	fprintSegment(f, INDENT, "};\n\n", 0);
	i++;
    }
}

static void fprintIdentities(FILE *f, SmiModule *smiModule)
{
    int		 i;
    SmiIdentity	 *smiIdentity;
    SmiIdentity  *tmpIdentity;
    
    for(i = 0, smiIdentity = smiGetFirstIdentity(smiModule);
	smiIdentity; smiIdentity = smiGetNextIdentity(smiIdentity)) {
	    
	if (!i && !silent) {
	    fprint(f, "//\n// IDENTITY DEFINITIONS\n//\n\n");
	}
	fprintSegment(f, INDENT, "", 0);
	fprint(f, "identity %s {\n", smiIdentity->name);

	tmpIdentity = smiGetParentIdentity(smiIdentity);
	if (tmpIdentity) {
	    fprintSegment(f, 2 * INDENT, "parent", INDENTVALUE);
	    fprint(f, "%s", tmpIdentity->name);
	    fprint(f, ";\n");
	}

	if ((smiIdentity->status != SMI_STATUS_UNKNOWN) &&
	    (smiIdentity->status != SMI_STATUS_MANDATORY) &&
	    (smiIdentity->status != SMI_STATUS_OPTIONAL)) {
	    fprintSegment(f, 2 * INDENT, "status", INDENTVALUE);
	    fprint(f, "%s;\n", getStringStatus(smiIdentity->status));
	}
	fprintSegment(f, 2 * INDENT, "description", INDENTVALUE);
	fprint(f, "\n");
	fprintMultilineString(f, 2 * INDENT, smiIdentity->description);
	fprint(f, ";\n");
	if (smiIdentity->reference) {
	    fprintSegment(f, 2 * INDENT, "reference", INDENTVALUE);
	    fprint(f, "\n");
	    fprintMultilineString(f, 2 * INDENT, smiIdentity->reference);
	    fprint(f, ";\n");
	}
	fprintSegment(f, INDENT, "};\n\n", 0);
	i++;
    }
}

static void fprintExtensions(FILE *f, SmiModule *smiModule)
{
    int		 i;
    SmiMacro	 *smiMacro;
    
    for(i = 0, smiMacro = smiGetFirstMacro(smiModule);
	smiMacro; smiMacro = smiGetNextMacro(smiMacro)) {
	    
	if (!i && !silent) {
	    fprint(f, "//\n// EXTENSION DEFINITIONS\n//\n\n");
	}
	fprintSegment(f, INDENT, "", 0);
	fprint(f, "extension %s {\n", smiMacro->name);
	
	if ((smiMacro->status != SMI_STATUS_UNKNOWN) &&
	    (smiMacro->status != SMI_STATUS_MANDATORY) &&
	    (smiMacro->status != SMI_STATUS_OPTIONAL)) {
	    fprintSegment(f, 2 * INDENT, "status", INDENTVALUE);
	    fprint(f, "%s;\n", getStringStatus(smiMacro->status));
	}
	fprintSegment(f, 2 * INDENT, "description", INDENTVALUE);
	fprint(f, "\n");
	fprintMultilineString(f, 2 * INDENT, smiMacro->description);
	fprint(f, ";\n");
	if (smiMacro->reference) {
	    fprintSegment(f, 2 * INDENT, "reference", INDENTVALUE);
	    fprint(f, "\n");
	    fprintMultilineString(f, 2 * INDENT, smiMacro->reference);
	    fprint(f, ";\n");
	}
	
	if(smiMacro->abnf) {
		fprintSegment(f, 2 * INDENT, "abnf", INDENTVALUE);
	    fprint(f, "\n");
	    fprintMultilineString(f, 2 * INDENT, smiMacro->abnf);
	    fprint(f, ";\n");
	}
	fprintSegment(f, INDENT, "};\n\n", 0);
	i++;
    }
}

static void fprintUniqueStatement(FILE *f, SmiClass *smiClass)
{
    SmiAttribute *attributePtr;
    int i;
    
    if(smiIsClassScalar(smiClass))
    {
    	fprintSegment(f, 2 * INDENT, "unique", INDENTVALUE);
	    fprint(f, "();\n");
	}
    
    attributePtr = smiGetFirstUniqueAttribute(smiClass);
    if (attributePtr) {
	fprintSegment(f, 2 * INDENT, "unique", INDENTVALUE);
	fprint(f, "(");
	for (i=0; attributePtr; 
	     attributePtr = smiGetNextUniqueAttribute(attributePtr))
	{
	    if (i) {
		fprint(f, ", %s",attributePtr->name);
	    } else {
		fprint(f, "%s",attributePtr->name);
	    }
	    i++;
	}
	fprint(f, ");\n\n");
    }
}
static void fprintAttributes(FILE *f, SmiClass *smiClass)
{
    int		 i;
    SmiAttribute *smiAttribute;
    SmiType	 *tmpType;
    SmiClass *tmpClass;
    
    for (i = 0, smiAttribute = smiGetFirstAttribute(smiClass);
	 smiAttribute; smiAttribute = smiGetNextAttribute(smiAttribute)) {

	fprintSegment(f, 2*INDENT, "", 0);
	fprint(f, "attribute %s {\n", smiAttribute->name);
	
	tmpType = smiGetAttributeParentType(smiAttribute);
	if (tmpType) {
	    fprintSegment(f, 3 * INDENT, "type", INDENTVALUE);
	    fprint(f, "%s ", tmpType->name);
	    fprintAttributeSubtype(f, smiAttribute);
	    fprint(f, ";\n");
	    fprintSegment(f, 3 * INDENT, "access", INDENTVALUE);
	    switch (smiAttribute->access) {
	    case SMI_ACCESS_READ_ONLY:
		fprint(f, "readonly;\n");
		break;
	    case SMI_ACCESS_READ_WRITE:
		fprint(f, "readwrite;\n");
		break;
	    case SMI_ACCESS_EVENT_ONLY:
		fprint(f, "eventonly;\n");
		break;
	    default:
		fprint(f, ";\n");
		break;	
	    }
	    
	    if (smiAttribute->value.basetype != SMI_BASETYPE_UNKNOWN) {
		fprintSegment(f, 3 * INDENT, "default", INDENTVALUE);
		fprint(f, "%s", getValueString(&smiAttribute->value, smiGetAttributeParentType(smiAttribute)));
		fprint(f, ";\n");
	    }
	    
	    if (smiAttribute->format) {
		fprintSegment(f, 3 * INDENT, "format", INDENTVALUE);
		fprint(f, "\"%s\";\n", smiAttribute->format);
	    }
	    if (smiAttribute->units) {
		fprintSegment(f, 3 * INDENT, "units", INDENTVALUE);
		fprint(f, "\"%s\";\n", smiAttribute->units);
	    }		
	}
	
	tmpClass = smiGetAttributeParentClass(smiAttribute);
	if (tmpClass) {
	    fprintSegment(f, 3 * INDENT, "type", INDENTVALUE);
	    fprint(f, "%s;\n", tmpClass->name);
	}
	
	if ((smiAttribute->status != SMI_STATUS_UNKNOWN) &&
	    (smiAttribute->status != SMI_STATUS_MANDATORY) &&
	    (smiAttribute->status != SMI_STATUS_OPTIONAL)) {
	    fprintSegment(f, 3 * INDENT, "status", INDENTVALUE);
	    fprint(f, "%s;\n", getStringStatus(smiAttribute->status));
	}
	fprintSegment(f, 3 * INDENT, "description", INDENTVALUE);
	fprint(f, "\n");
	fprintMultilineString(f, 3 * INDENT, smiAttribute->description);
	fprint(f, ";\n");
	if (smiAttribute->reference) {
	    fprintSegment(f, 3 * INDENT, "reference", INDENTVALUE);
	    fprint(f, "\n");
	    fprintMultilineString(f, 3 * INDENT, smiAttribute->reference);
	    fprint(f, ";\n");
	}
	
	fprintSegment(f, 2*INDENT, "};\n\n", 0);
	i++;
    }
}

static void fprintEvents(FILE *f, SmiClass *smiClass)
{
    int		 i;
    SmiEvent *smiEvent;
    
    for(i = 0, smiEvent = smiGetFirstEvent(smiClass);
	smiEvent; smiEvent = smiGetNextEvent(smiEvent)) {
	/*    
	if (!i && !silent) {
		fprint(f,"\n");
		fprintSegment(f, 2 * INDENT, "// ATTRIBUTE DEFINITIONS\n\n",0);
	}*/
	fprintSegment(f, 2*INDENT, "", 0);
	fprint(f, "event %s {\n", smiEvent->name);
	
	if ((smiEvent->status != SMI_STATUS_UNKNOWN) &&
	    (smiEvent->status != SMI_STATUS_MANDATORY) &&
	    (smiEvent->status != SMI_STATUS_OPTIONAL)) {
	    fprintSegment(f, 3 * INDENT, "status", INDENTVALUE);
	    fprint(f, "%s;\n", getStringStatus(smiEvent->status));
	}
	fprintSegment(f, 3 * INDENT, "description", INDENTVALUE);
	fprint(f, "\n");
	fprintMultilineString(f, 3 * INDENT, smiEvent->description);
	fprint(f, ";\n");
	if (smiEvent->reference) {
	    fprintSegment(f, 3 * INDENT, "reference", INDENTVALUE);
	    fprint(f, "\n");
	    fprintMultilineString(f, 3 * INDENT, smiEvent->reference);
	    fprint(f, ";\n");
	}
	
	fprintSegment(f, 2*INDENT, "};\n\n", 0);
	i++;
    }
}

static void fprintClasses(FILE *f, SmiModule *smiModule)
{
    int		 i;
    SmiClass	 *smiClass;
    SmiClass	 *tmpClass;
    
    for (i = 0, smiClass = smiGetFirstClass(smiModule);
	 smiClass; smiClass = smiGetNextClass(smiClass)) {
	    
	if (!i && !silent) {
	    fprint(f, "//\n// CLASS DEFINITIONS\n//\n\n");
	}
	fprintSegment(f, INDENT, "", 0);
	fprint(f, "class %s {\n", smiClass->name);

	tmpClass = smiGetParentClass(smiClass);
	if (tmpClass) {
	    fprintSegment(f, 2 * INDENT, "extends", INDENTVALUE);
	    fprint(f, "%s;\n\n", tmpClass->name);
	}
	
	fprintAttributes(f,smiClass);
	
	fprintUniqueStatement(f,smiClass);
	
	fprintEvents(f,smiClass);
	
	if ((smiClass->status != SMI_STATUS_UNKNOWN) &&
	    (smiClass->status != SMI_STATUS_MANDATORY) &&
	    (smiClass->status != SMI_STATUS_OPTIONAL)) {
	    fprintSegment(f, 2 * INDENT, "status", INDENTVALUE);
	    fprint(f, "%s;\n", getStringStatus(smiClass->status));
	}
	fprintSegment(f, 2 * INDENT, "description", INDENTVALUE);
	fprint(f, "\n");
	fprintMultilineString(f, 2 * INDENT, smiClass->description);
	fprint(f, ";\n");
	if (smiClass->reference) {
	    fprintSegment(f, 2 * INDENT, "reference", INDENTVALUE);
	    fprint(f, "\n");
	    fprintMultilineString(f, 2 * INDENT, smiClass->reference);
	    fprint(f, ";\n");
	}
	
	fprintSegment(f, INDENT, "};\n\n", 0);
	i++;
    }
}

static void fprintObjects(FILE *f, SmiModule *smiModule)
{
    int		 i, j;
    SmiNode	 *smiNode, *relatedNode;
    SmiElement   *smiElement;
    SmiType	 *smiType;
    int		 indent = 0;
    int		 lastindent = -1;
    char	 *s = NULL;
    SmiNodekind  nodekinds;

    nodekinds =  SMI_NODEKIND_NODE | SMI_NODEKIND_TABLE |
	SMI_NODEKIND_ROW | SMI_NODEKIND_COLUMN | SMI_NODEKIND_SCALAR |
	SMI_NODEKIND_CAPABILITIES;
    
    for(i = 0, smiNode = smiGetFirstNode(smiModule, nodekinds);
	smiNode; smiNode = smiGetNextNode(smiNode, nodekinds)) {

	if (smiNode->nodekind == SMI_NODEKIND_NODE) {
	    indent = 0;
	    s = "node";
	} else if (smiNode->nodekind == SMI_NODEKIND_CAPABILITIES) {
	    indent = 0;
	    s = "node";
	} else if (smiNode->nodekind == SMI_NODEKIND_TABLE) {
	    indent = 0;
	    s = "table";
	} else if (smiNode->nodekind == SMI_NODEKIND_ROW) {
	    indent = 1;
	    s = "row";
	} else if (smiNode->nodekind == SMI_NODEKIND_COLUMN) {
	    indent = 2;
	    s = "column";
	} else if (smiNode->nodekind == SMI_NODEKIND_SCALAR) {
	    indent = 0;
	    s = "scalar";
	}

	if (!i && !silent) {
	    fprint(f, "//\n// OBJECT DEFINITIONS\n//\n\n");
	}

	for (j = lastindent; j >= indent; j--) {
	    fprintSegment(f, (1 + j) * INDENT, "", 0);
	    fprint(f, "};\n");
	}
	fprint(f, "\n");
	lastindent = indent;
	
	if (smiNode->nodekind == SMI_NODEKIND_CAPABILITIES) {
	    fprintSegment(f, (1 + indent) * INDENT, "", 0);
	    fprint(f, "-- This has been an SMIv2 AGENT-CAPABILITIES node:\n");
	}
	
	fprintSegment(f, (1 + indent) * INDENT, "", 0);
	fprint(f, "%s %s {\n", s, smiNode->name);
	
	if (smiNode->oid) {
	    fprintSegment(f, (2 + indent) * INDENT, "oid", INDENTVALUE);
	    fprint(f, "%s;\n", getOidString(smiNode, 0));
	}

	smiType = smiGetNodeType(smiNode);
	if (smiType && (smiType->basetype != SMI_BASETYPE_UNKNOWN)) {
	    fprintSegment(f, (2 + indent) * INDENT, "type", INDENTVALUE);
	    if (!smiType->name) {
		/*
		 * an implicitly restricted type.
		 */
		fprint(f, "%s", getTypeString(smiType->basetype,
					      smiGetParentType(smiType)));
		fprintSubtype(f, smiType);
		fprint(f, ";\n");
	    } else {
		fprint(f, "%s;\n",
		       getTypeString(smiType->basetype, smiType));
	    }
	}

	if ((smiNode->nodekind != SMI_NODEKIND_TABLE) &&
	    (smiNode->nodekind != SMI_NODEKIND_ROW) &&
	    (smiNode->nodekind != SMI_NODEKIND_CAPABILITIES) &&
	    (smiNode->nodekind != SMI_NODEKIND_NODE)) {
	    if (smiNode->access != SMI_ACCESS_UNKNOWN) {
		fprintSegment(f, (2 + indent) * INDENT, "access", INDENTVALUE);
		fprint(f, "%s;\n", getAccessString(smiNode->access));
	    }
	}

	relatedNode = smiGetRelatedNode(smiNode);
	switch (smiNode->indexkind) {
	case SMI_INDEX_INDEX:
	    if (smiNode->implied) {
		fprintSegment(f, (2 + indent) * INDENT, "index implied",
			      INDENTVALUE);
	    } else {
		fprintSegment(f, (2 + indent) * INDENT, "index", INDENTVALUE);
	    }
	    fprint(f, "(");
	    for (j = 0, smiElement = smiGetFirstElement(smiNode); smiElement;
		 j++, smiElement = smiGetNextElement(smiElement)) {
		if (j) {
		    fprint(f, ", ");
		}
		fprintWrapped(f, INDENTVALUE + 1,
			      smiGetElementNode(smiElement)->name);
		/* TODO: non-local name if non-local */
	    } /* TODO: empty? -> print error */
	    fprint(f, ");\n");
	    break;
	case SMI_INDEX_AUGMENT:
	    if (relatedNode) {
		fprintSegment(f, (2 + indent) * INDENT, "augments",
			      INDENTVALUE);
		fprint(f, "%s;\n", relatedNode->name);
		/* TODO: non-local name if non-local */
	    } /* TODO: else print error */
	    break;
	case SMI_INDEX_REORDER:
	    if (relatedNode) {
		fprintSegment(f, (2 + indent) * INDENT, "", 0);
		fprint(f, "reorders %s", relatedNode->name);
		/* TODO: non-local name if non-local */
		if (smiNode->implied) {
		    fprint(f, " implied");
		}
		fprint(f, " (");
		for (j = 0, smiElement = smiGetFirstElement(smiNode);
		     smiElement;
		     j++, smiElement = smiGetNextElement(smiElement)) {
		    if (j) {
			fprint(f, ", ");
		    }
		    fprintWrapped(f, INDENTVALUE + 1,
				  smiGetElementNode(smiElement)->name);
		    /* TODO: non-local name if non-local */
		} /* TODO: empty? -> print error */
		fprint(f, ");\n");
	    } /* TODO: else print error */
	    break;
	case SMI_INDEX_SPARSE:
	    if (relatedNode) {
		fprintSegment(f, (2 + indent) * INDENT, "sparse", INDENTVALUE);
		fprint(f, "%s;\n", relatedNode->name);
		/* TODO: non-local name if non-local */
	    } /* TODO: else print error */
	    break;
	case SMI_INDEX_EXPAND:
	    if (relatedNode) {
		fprintSegment(f, (2 + indent) * INDENT, "", 0);
		fprint(f, "expands %s", relatedNode->name);
		/* TODO: non-local name if non-local */
		if (smiNode->implied) {
		    fprint(f, " implied");
		}
		fprint(f, " (");
		for (j = 0, smiElement = smiGetFirstElement(smiNode);
		     smiElement;
		     j++, smiElement = smiGetNextElement(smiElement)) {
		    if (j) {
			fprint(f, ", ");
		    }
		    fprintWrapped(f, INDENTVALUE + 1,
				  smiGetElementNode(smiElement)->name);
		    /* TODO: non-local name if non-local */
		} /* TODO: empty? -> print error */
		fprint(f, ");\n");
	    } /* TODO: else print error */
	    break;
	case SMI_INDEX_UNKNOWN:
	    break;
	}
	
	if (smiNode->create) {
	    fprintSegment(f, (2 + indent) * INDENT, "create", INDENTVALUE);
	    /* TODO: create list */
	    fprint(f, ";\n");
	}
	
	if (smiNode->value.basetype != SMI_BASETYPE_UNKNOWN) {
	    fprintSegment(f, (2 + indent) * INDENT, "default", INDENTVALUE);
	    fprint(f, "%s", getValueString(&smiNode->value, smiType));
	    fprint(f, ";\n");
	}
	
	if (smiNode->format) {
	    fprintSegment(f, (2 + indent) * INDENT, "format", INDENTVALUE);
	    fprint(f, "\"%s\";\n",smiNode->format);
	}
	if (smiNode->units) {
	    fprintSegment(f, (2 + indent) * INDENT, "units", INDENTVALUE);
	    fprint(f, "\"%s\";\n", smiNode->units);
	}
	if ((smiNode->status != SMI_STATUS_CURRENT) &&
	    (smiNode->status != SMI_STATUS_UNKNOWN) &&
	    (smiNode->status != SMI_STATUS_MANDATORY) &&
	    (smiNode->status != SMI_STATUS_OPTIONAL)) {
	    fprintSegment(f, (2 + indent) * INDENT, "status", INDENTVALUE);
	    fprint(f, "%s;\n", getStringStatus(smiNode->status));
	}
	if (smiNode->description) {
	    fprintSegment(f, (2 + indent) * INDENT, "description",
			  INDENTVALUE);
	    fprint(f, "\n");
	    fprintMultilineString(f, (2 + indent) * INDENT,
				  smiNode->description);
	    fprint(f, ";\n");
	}
	if (smiNode->reference) {
	    fprintSegment(f, (2 + indent) * INDENT, "reference",
			  INDENTVALUE);
	    fprint(f, "\n");
	    fprintMultilineString(f, (2 + indent) * INDENT,
				  smiNode->reference);
	    fprint(f, ";\n");
	}
	i++;
    }
    
    if (i) {
	fprintSegment(f, (1 + indent) * INDENT, "", 0);
	fprint(f, "};\n\n");
    }
}



static void fprintNotifications(FILE *f, SmiModule *smiModule)
{
    int		 i, j;
    SmiNode	 *smiNode;
    SmiElement   *smiElement;
    
    for(i = 0, smiNode = smiGetFirstNode(smiModule,
					 SMI_NODEKIND_NOTIFICATION);
	smiNode;
	smiNode = smiGetNextNode(smiNode, SMI_NODEKIND_NOTIFICATION)) {

	if (!i && !silent) {
	    fprint(f, "//\n// NOTIFICATION DEFINITIONS\n//\n\n");
	}

	fprintSegment(f, INDENT, "", 0);
	fprint(f, "notification %s {\n", smiNode->name);

	if (smiNode->oid) {
	    fprintSegment(f, 2 * INDENT, "oid", INDENTVALUE);
	    fprint(f, "%s;\n", getOidString(smiNode, 0));
	}

	if ((smiElement = smiGetFirstElement(smiNode))) {
	    fprintSegment(f, 2 * INDENT, "objects", INDENTVALUE);
	    fprint(f, "(");
	    for (j = 0; smiElement;
		 j++, smiElement = smiGetNextElement(smiElement)) {
		if (j) {
		    fprint(f, ", ");
		}
		fprintWrapped(f, INDENTVALUE + 1,
			      smiGetElementNode(smiElement)->name);
		/* TODO: non-local name if non-local */
	    } /* TODO: empty? -> print error */
	    fprint(f, ");\n");
	}
	
	if ((smiNode->status != SMI_STATUS_CURRENT) &&
	    (smiNode->status != SMI_STATUS_UNKNOWN) &&
	    (smiNode->status != SMI_STATUS_MANDATORY) &&
	    (smiNode->status != SMI_STATUS_OPTIONAL)) {
	    fprintSegment(f, 2 * INDENT, "status", INDENTVALUE);
	    fprint(f, "%s;\n", getStringStatus(smiNode->status));
	}
	
	if (smiNode->description) {
	    fprintSegment(f, 2 * INDENT, "description", INDENTVALUE);
	    fprint(f, "\n");
	    fprintMultilineString(f, 2 * INDENT, smiNode->description);
	    fprint(f, ";\n");
	}

	if (smiNode->reference) {
	    fprintSegment(f, 2 * INDENT, "reference", INDENTVALUE);
	    fprint(f, "\n");
	    fprintMultilineString(f, 2 * INDENT, smiNode->reference);
	    fprint(f, ";\n");
	}

	fprintSegment(f, INDENT, "", 0);
	fprint(f, "};\n\n");
	i++;
    }
}



static void fprintGroups(FILE *f, SmiModule *smiModule)
{
    int		 i, j;
    SmiNode	 *smiNode;
    SmiElement   *smiElement;
    
    for(i = 0, smiNode = smiGetFirstNode(smiModule, SMI_NODEKIND_GROUP);
	smiNode; smiNode = smiGetNextNode(smiNode, SMI_NODEKIND_GROUP)) {
	
	if (!i && !silent) {
	    fprint(f, "//\n// GROUP DEFINITIONS\n//\n\n");
	}
	
	fprintSegment(f, INDENT, "", 0);
	fprint(f, "group %s {\n", smiNode->name);
	
	if (smiNode->oid) {
	    fprintSegment(f, 2 * INDENT, "oid", INDENTVALUE);
	    fprint(f, "%s;\n", getOidString(smiNode, 0));
	}
	
	fprintSegment(f, 2 * INDENT, "members", INDENTVALUE);
	fprint(f, "(");
	for (j = 0, smiElement = smiGetFirstElement(smiNode); smiElement;
		 j++, smiElement = smiGetNextElement(smiElement)) {
	    if (j) {
		fprint(f, ", ");
	    }
	    fprintWrapped(f, INDENTVALUE + 1,
			  smiGetElementNode(smiElement)->name);
	    /* TODO: non-local name if non-local */
	} /* TODO: empty? -> print error */
	fprint(f, ");\n");
	    
	if ((smiNode->status != SMI_STATUS_CURRENT) &&
	    (smiNode->status != SMI_STATUS_UNKNOWN) &&
	    (smiNode->status != SMI_STATUS_MANDATORY) &&
	    (smiNode->status != SMI_STATUS_OPTIONAL)) {
	    fprintSegment(f, 2 * INDENT, "status", INDENTVALUE);
	    fprint(f, "%s;\n", getStringStatus(smiNode->status));
	}
	
	if (smiNode->description) {
	    fprintSegment(f, 2 * INDENT, "description", INDENTVALUE);
	    fprint(f, "\n");
	    fprintMultilineString(f, 2 * INDENT, smiNode->description);
	    fprint(f, ";\n");
	}
	
	if (smiNode->reference) {
	    fprintSegment(f, 2 * INDENT, "reference", INDENTVALUE);
	    fprint(f, "\n");
	    fprintMultilineString(f, 2 * INDENT, smiNode->reference);
	    fprint(f, ";\n");
	}
	
	fprintSegment(f, INDENT, "", 0);
	fprint(f, "};\n\n");
	i++;
    }
}



static void fprintCompliances(FILE *f, SmiModule *smiModule)
{
    int		  i, j;
    SmiNode	  *smiNode, *smiNode2;
    SmiType	  *smiType;
    SmiOption	  *smiOption;
    SmiRefinement *smiRefinement;
    SmiElement    *smiElement;
    
    for(i = 0, smiNode = smiGetFirstNode(smiModule, SMI_NODEKIND_COMPLIANCE);
	smiNode; smiNode = smiGetNextNode(smiNode, SMI_NODEKIND_COMPLIANCE)) {
	
	if (!i && !silent) {
	    fprint(f, "//\n// COMPLIANCE DEFINITIONS\n//\n\n");
	}

	fprintSegment(f, INDENT, "", 0);
	fprint(f, "compliance %s {\n", smiNode->name);
	    
	if (smiNode->oid) {
	    fprintSegment(f, 2 * INDENT, "oid", INDENTVALUE);
	    fprint(f, "%s;\n", getOidString(smiNode, 0));
	}
	    
	if ((smiNode->status != SMI_STATUS_CURRENT) &&
	    (smiNode->status != SMI_STATUS_UNKNOWN) &&
	    (smiNode->status != SMI_STATUS_MANDATORY) &&
	    (smiNode->status != SMI_STATUS_OPTIONAL)) {
	    fprintSegment(f, 2 * INDENT, "status", INDENTVALUE);
	    fprint(f, "%s;\n", getStringStatus(smiNode->status));
	}
	    
	if (smiNode->description) {
	    fprintSegment(f, 2 * INDENT, "description", INDENTVALUE);
	    fprint(f, "\n");
	    fprintMultilineString(f, 2 * INDENT, smiNode->description);
	    fprint(f, ";\n");
	}
	    
	if (smiNode->reference) {
	    fprintSegment(f, 2 * INDENT, "reference", INDENTVALUE);
	    fprint(f, "\n");
	    fprintMultilineString(f, 2 * INDENT, smiNode->reference);
	    fprint(f, ";\n");
	}

	if ((smiElement = smiGetFirstElement(smiNode))) {
	    fprint(f, "\n");
	    fprintSegment(f, 2 * INDENT, "mandatory", INDENTVALUE);
	    fprint(f, "(");
	    for (j = 0; smiElement;
		 j++, smiElement = smiGetNextElement(smiElement)) {
		if (j) {
		    fprint(f, ", ");
		}
		fprintWrapped(f, INDENTVALUE + 1,
			      smiGetElementNode(smiElement)->name);
		/* TODO: non-local name if non-local */
	    } /* TODO: empty? -> print error */
	    fprint(f, ");\n");
	}
	
	if ((smiOption = smiGetFirstOption(smiNode))) {
	    fprint(f, "\n");
	    for(; smiOption; smiOption = smiGetNextOption(smiOption)) {
		smiNode2 = smiGetOptionNode(smiOption);
		fprintSegment(f, 2 * INDENT, "", 0);
		fprint(f, "optional %s {\n", smiNode2->name);
		fprintSegment(f, 3 * INDENT, "description", INDENTVALUE);
		fprint(f, "\n");
		fprintMultilineString(f, 3 * INDENT, smiOption->description);
		fprint(f, ";\n");
		fprintSegment(f, 2 * INDENT, "};\n", 0);
	    }
	}
	
	if ((smiRefinement = smiGetFirstRefinement(smiNode))) {
	    fprint(f, "\n");
	    for(; smiRefinement;
		smiRefinement = smiGetNextRefinement(smiRefinement)) {
		fprintSegment(f, 2 * INDENT, "", 0);
		fprint(f, "refine %s {\n",
		       smiGetRefinementNode(smiRefinement)->name);

		smiType = smiGetRefinementType(smiRefinement);
		if (smiType) {
		    fprintSegment(f, 3 * INDENT, "type", INDENTVALUE);
		    fprint(f, "%s",
			   getTypeString(smiType->basetype,
					 smiGetParentType(smiType)));
		    fprintSubtype(f, smiType);
		    fprint(f, ";\n");
		}

		smiType = smiGetRefinementWriteType(smiRefinement);
		if (smiType) {
		    fprintSegment(f, 3 * INDENT, "writetype", INDENTVALUE);
		    fprint(f, "%s",
			   getTypeString(smiType->basetype,
					 smiGetParentType(smiType)));
		    fprintSubtype(f, smiType);
		    fprint(f, ";\n");
		}

		if (smiRefinement->access != SMI_ACCESS_UNKNOWN) {
		    fprintSegment(f, 3 * INDENT, "access", INDENTVALUE);
		    fprint(f, "%s;\n", getAccessString(smiRefinement->access));
		}
		fprintSegment(f, 3 * INDENT, "description", INDENTVALUE);
		fprint(f, "\n");
		fprintMultilineString(f, 3 * INDENT,
				      smiRefinement->description);
		fprint(f, ";\n");
		fprintSegment(f, 2 * INDENT, "};\n", 0);
	    }
	}
	
	fprintSegment(f, INDENT, "", 0);
	fprint(f, "};\n\n");
	i++;
    }
}



static void dumpSming(int modc, SmiModule **modv, int flags, char *output)
{
    SmiModule   *smiModule;
    SmiNode	*smiNode;
    int		i;
    FILE	*f = stdout;

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

	smiModule = modv[i];
    
	fprint(f, "//\n");
	fprint(f, "// This module has been generated by smidump "
	       SMI_VERSION_STRING ". Do not edit.\n");
	fprint(f, "//\n");
	fprint(f, "module %s ", smiModule->name);
	fprint(f, "{\n");
	fprint(f, "\n");
	
	fprintImports(f, smiModule);
	
	if (! silent) {
	    fprint(f, "//\n// MODULE META INFORMATION\n//\n\n");
	}
	fprintSegment(f, INDENT, "organization", INDENTVALUE);
	fprint(f, "\n");
	fprintMultilineString(f, INDENT, smiModule->organization);
	fprint(f, ";\n\n");
	fprintSegment(f, INDENT, "contact", INDENTVALUE);
	fprint(f, "\n");
	fprintMultilineString(f, INDENT, smiModule->contactinfo);
	fprint(f, ";\n\n");
	fprintSegment(f, INDENT, "description", INDENTVALUE);
	fprint(f, "\n");
	fprintMultilineString(f, INDENT, smiModule->description);
	fprint(f, ";\n\n");
	if (smiModule->reference) {
	    fprintSegment(f, INDENT, "reference", INDENTVALUE);
	    fprint(f, "\n");
	    fprintMultilineString(f, INDENT, smiModule->reference);
	    fprint(f, ";\n\n");
	}
	
	fprintRevisions(f, smiModule);
	
	smiNode = smiGetModuleIdentityNode(smiModule);
	if (smiNode) {
	    fprintSegment(f, INDENT, "identity", INDENTVALUE);
	    fprint(f, "%s;\n\n", smiNode->name);
	}
	
	fprintExtensions(f, smiModule);
	fprintIdentities(f, smiModule);
	fprintTypedefs(f, smiModule);
	fprintClasses(f, smiModule);
	fprintObjects(f, smiModule);
	fprintNotifications(f, smiModule);
	fprintGroups(f, smiModule);
	fprintCompliances(f, smiModule);
	
	fprint(f, "}; // end of module %s.\n", smiModule->name);
    }
    
    if (fflush(f) || ferror(f)) {
	perror("smidump: write error");
	exit(1);
    }

    if (output) {
	fclose(f);
    }
}



void initSming()
{
    static SmidumpDriver driver = {
	"sming",
	dumpSming,
	0,
	SMIDUMP_DRIVER_CANT_UNITE,
	"SMIng",
	NULL,
	NULL
    };
    
    smidumpRegisterDriver(&driver);
}
