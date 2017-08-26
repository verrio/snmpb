/*
 * smi2yang.c --
 *
 *      Conversion of SMIv2 modules to YANG modules
 *
 * Copyright (c) 2007-2011 J. Schoenwaelder, Jacobs University Bremen
 *
 * See the file "COPYING" for information on usage and redistribution
 * of this file, and for a DISCLAIMER OF ALL WARRANTIES.
 *
 * Author: Juergen Schoenwaelder
 * @(#) $Id: smi.c 20582 2009-09-03 10:34:06Z schoenw $
 */

/*
 * Rewrite the code to generate imports following the algorithm in
 * the I-D.
 */

/*
 * TODO:
 * - fix addYangNode to pick up a suitable line number?
 * - should createModuleInfo set the correct path?
 * - reconsider the format to pattern translation
 */

#include <config.h>

#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <ctype.h>

#include "smi2yang.h"
#include "yang-data.h"
#include "error.h"

#ifdef HAVE_DMALLOC_H
#include <dmalloc.h>
#endif

#define  URNBASE        "urn:ietf:params:xml:ns:yang:smiv2:"

static const char *convertType[] = {

    /*
     * Translation of the SMIng built-in types to the YANG
     * equivalents.
     */
    
    "",		  "Integer32",   NULL,	     "int32",
    "",		  "Integer64",   NULL,	     "int64",
    "",		  "Unsigned32",  NULL,	     "uint32",
    "",		  "Unsigned64",  NULL,	     "uint64",
    "",		  "OctetString", NULL,	     "binary",
    "",		  "Enumeration", NULL,	     "enumeration",
    "",		  "Bits",	 NULL,	     "bits",
    "",		  "ObjectIdentifier", "ietf-yang-types", "object-identifier-128",

    /*
     * We want to do these translations as well in order to retire the
     * SNMPv2-SMI module which is not really an SMIv2 module but part
     * of the definition of SNMPv2-SMI itself.
     */

    "SNMPv2-SMI", "Integer32",  NULL,        "int32",
    "SNMPv2-SMI", "Integer64",  NULL,        "int64",
    "SNMPv2-SMI", "Unsigned32", NULL,        "uint32",
    "SNMPv2-SMI", "Opaque",    "ietf-yang-smiv2", "opaque",
    "SNMPv2-SMI", "Counter32", "ietf-yang-types", "counter32",
    "SNMPv2-SMI", "Counter64", "ietf-yang-types", "counter64",
    "SNMPv2-SMI", "Gauge32",   "ietf-yang-types", "gauge32",
    "SNMPv2-SMI", "TimeTicks", "ietf-yang-types", "timeticks",
    "SNMPv2-SMI", "IpAddress", "ietf-inet-types", "ipv4-address",

    /*
     * And we like to do the same for RFC1155-SMI definitions...
     */

    "RFC1155-SMI", "Opaque",    "ietf-yang-smiv2", "opaque",
    "RFC1155-SMI", "Counter",   "ietf-yang-types", "counter32",
    "RFC1155-SMI", "Gauge",     "ietf-yang-types", "gauge32",
    "RFC1155-SMI", "TimeTicks", "ietf-yang-types", "timeticks",
    "RFC1155-SMI", "IpAddress", "ietf-inet-types", "ipv4-address",
    "RFC1155-SMI", "NetworkAddress", "ietf-inet-types", "ipv4-address",
    
    /*
     * We also translate frequently used SNMPv2-TCs that have a YANG
     * equivalent. Note that DateAndTime is slightly different from
     * the ISO profile used by date-and-time.
     */

    "SNMPv2-TC", "PhysAddress", "ietf-yang-types", "phys-address",
    "SNMPv2-TC", "MacAddress",  "ietf-yang-types", "mac-address",
    "SNMPv2-TC", "TruthValue",  NULL,              "boolean",
    "SNMPv2-TC", "TimeStamp",   "ietf-yang-types", "timestamp",

    "RMON2-MIB", "ZeroBasedCounter32",  "ietf-yang-types", "zero-based-counter32",

    "HCNUM-TC", "ZeroBasedCounter64",  "ietf-yang-types", "zero-based-counter64",
    "HCNUM-TC", "CounterBasedGauge64", "ietf-yang-types", "gauge64",

    "INET-ADDRESS-MIB", "InetVersion",                "ietf-inet-types", "ip-version",
    "INET-ADDRESS-MIB", "InetPortNumber",             "ietf-inet-types", "port-number",
    "INET-ADDRESS-MIB", "InetAutonomousSystemNumber", "ietf-inet-types", "as-number",

    "DIFFSERV-DSCP-TC", "Dscp", "ietf-inet-types", "dscp",

    "IPV6-FLOW-LABEL-MIB", "IPv6FlowLabel", "ietf-inet-types", "ipv6-flow-label",

    "URI-TC-MIB", "Uri", "ietf-inet-types", "uri",

    NULL, NULL, NULL, NULL
};


static const char *convertImport[] = {

    /*
     * Things that are not types and removed from imports...
     */

    "SNMPv2-SMI",  "MODULE-IDENTITY",    NULL, NULL,
    "SNMPv2-SMI",  "OBJECT-IDENTITY",    NULL, NULL,
    "SNMPv2-SMI",  "OBJECT-TYPE",        NULL, NULL,
    "SNMPv2-SMI",  "NOTIFICATION-TYPE",  NULL, NULL,
    "SNMPv2-SMI",  "mib-2",              NULL, NULL,
    "SNMPv2-TC",   "TEXTUAL-CONVENTION", NULL, NULL,
    "SNMPv2-CONF", "OBJECT-GROUP",       NULL, NULL,
    "SNMPv2-CONF", "NOTIFICATION-GROUP", NULL, NULL,
    "SNMPv2-CONF", "MODULE-COMPLIANCE",  NULL, NULL,
    "SNMPv2-CONF", "AGENT-CAPABILITIES", NULL, NULL,
    "SNMPv2-MIB",  "snmpTraps",	         NULL, NULL,

    NULL, NULL, NULL, NULL
};

/*
 * SMIv2 modules we never like to import from...
 */

static const char *ignoreImports[] = {
    "SNMPv2-SMI", "SNMPv2-CONF", NULL
};

/*
 * Structure used to build a list of imported types.
 */

typedef struct syImport {
    char            *module;
    char	    *prefix;
    struct syImport *nextPtr;
} syImport;

static syImport *importList = NULL;

static char*
getStringDate(time_t t)
{
    static char   s[27];
    struct tm	  *tm;

    tm = gmtime(&t);
    sprintf(s, "%04d-%02d-%02d",
	    tm->tm_year + 1900, tm->tm_mon + 1, tm->tm_mday);
    return s;
}


static int
isAccessibleLeaf(SmiNode *smiNode)
{
    return (smiNode->access == SMI_ACCESS_READ_ONLY
	    || smiNode->access == SMI_ACCESS_READ_WRITE);
}


static int
isPrefixUnique(const char *prefix)
{
    syImport *import;

    for (import = importList; import; import = import->nextPtr) {
         if (strcmp(prefix, import->prefix) == 0) {
             return 0;
	 }
    }

    return 1;
}


static char*
guessNicePrefix(const char *moduleName)
{
    char *prefix;
    int i, d;

    char *specials[] = {
	"ietf-yang-smiv2", "smiv2",
	"ietf-yang-types", "yang",
	"ietf-inet-types", "inet",
	NULL, NULL
    };

    for (i = 0; specials[i]; i +=2) {
	if (strcmp(moduleName, specials[i]) == 0) {
            if (isPrefixUnique(specials[i+1])) {
                return smiStrdup(specials[i+1]);
	    }
	}
    }

    prefix = smiStrdup(moduleName);
    for (i = 0; prefix[i]; i++) {
        prefix[i] = tolower(prefix[i]);
    }

    for (i = 0, d = 0; prefix[i]; i++) {
        if (prefix[i] == '-') {
	    d++;
	    if (d > 1) {
		prefix[i] = 0;
		if (isPrefixUnique(prefix)) {
		    return prefix;
		}
		prefix[i] = '-';
	    }
	}
    }

    return prefix;
}


static const char*
getModulePrefix(const char *moduleName)
{
    syImport *import;
    static char *prefix = NULL;

    for (import = importList; import; import = import->nextPtr) {
        if (strcmp(moduleName, import->module) == 0) {
            return import->prefix;
	}
    }
 
    if (prefix) smiFree(prefix);
    prefix = guessNicePrefix(moduleName);
    return prefix;
}


static syImport*
syaddImport(char *module, char *name)
{
    syImport **import, *newImport;
    
    if (!module || !name) {
	return NULL;
    }
	    
    for (import = &importList; *import; import = &(*import)->nextPtr) {
	int c = strcmp((*import)->module, module);
	if (c == 0) return *import;
	if (c > 0) break;
    }

    newImport = smiMalloc(sizeof(Import));
    newImport->module = module;
    newImport->prefix = guessNicePrefix(module);

    newImport->nextPtr = *import;
    *import = newImport;
	
    return *import;
}



static void
createImportList(SmiModule *smiModule)
{
    SmiImport   *smiImport;
    SmiIdentifier impModule, impName;
    SmiType	*smiType;
    SmiNode	*smiNode;
    int i;

    for (smiImport = smiGetFirstImport(smiModule); smiImport;
	 smiImport = smiGetNextImport(smiImport)) {

	impModule = smiImport->module;
	impName = smiImport->name;

	for (i = 0; convertType[i]; i += 4) {
	    if (strcmp(smiImport->module, convertType[i]) == 0
		&& strcmp(smiImport->name, convertType[i+1]) == 0) {
		impModule = (SmiIdentifier) convertType[i+2];
		impName = (SmiIdentifier) convertType[i+3];
		break;
	    }
	}

	if (! impModule || ! impName) continue;

	for (i = 0; convertImport[i]; i += 4) {
	    if (strcmp(smiImport->module, convertImport[i]) == 0
		&& strcmp(smiImport->name, convertImport[i+1]) == 0) {
		impModule = (SmiIdentifier) convertImport[i+2];
		impName = (SmiIdentifier) convertImport[i+3];
		break;
	    }
	}

	if (! impModule || ! impName) continue;
	syaddImport(impModule, impName);
    }

    /*
     * Add an import for the smiv2:oid extension and friends.
     */
    syaddImport("ietf-yang-smiv2", "smiv2");

    /*
     * Add import for yang-types that were originally ASN.1
     * builtins...
     */

    for (smiType = smiGetFirstType(smiModule);
	 smiType; smiType = smiGetNextType(smiType)) {
	SmiType *parentType = smiGetParentType(smiType);
	if (parentType && strcmp(parentType->name, "ObjectIdentifier") == 0) {
	    syaddImport("ietf-yang-types", "object-identifier-128");
	}
    }

    for (smiNode = smiGetFirstNode(smiModule,
				   SMI_NODEKIND_SCALAR | SMI_NODEKIND_COLUMN);
	 smiNode;
	 smiNode = smiGetNextNode(smiNode,
				  SMI_NODEKIND_SCALAR | SMI_NODEKIND_COLUMN)) {
	smiType = smiGetNodeType(smiNode);
	if (smiType && !smiType->name) {
	    smiType = smiGetParentType(smiType);
	}
	if (smiType && strcmp(smiType->name, "ObjectIdentifier") == 0) {
	    syaddImport("ietf-yang-types", "object-identifier-128");
	}
    }
}



static void
freeImportList(void)
{
    syImport *import, *freeme;

    for (import = importList; import; ) {
        smiFree(import->prefix);
	freeme = import;
	import = import->nextPtr;
	smiFree(freeme);
    }
    importList = NULL;
}


static void
smi2yangNamespace(SmiModule *smiModule, _YangNode *yangModulePtr,
		  _YangModuleInfo *yangModuleInfoPtr)
{
    char *s;
    const char *p;
    _YangNode *node = NULL;

    smiAsprintf(&s, "%s%s", URNBASE, smiModule->name);
    node = addYangNode(s, YANG_DECL_NAMESPACE, yangModulePtr);
    smiFree(s);
    if (yangModuleInfoPtr) {
	yangModuleInfoPtr->namespace = node->export.value;
    }

    p = getModulePrefix(smiModule->name);
    (void) addYangNode(p, YANG_DECL_PREFIX, yangModulePtr);
    if (yangModuleInfoPtr) {
	yangModuleInfoPtr->prefix = smiStrdup(p);
    }
}


static void
smi2yangImports(SmiModule *smiModule, _YangNode *yangModulePtr)
{
    int i;
    syImport *import;
    _YangNode *node;
    
    for (import = importList; import; import = import->nextPtr) {
	for (i = 0; ignoreImports[i]; i++) {
	    if (strcmp(ignoreImports[i], import->module) == 0) {
		break;
	    }
	}
	if (ignoreImports[i] == NULL) {
	    node = addYangNode(import->module, YANG_DECL_IMPORT, yangModulePtr);
	    (void) addYangNode(import->prefix, YANG_DECL_PREFIX, node);
	}
    }
}


static char*
smi2yangLeafPath(SmiNode *smiNode)
{
    SmiModule *smiModule, *parentModule = NULL;
    SmiNode *smiEntryNode = NULL, *smiTableNode = NULL, *smiParentNode = NULL;
    char *toplevel = NULL;
    char *s = NULL;
    const char *prefix;

    smiModule = smiGetNodeModule(smiNode);
    if (! smiModule) {
	return NULL;
    }
    prefix = getModulePrefix(smiModule->name);
    toplevel = smiModule->name;
    
    switch (smiNode->nodekind) {
    case SMI_NODEKIND_SCALAR:
	smiParentNode = smiGetParentNode(smiNode);
	if (toplevel) {
	    if (smiParentNode && smiParentNode->name) {
		smiAsprintf(&s, "/%s:%s/%s:%s/%s:%s",
			    prefix, toplevel,
			    prefix, smiParentNode->name,
			    prefix, smiNode->name);
	    } else {
		smiAsprintf(&s, "/%s:%s/%s:%s",
			    prefix, toplevel,
			    prefix, smiNode->name);
	    }
	} else {
	    if (smiParentNode && smiParentNode->name) {
		smiAsprintf(&s, "/%s:%s/%s:%s",
			    prefix, smiParentNode->name,
			    prefix, smiNode->name);
	    } else {
		smiAsprintf(&s, "/%s:%s",
			    prefix, smiNode->name);
	    }
	}
	break;
    case SMI_NODEKIND_COLUMN:
	smiEntryNode = smiGetParentNode(smiNode);
	if (smiEntryNode) {
	    if (smiEntryNode->indexkind == SMI_INDEX_AUGMENT) {
		smiEntryNode = smiGetRelatedNode(smiEntryNode);
	    }
	    if (smiEntryNode) {
		smiTableNode = smiGetParentNode(smiEntryNode);
	    }
	}
	if (smiEntryNode && smiTableNode) {
	    const char *parent_prefix = prefix;
	    if (smiTableNode) {
		SmiModule *m = smiGetNodeModule(smiTableNode);
		if (m) {
		    toplevel = m->name;
		    parent_prefix = getModulePrefix(m->name);
		}
	    }
	    smiAsprintf(&s, "/%s:%s/%s:%s/%s:%s/%s:%s",
			parent_prefix, toplevel,
			parent_prefix, smiTableNode->name,
			parent_prefix, smiEntryNode->name,
			prefix, smiNode->name);
	}
	break;
    default:
	break;
    }

    return s;
}

static char*
smi2yangListPath(SmiNode *smiEntryNode)
{
    SmiModule *smiModule;
    SmiNode *smiTableNode;
    char *toplevel = NULL;
    char *s = NULL;
    const char *prefix;

    smiModule = smiGetNodeModule(smiEntryNode);
    if (! smiModule) {
	return NULL;
    }
    prefix = getModulePrefix(smiModule->name);

    toplevel = smiModule->name;

    smiTableNode = smiGetParentNode(smiEntryNode);
    if (smiTableNode) {
	smiAsprintf(&s, "/%s:%s/%s:%s/%s:%s",
		    prefix, toplevel,
		    prefix, smiTableNode->name,
		    prefix, smiEntryNode->name);
    }

    return s;
}


static void
smi2yangMeta(SmiModule *smiModule, _YangNode *yangModulePtr,
	     _YangModuleInfo *yangModuleInfoPtr)
{
    _YangNode *node;

    if (smiModule->organization) {
	node = addYangNode(smiModule->organization,
			   YANG_DECL_ORGANIZATION, yangModulePtr);
	yangModuleInfoPtr->organization = node->export.value;
    }

    if (smiModule->contactinfo) {
	node = addYangNode(smiModule->contactinfo,
			   YANG_DECL_CONTACT, yangModulePtr);
	yangModuleInfoPtr->contact = node->export.value;
    }

    if (smiModule->description) {
	node = addYangNode(smiModule->description,
			   YANG_DECL_DESCRIPTION, yangModulePtr);
	setDescription(yangModulePtr, smiModule->description);
    }
}


static void
smi2yangRevisions(SmiModule *smiModule, _YangNode *yangModulePtr)
{
    SmiRevision *smiRevision;
    _YangNode *node = NULL;
    
    for (smiRevision = smiGetFirstRevision(smiModule);
	smiRevision; smiRevision = smiGetNextRevision(smiRevision)) {
	node = addYangNode(getStringDate(smiRevision->date),
			   YANG_DECL_REVISION, yangModulePtr);
	if (smiRevision->description) {
	    addYangNode(smiRevision->description,
			YANG_DECL_DESCRIPTION, node);
	}
    }
}


static void
smi2yangUnits(_YangNode *node, char *units)
{
    if (units) {
	(void) addYangNode(units, YANG_DECL_UNITS, node);
    }
}


static void
smi2yangDefault(_YangNode *node, SmiValue *smiValue, SmiType *smiType)
{
    char *s;

    if (smiValue->basetype != SMI_BASETYPE_UNKNOWN) {
	s = smiValueAsString(smiValue, smiType, SMI_LANGUAGE_SMIV2);
	if (s) {
	    /* Note that printable OCTET STRING values are returned as
	       a quoted string. In this case, we drop the quotes since
	       we only care about the string value itself. */
	    int len = strlen(s);
	    if (smiValue->basetype == SMI_BASETYPE_OCTETSTRING
		&& len && s[0] == '"' && s[len-1] == '"') {
		s[len-1] = '\0';
		s++;
	    }
	    (void) addYangNode(s, YANG_DECL_SMI_DEFAULT, node);
	}
    }
}


static void
smi2yangStatus(_YangNode *node, SmiStatus status)
{
    YangStatus s = YANG_STATUS_DEFAULT_CURRENT;

    switch (status) {
    case SMI_STATUS_UNKNOWN:
    case SMI_STATUS_CURRENT:
    case SMI_STATUS_MANDATORY:
    case SMI_STATUS_OPTIONAL:
	return;
    case SMI_STATUS_DEPRECATED:
	s = YANG_STATUS_DEPRECATED;
	break;
    case SMI_STATUS_OBSOLETE:
	s = YANG_STATUS_OBSOLETE;
	break;
    }
    
    (void) addYangNode(yangStatusAsString(s), YANG_DECL_STATUS, node);
    setStatus(node, s);
}


static void
smi2yangDescription(_YangNode *node, char *description)
{
    if (description) {
	(void) addYangNode(description, YANG_DECL_DESCRIPTION, node);
	setDescription(node, description);
    }
}


static void
smi2yangReference(_YangNode *node, char *reference)
{
    if (reference) {
	(void) addYangNode(reference, YANG_DECL_REFERENCE, node);
	setDescription(node, reference);
    }
}


static void
smi2yangAccess(_YangNode *node, SmiAccess access)
{
    char *s;

    s = smiAccessAsString(access);
    if (s) {
	(void) addYangNode(s, YANG_DECL_SMI_MAX_ACCESS, node);
    }
}


static void
smi2yangOID(_YangNode *node, SmiSubid *oid, unsigned int oidlen)
{
    char *s;
    
    s = smiRenderOID(oidlen, oid, 0);
    (void) addYangNode(s, YANG_DECL_SMI_OID, node);
    smiFree(s);
}

static void
smi2yangAlias(_YangNode *node, SmiNode *smiNode)
{
    _YangNode *alias;

    if (smiNode && smiNode->name) {
	alias = addYangNode(smiNode->name, YANG_DECL_SMI_ALIAS, node);
	smi2yangStatus(alias, smiNode->status);
	smi2yangDescription(alias, smiNode->description);
	smi2yangReference(alias, smiNode->reference);
	smi2yangOID(alias, smiNode->oid, smiNode->oidlen);
    }
}


static void
smi2yangFormat(_YangNode *node, const char *format)
{
    if (format) {
	(void) addYangNode(format, YANG_DECL_SMI_DISPLAY_HINT, node);
    }
}


static void
smi2yangImplied(_YangNode *node, const char *object)
{
    if (object) {
	(void) addYangNode(object, YANG_DECL_SMI_IMPLIED, node);
    }
}


static void
smi2yangSubtype(_YangNode *node, SmiType *smiType)
{
    if ((smiType->basetype == SMI_BASETYPE_ENUM) ||
	(smiType->basetype == SMI_BASETYPE_BITS)) {
	SmiNamedNumber *nn;
	_YangNode *nam_node = NULL;
	YangDecl nam_kind, val_kind;
	nam_kind = (smiType->basetype == SMI_BASETYPE_BITS)
	    ? YANG_DECL_BIT : YANG_DECL_ENUM;
	val_kind = (smiType->basetype == SMI_BASETYPE_BITS)
	    ? YANG_DECL_POSITION : YANG_DECL_VALUE;
	for (nn = smiGetFirstNamedNumber(smiType);
	     nn; nn = smiGetNextNamedNumber(nn)) {
	    nam_node = addYangNode(nn->name, nam_kind, node);
	    if (nam_node) {
		addYangNode(smiValueAsString(&nn->value, smiType,
					     SMI_LANGUAGE_YANG),
			    val_kind, nam_node);
	    }
	}
    } else {
	SmiRange *range;
	char *sn = NULL, *so = NULL;
	for (range = smiGetFirstRange(smiType);
	     range; range = smiGetNextRange(range)) {
	    smiAsprintf(&sn, "%s%s%s",
			so ? so : "",
			so ? "|" : "",
			smiValueAsString(&range->minValue, smiType,
					 SMI_LANGUAGE_YANG));
	    if (sn && memcmp(&range->minValue, &range->maxValue,
			     sizeof(SmiValue))) {
		char *st;
		smiAsprintf(&st, "%s..%s",
			    sn,
			    smiValueAsString(&range->maxValue, smiType,
					     SMI_LANGUAGE_YANG));
		free(sn);
		sn = st;
	    }
	    if (so) free(so);
	    so = sn;
	    sn = NULL;
	}
	if (so) {
	    if (smiType->basetype == SMI_BASETYPE_OCTETSTRING) {
		/*
		 * We should try to translate meaningfully here if
		 * possible, at least for frequent simple cases.
		 */
	        (void) addYangNode(so, YANG_DECL_LENGTH, node);
	    } else {
		(void) addYangNode(so, YANG_DECL_RANGE, node);
	    }
	    free(so);
	}
    }

#if 0
    /*
     * We should try to translate meaningfully here if
     * possible, at least for frequent simple cases.
     */
    if (smiType->format
	&& smiType->basetype == SMI_BASETYPE_OCTETSTRING) {
	char *pattern;
	pattern = smiFormatToPattern(smiType->format,
				     smiGetFirstRange(smiType));
	if (pattern) {
	    (void) addYangNode(pattern, YANG_DECL_PATTERN, node);
	}
    }
#endif
}


static _YangNode*
smi2yangType(_YangNode *node, SmiType *smiType, int hasFormat)
{
    const char *typeModule = NULL, *typeName = NULL;
    SmiModule *smiModule;
    int i;
    _YangNode *typeNode = NULL;

    if (! smiType) return NULL;

    smiModule = smiGetTypeModule(smiType);

    if (smiType && ! smiType->name) {
	return smi2yangType(node, smiGetParentType(smiType), hasFormat);
    }

    for (i = 0; convertType[i]; i += 4) {
	if (strcmp(smiModule->name, convertType[i]) == 0
	    && strcmp(smiType->name, convertType[i+1]) == 0) {
	    typeModule = convertType[i+2];
	    typeName = convertType[i+3];
	    break;
	}
    }

    if (! typeName) {
	typeModule = smiModule->name;
	typeName = smiType->name;
    }

    if (typeModule) {
	typeModule = getModulePrefix(typeModule);
    }

    /*
     * We handle a special case here. If we have a format string and
     * the type is binary, we turn it into string.
     */
    
    if (! typeModule && typeName && strcmp(typeName, "binary") == 0) {
	if (hasFormat) {
	    typeName = "string";
	}
    }
	
    if (typeModule && typeName) {
	char *s;
	smiAsprintf(&s, "%s:%s", typeModule, typeName);
	typeNode = addYangNode(s, YANG_DECL_TYPE, node);
	smiFree(s);
    } else {
	typeNode = addYangNode(typeName, YANG_DECL_TYPE, node);
	createTypeInfo(typeNode);
    }

    return typeNode;
}


static void
smi2yangTypedefs(SmiModule *smiModule, _YangNode *yangModulePtr)
{
    SmiType *smiType, *baseType;
    _YangNode *node, *typeNode;

    for (smiType = smiGetFirstType(smiModule);
	 smiType; smiType = smiGetNextType(smiType)) {

	baseType = smiGetParentType(smiType);

	node = addYangNode(smiType->name, YANG_DECL_TYPEDEF, yangModulePtr);
	typeNode = smi2yangType(node, baseType, smiType->format != NULL);
	if (typeNode) {
	    smi2yangSubtype(typeNode, smiType);
	}
	smi2yangUnits(node, smiType->units);
	smi2yangStatus(node, smiType->status);
	smi2yangDescription(node, smiType->description);
	smi2yangReference(node, smiType->reference);
	smi2yangFormat(node, smiType->format);
	smi2yangDefault(node, &smiType->value, smiType);
    }
}


static void
smi2yangIdentity(_YangNode *node, SmiNode *smiNode)
{
    _YangNode *identityNode;

    if (! smiNode) {
	return;
    }

    identityNode = addYangNode(smiNode->name, YANG_DECL_IDENTITY, node);
    (void) addYangNode("smiv2:object-identity", YANG_DECL_BASE, identityNode);
    smi2yangStatus(identityNode, smiNode->status);
    smi2yangDescription(identityNode, smiNode->description);
    smi2yangReference(identityNode, smiNode->reference);
    smi2yangOID(identityNode, smiNode->oid, smiNode->oidlen);
}


static void
smi2yangIdentities(SmiModule *smiModule, _YangNode *yangModulePtr)
{
    SmiNode *smiNode;

    for (smiNode = smiGetFirstNode(smiModule, SMI_NODEKIND_NODE);
	 smiNode;
	 smiNode = smiGetNextNode(smiNode, SMI_NODEKIND_NODE)) {
	
	if (smiNode == smiGetModuleIdentityNode(smiModule)) {
	    continue;
	}
	
	if (smiNode->status != SMI_STATUS_UNKNOWN) {
	    smi2yangIdentity(yangModulePtr, smiNode);
	}
    }
}


static void
smi2yangLeaf(_YangNode *container, SmiNode *smiNode, const char *altname)
{
    _YangNode *node, *typeNode;
    SmiType *smiType;
    
    smiType = smiGetNodeType(smiNode);

    if (! altname) altname = smiNode->name;
    node = addYangNode(altname, YANG_DECL_LEAF, container);

    if (smiType && ! smiType->name) {
	SmiType *baseType;
	/*
	 * YANG (RFC 6020) does not allow subtyping of enumerations
	 * and bits. Hence, we have to special case them here. Ugly.
	 */
	if (smiType->basetype == SMI_BASETYPE_ENUM
	    || smiType->basetype == SMI_BASETYPE_BITS) {
	    for (baseType = smiType;
		 smiGetParentType(baseType);
		 baseType = smiGetParentType(baseType)) ;
	    typeNode = smi2yangType(node, baseType, 1);
	} else {
	    typeNode = smi2yangType(node, smiType,
				    smiType && smiType->format != NULL);
	}
	if (smiType && ! smiType->name) {
	    smi2yangSubtype(typeNode, smiType);
	}
    } else {
	typeNode = smi2yangType(node, smiType,
				smiType && smiType->format != NULL);
    }
    smi2yangUnits(node, smiNode->units);
    smi2yangStatus(node, smiNode->status);
    smi2yangDescription(node, smiNode->description);
    smi2yangReference(node, smiNode->reference);
    smi2yangFormat(node, smiNode->format);
    smi2yangDefault(node, &smiNode->value, smiType);
    smi2yangAccess(node, smiNode->access);
    smi2yangOID(node, smiNode->oid, smiNode->oidlen);
}


static void
smi2yangLeafrefLeaf(_YangNode *node, SmiNode *smiNode, char *altname)
{
    _YangNode *leafNode, *typeNode;
    char *s;

    if (! altname) altname = smiNode->name;
    leafNode = addYangNode(altname, YANG_DECL_LEAF, node);

    typeNode = addYangNode("leafref", YANG_DECL_TYPE, leafNode);
    s = smi2yangLeafPath(smiNode);
    (void) addYangNode(s, YANG_DECL_PATH, typeNode);
    smiFree(s);
    smi2yangStatus(leafNode, smiNode->status);
}


static void
smi2yangKey(_YangNode *node, SmiNode *smiNode)
{
    SmiElement *smiElement, *chkElement;
    SmiElement *lastElement = NULL;
    char *s = NULL, *o = NULL;
    int i, c, n;

    /*
     * We need to detect duplicate names and translate them so that
     * they are unique.
     */

    for (c = 0, smiElement = smiGetFirstElement(smiNode); smiElement;
	 smiElement = smiGetNextElement(smiElement), c++) {

	for (i = 0, n = 0, chkElement = smiGetFirstElement(smiNode);
	     chkElement && i < c;
	     chkElement = smiGetNextElement(chkElement), i++) {
	    if (strcmp(smiGetElementNode(smiElement)->name,
		       smiGetElementNode(chkElement)->name) == 0) {
		n++;
	    }
	}
	
	o = s;
	if (n > 0) {
	    smiAsprintf(&s, "%s%s%s_%d", o ? o : "", c ? " " : "",
			smiGetElementNode(smiElement)->name, n+1);
	} else {
	    smiAsprintf(&s, "%s%s%s", o ? o : "", c ? " " : "",
			smiGetElementNode(smiElement)->name);
	}
	if (o) smiFree(o);
	lastElement = smiElement;
    }

    if (s) {
	(void) addYangNode(s, YANG_DECL_KEY, node);
	smiFree(s);
    }

    if (s && smiNode->implied && lastElement) {
	smi2yangImplied(node, smiGetElementNode(lastElement)->name);
    }
}


static _YangNode*
smi2yangToplevel(SmiModule *smiModule, _YangNode *node)
{
    SmiNode *smiNode;

    /*
     * Check whether we have any scalars or non augmenting tables
     * defined in the SMIv2 module - otherwise we can prune the
     * container.
     */

    smiNode = smiGetFirstNode(smiModule, SMI_NODEKIND_SCALAR);
    if (! smiNode) {
	for (smiNode = smiGetFirstNode(smiModule, SMI_NODEKIND_ROW);
	     smiNode;
	     smiNode = smiGetNextNode(smiNode, SMI_NODEKIND_ROW)) {
	    if (smiNode->indexkind != SMI_INDEX_AUGMENT) {
		break;
	    }
	}
    }

    if (! smiNode) {
	return NULL;
    }
    
    /* Generate the top-level container. We always use the module
     * name since this works both for SMIv1 and SMIv2 and does not
     * cause issues if an SMIv1 modules gets converted to SMIv2. */

    node = addYangNode(smiModule->name, YANG_DECL_CONTAINER, node);
    (void) addYangNode("false", YANG_DECL_CONFIG, node);
    return node;
}


static void
smi2yangColumnarLeafs(_YangNode *node, SmiNode *smiNode)
{
    SmiNode *childNode;
    SmiElement *smiElement;
    int c, i;
    char *s = NULL;
    
    for (childNode = smiGetFirstChildNode(smiNode);
	 childNode;
	 childNode = smiGetNextChildNode(childNode)) {
	if (childNode->nodekind != SMI_NODEKIND_COLUMN) {
	    continue;
	}

	/*
	 * Check whether this column belongs to the table index. If
	 * so, we have to create a leaf regardless of the max-access;
	 * there apparently are tables with accessible-for-notify
	 * index objects. Furthermore, we have to handle tables where
	 * the same column appears multiple times in the index by
	 * creating multiple leafs. Hence, we first count how many
	 * times the column appears in the index.
	 */

	for (c = 0, smiElement = smiGetFirstElement(smiNode);
	     smiElement;
	     smiElement = smiGetNextElement(smiElement)) {
	    if (childNode->name && smiGetElementNode(smiElement)->name
		&& strcmp(childNode->name,
			  smiGetElementNode(smiElement)->name) == 0) {
		c++;
	    }
	}

	/*
	 * If the column is not in the index but it is accessible,
	 * we have to create a single leaf. This is the default case.
	 */

	if (c == 0 && isAccessibleLeaf(childNode)) {
	    c = 1;
	}

	for (i = 0; i < c; i++) {
	    if (i) {
		smiAsprintf(&s, "%s_%d", childNode->name, i+1);
		smi2yangLeaf(node, childNode, s);
		smiFree(s);
	    } else {
		smi2yangLeaf(node, childNode, NULL);
	    }
	}
    }
}

static void
smi2yangScalarLeafs(_YangNode *node, SmiNode *smiNode)
{
    SmiNode *childNode;

    for (childNode = smiGetFirstChildNode(smiNode);
	 childNode;
	 childNode = smiGetNextChildNode(childNode)) {
	if (childNode->nodekind != SMI_NODEKIND_SCALAR) {
	    continue;
	}
	if (isAccessibleLeaf(childNode)) {
	    smi2yangLeaf(node, childNode, NULL);
	}
    }
}


static void
smi2yangScalars(SmiModule *smiModule, _YangNode *node)
{
    SmiNode *smiNode, *parentNode, *aliasNode;
    _YangNode *container;
    int i, cnt;
    SmiNode **groups = NULL;

    for (cnt = 0, smiNode = smiGetFirstNode(smiModule, SMI_NODEKIND_SCALAR);
	 smiNode;
	 smiNode = smiGetNextNode(smiNode, SMI_NODEKIND_SCALAR)) {
	cnt++;
    }

    if (cnt == 0) {
	return;
    }

    groups = (SmiNode **) smiMalloc(sizeof(SmiNode) * cnt);
    memset(groups, 0, sizeof(SmiNode) * cnt);
    
    for (cnt = 0, smiNode = smiGetFirstNode(smiModule, SMI_NODEKIND_SCALAR);
	 smiNode;
	 smiNode = smiGetNextNode(smiNode, SMI_NODEKIND_SCALAR)) {
	parentNode = smiGetParentNode(smiNode);
	for (aliasNode = smiGetFirstAlias(parentNode);
	     aliasNode; aliasNode = smiGetNextAlias(aliasNode)) {
	    if (aliasNode != parentNode) {
		smiPrintError(NULL, ERR_SMI2YANG_NON_UNIQUE_NAME,
			      parentNode->name, aliasNode->name,
			      parentNode->name);
	    }
	}
	if (parentNode && isAccessibleLeaf(smiNode)) {
	    for (i = 0; i < cnt; i++) {
		if (groups[i] == parentNode) {
		    break;
		}
	    }
	    if (! (i < cnt)) groups[cnt++] = parentNode;
	}
    }

    for (cnt = 0; groups[cnt]; cnt++) {
	smiNode = groups[cnt];
	container = addYangNode(smiNode->name, YANG_DECL_CONTAINER, node);
	smi2yangStatus(container, smiNode->status);
	smi2yangDescription(container, smiNode->description);
	smi2yangReference(container, smiNode->reference);
	smi2yangOID(container, smiNode->oid, smiNode->oidlen);
	smi2yangScalarLeafs(container, smiNode);
    }

    smiFree(groups);
}


static void
smi2yangTable(_YangNode *node, SmiNode *smiTableNode, SmiNode *smiEntryNode)
{
    _YangNode *tableNode, *listNode;
    SmiElement *smiElement, *chkElement;
    SmiNode *childNode, *parentNode;
    int i, c, n;

    if (!smiTableNode || !smiEntryNode) {
	return;
    }
    
    tableNode = addYangNode(smiTableNode->name, YANG_DECL_CONTAINER, node);
    
    smi2yangStatus(tableNode, smiTableNode->status);
    smi2yangDescription(tableNode, smiTableNode->description);
    smi2yangReference(tableNode, smiTableNode->reference);
    smi2yangOID(tableNode, smiTableNode->oid, smiTableNode->oidlen);

    listNode = addYangNode(smiEntryNode->name, YANG_DECL_LIST, tableNode);

    smi2yangKey(listNode, smiEntryNode);
    smi2yangStatus(listNode, smiEntryNode->status);
    smi2yangDescription(listNode, smiEntryNode->description);
    smi2yangReference(listNode, smiEntryNode->reference);
    smi2yangOID(listNode, smiEntryNode->oid, smiEntryNode->oidlen);

    /*
     * Add leafref nodes for foreign key elements. We have to be
     * careful with leafref nodes appearing multiple times and
     * disambiguate the names.
     */

    for (c = 0, smiElement = smiGetFirstElement(smiEntryNode);
	 smiElement;
	 smiElement = smiGetNextElement(smiElement), c++) {

	for (i = 0, n = 0, chkElement = smiGetFirstElement(smiEntryNode);
	     chkElement && i < c;
	     chkElement = smiGetNextElement(chkElement), i++) {
	    if (strcmp(smiGetElementNode(smiElement)->name,
		       smiGetElementNode(chkElement)->name) == 0) {
		n++;
	    }
	}
	
	childNode = smiGetElementNode(smiElement);
	parentNode = smiGetParentNode(childNode);
        if (childNode->nodekind == SMI_NODEKIND_COLUMN
            && parentNode != smiEntryNode) {
	    if (n > 0) {
		char *s = NULL;
		smiAsprintf(&s, "%s_%d", childNode->name, n+1);
		smi2yangLeafrefLeaf(listNode, childNode, s);
		smiFree(s);
	    } else {
		smi2yangLeafrefLeaf(listNode, childNode, NULL);
	    }
	}
    }
    
    smi2yangColumnarLeafs(listNode, smiEntryNode);
}


static void
smi2yangTables(SmiModule *smiModule, _YangNode *node)
{
    SmiNode *smiTableNode, *smiEntryNode;

    for (smiTableNode = smiGetFirstNode(smiModule, SMI_NODEKIND_TABLE);
	 smiTableNode;
	 smiTableNode = smiGetNextNode(smiTableNode, SMI_NODEKIND_TABLE)) {
	smiEntryNode = smiGetFirstChildNode(smiTableNode);
	if (smiEntryNode) {
	    switch (smiEntryNode->indexkind) {
	    case SMI_INDEX_INDEX:
	    case SMI_INDEX_REORDER:
	    case SMI_INDEX_SPARSE:
	    case SMI_INDEX_EXPAND:
		smi2yangTable(node, smiTableNode, smiEntryNode);
		break;
	    default:
		break;
	    }
	}
    }
}


static void
smi2yangAugment(_YangNode *node, SmiNode *smiNode)
{
    SmiNode *baseEntryNode = NULL;
    SmiNode *parentNode = NULL;
    _YangNode *augmentNode;
    char *s;

    if (! smiNode) {
	return;
    }
    
    baseEntryNode = smiGetRelatedNode(smiNode);
    if (! baseEntryNode) {
        return;
    }

    parentNode = smiGetParentNode(smiNode);
    if (! parentNode) {
	return;
    }

    smi2yangAlias(node, parentNode);
    smi2yangAlias(node, smiNode);

    s = smi2yangListPath(baseEntryNode);
    augmentNode = addYangNode(s, YANG_DECL_AUGMENT, node);
    smiFree(s); 

    smi2yangStatus(augmentNode, smiNode->status);
    smi2yangDescription(augmentNode, smiNode->description);
    smi2yangReference(augmentNode, smiNode->reference);
    smi2yangOID(augmentNode, smiNode->oid, smiNode->oidlen);

    smi2yangColumnarLeafs(augmentNode, smiNode);
}


static void
smi2yangAugments(SmiModule *smiModule, _YangNode *node)
{
    SmiNode *smiNode;

    for (smiNode = smiGetFirstNode(smiModule, SMI_NODEKIND_ROW);
	 smiNode;
	 smiNode = smiGetNextNode(smiNode, SMI_NODEKIND_ROW)) {
	if (smiNode->indexkind == SMI_INDEX_AUGMENT) {
	    smi2yangAugment(node, smiNode);
	}
    }
}


static void
smi2yangNotificationIndex(_YangNode *node,
			  SmiNode *entryNode, SmiNode *ignoreNode)
{
    SmiElement *smiElement;
    SmiNode *childNode;

    for (smiElement = smiGetFirstElement(entryNode); smiElement;
	 smiElement = smiGetNextElement(smiElement)) {
	childNode = smiGetElementNode(smiElement);
	if (childNode != ignoreNode) {
	    smi2yangLeafrefLeaf(node, childNode, NULL);
	}
    }
}


static void
smi2yangNotification(_YangNode *container, SmiNode *smiNode)
{
    SmiElement *smiElement;
    SmiNode *vbNode, *entryNode;
    _YangNode *node, *conti;
    int cnt;
    char *s;

    node = addYangNode(smiNode->name, YANG_DECL_NOTIFICATION, container);
    smi2yangStatus(node, smiNode->status);
    smi2yangDescription(node, smiNode->description);
    smi2yangReference(node, smiNode->reference);
    smi2yangOID(node, smiNode->oid, smiNode->oidlen);

    for (smiElement = smiGetFirstElement(smiNode), cnt = 1; smiElement;
	 smiElement = smiGetNextElement(smiElement), cnt++) {
	vbNode = smiGetElementNode(smiElement);
	if (! vbNode) continue;

	entryNode = (vbNode->nodekind == SMI_NODEKIND_COLUMN)
	    ? smiGetParentNode(vbNode) : NULL;

	smiAsprintf(&s, "object-%d", cnt);
	conti = addYangNode(s, YANG_DECL_CONTAINER, node);
	smiFree(s);

	if (entryNode) {
	    switch (entryNode->indexkind) {
	    case SMI_INDEX_INDEX:
		smi2yangNotificationIndex(conti, entryNode, vbNode);
		break;
	    case SMI_INDEX_AUGMENT:
		smi2yangNotificationIndex(conti,
				  smiGetRelatedNode(entryNode), vbNode);
		break;
	    default:
		break;
	    }
	}

	if (isAccessibleLeaf(vbNode)) {
	    smi2yangLeafrefLeaf(conti, vbNode, NULL);
	} else {
	    smi2yangLeaf(conti, vbNode, NULL);
	}
    }
}


static void
smi2yangNotifications(SmiModule *smiModule, _YangNode *yangModulePtr)
{
    SmiNode *smiNode;

    for (smiNode = smiGetFirstNode(smiModule, SMI_NODEKIND_NOTIFICATION);
	 smiNode;
	 smiNode = smiGetNextNode(smiNode, SMI_NODEKIND_NOTIFICATION)) {
	smi2yangNotification(yangModulePtr, smiNode);
    }
}


static void
smi2yangAliases(SmiModule *smiModule, _YangNode *node)
{
    SmiNode *smiNode;

    for (smiNode = smiGetFirstNode(smiModule, SMI_NODEKIND_ANY);
	 smiNode;
	 smiNode = smiGetNextNode(smiNode, SMI_NODEKIND_ANY)) {
	switch (smiNode->nodekind) {
	case SMI_NODEKIND_NODE:
	    smi2yangAlias(node, smiNode);
	    break;
	default:
	    break;
	}
    }
}


YangNode*
yangGetModuleFromSmiModule(SmiModule *smiModule, int flags)
{
    _YangNode       *yangModulePtr = NULL;
    _YangNode	    *yangToplevelPtr = NULL;
    _YangModuleInfo *yangModuleInfoPtr = NULL;

    createImportList(smiModule);

    yangModulePtr = addYangNode(smiModule->name, YANG_DECL_MODULE, NULL);
    yangModuleInfoPtr = createModuleInfo(yangModulePtr);

    smi2yangNamespace(smiModule, yangModulePtr, yangModuleInfoPtr);
    smi2yangImports(smiModule, yangModulePtr);
    smi2yangMeta(smiModule, yangModulePtr, yangModuleInfoPtr);
    smi2yangRevisions(smiModule, yangModulePtr);
    
    /* body statements */

    smi2yangTypedefs(smiModule, yangModulePtr);
    smi2yangIdentities(smiModule, yangModulePtr);
    
    yangToplevelPtr = smi2yangToplevel(smiModule, yangModulePtr);
    if (yangToplevelPtr) {
	smi2yangScalars(smiModule, yangToplevelPtr);
	smi2yangTables(smiModule, yangToplevelPtr);
    }
    
    smi2yangAugments(smiModule, yangModulePtr);
    smi2yangNotifications(smiModule, yangModulePtr);

    smi2yangAliases(smiModule, yangModulePtr);
    
    yangModuleInfoPtr->parsingState = YANG_PARSING_DONE;
    freeImportList();
#if 0
    /* oops - this expands as a side effect augments */
    yangSemanticAnalysis(yangModulePtr);
#endif
    return &(yangModulePtr->export);
}
