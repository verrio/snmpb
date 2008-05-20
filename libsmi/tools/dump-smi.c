/*
 * dump-smi.c --
 *
 *      Operations to dump SMIv1/v2 module information.
 *
 * Copyright (c) 1999 Frank Strauss, Technical University of Braunschweig.
 *
 * See the file "COPYING" for information on usage and redistribution
 * of this file, and for a DISCLAIMER OF ALL WARRANTIES.
 *
 * @(#) $Id: dump-smi.c 8090 2008-04-18 12:56:29Z strauss $
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



extern int smiAsprintf(char **strp, const char *format, ...);



#define  INDENT		4    /* indent factor */
#define  INDENTVALUE	16   /* column to start values, except multiline */
#define  INDENTTEXTS	 9   /* column to start multiline texts */
#define  INDENTTEXTS2	15   /* column to start indented multiline texts */
#define  INDENTMAX	72   /* max column to fill, break lines otherwise */



static char *convertTypev1[] = {
    NULL,		 "Counter32",	       NULL,	   "Counter",
    NULL,		 "Gauge32",	       NULL,	   "Gauge",
    NULL,		 "Integer32",	       NULL,	   "INTEGER",
    NULL,		 "Unsigned32",	       NULL,	   "Gauge",
    NULL,		 "Bits",	       NULL,	   "OCTET STRING",
    NULL,		 "OctetString",	       NULL,	   "OCTET STRING",
    NULL,		 "ObjectIdentifier",   NULL,	   "OBJECT IDENTIFIER",
    NULL, NULL, NULL, NULL };


static char *convertTypev2[] = {
    NULL,		 "INTEGER",	       NULL,	   "Integer32",
    NULL,		 "Enumeration",	       NULL,	   "INTEGER",
    NULL,		 "Bits",	       NULL,	   "BITS",
    NULL,		 "OctetString",	       NULL,	   "OCTET STRING",
    NULL,		 "ObjectIdentifier",   NULL,	   "OBJECT IDENTIFIER",
    NULL,		 "Counter",	       NULL,	   "Counter32",
    NULL,		 "Gauge",	       NULL,	   "Gauge32",
    NULL, NULL, NULL, NULL };

static char *convertTypePIBtoMIB[] = {
    NULL,		 "Unsigned64",	       NULL,	   "IBRUnsigned64",
    NULL,		 "Integer64",	       NULL,	   "IBRInteger64",
/* XXX This breaks SMIv2 -> SMIv? compilation such that the IMPORT statement
   for SnmpAdminString is removed. I've remove this as a quick fix, but I'm
   not sure what to do for PIB compilation here.
    NULL,		 "SnmpAdminString",    NULL,	   "OCTET STRING",
*/
    NULL, NULL, NULL, NULL };

static char *convertImportv2[] = {
    "IRTF-NMRG-SMING-TYPES", "IpAddress",       "SNMPv2-SMI", "IpAddress",
    "IRTF-NMRG-SMING-TYPES", "Counter32",       "SNMPv2-SMI", "Counter32",
    "IRTF-NMRG-SMING-TYPES", "Gauge32",	        "SNMPv2-SMI", "Gauge32",
    "IRTF-NMRG-SMING-TYPES", "TimeTicks",       "SNMPv2-SMI", "TimeTicks",
    "IRTF-NMRG-SMING-TYPES", "Opaque",	        "SNMPv2-SMI", "Opaque",
    "IRTF-NMRG-SMING-TYPES", "Counter64",       "SNMPv2-SMI", "Counter64",
    "IRTF-NMRG-SMING-TYPES", "DisplayString",   "SNMPv2-TC", "DisplayString",
    "IRTF-NMRG-SMING-TYPES", "PhysAddress",     "SNMPv2-TC", "PhysAddress",
    "IRTF-NMRG-SMING-TYPES", "MacAddress",      "SNMPv2-TC", "MacAddress",
    "IRTF-NMRG-SMING-TYPES", "TruthValue",      "SNMPv2-TC", "TruthValue",
    "IRTF-NMRG-SMING-TYPES", "TestAndIncr",     "SNMPv2-TC", "TestAndIncr",
    "IRTF-NMRG-SMING-TYPES", "AutonomousType",  "SNMPv2-TC", "AutonomousType",
    "IRTF-NMRG-SMING-TYPES", "InstancePointer", "SNMPv2-TC", "InstancePointer",
    "IRTF-NMRG-SMING-TYPES", "VariablePointer", "SNMPv2-TC", "VariablePointer",
    "IRTF-NMRG-SMING-TYPES", "RowPointer",      "SNMPv2-TC", "RowPointer",
    "IRTF-NMRG-SMING-TYPES", "RowStatus",       "SNMPv2-TC", "RowStatus",
    "IRTF-NMRG-SMING-TYPES", "TimeStamp",       "SNMPv2-TC", "TimeStamp",
    "IRTF-NMRG-SMING-TYPES", "TimeInterval",    "SNMPv2-TC", "TimeInterval",
    "IRTF-NMRG-SMING-TYPES", "DateAndTime",     "SNMPv2-TC", "DateAndTime",
    "IRTF-NMRG-SMING-TYPES", "StorageType",     "SNMPv2-TC", "StorageType",
    "IRTF-NMRG-SMING-TYPES", "TDomain",         "SNMPv2-TC", "TDomain",
    "IRTF-NMRG-SMING-TYPES", "TAddress",        "SNMPv2-TC", "TAddress",
    "IRTF-NMRG-SMING", NULL,                    "SNMPv2-SMI", NULL,

    "RFC1155-SMI", "internet",	    "SNMPv2-SMI", "internet",
    "RFC1155-SMI", "directory",	    "SNMPv2-SMI", "directory",
    "RFC1155-SMI", "mgmt",	    "SNMPv2-SMI", "mgmt",
    "RFC1155-SMI", "experimental",  "SNMPv2-SMI", "experimental",
    "RFC1155-SMI", "private",	    "SNMPv2-SMI", "private",
    "RFC1155-SMI", "enterprises",   "SNMPv2-SMI", "enterprises",
    "RFC1155-SMI", "IpAddress",     "SNMPv2-SMI", "IpAddress",
    "RFC1155-SMI", "Counter",       "SNMPv2-SMI", "Counter32",
    "RFC1155-SMI", "Gauge",         "SNMPv2-SMI", "Gauge32",
    "RFC1155-SMI", "TimeTicks",     "SNMPv2-SMI", "TimeTicks",
    "RFC1155-SMI", "Opaque",        "SNMPv2-SMI", "Opaque",
    "RFC1213-MIB", "mib-2",         "SNMPv2-SMI", "mib-2",    
    "RFC1213-MIB", "DisplayString", "SNMPv2-TC",  "DisplayString",    

/* import types for PIB to MIB conversion */

    NULL, NULL, NULL, NULL };

static char *convertImportv1[] = {
    "IRTF-NMRG-SMING-TYPES", "IpAddress",   "RFC1155-SMI", "IpAddress",
    "IRTF-NMRG-SMING-TYPES", "Counter32",   "RFC1155-SMI", "Counter",
    "IRTF-NMRG-SMING-TYPES", "Gauge32",	    "RFC1155-SMI", "Gauge",
    "IRTF-NMRG-SMING-TYPES", "TimeTicks",   "RFC1155-SMI", "TimeTicks",
    "IRTF-NMRG-SMING-TYPES", "Opaque",	    "RFC1155-SMI", "Opaque",
    "IRTF-NMRG-SMING-TYPES", "Counter64",   NULL,          NULL,
    "IRTF-NMRG-SMING",       "mib-2",	    "RFC1213-MIB", "mib-2",
    
    "SNMPv2-SMI", "IpAddress",              "RFC1155-SMI", "IpAddress",
    "SNMPv2-SMI", "Counter32",              "RFC1155-SMI", "Counter",
    "SNMPv2-SMI", "Gauge32",	            "RFC1155-SMI", "Gauge",
    "SNMPv2-SMI", "TimeTicks",              "RFC1155-SMI", "TimeTicks",
    "SNMPv2-SMI", "Opaque",	            "RFC1155-SMI", "Opaque",
    "SNMPv2-SMI", "Integer32",	            NULL,	   NULL,
    "SNMPv2-SMI", "Unsigned32",	            NULL,	   NULL,
    "SNMPv2-SMI", "Counter64",	            NULL,	   NULL,
    "SNMPv2-SMI", "mib-2",                  "RFC1213-MIB", "mib-2",
    "SNMPv2-SMI", "MODULE-IDENTITY",        NULL,	   NULL,
    "SNMPv2-SMI", "OBJECT-IDENTITY",        NULL,	   NULL,
    "SNMPv2-SMI", "ObjectName",		    NULL,	   NULL,
    "SNMPv2-SMI", "NotificationName",       NULL,	   NULL,
    "SNMPv2-SMI", "ObjectSyntax",	    NULL,	   NULL,
    "SNMPv2-SMI", "SimpleSyntax",	    NULL,	   NULL,
    "SNMPv2-SMI", "ApplicationSyntax",      NULL,	   NULL,
    "SNMPv2-SMI", "OBJECT-TYPE",	    NULL,	   NULL,
    "SNMPv2-SMI", "NOTIFICATION-TYPE",      NULL,	   NULL,
    NULL, NULL, NULL, NULL };

static int current_column = 0;
static int smiv1 = 0;
static int silent = 0;
static int pibtomib = 0;


typedef struct Import {
    char          *module;
    char          *name;
    struct Import *nextPtr;
} Import;

static Import *importList = NULL;



static int invalidType(SmiBasetype basetype)
{
    if (smiv1) {
	return (basetype == SMI_BASETYPE_INTEGER64)
	    || (basetype == SMI_BASETYPE_UNSIGNED64)
	    || (basetype == SMI_BASETYPE_FLOAT32)
	    || (basetype == SMI_BASETYPE_FLOAT64)
	    || (basetype == SMI_BASETYPE_FLOAT128);
    } else if (!pibtomib) {
	return (basetype == SMI_BASETYPE_INTEGER64)
	    || (basetype == SMI_BASETYPE_FLOAT32)
	    || (basetype == SMI_BASETYPE_FLOAT64)
	    || (basetype == SMI_BASETYPE_FLOAT128);
    } else {
	return (basetype == SMI_BASETYPE_FLOAT32)
	    || (basetype == SMI_BASETYPE_FLOAT64)
	    || (basetype == SMI_BASETYPE_FLOAT128);
    }
}



static char *getStatusString(SmiStatus status)
{
    if (smiv1) {
	return
	    (status == SMI_STATUS_CURRENT)     ? "mandatory" :
	    (status == SMI_STATUS_DEPRECATED)  ? "deprecated" :
	    (status == SMI_STATUS_OBSOLETE)    ? "obsolete" :
	    (status == SMI_STATUS_MANDATORY)   ? "mandatory" :
	    (status == SMI_STATUS_OPTIONAL)    ? "optional" :
					         "<unknown>";
    } else {
	return
	    (status == SMI_STATUS_CURRENT)     ? "current" :
	    (status == SMI_STATUS_DEPRECATED)  ? "deprecated" :
	    (status == SMI_STATUS_OBSOLETE)    ? "obsolete" :
	    (status == SMI_STATUS_MANDATORY)   ? "current" :
	    (status == SMI_STATUS_OPTIONAL)    ? "current" :
					         "<unknown>";
    }
}



static char *getAccessString(SmiAccess access, int create)
{
    if (smiv1) {
	return
	    (access == SMI_ACCESS_NOT_ACCESSIBLE) ? "not-accessible" :
	    (access == SMI_ACCESS_NOTIFY)	  ? "read-only" :
	    (access == SMI_ACCESS_READ_ONLY)      ? "read-only" :
	    (access == SMI_ACCESS_READ_WRITE)     ? "read-write" :
						    "<unknown>";

    } else {
	if (create && (access == SMI_ACCESS_READ_WRITE)) {
	    return "read-create";
	} else {
            if (pibtomib)
                return
	        (access == SMI_ACCESS_NOT_ACCESSIBLE) ? "not-accessible" :
	        (access == SMI_ACCESS_NOTIFY)	      ? "read-only" :
	        (access == SMI_ACCESS_READ_ONLY)      ? "read-only" :
	        (access == SMI_ACCESS_READ_WRITE)     ? "read-write" :
	        (access == SMI_ACCESS_INSTALL)        ? "read-create" :
	        (access == SMI_ACCESS_INSTALL_NOTIFY) ? "read-create" :
						        "<unknown>";
            else
	        return
	        (access == SMI_ACCESS_NOT_ACCESSIBLE) ? "not-accessible" :
	        (access == SMI_ACCESS_NOTIFY)	  ? "accessible-for-notify" :
	        (access == SMI_ACCESS_READ_ONLY)      ? "read-only" :
	        (access == SMI_ACCESS_READ_WRITE)     ? "read-write" :
						        "<unknown>";
	}
    }
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
    char	**convertType;
    char        *typeModule, *typeName;

    typeName = smiType ? smiType->name : NULL;
    typeModule = smiType ? smiGetTypeModule(smiType)->name : NULL;

    convertType = smiv1 ? convertTypev1 : convertTypev2;

    if (typeName &&
	(basetype != SMI_BASETYPE_ENUM) &&
	(basetype != SMI_BASETYPE_BITS)) {
        if (pibtomib)
	    for(i=0; convertTypePIBtoMIB[i+1]; i += 4) {
	        if ((!strcmp(typeName, convertTypePIBtoMIB[i+1])) &&
		    ((!typeModule) || (!convertTypePIBtoMIB[i]) ||
		     (!strcmp(typeModule, convertTypePIBtoMIB[i])))) {
		    return convertTypePIBtoMIB[i+3];
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
	    if (smiv1) {
		return "OCTET STRING";
	    } else {
		return "BITS";
	    }
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
	if (parentNode->decl != SMI_DECL_IMPL_OBJECT) {
	    sprintf(append, " %u%s", parentNode->oid[parentNode->oidlen-1], s);
	} else {
	    sprintf(append, " %s(%u)%s",
		    parentNode->name,
		    parentNode->oid[parentNode->oidlen-1], s);
	}
	
	/* retrieve the parent SmiNode */
	parentNode = smiGetParentNode(parentNode);

	if (!parentNode) {
	    sprintf(s, "%s", append);
	    return s;
	}
	
	/* found an imported or a local parent node? */
	if ((parentNode->decl != SMI_DECL_IMPL_OBJECT) &&
	    ((parentNode->name && strlen(parentNode->name)) &&
	    (smiIsImported(smiModule, NULL, parentNode->name) ||
	     (!importedParent &&
	      (smiGetNodeModule(parentNode) == smiModule)) ||
	     (parentNode->oidlen == 1)))) {
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



static int isNotificationGroup(SmiNode *groupNode)
{
    SmiNode     *smiNode;
    SmiElement  *smiElement;
    
    for (smiElement = smiGetFirstElement(groupNode); smiElement;
	 smiElement = smiGetNextElement(smiElement)) {

	smiNode = smiGetElementNode(smiElement);
	
	if (smiNode->nodekind != SMI_NODEKIND_NOTIFICATION) {
	    return 0;
	}
    }

    return 1;
}



static Import* addImport(char *module, char *name)
{
    Import **import, *newImport;
    char   **convertImport;
    int    i;

    convertImport = smiv1 ? convertImportv1 : convertImportv2;

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
    
    if (pibtomib) {
        addImport("TUBS-SMI", "ibrpibtomib");
	for(smiType = smiGetFirstType(smiModule);
	    smiType; smiType = smiGetNextType(smiType)) {
	    if (smiType->basetype == SMI_BASETYPE_UNSIGNED32)
	        addImport("SNMPv2-SMI", "Unsigned32"); 
            else if (smiType->basetype == SMI_BASETYPE_UNSIGNED64)
	        addImport("TUBS-SMI", "IBRUnsigned64"); 
            else if (smiType->basetype == SMI_BASETYPE_INTEGER64)
	        addImport("TUBS-SMI", "IBRInteger64"); 
        }
        if (smiGetFirstNode(smiModule, SMI_NODEKIND_TABLE) != NULL)
            addImport("SNMPv2-TC", "RowStatus");
        for(smiNode = smiGetFirstNode(smiModule, SMI_NODEKIND_ROW);
            smiNode; smiNode = smiGetNextNode(smiNode, SMI_NODEKIND_ROW)) {
            if (smiNode->indexkind == SMI_INDEX_SPARSE) {
                SmiNode *smiNode2 = smiGetRelatedNode(smiNode);
                while (smiNode2 && (smiNode2->indexkind == SMI_INDEX_SPARSE))
                    smiNode2 = smiGetRelatedNode(smiNode2);
                if (smiNode2) {
                    smiModule2 = smiGetNodeModule(smiNode2);
                    if (smiModule != smiModule2)
                        addImport(smiModule2->name, smiNode2->name);
                }
            }
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
                size_t len = strlen(smiModule2->name);
		if (len && smiType->name) {
                    if (pibtomib && (len > 4) &&
                        !strcmp(&smiModule2->name[len - 4], "-PIB")) {
                        char *newName = xmalloc(len + 5);
                        strcpy(newName, smiModule2->name);
                        strcat(newName, "-MIB");
                        addImport(newName, smiType->name);
                    } else
		        addImport(smiModule2->name, smiType->name);
		}
	    }
	}

	if (pibtomib) {
            if (smiType && (smiType->basetype == SMI_BASETYPE_UNSIGNED32))
	        addImport("SNMPv2-SMI", "Unsigned32");
            if (smiType && (smiType->basetype == SMI_BASETYPE_UNSIGNED64))
	        addImport("TUBS-SMI", "IBRUnsigned64");
            if (smiType && (smiType->basetype == SMI_BASETYPE_INTEGER64))
	        addImport("TUBS-SMI", "IBRInteger64");
	}

	if (! smiv1 &&
	    smiType && smiType->basetype == SMI_BASETYPE_INTEGER32) {
	    addImport("SNMPv2-SMI", "Integer32");
	}

	if ((!smiv1) &&
	    (smiNode->value.basetype == SMI_BASETYPE_OBJECTIDENTIFIER) &&
	    (!strcmp(smiNode->value.value.ptr, "zeroDotZero"))) {
	    addImport("SNMPv2-SMI", "zeroDotZero");
	}
    }

    smiNode = smiGetFirstNode(smiModule,
			      SMI_NODEKIND_SCALAR | SMI_NODEKIND_COLUMN
			      | SMI_NODEKIND_TABLE | SMI_NODEKIND_ROW);
    if (smiNode) {
	addImport(smiv1 ? "RFC-1212" : "SNMPv2-SMI", "OBJECT-TYPE");
    }

    smiNode = smiGetFirstNode(smiModule, SMI_NODEKIND_NOTIFICATION);
    if (smiNode) {
	addImport(smiv1 ? "RFC-1215" : "SNMPv2-SMI",
		  smiv1 ? "TRAP-TYPE" : "NOTIFICATION-TYPE");
    }

    if (! smiv1) {
	smiNode = smiGetModuleIdentityNode(smiModule);
	if (smiNode) {
	    if (strcmp("SNMPv2-SMI", smiModule->name)) {
		addImport("SNMPv2-SMI", "MODULE-IDENTITY");
	    }
	}
    }
    
    if (! smiv1) {
	for(smiNode = smiGetFirstNode(smiModule, SMI_NODEKIND_NODE);
	    smiNode; smiNode = smiGetNextNode(smiNode, SMI_NODEKIND_NODE)) {
	    if (smiNode->status != SMI_STATUS_UNKNOWN &&
		smiNode != smiGetModuleIdentityNode(smiModule)) {
		if (strcmp("SNMPv2-SMI", smiModule->name)) {
		    addImport("SNMPv2-SMI", "OBJECT-IDENTITY");
		}
		break;
	    }
	}
    }

    if (! smiv1) {
	smiNode = smiGetFirstNode(smiModule, SMI_NODEKIND_COMPLIANCE);
	if (smiNode) {
	    if (strcmp("SNMPv2-CONF", smiModule->name)) {
		addImport("SNMPv2-CONF", "MODULE-COMPLIANCE");
	    }
	}
    }

    if (! smiv1) {
	for(smiNode = smiGetFirstNode(smiModule, SMI_NODEKIND_GROUP);
	    smiNode;
	    smiNode = smiGetNextNode(smiNode, SMI_NODEKIND_GROUP)) {
	    if (isObjectGroup(smiNode)) {
		if (strcmp("SNMPv2-CONF", smiModule->name)) {
		    addImport("SNMPv2-CONF", "OBJECT-GROUP");
		}
	    } else if (isNotificationGroup(smiNode)) {
		if (strcmp("SNMPv2-CONF", smiModule->name)) {
		    addImport("SNMPv2-CONF", "NOTIFICATION-GROUP");
		}
	    }
	}
    }

    if (! smiv1) {
	for(smiType = smiGetFirstType(smiModule);
	    smiType; smiType = smiGetNextType(smiType)) {
	    if (smiType->description) {
		if (strcmp("SNMPv2-TC", smiModule->name)) {
		    addImport("SNMPv2-TC", "TEXTUAL-CONVENTION");
		}
		break;
	    }
	}
    }

    for (smiImport = smiGetFirstImport(smiModule); smiImport;
	 smiImport = smiGetNextImport(smiImport)) {
	if (islower((int) smiImport->name[0]) ||
	    (smiImport->module && !strcmp(smiImport->module, "SNMPv2-SMI")) ||
	    (!smiv1 && smiImport->module && !strcmp(smiImport->module, "SNMPv2-TC"))) {
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



static void fprintMultilineString2(FILE *f, const char *s, const int comment)
{
    int i, len;
    
    fprintSegment(f, INDENTTEXTS2 - 1, "\"", 0, comment);
    if (s) {
	len = strlen(s);
	for (i=0; i < len; i++) {
	    putc(s[i], f);
	    current_column++;
	    if (s[i] == '\n') {
		current_column = 0;
		fprintSegment(f, INDENTTEXTS2, "", 0, comment);
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
    case SMI_BASETYPE_FLOAT32:
    case SMI_BASETYPE_FLOAT64:
    case SMI_BASETYPE_FLOAT128:
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
	if (smiv1) {
            sprintf(s, "'%*s'H", 2 * valuePtr->len, " ");
            for (i=0; i < valuePtr->len; i++) {
                sprintf(ss, "%02x", valuePtr->value.ptr[i]);
                strncpy(&s[1+2*i], ss, 2);
            }
	} else {
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
	}
	break;
    case SMI_BASETYPE_UNKNOWN:
	break;
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
    int		   i, c = comment;

    if (pibtomib && ((smiType->basetype == SMI_BASETYPE_UNSIGNED64) ||
        (smiType->basetype == SMI_BASETYPE_INTEGER64))) {
        c = 1;
        fprint(f, " -- commented out by smidump\n-- ");
    }
    c = c || (smiv1 && smiType->basetype == SMI_BASETYPE_BITS);
    
    if ((smiType->basetype == SMI_BASETYPE_ENUM) ||
	(smiType->basetype == SMI_BASETYPE_BITS)) {
	for(i = 0, nn = smiGetFirstNamedNumber(smiType);
	    nn; i++, nn = smiGetNextNamedNumber(nn)) {
	    if (i) {
		fprint(f, ", ");
	    } else {
		if (c) {
		    fprint(f, "\n");
		    fprintSegment(f, INDENT, "", INDENTVALUE, c);
		    fprint(f, "  { ");
		} else {
		    fprint(f, " { ");
		}
	    }
	    sprintf(s, "%s(%s)", nn->name,
		    getValueString(&nn->value, smiType));
	    fprintWrapped(f, INDENTVALUE + INDENT, s, c);
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
    SmiElement *smiElement, *smiFirstElement;
    int        n, j;

    for (n = 0, smiElement = smiGetFirstElement(indexNode); smiElement;
	 n++, smiElement = smiGetNextElement(smiElement));
    if (pibtomib && (n > 1))
        smiFirstElement = smiGetNextElement(smiGetFirstElement(indexNode));
    else
        smiFirstElement = smiGetFirstElement(indexNode);
    fprintSegment(f, INDENT, "INDEX", INDENTVALUE, comment);
    fprint(f, "{ ");
    for (j = 0, smiElement = smiFirstElement; smiElement;
	 j++, smiElement = smiGetNextElement(smiElement)) {
	if (j) {
	    fprint(f, ", ");
	}
	if (indexNode->implied && ((j+1) == n)) {
	    fprintWrapped(f, INDENTVALUE + 2, "IMPLIED ", 0);
	}
	fprintWrapped(f, INDENTVALUE + 2,
		      smiGetElementNode(smiElement)->name, 0);
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
    char	  **convertImport;

    convertImport = smiv1 ? convertImportv1 : convertImportv2;

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
    SmiNode      *smiNode;

    smiNode = smiGetModuleIdentityNode(smiModule);
    
    if (smiNode) {

	if (smiv1 && smiNode) {
	    fprint(f, "%s OBJECT IDENTIFIER\n", smiNode->name);
	    fprintSegment(f, INDENT, "::= ", 0, 0);
	    fprint(f, "{ %s }\n\n", getOidString(smiNode, 0));
	}

	if (! smiv1 || ! silent) {

	    if (smiv1) {
		fprint(f, "-- %s MODULE-IDENTITY\n", smiNode->name);
	    } else {
		fprint(f, "%s MODULE-IDENTITY\n", smiNode->name);
	    }
	    fprintSegment(f, INDENT, "LAST-UPDATED", INDENTVALUE, smiv1);
	    smiRevision = smiGetFirstRevision(smiModule);
	    if (smiRevision)
		fprint(f, "\"%s\"\n", getTimeString(smiRevision->date));
	    else
		fprint(f, "\"197001010000Z\"\n");
	    fprintSegment(f, INDENT, "ORGANIZATION", INDENTVALUE, smiv1);
	    fprint(f, "\n");
	    fprintMultilineString(f, smiModule->organization, smiv1);
	    fprint(f, "\n");
	    fprintSegment(f, INDENT, "CONTACT-INFO", INDENTVALUE, smiv1);
	    fprint(f, "\n");
	    fprintMultilineString(f, smiModule->contactinfo, smiv1);
	    fprint(f, "\n");
	    fprintSegment(f, INDENT, "DESCRIPTION", INDENTVALUE, smiv1);
	    fprint(f, "\n");
	    if (smiModule->description) {
		fprintMultilineString(f, smiModule->description, smiv1);
	    } else {
		fprintMultilineString(f, "...", smiv1);
	    }
	    fprint(f, "\n");
	    
	    for(; smiRevision;
		smiRevision = smiGetNextRevision(smiRevision)) {
		if (!smiRevision->description
		    || strcmp(smiRevision->description,
	   "[Revision added by libsmi due to a LAST-UPDATED clause.]")) {
		    fprintSegment(f, INDENT, "REVISION", INDENTVALUE, smiv1);
		    fprint(f, "\"%s\"\n", getTimeString(smiRevision->date));
		    fprintSegment(f, INDENT, "DESCRIPTION", INDENTVALUE,
				  smiv1);
		    fprint(f, "\n");
		    if (smiRevision->description) {
			fprintMultilineString(f, smiRevision->description,
					      smiv1);
		    } else {
			fprintMultilineString(f, "...", smiv1);
		    }
		    fprint(f, "\n");
		}
	    }
	    
	    if (smiNode) {
		fprintSegment(f, INDENT, "::= ", 0, smiv1);
                if (!pibtomib)
		    fprint(f, "{ %s }\n\n", getOidString(smiNode, 0));
                else
                    fprint(f, "{ ibrpibtomib %d }\n\n",
                           smiNode->oid[smiNode->oidlen - 1]);
	    }
	    /* TODO: else error */
	    
	    fprint(f, "\n");
	}
    }
}



static void fprintTypeDefinition(FILE *f, SmiType *smiType)
{
    int		 invalid;

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



static void fprintTypeDefinitions(FILE *f, SmiModule *smiModule)
{
    SmiType	 *smiType;

    for(smiType = smiGetFirstType(smiModule);
	smiType; smiType = smiGetNextType(smiType)) {
	fprintTypeDefinition(f, smiType);
    }
}



static void fprintTextualConvention(FILE *f, SmiType *smiType)
{
    SmiType      *baseType;
    SmiModule    *baseModule;
    int		 i, invalid;

    char *smiv2basetypes[] = {
	"SNMPv2-SMI", "IpAddress",
	"SNMPv2-SMI", "Counter32",
	"SNMPv2-SMI", "Gauge32",
	"SNMPv2-SMI", "TimeTicks",    
	"SNMPv2-SMI", "Counter64",
	NULL, NULL
    };
    
    if (smiType->status != SMI_STATUS_UNKNOWN) {
	invalid = invalidType(smiType->basetype);
	if (smiv1 && !invalid) {
	    fprint(f, "%s ::=\n", smiType->name);
	    fprintSegment(f, INDENT, "", 0, invalid);
	    fprint(f, "%s", getTypeString(smiType->basetype,
					  smiGetParentType(smiType)));
	    fprintSubtype(f, smiType, invalid);
	    fprint(f, "\n\n");
	}
	
	if (! smiv1 || ! silent) {
	    
	    if (smiv1 || invalid) {
		fprint(f, "-- %s ::= TEXTUAL-CONVENTION\n", smiType->name);
	    } else {
		fprint(f, "%s ::= TEXTUAL-CONVENTION\n", smiType->name);
	    }
	    
	    if (smiType->format) {
		fprintSegment(f, INDENT, "DISPLAY-HINT", INDENTVALUE,
			      smiv1 || invalid);
		fprint(f, "\"%s\"\n", smiType->format);
	    }
	    
	    fprintSegment(f, INDENT, "STATUS", INDENTVALUE,
			  smiv1 || invalid);
	    fprint(f, "%s\n", getStatusString(smiType->status));
	    
	    fprintSegment(f, INDENT, "DESCRIPTION", INDENTVALUE,
			  smiv1 || invalid);
	    fprint(f, "\n");
	    if (smiType->description) {
		fprintMultilineString(f, smiType->description,
				      smiv1 || invalid);
	    } else {
		fprintMultilineString(f, "...", smiv1 || invalid);
	    }
	    fprint(f, "\n");
	    
	    if (smiType->reference) {
		fprintSegment(f, INDENT, "REFERENCE", INDENTVALUE,
			      smiv1 || invalid);
		fprint(f, "\n");
		fprintMultilineString(f, smiType->reference,
				      smiv1 || invalid);
		fprint(f, "\n");
	    }
	    
	    for (baseType = smiGetParentType(smiType);
		 baseType;
		 baseType = smiGetParentType(baseType)) {
		SmiType *parent = smiGetParentType(baseType);

		baseModule = smiGetTypeModule(baseType);
		if (baseModule && baseModule->name) {
		    for (i = 0; smiv2basetypes[i]; i += 2) {
			if (strcmp(smiv2basetypes[i+1], baseType->name) == 0
			    && strcmp(smiv2basetypes[i], baseModule->name) == 0) {
			    parent = NULL;
			}
		    }
		}
		
		fprintSegment(f, INDENT, "SYNTAX", INDENTVALUE,
			      smiv1 || invalid || parent);
		fprint(f, "%s",
		       getTypeString(baseType->basetype, baseType));
		fprintSubtype(f, smiType, smiv1 || invalid || parent);
		if (parent) {
		    fprintf(f, "\n");
		}
		if (!parent) break;
	    }
	    fprint(f, "\n\n");
	}
    }
}



static void fprintTextualConventions(FILE *f, SmiModule *smiModule)
{
    SmiType	 *smiType;
    
    for(smiType = smiGetFirstType(smiModule);
	smiType; smiType = smiGetNextType(smiType)) {
	fprintTextualConvention(f, smiType);
    }
}


static int isInIndex(SmiNode *node, SmiNode *parentNode)
{
    SmiElement *element;
    
    if ((parentNode->indexkind != SMI_INDEX_INDEX) &&
        (parentNode->indexkind != SMI_INDEX_EXPAND))
        return 0;
    
    for (element = smiGetNextElement(smiGetFirstElement(parentNode)); element;
         element = smiGetNextElement(element))
        if (smiGetElementNode(element)->name &&
            node->name &&
            !strcmp(smiGetElementNode(element)->name, node->name))
            return 1;
    return 0;
}


static void fprintObjects(FILE *f, SmiModule *smiModule)
{
    SmiNode	 *smiNode, *rowNode, *colNode, *smiParentNode, *relatedNode;
    SmiType	 *smiType;
    SmiNodekind  nodekinds;
    int		 i, invalid, create, assignement, indentsequence, addrowstatus;
    
    nodekinds =  SMI_NODEKIND_NODE | SMI_NODEKIND_TABLE |
	SMI_NODEKIND_ROW | SMI_NODEKIND_COLUMN | SMI_NODEKIND_SCALAR |
	SMI_NODEKIND_CAPABILITIES;
    
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
	    if (smiv1) {
		assignement = 1;
		fprint(f, "%s OBJECT IDENTIFIER\n", smiNode->name);
	    } else {
		fprint(f, "%s OBJECT-IDENTITY\n", smiNode->name);
	    }
	} else if (smiNode->nodekind == SMI_NODEKIND_CAPABILITIES) {
	    /* TODO: real agent capabilities */
	    fprint(f, "-- This has been an SMIv2 AGENT-CAPABILITIES node:\n");
	    if (smiv1) {
		assignement = 1;
		fprint(f, "%s OBJECT IDENTIFIER\n", smiNode->name);
	    } else {
		fprint(f, "%s OBJECT-IDENTITY\n", smiNode->name);
	    }
	} else {
	    if (invalid) {
		fprint(f, "-- %s OBJECT-TYPE\n", smiNode->name);
	    } else {
		fprint(f, "%s OBJECT-TYPE\n", smiNode->name);
	    }
	}

	if ((smiNode->nodekind == SMI_NODEKIND_TABLE) ||
	    (smiNode->nodekind == SMI_NODEKIND_ROW) ||
	    (smiType)) {
	    fprintSegment(f, INDENT, "SYNTAX", INDENTVALUE, invalid);
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

	if (! assignement && smiNode->units) {
	    fprintSegment(f, INDENT, "UNITS", INDENTVALUE, smiv1 || invalid);
	    fprint(f, "\"%s\"\n", smiNode->units);
	}
	
	if (! assignement &&
	    smiNode->nodekind != SMI_NODEKIND_CAPABILITIES &&
	    smiNode->nodekind != SMI_NODEKIND_NODE) {
	    if (smiv1) {
		fprintSegment(f, INDENT, "ACCESS", INDENTVALUE, invalid);
	    } else {
 		fprintSegment(f, INDENT, "MAX-ACCESS", INDENTVALUE, 0);
	    }
            if (pibtomib) {
                if ((smiNode->nodekind == SMI_NODEKIND_TABLE) ||
                    (smiNode->nodekind == SMI_NODEKIND_ROW))
                    fprint(f, "not-accessible\n");
                else if ((smiNode->nodekind == SMI_NODEKIND_COLUMN) &&
                         isInIndex(smiNode, smiParentNode))
	            fprint(f, "not-accessible\n");
                else if (smiNode->nodekind == SMI_NODEKIND_COLUMN)
                    fprint(f, "read-create\n");
                else
	            fprint(f, "%s\n", getAccessString(smiNode->access, create));
            } else
	        fprint(f, "%s\n", getAccessString(smiNode->access, create));
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
	    
	if (! assignement && smiNode->reference) {
	    fprintSegment(f, INDENT, "REFERENCE", INDENTVALUE,
			  smiv1 || invalid);
	    fprint(f, "\n");
	    fprintMultilineString(f, smiNode->reference, smiv1 || invalid);
	    fprint(f, "\n");
	}

	relatedNode = smiGetRelatedNode(smiNode);
	switch (smiNode->indexkind) {
	case SMI_INDEX_INDEX:
	case SMI_INDEX_REORDER:
	    fprintIndex(f, smiNode, invalid);
	    break;
	case SMI_INDEX_EXPAND:  /* TODO: we have to do more work here! */
	    fprintIndex(f, smiNode, invalid);
	    break;
	case SMI_INDEX_AUGMENT:
	    if (smiv1 && ! invalid) {
		if (relatedNode) {
		    fprintIndex(f, relatedNode, invalid);
		}
	    }
	    if ((! smiv1 || ! silent) && relatedNode) {
		fprintSegment(f, INDENT, "AUGMENTS", INDENTVALUE,
			      smiv1 || invalid);
		fprint(f, "{ %s }\n", relatedNode->name);
	    }
	    break;
	case SMI_INDEX_SPARSE:
	    if (! invalid) {
                while (relatedNode &&
                       (relatedNode->indexkind == SMI_INDEX_SPARSE))
                    relatedNode = smiGetRelatedNode(relatedNode);
		if (relatedNode) {
		    fprintIndex(f, relatedNode, invalid);
		}
	    }
	    /* TODO: non-local name if non-local */
	    break;
	case SMI_INDEX_UNKNOWN:
	    break;
	}
	
	if (smiNode->value.basetype != SMI_BASETYPE_UNKNOWN) {
	    fprintSegment(f, INDENT, "DEFVAL", INDENTVALUE, invalid);
	    fprint(f, "{ %s }", getValueString(&smiNode->value, smiType));
	    fprint(f, "\n");
	}

	fprintSegment(f, INDENT, "::= ", 0, invalid);
	fprint(f, "{ %s }\n\n", getOidString(smiNode, 0));

	smiType = smiGetNodeType(smiNode);
        addrowstatus = 0;
	if (smiNode->nodekind == SMI_NODEKIND_ROW) {
            if (pibtomib)
                addrowstatus = 1;
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
            if (pibtomib) {
                int len = strlen(smiParentNode->name) + 9;
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
            if (pibtomib) {
                size_t len = strlen(smiParentNode->name);
                int maxid;
                char *rowStatus = xmalloc(len + 10);
                strcpy(rowStatus, smiParentNode->name);
                if (len > 55)
                  len = 55;
                strcpy(&rowStatus[len], "RowStatus");
                fprint(f, ",\n");
                fprintSegment(f, INDENT, rowStatus, indentsequence, 0);
                fprint(f, "RowStatus\n}\n\n");
                
                fprint(f, "%s OBJECT-TYPE\n", rowStatus);
	        fprintSegment(f, INDENT, "SYNTAX", INDENTVALUE, 0);
                fprint(f, "RowStatus\n");
                fprintSegment(f, INDENT, "MAX-ACCESS", INDENTVALUE, 0);
                fprint(f, "read-create\n");
                fprintSegment(f, INDENT, "STATUS", INDENTVALUE, 0);
                fprint(f, "current\n");
                fprintSegment(f, INDENT, "DESCRIPTION", INDENTVALUE, 0);
                fprint(f, "\n");
		fprintMultilineString(f, "Added by smidump for automatic " \
                                      "PIB to MIB conversion.", 0);
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
                xfree(rowStatus);
            } else
 	        fprint(f, "\n}\n\n");
	}
    }
}



static void fprintNotifications(FILE *f, SmiModule *smiModule)
{
    SmiNode	 *smiNode, *parentNode;
    SmiElement   *smiElement;
    int          j;
    
    for(smiNode = smiGetFirstNode(smiModule, SMI_NODEKIND_NOTIFICATION);
	smiNode;
	smiNode = smiGetNextNode(smiNode, SMI_NODEKIND_NOTIFICATION)) {

	if (smiv1) {
	    fprint(f, "%s TRAP-TYPE\n", smiNode->name);
	    parentNode = smiGetParentNode(smiNode);
	    while ((parentNode->oidlen > 0) &&
		   ((!parentNode->name) ||
		    (!parentNode->oid[parentNode->oidlen-1]))) {
		parentNode = smiGetParentNode(parentNode);
	    }
	    fprintSegment(f, INDENT, "ENTERPRISE", INDENTVALUE, 0);
	    fprint(f, "%s\n", parentNode->name);
	} else {
	    fprint(f, "%s NOTIFICATION-TYPE\n", smiNode->name);
	}

	if ((smiElement = smiGetFirstElement(smiNode))) {
	    if (smiv1) {
		fprintSegment(f, INDENT, "VARIABLES", INDENTVALUE, 0);
	    } else {
		fprintSegment(f, INDENT, "OBJECTS", INDENTVALUE, 0);
	    }
	    fprint(f, "{ ");
	    for (j = 0; smiElement;
		 j++, smiElement = smiGetNextElement(smiElement)) {
		if (j) {
		    fprint(f, ", ");
		}
		fprintWrapped(f, INDENTVALUE + 2,
			      smiGetElementNode(smiElement)->name, 0);
		/* TODO: non-local name if non-local */
	    } /* TODO: empty? -> print error */
	    fprint(f, " }\n");
	}

	if (! smiv1 || ! silent) {
	    fprintSegment(f, INDENT, "STATUS", INDENTVALUE, smiv1);
	    fprint(f, "%s\n", getStatusString(smiNode->status));
	}

	fprintSegment(f, INDENT, "DESCRIPTION", INDENTVALUE, 0);
	fprint(f, "\n");
	if (smiNode->description) {
	    fprintMultilineString(f, smiNode->description, 0);
	} else {
	    fprintMultilineString(f, "...", 0);
	}
	fprint(f, "\n");
	    
	if (smiNode->reference) {
	    fprintSegment(f, INDENT, "REFERENCE", INDENTVALUE, smiv1);
	    fprint(f, "\n");
	    fprintMultilineString(f, smiNode->reference, smiv1);
	    fprint(f, "\n");
	}

	fprintSegment(f, INDENT, "::= ", 0, 0);
	if (smiv1) {
	    fprint(f, "%d\n\n", smiNode->oid[smiNode->oidlen-1]);
	} else {
	    fprint(f, "{ %s }\n\n", getOidString(smiNode, 0));
	}
    }
}



static void fprintGroups(FILE *f, SmiModule *smiModule)
{
    SmiNode	 *smiNode;
    SmiElement   *smiElement;
    int          j, objectGroup = 0, notificationGroup = 0;
    
    for (smiNode = smiGetFirstNode(smiModule, SMI_NODEKIND_GROUP);
	 smiNode; smiNode = smiGetNextNode(smiNode, SMI_NODEKIND_GROUP)) {

	objectGroup = isObjectGroup(smiNode);
	if (! objectGroup) {
	    notificationGroup = isNotificationGroup(smiNode);
	}

	if (smiv1 || (!objectGroup && !notificationGroup)) {
	    fprint(f, "%s OBJECT IDENTIFIER\n", smiNode->name);
	    fprintSegment(f, INDENT, "::= ", 0, 0);
	    fprint(f, "{ %s }\n\n", getOidString(smiNode, 0));
	}

	if (!objectGroup && !notificationGroup) {
	    continue;
	}

	if (! smiv1 || ! silent) {

	    if (smiv1) {
		fprint(f, "-- %s %s\n", smiNode->name,
		       objectGroup ? "OBJECT-GROUP" : "NOTIFICATION-GROUP");
	    } else {
		fprint(f, "%s %s\n", smiNode->name,
		       objectGroup ? "OBJECT-GROUP" : "NOTIFICATION-GROUP");
	    }

	    if (objectGroup) {
		fprintSegment(f, INDENT, "OBJECTS", INDENTVALUE, smiv1);
	    } else {
		fprintSegment(f, INDENT, "NOTIFICATIONS", INDENTVALUE, smiv1);
	    }
	    fprint(f, "{ ");
	    for (j = 0, smiElement = smiGetFirstElement(smiNode);
		 smiElement;
		 j++, smiElement = smiGetNextElement(smiElement)) {
		if (j) {
		    fprint(f, ", ");
		}
		fprintWrapped(f, INDENTVALUE + 2,
			      smiGetElementNode(smiElement)->name, smiv1);
		/* TODO: non-local name if non-local */
	    } /* TODO: empty? -> print error */
	    fprint(f, " }\n");
	    
	    fprintSegment(f, INDENT, "STATUS", INDENTVALUE, smiv1);
	    fprint(f, "%s\n", getStatusString(smiNode->status));
	    
	    fprintSegment(f, INDENT, "DESCRIPTION", INDENTVALUE, smiv1);
	    fprint(f, "\n");
	    if (smiNode->description) {
		fprintMultilineString(f, smiNode->description, smiv1);
	    } else {
		fprintMultilineString(f, "...", smiv1);
	    }
	    fprint(f, "\n");
	    
	    if (smiNode->reference) {
		fprintSegment(f, INDENT, "REFERENCE", INDENTVALUE, smiv1);
		fprint(f, "\n");
		fprintMultilineString(f, smiNode->reference, smiv1);
		fprint(f, "\n");
	    }
	    
	    fprintSegment(f, INDENT, "::= ", 0, smiv1);
	    fprint(f, "{ %s }\n\n", getOidString(smiNode, 0));
	}
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
	
	if (smiv1) {
	    fprint(f, "%s OBJECT IDENTIFIER\n", smiNode->name);
	    fprintSegment(f, INDENT, "::= ", 0, 0);
	    fprint(f, "{ %s }\n\n", getOidString(smiNode, 0));
	}

	if (! smiv1 || ! silent) {

	    if (smiv1) {
		fprint(f, "-- %s MODULE-COMPLIANCE\n", smiNode->name);
	    } else {
		fprint(f, "%s MODULE-COMPLIANCE\n", smiNode->name);
	    }
	    
	    fprintSegment(f, INDENT, "STATUS", INDENTVALUE, smiv1);
	    fprint(f, "%s\n", getStatusString(smiNode->status));
	    
	    fprintSegment(f, INDENT, "DESCRIPTION", INDENTVALUE, smiv1);
	    fprint(f, "\n");
	    if (smiNode->description) {
		fprintMultilineString(f, smiNode->description, smiv1);
	    } else {
		fprintMultilineString(f, "...", smiv1);
	    }
	    fprint(f, "\n");
	    
	    if (smiNode->reference) {
		fprintSegment(f, INDENT, "REFERENCE", INDENTVALUE, smiv1);
		fprint(f, "\n");
		fprintMultilineString(f, smiNode->reference, smiv1);
		fprint(f, "\n");
	    }
	    
	    /* `this module' always first */
	    done = xstrdup("+");
	    for (module = smiModule->name; module; ) {
		
		fprint(f, "\n");
		fprintSegment(f, INDENT, "MODULE", INDENTVALUE, smiv1);
		if (strlen(module) && strcmp(smiModule->name, module)) {
		    fprint(f, "%s\n", module);
		} else {
		    if (smiv1) {
			fprint(f, "-- -- this module\n");
		    } else {
			fprint(f, "-- this module\n");
		    }
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
					  INDENTVALUE, smiv1);
			    fprint(f, "{ ");
			}
			j++;
			fprintWrapped(f, INDENTVALUE + 2,
				      smiGetElementNode(smiElement)->name,
				      smiv1);
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
				      INDENTVALUE, smiv1);
			fprint(f, "%s\n", smiNode2->name);
			fprintSegment(f, 2 * INDENT, "DESCRIPTION",
				      INDENTVALUE, smiv1);
			fprint(f, "\n");
			if (smiOption->description) {
			    fprintMultilineString2(f, smiOption->description,
						  smiv1);
			} else {
			    fprintMultilineString2(f, "...", smiv1);
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
				      INDENTVALUE, smiv1);
			fprint(f, "%s\n", smiNode2->name);

			smiType = smiGetRefinementType(smiRefinement);
			if (smiType) {
			    fprintSegment(f, 2 * INDENT, "SYNTAX", INDENTVALUE,
					  smiv1);
			    fprint(f, "%s",
				   getTypeString(smiType->basetype,
						 smiGetParentType(smiType)));
			    fprintSubtype(f, smiType, smiv1);
			    fprint(f, "\n");
			}
			
			smiType = smiGetRefinementWriteType(smiRefinement);
			if (smiType) {
			    fprintSegment(f, 2 * INDENT, "WRITE-SYNTAX",
					  INDENTVALUE, smiv1);
			    fprint(f, "%s",
				   getTypeString(smiType->basetype,
						 smiGetParentType(smiType)));
			    fprintSubtype(f, smiType, smiv1);
			    fprint(f, "\n");
			}
			
			if (smiRefinement->access != SMI_ACCESS_UNKNOWN) {
			    fprintSegment(f, 2 * INDENT, "MIN-ACCESS",
					  INDENTVALUE, smiv1);
			    fprint(f, "%s\n",
				   getAccessString(smiRefinement->access, 0));
			    /* we assume, that read-create does not appear in
			     * an OT refinement.
			     */
			}
			fprintSegment(f, 2 * INDENT, "DESCRIPTION",
				      INDENTVALUE, smiv1);
			fprint(f, "\n");
			if (smiRefinement->description) {
			    fprintMultilineString2(f,
						  smiRefinement->description,
						  smiv1);
			} else {
			    fprintMultilineString2(f, "...", smiv1);
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
	    fprintSegment(f, INDENT, "::= ", 0, smiv1);
	    fprint(f, "{ %s }\n\n", getOidString(smiNode, 0));
	}
    }
    xfree(done);
}



static void dumpSmi(FILE *f, SmiModule *smiModule, int flags)
{
    if (smiModule->language == SMI_LANGUAGE_SPPI) /* PIB to MIB conversion */
        pibtomib = 1;
    else
        pibtomib = 0;

    createImportList(smiModule);
    
    fprint(f, "--\n");
    fprint(f, "-- This %s module has been generated by smidump "
	   SMI_VERSION_STRING ". Do not edit.\n", smiv1 ? "SMIv1" : "SMIv2");
    fprint(f, "--\n\n");
    if (pibtomib && smiv1) {
        fprint(f, "--\n");
        fprint(f, "-- WARNING: SPPI to SMIv1 conversion is not explicitly " \
          "supported.\n");
        fprint(f, "-- Expect flawed output.\n");
        fprint(f, "--\n\n");
    }
    if (! (flags & SMIDUMP_FLAG_SILENT) && (flags & SMIDUMP_FLAG_ERROR)) {
	fprintf(f, "--\n-- WARNING: this output may be incorrect due to "
		"significant parse errors\n--\n\n");
    }
    fprint(f, "%s%s DEFINITIONS ::= BEGIN\n\n", smiModule->name,
           (pibtomib ? "-MIB" : ""));
	
    fprintImports(f);
    fprintModuleIdentity(f, smiModule);
    fprintTypeDefinitions(f, smiModule);
    fprintTextualConventions(f, smiModule);
    fprintObjects(f, smiModule);
    fprintNotifications(f, smiModule);
    fprintGroups(f, smiModule);
    fprintModuleCompliances(f, smiModule);
    
    fprint(f, "END -- end of module %s%s.\n", smiModule->name,
           (pibtomib ? "-MIB" : ""));

    freeImportList();
}


static void dumpSmiV1(int modc, SmiModule **modv, int flags, char *output)
{
    int  i;
    FILE *f = stdout;
    
    smiv1 = 1;
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
	dumpSmi(f, modv[i], flags);
	if (fflush(f) || ferror(f)) {
	    perror("smidump: write error");
	    exit(1);
	}
    }

    if (output) {
	fclose(f);
    }
}


static void dumpSmiV2(int modc, SmiModule **modv, int flags, char *output)
{
    int  i;
    FILE *f = stdout;

    smiv1 = 0;
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
	dumpSmi(f, modv[i], flags);
    }
    
    if (fflush(f) || ferror(f)) {
	perror("smidump: write error");
	exit(1);
    }

    if (output) {
	fclose(f);
    }
}



void initSmi()
{
    static SmidumpDriver driver1 = {
	"smiv1",
	dumpSmiV1,
	0,
	SMIDUMP_DRIVER_CANT_UNITE,
	"SMIv1 (RFC 1155, RFC 1212, RFC 1215)",
	NULL,
	NULL
    };
    
    static SmidumpDriver driver2 = {
	"smiv2",
	dumpSmiV2,
	0,
	SMIDUMP_DRIVER_CANT_UNITE,
	"SMIv2 (RFC 2578, RFC 2579, RFC 2580)",
	NULL,
	NULL
    };
    
    smidumpRegisterDriver(&driver1);
    smidumpRegisterDriver(&driver2);
}
