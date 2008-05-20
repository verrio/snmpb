/*
 * parser-smi.y --
 *
 *      Syntax rules for parsing the SMIv1/v2 MIB module language.
 *
 * Copyright (c) 1999 Frank Strauss, Technical University of Braunschweig.
 *
 * See the file "COPYING" for information on usage and redistribution
 * of this file, and for a DISCLAIMER OF ALL WARRANTIES.
 *
 * @(#) $Id: parser-smi.y 8090 2008-04-18 12:56:29Z strauss $
 */

%{

#include <config.h>
    
#ifdef BACKEND_SMI

#include <stdio.h>
#include <errno.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>
#include <time.h>
#include <limits.h>

#if defined(_MSC_VER)
#include <malloc.h>
#endif

#include "smi.h"
#include "error.h"
#include "parser-smi.h"
#include "scanner-smi.h"
#include "data.h"
#include "check.h"
#include "util.h"
    
#ifdef HAVE_DMALLOC_H
#include <dmalloc.h>
#endif



/*
 * These arguments are passed to yyparse() and yylex().
 */
#define YYPARSE_PARAM parserPtr
#define YYLEX_PARAM   parserPtr

    
    
#define thisParserPtr      ((Parser *)parserPtr)
#define thisModulePtr     (((Parser *)parserPtr)->modulePtr)


    
/*
 * NOTE: The argument lvalp is not really a void pointer. Unfortunately,
 * we don't know it better at this point. bison generated C code declares
 * YYSTYPE just a few lines below based on the `%union' declaration.
 */
extern int yylex(void *lvalp, Parser *parserPtr);



static int	   impliedFlag;
static SmiNodekind variationkind;
static SmiBasetype defaultBasetype;
static int	   indexFlag;

/*
 * Values for the indexFlag variable
 */
#define INDEXFLAG_NONE     0
#define INDEXFLAG_PIBINDEX 1
#define INDEXFLAG_AUGMENTS 2
#define INDEXFLAG_EXTENDS  3
 

#define SMI_EPOCH	631152000	/* 01 Jan 1990 00:00:00 */ 

 

static char *convertImportv2[] = {
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
    "RFC1065-SMI", "internet",	    "SNMPv2-SMI", "internet",
    "RFC1065-SMI", "directory",	    "SNMPv2-SMI", "directory",
    "RFC1065-SMI", "mgmt",	    "SNMPv2-SMI", "mgmt",
    "RFC1065-SMI", "experimental",  "SNMPv2-SMI", "experimental",
    "RFC1065-SMI", "private",	    "SNMPv2-SMI", "private",
    "RFC1065-SMI", "enterprises",   "SNMPv2-SMI", "enterprises",
    "RFC1065-SMI", "IpAddress",     "SNMPv2-SMI", "IpAddress",
    "RFC1065-SMI", "Counter",       "SNMPv2-SMI", "Counter32",
    "RFC1065-SMI", "Gauge",         "SNMPv2-SMI", "Gauge32",
    "RFC1065-SMI", "TimeTicks",     "SNMPv2-SMI", "TimeTicks",
    "RFC1065-SMI", "Opaque",        "SNMPv2-SMI", "Opaque",
    "RFC1213-MIB", "mib-2",         "SNMPv2-SMI", "mib-2",    
    "RFC1213-MIB", "DisplayString", "SNMPv2-TC",  "DisplayString",    
    NULL, NULL, NULL, NULL
};



static void
checkDescr(Parser *parser, char *descr)
{
    if (descr) {
	if (descr[0] == 0) {
	    smiPrintError(parser, ERR_EMPTY_DESCRIPTION);
	}
	/* we might want to add more checks since I have recently
	   seen things like DESCRIPTION "." to cirumvent warnings */
    }
}
 

 
static void
checkNameLen(Parser *parser, char *name, int error_32, int error_64)
{
    int len = strlen(name);
    
    if (len > 64) {
	smiPrintError(parser, error_64, name);
    } else if (len > 32) {
	smiPrintError(parser, error_32, name);
    }
}


 
static void
checkModuleName(Parser *parserPtr, Module *modulePtr)
{
     static char *mib_ignore[] = {
	 "SNMPv2-SMI", "SNMPv2-TC", "SNMPv2-CONF", NULL
     };
     
     static char *pib_ignore[] = {
	 "COPS-PR-SPPI", "COPS-PR-SPPI-TC",
	 "SNMPv2-SMI", "SNMPv2-TC", "SNMPv2-CONF", NULL
     };

     const char *name = thisModulePtr->export.name;
     const int len = strlen(name);
     int i;

     switch (modulePtr->export.language) {
     case SMI_LANGUAGE_SMIV1:
     case SMI_LANGUAGE_SMIV2:
     case SMI_LANGUAGE_SMING:
	 for (i = 0; mib_ignore[i]; i++) {
	     if (strcmp(mib_ignore[i], name) == 0) {
		 return;
	     }
	 }
	 if (len > 3 && (strcmp(name + len - 4, "-MIB") != 0)) {
	     smiPrintError(parserPtr, ERR_MIB_MODULENAME_SUFFIX, name);
	     return;
	 }
	 break;
     case SMI_LANGUAGE_SPPI:
	 for (i = 0; pib_ignore[i]; i++) {
	     if (strcmp(pib_ignore[i], name) == 0) {
		 return;
	     }
	 }
	 if (len > 3 && (strcmp(name + len - 4, "-PIB") != 0)) {
	     smiPrintError(parserPtr, ERR_PIB_MODULENAME_SUFFIX, name);
	 }
	 break;
     case SMI_LANGUAGE_UNKNOWN:
	 break;
     }
}



static void
checkModuleIdentity(Parser *parserPtr, Module *modulePtr)
{
    if ((modulePtr->export.language == SMI_LANGUAGE_SMIV2)
	&& (modulePtr->numModuleIdentities < 1)
	&& strcmp(modulePtr->export.name, "SNMPv2-SMI")
	&& strcmp(modulePtr->export.name, "SNMPv2-CONF")
	&& strcmp(modulePtr->export.name, "SNMPv2-TC")
        && strcmp(modulePtr->export.name, "COPS-PR-SPPI")) {
	smiPrintError(parserPtr, ERR_NO_MODULE_IDENTITY);
    }
}



static void
checkObjects(Parser *parserPtr, Module *modulePtr)
{
    Object *objectPtr;
    Node *nodePtr;
    int i;
    Type *counterTypePtr, *counter32TypePtr, *counter64TypePtr;
    
    counterTypePtr = findTypeByName("Counter");
    counter32TypePtr = findTypeByModulenameAndName("SNMPv2-SMI", "Counter32");
    counter64TypePtr = findTypeByModulenameAndName("SNMPv2-SMI", "Counter64");
    
    for (objectPtr = modulePtr->firstObjectPtr;
	 objectPtr; objectPtr = objectPtr->nextPtr) {

	Object *parentPtr;
	
	if ((objectPtr->export.decl != SMI_DECL_UNKNOWN) &&
	    objectPtr->nodePtr->parentPtr &&
	    objectPtr->nodePtr->parentPtr->lastObjectPtr) {
	    parentPtr = objectPtr->nodePtr->parentPtr->lastObjectPtr;
	} else {
	    parentPtr = NULL;
	}

	/*
	 * Set nodekinds of all newly defined objects.
	 */
	
	if (objectPtr->export.decl == SMI_DECL_MODULEIDENTITY) {
	    objectPtr->export.nodekind = SMI_NODEKIND_NODE;
	} else if ((objectPtr->export.decl == SMI_DECL_VALUEASSIGNMENT) ||
		   (objectPtr->export.decl == SMI_DECL_OBJECTIDENTITY)) {
	    objectPtr->export.nodekind = SMI_NODEKIND_NODE;
	} else if ((objectPtr->export.decl == SMI_DECL_OBJECTTYPE) &&
		   (objectPtr->typePtr) &&
		   (objectPtr->typePtr->export.decl == SMI_DECL_IMPL_SEQUENCEOF)) {
	    objectPtr->export.nodekind = SMI_NODEKIND_TABLE;
	} else if ((objectPtr->export.decl == SMI_DECL_OBJECTTYPE) &&
		   (objectPtr->export.indexkind != SMI_INDEX_UNKNOWN)) {
	    objectPtr->export.nodekind = SMI_NODEKIND_ROW;
	} else if ((objectPtr->export.decl == SMI_DECL_NOTIFICATIONTYPE) ||
		   (objectPtr->export.decl == SMI_DECL_TRAPTYPE)) {
	    objectPtr->export.nodekind = SMI_NODEKIND_NOTIFICATION;
	} else if ((objectPtr->export.decl == SMI_DECL_OBJECTGROUP) ||
		   (objectPtr->export.decl == SMI_DECL_NOTIFICATIONGROUP)) {
	    objectPtr->export.nodekind = SMI_NODEKIND_GROUP;
	} else if (objectPtr->export.decl == SMI_DECL_MODULECOMPLIANCE) {
	    objectPtr->export.nodekind = SMI_NODEKIND_COMPLIANCE;
	} else if (objectPtr->export.decl == SMI_DECL_AGENTCAPABILITIES) {
	    objectPtr->export.nodekind = SMI_NODEKIND_CAPABILITIES;
	} else if ((objectPtr->export.decl == SMI_DECL_OBJECTTYPE) &&
		   (parentPtr) &&
		   (parentPtr->export.indexkind != SMI_INDEX_UNKNOWN)) {
	    objectPtr->export.nodekind = SMI_NODEKIND_COLUMN;
	} else if ((objectPtr->export.decl == SMI_DECL_OBJECTTYPE) &&
		   (parentPtr) &&
		   (parentPtr->export.indexkind == SMI_INDEX_UNKNOWN)) {
	    objectPtr->export.nodekind = SMI_NODEKIND_SCALAR;
	}
    }

    for (objectPtr = modulePtr->firstObjectPtr;
	 objectPtr; objectPtr = objectPtr->nextPtr) {

	Object *parentPtr;
	
	if (objectPtr->nodePtr->parentPtr &&
	    objectPtr->nodePtr->parentPtr->lastObjectPtr) {
	    parentPtr = objectPtr->nodePtr->parentPtr->lastObjectPtr;
	} else {
	    parentPtr = NULL;
	}

	/*
	 * Check whether the associated type resolves to a known base type.
	 */
	
	if (objectPtr->typePtr
	    && (objectPtr->export.nodekind == SMI_NODEKIND_COLUMN
		|| objectPtr->export.nodekind == SMI_NODEKIND_SCALAR)
	    && objectPtr->typePtr->export.basetype == SMI_BASETYPE_UNKNOWN) {
	    smiPrintErrorAtLine(parserPtr, ERR_BASETYPE_UNKNOWN,
				objectPtr->line,
				objectPtr->typePtr->export.name ?
				objectPtr->typePtr->export.name : "[unknown]",
				objectPtr->export.name);
	    if (objectPtr->nodePtr->parentPtr->firstObjectPtr->export.nodekind
		== SMI_NODEKIND_TABLE) {
		/* the parent node is a table node, so assume this is
		 *  a row node. this adjusts missing INDEXs in RFC 1158.
		 */
		objectPtr->export.nodekind = SMI_NODEKIND_ROW;
	    }
	}

	/*
	 * Mark types that are referenced in this module.
	 */

	if (objectPtr->typePtr
	    && (objectPtr->export.nodekind == SMI_NODEKIND_COLUMN
		|| objectPtr->export.nodekind == SMI_NODEKIND_SCALAR)
	    && (objectPtr->typePtr->export.decl == SMI_DECL_TYPEDEF
		|| objectPtr->typePtr->export.decl == SMI_DECL_TEXTUALCONVENTION
		|| objectPtr->typePtr->export.decl == SMI_DECL_IMPLICIT_TYPE)) {
	    addTypeFlags(objectPtr->typePtr, FLAG_INSYNTAX);
	    if (objectPtr->typePtr->export.decl == SMI_DECL_IMPLICIT_TYPE) {
		addTypeFlags(objectPtr->typePtr->parentPtr, FLAG_INSYNTAX);
	    }
	}
	
	/*
	 * Check whether the status of the associated type matches the
	 * status of the object.
	 */

	if (objectPtr->typePtr
	    && (objectPtr->export.nodekind == SMI_NODEKIND_COLUMN
		|| objectPtr->export.nodekind == SMI_NODEKIND_SCALAR)
	    && (objectPtr->export.status < objectPtr->typePtr->export.status)) {
	    if (objectPtr->typePtr->export.status == SMI_STATUS_DEPRECATED) {
		smiPrintErrorAtLine(parserPtr, ERR_TYPE_STATUS_DEPRECATED,
				    objectPtr->line,
				    objectPtr->typePtr->export.name,
				    objectPtr->export.name);
	    }
	    if (objectPtr->typePtr->export.status == SMI_STATUS_OBSOLETE) {
		smiPrintErrorAtLine(parserPtr, ERR_TYPE_STATUS_OBSOLETE,
				    objectPtr->line,
				    objectPtr->typePtr->export.name,
				    objectPtr->export.name);
	    }
	}

	/*
	 * Check the nodekind of the parent node.
	 */

	if (parentPtr) {
	    switch (objectPtr->export.nodekind) {
	    case SMI_NODEKIND_COLUMN:
		if (parentPtr->export.nodekind != SMI_NODEKIND_ROW) {
		    smiPrintErrorAtLine(parserPtr, ERR_COLUMN_PARENT_TYPE,
					objectPtr->line, objectPtr->export.name);
		}
		break;
	    case SMI_NODEKIND_ROW:
		if (parentPtr->export.nodekind != SMI_NODEKIND_TABLE) {
		    smiPrintErrorAtLine(parserPtr, ERR_ROW_PARENT_TYPE,
					objectPtr->line, objectPtr->export.name);
		}
		if (parentPtr->typePtr && parentPtr->typePtr->parentPtr &&
		    strcmp(parentPtr->typePtr->parentPtr->export.name,
			   objectPtr->typePtr->export.name)) {
		    smiPrintErrorAtLine(parserPtr, ERR_TABLE_ROW_TYPE_MISMATCH,
					objectPtr->line,
					objectPtr->export.name);
		}
		break;
	    case SMI_NODEKIND_TABLE:
		if ((parentPtr->export.nodekind != SMI_NODEKIND_UNKNOWN) &&
		    (parentPtr->export.nodekind != SMI_NODEKIND_NODE)) {
		    smiPrintErrorAtLine(parserPtr, ERR_TABLE_PARENT_TYPE,
					objectPtr->line, objectPtr->export.name);
		}
		break;
	    case SMI_NODEKIND_SCALAR:
		if ((parentPtr->export.nodekind != SMI_NODEKIND_UNKNOWN) &&
		    (parentPtr->export.nodekind != SMI_NODEKIND_NODE)) {
		    smiPrintErrorAtLine(parserPtr, ERR_SCALAR_PARENT_TYPE,
					objectPtr->line, objectPtr->export.name);
		}
		break;
	    case SMI_NODEKIND_NOTIFICATION:
		if ((parentPtr->export.nodekind != SMI_NODEKIND_NODE) &&
		    (parentPtr->export.nodekind != SMI_NODEKIND_UNKNOWN)) {
		    smiPrintErrorAtLine(parserPtr, ERR_NOTIFICATION_PARENT_TYPE,
					objectPtr->line, objectPtr->export.name);
		}
		if (parserPtr && parentPtr->nodePtr->parentPtr &&
		    parentPtr->nodePtr->parentPtr->lastObjectPtr) {
		    Object *parent2Ptr = parentPtr->nodePtr->parentPtr->lastObjectPtr;
		    if ((parent2Ptr->export.nodekind != SMI_NODEKIND_NODE) &&
			(parent2Ptr->export.nodekind != SMI_NODEKIND_UNKNOWN)) {
			smiPrintErrorAtLine(parserPtr,
					    ERR_NOTIFICATION_PARENT_TYPE,
					    objectPtr->line,
					    objectPtr->export.name);
		    }
		}
		break;
	    case SMI_NODEKIND_NODE:
		/* Node defined by OBJECT IDENTIFIER assignments can have
		   arbitrary parent node. */
		if ((parentPtr->export.nodekind != SMI_NODEKIND_UNKNOWN) &&
		    (parentPtr->export.nodekind != SMI_NODEKIND_NODE) &&
		    (objectPtr->export.decl != SMI_DECL_VALUEASSIGNMENT)) {
		    smiPrintErrorAtLine(parserPtr, ERR_NODE_PARENT_TYPE,
					objectPtr->line, objectPtr->export.name);
		}
		break;
	    case SMI_NODEKIND_GROUP:
		if ((parentPtr->export.nodekind != SMI_NODEKIND_UNKNOWN) &&
		    (parentPtr->export.nodekind != SMI_NODEKIND_NODE)) {
		    smiPrintErrorAtLine(parserPtr, ERR_GROUP_PARENT_TYPE,
					objectPtr->line, objectPtr->export.name);
		}
		break;
	    case SMI_NODEKIND_COMPLIANCE:
		if ((parentPtr->export.nodekind != SMI_NODEKIND_UNKNOWN) &&
		    (parentPtr->export.nodekind != SMI_NODEKIND_NODE)) {
		    smiPrintErrorAtLine(parserPtr, ERR_COMPLIANCE_PARENT_TYPE,
					objectPtr->line, objectPtr->export.name);
		}
		break;
	    case SMI_NODEKIND_CAPABILITIES:
		if ((parentPtr->export.nodekind != SMI_NODEKIND_UNKNOWN) &&
		    (parentPtr->export.nodekind != SMI_NODEKIND_NODE)) {
		    smiPrintErrorAtLine(parserPtr, ERR_CAPABILITIES_PARENT_TYPE,
					objectPtr->line, objectPtr->export.name);
		}
		break;
	    }
	}
	
	/*
	 * Check whether groups only contain scalars, columns and
	 * notifications.
	 */

	if (objectPtr->export.nodekind == SMI_NODEKIND_GROUP) {
	    smiCheckGroupMembers(parserPtr, objectPtr);
	}

	/*
	 * Check whether compliance statements contain out of date
	 * groups or objects.
	 */

	if (objectPtr->export.nodekind == SMI_NODEKIND_COMPLIANCE) {
	    smiCheckComplianceStatus(parserPtr, objectPtr);
	}

	/*
	 * Check whether notification statements contain useful
	 * objects.
	 */

	if (objectPtr->export.nodekind == SMI_NODEKIND_NOTIFICATION) {
	    smiCheckNotificationOid(parserPtr, modulePtr, objectPtr);
	    smiCheckNotificationMembers(parserPtr, objectPtr);
	}

        if (modulePtr->export.language != SMI_LANGUAGE_SPPI) {
	    /*
	     * Check whether tables and rows are not accessible
	     * (RFC 2578 7.1.12).
	     */

	    if (objectPtr->export.nodekind == SMI_NODEKIND_TABLE
	        && objectPtr->export.access != SMI_ACCESS_NOT_ACCESSIBLE) {
	        smiPrintErrorAtLine(parserPtr, ERR_TABLE_ACCESS,
				    objectPtr->line, objectPtr->export.name);
	    }

	    if (objectPtr->export.nodekind == SMI_NODEKIND_ROW
	        && objectPtr->export.access != SMI_ACCESS_NOT_ACCESSIBLE) {
	        smiPrintErrorAtLine(parserPtr, ERR_ROW_ACCESS,
				    objectPtr->line, objectPtr->export.name);
	    }

	    /*
	     * Check whether counter objects are read-only or
	     * accessible-for-notify (RFC 2578, 7.1.6).
	     */
	    if (((objectPtr->export.nodekind == SMI_NODEKIND_SCALAR) ||
		 (objectPtr->export.nodekind == SMI_NODEKIND_COLUMN)) &&
	        (objectPtr->export.access != SMI_ACCESS_NOTIFY) &&
		(objectPtr->export.access != SMI_ACCESS_READ_ONLY) &&
		(smiTypeDerivedFrom(objectPtr->typePtr, counterTypePtr) ||
		 smiTypeDerivedFrom(objectPtr->typePtr, counter32TypePtr) ||
		 smiTypeDerivedFrom(objectPtr->typePtr, counter64TypePtr))) {
	        smiPrintErrorAtLine(parserPtr, ERR_COUNTER_ACCESS,
				    objectPtr->line, objectPtr->export.name);
	    }
        }
	
	/*
	 * Check whether a row's subid is 1, see RFC 2578 7.10 (1).
	 */
	
	if (objectPtr->export.nodekind == SMI_NODEKIND_ROW) {
	    int len;
	    
	    if (objectPtr->nodePtr->subid != 1) {
		smiPrintErrorAtLine(parserPtr, ERR_ROW_SUBID_ONE,
				    objectPtr->line, objectPtr->export.name);
	    }

	    len = strlen(objectPtr->export.name);
	    if (len < 6 || strcmp(objectPtr->export.name+len-5, "Entry")) {
		smiPrintErrorAtLine(parserPtr, ERR_ROWNAME_ENTRY,
				    objectPtr->line, objectPtr->export.name);
	    } else {

		/*
		 * This misreports some cases where the table name
		 * does not have the "*Table" suffix.  This is trying
		 * to allow Entry names of either fooTableEntry or
		 * fooEntry.
		 */
		if (parentPtr &&
		    !(((int)strlen(parentPtr->export.name) == len ||
		       (int)strlen(parentPtr->export.name) == len - 5) &&
		      !strncmp(objectPtr->export.name, parentPtr->export.name,
			len - 5))) {
		    smiPrintErrorAtLine(parserPtr, ERR_ROWNAME_TABLENAME,
					objectPtr->line,
					objectPtr->export.name,
					parentPtr->export.name);
		}
	    }
	}

	/*
	 * Check whether a row's SEQUENCE contains exactly the list
	 * of child nodes (columns).  An unknown SEQUENCE type
	 * is handled later.
	 */

	if (objectPtr->export.nodekind == SMI_NODEKIND_ROW &&
	    ((objectPtr->typePtr->flags & FLAG_INCOMPLETE) == 0)) {
	    List *p;
	    Node *seqNodePtr, *childNodePtr;
	    Object *colPtr;
	    int i;
	    
	    /*
	     * Walk through the SEQUENCE elements and find those
	     * that are misordered or have no matching columnar object.
	     */
	    for (p = objectPtr->typePtr->listPtr, i = 1,
		     childNodePtr = objectPtr->nodePtr->firstChildPtr;
		 p && childNodePtr;
		 p = p->nextPtr, childNodePtr = childNodePtr->nextPtr, i++) {
		seqNodePtr = ((Object *)p->ptr)->nodePtr;

		if (seqNodePtr->parentPtr != childNodePtr->parentPtr) {
		    smiPrintErrorAtLine(parserPtr, ERR_SEQUENCE_NO_COLUMN,
					objectPtr->typePtr->line,
					i,
					((Object *)p->ptr)->export.name,
					objectPtr->export.name);
		    continue;
		}

		if (seqNodePtr != childNodePtr) {
		    smiPrintErrorAtLine(parserPtr, ERR_SEQUENCE_ORDER,
					objectPtr->typePtr->line,
					i,
					((Object *)p->ptr)->export.name,
					objectPtr->export.name);
		    break;
		}
	    }
	    if ((p != NULL) && (childNodePtr == NULL)) {
		smiPrintErrorAtLine(parserPtr, ERR_SEQUENCE_NO_COLUMN,
				    objectPtr->typePtr->line,
				    i, 
				    ((Object *)p->ptr)->export.name,
				    objectPtr->export.name);
	    }

	    /*
	     * Walk through all child objects and find those
	     * that were missing in the SEQUENCE.
	     */
	    for (childNodePtr = objectPtr->nodePtr->firstChildPtr;
		 childNodePtr; childNodePtr = childNodePtr->nextPtr) {
		colPtr = findObjectByModuleAndNode(modulePtr, childNodePtr);
		if (!colPtr) continue;
		for (p = objectPtr->typePtr->listPtr; p; p = p->nextPtr) {
		    if (((Object *)p->ptr)->nodePtr == colPtr->nodePtr)
			break;
		}
		if (!p) {
		    if (colPtr->export.name) {
			/*
			 * Don't complain, if it's an implcitly defined
			 * unnamed node (could happen for parent node of
			 * TRAP-TYPE definitions).
			 */
			smiPrintErrorAtLine(parserPtr,
					    ERR_SEQUENCE_MISSING_COLUMN,
					    objectPtr->typePtr->line,
					    objectPtr->typePtr->export.name,
					    colPtr->export.name);
		    }
		}
	    }
	    
	}
	
	if (objectPtr->export.nodekind == SMI_NODEKIND_TABLE) {
	    int len;

	    len = strlen(objectPtr->export.name);
	    if (len < 6 || strcmp(objectPtr->export.name+len-5, "Table")) {
		smiPrintErrorAtLine(parserPtr, ERR_TABLENAME_TABLE,
				    objectPtr->line, objectPtr->export.name);
	    }
	}

	/*
	 * TODO: check whether the row is the only node below the
         * table node
	 */

	/*
	 * Check references to unknown identifiers.
	 */

	if ((objectPtr->flags & FLAG_INCOMPLETE) &&
	    (objectPtr->export.decl != SMI_DECL_IMPL_OBJECT)) {
	    if (objectPtr->export.name) {
		smiPrintErrorAtLine(parserPtr, ERR_UNKNOWN_OIDLABEL,
				    objectPtr->line, objectPtr->export.name);
	    } else {
		smiPrintErrorAtLine(parserPtr, ERR_IMPLICIT_NODE,
				    objectPtr->line);
	    }
	}

	/*
	 * Adjust the status of implicit type definitions.
	 */
	
	if (objectPtr->typePtr
	    && (objectPtr->typePtr->export.decl == SMI_DECL_IMPLICIT_TYPE)
	    && (objectPtr->typePtr->export.status == SMI_STATUS_UNKNOWN)) {
	    objectPtr->typePtr->export.status = objectPtr->export.status;
	}

	/*
	 * Link implicit type definition from refinements into
	 * the type derivation tree. Adjust the status of implicit
	 * type definitions in refinements. 
	 */

	if (objectPtr->export.nodekind == SMI_NODEKIND_COMPLIANCE) {

	    List *listPtr;

	    for (listPtr = objectPtr->refinementlistPtr;
		 listPtr;
		 listPtr = listPtr->nextPtr) {

		Refinement *refinementPtr;
		Type *typePtr;
		
		refinementPtr = ((Refinement *)(listPtr->ptr));
		typePtr = refinementPtr->typePtr;
		if (typePtr) {
		    if (typePtr->export.status == SMI_STATUS_UNKNOWN) {
			typePtr->export.status = objectPtr->export.status;
		    }
		}

		typePtr = refinementPtr->writetypePtr;
		if (typePtr) {
		    if (typePtr->export.status == SMI_STATUS_UNKNOWN) {
			typePtr->export.status = objectPtr->export.status;
		    }
		}
		
	    }
	    /* relocate the refinement type into the type tree */
	    /* relocate the write refinement type into the type tree */
	}

	/*
	 * Set the oidlen/oid values that are not yet correct.
	 */

	if (objectPtr->export.oidlen == 0) {
	    if (objectPtr->nodePtr->oidlen == 0) {
		for (nodePtr = objectPtr->nodePtr, i = 1;
		     nodePtr->parentPtr != thisParserPtr->pendingNodePtr &&
			 nodePtr->parentPtr != smiHandle->rootNodePtr &&
			 nodePtr != nodePtr->parentPtr &&
			 i <= 128;
		     nodePtr = nodePtr->parentPtr, i++);
		if ((objectPtr->export.name) &&
		    ((i > 128) || (nodePtr == nodePtr->parentPtr))) {
		    smiPrintErrorAtLine(parserPtr, ERR_OID_RECURSIVE,
					objectPtr->line,
					objectPtr->export.name);
		}
		objectPtr->nodePtr->oid = smiMalloc(i * sizeof(SmiSubid));
		objectPtr->nodePtr->oidlen = i;
		for (nodePtr = objectPtr->nodePtr; i > 0; i--) {
		    objectPtr->nodePtr->oid[i-1] = nodePtr->subid;
		    nodePtr = nodePtr->parentPtr;
		}
	    }
	    objectPtr->export.oidlen = objectPtr->nodePtr->oidlen;
	    objectPtr->export.oid = objectPtr->nodePtr->oid;
	}

	if ((objectPtr->export.decl != SMI_DECL_UNKNOWN)
	    && (objectPtr->export.nodekind != SMI_NODEKIND_NODE)
	    && objectPtr->export.name
	    && objectPtr->export.oid[objectPtr->export.oidlen-1] == 0
	    && objectPtr->export.oidlen != 2 && objectPtr->export.oid[0] != 0) {
	    smiPrintErrorAtLine(parserPtr, ERR_OID_ADMIN_ZERO,
				objectPtr->line,
				objectPtr->export.name);
	}

	/*
	 * Check whether the module identity is registered in a well
	 * known controlled location. 
	 */

	if (objectPtr->export.decl == SMI_DECL_MODULEIDENTITY) {
	    smiCheckModuleIdentityRegistration(parserPtr, objectPtr);
	}

	/*
	 * Check table linkage constraints for row objects.
	 */

	if (objectPtr->export.nodekind == SMI_NODEKIND_ROW) {
	    switch (objectPtr->export.indexkind) {
	    case SMI_INDEX_INDEX:
		smiCheckIndex(parserPtr, objectPtr);
		break;
	    case SMI_INDEX_AUGMENT:
            case SMI_INDEX_SPARSE:
		smiCheckAugment(parserPtr, objectPtr);
		break;
	    default:
		break;
	    }
	}
	
	/*
	 * Determine the longest common OID prefix of all nodes.
	 */

	if (!modulePtr->prefixNodePtr) {
	    modulePtr->prefixNodePtr = objectPtr->nodePtr;
	} else {
	    if (objectPtr->nodePtr->oidlen < modulePtr->prefixNodePtr->oidlen) {
		Node *nodePtr = findNodeByOid(objectPtr->nodePtr->oidlen,
					      modulePtr->prefixNodePtr->oid);
		if (nodePtr)
		    modulePtr->prefixNodePtr = nodePtr;
		else
		    smiPrintError(parserPtr, ERR_OTHER_ERROR,
				  "Failed to create complete object tree - "
				  "expect incorrect output");
	    }
	    for (i = 0; i < modulePtr->prefixNodePtr->oidlen; i++) {
		if (modulePtr->prefixNodePtr->oid[i] !=
		    objectPtr->nodePtr->oid[i]) {
		    modulePtr->prefixNodePtr =
			findNodeByOid(i, modulePtr->prefixNodePtr->oid);
		    break;
		}
	    }
	}
    }

    if (modulePtr->export.language == SMI_LANGUAGE_SMIV2) {
	for (objectPtr = modulePtr->firstObjectPtr;
	     objectPtr; objectPtr = objectPtr->nextPtr) {
	    
	    /*
	     * Check whether all objects and notifications are contained in at
	     * least one conformance group (RFC 2580 3.3 and 4.1).
	     */
	    
	    smiCheckGroupMembership(parserPtr, objectPtr);
	}
    }

    if (modulePtr->export.language == SMI_LANGUAGE_SPPI) {
        Object *parentPtr;
        
        for (objectPtr = modulePtr->firstObjectPtr;
             objectPtr; objectPtr = objectPtr->nextPtr) {
            /*
             * All checks for SPPI constructs
             */
            if (objectPtr->nodePtr->parentPtr)
                parentPtr = objectPtr->nodePtr->parentPtr->lastObjectPtr;
            else
                parentPtr = NULL;
            
            /*
             * Do all rows contain a PIB-INDEX/AUGMENTS/EXTENDS ?
             * See RFC 3159 7.5, 7.7, 7.8
             */
            if (parentPtr  &&
                (parentPtr->export.nodekind == SMI_NODEKIND_TABLE) &&
                (objectPtr->export.indexkind != SMI_INDEX_INDEX) &&
                (objectPtr->export.indexkind != SMI_INDEX_AUGMENT) &&
                (objectPtr->export.indexkind != SMI_INDEX_SPARSE))
                smiPrintErrorAtLine(parserPtr, ERR_ROW_LACKS_PIB_INDEX,
                                    objectPtr->line);

            /*
             * Does any non row contain a PIB-INDEX/AUGMENTS/EXTENDS ?
             * See RFC 3159 7.5, 7.7, 7.8
             */
            if ((objectPtr->export.nodekind != SMI_NODEKIND_ROW) &&
                (objectPtr->export.indexkind != SMI_INDEX_UNKNOWN))
                smiPrintErrorAtLine(parserPtr, ERR_PIB_INDEX_FOR_NON_ROW_TYPE,
                                    objectPtr->line);

            /*
             * Check the PIB-INDEX and other indices
             */
            if ((objectPtr->export.nodekind == SMI_NODEKIND_ROW) &&
                (objectPtr->export.indexkind == SMI_INDEX_INDEX)) {

                /*
                 * Only the first element (PIB-INDEX) has to be an InstanceId.
                 * See RFC 3159 7.5
                 */
                if (objectPtr->listPtr && objectPtr->listPtr->ptr) {
                    Object *pibindex = (Object *)objectPtr->listPtr->ptr;
                    if (pibindex->typePtr && pibindex->typePtr->export.name &&
                        strcmp(pibindex->typePtr->export.name, "InstanceId"))
                        smiPrintErrorAtLine(thisParserPtr, ERR_PIB_INDEX_NOT_INSTANCEID,
                                            pibindex->line, pibindex->export.name);
                }
            }
            
            /*
             * Do all tables contain a PIB-ACCESS clause?
             * See RFC 3159 7.3
             */
            if ((objectPtr->export.nodekind == SMI_NODEKIND_TABLE) &&
                (objectPtr->export.access == SMI_ACCESS_UNKNOWN))
                smiPrintErrorAtLine(parserPtr, ERR_TABLE_LACKS_PIB_ACCESS,
                                    objectPtr->line);

            /*
             * Does any non table types contain a PIB-ACCESS clause?
             * See RFC 3159 7.3
             */
            if (((objectPtr->export.nodekind == SMI_NODEKIND_NODE) ||
                 (objectPtr->export.nodekind == SMI_NODEKIND_ROW) ||
                 (objectPtr->export.nodekind == SMI_NODEKIND_SCALAR)) &&
                (objectPtr->export.access != SMI_ACCESS_UNKNOWN))
                smiPrintErrorAtLine(parserPtr, ERR_PIB_ACCESS_FOR_NON_TABLE,
                                    objectPtr->line);

            /*
             * Check the UNIQUENESS clause and its entries
             * See RFC 3159 7.9
             */
            if (objectPtr->uniquenessPtr) {
                if (objectPtr->export.nodekind != SMI_NODEKIND_ROW)
                    smiPrintErrorAtLine(parserPtr, ERR_UNIQUENESS_FOR_NON_ROW,
                                        objectPtr->line);
                else
                    smiCheckUniqueness(parserPtr, objectPtr);
            }
            
            /*
             * Does the PIB-REFERENCES object point to a PRC (table)?
             * See RFC 3159 7.10
             */
            if (objectPtr->typePtr && objectPtr->typePtr->export.name &&
                !strcmp(objectPtr->typePtr->export.name, "ReferenceId") &&
                objectPtr->relatedPtr &&
                (objectPtr->relatedPtr->export.nodekind != SMI_NODEKIND_ROW))
                smiPrintErrorAtLine(parserPtr, ERR_PIB_REFERENCES_NOT_ROW,
                                    objectPtr->line);

            /*
             * Do all PIB-TAGs point to objects with a SYNTAX of TagId?
             * See RFC 3159 7.12
             */
            if (objectPtr->typePtr && objectPtr->typePtr->export.name &&
                !strcmp(objectPtr->typePtr->export.name, "TagReferenceId") &&
                objectPtr->relatedPtr && objectPtr->relatedPtr->typePtr &&
                objectPtr->relatedPtr->typePtr->export.name &&
                strcmp(objectPtr->relatedPtr->typePtr->export.name, "TagId"))
                smiPrintErrorAtLine(parserPtr, ERR_PIB_TAG_TYPE, objectPtr->line);
            
            /*
             * Is the attribute member of at least one compliance group?
             * See RFC 3159 9.1
             */
            if (objectPtr->export.nodekind & SMI_NODEKIND_COLUMN) {
                Object *group;
                int found = 0;
                
                for (group = modulePtr->firstObjectPtr; group;
                     group = group->nextPtr) {
                    if ((group->export.nodekind == SMI_NODEKIND_GROUP) &&
                        group->listPtr) {
                        List *l;
                        
                        for (l = group->listPtr; l; l = l->nextPtr)
                            if (((Object *)l->ptr)->export.name &&
                                !strcmp(((Object *)l->ptr)->export.name,
                                        objectPtr->export.name)) {
                                found = 1;
                                break;
                            }
                    }
                    if (found)
                        break;
                }
                if (!found)
                    smiPrintErrorAtLine(parserPtr, ERR_ATTRIBUTE_NOT_IN_GROUP,
                                        objectPtr->line, objectPtr->export.name);
            }
        }
    }
}



static void
checkTypes(Parser *parserPtr, Module *modulePtr)
{
    Type *typePtr;
    
    for (typePtr = modulePtr->firstTypePtr;
	typePtr; typePtr = typePtr->nextPtr) {

	/*
	 * Check references to unknown types.
	 */
	
	if ((typePtr->flags & FLAG_INCOMPLETE)
	    && typePtr->export.name
	    && (typePtr->export.decl == SMI_DECL_UNKNOWN)) {
	    smiPrintErrorAtLine(parserPtr, ERR_UNKNOWN_TYPE,
				typePtr->line, typePtr->export.name);
	}

	/*
	 * Use TCs instead of type assignments in SMIv2.
	 */

	if (thisModulePtr->export.language == SMI_LANGUAGE_SMIV2
	    && typePtr->export.decl == SMI_DECL_TYPEASSIGNMENT
	    && typePtr->export.basetype != SMI_BASETYPE_UNKNOWN
	    && strcmp(thisModulePtr->export.name, "SNMPv2-SMI")) {
	    smiPrintErrorAtLine(parserPtr, ERR_SMIV2_TYPE_ASSIGNEMENT,
				typePtr->line, typePtr->export.name);
	}

	/*
	 * Check whether we have types that are not used in this
	 * module.
	 */

	if ((typePtr->export.decl == SMI_DECL_TYPEDEF
	     || typePtr->export.decl == SMI_DECL_TEXTUALCONVENTION)
	    && ! (typePtr->flags & FLAG_INSYNTAX)) {
	    static char *status[] = { "Unknown", "current", "deprecated",
				      "mandatory", "optional", "obsolete" };
	    smiPrintErrorAtLine(parserPtr, ERR_TYPE_UNREF,
				typePtr->line,
				status[typePtr->export.status],
				typePtr->export.name);
	}

	/*
	 * Complain about TCs derived from other TCs (RFC 2579 3.5).
	 */
	
	if (typePtr->parentPtr
	    && typePtr->export.decl == SMI_DECL_TEXTUALCONVENTION
	    && typePtr->export.decl == typePtr->parentPtr->export.decl) {
	    smiPrintErrorAtLine(parserPtr, ERR_SMIV2_NESTED_TEXTUAL_CONVENTION,
				typePtr->line,
				typePtr->export.name,
				typePtr->parentPtr->export.name);

	    if (typePtr->export.status < typePtr->parentPtr->export.status) {
		if (typePtr->parentPtr->export.status == SMI_STATUS_DEPRECATED) {
		    smiPrintErrorAtLine(parserPtr, ERR_TYPE_STATUS_DEPRECATED,
					typePtr->line,
					typePtr->parentPtr->export.name,
					typePtr->export.name);
		}
		if (typePtr->parentPtr->export.status == SMI_STATUS_OBSOLETE) {
		    smiPrintErrorAtLine(parserPtr, ERR_TYPE_STATUS_OBSOLETE,
					typePtr->line,
					typePtr->parentPtr->export.name,
					typePtr->export.name);
		}
	    }
	}

	smiCheckTypeFormat(parserPtr, typePtr);
	smiCheckNamedNumberRedefinition(parserPtr, typePtr);
	smiCheckNamedNumberSubtyping(parserPtr, typePtr);
    }
}


				
static void
adjustDefval(Parser *parserPtr, SmiValue *valuePtr, Type *typePtr, int line)
{
    Object *object2Ptr;
    List *bitsListPtr, *valueListPtr, *p, *pp, *nextPtr, *listPtr;
    Import *importPtr;
    int nBits, bit;
    
    if (valuePtr->basetype == SMI_BASETYPE_UNKNOWN)
	return;
	
    if (valuePtr->basetype == SMI_BASETYPE_OBJECTIDENTIFIER) {
	/* a len of -1 indicates an unresolved label in ptr */
	if (valuePtr->len == -1) {
	    object2Ptr = findObjectByModuleAndName(parserPtr->modulePtr,
						  (char *)valuePtr->value.ptr);
	    if (!object2Ptr) {
		importPtr = findImportByName(
		    (char *)valuePtr->value.ptr, parserPtr->modulePtr);
		if (importPtr) {		/* imported object */
		    importPtr->use++;
		    object2Ptr = findObjectByModulenameAndName(
			importPtr->export.module,
			importPtr->export.name);
		}
	    }
	    if (!object2Ptr) {
		smiPrintErrorAtLine(parserPtr, ERR_UNKNOWN_OIDLABEL,
				    line,
				    (char *)valuePtr->value.ptr);
		smiFree(valuePtr->value.ptr);
		valuePtr->value.ptr = NULL;
		valuePtr->basetype = SMI_BASETYPE_UNKNOWN;
	    } else {
		smiFree(valuePtr->value.ptr);
		valuePtr->len = object2Ptr->export.oidlen;
		valuePtr->value.ptr = smiMalloc(object2Ptr->export.oidlen *
						sizeof(SmiSubid));
		memcpy(valuePtr->value.ptr,
		       object2Ptr->export.oid,
		       object2Ptr->export.oidlen * sizeof(SmiSubid));
	    }
	}
    } else if (valuePtr->basetype == SMI_BASETYPE_BITS) {
	bitsListPtr = typePtr->listPtr;
	valueListPtr = (void *)valuePtr->value.ptr;
	for (nBits = 0, p = bitsListPtr; p; p = p->nextPtr) {
	    if (nBits < 1+((NamedNumber *)(p->ptr))->export.value.value.integer32) {
		nBits = 1+((NamedNumber *)(p->ptr))->export.value.value.integer32;
	    }
	}
	valuePtr->value.ptr = smiMalloc((nBits+7)/8);
	memset(valuePtr->value.ptr, 0, (nBits+7)/8);
	valuePtr->len = (nBits+7)/8;
	for (p = valueListPtr; p;) {
	    for (pp = bitsListPtr; pp; pp = pp->nextPtr) {
		if (!strcmp(p->ptr,
			    ((NamedNumber *)(pp->ptr))->export.name)) {
		    bit = ((NamedNumber *)(pp->ptr))->export.value.value.integer32;
		    valuePtr->value.ptr[bit/8] |=
			1 << (7-(bit%8));
		}
	    }
	    smiFree(p->ptr);
	    nextPtr = p->nextPtr;
	    smiFree(p);
	    p = nextPtr;
	}
    } else if (valuePtr->basetype == SMI_BASETYPE_ENUM) {
	/* a len of -1 indicates an unresolved enum label in ptr */
	if (valuePtr->len == -1) {
	    for (listPtr = typePtr->listPtr; listPtr;
		 listPtr = listPtr->nextPtr) {
		if (!strcmp(((NamedNumber *)(listPtr->ptr))->export.name,
			    (char *)valuePtr->value.ptr)) {
		    smiFree(valuePtr->value.ptr);
		    valuePtr->value.integer32 =
			((NamedNumber *)(listPtr->ptr))->
			export.value.value.integer32;
		    valuePtr->len = 1;
		    break;
		}
	    }
	}
    }
}



static void
checkDefvals(Parser *parserPtr, Module *modulePtr)
{
    Object *objectPtr;
    
    /*
     * Check unknown identifiers in OID DEFVALs.
     */

    for(objectPtr = modulePtr->firstObjectPtr;
	objectPtr; objectPtr = objectPtr->nextPtr) {

	adjustDefval(parserPtr, &objectPtr->export.value,
		     objectPtr->typePtr, objectPtr->line);
	
	if (objectPtr->export.value.len == -1) {
	    smiPrintErrorAtLine(parserPtr,
				ERR_DEFVAL_SYNTAX, objectPtr->line);
	}
	
	smiCheckDefault(parserPtr, objectPtr);
    }
}



static void
checkImportsUsage(Parser *parserPtr, Module *modulePtr)
{
    Import *importPtr;
    
    /*
     * Check usage of all imported identifiers.
     */

    if (strcmp(modulePtr->export.name, "SNMPv2-TC") &&
	strcmp(modulePtr->export.name, "SNMPv2-CONF") &&
	strcmp(modulePtr->export.name, "RFC-1212") &&
	strcmp(modulePtr->export.name, "RFC-1215")) {
	for(importPtr = modulePtr->firstImportPtr;
	    importPtr; importPtr = importPtr->nextPtr) {
	    if (! strcmp(importPtr->export.module, "SNMPv2-SMI")) {
		if (! strcmp(importPtr->export.name, "ExtUTCTime")
		    || !strcmp(importPtr->export.name, "ObjectName")
		    || !strcmp(importPtr->export.name, "NotificationName")) {
		    smiPrintErrorAtLine(parserPtr, ERR_ILLEGAL_IMPORT,
					importPtr->line,
					importPtr->export.name,
					importPtr->export.module);
		}
	    }
	    /* checkImports() handles KIND_NOTFOUND */
	    if (importPtr->use == 0 && importPtr->kind != KIND_NOTFOUND) {
		smiPrintErrorAtLine(parserPtr, ERR_UNUSED_IMPORT,
				    importPtr->line,
				    importPtr->export.name,
				    importPtr->export.module);
	    }

	    if (modulePtr->export.language == SMI_LANGUAGE_SMIV2) {
		int j;
		for (j = 0; convertImportv2[j]; j += 4) {
		    if ((strcmp(convertImportv2[j],
				importPtr->export.module) == 0)
			&& (strcmp(convertImportv2[j+1],
				   importPtr->export.name) == 0)) {
			smiPrintErrorAtLine(parserPtr,
					    ERR_OBSOLETE_IMPORT,
					    importPtr->line,
					    importPtr->export.name,
					    convertImportv2[j+2],
					    importPtr->export.module);
		    }
		}
	    }
	}
    }
}



static time_t
checkDate(Parser *parserPtr, char *date)
{
    struct tm	tm;
    time_t	anytime;
    int		i, len;
    char	*p;
    
    memset(&tm, 0, sizeof(tm));
    anytime = 0;
    
    len = strlen(date);
    if (len == 11 || len == 13) {
	for (i = 0; i < len; i++) {
	    if ( (i < len-1 && ! isdigit((int) date[i]))
		 || (i == len-1 && date[len-1] != 'Z')) {
		smiPrintError(parserPtr, ERR_DATE_CHARACTER, date);
		anytime = (time_t) -1;
		break;
	    }
	}
    } else {
	smiPrintError(parserPtr, ERR_DATE_LENGTH, date);
	anytime = (time_t) -1;
    }

    if (anytime == 0) {
	for (i = 0, p = date, tm.tm_year = 0;
	     i < ((len == 11) ? 2 : 4); i++, p++) {
	    tm.tm_year = tm.tm_year * 10 + (*p - '0');
	}
	if (len == 11) {
	    tm.tm_year += 1900;
	    if (tm.tm_year < 1990)
		smiPrintError(parserPtr, ERR_DATE_YEAR_2DIGITS,
			      date, tm.tm_year);
	}
	tm.tm_mon  = (p[0]-'0') * 10 + (p[1]-'0');
	p += 2;
	tm.tm_mday = (p[0]-'0') * 10 + (p[1]-'0');
	p += 2;
	tm.tm_hour = (p[0]-'0') * 10 + (p[1]-'0');
	p += 2;
	tm.tm_min  = (p[0]-'0') * 10 + (p[1]-'0');
	
	if (tm.tm_mon < 1 || tm.tm_mon > 12) {
	    smiPrintError(parserPtr, ERR_DATE_MONTH, date);
	}
	if (tm.tm_mday < 1 || tm.tm_mday > 31) {
	    smiPrintError(parserPtr, ERR_DATE_DAY, date);
	}
	if (tm.tm_hour < 0 || tm.tm_hour > 23) {
	    smiPrintError(parserPtr, ERR_DATE_HOUR, date);
	}
	if (tm.tm_min < 0 || tm.tm_min > 59) {
	    smiPrintError(parserPtr, ERR_DATE_MINUTES, date);
	}
	
	tm.tm_year -= 1900;
	tm.tm_mon -= 1;
	tm.tm_isdst = 0;

	anytime = timegm(&tm);
	
	if (anytime == (time_t) -1) {
	    smiPrintError(parserPtr, ERR_DATE_VALUE, date);
	} else {
	    if (anytime < SMI_EPOCH) {
		smiPrintError(parserPtr, ERR_DATE_IN_PAST, date);
	    }
	    if (anytime > time(NULL)) {
		smiPrintError(parserPtr, ERR_DATE_IN_FUTURE, date);
	    }
	}
    }

    return (anytime == (time_t) -1) ? 0 : anytime;
}

%}

/*
 * The grammars start symbol.
 */
%start mibFile



/*
 * We call the parser from within the parser when IMPORTing modules,
 * hence we need reentrant parser code. This is a bison feature.
 */
%pure_parser



/*
 * The attributes.
 */
%union {
    char           *text;	  		/* scanned quoted text       */
    char           *id;				/* identifier name           */
    int            err;				/* actually just a dummy     */
    time_t	   date;			/* a date value              */
    Object         *objectPtr;			/* object identifier         */
    SmiStatus      status;			/* a STATUS value            */
    SmiAccess      access;			/* an ACCESS value           */
    Type           *typePtr;
    List           *listPtr;			/* SEQUENCE and INDEX lists  */
    NamedNumber    *namedNumberPtr;		/* BITS or enum item         */
    Range          *rangePtr;			/* type restricting range    */
    SmiValue	   *valuePtr;
    SmiUnsigned32  unsigned32;			/*                           */
    SmiInteger32   integer32;			/*                           */
    SmiUnsigned64  unsigned64;			/*                           */
    SmiInteger64   integer64;			/*                           */
    struct Compl   compl;
    struct Index   index;
    Module	   *modulePtr;
    SubjectCategories *subjectCategoriesPtr;
}



/*
 * Tokens and their attributes.
 */
%token DOT_DOT
%token COLON_COLON_EQUAL

%token <id>UPPERCASE_IDENTIFIER
%token <id>LOWERCASE_IDENTIFIER
%token <unsigned32>NUMBER
%token <integer32>NEGATIVENUMBER
%token <unsigned64>NUMBER64
%token <integer64>NEGATIVENUMBER64
%token <text>BIN_STRING
%token <text>HEX_STRING
%token <text>QUOTED_STRING

%token <id>ACCESS
%token <id>AGENT_CAPABILITIES
%token <id>APPLICATION
%token <id>AUGMENTS
%token <id>BEGIN_
%token <id>BITS
%token <id>CHOICE
%token <id>CONTACT_INFO
%token <id>CREATION_REQUIRES
%token <id>COUNTER32
%token <id>COUNTER64
%token <id>DEFINITIONS
%token <id>DEFVAL
%token <id>DESCRIPTION
%token <id>DISPLAY_HINT
%token <id>END
%token <id>ENTERPRISE
%token <id>EXPORTS
%token <id>EXTENDS
%token <id>FROM
%token <id>GROUP
%token <id>GAUGE32
%token <id>IDENTIFIER
%token <id>IMPLICIT
%token <id>IMPLIED
%token <id>IMPORTS
%token <id>INCLUDES
%token <id>INDEX
%token <id>INSTALL_ERRORS
%token <id>INTEGER
%token <id>INTEGER32
%token <id>INTEGER64
%token <id>IPADDRESS
%token <id>LAST_UPDATED
%token <id>MACRO
%token <id>MANDATORY_GROUPS
%token <id>MAX_ACCESS
%token <id>MIN_ACCESS
%token <id>MODULE
%token <id>MODULE_COMPLIANCE
%token <id>MODULE_IDENTITY
%token <id>NOT_ACCESSIBLE
%token <id>NOTIFICATIONS
%token <id>NOTIFICATION_GROUP
%token <id>NOTIFICATION_TYPE
%token <id>OBJECT
%token <id>OBJECT_GROUP
%token <id>OBJECT_IDENTITY
%token <id>OBJECT_TYPE
%token <id>OBJECTS
%token <id>OCTET
%token <id>OF
%token <id>ORGANIZATION
%token <id>OPAQUE
%token <id>PIB_ACCESS
%token <id>PIB_DEFINITIONS
%token <id>PIB_INDEX
%token <id>PIB_MIN_ACCESS
%token <id>PIB_REFERENCES
%token <id>PIB_TAG
%token <id>POLICY_ACCESS
%token <id>PRODUCT_RELEASE
%token <id>REFERENCE
%token <id>REVISION
%token <id>SEQUENCE
%token <id>SIZE
%token <id>STATUS
%token <id>STRING
%token <id>SUBJECT_CATEGORIES
%token <id>SUPPORTS
%token <id>SYNTAX
%token <id>TEXTUAL_CONVENTION
%token <id>TIMETICKS
%token <id>TRAP_TYPE
%token <id>UNIQUENESS
%token <id>UNITS
%token <id>UNIVERSAL
%token <id>UNSIGNED32
%token <id>UNSIGNED64
%token <id>VALUE
%token <id>VARIABLES
%token <id>VARIATION
%token <id>WRITE_SYNTAX



/*
 * Types of non-terminal symbols.
 */
%type  <err>mibFile
%type  <err>modules
%type  <err>module
%type  <err>moduleOid
%type  <id>moduleName
%type  <id>importIdentifier
%type  <err>importIdentifiers
%type  <id>importedKeyword
%type  <id>importedSMIKeyword
%type  <id>importedSPPIKeyword
%type  <err>linkagePart
%type  <err>linkageClause
%type  <err>importPart
%type  <err>imports
%type  <err>declarationPart
%type  <err>declarations
%type  <err>declaration
%type  <err>exportsClause
%type  <err>macroClause
%type  <id>macroName
%type  <typePtr>choiceClause
%type  <id>typeName
%type  <id>typeSMI
%type  <id>typeSMIonly
%type  <id>typeSMIandSPPI
%type  <id>typeSPPIonly
%type  <err>typeTag
%type  <id>fuzzy_lowercase_identifier
%type  <err>valueDeclaration
%type  <typePtr>conceptualTable
%type  <typePtr>row
%type  <typePtr>entryType
%type  <listPtr>sequenceItems
%type  <objectPtr>sequenceItem
%type  <typePtr>Syntax
%type  <typePtr>sequenceSyntax
%type  <listPtr>NamedBits
%type  <namedNumberPtr>NamedBit
%type  <err>objectIdentityClause
%type  <err>objectTypeClause
%type  <err>trapTypeClause
%type  <text>descriptionClause
%type  <listPtr>VarPart
%type  <listPtr>VarTypes
%type  <objectPtr>VarType
%type  <text>DescrPart
%type  <access>MaxAccessPart
%type  <access>MaxOrPIBAccessPart
%type  <access>PibAccessPart
%type  <err>notificationTypeClause
%type  <err>moduleIdentityClause
%type  <err>typeDeclaration
%type  <typePtr>typeDeclarationRHS
%type  <typePtr>ObjectSyntax
%type  <typePtr>sequenceObjectSyntax
%type  <valuePtr>valueofObjectSyntax
%type  <typePtr>SimpleSyntax
%type  <valuePtr>valueofSimpleSyntax
%type  <typePtr>sequenceSimpleSyntax
%type  <typePtr>ApplicationSyntax
%type  <typePtr>sequenceApplicationSyntax
%type  <listPtr>anySubType
%type  <listPtr>integerSubType
%type  <listPtr>octetStringSubType
%type  <listPtr>ranges
%type  <rangePtr>range
%type  <valuePtr>value
%type  <listPtr>enumSpec
%type  <listPtr>enumItems
%type  <namedNumberPtr>enumItem
%type  <valuePtr>enumNumber
%type  <status>Status
%type  <status>Status_Capabilities
%type  <text>DisplayPart
%type  <text>UnitsPart
%type  <access>Access
%type  <index>IndexPart
%type  <index>MibIndex
%type  <listPtr>IndexTypes
%type  <objectPtr>IndexType
%type  <objectPtr>Index
%type  <objectPtr>Entry
%type  <valuePtr>DefValPart
%type  <valuePtr>Value
%type  <listPtr>BitsValue
%type  <listPtr>BitNames
%type  <objectPtr>ObjectName
%type  <objectPtr>NotificationName
%type  <text>ReferPart
%type  <err>RevisionPart
%type  <err>Revisions
%type  <err>Revision
%type  <listPtr>NotificationObjectsPart
%type  <listPtr>ObjectGroupObjectsPart
%type  <listPtr>Objects
%type  <objectPtr>Object
%type  <listPtr>NotificationsPart
%type  <listPtr>Notifications
%type  <objectPtr>Notification
%type  <text>Text
%type  <date>ExtUTCTime
%type  <objectPtr>objectIdentifier
%type  <objectPtr>subidentifiers
%type  <objectPtr>subidentifier
%type  <text>objectIdentifier_defval
%type  <err>subidentifiers_defval
%type  <err>subidentifier_defval
%type  <err>objectGroupClause
%type  <err>notificationGroupClause
%type  <err>moduleComplianceClause
%type  <compl>ComplianceModulePart
%type  <compl>ComplianceModules
%type  <compl>ComplianceModule
%type  <modulePtr>ComplianceModuleName
%type  <listPtr>MandatoryPart
%type  <listPtr>MandatoryGroups
%type  <objectPtr>MandatoryGroup
%type  <compl>CompliancePart
%type  <compl>Compliances
%type  <compl>Compliance
%type  <listPtr>ComplianceGroup
%type  <listPtr>ComplianceObject
%type  <typePtr>SyntaxPart
%type  <typePtr>WriteSyntaxPart
%type  <typePtr>WriteSyntax
%type  <access>AccessPart
%type  <err>agentCapabilitiesClause
%type  <err>ModulePart_Capabilities
%type  <err>Modules_Capabilities
%type  <err>Module_Capabilities
%type  <modulePtr>ModuleName_Capabilities
%type  <listPtr>CapabilitiesGroups
%type  <listPtr>CapabilitiesGroup
%type  <err>VariationPart
%type  <err>Variations
%type  <err>Variation
%type  <access>VariationAccessPart
%type  <access>VariationAccess
%type  <err>CreationPart
%type  <err>Cells
%type  <err>Cell
%type  <objectPtr>SPPIPibReferencesPart
%type  <objectPtr>SPPIPibTagPart
%type  <subjectCategoriesPtr>SubjectCategoriesPart
%type  <subjectCategoriesPtr>SubjectCategories
%type  <listPtr>CategoryIDs
%type  <objectPtr>CategoryID
%type  <objectPtr>UniqueType
%type  <listPtr>UniqueTypes
%type  <listPtr>UniqueTypesPart
%type  <listPtr>SPPIUniquePart
%type  <objectPtr>Error
%type  <listPtr>Errors
%type  <listPtr>SPPIErrorsPart

%%

/*
 * Yacc rules.
 *
 */


/*
 * One mibFile may contain multiple MIB modules.
 * It's also possible that there's no module in a file.
 */
mibFile:
    modules
    {
        $$ = 0;
    }
| /* empty */
    {
	$$ = 0;
    }
;

modules:		module
			{ $$ = 0; }
	|		modules module
			{ $$ = 0; }
	;

/*
 * The general structure of a module is described at REF:RFC1902,3. .
 * An example is given at REF:RFC1902,5.7. .
 */
module:			moduleName
			{
			    thisParserPtr->firstStatementLine = thisParserPtr->line;
			    thisParserPtr->currentDecl = SMI_DECL_MODULE;
			    
			    thisParserPtr->modulePtr = findModuleByName($1);
			    if (!thisParserPtr->modulePtr) {
				thisParserPtr->modulePtr =
				    addModule($1,
					      smiStrdup(thisParserPtr->path),
					      0,
					      thisParserPtr);
				thisParserPtr->modulePtr->
				    numImportedIdentifiers = 0;
				thisParserPtr->modulePtr->
				    numStatements = 0;
				thisParserPtr->modulePtr->
				    numModuleIdentities = 0;
				if (!strcmp($1, "SNMPv2-SMI")) {
			            /*
				     * SNMPv2-SMI is an SMIv2 module
				     * that cannot be identified by
				     * importing from SNMPv2-SMI.
				     */
				    thisModulePtr->export.language =
					SMI_LANGUAGE_SMIV2;
				}
			    } else {
			        smiPrintError(thisParserPtr,
					      ERR_MODULE_ALREADY_LOADED,
					      $1);
				/*
				 * this aborts parsing the whole file,
				 * not only the current module.
				 */
				YYABORT;
			    }
			}
			moduleOid
			definitions
                        COLON_COLON_EQUAL BEGIN_
			exportsClause
			linkagePart
			declarationPart
			END
			{
			    if (thisModulePtr->export.language == SMI_LANGUAGE_UNKNOWN)
				thisModulePtr->export.language = SMI_LANGUAGE_SMIV1;
			    checkModuleName(thisParserPtr, thisModulePtr);
			    checkModuleIdentity(thisParserPtr, thisModulePtr);
			    checkObjects(thisParserPtr, thisModulePtr);
			    checkTypes(thisParserPtr, thisModulePtr);
			    checkDefvals(thisParserPtr, thisModulePtr);
			    checkImportsUsage(thisParserPtr, thisModulePtr);
			    smiCheckTypeUsage(thisParserPtr, thisModulePtr);
			    
			    thisParserPtr->capabilitiesModulePtr = NULL;

                            $$ = 0;
			}
	;

moduleOid:		'{' objectIdentifier '}'
			{ $$ = 0; }
	|		/* empty */
			{ $$ = 0; }
	;

definitions:            DEFINITIONS
                        { }
        |               PIB_DEFINITIONS
                        {
                            thisModulePtr->export.language = SMI_LANGUAGE_SPPI;
                        }
        ;

/*
 * REF:RFC1902,3.2.
 */
linkagePart:		linkageClause
			{ $$ = 0; }
	|		/* empty */
			{ $$ = 0; }
	;

linkageClause:		IMPORTS importPart ';'
			{
			    thisParserPtr->firstStatementLine = thisParserPtr->line;
			    thisParserPtr->currentDecl = SMI_DECL_MODULE;

			    if ((thisModulePtr->export.language != SMI_LANGUAGE_SMIV2) &&
                                (thisModulePtr->export.language != SMI_LANGUAGE_SPPI))
				thisModulePtr->export.language = SMI_LANGUAGE_SMIV1;
			    
			    $$ = 0;
			}
			    
        ;

exportsClause:		/* empty */
			{ $$ = 0; }
	|		EXPORTS
			{
                            if (thisParserPtr->modulePtr->export.language ==
                                 SMI_LANGUAGE_SPPI)
                                smiPrintError(thisParserPtr, ERR_SMI_CONSTRUCT_IN_PIB, "EXPORTS");
                            
			    thisParserPtr->firstStatementLine = thisParserPtr->line;
			    thisParserPtr->currentDecl = SMI_DECL_MODULE;

			    if (strcmp(thisParserPtr->modulePtr->export.name,
				       "RFC1155-SMI") &&
				strcmp(thisParserPtr->modulePtr->export.name,
				       "RFC1065-SMI")) {
			        smiPrintError(thisParserPtr, ERR_EXPORTS);
			    }
			}
			/* the scanner skips until... */
			';'
			{ $$ = 0; }
	;

importPart:		imports
			{ $$ = 0; }
	|		/* empty */
			{ $$ = 0; }
			/* TODO: ``IMPORTS ;'' allowed? refer ASN.1! */
	;

imports:		import
			{ $$ = 0; }
	|		imports import
			{ $$ = 0; }
	;

import:			importIdentifiers FROM moduleName
			/* TODO: multiple clauses with same moduleName
			 * allowed? I guess so. refer ASN.1! */
			{
			    Import      *importPtr;
			    Module      *modulePtr;
                            
			    /*
			     * Recursively call the parser to suffer
			     * the IMPORTS, if the module is not yet
			     * loaded.
			     */
			    modulePtr = findModuleByName($3);
			    if (!modulePtr) {
				modulePtr = loadModule($3, thisParserPtr);
			    }
			    checkImports(modulePtr, thisParserPtr);

			    if (modulePtr && !strcmp($3, "SNMPv2-SMI")) {
			        /*
				 * A module that imports a macro or
				 * type definition from SNMPv2-SMI
				 * seems to be SMIv2 style - but only if
                                 * it is not SPPI yet.
				 */
                                if (thisModulePtr->export.language != SMI_LANGUAGE_SPPI) {
				    for (importPtr =
					     thisModulePtr->firstImportPtr;
				         importPtr;
				         importPtr = importPtr->nextPtr) {
				        if ((!strcmp(importPtr->export.module,
						     $3)) &&
					    ((importPtr->kind == KIND_MACRO) ||
					     (importPtr->kind == KIND_TYPE))) {
					    thisModulePtr->export.language =
					        SMI_LANGUAGE_SMIV2;
				        }
				    }
                                }
			    }

			    smiFree($3);
			}
	;

importIdentifiers:	importIdentifier
			{ $$ = 0; }
	|		importIdentifiers ',' importIdentifier
			/* TODO: might this list list be empty? */
			{ $$ = 0; }
	;

/*
 * Note that some named types must not be imported, REF:RFC1902,590 .
 */
importIdentifier:	LOWERCASE_IDENTIFIER
			{
			    addImport($1, thisParserPtr);
			    thisParserPtr->modulePtr->numImportedIdentifiers++;
			    $$ = 0;
			}
	|		UPPERCASE_IDENTIFIER
			{
			    addImport($1, thisParserPtr);
			    thisParserPtr->modulePtr->numImportedIdentifiers++;
			    $$ = 0;
			}
	|		importedKeyword
			{
			    addImport(smiStrdup($1), thisParserPtr);
			    thisParserPtr->modulePtr->numImportedIdentifiers++;
			    $$ = 0;
			}
	;

/*
 * These keywords are no real keywords. They have to be imported
 * from the SMI, TC, CONF MIBs.
 */
/*
 * TODO: Think! Shall we really leave these words as keywords or should
 * we prefer the symbol table appropriately??
 */
importedKeyword:	importedSMIKeyword
                        {
                            /*
                             * There are PIBs that import e.g. Counter64 - so
                             * don't complain here about SMI keywords.
                             */
                           /* if (thisParserPtr->modulePtr->export.language == SMI_LANGUAGE_SPPI)
                              smiPrintError(thisParserPtr, ERR_SMI_CONSTRUCT_IN_PIB, $1);*/
                            $$ = $1;
                        }
        |               importedSPPIKeyword
                        {
                            if (thisParserPtr->modulePtr->export.language != SMI_LANGUAGE_SPPI)
				smiPrintError(thisParserPtr, ERR_SPPI_TYPE_IN_MIB, $1);
                            $$ = $1;
                        }
        |               BITS
	|		INTEGER32
	|		IPADDRESS
	|		MANDATORY_GROUPS
	|		MODULE_COMPLIANCE
	|		MODULE_IDENTITY
	|		OBJECT_GROUP
	|		OBJECT_IDENTITY
	|		OBJECT_TYPE
	|		OPAQUE
	|		TEXTUAL_CONVENTION
	|		TIMETICKS
	|		UNSIGNED32
        ;

importedSMIKeyword:     AGENT_CAPABILITIES
	|		COUNTER32
	|		COUNTER64
	|		GAUGE32
	|		NOTIFICATION_GROUP
	|		NOTIFICATION_TYPE
	|		TRAP_TYPE
	;

importedSPPIKeyword:	INTEGER64
	|		UNSIGNED64
	;

moduleName:		UPPERCASE_IDENTIFIER
			{
			    checkNameLen(thisParserPtr, $1,
					 ERR_MODULENAME_32, ERR_MODULENAME_64);
			    $$ = $1;
			}
	;

/*
 * The paragraph at REF:RFC1902,490 lists roughly what's allowed
 * in the body of an information module.
 */
declarationPart:	declarations
			{ $$ = 0; }
	|		/* empty */
			{ $$ = 0; }
			/* TODO: might this list really be emtpy? */
	;

declarations:		declaration
			{ $$ = 0; }
	|		declarations declaration
			{ $$ = 0; }
	;

declaration:		typeDeclaration
			{ 
			    thisParserPtr->modulePtr->numStatements++;
			    $$ = 0;
			}
	|		valueDeclaration
			{ 
			    thisParserPtr->modulePtr->numStatements++;
			    $$ = 0;
			}
	|		objectIdentityClause
			{ 
			    thisParserPtr->modulePtr->numStatements++;
			    $$ = 0;
			}
	|		objectTypeClause
			{ 
			    thisParserPtr->modulePtr->numStatements++;
			    $$ = 0;
			}
	|		trapTypeClause
			{ 
			    thisParserPtr->modulePtr->numStatements++;
			    $$ = 0;
			}
	|		notificationTypeClause
			{ 
			    thisParserPtr->modulePtr->numStatements++;
			    $$ = 0;
			}
	|		moduleIdentityClause
			{ 
			    thisParserPtr->modulePtr->numStatements++;
			    $$ = 0;
			}
	|		moduleComplianceClause
			{ 
			    thisParserPtr->modulePtr->numStatements++;
			    $$ = 0;
			}
	|		objectGroupClause
			{ 
			    thisParserPtr->modulePtr->numStatements++;
			    $$ = 0;
			}
	|		notificationGroupClause
			{
			    thisParserPtr->modulePtr->numStatements++;
			    $$ = 0;
			}
	|		agentCapabilitiesClause
			{ 
			    thisParserPtr->modulePtr->numStatements++;
			    $$ = 0;
			}
	|		macroClause
			{ 
			    thisParserPtr->modulePtr->numStatements++;
			    $$ = 0;
			}
	|		error '}'
			{
			    smiPrintError(thisParserPtr,
					  ERR_FLUSH_DECLARATION);
			    yyerrok;
			    $$ = 1;
			}
	;

/*
 * Macro clauses. Its contents are not really parsed, but skipped by
 * the scanner until 'END' is read. This is just to make the SMI
 * documents readable.
 */
macroClause:		macroName
			{
			    Macro *macroPtr;

			    thisParserPtr->firstStatementLine = thisParserPtr->line;
			    thisParserPtr->currentDecl = SMI_DECL_MACRO;

			    macroPtr = addMacro(smiStrdup($1),
						0, thisParserPtr);
			    setMacroLine(macroPtr, thisParserPtr->firstStatementLine,
					 thisParserPtr);
			}
			MACRO
			{
			    /*
			     * ASN.1 macros are known to be in these
			     * modules.
			     */
			    if (strcmp(thisParserPtr->modulePtr->export.name,
				       "SNMPv2-SMI") &&
			        strcmp(thisParserPtr->modulePtr->export.name,
				       "SNMPv2-TC") &&
				strcmp(thisParserPtr->modulePtr->export.name,
				       "SNMPv2-CONF") &&
				strcmp(thisParserPtr->modulePtr->export.name,
				       "RFC-1212") &&
				strcmp(thisParserPtr->modulePtr->export.name,
				       "RFC-1215") &&
				strcmp(thisParserPtr->modulePtr->export.name,
				       "RFC1065-SMI") &&
				strcmp(thisParserPtr->modulePtr->export.name,
				       "RFC1155-SMI") &&
                                strcmp(thisParserPtr->modulePtr->export.name,
                                       "COPS-PR-SPPI") &&
                                strcmp(thisParserPtr->modulePtr->export.name,
                                       "COPS-PR-SPPI-TC")) {
			        smiPrintError(thisParserPtr, ERR_MACRO);
			    }
			}
			/* the scanner skips until... */
			END
			{
			    $$ = 0;
                        }
	;

macroName:		MODULE_IDENTITY     { $$ = $1; }
	|		OBJECT_TYPE	    { $$ = $1; }
	|		TRAP_TYPE	    { $$ = $1; }
	|		NOTIFICATION_TYPE   { $$ = $1; }
	|		OBJECT_IDENTITY	    { $$ = $1; }
	|		TEXTUAL_CONVENTION  { $$ = $1; }
	|		OBJECT_GROUP	    { $$ = $1; }
	|		NOTIFICATION_GROUP  { $$ = $1; }
	|		MODULE_COMPLIANCE   { $$ = $1; }
	|		AGENT_CAPABILITIES  { $$ = $1; }
	;

choiceClause:		CHOICE
			{
			    if (strcmp(thisParserPtr->modulePtr->export.name,
				       "SNMPv2-SMI") &&
			        strcmp(thisParserPtr->modulePtr->export.name,
				       "SNMPv2-TC") &&
				strcmp(thisParserPtr->modulePtr->export.name,
				       "SNMPv2-CONF") &&
				strcmp(thisParserPtr->modulePtr->export.name,
				       "RFC-1212") &&
				strcmp(thisParserPtr->modulePtr->export.name,
				       "RFC1065-SMI") &&
				strcmp(thisParserPtr->modulePtr->export.name,
				       "RFC1155-SMI") && 
                                strcmp(thisParserPtr->modulePtr->export.name,
                                       "COPS-PR-SPPI")) {
			        smiPrintError(thisParserPtr, ERR_CHOICE);
			    }
			}
			/* the scanner skips until... */
			'}'
			{
			    $$ = addType(NULL, SMI_BASETYPE_UNKNOWN, 0,
					 thisParserPtr);
			}
	;

/*
 * The only ASN.1 value declarations are for OIDs, REF:RFC1902,491 .
 */
fuzzy_lowercase_identifier:	LOWERCASE_IDENTIFIER
			{
			  $$ = $1;
			}
	|
			UPPERCASE_IDENTIFIER
			{
			    smiPrintError (thisParserPtr,
					   ERR_BAD_LOWER_IDENTIFIER_CASE,
					   $1);
			    /* xxx
			    if ((thisParserPtr->flags & SMI_FLAG_BE_LAX) == 0) {
			        YYERROR;
			    }
			    */
			  $$ = $1;
			}
	;

/* valueDeclaration:	LOWERCASE_IDENTIFIER */
valueDeclaration:	fuzzy_lowercase_identifier
			{
			    thisParserPtr->firstStatementLine = thisParserPtr->line;
			    thisParserPtr->currentDecl = SMI_DECL_VALUEASSIGNMENT;

			    checkNameLen(thisParserPtr, $1,
					 ERR_OIDNAME_32, ERR_OIDNAME_64);
			    smiCheckObjectName(thisParserPtr,
					       thisModulePtr, $1);
			    if (thisModulePtr->export.language == SMI_LANGUAGE_SMIV2)
			    {
			        if (strchr($1, '-') &&
				    (strcmp($1, "mib-2") ||
				  strcmp(thisModulePtr->export.name, "SNMPv2-SMI"))) {
				    smiPrintError(thisParserPtr,
						  ERR_OIDNAME_INCLUDES_HYPHEN,
						  $1);
				}
			    }
			}
			OBJECT IDENTIFIER
			COLON_COLON_EQUAL '{' objectIdentifier '}'
			{
			    Object *objectPtr;
			    
			    objectPtr = $7;
			    smiCheckObjectReuse(thisParserPtr, $1, &objectPtr);
			    objectPtr = setObjectName(objectPtr, $1, thisParserPtr);
			    deleteObjectFlags(objectPtr, FLAG_INCOMPLETE);
			    setObjectLine(objectPtr, thisParserPtr->firstStatementLine,
					  thisParserPtr);
			    setObjectDecl(objectPtr,
					  SMI_DECL_VALUEASSIGNMENT);
			    $$ = 0;
			}
	;

/*
 * This is for simple ASN.1 style type assignments and textual conventions.
 */
typeDeclaration:	typeName
			{
			    thisParserPtr->firstStatementLine = thisParserPtr->line;
			    thisParserPtr->currentDecl = SMI_DECL_TYPEASSIGNMENT;

			    checkNameLen(thisParserPtr, $1,
					 ERR_TYPENAME_32, ERR_TYPENAME_64);
			}
			COLON_COLON_EQUAL typeDeclarationRHS
			{
			    Type *typePtr;
			    
			    if (strlen($1)) {
				if ($4->export.basetype != SMI_BASETYPE_UNKNOWN) {
				    smiCheckTypeName(thisParserPtr,
						     thisModulePtr, $1,
						     thisParserPtr->firstStatementLine);
				}
				setTypeLine($4, thisParserPtr->firstStatementLine,
					    thisParserPtr);
				setTypeName($4, $1);
				$$ = 0;
			    } else {
				$$ = 0;
			    }

			    /*
			     * If we are in an SMI module, some type
			     * definitions derived from ASN.1 `INTEGER'
			     * must be modified to libsmi basetypes.
			     */
			    if (thisModulePtr &&
				(!strcmp(thisModulePtr->export.name, "SNMPv2-SMI"))) {
				if (!strcmp($1, "Counter32")) {
				    $4->export.basetype = SMI_BASETYPE_UNSIGNED32;
				    setTypeParent($4, smiHandle->typeUnsigned32Ptr);
				    if ($4->listPtr) {
					((Range *)$4->listPtr->ptr)->export.minValue.basetype = SMI_BASETYPE_UNSIGNED32;
					((Range *)$4->listPtr->ptr)->export.minValue.value.unsigned32 = 0;
					((Range *)$4->listPtr->ptr)->export.maxValue.basetype = SMI_BASETYPE_UNSIGNED32;
					((Range *)$4->listPtr->ptr)->export.maxValue.value.unsigned32 = 4294967295U;
				    }
				} else if (!strcmp($1, "Gauge32")) {
				    $4->export.basetype = SMI_BASETYPE_UNSIGNED32;
				    setTypeParent($4, smiHandle->typeUnsigned32Ptr);
				    if ($4->listPtr) {
					((Range *)$4->listPtr->ptr)->export.minValue.basetype = SMI_BASETYPE_UNSIGNED32;
					((Range *)$4->listPtr->ptr)->export.minValue.value.unsigned32 = 0;
					((Range *)$4->listPtr->ptr)->export.maxValue.basetype = SMI_BASETYPE_UNSIGNED32;
					((Range *)$4->listPtr->ptr)->export.maxValue.value.unsigned32 = 4294967295U;
				    }
				} else if (!strcmp($1, "Unsigned32")) {
				    $4->export.basetype = SMI_BASETYPE_UNSIGNED32;
				    setTypeParent($4, smiHandle->typeUnsigned32Ptr);
				    if ($4->listPtr) {
					((Range *)$4->listPtr->ptr)->export.minValue.basetype = SMI_BASETYPE_UNSIGNED32;
					((Range *)$4->listPtr->ptr)->export.minValue.value.unsigned32 = 0;
					((Range *)$4->listPtr->ptr)->export.maxValue.basetype = SMI_BASETYPE_UNSIGNED32;
					((Range *)$4->listPtr->ptr)->export.maxValue.value.unsigned32 = 4294967295U;
				    }
				} else if (!strcmp($1, "TimeTicks")) {
				    $4->export.basetype = SMI_BASETYPE_UNSIGNED32;
				    setTypeParent($4, smiHandle->typeUnsigned32Ptr);
				    if ($4->listPtr) {
					((Range *)$4->listPtr->ptr)->export.minValue.basetype = SMI_BASETYPE_UNSIGNED32;
					((Range *)$4->listPtr->ptr)->export.minValue.value.unsigned32 = 0;
					((Range *)$4->listPtr->ptr)->export.maxValue.basetype = SMI_BASETYPE_UNSIGNED32;
					((Range *)$4->listPtr->ptr)->export.maxValue.value.unsigned32 = 4294967295U;
				    }
				} else if (!strcmp($1, "Counter64")) {
				    $4->export.basetype = SMI_BASETYPE_UNSIGNED64;
				    if ($4->listPtr) {
					((Range *)$4->listPtr->ptr)->export.minValue.basetype = SMI_BASETYPE_UNSIGNED64;
					((Range *)$4->listPtr->ptr)->export.minValue.value.unsigned64 = 0;
					((Range *)$4->listPtr->ptr)->export.maxValue.basetype = SMI_BASETYPE_UNSIGNED64;
					((Range *)$4->listPtr->ptr)->export.maxValue.value.unsigned64 = SMI_BASETYPE_UNSIGNED64_MAX;
				    }
				    setTypeParent($4, smiHandle->typeUnsigned64Ptr);
				}
			    }
			    if (thisModulePtr &&
				(!strcmp(thisModulePtr->export.name, "RFC1155-SMI") ||
				 !strcmp(thisModulePtr->export.name, "RFC1065-SMI"))) {
				if (!strcmp($1, "Counter")) {
				    $4->export.basetype = SMI_BASETYPE_UNSIGNED32;
				    setTypeParent($4, smiHandle->typeUnsigned32Ptr);
				    if ($4->listPtr) {
					((Range *)$4->listPtr->ptr)->export.minValue.basetype = SMI_BASETYPE_UNSIGNED32;
					((Range *)$4->listPtr->ptr)->export.minValue.value.unsigned32 = 0;
					((Range *)$4->listPtr->ptr)->export.maxValue.basetype = SMI_BASETYPE_UNSIGNED32;
					((Range *)$4->listPtr->ptr)->export.maxValue.value.unsigned32 = 4294967295U;
				    }
				} else if (!strcmp($1, "Gauge")) {
				    $4->export.basetype = SMI_BASETYPE_UNSIGNED32;
				    setTypeParent($4, smiHandle->typeUnsigned32Ptr);
				    if ($4->listPtr) {
					((Range *)$4->listPtr->ptr)->export.minValue.basetype = SMI_BASETYPE_UNSIGNED32;
					((Range *)$4->listPtr->ptr)->export.minValue.value.unsigned32 = 0;
					((Range *)$4->listPtr->ptr)->export.maxValue.basetype = SMI_BASETYPE_UNSIGNED32;
					((Range *)$4->listPtr->ptr)->export.maxValue.value.unsigned32 = 4294967295U;
				    }
				} else if (!strcmp($1, "TimeTicks")) {
				    $4->export.basetype = SMI_BASETYPE_UNSIGNED32;
				    setTypeParent($4, smiHandle->typeUnsigned32Ptr);
				    if ($4->listPtr) {
					((Range *)$4->listPtr->ptr)->export.minValue.basetype = SMI_BASETYPE_UNSIGNED32;
					((Range *)$4->listPtr->ptr)->export.minValue.value.unsigned32 = 0;
					((Range *)$4->listPtr->ptr)->export.maxValue.basetype = SMI_BASETYPE_UNSIGNED32;
					((Range *)$4->listPtr->ptr)->export.maxValue.value.unsigned32 = 4294967295U;
				    }
				} else if (!strcmp($1, "NetworkAddress")) {
				    setTypeName($4, smiStrdup("NetworkAddress"));
				    $4->export.basetype = SMI_BASETYPE_OCTETSTRING;
				    setTypeParent($4, findTypeByModuleAndName(
					                   thisModulePtr,
						           "IpAddress"));
				} else if (!strcmp($1, "IpAddress")) {
				    typePtr = findTypeByModuleAndName(
					thisModulePtr, "NetworkAddress");
				    if (typePtr) 
					setTypeParent(typePtr, $4);
				}
			    }
			    if (thisModulePtr &&
				(!strcmp(thisModulePtr->export.name, "COPS-PR-SPPI"))) {
				if (!strcmp($1, "Unsigned32")) {
				    $4->export.basetype = SMI_BASETYPE_UNSIGNED32;
				    setTypeParent($4, smiHandle->typeUnsigned32Ptr);
				    if ($4->listPtr) {
					((Range *)$4->listPtr->ptr)->export.minValue.basetype = SMI_BASETYPE_UNSIGNED32;
					((Range *)$4->listPtr->ptr)->export.minValue.value.unsigned32 = 0;
					((Range *)$4->listPtr->ptr)->export.maxValue.basetype = SMI_BASETYPE_UNSIGNED32;
					((Range *)$4->listPtr->ptr)->export.maxValue.value.unsigned32 = 4294967295U;
				    }
				} else if (!strcmp($1, "TimeTicks")) {
				    $4->export.basetype = SMI_BASETYPE_UNSIGNED32;
				    setTypeParent($4, smiHandle->typeUnsigned32Ptr);
				    if ($4->listPtr) {
					((Range *)$4->listPtr->ptr)->export.minValue.basetype = SMI_BASETYPE_UNSIGNED32;
					((Range *)$4->listPtr->ptr)->export.minValue.value.unsigned32 = 0;
					((Range *)$4->listPtr->ptr)->export.maxValue.basetype = SMI_BASETYPE_UNSIGNED32;
					((Range *)$4->listPtr->ptr)->export.maxValue.value.unsigned32 = 4294967295U;
				    }
				} else if (!strcmp($1, "Unsigned64")) {
				    $4->export.basetype = SMI_BASETYPE_UNSIGNED64;
				    if ($4->listPtr) {
					((Range *)$4->listPtr->ptr)->export.minValue.basetype = SMI_BASETYPE_UNSIGNED64;
					((Range *)$4->listPtr->ptr)->export.minValue.value.unsigned64 = 0;
					((Range *)$4->listPtr->ptr)->export.maxValue.basetype = SMI_BASETYPE_UNSIGNED64;
					((Range *)$4->listPtr->ptr)->export.maxValue.value.unsigned64 = SMI_BASETYPE_UNSIGNED64_MAX;
				    }
				    setTypeParent($4, smiHandle->typeUnsigned64Ptr);
				} else if (!strcmp($1, "Integer64")) {
				    $4->export.basetype = SMI_BASETYPE_INTEGER64;
				    if ($4->listPtr) {
					((Range *)$4->listPtr->ptr)->export.minValue.basetype = SMI_BASETYPE_INTEGER64;
					((Range *)$4->listPtr->ptr)->export.minValue.value.integer64 = SMI_BASETYPE_INTEGER64_MIN;
					((Range *)$4->listPtr->ptr)->export.maxValue.basetype = SMI_BASETYPE_INTEGER64;
					((Range *)$4->listPtr->ptr)->export.maxValue.value.integer64 = SMI_BASETYPE_INTEGER64_MAX;
				    }
				    setTypeParent($4, smiHandle->typeInteger64Ptr);
				}
			    }
			}
	;

typeName:		UPPERCASE_IDENTIFIER
			{
			    $$ = $1;
			}
	|		typeSMI
			{
			    $$ = smiStrdup($1);
			    /*
			     * well known types (keywords in this grammar)
			     * are known to be defined in these modules.
			     */
			    if (strcmp(thisParserPtr->modulePtr->export.name,
				       "SNMPv2-SMI") &&
			        strcmp(thisParserPtr->modulePtr->export.name,
				       "SNMPv2-TC") &&
				strcmp(thisParserPtr->modulePtr->export.name,
				       "SNMPv2-CONF") &&
				strcmp(thisParserPtr->modulePtr->export.name,
				       "RFC-1212") &&
				strcmp(thisParserPtr->modulePtr->export.name,
				       "RFC1065-SMI") &&
				strcmp(thisParserPtr->modulePtr->export.name,
				       "RFC1155-SMI") &&
				strcmp(thisParserPtr->modulePtr->export.name,
				       "COPS-PR-SPPI")) {
			        smiPrintError(thisParserPtr, ERR_TYPE_SMI_OR_SPPI, $1);
			    }
			}
        |               typeSPPIonly
                        {
			    $$ = smiStrdup($1);
			    /*
			     * well known types (keywords in this grammar)
			     * are known to be defined in these modules.
			     */
			    if ((strcmp(thisParserPtr->modulePtr->export.name,
					"COPS-PR-SPPI")) &&
				(thisParserPtr->modulePtr->export.language == SMI_LANGUAGE_SPPI))
			        smiPrintError(thisParserPtr, ERR_TYPE_SPPI, $1);
                        }
	;

typeSMI:                typeSMIandSPPI
        |               typeSMIonly
                        {
                            if ((thisParserPtr->modulePtr->export.language == SMI_LANGUAGE_SPPI) &&
                                !findImportByName($1, thisParserPtr->modulePtr))
                                smiPrintError(thisParserPtr, ERR_SMI_CONSTRUCT_IN_PIB, $1);
                            $$ = $1;
                        }
        ;

typeSMIandSPPI:		IPADDRESS
	|		TIMETICKS
	|		OPAQUE
	|		INTEGER32
	|		UNSIGNED32
        ;

typeSMIonly:		COUNTER32
	|		GAUGE32
	|		COUNTER64
	;

typeSPPIonly:           INTEGER64
        |               UNSIGNED64
        ;

typeDeclarationRHS:	Syntax
			{
			    if ($1->export.name) {
				/*
				 * If we found an already defined type,
				 * we have to inherit a new type structure.
				 * (Otherwise the `Syntax' rule created
				 * a new type for us.)
				 */
				$$ = duplicateType($1, 0, thisParserPtr);
				setTypeDecl($$, SMI_DECL_TYPEASSIGNMENT);
			    } else {
				$$ = $1;
				setTypeDecl($$, SMI_DECL_TYPEASSIGNMENT);
			    }
			}
	|		TEXTUAL_CONVENTION
			{
			    Import *importPtr;

			    thisParserPtr->currentDecl = SMI_DECL_TEXTUALCONVENTION;

			    if (thisModulePtr->export.language == SMI_LANGUAGE_UNKNOWN)
				thisModulePtr->export.language = SMI_LANGUAGE_SMIV2;

			    if (strcmp(thisModulePtr->export.name, "SNMPv2-TC")) {
				importPtr =
				    findImportByName("TEXTUAL-CONVENTION",
						     thisModulePtr);
				if (importPtr) {
				    importPtr->use++;
				} else {
                                    if (thisParserPtr->modulePtr->export.language != SMI_LANGUAGE_SPPI)
				        smiPrintError(thisParserPtr,
						      ERR_MACRO_NOT_IMPORTED,
						      "TEXTUAL-CONVENTION",
						      "SNMPv2-TC");
                                    else
				        smiPrintError(thisParserPtr,
						      ERR_MACRO_NOT_IMPORTED,
						      "TEXTUAL-CONVENTION",
						      "COPS-PR-SPPI");
				}
			    }
			}
			DisplayPart
			STATUS Status
			DESCRIPTION Text
			{
			    checkDescr(thisParserPtr, $7);
			}
			ReferPart
			SYNTAX Syntax
			{
			    if (($11) && !($11->export.name)) {
				/*
				 * If the Type we found has just been
				 * defined, we don't have to allocate
				 * a new one.
				 */
				$$ = $11;
			    } else {
				if (!($11))
				    smiPrintError(thisParserPtr, ERR_INTERNAL);
				/*
				 * Otherwise, we have to allocate a
				 * new Type struct, inherited from $10.
				 */
				$$ = duplicateType($11, 0, thisParserPtr);
			    }
			    setTypeDescription($$, $7, thisParserPtr);
			    if ($9) {
				setTypeReference($$, $9, thisParserPtr);
			    }
			    setTypeStatus($$, $5);
			    if ($3) {
				if (smiCheckFormat(thisParserPtr,
						   $$->export.basetype, $3,
						   thisParserPtr->firstStatementLine)) {
				    setTypeFormat($$, $3);
				}
			    }
			    setTypeDecl($$, SMI_DECL_TEXTUALCONVENTION);
			}
	|		choiceClause
			{
			    $$ = $1;
			    setTypeDecl($$, SMI_DECL_TYPEASSIGNMENT);
			}
	;

/* REF:RFC1902,7.1.12. */
conceptualTable:	SEQUENCE OF row
			{
			    if ($3) {
				$$ = addType(NULL,
					     SMI_BASETYPE_UNKNOWN, 0,
					     thisParserPtr);
				setTypeDecl($$, SMI_DECL_IMPL_SEQUENCEOF);
				setTypeParent($$, $3);
			    } else {
				$$ = NULL;
			    }
			}
	;

row:			UPPERCASE_IDENTIFIER
			/*
			 * In this case, we do NOT allow `Module.Type'.
			 * The identifier must be defined in the local
			 * module.
			 */
			{
			    Type *typePtr;
			    Import *importPtr;

			    $$ = findTypeByModulenameAndName(
				thisParserPtr->modulePtr->export.name, $1);
			    if (! $$) {
				importPtr = findImportByName($1,
							     thisModulePtr);
				if (!importPtr ||
				    (importPtr->kind == KIND_NOTFOUND)) {
				    /* 
				     * forward referenced type. create it,
				     * marked with FLAG_INCOMPLETE.
				     */
				    typePtr = addType($1,
						      SMI_BASETYPE_UNKNOWN,
						      FLAG_INCOMPLETE,
						      thisParserPtr);
				    $$ = typePtr;
				} else {
				    /*
				     * imported type.
				     * TODO: is this allowed in a SEQUENCE? 
				     */
				    importPtr->use++;
				    $$ = findTypeByModulenameAndName(
					importPtr->export.module,
					importPtr->export.name);
				    smiFree($1);
				}
			    } else {
				smiFree($1);
			    }
			}
		        /* TODO: this must be an entryType */
	;

/* REF:RFC1902,7.1.12. */
entryType:		SEQUENCE '{' sequenceItems '}'
			{
			    $$ = addType(NULL, SMI_BASETYPE_UNKNOWN, 0,
					 thisParserPtr);
			    setTypeList($$, $3);
			}
;

sequenceItems:		sequenceItem
			{
			    $$ = smiMalloc(sizeof(List));
			    $$->ptr = $1;
			    $$->nextPtr = NULL;
			}
	|		sequenceItems ',' sequenceItem
			/* TODO: might this list be emtpy? */
			{
			    List *p, *pp;
			    
			    p = smiMalloc(sizeof(List));
			    p->ptr = (void *)$3;
			    p->nextPtr = NULL;
			    for (pp = $1; pp->nextPtr; pp = pp->nextPtr);
			    pp->nextPtr = p;
			    $$ = $1;
			}
	;

/*
 * In a SEQUENCE { ... } there are no sub-types, enumerations or
 * named bits. REF: draft, p.29
 * NOTE: REF:RFC1902,7.1.12. was less clear, it said:
 * `normally omitting the sub-typing information'
 */
sequenceItem:		LOWERCASE_IDENTIFIER sequenceSyntax
			{
			    Object *objectPtr;
			    Import *importPtr;
			    Type *typePtr;
			    
			    objectPtr =
			        findObjectByModuleAndName(thisParserPtr->modulePtr,
							  $1);

			    if (!objectPtr) {
				importPtr = findImportByName($1,
							     thisModulePtr);
				if (!importPtr ||
				    (importPtr->kind == KIND_NOTFOUND)) {
				    objectPtr = addObject($1, thisParserPtr->pendingNodePtr,
					                  0,
					                  FLAG_INCOMPLETE |
							  FLAG_SEQTYPE,
						          thisParserPtr);
				    setObjectType(objectPtr, $2);
				} else {
				    /*
				     * imported object.
				     */
				    importPtr->use++;
				    objectPtr = findObjectByModulenameAndName(
					importPtr->export.module, $1);
				    smiFree($1);

				    if (objectPtr->typePtr->export.name) {
					typePtr = objectPtr->typePtr;
				    } else {
					typePtr = objectPtr->typePtr->parentPtr;
				    }
				    if (($2 != typePtr) &&
					(($2->export.basetype !=
					  SMI_BASETYPE_INTEGER32) ||
					 (typePtr->export.basetype !=
					  SMI_BASETYPE_ENUM)) &&
					(($2->export.basetype !=
					  SMI_BASETYPE_OCTETSTRING) ||
					 (typePtr->export.basetype !=
					  SMI_BASETYPE_BITS))) {
					smiPrintError(thisParserPtr,
						      ERR_SEQUENCE_TYPE_MISMATCH,
						      objectPtr->export.name);
				    }
				}
			    } else {
				smiFree($1);
				if (objectPtr->typePtr) {

				    if (objectPtr->typePtr->export.name) {
					typePtr = objectPtr->typePtr;
				    } else {
					typePtr = objectPtr->typePtr->parentPtr;
				    }
				    if (($2 != typePtr) &&
					(($2->export.basetype !=
					  SMI_BASETYPE_INTEGER32) ||
					 (typePtr->export.basetype !=
					  SMI_BASETYPE_ENUM)) &&
					(($2->export.basetype !=
					  SMI_BASETYPE_OCTETSTRING) ||
					 (typePtr->export.basetype !=
					  SMI_BASETYPE_BITS))) {
					smiPrintError(thisParserPtr,
						      ERR_SEQUENCE_TYPE_MISMATCH,
						      objectPtr->export.name);
				    }

				} else {
				    setObjectType(objectPtr, $2);
				    addObjectFlags(objectPtr,
						   FLAG_SEQTYPE);
				}
			    }

			    $$ = objectPtr;
			}
	;

Syntax:			ObjectSyntax
			{
			    $$ = $1;
			    if ($$)
				defaultBasetype = $$->export.basetype;
			}
	|		BITS '{' NamedBits '}'
			/* TODO: standalone `BITS' ok? seen in RMON2-MIB */
			/* -> no, it's only allowed in a SEQUENCE {...} */
			{
			    Type *typePtr;
			    List *p;
			    
			    defaultBasetype = SMI_BASETYPE_BITS;
			    typePtr = addType(NULL, SMI_BASETYPE_BITS,
					      FLAG_INCOMPLETE,
					      thisParserPtr);
			    setTypeDecl(typePtr, SMI_DECL_IMPLICIT_TYPE);
			    setTypeParent(typePtr, smiHandle->typeBitsPtr);
			    setTypeList(typePtr, $3);
			    for (p = $3; p; p = p->nextPtr)
				((NamedNumber *)p->ptr)->typePtr = typePtr;
			    smiCheckNamedNumbersOrder(parserPtr, typePtr);
			    $$ = typePtr;
			}
	;

sequenceSyntax:		/* ObjectSyntax */
			sequenceObjectSyntax
			{
			    $$ = $1;
			}
	|		BITS
			{
			    /* TODO: */
			    $$ = smiHandle->typeOctetStringPtr;
			}
	|		UPPERCASE_IDENTIFIER anySubType
			{
			    Type *typePtr;
			    Import *importPtr;
			    
			    $$ = findTypeByModulenameAndName(
				thisParserPtr->modulePtr->export.name, $1);
			    if (! $$) {
				importPtr = findImportByName($1,
							     thisModulePtr);
				if (!importPtr ||
				    (importPtr->kind == KIND_NOTFOUND)) {
				    /* 
				     * forward referenced type. create it,
				     * marked with FLAG_INCOMPLETE.
				     */
				    typePtr = addType($1, SMI_BASETYPE_UNKNOWN,
						      FLAG_INCOMPLETE,
						      thisParserPtr);
				    $$ = typePtr;
				} else {
				    importPtr->use++;
				    $$ = findTypeByModulenameAndName(
					importPtr->export.module,
					importPtr->export.name);
				    smiFree($1);
				}
			    } else {
				smiFree($1);
			    }
			}
	;

NamedBits:		NamedBit
			{
			    $$ = smiMalloc(sizeof(List));
			    $$->ptr = $1;
			    $$->nextPtr = NULL;
			}
	|		NamedBits ',' NamedBit
			{
			    List *p, *pp;
			    
			    p = smiMalloc(sizeof(List));
			    p->ptr = (void *)$3;
			    p->nextPtr = NULL;
			    for (pp = $1; pp->nextPtr; pp = pp->nextPtr);
			    pp->nextPtr = p;
			    $$ = $1;
			}
	;

NamedBit:		LOWERCASE_IDENTIFIER
			{
			    checkNameLen(thisParserPtr, $1,
					 ERR_BITNAME_32, ERR_BITNAME_64);
			    if (thisModulePtr->export.language == SMI_LANGUAGE_SMIV2)
			    {
				if (strchr($1, '-')) {
				    smiPrintError(thisParserPtr,
						  ERR_NAMEDBIT_INCLUDES_HYPHEN,
						  $1);
				}
			    }
			}
			'(' NUMBER ')'
			{
			    $$ = smiMalloc(sizeof(NamedNumber));
			    $$->export.name = $1;
			    $$->export.value.basetype =
				                       SMI_BASETYPE_UNSIGNED32;
			    $$->export.value.value.unsigned32 = $4;
			    /* RFC 2578 7.1.4 */
			    if ($4 >= 65535*8) {
				smiPrintError(thisParserPtr,
					      ERR_BITS_NUMBER_TOO_LARGE,
					      $1, $4);
			    } else {
				if ($4 >= 128) {
				    smiPrintError(thisParserPtr,
						  ERR_BITS_NUMBER_LARGE,
						  $1, $4);
				}
			    }
			}
	;

objectIdentityClause:	LOWERCASE_IDENTIFIER
			{
			    thisParserPtr->firstStatementLine = thisParserPtr->line;
			    thisParserPtr->currentDecl = SMI_DECL_OBJECTIDENTITY;

			    checkNameLen(thisParserPtr, $1,
					 ERR_OIDNAME_32, ERR_OIDNAME_64);
			    smiCheckObjectName(thisParserPtr,
					       thisModulePtr, $1);
			}
			OBJECT_IDENTITY
			{
			    Import *importPtr;

			    if (strcmp(thisModulePtr->export.name, "SNMPv2-SMI") &&
                                strcmp(thisModulePtr->export.name, "COPS-PR-SPPI")) {
				importPtr = findImportByName("OBJECT-IDENTITY",
							     thisModulePtr);
				if (importPtr) {
				    importPtr->use++;
				} else {
                                    if (thisParserPtr->modulePtr->export.language != SMI_LANGUAGE_SPPI)
				        smiPrintError(thisParserPtr,
						      ERR_MACRO_NOT_IMPORTED,
						      "OBJECT-IDENTITY",
						      "SNMPv2-SMI");
                                    else
				        smiPrintError(thisParserPtr,
						      ERR_MACRO_NOT_IMPORTED,
						      "OBJECT-IDENTITY",
						      "COPS-PR-SPPI");
				}
			    }
			}
			STATUS Status
			DESCRIPTION Text
			{
			    checkDescr(thisParserPtr, $8);
			}
			ReferPart
			COLON_COLON_EQUAL
			'{' objectIdentifier '}'
			{
			    Object *objectPtr;
			    
			    objectPtr = $13;
			    smiCheckObjectReuse(thisParserPtr, $1, &objectPtr);

			    objectPtr = setObjectName(objectPtr, $1, thisParserPtr);
			    setObjectDecl(objectPtr, SMI_DECL_OBJECTIDENTITY);
			    setObjectLine(objectPtr, thisParserPtr->firstStatementLine,
					  thisParserPtr);
			    setObjectStatus(objectPtr, $6);
			    setObjectDescription(objectPtr, $8, thisParserPtr);
			    if ($10) {
				setObjectReference(objectPtr, $10, thisParserPtr);
			    }
			    addObjectFlags(objectPtr, FLAG_REGISTERED);
			    deleteObjectFlags(objectPtr, FLAG_INCOMPLETE);
			    $$ = 0;
			}
	;

objectTypeClause:	LOWERCASE_IDENTIFIER
			{
			    thisParserPtr->firstStatementLine = thisParserPtr->line;
			    thisParserPtr->currentDecl = SMI_DECL_OBJECTTYPE;
			    
			    checkNameLen(thisParserPtr, $1,
					 ERR_OIDNAME_32, ERR_OIDNAME_64);
			    smiCheckObjectName(thisParserPtr,
					       thisModulePtr, $1);
			}
			OBJECT_TYPE
			{
			    Import *importPtr;
			    
			    importPtr = findImportByName("OBJECT-TYPE",
							 thisModulePtr);
			    if (importPtr) {
				importPtr->use++;
			    } else {
				if (thisModulePtr->export.language ==
				    SMI_LANGUAGE_SMIV2) {
				    smiPrintError(thisParserPtr,
						  ERR_MACRO_NOT_IMPORTED,
						  "OBJECT-TYPE", "SNMPv2-SMI");
				} else if (thisModulePtr->export.language ==
                                    SMI_LANGUAGE_SPPI) {
                                    smiPrintError(thisParserPtr,
                                                  ERR_MACRO_NOT_IMPORTED,
                                                  "OBJECT-TYPE", "COPS-PR-SPPI");
                                } else {
				    smiPrintError(thisParserPtr,
						  ERR_MACRO_NOT_IMPORTED,
						  "OBJECT-TYPE", "RFC-1212");
				}
			    }
                            indexFlag = 0;
			}
			SYNTAX Syntax                /* old $6, new $6 */
		        UnitsPart                    /* old $7, new $7 */
                        MaxOrPIBAccessPart           /* old $8, new $8 */
                        SPPIPibReferencesPart        /* SPPI only, $9 */
                        SPPIPibTagPart               /* SPPI only, $10 */
			STATUS Status                /* old $9 $10, new $11 $12 */
			descriptionClause            /* old $11, new $13 */
                        SPPIErrorsPart               /* SPPI only, $14 */
			ReferPart                    /* old $12, new $15 */
			IndexPart                    /* modified, old $13, new $16 */
                        MibIndex                     /* new, $17 */
                        SPPIUniquePart               /* SPPI only, $18 */
			DefValPart                   /* old $14, new $19 */
			COLON_COLON_EQUAL '{' ObjectName '}' /* old $17, new $22 */
			{
			    Object *objectPtr, *parentPtr;
			    Type *typePtr = NULL;
			    
			    objectPtr = $22;

			    smiCheckObjectReuse(thisParserPtr, $1, &objectPtr);

			    objectPtr = setObjectName(objectPtr, $1, thisParserPtr);
			    setObjectDecl(objectPtr, SMI_DECL_OBJECTTYPE);
			    setObjectLine(objectPtr, thisParserPtr->firstStatementLine,
					  thisParserPtr);
			    if (checkObjectFlags(objectPtr, FLAG_SEQTYPE)) {
				deleteObjectFlags(objectPtr, FLAG_SEQTYPE);
				if ($6) {
				    if ($6->export.name) {
					typePtr = $6;
                                        /*
                                         * According to RFC 3159 7.1.3. Opaque must not be used
                                         * in a SYNTAX clause.
                                         */
                                        if ((thisModulePtr->export.language == SMI_LANGUAGE_SPPI) &&
                                            !strcmp(typePtr->export.name, "Opaque"))
                                            smiPrintError(thisParserPtr, ERR_OPAQUE_IN_SYNTAX);
                                        /*
                                         * According to RFC 3159 7.1.4. IpAddress must not be used
                                         * in a SYNTAX clause.
                                         */
                                        if ((thisModulePtr->export.language == SMI_LANGUAGE_SPPI) &&
                                            !strcmp(typePtr->export.name, "IpAddress"))
                                            smiPrintError(thisParserPtr, ERR_IPADDRESS_IN_SYNTAX);
				    } else {
					typePtr = $6->parentPtr;
				    }
				    if ((objectPtr->typePtr != typePtr) &&
					((objectPtr->typePtr->export.basetype !=
					  SMI_BASETYPE_INTEGER32) ||
					 (typePtr->export.basetype !=
					  SMI_BASETYPE_ENUM)) &&
					((objectPtr->typePtr->export.basetype !=
					  SMI_BASETYPE_OCTETSTRING) ||
					 (typePtr->export.basetype !=
					  SMI_BASETYPE_BITS))) {
					smiPrintError(thisParserPtr,
						      ERR_SEQUENCE_TYPE_MISMATCH,
						      objectPtr->export.name);
				    }
				}
			    }
			    setObjectType(objectPtr, $6);
			    if (!($6->export.name)) {
				/*
				 * An inlined type.
				 */
#if 0 /* export implicitly defined types by the node's lowercase name */
				setTypeName($6, $1);
#endif
			    }
			    setObjectUnits(objectPtr, $7);
			    setObjectAccess(objectPtr, $8);
			    if (thisParserPtr->flags & FLAG_CREATABLE) {
				thisParserPtr->flags &= ~FLAG_CREATABLE;
				parentPtr =
				  objectPtr->nodePtr->parentPtr->lastObjectPtr;
				if (parentPtr &&
				    parentPtr->export.indexkind !=
				       SMI_INDEX_UNKNOWN) {
				    /*
				     * add objectPtr to the parent object's
				     * listPtr, which is the list of columns
				     * needed for row creation.
				     *
				     * Note, that this would clash, if the
				     * parent row object-type is not yet
				     * defined.
				     */
				    /*
				    newlistPtr = smiMalloc(sizeof(List));
				    newlistPtr->nextPtr = NULL;
				    newlistPtr->ptr = objectPtr;
				    */
				    /*
				     * Look up the parent object-type.
				     */
				    /*
				    if (parentPtr->listPtr) {
					for(listPtr = parentPtr->listPtr;
					    listPtr->nextPtr;
					    listPtr = listPtr->nextPtr);
					listPtr->nextPtr = newlistPtr;
				    } else {
					parentPtr->listPtr = newlistPtr;
				    }
				    */
				    addObjectFlags(parentPtr, FLAG_CREATABLE);
				    setObjectCreate(parentPtr, 1);
				} else {
				    smiPrintError(thisParserPtr,
						  ERR_SCALAR_READCREATE);
				}
			    }
			    setObjectStatus(objectPtr, $12);
			    addObjectFlags(objectPtr, FLAG_REGISTERED);
			    deleteObjectFlags(objectPtr, FLAG_INCOMPLETE);
			    if ($13) {
				setObjectDescription(objectPtr, $13, thisParserPtr);
			    }
			    if ($15) {
				setObjectReference(objectPtr, $15, thisParserPtr);
			    }
                            if (thisParserPtr->modulePtr->export.language != SMI_LANGUAGE_SPPI) {
                                /*
                                 * For SMI documents either $16 (IndexPart) or $17 (MibIndex)
                                 * are used, but not both. This is signalled via the indexFlag
                                 * which is 1 if IndexPart has been used.
                                 */
                                if (indexFlag == INDEXFLAG_AUGMENTS) { /* IndexPart was used */
			            if ($16.indexkind != SMI_INDEX_UNKNOWN) {
				        setObjectList(objectPtr, $16.listPtr);
				        setObjectImplied(objectPtr, $16.implied);
				        setObjectIndexkind(objectPtr, $16.indexkind);
				        setObjectRelated(objectPtr, $16.rowPtr);
			            }
                                } else {
			            if ($17.indexkind != SMI_INDEX_UNKNOWN) {
				        setObjectList(objectPtr, $17.listPtr);
				        setObjectImplied(objectPtr, $17.implied);
				        setObjectIndexkind(objectPtr, $17.indexkind);
				        setObjectRelated(objectPtr, $17.rowPtr);
			            }
                                }
                            } else {
                                /*
                                 * PIBs contain either PIB-INDEX or AUGMENTS or EXTENDS - 
                                 * but only with one Index entry. A PIB-INDEX may be
                                 * followed by a full INDEX. We get the indexkind
                                 * from the first.
                                 * Note that PIB-INDEX/AUGMENTS/EXTENS is always
                                 * the first element in objectPtr->listPtr.
                                 * If an optional INDEX exists then it is
                                 * appended to this list.
                                 */
			        if ($16.indexkind != SMI_INDEX_UNKNOWN) {
                                    setObjectList(objectPtr, $16.listPtr);
				    setObjectIndexkind(objectPtr, $16.indexkind);
				    setObjectRelated(objectPtr, $16.rowPtr);
			        }
			        if ($17.indexkind != SMI_INDEX_UNKNOWN) {
                                    if (objectPtr->listPtr) {
                                        List *p;
                                        for (p = objectPtr->listPtr; p->nextPtr;
                                             p = p->nextPtr);
                                        p->nextPtr = $17.listPtr;
                                    }
				    setObjectImplied(objectPtr, $17.implied);
			        }
                            }
                            if ($18) {
                                setObjectUniqueness(objectPtr, $18);
                            }
			    if ($19) {
				if (objectPtr->typePtr
				    && (((objectPtr->typePtr->export.basetype == SMI_BASETYPE_OCTETSTRING) &&
					 ($19->basetype != SMI_BASETYPE_OCTETSTRING))
					|| ((objectPtr->typePtr->export.basetype == SMI_BASETYPE_OBJECTIDENTIFIER) &&
					    ($19->basetype != SMI_BASETYPE_OBJECTIDENTIFIER)))) {
				    smiPrintError(thisParserPtr,
						  ERR_DEFVAL_SYNTAX);
				    if ($19->basetype == SMI_BASETYPE_OBJECTIDENTIFIER) {
					smiFree($19->value.oid);
				    }
				    if (($19->basetype == SMI_BASETYPE_BITS) ||
					($19->basetype == SMI_BASETYPE_OCTETSTRING)) {
					smiFree($19->value.ptr);
				    }
				    smiFree($19);
				} else {
				    setObjectValue(objectPtr, $19);
				}
			    }
                            if ($9) {
                                if (objectPtr->relatedPtr)
                                    smiPrintError(thisParserPtr, ERR_OBJECTPTR_ELEMENT_IN_USE,
                                                  "relatedPtr", "PIB-REFERENCES");
                                /*
                                 * PIB-REFERENCES clauses are only allowed for
                                 * objects with a SYNTAX of 'ReferenceId'.
                                 * See RFC 3159 7.10
                                 */
                                if (objectPtr->typePtr && objectPtr->typePtr->export.name &&
                                    strcmp(objectPtr->typePtr->export.name, "ReferenceId"))
                                    smiPrintErrorAtLine(parserPtr, ERR_PIB_REFERENCES_WRONG_TYPE,
                                                        objectPtr->line);
                                else
                                    setObjectRelated(objectPtr, $9);
                            } else {
                               /*
                                * Does this object have a SYNTAX of 'ReferenceId'
                                * and a PIB-REFERENCES clause?
                                * See RFC 3159 7.10
                                */
                               if ((thisModulePtr->export.language == SMI_LANGUAGE_SPPI) &&
                                   objectPtr->typePtr && objectPtr->typePtr->export.name &&
                                   !strcmp(objectPtr->typePtr->export.name, "ReferenceId"))
                                   smiPrintErrorAtLine(parserPtr, ERR_LACKING_PIB_REFERENCES,
                                                       objectPtr->line);
                            }
                            if ($10) {
                                if (objectPtr->relatedPtr)
                                    smiPrintError(thisParserPtr, ERR_OBJECTPTR_ELEMENT_IN_USE,
                                                  "relatedPtr", "PIB-TAG");
                                /*
                                 * PIB-TAG clauses are only allowed for
                                 * objects with a SYNTAX of 'TagReferenceId'.
                                 * See RFC 3159 7.11
                                 */
                                if (objectPtr->typePtr && objectPtr->typePtr->export.name &&
                                    strcmp(objectPtr->typePtr->export.name, "TagReferenceId"))
                                    smiPrintErrorAtLine(parserPtr, ERR_PIB_TAG_WRONG_TYPE,
                                                        objectPtr->line);
                                else
                                    setObjectRelated(objectPtr, $10);
                            } else {
                                /*
                                 * Does this object have a SYNTAX of 'TagReferenceId'
                                 * and a PIB-TAG clause?
                                 * See RFC 3159 7.11
                                 */
                                if ((thisModulePtr->export.language == SMI_LANGUAGE_SPPI) &&
                                    objectPtr->typePtr && objectPtr->typePtr->export.name &&
                                    !strcmp(objectPtr->typePtr->export.name, "TagReferenceId"))
                                    smiPrintErrorAtLine(parserPtr, ERR_LACKING_PIB_TAG,
                                                        objectPtr->line);

                            }
                            if ($14) {
                                if (objectPtr->listPtr)
                                    smiPrintError(thisParserPtr, ERR_OBJECTPTR_ELEMENT_IN_USE,
                                                  "listPtr", "INSTALL-ERRORS");
                                /*
                                 * Are INSTALL-ERRORS only used with tables?
                                 * See RFC 3159 7.4
                                 */
                                if (!((objectPtr->export.decl == SMI_DECL_OBJECTTYPE) &&
                                    (objectPtr->typePtr) &&
                                    (objectPtr->typePtr->export.decl == SMI_DECL_IMPL_SEQUENCEOF)))
                                    smiPrintErrorAtLine(parserPtr, ERR_INSTALL_ERRORS_FOR_NON_TABLE,
                                                        objectPtr->line);
                                else
                                    setObjectList(objectPtr, $14);
                            }
			    $$ = 0;
			}
	;

descriptionClause:	/* empty */
			{
			    if ((thisModulePtr->export.language == SMI_LANGUAGE_SMIV2) ||
                                (thisModulePtr->export.language == SMI_LANGUAGE_SPPI))
			    {
				smiPrintError(thisParserPtr,
					      ERR_MISSING_DESCRIPTION);
			    }
			    $$ = NULL;
			}
	|		DESCRIPTION Text
			{
			    $$ = $2;
			    checkDescr(thisParserPtr, $2);
			}
	;

trapTypeClause:		fuzzy_lowercase_identifier
			{
			    thisParserPtr->firstStatementLine = thisParserPtr->line;
			    thisParserPtr->currentDecl = SMI_DECL_TRAPTYPE;
			    
			    checkNameLen(thisParserPtr, $1,
					 ERR_OIDNAME_32, ERR_OIDNAME_64);
			    smiCheckObjectName(thisParserPtr,
					       thisModulePtr, $1);
			}
			TRAP_TYPE
			{
			    Import *importPtr;
                            if (thisParserPtr->modulePtr->export.language == SMI_LANGUAGE_SPPI)
                                smiPrintError(thisParserPtr, ERR_SMI_CONSTRUCT_IN_PIB, "TRAP-TYPE");
			    
			    if (thisModulePtr->export.language == SMI_LANGUAGE_SMIV2)
			    {
			        smiPrintError(thisParserPtr, ERR_TRAP_TYPE);
			    }

			    importPtr = findImportByName("TRAP-TYPE",
							 thisModulePtr);
			    if (importPtr) {
				importPtr->use++;
			    } else {
				smiPrintError(thisParserPtr,
					      ERR_MACRO_NOT_IMPORTED,
					      "TRAP-TYPE", "RFC-1215");
			    }
			}
			ENTERPRISE objectIdentifier
			VarPart
			DescrPart
			ReferPart
			COLON_COLON_EQUAL NUMBER
			/* TODO: range of number? */
			{
			    Object *objectPtr;
			    Node *nodePtr;
			    
			    objectPtr = $6;
			    nodePtr = findNodeByParentAndSubid(
				objectPtr->nodePtr, 0);
			    if (nodePtr && nodePtr->lastObjectPtr &&
	       		        (nodePtr->lastObjectPtr->modulePtr == thisModulePtr)) {
				/*
				 * hopefully, the last defined Object for
				 * this Node is the one we expect.
				 */
				objectPtr = nodePtr->lastObjectPtr;
			    } else {
				objectPtr = addObject(NULL,
						      objectPtr->nodePtr,
						      0,
						      FLAG_INCOMPLETE,
						      thisParserPtr);
			    }
			    objectPtr = addObject(NULL,
						  objectPtr->nodePtr,
						  $11,
						  FLAG_INCOMPLETE,
						  thisParserPtr);
			    
			    smiCheckObjectReuse(thisParserPtr, $1, &objectPtr);

			    objectPtr = setObjectName(objectPtr, $1, thisParserPtr);
			    setObjectDecl(objectPtr,
					  SMI_DECL_TRAPTYPE);
			    setObjectLine(objectPtr, thisParserPtr->firstStatementLine,
					  thisParserPtr);
			    addObjectFlags(objectPtr, FLAG_REGISTERED);
			    deleteObjectFlags(objectPtr, FLAG_INCOMPLETE);
			    setObjectList(objectPtr, $7);
			    setObjectStatus(objectPtr, SMI_STATUS_CURRENT); 
			    setObjectDescription(objectPtr, $8, thisParserPtr);
			    if ($9) {
				setObjectReference(objectPtr, $9, thisParserPtr);
			    }
			    $$ = 0;
			}
	;

VarPart:		VARIABLES '{' VarTypes '}'
			{
			    $$ = $3;
			}
	|		/* empty */
			{
			    $$ = NULL;
			}
	;

VarTypes:		VarType
			{
			    if ($1) {
				$$ = smiMalloc(sizeof(List));
				$$->ptr = $1;
				$$->nextPtr = NULL;
			    } else {
				$$ = NULL;
			    }
			}
	|		VarTypes ',' VarType
			{
			    List *p, *pp;

			    if ($3) {
				p = smiMalloc(sizeof(List));
				p->ptr = $3;
				p->nextPtr = NULL;
				if ($1) {
				    for (pp = $1; pp->nextPtr;
					 pp = pp->nextPtr);
				    pp->nextPtr = p;
				    $$ = $1;
				} else {
				    $$ = p;
				}
			    } else {
				$$ = $1;
			    }
			}
	;

VarType:		ObjectName
			{
			    $$ = $1;
			}
	;

DescrPart:		DESCRIPTION Text
			{
			    $$ = $2;
			    checkDescr(thisParserPtr, $2);
			}
	|		/* empty */
			{ $$ = NULL; }
	;

MaxOrPIBAccessPart:     MaxAccessPart
                        {
                            $$ = $1;
                        }
        |               PibAccessPart
                        {
                            if (thisParserPtr->modulePtr->export.language != SMI_LANGUAGE_SPPI)
                                smiPrintError(thisParserPtr, ERR_SPPI_CONSTRUCT_IN_MIB, "MAX-ACCESS");
                            if ($1 == SMI_ACCESS_NOT_ACCESSIBLE)
                                smiPrintError(thisParserPtr, ERR_NOT_ACCESSIBLE_IN_PIB_ACCESS);
                            $$ = $1;
                        }
        |               /* empty */
                        { $$ = 0;  }
        ;

PibAccessPart:          PibAccess Access
                        { $$ = $2; }
        ;

PibAccess:              POLICY_ACCESS
                        { 
                            smiPrintError(thisParserPtr, ERR_POLICY_ACCESS_IN_PIB);
                        }
        |               PIB_ACCESS
                        { }
        ;        

SPPIPibReferencesPart:  PIB_REFERENCES
                        {
                            if (thisParserPtr->modulePtr->export.language != SMI_LANGUAGE_SPPI)
                                smiPrintError(thisParserPtr, ERR_SPPI_CONSTRUCT_IN_MIB, "PIB-REFERENCES");
                        }
                        '{' Entry '}'
                        { $$ = $4; }
        |               /* empty */
                        { $$ = 0; }
        ;

SPPIPibTagPart:         PIB_TAG
                        {
                            if (thisParserPtr->modulePtr->export.language != SMI_LANGUAGE_SPPI)
                                smiPrintError(thisParserPtr, ERR_SPPI_CONSTRUCT_IN_MIB, "PIB-TAG");
                        }
                        '{' ObjectName '}'
                        { $$ = $4; }
        |               /* empty */
                        { $$ = 0; }
        ;


SPPIUniquePart:         UNIQUENESS
                        {
                            if (thisParserPtr->modulePtr->export.language != SMI_LANGUAGE_SPPI)
                                smiPrintError(thisParserPtr, ERR_SPPI_CONSTRUCT_IN_MIB, "UNIQUENESS");
                        }
                        '{' UniqueTypesPart '}'
                        { $$ = $4; }
        |               /* empty */
                        { $$ = NULL; }
        ;

UniqueTypesPart:        UniqueTypes
                        { $$ = $1; }
        |               /* empty */
                        { $$ = NULL; }
        ;

UniqueTypes:            UniqueType
                        {
			    $$ = smiMalloc(sizeof(List));
			    $$->ptr = $1;
			    $$->nextPtr = NULL;
			}
        |               UniqueTypes ',' UniqueType
			/* TODO: might this list be emtpy? */
			{
			    List *p, *pp;
			    
			    p = smiMalloc(sizeof(List));
			    p->ptr = $3;
			    p->nextPtr = NULL;
			    for (pp = $1; pp->nextPtr; pp = pp->nextPtr);
			    pp->nextPtr = p;
			    $$ = $1;
                        }
        ;

UniqueType:             ObjectName
                        { $$ = $1; }
        ;

SPPIErrorsPart:         INSTALL_ERRORS
                        {
                            if (thisParserPtr->modulePtr->export.language != SMI_LANGUAGE_SPPI)
                                smiPrintError(thisParserPtr, ERR_SPPI_CONSTRUCT_IN_MIB, "INSTALL-ERRORS");
                        }
                        '{' Errors '}'
                        { $$ = $4; }
        |               /* empty */
                        { $$ = NULL; }
        ;

Errors:                 Error
                        {
			    $$ = smiMalloc(sizeof(List));
			    $$->ptr = $1;
			    $$->nextPtr = NULL;
			}
        |               Errors ',' Error
			/* TODO: might this list be emtpy? */
			{
			    List *p, *pp;
			    
			    p = smiMalloc(sizeof(List));
			    p->ptr = $3;
			    p->nextPtr = NULL;
			    for (pp = $1; pp->nextPtr; pp = pp->nextPtr);
			    pp->nextPtr = p;
			    $$ = $1;
                        }
        ;

Error:                  LOWERCASE_IDENTIFIER '(' NUMBER ')'
			{
			    Object *objectPtr;
			    
                            if (($3 < 1) || ($3 > 65536))
                                smiPrintError(thisParserPtr, ERR_ERROR_NUMBER_RANGE, $3);
                            /*
                             * This is not a regular object that will be added vid
                             * 'addObject' as error identifier have no other
                             * meaning in PIBs. They are just used for
                             * a direct mapping to the actual protocol fields.
                             */
                            objectPtr = smiMalloc(sizeof(Object));
                            objectPtr->export.name = $1;
                            objectPtr->export.oidlen = 1;
                            objectPtr->export.oid = (void *)$3;
			    $$ = objectPtr;
			}
        ;


MaxAccessPart:		MAX_ACCESS
			{
			    if (thisModulePtr->export.language == SMI_LANGUAGE_SMIV1)
			    {
			        smiPrintError(thisParserPtr,
					      ERR_MAX_ACCESS_IN_SMIV1);
			    } else if (thisParserPtr->modulePtr->export.language == SMI_LANGUAGE_SPPI)
                                smiPrintError(thisParserPtr, ERR_SMI_CONSTRUCT_IN_PIB, "MAX-ACCESS");
			}
			Access
			{ $$ = $3; }
	|		ACCESS
			{
			    if (thisModulePtr->export.language == SMI_LANGUAGE_SMIV2)
			    {
			        smiPrintError(thisParserPtr, ERR_ACCESS_IN_SMIV2);
			    } else if (thisParserPtr->modulePtr->export.language == SMI_LANGUAGE_SPPI)
                                smiPrintError(thisParserPtr, ERR_SMI_CONSTRUCT_IN_PIB, "ACCESS");
			}
			Access
			/* TODO: limited values in v1 */
			{ $$ = $3; }
	;

notificationTypeClause:	LOWERCASE_IDENTIFIER
			{
			    thisParserPtr->firstStatementLine = thisParserPtr->line;
			    thisParserPtr->currentDecl = SMI_DECL_NOTIFICATIONTYPE;

			    checkNameLen(thisParserPtr, $1,
					 ERR_OIDNAME_32, ERR_OIDNAME_64);
			    smiCheckObjectName(thisParserPtr,
					       thisModulePtr, $1);
			}
			NOTIFICATION_TYPE
			{
			    Import *importPtr;

                            if (thisParserPtr->modulePtr->export.language == SMI_LANGUAGE_SPPI)
                                smiPrintError(thisParserPtr, ERR_SMI_CONSTRUCT_IN_PIB, "NOTIFICATION-TYPE");
                                
			    importPtr = findImportByName("NOTIFICATION-TYPE",
							 thisModulePtr);
			    if (importPtr) {
				importPtr->use++;
			    } else {
				smiPrintError(thisParserPtr,
					      ERR_MACRO_NOT_IMPORTED,
					      "NOTIFICATION-TYPE",
					      "SNMPv2-SMI");
			    }
			}
			NotificationObjectsPart
			STATUS Status
			DESCRIPTION Text
			{
			    checkDescr(thisParserPtr, $9);
			}
			ReferPart
			COLON_COLON_EQUAL
			'{' NotificationName '}'
			{
			    Object *objectPtr;
			    
			    objectPtr = $14;

			    smiCheckObjectReuse(thisParserPtr, $1, &objectPtr);

			    objectPtr = setObjectName(objectPtr, $1, thisParserPtr);
			    setObjectDecl(objectPtr,
					  SMI_DECL_NOTIFICATIONTYPE);
			    setObjectLine(objectPtr, thisParserPtr->firstStatementLine,
					  thisParserPtr);
			    addObjectFlags(objectPtr, FLAG_REGISTERED);
			    deleteObjectFlags(objectPtr, FLAG_INCOMPLETE);
			    setObjectList(objectPtr, $5);
			    setObjectStatus(objectPtr, $7);
			    setObjectDescription(objectPtr, $9, thisParserPtr);
			    if ($11) {
				setObjectReference(objectPtr, $11, thisParserPtr);
			    }
			    $$ = 0;
			}
	;

moduleIdentityClause:	LOWERCASE_IDENTIFIER
			{
			    thisParserPtr->firstStatementLine = thisParserPtr->line;
			    thisParserPtr->currentDecl = SMI_DECL_MODULEIDENTITY;
			    
			    checkNameLen(thisParserPtr, $1,
					 ERR_OIDNAME_32, ERR_OIDNAME_64);
			    smiCheckObjectName(thisParserPtr,
					       thisModulePtr, $1);
			}
			MODULE_IDENTITY
			{
			    Import *importPtr;
			    
			    importPtr = findImportByName("MODULE-IDENTITY",
							 thisModulePtr);
			    if (importPtr) {
				importPtr->use++;
			    } else {
				smiPrintError(thisParserPtr,
					      ERR_MACRO_NOT_IMPORTED,
					      "MODULE-IDENTITY",
					      "SNMPv2-SMI");
			    }
			    
			    if (thisParserPtr->modulePtr->numModuleIdentities > 0)
			    {
			        smiPrintError(thisParserPtr,
					      ERR_TOO_MANY_MODULE_IDENTITIES);
			    }
			    if (thisParserPtr->modulePtr->numStatements > 0) {
			        smiPrintError(thisParserPtr,
					      ERR_MODULE_IDENTITY_NOT_FIRST);
			    }
			}
                        SubjectCategoriesPart        /* SPPI only */
                        {
                          /* do nothing at the moment */
                        }
			LAST_UPDATED ExtUTCTime
			{
			    setModuleLastUpdated(thisParserPtr->modulePtr, $8);
			}
			ORGANIZATION Text
			{
			    if ($11 && !strlen($11)) {
				smiPrintError(thisParserPtr,
					      ERR_EMPTY_ORGANIZATION);
			    }
			}
			CONTACT_INFO Text
			{
			    if ($14 && !strlen($14)) {
				smiPrintError(thisParserPtr,
					      ERR_EMPTY_CONTACT);
			    }
			}
			DESCRIPTION Text
			{
			    checkDescr(thisParserPtr, $17);
			}
			RevisionPart
                        {
			    if ((!thisModulePtr->firstRevisionPtr) ||
				(thisModulePtr->firstRevisionPtr->export.date !=
				 thisModulePtr->lastUpdated)) {
				smiPrintError(thisParserPtr,
					      ERR_REVISION_MISSING);
				addRevision(thisModulePtr->lastUpdated,
					    smiStrdup(
						"[Revision added by libsmi due to a LAST-UPDATED clause.]"),
					    thisParserPtr);
			    }
			}
			COLON_COLON_EQUAL
			'{' objectIdentifier '}'
			{
			    Object *objectPtr;
			    
			    objectPtr = $23;
			    smiCheckObjectReuse(thisParserPtr, $1, &objectPtr);

			    thisParserPtr->modulePtr->numModuleIdentities++;

			    objectPtr = setObjectName(objectPtr, $1, thisParserPtr);
			    setObjectDecl(objectPtr, SMI_DECL_MODULEIDENTITY);
			    setObjectLine(objectPtr, thisParserPtr->firstStatementLine,
					  thisParserPtr);
			    setObjectStatus(objectPtr, SMI_STATUS_CURRENT);
			    addObjectFlags(objectPtr, FLAG_REGISTERED);
			    deleteObjectFlags(objectPtr, FLAG_INCOMPLETE);
			    setModuleIdentityObject(thisParserPtr->modulePtr,
						    objectPtr);
			    setModuleOrganization(thisParserPtr->modulePtr,
						  $11);
			    setModuleContactInfo(thisParserPtr->modulePtr,
						 $14);
			    setModuleDescription(thisParserPtr->modulePtr,
						 $17, thisParserPtr);
                            if ($5 != NULL) {
                                setObjectList(objectPtr, $5->categories);
                                smiFree($5);
                            }
			    /* setObjectDescription(objectPtr, $15); */
			    $$ = 0;
			}
        ;

SubjectCategoriesPart:  SUBJECT_CATEGORIES '{' SubjectCategories '}'
                        {
                            if (thisParserPtr->modulePtr->export.language != SMI_LANGUAGE_SPPI)
                                smiPrintError(thisParserPtr, ERR_SPPI_CONSTRUCT_IN_MIB, "SUBJECT-CATEGORIES");
                            $$ = $3;
                        }
        |               /* empty */
                        {
                            if (thisParserPtr->modulePtr->export.language == SMI_LANGUAGE_SPPI)
                                smiPrintError(thisParserPtr, ERR_SUBJECT_CATEGORIES_MISSING);
                            $$ = NULL;
                        }
        ;
                        
SubjectCategories:      CategoryIDs
                        {
                            $$ = smiMalloc(sizeof(SubjectCategories));
                            $$->categories    = $1;
                        }
        ;

CategoryIDs:            CategoryID
			{
			    $$ = smiMalloc(sizeof(List));
			    $$->ptr = $1;
			    $$->nextPtr = NULL;
			}
        |               CategoryIDs ',' CategoryID
			{
			    List *p, *pp;
			    
                            if ($1->ptr == NULL)
                                smiPrintError(thisParserPtr, ERR_SUBJECT_CATEGORIES_ALL);
			    p = smiMalloc(sizeof(List));
			    p->ptr = (void *)$3;
			    p->nextPtr = NULL;
			    for (pp = $1; pp->nextPtr; pp = pp->nextPtr);
			    pp->nextPtr = p;
			    $$ = $1;
			}
        ;
        
CategoryID:		LOWERCASE_IDENTIFIER
                        {
                            Object *objectPtr;
                            
                            if (strcmp($1, "all"))
                                smiPrintError(thisParserPtr, ERR_SUBJECT_CATEGORIES_MISSING_SUBID);
                            else {
                                /*
                                 * This is not a regular object that will be added via
                                 * 'addObject' as subject category dentifier have no
                                 * other meaning in PIBs. They are just used for
                                 * a direct mapping to the actual protocol fields.
                                 */
                                objectPtr = smiMalloc(sizeof(Object));
                                objectPtr->export.name = "all";
                                objectPtr->export.oidlen = 0;
                                objectPtr->export.oid = 0;
                                $$ = objectPtr;
                            }
                            smiFree($1);
                        }
        |               LOWERCASE_IDENTIFIER '(' NUMBER ')'
			{
			    Object *objectPtr;
			    
                            if (!strcmp($1, "all")) {
                                smiPrintError(thisParserPtr, ERR_SUBJECT_CATEGORIES_ALL_WITH_SUBID);
                                $$ = NULL;
                            } else {
                                /*
                                 * This is not a regular object that will be added via
                                 * 'addObject' as subject category dentifier have no
                                 * other meaning in PIBs. They are just used for
                                 * a direct mapping to the actual protocol fields.
                                 */
                                objectPtr = smiMalloc(sizeof(Object));
                                objectPtr->export.name = $1;
                                objectPtr->export.oidlen = 1;
                                objectPtr->export.oid = (void *)$3;
                                $$ = objectPtr;
                            }
			}
        ;

ObjectSyntax:		SimpleSyntax
			{
			    $$ = $1;
			}
	|		typeTag SimpleSyntax
			{
			    if (strcmp(thisParserPtr->modulePtr->export.name,
				       "SNMPv2-SMI") &&
			        strcmp(thisParserPtr->modulePtr->export.name,
				       "SNMPv2-TC") &&
				strcmp(thisParserPtr->modulePtr->export.name,
				       "SNMPv2-CONF") &&
				strcmp(thisParserPtr->modulePtr->export.name,
				       "RFC-1212") &&
				strcmp(thisParserPtr->modulePtr->export.name,
				       "RFC1065-SMI") &&
				strcmp(thisParserPtr->modulePtr->export.name,
				       "RFC1155-SMI") &&
                                strcmp(thisParserPtr->modulePtr->export.name,
                                       "COPS-PR-SPPI")) {
			        smiPrintError(thisParserPtr, ERR_TYPE_TAG, $1);
			    }
			    $$ = $2;
			}
	|		conceptualTable	     /* TODO: possible? row? entry? */
			{
			    /* TODO */
			    $$ = $1;
			}
	|		row		     /* the uppercase name of a row  */
			{
			    /* TODO */
			    $$ = $1;
			}
	|		entryType	     /* SEQUENCE { ... } phrase */
			{
			    /* TODO */
			    $$ = $1;
			}
	|		ApplicationSyntax
			{
			    Import *importPtr;

			    if ($1 && $1->export.name) {
				importPtr = findImportByName($1->export.name,
							     thisModulePtr);
				if (importPtr) {
				    importPtr->use++;
				}
			    }

			    /* TODO */
			    $$ = $1;
			}
        ;

typeTag:		'[' APPLICATION NUMBER ']' IMPLICIT
			{ $$ = 0; /* TODO: check range */ }
	|		'[' UNIVERSAL NUMBER ']' IMPLICIT
			{ $$ = 0; /* TODO: check range */ }
	;

/*
 * In a SEQUENCE { ... } there are no sub-types, enumerations or
 * named bits. REF: draft, p.29
 */
sequenceObjectSyntax:	sequenceSimpleSyntax
			{ $$ = $1; }
/*	|		conceptualTable	     /* TODO: possible? row? entry? */
/*	|		row		     /* the uppercase name of a row  */
/*	|		entryType	     /* it's SEQUENCE { ... } phrase */
	|		sequenceApplicationSyntax
			{
			    Import *importPtr;

			    if ($1 && $1->export.name) {
				importPtr = findImportByName($1->export.name,
							     thisModulePtr);
				if (importPtr) {
				    importPtr->use++;
				}
			    }

			    /* TODO */
			    $$ = $1;
			}
        ;

valueofObjectSyntax:	valueofSimpleSyntax
			{ $$ = $1; }
			/* conceptualTables and rows do not have DEFVALs
			 */
			/* valueofApplicationSyntax would not introduce any
			 * further syntax of ObjectSyntax values.
			 */
	;

SimpleSyntax:		INTEGER			/* (-2147483648..2147483647) */
			{
			    if ((thisModulePtr->export.language == SMI_LANGUAGE_SMIV2)
				&&
				(strcmp(thisModulePtr->export.name, "SNMPv2-SMI") &&
				 strcmp(thisModulePtr->export.name, "SNMPv2-TC") &&
                                 strcmp(thisModulePtr->export.name, "COPS-PR-SPPI")))
				smiPrintError(thisParserPtr,
					      ERR_INTEGER_IN_SMIV2);

			    defaultBasetype = SMI_BASETYPE_INTEGER32;
			    $$ = smiHandle->typeInteger32Ptr;
			}
	|		INTEGER
			{
			    defaultBasetype = SMI_BASETYPE_INTEGER32;
			}
			integerSubType
			{
			    if ((thisModulePtr->export.language == SMI_LANGUAGE_SMIV2)
				&&
				(strcmp(thisModulePtr->export.name, "SNMPv2-SMI") &&
				 strcmp(thisModulePtr->export.name, "SNMPv2-TC") &&
                                 strcmp(thisModulePtr->export.name, "COPS-PR-SPPI")))
				smiPrintError(thisParserPtr,
					      ERR_INTEGER_IN_SMIV2);

			    $$ = duplicateType(smiHandle->typeInteger32Ptr, 0,
					       thisParserPtr);
			    setTypeList($$, $3);
			    smiCheckTypeRanges(thisParserPtr, $$);
			}
	|		INTEGER
			{
			    defaultBasetype = SMI_BASETYPE_ENUM;
			}
			enumSpec
			{
			    List *p;
			    
			    $$ = duplicateType(smiHandle->typeEnumPtr, 0,
					       thisParserPtr);
			    setTypeList($$, $3);
			    for (p = $3; p; p = p->nextPtr)
				((NamedNumber *)p->ptr)->typePtr = $$;
			    smiCheckNamedNumbersOrder(parserPtr, $$);
			}
	|		INTEGER32		/* (-2147483648..2147483647) */
			{
			    Import *importPtr;

			    defaultBasetype = SMI_BASETYPE_INTEGER32;
			    importPtr = findImportByName("Integer32",
							 thisModulePtr);
			    if (importPtr) {
				importPtr->use++;
			    } else {
				if (thisModulePtr->export.language ==
				    SMI_LANGUAGE_SMIV2) {
				    smiPrintError(thisParserPtr,
						  ERR_SMIV2_BASETYPE_NOT_IMPORTED,
						  "Integer32");
				} else if (thisModulePtr->export.language ==
					   SMI_LANGUAGE_SPPI) {
				    smiPrintError(thisParserPtr,
						  ERR_SPPI_BASETYPE_NOT_IMPORTED,
						  "Integer32");
				}
			    }

			    /* TODO: any need to distinguish from INTEGER? */
			    $$ = smiHandle->typeInteger32Ptr;
			}
        |		INTEGER32
			{
			    defaultBasetype = SMI_BASETYPE_INTEGER32;
			}
			integerSubType
			{
			    Import *importPtr;
			    
			    importPtr = findImportByName("Integer32",
							 thisModulePtr);
			    if (importPtr) {
				importPtr->use++;
			    } else {
				if (thisModulePtr->export.language ==
				    SMI_LANGUAGE_SMIV2) {
				    smiPrintError(thisParserPtr,
						  ERR_SMIV2_BASETYPE_NOT_IMPORTED,
						  "Integer32");
				} else if (thisModulePtr->export.language ==
					   SMI_LANGUAGE_SPPI) {
				    smiPrintError(thisParserPtr,
						  ERR_SPPI_BASETYPE_NOT_IMPORTED,
						  "Integer32");
				}
			    }

			    $$ = duplicateType(smiHandle->typeInteger32Ptr, 0,
					       thisParserPtr);
			    setTypeList($$, $3);
			    smiCheckTypeRanges(thisParserPtr, $$);
			}
	|		UPPERCASE_IDENTIFIER
			{
			    defaultBasetype = SMI_BASETYPE_ENUM;
			}
			enumSpec
			{
			    Type *parentPtr;
			    Import *importPtr;
			    List *p;
			    
			    parentPtr = findTypeByModuleAndName(
			        thisParserPtr->modulePtr, $1);
			    if (!parentPtr) {
			        importPtr = findImportByName($1,
							     thisModulePtr);
				if (importPtr &&
				    importPtr->kind == KIND_TYPE) {
				    importPtr->use++;
				    parentPtr = findTypeByModulenameAndName(
					importPtr->export.module, $1);
				}
			    }
			    if (parentPtr) {
				if ((parentPtr->export.basetype !=
				     SMI_BASETYPE_ENUM) &&
				    (parentPtr->export.basetype !=
				     SMI_BASETYPE_BITS)) {
				    smiPrintError(thisParserPtr,
						  ERR_ILLEGAL_ENUM_FOR_PARENT_TYPE,
						  $1);
				    $$ = duplicateType(smiHandle->typeEnumPtr, 0,
						       thisParserPtr);
				} else {
				    $$ = duplicateType(parentPtr, 0,
						       thisParserPtr);
				}
				smiFree($1);
			    } else {
				/* 
				 * forward referenced type. create it,
				 * marked with FLAG_INCOMPLETE.
				 */
				parentPtr = addType($1,
						    SMI_BASETYPE_UNKNOWN,
						    FLAG_INCOMPLETE,
						    thisParserPtr);
				$$ = duplicateType(parentPtr, 0,
						   thisParserPtr);
			    }
			    setTypeList($$, $3);
			    for (p = $3; p; p = p->nextPtr)
				((NamedNumber *)p->ptr)->typePtr = $$;
			    smiCheckNamedNumbersOrder(parserPtr, $$);
			}
	|		moduleName '.' UPPERCASE_IDENTIFIER enumSpec
			/* TODO: UPPERCASE_IDENTIFIER must be an INTEGER */
			{
			    Type *parentPtr;
			    Import *importPtr;
			    List *p;
			    
                            defaultBasetype = SMI_BASETYPE_ENUM;
                            parentPtr = findTypeByModulenameAndName($1, $3);
			    if (!parentPtr) {
				importPtr =
				    findImportByModulenameAndName($1,
							  $3, thisModulePtr);
				if (importPtr &&
				    importPtr->kind == KIND_TYPE) {
				    importPtr->use++;
				    parentPtr =
					findTypeByModulenameAndName($1, $3);
				}
			    }
			    if (parentPtr) {
				if ((parentPtr->export.basetype !=
				     SMI_BASETYPE_ENUM) &&
				    (parentPtr->export.basetype !=
				     SMI_BASETYPE_BITS)) {
				    smiPrintError(thisParserPtr,
						  ERR_ILLEGAL_ENUM_FOR_PARENT_TYPE,
						  $3);
				    $$ = duplicateType(smiHandle->typeEnumPtr, 0,
						       thisParserPtr);
				} else {
				    $$ = duplicateType(parentPtr, 0,
						       thisParserPtr);
				}
			    } else {
				smiPrintError(thisParserPtr,
					      ERR_UNKNOWN_TYPE, $3);
				$$ = duplicateType(smiHandle->typeEnumPtr, 0,
						   thisParserPtr);
			    }
			    setTypeList($$, $4);
			    for (p = $4; p; p = p->nextPtr)
				((NamedNumber *)p->ptr)->typePtr = $$;
			    smiCheckNamedNumbersOrder(parserPtr, $$);
			    smiFree($1);
			    smiFree($3);
			}
	|		UPPERCASE_IDENTIFIER integerSubType
			{
			    Type *parentPtr;
			    Import *importPtr;
			    
			    parentPtr = findTypeByModuleAndName(
				thisParserPtr->modulePtr, $1);
			    if (!parentPtr) {
				importPtr = findImportByName($1,
							     thisModulePtr);
				if (importPtr &&
				    importPtr->kind == KIND_TYPE) {
				    importPtr->use++;
				    parentPtr = findTypeByModulenameAndName(
					importPtr->export.module, $1);
				}
			    }
			    if (parentPtr) {
				if ((parentPtr->export.basetype !=
				    SMI_BASETYPE_INTEGER32) &&
				    (parentPtr->export.basetype !=
				     SMI_BASETYPE_INTEGER64) &&
				    (parentPtr->export.basetype !=
				     SMI_BASETYPE_UNSIGNED32) &&
				    (parentPtr->export.basetype !=
				     SMI_BASETYPE_UNSIGNED64)) {
				    smiPrintError(thisParserPtr,
					     ERR_ILLEGAL_RANGE_FOR_PARENT_TYPE,
						  $1);
				    $$ = duplicateType(smiHandle->typeInteger32Ptr, 0,
						       thisParserPtr);
				    defaultBasetype = SMI_BASETYPE_INTEGER32;
				} else {
				    defaultBasetype =
					parentPtr->export.basetype;
				    $$ = duplicateType(parentPtr, 0,
						       thisParserPtr);
				}
				smiFree($1);
			    } else {
				/* 
				 * forward referenced type. create it,
				 * marked with FLAG_INCOMPLETE.
				 */
				parentPtr = addType($1,
						    SMI_BASETYPE_UNKNOWN,
						    FLAG_INCOMPLETE,
						    thisParserPtr);
				$$ = duplicateType(parentPtr, 0,
						   thisParserPtr);
				defaultBasetype = SMI_BASETYPE_INTEGER32;
			    }
			    setTypeList($$, $2);
			    smiCheckTypeRanges(thisParserPtr, $$);
			}
	|		moduleName '.' UPPERCASE_IDENTIFIER integerSubType
			/* TODO: UPPERCASE_IDENTIFIER must be an INT/Int32. */
			{
			    Type *parentPtr;
			    Import *importPtr;
			    
			    parentPtr = findTypeByModulenameAndName($1, $3);
			    if (!parentPtr) {
				importPtr = findImportByModulenameAndName($1,
							  $3, thisModulePtr);
				if (importPtr &&
				    importPtr->kind == KIND_TYPE) {
				    importPtr->use++;
				    parentPtr = findTypeByModulenameAndName(
					$1, $3);
				}
			    }
			    if (parentPtr) {
				if ((parentPtr->export.basetype !=
				    SMI_BASETYPE_INTEGER32) &&
				    (parentPtr->export.basetype !=
				     SMI_BASETYPE_INTEGER64) &&
				    (parentPtr->export.basetype !=
				     SMI_BASETYPE_UNSIGNED32) &&
				    (parentPtr->export.basetype !=
				     SMI_BASETYPE_UNSIGNED64)) {
				    smiPrintError(thisParserPtr,
					     ERR_ILLEGAL_RANGE_FOR_PARENT_TYPE,
						  $3);
				    $$ = duplicateType(smiHandle->typeInteger32Ptr, 0,
						       thisParserPtr);
				    defaultBasetype = SMI_BASETYPE_INTEGER32;
				} else {
				    defaultBasetype =
					parentPtr->export.basetype;
				    $$ = duplicateType(parentPtr, 0,
						       thisParserPtr);
				}
			    } else {
				smiPrintError(thisParserPtr,
					      ERR_UNKNOWN_TYPE, $3);
				$$ = duplicateType(smiHandle->typeInteger32Ptr, 0,
						   thisParserPtr);
				defaultBasetype = SMI_BASETYPE_INTEGER32;
			    }
			    setTypeList($$, $4);
			    smiCheckTypeRanges(thisParserPtr, $$);
			    smiFree($1);
			    smiFree($3);
			}
	|		OCTET STRING		/* (SIZE (0..65535))	     */
			{
			    defaultBasetype = SMI_BASETYPE_OCTETSTRING;
			    $$ = smiHandle->typeOctetStringPtr;
			}
	|		OCTET STRING
			{
			    defaultBasetype = SMI_BASETYPE_OCTETSTRING;
			}
			octetStringSubType
			{
			    
			    $$ = duplicateType(smiHandle->typeOctetStringPtr, 0,
					       thisParserPtr);
			    setTypeList($$, $4);
			    smiCheckTypeRanges(thisParserPtr, $$);
			}
	|		UPPERCASE_IDENTIFIER octetStringSubType
			{
			    Type *parentPtr;
			    Import *importPtr;
			    
			    defaultBasetype = SMI_BASETYPE_OCTETSTRING;
			    parentPtr = findTypeByModuleAndName(
				thisParserPtr->modulePtr, $1);
			    if (!parentPtr) {
				importPtr = findImportByName($1,
							     thisModulePtr);
				if (importPtr &&
				    importPtr->kind == KIND_TYPE) {
				    importPtr->use++;
				    parentPtr = findTypeByModulenameAndName(
					importPtr->export.module, $1);
				}
			    }
			    if (parentPtr) {
				if (parentPtr->export.basetype !=
				    SMI_BASETYPE_OCTETSTRING) {
				    smiPrintError(thisParserPtr,
					      ERR_ILLEGAL_SIZE_FOR_PARENT_TYPE,
						  $1);
				    $$ = duplicateType(smiHandle->typeOctetStringPtr, 0,
						       thisParserPtr);
				} else {
				    $$ = duplicateType(parentPtr, 0,
						       thisParserPtr);
				}
				smiFree($1);
			    } else {
				/* 
				 * forward referenced type. create it,
				 * marked with FLAG_INCOMPLETE.
				 */
				parentPtr = addType($1,
						    SMI_BASETYPE_UNKNOWN,
						    FLAG_INCOMPLETE,
						    thisParserPtr);
				$$ = duplicateType(parentPtr, 0,
						   thisParserPtr);
			    }
			    setTypeList($$, $2);
			    smiCheckTypeRanges(thisParserPtr, $$);
			}
	|		moduleName '.' UPPERCASE_IDENTIFIER octetStringSubType
			/* TODO: UPPERCASE_IDENTIFIER must be an OCTET STR. */
			{
			    Type *parentPtr;
			    Import *importPtr;
			    
			    defaultBasetype = SMI_BASETYPE_OCTETSTRING;
			    parentPtr = findTypeByModulenameAndName($1, $3);
			    if (!parentPtr) {
				importPtr = findImportByModulenameAndName($1,
							  $3, thisModulePtr);
				if (importPtr &&
				    importPtr->kind == KIND_TYPE) {
				    importPtr->use++;
				    parentPtr = findTypeByModulenameAndName(
					$1, $3);
				}
			    }
			    if (parentPtr) {
				if (parentPtr->export.basetype !=
				    SMI_BASETYPE_OCTETSTRING) {
				    smiPrintError(thisParserPtr,
					      ERR_ILLEGAL_SIZE_FOR_PARENT_TYPE,
						  $3);
				    $$ = duplicateType(smiHandle->typeOctetStringPtr, 0,
						       thisParserPtr);
				} else {
				    $$ = duplicateType(parentPtr, 0,
						       thisParserPtr);
				}
			    } else {
				smiPrintError(thisParserPtr,
					      ERR_UNKNOWN_TYPE, $3);
				$$ = duplicateType(smiHandle->typeOctetStringPtr, 0,
						   thisParserPtr);
			    }
			    setTypeList($$, $4);
			    smiCheckTypeRanges(thisParserPtr, $$);
			    smiFree($1);
			    smiFree($3);
			}
	|		OBJECT IDENTIFIER anySubType
			{
			    defaultBasetype = SMI_BASETYPE_OBJECTIDENTIFIER;
			    $$ = smiHandle->typeObjectIdentifierPtr;
			}
        ;

valueofSimpleSyntax:	NUMBER			/* 0..2147483647 */
			/* NOTE: Counter64 must not have a DEFVAL */
			{
			    $$ = smiMalloc(sizeof(SmiValue));
			    $$->basetype = SMI_BASETYPE_UNSIGNED32;
			    $$->value.unsigned32 = $1;
			}
	|		NEGATIVENUMBER		/* -2147483648..0 */
			{
			    $$ = smiMalloc(sizeof(SmiValue));
			    $$->basetype = SMI_BASETYPE_INTEGER32;
			    $$->value.integer32 = $1;
			}
        |               NUMBER64		/* 0..18446744073709551615 */
			{   
                            /* The scanner already checks for the language */
			    $$ = smiMalloc(sizeof(SmiValue));
			    $$->basetype = SMI_BASETYPE_UNSIGNED64;
			    $$->value.unsigned64 = $1;
			}
	|		NEGATIVENUMBER64	/* -9223372036854775807..0 */
			{
                            /* The scanner already checks for the language */
			    $$ = smiMalloc(sizeof(SmiValue));
			    $$->basetype = SMI_BASETYPE_INTEGER64;
			    $$->value.integer64 = $1;
			}
	|		BIN_STRING		/* number or OCTET STRING */
			{
			    char s[9];
			    int i, len, j;
			    
			    $$ = smiMalloc(sizeof(SmiValue));
			    if (defaultBasetype == SMI_BASETYPE_OCTETSTRING) {
				$$->basetype = SMI_BASETYPE_OCTETSTRING;
				len = strlen($1);
				$$->value.ptr =
				    smiMalloc((len+7)/8+1);
				for (i = 0; i < len; i += 8) {
				    strncpy(s, &$1[i], 8);
				    for (j = 1; j < 8; j++) {
					if (!s[j]) s[j] = '0';
				    }
				    s[8] = 0;
				    $$->value.ptr[i/8] =
					(unsigned char)strtol(s, 0, 2);
				}
				$$->len = (len+7)/8;
			    } else {
				$$->basetype = SMI_BASETYPE_UNSIGNED32;
				$$->value.unsigned32 = strtoul($1, NULL, 2);
			    }
			}
	|		HEX_STRING		/* number or OCTET STRING */
			{
			    char s[3];
			    int i, len;
			    
			    $$ = smiMalloc(sizeof(SmiValue));
			    if (defaultBasetype == SMI_BASETYPE_OCTETSTRING) {
				$$->basetype = SMI_BASETYPE_OCTETSTRING;
				len = strlen($1);
				$$->value.ptr = smiMalloc((len+1)/2+1);
				for (i = 0; i < len; i += 2) {
				    strncpy(s, &$1[i], 2);
				    if (!s[1]) s[1] = '0';
				    s[2] = 0;
				    $$->value.ptr[i/2] =
					(unsigned char)strtol(s, 0, 16);
				}
				$$->len = (len+1)/2;
			    } else {
				$$->basetype = SMI_BASETYPE_UNSIGNED32;
				$$->value.unsigned32 = strtoul($1, NULL, 16);
			    }
			}
	|		LOWERCASE_IDENTIFIER	/* enumeration or named oid */
			{
			    $$ = smiMalloc(sizeof(SmiValue));
			    if ((defaultBasetype != SMI_BASETYPE_ENUM) &&
				(defaultBasetype != SMI_BASETYPE_OBJECTIDENTIFIER)) {
				smiPrintError(thisParserPtr, ERR_DEFVAL_SYNTAX);
				$$->basetype = defaultBasetype;
				if (defaultBasetype == SMI_BASETYPE_ENUM) {
				    $$->len = 1;
				    $$->value.unsigned32 = 0;
				} else {
				    $$->len = 0;
				    $$->value.ptr = NULL;
				}
			    } else {
				$$->basetype = defaultBasetype;
				$$->len = -1;  /* indicates unresolved ptr */
				$$->value.ptr = $1; /* JS: needs strdup? */
			    }
			}
	|		QUOTED_STRING		/* an OCTET STRING */
			{
			    $$ = smiMalloc(sizeof(SmiValue));
			    $$->basetype = SMI_BASETYPE_OCTETSTRING;
			    $$->value.ptr = smiStrdup($1);
			    $$->len = strlen($1);
			}
			/* NOTE: If the value is an OBJECT IDENTIFIER, then
			 *       it must be expressed as a single ASN.1
			 *	 identifier, and not as a collection of
			 *	 of sub-identifiers.
			 *	 REF: draft,p.34
			 *	 Anyway, we try to accept it. But it's only
			 *	 possible for numbered sub-identifiers, since
			 *	 other identifiers would make something like
			 *	 { gaga } indistiguishable from a BitsValue.
			 */
	|		'{' objectIdentifier_defval '}'
			/*
			 * This is only for some MIBs with invalid numerical
			 * OID notation for DEFVALs. We DO NOT parse them
			 * correctly. We just don't want to produce a
			 * parser error.
			 */
			{
			    /*
			     * SMIv1 allows something like { 0 0 } !
			     * SMIv2 does not!
			     */
			    /* TODO: make it work correctly for SMIv1 */
			    if (thisModulePtr->export.language == SMI_LANGUAGE_SMIV2) {
				smiPrintError(thisParserPtr,
					      ERR_OID_DEFVAL_TOO_LONG_SMIV2);
			    } else {
				smiPrintError(thisParserPtr,
					      ERR_OID_DEFVAL_TOO_LONG_SMIV1);
			    }
			    $$ = smiMalloc(sizeof(SmiValue));
			    $$->basetype = SMI_BASETYPE_OBJECTIDENTIFIER;
			    $$->len = 2;
			    $$->value.oid = smiMalloc(2 * sizeof(SmiSubid));
			    $$->value.oid[0] = 0;
			    $$->value.oid[1] = 0;
			}
	;

/*
 * In a SEQUENCE { ... } there are no sub-types, enumerations or
 * named bits. REF: draft, p.29
 */
sequenceSimpleSyntax:	INTEGER	anySubType
			{
			    $$ = smiHandle->typeInteger32Ptr;
			}
        |		INTEGER32 anySubType
			{
			    Import *importPtr;
			    
			    /* TODO: any need to distinguish from INTEGER? */
			    $$ = smiHandle->typeInteger32Ptr;

			    importPtr = findImportByName("Integer32",
							 thisModulePtr);
			    if (importPtr) {
				importPtr->use++;
			    } else {
				if (thisModulePtr->export.language ==
				    SMI_LANGUAGE_SMIV2) {
				    smiPrintError(thisParserPtr,
						  ERR_SMIV2_BASETYPE_NOT_IMPORTED,
						  "Integer32");
				} else if (thisModulePtr->export.language ==
					   SMI_LANGUAGE_SPPI) {
				    smiPrintError(thisParserPtr,
						  ERR_SPPI_BASETYPE_NOT_IMPORTED,
						  "Integer32");
				}
			    }
			}
	|		OCTET STRING anySubType
			{
			    $$ = smiHandle->typeOctetStringPtr;
			}
	|		OBJECT IDENTIFIER anySubType
			{
			    $$ = smiHandle->typeObjectIdentifierPtr;
			}
	;

ApplicationSyntax:	IPADDRESS anySubType
			{
			    Import *importPtr;
			    
			    $$ = findTypeByName("IpAddress");
			    if (! $$) {
				smiPrintError(thisParserPtr, ERR_UNKNOWN_TYPE,
					      "IpAddress");
			    }
			    
			    importPtr = findImportByName("IpAddress",
							 thisModulePtr);
			    if (importPtr) {
				importPtr->use++;
			    } else {
				if (thisModulePtr->export.language ==
				    SMI_LANGUAGE_SMIV2) {
				    smiPrintError(thisParserPtr,
						  ERR_SMIV2_BASETYPE_NOT_IMPORTED,
						  "IpAddress");
				} else if (thisModulePtr->export.language ==
					   SMI_LANGUAGE_SPPI) {
				    smiPrintError(thisParserPtr,
						  ERR_SPPI_BASETYPE_NOT_IMPORTED,
						  "IpAddress");
				}
			    }
			}
	|		COUNTER32 anySubType  /* (0..4294967295)	     */
			{
			    Import *importPtr;

                            if ((thisParserPtr->modulePtr->export.language == SMI_LANGUAGE_SPPI) &&
                                !findImportByName("Counter32", thisParserPtr->modulePtr))
                                smiPrintError(thisParserPtr, ERR_SMI_CONSTRUCT_IN_PIB, "COUNTER32");
			    $$ = findTypeByName("Counter32");
			    if (! $$) {
				smiPrintError(thisParserPtr, ERR_UNKNOWN_TYPE,
					      "Counter32");
			    }
			    
			    importPtr = findImportByName("Counter32",
							 thisModulePtr);
			    if (importPtr) {
				importPtr->use++;
			    } else {
				if (thisModulePtr->export.language ==
				    SMI_LANGUAGE_SMIV2) {
				    smiPrintError(thisParserPtr,
						  ERR_SMIV2_BASETYPE_NOT_IMPORTED,
						  "Counter32");
				}
			    }
			}
	|		COUNTER32 integerSubType
			{
			    Import *importPtr;
			    List *listPtr, *nextListPtr;
			    
			    smiPrintError(thisParserPtr,
					  ERR_ILLEGAL_RANGE_FOR_COUNTER,
					  "Counter32");
			    for (listPtr = $2; listPtr;
				 listPtr = nextListPtr) {
				nextListPtr = listPtr->nextPtr;
				smiFree((Range *)listPtr->ptr);
				smiFree(listPtr);
			    }
			    
                            if ((thisParserPtr->modulePtr->export.language == SMI_LANGUAGE_SPPI) &&
                                !findImportByName("Counter32", thisParserPtr->modulePtr))
                                smiPrintError(thisParserPtr, ERR_SMI_CONSTRUCT_IN_PIB, "Counter32");
			    $$ = findTypeByName("Counter32");
			    if (! $$) {
				smiPrintError(thisParserPtr, ERR_UNKNOWN_TYPE,
					      "Counter32");
			    }
			    
			    importPtr = findImportByName("Counter32",
							 thisModulePtr);
			    if (importPtr) {
				importPtr->use++;
			    } else {
				if (thisModulePtr->export.language ==
				    SMI_LANGUAGE_SMIV2) {
				    smiPrintError(thisParserPtr,
						  ERR_SMIV2_BASETYPE_NOT_IMPORTED,
						  "Counter32");
				}
			    }
			}
	|		GAUGE32			/* (0..4294967295)	     */
			{
			    Import *importPtr;

                            if ((thisParserPtr->modulePtr->export.language == SMI_LANGUAGE_SPPI) &&
                                !findImportByName("Gauge32", thisParserPtr->modulePtr))
                                smiPrintError(thisParserPtr, ERR_SMI_CONSTRUCT_IN_PIB, "Gauge32");
			    $$ = findTypeByName("Gauge32");
			    if (! $$) {
				smiPrintError(thisParserPtr, ERR_UNKNOWN_TYPE,
					      "Gauge32");
			    }
			    
			    importPtr = findImportByName("Gauge32",
							 thisModulePtr);
			    if (importPtr) {
				importPtr->use++;
			    } else {
				if (thisModulePtr->export.language ==
				    SMI_LANGUAGE_SMIV2) {
				    smiPrintError(thisParserPtr,
						  ERR_SMIV2_BASETYPE_NOT_IMPORTED,
						  "Gauge32");
				}
			    }
			}
	|		GAUGE32 integerSubType
			{
			    Type *parentPtr;
			    Import *importPtr;
			    
                            if ((thisParserPtr->modulePtr->export.language == SMI_LANGUAGE_SPPI) &&
                                !findImportByName("Gauge32", thisParserPtr->modulePtr))
                                smiPrintError(thisParserPtr, ERR_SMI_CONSTRUCT_IN_PIB, "Gauge32");
			    parentPtr = findTypeByName("Gauge32");
			    if (! parentPtr) {
				smiPrintError(thisParserPtr, ERR_UNKNOWN_TYPE,
					      "Gauge32");
				$$ = NULL;
			    } else {
				$$ = duplicateType(parentPtr, 0,
						   thisParserPtr);
				setTypeList($$, $2);
				smiCheckTypeRanges(thisParserPtr, $$);
			    }
			    
			    importPtr = findImportByName("Gauge32",
							 thisModulePtr);
			    if (importPtr) {
				importPtr->use++;
			    } else {
				if (thisModulePtr->export.language ==
				    SMI_LANGUAGE_SMIV2) {
				    smiPrintError(thisParserPtr,
						  ERR_SMIV2_BASETYPE_NOT_IMPORTED,
						  "Gauge32");
				}
			    }
			}
	|		UNSIGNED32		/* (0..4294967295)	     */
			{
			    Import *importPtr;

			    $$ = smiHandle->typeUnsigned32Ptr;
			    
			    importPtr = findImportByName("Unsigned32",
							 thisModulePtr);
			    if (importPtr) {
				importPtr->use++;
			    } else {
				if (thisModulePtr->export.language ==
				    SMI_LANGUAGE_SMIV2) {
				    smiPrintError(thisParserPtr,
						  ERR_SMIV2_BASETYPE_NOT_IMPORTED,
						  "Unsigned32");
				} else if (thisModulePtr->export.language ==
					   SMI_LANGUAGE_SPPI) {
				    smiPrintError(thisParserPtr,
						  ERR_SPPI_BASETYPE_NOT_IMPORTED,
						  "Unsigned32");
				}
			    }
			}
	|		UNSIGNED32 integerSubType
			{
			    Import *importPtr;
			    
			    $$ = duplicateType(smiHandle->typeUnsigned32Ptr, 0,
					       thisParserPtr);
			    setTypeList($$, $2);
			    smiCheckTypeRanges(thisParserPtr, $$);

			    importPtr = findImportByName("Unsigned32",
							 thisModulePtr);
			    if (importPtr) {
				importPtr->use++;
			    } else {
				if (thisModulePtr->export.language ==
				    SMI_LANGUAGE_SMIV2) {
				    smiPrintError(thisParserPtr,
						  ERR_SMIV2_BASETYPE_NOT_IMPORTED,
						  "Unsigned32");
				} else if (thisModulePtr->export.language ==
					   SMI_LANGUAGE_SPPI) {
				    smiPrintError(thisParserPtr,
						  ERR_SPPI_BASETYPE_NOT_IMPORTED,
						  "Unsigned32");
				}
			    }
			}
	|		TIMETICKS anySubType
			{
			    Import *importPtr;
			    
			    $$ = findTypeByName("TimeTicks");
			    if (! $$) {
				smiPrintError(thisParserPtr, ERR_UNKNOWN_TYPE,
					      "TimeTicks");
			    }
			    
			    importPtr = findImportByName("TimeTicks",
							 thisModulePtr);
			    if (importPtr) {
				importPtr->use++;
			    } else {
				if (thisModulePtr->export.language ==
				    SMI_LANGUAGE_SMIV2) {
				    smiPrintError(thisParserPtr,
						  ERR_SMIV2_BASETYPE_NOT_IMPORTED,
						  "TimeTicks");
				} else if (thisModulePtr->export.language ==
					   SMI_LANGUAGE_SPPI) {
				    smiPrintError(thisParserPtr,
						  ERR_SPPI_BASETYPE_NOT_IMPORTED,
						  "TimeTicks");
				}
			    }
			}
	|		OPAQUE			/* IMPLICIT OCTET STRING     */
			{
			    Import *importPtr;
			    
			    $$ = findTypeByName("Opaque");
			    if (! $$) {
				smiPrintError(thisParserPtr, ERR_UNKNOWN_TYPE,
					      "Opaque");
			    } else {
				if (thisModulePtr->export.language ==
				    SMI_LANGUAGE_SMIV2) {
				    smiPrintError(thisParserPtr,
						  ERR_SMIV2_OPAQUE_OBSOLETE);
				} else if (thisModulePtr->export.language ==
					   SMI_LANGUAGE_SPPI) {
				    smiPrintError(thisParserPtr,
						  ERR_SPPI_OPAQUE_OBSOLETE);
				}
			    }
			    
			    importPtr = findImportByName("Opaque",
							 thisModulePtr);
			    if (importPtr) {
				importPtr->use++;
			    } else {
				if (thisModulePtr->export.language ==
				    SMI_LANGUAGE_SMIV2) {
				    smiPrintError(thisParserPtr,
						  ERR_SMIV2_BASETYPE_NOT_IMPORTED,
						  "Opaque");
				} else if (thisModulePtr->export.language ==
					   SMI_LANGUAGE_SPPI) {
				    smiPrintError(thisParserPtr,
						  ERR_SPPI_BASETYPE_NOT_IMPORTED,
						  "Opaque");
				}
			    }
			}
	|		OPAQUE octetStringSubType
			{
			    Type *parentPtr;
			    Import *importPtr;
			    
			    parentPtr = findTypeByName("Opaque");
			    if (! parentPtr) {
				smiPrintError(thisParserPtr, ERR_UNKNOWN_TYPE,
					      "Opaque");
				$$ = NULL;
			    } else {
				if (thisModulePtr->export.language ==
				    SMI_LANGUAGE_SMIV2) {
				    smiPrintError(thisParserPtr,
						  ERR_SMIV2_OPAQUE_OBSOLETE,
						  "Opaque");
				} else if (thisModulePtr->export.language ==
					   SMI_LANGUAGE_SPPI) {
				    smiPrintError(thisParserPtr,
						  ERR_SPPI_OPAQUE_OBSOLETE,
						  "Opaque");
				}
				$$ = duplicateType(parentPtr, 0,
						   thisParserPtr);
				setTypeList($$, $2);
				smiCheckTypeRanges(thisParserPtr, $$);
			    }

			    importPtr = findImportByName("Opaque",
							 thisModulePtr);
			    if (importPtr) {
				importPtr->use++;
			    } else {
				if (thisModulePtr->export.language ==
				    SMI_LANGUAGE_SMIV2) {
				    smiPrintError(thisParserPtr,
						  ERR_SMIV2_BASETYPE_NOT_IMPORTED,
						  "Opaque");
				} else if (thisModulePtr->export.language ==
					   SMI_LANGUAGE_SPPI) {
				    smiPrintError(thisParserPtr,
						  ERR_SPPI_BASETYPE_NOT_IMPORTED,
						  "Opaque");
				}
			    }
			}
	|		COUNTER64 anySubType   /* (0..18446744073709551615) */
			{
			    Import *importPtr;

                            if ((thisParserPtr->modulePtr->export.language == SMI_LANGUAGE_SPPI) &&
                                !findImportByName("Counter64", thisParserPtr->modulePtr))
                                smiPrintError(thisParserPtr, ERR_SMI_CONSTRUCT_IN_PIB, "Counter64");
			    $$ = findTypeByName("Counter64");
			    if (! $$) {
				smiPrintError(thisParserPtr, ERR_UNKNOWN_TYPE,
					      "Counter64");
			    }

			    importPtr = findImportByName("Counter64",
							 thisModulePtr);
			    if (importPtr) {
				importPtr->use++;
			    } else {
				if (thisModulePtr->export.language ==
				    SMI_LANGUAGE_SMIV2) {
				    smiPrintError(thisParserPtr,
						  ERR_SMIV2_BASETYPE_NOT_IMPORTED,
						  "Counter64");
				}
			    }
			}
	|		COUNTER64 integerSubType
			{
			    Import *importPtr;
			    List *listPtr, *nextListPtr;
			    
			    smiPrintError(thisParserPtr,
					  ERR_ILLEGAL_RANGE_FOR_COUNTER,
					  "Counter64");
			    for (listPtr = $2; listPtr;
				 listPtr = nextListPtr) {
				nextListPtr = listPtr->nextPtr;
				smiFree((Range *)listPtr->ptr);
				smiFree(listPtr);
			    }
			    
                            if ((thisParserPtr->modulePtr->export.language == SMI_LANGUAGE_SPPI) &&
                                !findImportByName("Counter64", thisParserPtr->modulePtr))
                                smiPrintError(thisParserPtr, ERR_SMI_CONSTRUCT_IN_PIB, "Counter64");
			    $$ = findTypeByName("Counter64");
			    if (! $$) {
				smiPrintError(thisParserPtr, ERR_UNKNOWN_TYPE,
					      "Counter64");
			    }

			    importPtr = findImportByName("Counter64",
							 thisModulePtr);
			    if (importPtr) {
				importPtr->use++;
			    } else {
				if (thisModulePtr->export.language ==
				    SMI_LANGUAGE_SMIV2) {
				    smiPrintError(thisParserPtr,
						  ERR_SMIV2_BASETYPE_NOT_IMPORTED,
						  "Counter64");
				}
			    }
			}
	|		INTEGER64               /* (-9223372036854775807..9223372036854775807) */
			{
			    Import *importPtr;

			    $$ = findTypeByModulenameAndName(
				thisParserPtr->modulePtr->export.name, "Integer64");
			    if (! $$) {
				importPtr = findImportByName("Integer64",
							     thisModulePtr);
				if (!importPtr) {
				    $$ = findTypeByName("Integer64");
				    if ((thisParserPtr->modulePtr->export.language != SMI_LANGUAGE_SPPI)) {
					smiPrintError(thisParserPtr, ERR_SPPI_CONSTRUCT_IN_MIB, "Integer64");
				    } else {
					smiPrintError(thisParserPtr,
						      ERR_SPPI_BASETYPE_NOT_IMPORTED,
						      "Integer64");
				    }
				} else {
				    importPtr->use++;
				    $$ = findTypeByModulenameAndName(
					importPtr->export.module,
					importPtr->export.name);
				}
			    }
			}
	|		INTEGER64 integerSubType
			{
			    Type *parentPtr;
			    Import *importPtr;
			    
			    parentPtr = findTypeByModulenameAndName(
				thisParserPtr->modulePtr->export.name, "Integer64");
			    if (! parentPtr) {
				importPtr = findImportByName("Integer64",
							     thisModulePtr);
				if (!importPtr) {
				    parentPtr = findTypeByName("Integer64");
				    if ((thisParserPtr->modulePtr->export.language != SMI_LANGUAGE_SPPI)) {
					smiPrintError(thisParserPtr, ERR_SPPI_CONSTRUCT_IN_MIB, "Integer64");
				    } else {
					smiPrintError(thisParserPtr,
						      ERR_SPPI_BASETYPE_NOT_IMPORTED,
						      "Integer64");
				    }
				} else {
				    importPtr->use++;
				    parentPtr = findTypeByModulenameAndName(
					importPtr->export.module,
					importPtr->export.name);
				}
			    }
			    if (! parentPtr) {
				smiPrintError(thisParserPtr, ERR_UNKNOWN_TYPE,
					      "Integer64");
				$$ = NULL;
			    } else {
				$$ = duplicateType(parentPtr, 0,
						   thisParserPtr);
				setTypeList($$, $2);
				smiCheckTypeRanges(thisParserPtr, $$);
			    }
			}
	|		UNSIGNED64	        /* (0..18446744073709551615) */
			{
			    Import *importPtr;

			    $$ = findTypeByModulenameAndName(
				thisParserPtr->modulePtr->export.name, "Unsigned64");
			    if (! $$) {
				importPtr = findImportByName("Unsigned64",
							     thisModulePtr);
				if (!importPtr) {
				    $$ = findTypeByName("Unsigned64");
				    if ((thisParserPtr->modulePtr->export.language != SMI_LANGUAGE_SPPI)) {
					smiPrintError(thisParserPtr, ERR_SPPI_CONSTRUCT_IN_MIB, "Unsigned64");
				    } else {
					smiPrintError(thisParserPtr,
						      ERR_SPPI_BASETYPE_NOT_IMPORTED,
						      "Unsigned64");
				    }
				} else {
				    importPtr->use++;
				    $$ = findTypeByModulenameAndName(
					importPtr->export.module,
					importPtr->export.name);
				}
			    }
			}
	|		UNSIGNED64 integerSubType
			{
			    Type *parentPtr;
			    Import *importPtr;
			    
			    parentPtr = findTypeByModulenameAndName(
				thisParserPtr->modulePtr->export.name, "Unsigned64");
			    if (! parentPtr) {
				importPtr = findImportByName("Unsigned64",
							     thisModulePtr);
				if (!importPtr) {
				    parentPtr = findTypeByName("Unsigned64");
				    if ((thisParserPtr->modulePtr->export.language != SMI_LANGUAGE_SPPI)) {
					smiPrintError(thisParserPtr, ERR_SPPI_CONSTRUCT_IN_MIB, "Unsigned64");
				    } else {
					smiPrintError(thisParserPtr,
						      ERR_SPPI_BASETYPE_NOT_IMPORTED,
						      "Unsigned64");
				    }
				} else {
				    importPtr->use++;
				    parentPtr = findTypeByModulenameAndName(
					importPtr->export.module,
					importPtr->export.name);
				}
			    }
			    if (! parentPtr) {
				smiPrintError(thisParserPtr, ERR_UNKNOWN_TYPE,
					      "Unsigned64");
				$$ = NULL;
			    } else {
				$$ = duplicateType(parentPtr, 0,
						   thisParserPtr);
				setTypeList($$, $2);
				smiCheckTypeRanges(thisParserPtr, $$);
			    }
			}
	;

/*
 * In a SEQUENCE { ... } there are no sub-types, enumerations or
 * named bits. REF: draft, p.29
 */
sequenceApplicationSyntax: IPADDRESS anySubType
			{
			    Import *importPtr;
			    
			    $$ = findTypeByName("IpAddress");
			    if (! $$) {
				smiPrintError(thisParserPtr, ERR_UNKNOWN_TYPE,
					      "IpAddress");
			    }

			    importPtr = findImportByName("IpAddress",
							 thisModulePtr);
			    if (importPtr) {
				importPtr->use++;
			    } else {
				if (thisModulePtr->export.language ==
				    SMI_LANGUAGE_SMIV2) {
				    smiPrintError(thisParserPtr,
						  ERR_SMIV2_BASETYPE_NOT_IMPORTED,
						  "IpAddress");
				} else if (thisModulePtr->export.language ==
					   SMI_LANGUAGE_SPPI) {
				    smiPrintError(thisParserPtr,
						  ERR_SPPI_BASETYPE_NOT_IMPORTED,
						  "IpAddress");
				}
			    }
			}
	|		COUNTER32 anySubType
			{
			    Import *importPtr;
			    
                            if ((thisParserPtr->modulePtr->export.language == SMI_LANGUAGE_SPPI) &&
                                !findImportByName("Counter32", thisParserPtr->modulePtr))
                                smiPrintError(thisParserPtr, ERR_SMI_CONSTRUCT_IN_PIB, "Counter32");
			    $$ = findTypeByName("Counter32");
			    if (! $$) {
				smiPrintError(thisParserPtr, ERR_UNKNOWN_TYPE,
					      "Counter32");
			    }

			    importPtr = findImportByName("Counter32",
							 thisModulePtr);
			    if (importPtr) {
				importPtr->use++;
			    } else {
				if (thisModulePtr->export.language ==
				    SMI_LANGUAGE_SMIV2) {
				    smiPrintError(thisParserPtr,
						  ERR_SMIV2_BASETYPE_NOT_IMPORTED,
						  "Counter32");
				}
			    }
			}
	|		GAUGE32	anySubType	/* (0..4294967295)	     */
			{
			    Import *importPtr;
			    
                            if ((thisParserPtr->modulePtr->export.language == SMI_LANGUAGE_SPPI) &&
                                !findImportByName("Gauge32", thisParserPtr->modulePtr))
                                smiPrintError(thisParserPtr, ERR_SMI_CONSTRUCT_IN_PIB, "Gauge32");
			    $$ = findTypeByName("Gauge32");
			    if (! $$) {
				smiPrintError(thisParserPtr, ERR_UNKNOWN_TYPE,
					      "Gauge32");
			    }

			    importPtr = findImportByName("Gauge32",
							 thisModulePtr);
			    if (importPtr) {
				importPtr->use++;
			    } else {
				if (thisModulePtr->export.language ==
				    SMI_LANGUAGE_SMIV2) {
				    smiPrintError(thisParserPtr,
						  ERR_SMIV2_BASETYPE_NOT_IMPORTED,
						  "Gauge32");
				}
			    }
			}
	|		UNSIGNED32 anySubType /* (0..4294967295)	     */
			{
			    Import *importPtr;
			    
			    $$ = smiHandle->typeUnsigned32Ptr;

			    importPtr = findImportByName("Unsigned32",
							 thisModulePtr);
			    if (importPtr) {
				importPtr->use++;
			    } else {
				if (thisModulePtr->export.language ==
				    SMI_LANGUAGE_SMIV2) {
				    smiPrintError(thisParserPtr,
						  ERR_SMIV2_BASETYPE_NOT_IMPORTED,
						  "Unsigned32");
				} else if (thisModulePtr->export.language ==
					   SMI_LANGUAGE_SPPI) {
				    smiPrintError(thisParserPtr,
						  ERR_SPPI_BASETYPE_NOT_IMPORTED,
						  "Unsigned32");
				}
			    }
			}
	|		TIMETICKS anySubType	/* (0..4294967295)	     */
			{
			    Import *importPtr;
			    
			    $$ = findTypeByName("TimeTicks");
			    if (! $$) {
				smiPrintError(thisParserPtr, ERR_UNKNOWN_TYPE,
					      "TimeTicks");
			    }

			    importPtr = findImportByName("TimeTicks",
							 thisModulePtr);
			    if (importPtr) {
				importPtr->use++;
			    } else {
				if (thisModulePtr->export.language ==
				    SMI_LANGUAGE_SMIV2) {
				    smiPrintError(thisParserPtr,
						  ERR_SMIV2_BASETYPE_NOT_IMPORTED,
						  "TimeTicks");
				} else if (thisModulePtr->export.language ==
					   SMI_LANGUAGE_SPPI) {
				    smiPrintError(thisParserPtr,
						  ERR_SPPI_BASETYPE_NOT_IMPORTED,
						  "TimeTicks");
				}
			    }
			}
	|		OPAQUE			/* IMPLICIT OCTET STRING     */
			{
			    Import *importPtr;
			    
			    $$ = findTypeByName("Opaque");
			    if (! $$) {
				smiPrintError(thisParserPtr, ERR_UNKNOWN_TYPE,
					      "Opaque");
			    } else {
				if (thisModulePtr->export.language ==
				    SMI_LANGUAGE_SMIV2) {
				    smiPrintError(thisParserPtr,
						  ERR_SMIV2_OPAQUE_OBSOLETE,
						  "Opaque");
				} else if (thisModulePtr->export.language ==
					   SMI_LANGUAGE_SPPI) {
				    smiPrintError(thisParserPtr,
						  ERR_SPPI_OPAQUE_OBSOLETE,
						  "Opaque");
				}
			    }

			    importPtr = findImportByName("Opaque",
							 thisModulePtr);
			    if (importPtr) {
				importPtr->use++;
			    } else {
				if (thisModulePtr->export.language ==
				    SMI_LANGUAGE_SMIV2) {
				    smiPrintError(thisParserPtr,
						  ERR_SMIV2_BASETYPE_NOT_IMPORTED,
						  "Opaque");
				} else if (thisModulePtr->export.language ==
					   SMI_LANGUAGE_SPPI) {
				    smiPrintError(thisParserPtr,
						  ERR_SPPI_BASETYPE_NOT_IMPORTED,
						  "Opaque");
				}
			    }
			}
	|		COUNTER64 anySubType    /* (0..18446744073709551615) */
			{
			    Import *importPtr;

                            if ((thisParserPtr->modulePtr->export.language == SMI_LANGUAGE_SPPI) &&
                                !findImportByName("Counter64", thisModulePtr))
                                smiPrintError(thisParserPtr, ERR_SMI_CONSTRUCT_IN_PIB, "Counter64");
			    $$ = findTypeByName("Counter64");
			    if (! $$) {
				smiPrintError(thisParserPtr, ERR_UNKNOWN_TYPE,
					      "Counter64");
			    }

			    importPtr = findImportByName("Counter64",
							 thisModulePtr);
			    if (importPtr) {
				importPtr->use++;
			    } else {
				if (thisModulePtr->export.language ==
				    SMI_LANGUAGE_SMIV2) {
				    smiPrintError(thisParserPtr,
						  ERR_SMIV2_BASETYPE_NOT_IMPORTED,
						  "Counter64");
				}
			    }
			}
	|		INTEGER64	        /* (-9223372036854775807..9223372036854775807) */
			{
			    Import *importPtr;

                            if (thisParserPtr->modulePtr->export.language != SMI_LANGUAGE_SPPI)
                                smiPrintError(thisParserPtr, ERR_SPPI_CONSTRUCT_IN_MIB, "Integer64");
			    $$ = findTypeByName("Integer64");
			    if (! $$) {
				smiPrintError(thisParserPtr, ERR_UNKNOWN_TYPE,
					      "Integer64");
			    }

			    importPtr = findImportByName("Integer64",
							 thisModulePtr);
			    if (importPtr) {
				importPtr->use++;
			    } else {
				if (thisModulePtr->export.language ==
					   SMI_LANGUAGE_SPPI) {
				    smiPrintError(thisParserPtr,
						  ERR_SPPI_BASETYPE_NOT_IMPORTED,
						  "Integer64");
				}
			    }
			}
	|		UNSIGNED64	        /* (0..18446744073709551615) */
			{
			    Import *importPtr;

			    importPtr = findImportByName("Unsigned64",
							 thisModulePtr);
                            if ((thisParserPtr->modulePtr->export.language != SMI_LANGUAGE_SPPI) && (!importPtr))
                                smiPrintError(thisParserPtr, ERR_SPPI_CONSTRUCT_IN_MIB, "Unsigned64");
			    $$ = findTypeByName("Unsigned64");
			    if (! $$) {
				smiPrintError(thisParserPtr, ERR_UNKNOWN_TYPE,
					      "Unsigned64");
			    }

			    if (importPtr) {
				importPtr->use++;
			    } else {
				if (thisModulePtr->export.language ==
					   SMI_LANGUAGE_SPPI) {
				    smiPrintError(thisParserPtr,
						  ERR_SPPI_BASETYPE_NOT_IMPORTED,
						  "Unsigned64");
				}
			    }
			}
	;

anySubType:		integerSubType
			{
			    List *listPtr, *nextListPtr;

			    if (thisModulePtr->export.language == SMI_LANGUAGE_SMIV2)
				smiPrintError(thisParserPtr,
					      ERR_UNEXPECTED_TYPE_RESTRICTION);

			    for (listPtr = $1; listPtr;
				 listPtr = nextListPtr) {
				nextListPtr = listPtr->nextPtr;
				smiFree((Range *)(listPtr->ptr));
				smiFree(listPtr);
			    }

			    $$ = NULL;
			}
	|	        octetStringSubType
			{
			    List *listPtr, *nextListPtr;

			    if (thisModulePtr->export.language == SMI_LANGUAGE_SMIV2)
				smiPrintError(thisParserPtr,
					      ERR_UNEXPECTED_TYPE_RESTRICTION);

			    for (listPtr = $1; listPtr;
				 listPtr = nextListPtr) {
				nextListPtr = listPtr->nextPtr;
				smiFree((Range *)(listPtr->ptr));
				smiFree(listPtr);
			    }

			    $$ = NULL;
			}
	|		enumSpec
			{
			    List *listPtr, *nextListPtr;

			    if (thisModulePtr->export.language == SMI_LANGUAGE_SMIV2)
				smiPrintError(thisParserPtr,
					      ERR_UNEXPECTED_TYPE_RESTRICTION);

			    for (listPtr = $1; listPtr;
				 listPtr = nextListPtr) {
				nextListPtr = listPtr->nextPtr;
				smiFree(((NamedNumber *)(listPtr->ptr))->export.name);
				smiFree((NamedNumber *)(listPtr->ptr));
				smiFree(listPtr);
			    }

			    $$ = NULL;
			}
	|		/* empty */
			{
			    $$ = NULL;
			}
        ;		      


/* REF: draft,p.46 */
integerSubType:		'(' ranges ')'		/* at least one range        */
			/*
			 * the specification mentions an alternative of an
			 * empty RHS here. this would lead to reduce/reduce
			 * conflicts. instead, we differentiate the parent
			 * rule(s) (SimpleSyntax).
			 */
			{ $$ = $2; }
	;

octetStringSubType:	'(' SIZE '(' ranges ')' ')'
			/*
			 * the specification mentions an alternative of an
			 * empty RHS here. this would lead to reduce/reduce
			 * conflicts. instead, we differentiate the parent
			 * rule(s) (SimpleSyntax).
			 */
			{
			    $$ = $4;
			}
	;

ranges:			range
			{
			    $$ = smiMalloc(sizeof(List));
			    $$->ptr = $1;
			    $$->nextPtr = NULL;
			}
	|		ranges '|' range
			{
			    List *p, *pp;
			    
			    p = smiMalloc(sizeof(List));
			    p->ptr = (void *)$3;
			    p->nextPtr = NULL;
			    for (pp = $1; pp->nextPtr; pp = pp->nextPtr);
			    pp->nextPtr = p;
			    
			    $$ = $1;
			}
	;

range:			value
			{
			    $$ = smiMalloc(sizeof(Range));
			    $$->export.minValue = *$1;
			    $$->export.maxValue = *$1;
			    smiFree($1);
			}
	|		value DOT_DOT value
			{
			    $$ = smiMalloc(sizeof(Range));
			    $$->export.minValue = *$1;
			    $$->export.maxValue = *$3;
			    smiFree($1);
			    smiFree($3);
			}
	;

value:			NEGATIVENUMBER
			{
			    $$ = smiMalloc(sizeof(SmiValue));
			    $$->basetype = SMI_BASETYPE_INTEGER32;
			    $$->value.integer32 = $1;
			}
	|		NUMBER
			{
			    $$ = smiMalloc(sizeof(SmiValue));
			    $$->basetype = SMI_BASETYPE_UNSIGNED32;
			    $$->value.unsigned32 = $1;
			}
	|		NEGATIVENUMBER64
			{
			    $$ = smiMalloc(sizeof(SmiValue));
			    $$->basetype = SMI_BASETYPE_INTEGER64;
			    $$->value.integer64 = $1;
			}
	|		NUMBER64
			{
			    $$ = smiMalloc(sizeof(SmiValue));
			    $$->basetype = SMI_BASETYPE_UNSIGNED64;
			    $$->value.unsigned64 = $1;
			}
	|		HEX_STRING
			{
			    char s[3];
			    int i, len;
			    
			    $$ = smiMalloc(sizeof(SmiValue));
			    if (defaultBasetype == SMI_BASETYPE_OCTETSTRING) {
				$$->basetype = SMI_BASETYPE_OCTETSTRING;
				len = strlen($1);
				$$->value.ptr = smiMalloc((len+1)/2+1);
				for (i = 0; i < len; i += 2) {
				    strncpy(s, &$1[i], 2);
				    if (!s[1]) s[1] = '0';
				    s[2] = 0;
				    $$->value.ptr[i/2] =
					(unsigned char)strtol(s, 0, 16);
				}
				$$->len = (len+1)/2;
			    } else {
				$$->basetype = SMI_BASETYPE_UNSIGNED32;
				$$->value.unsigned32 = strtoul($1, NULL, 16);
			    }
			}
	|		BIN_STRING
			{
			    char s[9];
			    int i, len, j;
			    
			    $$ = smiMalloc(sizeof(SmiValue));
			    if (defaultBasetype == SMI_BASETYPE_OCTETSTRING) {
				$$->basetype = SMI_BASETYPE_OCTETSTRING;
				len = strlen($1);
				$$->value.ptr = smiMalloc((len+7)/8+1);
				for (i = 0; i < len; i += 8) {
				    strncpy(s, &$1[i], 8);
				    for (j = 1; j < 8; j++) {
					if (!s[j]) s[j] = '0';
				    }
				    s[8] = 0;
				    $$->value.ptr[i/8] =
					(unsigned char)strtol(s, 0, 2);
				}
				$$->len = (len+7)/8;
			    } else {
				$$->basetype = SMI_BASETYPE_UNSIGNED32;
				$$->value.unsigned32 = strtoul($1, NULL, 2);
			    }
			}
	;

enumSpec:		'{' enumItems '}'
			{
			    $$ = $2;
			}
	;

enumItems:		enumItem
			{
			    $$ = smiMalloc(sizeof(List));
			    $$->ptr = $1;
			    $$->nextPtr = NULL;
			}
	|		enumItems ',' enumItem
			{
			    List *p, *pp;
			    
			    p = smiMalloc(sizeof(List));
			    p->ptr = (void *)$3;
			    p->nextPtr = NULL;
			    for (pp = $1; pp->nextPtr; pp = pp->nextPtr);
			    pp->nextPtr = p;
			    $$ = $1;
			}
	;

enumItem:		LOWERCASE_IDENTIFIER
			{
			    checkNameLen(thisParserPtr, $1,
					 ERR_ENUMNAME_32, ERR_ENUMNAME_64);
			    if (thisModulePtr->export.language == SMI_LANGUAGE_SMIV2)
			    {
				if (strchr($1, '-')) {
				    smiPrintError(thisParserPtr,
					  ERR_NAMEDNUMBER_INCLUDES_HYPHEN,
						  $1);
				}
			    }
			}
			'(' enumNumber ')'
			{
			    $$ = smiMalloc(sizeof(NamedNumber));
			    $$->export.name = $1;
			    $$->export.value = *$4;
			    smiFree($4);
			}
	;

enumNumber:		NUMBER
			{
			    if ($1 > SMI_BASETYPE_INTEGER32_MAX) {
				smiPrintError(thisParserPtr,
					      ERR_INTEGER32_TOO_LARGE, $1);
			    }
			    if ((thisModulePtr->export.language == SMI_LANGUAGE_SMIV1) &&
				($1 == 0)) {
				smiPrintError(thisParserPtr,
					      ERR_ENUM_ZERO);
			    }
			    $$ = smiMalloc(sizeof(SmiValue));
			    $$->basetype = SMI_BASETYPE_INTEGER32;
			    $$->value.integer32 = $1;
			}
	|		NEGATIVENUMBER
			{
			    $$ = smiMalloc(sizeof(SmiValue));
			    $$->basetype = SMI_BASETYPE_INTEGER32;
			    $$->value.integer32 = $1;
			    /* TODO: non-negative is suggested */
			}
	;

Status:			LOWERCASE_IDENTIFIER
			{
			    if (thisModulePtr->export.language == SMI_LANGUAGE_SMIV2)
			    {
			        if (!strcmp($1, "current")) {
				    $$ = SMI_STATUS_CURRENT;
			        } else if (!strcmp($1, "deprecated")) {
				    $$ = SMI_STATUS_DEPRECATED;
			        } else if (!strcmp($1, "obsolete")) {
				    $$ = SMI_STATUS_OBSOLETE;
			        } else {
				    smiPrintError(thisParserPtr,
					          ERR_INVALID_SMIV2_STATUS,
					          $1);
				    if (!strcmp($1, "mandatory")
				        || !strcmp($1, "optional")) {
				        /* best guess */
				        $$ = SMI_STATUS_CURRENT;
				    } else {
				        $$ = SMI_STATUS_UNKNOWN;
				    }
			        }
			    } else if (thisModulePtr->export.language != SMI_LANGUAGE_SPPI) {
			        if (!strcmp($1, "mandatory")) {
				    $$ = SMI_STATUS_MANDATORY;
			        } else if (!strcmp($1, "optional")) {
				    $$ = SMI_STATUS_OPTIONAL;
			        } else if (!strcmp($1, "obsolete")) {
				    $$ = SMI_STATUS_OBSOLETE;
			        } else if (!strcmp($1, "deprecated")) {
				    $$ = SMI_STATUS_OBSOLETE;
			        } else {
				    smiPrintError(thisParserPtr,
					          ERR_INVALID_SMIV1_STATUS,
					          $1);
				    if (!strcmp($1, "current")) {
				        /* best guess */
				        $$ = SMI_STATUS_MANDATORY; 
				    } else {
				        $$ = SMI_STATUS_UNKNOWN;
				    }
			        }
			    } else { /* it is SPPI */
			        if (!strcmp($1, "current")) {
				    $$ = SMI_STATUS_CURRENT;
			        } else if (!strcmp($1, "obsolete")) {
				    $$ = SMI_STATUS_OBSOLETE;
			        } else if (!strcmp($1, "deprecated")) {
				    $$ = SMI_STATUS_OBSOLETE;
                                } else {
                                    smiPrintError(thisParserPtr,
                                                  ERR_INVALID_SPPI_STATUS, $1);
                                    $$ = SMI_STATUS_UNKNOWN;
                                }
                            }
			    smiFree($1);
			}
        ;		

Status_Capabilities:	LOWERCASE_IDENTIFIER
			{
			    if (!strcmp($1, "current")) {
				$$ = SMI_STATUS_CURRENT;
			    } else if (!strcmp($1, "obsolete")) {
				$$ = SMI_STATUS_OBSOLETE;
			    } else {
				smiPrintError(thisParserPtr,
					      ERR_INVALID_CAPABILITIES_STATUS,
					      $1);
				$$ = SMI_STATUS_UNKNOWN;
			    }
			    smiFree($1);
			}
        ;

DisplayPart:		DISPLAY_HINT Text
			{
			    $$ = $2;
			    
			    if ($2 && !strlen($2)) {
				smiPrintError(thisParserPtr,
					      ERR_EMPTY_FORMAT);
			    }
			}
        |		/* empty */
			{
			    $$ = NULL;
			}
        ;

UnitsPart:		UNITS Text
			{
			    $$ = $2;
			    
			    if ($2 && !strlen($2)) {
				smiPrintError(thisParserPtr,
					      ERR_EMPTY_UNITS);
			    }
			}
        |		/* empty */
			{
			    $$ = NULL;
			}
        ;

Access:			LOWERCASE_IDENTIFIER
			{
			    if (thisModulePtr->export.language == SMI_LANGUAGE_SMIV2)
			    {
				if (!strcmp($1, "not-accessible")) {
				    $$ = SMI_ACCESS_NOT_ACCESSIBLE;
				} else if (!strcmp($1,
						   "accessible-for-notify")) {
				    $$ = SMI_ACCESS_NOTIFY;
				} else if (!strcmp($1, "read-only")) {
				    $$ = SMI_ACCESS_READ_ONLY;
				} else if (!strcmp($1, "read-write")) {
				    $$ = SMI_ACCESS_READ_WRITE;
				} else if (!strcmp($1, "read-create")) {
				    $$ = SMI_ACCESS_READ_WRITE;
				    thisParserPtr->flags |= FLAG_CREATABLE;
				    /* TODO:remember it's really read-create */
				} else if (!strcmp($1, "write-only")) {
				    smiPrintError(thisParserPtr,
						  ERR_SMIV2_WRITE_ONLY);
				    $$ = SMI_ACCESS_READ_WRITE;
				} else {
				    smiPrintError(thisParserPtr,
						  ERR_INVALID_SMIV2_ACCESS,
						  $1);
				    $$ = SMI_ACCESS_UNKNOWN;
				}
			    } else if (thisModulePtr->export.language != SMI_LANGUAGE_SPPI) {
				if (!strcmp($1, "not-accessible")) {
				    $$ = SMI_ACCESS_NOT_ACCESSIBLE;
				} else if (!strcmp($1, "read-only")) {
				    $$ = SMI_ACCESS_READ_ONLY;
				} else if (!strcmp($1, "read-write")) {
				    $$ = SMI_ACCESS_READ_WRITE;
				} else if (!strcmp($1, "write-only")) {
				    smiPrintError(thisParserPtr,
						  ERR_SMIV1_WRITE_ONLY);
				    $$ = SMI_ACCESS_READ_WRITE;
				} else {
				    smiPrintError(thisParserPtr,
						  ERR_INVALID_SMIV1_ACCESS,
						  $1);
				    $$ = SMI_ACCESS_UNKNOWN;
				}
			    } else {
			        if (!strcmp($1, "install")) {
				    $$ = SMI_ACCESS_INSTALL;
			        } else if (!strcmp($1, "install-notify")) {
				    $$ = SMI_ACCESS_INSTALL_NOTIFY;
			        } else if (!strcmp($1, "notify")) {
				    $$ = SMI_ACCESS_NOTIFY;
			        } else if (!strcmp($1, "report-only")) {
				    $$ = SMI_ACCESS_REPORT_ONLY;
			        } else if (!strcmp($1, "not-accessible")) {
				    $$ = SMI_ACCESS_NOT_ACCESSIBLE;
                                } else {
				    smiPrintError(thisParserPtr,
					          ERR_INVALID_SPPI_ACCESS,
					          $1);
				    $$ = SMI_ACCESS_UNKNOWN;
			        }
                            }
			    smiFree($1);
			}
        ;

IndexPart:              PIB_INDEX
                        {
                            if (thisParserPtr->modulePtr->export.language != SMI_LANGUAGE_SPPI)
                                smiPrintError(thisParserPtr, ERR_SPPI_CONSTRUCT_IN_MIB, "PIB-INDEX");
                        }
                        '{' Entry '}'
                        {
                            List *p = smiMalloc(sizeof(List));
                            
                            p->ptr       = $4;
                            p->nextPtr   = NULL;
                            
			    $$.indexkind = SMI_INDEX_INDEX;
			    $$.implied   = impliedFlag;
			    $$.listPtr   = p;
			    $$.rowPtr    = NULL;
                            indexFlag    = INDEXFLAG_PIBINDEX;
			}
        |		AUGMENTS '{' Entry '}'
			/* TODO: no AUGMENTS clause in v1 */
			/* TODO: how to differ INDEX and AUGMENTS ? */
			{
			    $$.indexkind    = SMI_INDEX_AUGMENT;
			    $$.implied      = 0;
			    $$.listPtr      = NULL;
			    $$.rowPtr       = $3;
                            indexFlag       = INDEXFLAG_AUGMENTS;
			}
        |		EXTENDS
                        {
                            if (thisParserPtr->modulePtr->export.language != SMI_LANGUAGE_SPPI)
                                smiPrintError(thisParserPtr, ERR_SPPI_CONSTRUCT_IN_MIB, "EXTENDS");
                        }
                        '{' Entry '}'
			{
			    $$.indexkind    = SMI_INDEX_SPARSE;
			    $$.implied      = 0;
			    $$.listPtr      = NULL;
			    $$.rowPtr       = $4;
                            indexFlag       = INDEXFLAG_EXTENDS;
			} 
        |		/* empty */
			{
			    $$.indexkind = SMI_INDEX_UNKNOWN;
			}
	;

MibIndex:		INDEX
                        {
                            /* 
                             * To avoid ambiguity caused by merging
                             * the SMI and SPPI parser we use a flag.
                             */
                            if (thisParserPtr->modulePtr->export.language != SMI_LANGUAGE_SPPI) {
                                /*
                                 * Only INDEX or AUGMENTS are allowed for SMI
                                 */
                                if (indexFlag != INDEXFLAG_NONE)
                                    smiPrintError(thisParserPtr, ERR_INDEX_AND_AUGMENTS_USED);
                            } else {
                                /*
                                 * INDEX may only be used if PIB_INDEX was used
                                 */
                                if (indexFlag != INDEXFLAG_PIBINDEX)
                                    smiPrintError(thisParserPtr, ERR_INDEX_WITHOUT_PIB_INDEX);
                            }
                            
			    /*
			     * Use a global variable to fetch and remember
			     * whether we have seen an IMPLIED keyword.
			     */
			    impliedFlag = 0;
			}
			'{' IndexTypes '}'
			{
			    $$.indexkind = SMI_INDEX_INDEX;
			    $$.implied   = impliedFlag;
			    $$.listPtr   = $4;
			    $$.rowPtr    = NULL;
                        }
        |               /* empty */
			{
			    $$.indexkind = SMI_INDEX_UNKNOWN;
			}
        ;
        
IndexTypes:		IndexType
			{
			    $$ = smiMalloc(sizeof(List));
			    $$->ptr = $1;
			    $$->nextPtr = NULL;
			}
        |		IndexTypes ',' IndexType
			/* TODO: might this list be emtpy? */
			{
			    List *p, *pp;
			    
			    p = smiMalloc(sizeof(List));
			    p->ptr = $3;
			    p->nextPtr = NULL;
			    for (pp = $1; pp->nextPtr; pp = pp->nextPtr);
			    pp->nextPtr = p;
			    $$ = $1;
			}
	;

IndexType:		IMPLIED Index
			{
			    impliedFlag = 1;
			    $$ = $2;
			}
	|		Index
			{
			    $$ = $1;
			}
	;

Index:			ObjectName
			/*
			 * TODO: use the SYNTAX value of the correspondent
			 *       OBJECT-TYPE invocation
			 */
			{
			    $$ = $1;
			}
        ;

Entry:			ObjectName
			{
			    $$ = $1;
			}
        ;

DefValPart:		DEFVAL '{' Value '}'
			{
			    $$ = $3;
			    if ((defaultBasetype == SMI_BASETYPE_BITS) &&
				($$->basetype != SMI_BASETYPE_BITS)) {
				smiPrintError(thisParserPtr,
					      ERR_DEFVAL_SYNTAX);
				if ($$->basetype == SMI_BASETYPE_OCTETSTRING) {
				    smiFree($$->value.ptr);
				}
				smiFree($$);
				$$ = NULL;
			    }
			}
	|		/* empty */
			{ $$ = NULL; }
			/* TODO: different for DefValPart in AgentCaps ? */
	;

Value:			valueofObjectSyntax
			{ $$ = $1; }
	|		'{' BitsValue '}'
			{
			    $$ = smiMalloc(sizeof(SmiValue));
			    $$->basetype = SMI_BASETYPE_BITS;
			    $$->value.ptr = (void *)$2;
			}
	;

BitsValue:		BitNames
			{ $$ = $1; }
	|		/* empty */
			{ $$ = NULL; }
	;

BitNames:		LOWERCASE_IDENTIFIER
			{
			    $$ = smiMalloc(sizeof(List));
			    $$->ptr = $1;
			    $$->nextPtr = NULL;
			}
	|		BitNames ',' LOWERCASE_IDENTIFIER
			{
			    List *p, *pp;
			    
			    p = smiMalloc(sizeof(List));
			    p->ptr = $3;
			    p->nextPtr = NULL;
			    for (pp = $1; pp->nextPtr; pp = pp->nextPtr);
			    pp->nextPtr = p;
			    $$ = $1;
			}
	;

ObjectName:		objectIdentifier
			{
			    $$ = $1;
			}
	;

NotificationName:	objectIdentifier
			{
			    $$ = $1;
			}
	;

ReferPart:		REFERENCE Text
			{
			    $$ = $2;

			    if ($2 && !strlen($2)) {
				smiPrintError(thisParserPtr,
					      ERR_EMPTY_REFERENCE);
			    }
			}
	|		/* empty */
			{ $$ = NULL; }
	;

RevisionPart:		Revisions
			{ $$ = 0; }
	|		/* empty */
			{ $$ = 0; }
	;

Revisions:		Revision
			{ $$ = 0; }
	|		Revisions Revision
			{ $$ = 0; }
	;

Revision:		REVISION ExtUTCTime
			{
			    thisParserPtr->firstRevisionLine = thisParserPtr->line;

			    if (thisParserPtr->modulePtr->lastRevisionPtr &&
				($2 >= thisParserPtr->modulePtr->lastRevisionPtr->export.date)) {
				smiPrintError(thisParserPtr,
					      ERR_REVISION_NOT_DESCENDING);
			    }

			    if ($2 > thisParserPtr->modulePtr->lastUpdated) {
				smiPrintError(thisParserPtr,
					      ERR_REVISION_AFTER_LAST_UPDATE);
			    }
			}
			DESCRIPTION Text
			{
			    Revision *revisionPtr;

			    checkDescr(thisParserPtr, $5);

			    revisionPtr = addRevision($2, $5, thisParserPtr);
			    if (revisionPtr) {
				setRevisionLine(revisionPtr,
						thisParserPtr->firstRevisionLine,
						thisParserPtr);
			    }
			    $$ = revisionPtr ? 0 : -1;
			}
	;

NotificationObjectsPart: OBJECTS '{' Objects '}'
			{
			    $$ = $3;
			}
	|		/* empty */
			{
			    $$ = NULL;
			}
	;

ObjectGroupObjectsPart:	OBJECTS '{' Objects '}'
			{
			    $$ = $3;
			}
	;

Objects:		Object
			{
			    $$ = smiMalloc(sizeof(List));
			    $$->ptr = $1;
			    $$->nextPtr = NULL;
			}
	|		Objects ',' Object
			{
			    List *p, *pp;
			    
			    p = smiMalloc(sizeof(List));
			    p->ptr = $3;
			    p->nextPtr = NULL;
			    for (pp = $1; pp->nextPtr; pp = pp->nextPtr);
			    pp->nextPtr = p;
			    $$ = $1;
			}
	;

Object:			ObjectName
			{
			    $$ = $1;
			    if ((thisParserPtr->currentDecl == SMI_DECL_OBJECTGROUP) &&
				$$->modulePtr != thisParserPtr->modulePtr) {
                                smiPrintError(thisParserPtr,
					      ERR_COMPLIANCE_MEMBER_NOT_LOCAL,
					      $$->export.name);
			    }
			}
	;

NotificationsPart:	NOTIFICATIONS '{' Notifications '}'
			{
			    $$ = $3;
			}
	;

Notifications:		Notification
			{
			    $$ = smiMalloc(sizeof(List));
			    $$->ptr = $1;
			    $$->nextPtr = NULL;
			}
	|		Notifications ',' Notification
			{
			    List *p, *pp;
			    
			    p = smiMalloc(sizeof(List));
			    p->ptr = $3;
			    p->nextPtr = NULL;
			    for (pp = $1; pp->nextPtr; pp = pp->nextPtr);
			    pp->nextPtr = p;
			    $$ = $1;
			}
	;

Notification:		NotificationName
			{
			    $$ = $1;
			    if ($$->modulePtr != thisParserPtr->modulePtr) {
                                smiPrintError(thisParserPtr,
					      ERR_COMPLIANCE_MEMBER_NOT_LOCAL,
					      $$->export.name);
			    }
			}
	;

Text:			QUOTED_STRING
			{
			    int len;
			    $$ = smiStrdup($1);
			    len = strlen($$);
			    while (len > 0 && $$[len-1] == '\n') {
				$$[--len] = 0;
			    }
			}
	;

ExtUTCTime:		QUOTED_STRING
			{
			    $$ = checkDate(thisParserPtr, $1);
			}
	;

objectIdentifier:	{
			    thisParserPtr->parentNodePtr = smiHandle->rootNodePtr;
			}
			subidentifiers
			{
			    $$ = $2;
			    if ($$) {
				thisParserPtr->parentNodePtr = $2->nodePtr;
			    } else {
				thisParserPtr->parentNodePtr = NULL;
			    }
			}
	;

subidentifiers:
			subidentifier
			{
			    $$ = $1;
			}
	|		subidentifiers
			subidentifier
			{
			    $$ = $2;
			}
        ;

subidentifier:
			/* LOWERCASE_IDENTIFIER */
			fuzzy_lowercase_identifier
			{
			    Object *objectPtr;
			    Import *importPtr;
			    
			    if (thisParserPtr->parentNodePtr != smiHandle->rootNodePtr) {
				smiPrintError(thisParserPtr,
					      ERR_OIDLABEL_NOT_FIRST, $1);
			    }
			    objectPtr = findObjectByModuleAndName(
				thisParserPtr->modulePtr, $1);
			    if (objectPtr) {
				$$ = objectPtr;
				smiFree($1);
			    } else {
				importPtr = findImportByName($1,
							     thisModulePtr);
				if (!importPtr ||
				    (importPtr->kind == KIND_NOTFOUND)) {
				    /*
				     * If we are in a MODULE-COMPLIANCE
				     * statement with a given MODULE...
				     */
				    if (thisParserPtr->complianceModulePtr) {
					objectPtr =
					    findObjectByModuleAndName(
						thisParserPtr->complianceModulePtr, $1);
					if (objectPtr) {
					    importPtr = addImport(
						$1,
						thisParserPtr);
					    setImportModulename(importPtr,
								thisParserPtr->complianceModulePtr->export.name);
					    addImportFlags(importPtr,
							   FLAG_INCOMPLIANCE);
					    importPtr->use++;
					} else {
					    objectPtr = addObject($1,
								  thisParserPtr->pendingNodePtr, 0,
								  FLAG_INCOMPLETE,
								  thisParserPtr);
					    smiPrintError(thisParserPtr,
							  ERR_IDENTIFIER_NOT_IN_MODULE, $1,
							  thisParserPtr->complianceModulePtr->export.name);
					}
				    } else if (thisParserPtr->capabilitiesModulePtr) {
					objectPtr =
					    findObjectByModuleAndName(
						thisParserPtr->capabilitiesModulePtr, $1);
					if (objectPtr) {
					    importPtr = addImport(
						$1,
						thisParserPtr);
					    setImportModulename(importPtr,
								thisParserPtr->capabilitiesModulePtr->
								export.name);
					    addImportFlags(importPtr,
							   FLAG_INCOMPLIANCE);
					    importPtr->use++;
					} else {
					    objectPtr = addObject($1,
								  thisParserPtr->pendingNodePtr, 0,
								  FLAG_INCOMPLETE,
								  thisParserPtr);
					    smiPrintError(thisParserPtr,
							  ERR_IDENTIFIER_NOT_IN_MODULE, $1,
							  thisParserPtr->capabilitiesModulePtr->export.name);
					}
				    } else {
					/* 
					 * forward referenced node.
					 * create it,
					 * marked with FLAG_INCOMPLETE.
					 */
					objectPtr = addObject($1,
							      thisParserPtr->pendingNodePtr,
							      0,
							      FLAG_INCOMPLETE,
							      thisParserPtr);
				    }
				    $$ = objectPtr;
				} else {
				    /*
				     * imported object.
				     */
				    importPtr->use++;
				    $$ = findObjectByModulenameAndName(
					importPtr->export.module, $1);
				    smiFree($1);
				}
			    }
			    if ($$)
				thisParserPtr->parentNodePtr = $$->nodePtr;
			}
	|		moduleName '.' LOWERCASE_IDENTIFIER
			{
			    Object *objectPtr;
			    Import *importPtr;
			    char *md;
			    
			    if (thisParserPtr->parentNodePtr != smiHandle->rootNodePtr) {
				md = smiMalloc(sizeof(char) *
					       (strlen($1) + strlen($3) + 2));
				sprintf(md, "%s.%s", $1, $3);
				smiPrintError(thisParserPtr,
					      ERR_OIDLABEL_NOT_FIRST, md);
				smiFree(md);
			    } else {
				objectPtr = findObjectByModulenameAndName(
				    $1, $3);
				if (objectPtr) {
				    $$ = objectPtr;
				    smiFree($1);
				    smiFree($3);
				} else {
				    importPtr = findImportByModulenameAndName(
					$1, $3, thisModulePtr);
				    if (!importPtr ||
					(importPtr->kind == KIND_NOTFOUND)) {
					/* TODO: check: $1 == thisModule ? */
					/*
					 * If we are in a MODULE-COMPLIANCE
					 * statement with a given MODULE...
					 */
					if (thisParserPtr->complianceModulePtr) {
					    objectPtr =
						findObjectByModuleAndName(
						    thisParserPtr->complianceModulePtr, $1);
					    if (objectPtr) {
						importPtr = addImport(
						    $1,
						    thisParserPtr);
						setImportModulename(importPtr,
						    thisParserPtr->complianceModulePtr->export.name);
						addImportFlags(importPtr,
							       FLAG_INCOMPLIANCE);
						importPtr->use++;
					    } else {
						objectPtr = addObject($1,
						    thisParserPtr->pendingNodePtr, 0,
						    FLAG_INCOMPLETE,
						    thisParserPtr);
						smiPrintError(thisParserPtr,
					      ERR_IDENTIFIER_NOT_IN_MODULE, $1,
					     thisParserPtr->complianceModulePtr->export.name);
					    }
					} else if (thisParserPtr->capabilitiesModulePtr) {
					    objectPtr =
						findObjectByModuleAndName(
						    thisParserPtr->capabilitiesModulePtr, $1);
					    if (objectPtr) {
						importPtr = addImport(
						    $1,
						    thisParserPtr);
						setImportModulename(importPtr,
						        thisParserPtr->capabilitiesModulePtr->
								  export.name);
						addImportFlags(importPtr,
							       FLAG_INCOMPLIANCE);
						importPtr->use++;
					    } else {
						objectPtr = addObject($1,
						    thisParserPtr->pendingNodePtr, 0,
						    FLAG_INCOMPLETE,
						    thisParserPtr);
						smiPrintError(thisParserPtr,
					      ERR_IDENTIFIER_NOT_IN_MODULE, $1,
					   thisParserPtr->capabilitiesModulePtr->export.name);
					    }
					} else {
					    /* 
					     * forward referenced node.
					     * create it,
					     * marked with FLAG_INCOMPLETE.
					     */
					    objectPtr = addObject($3,
							    thisParserPtr->pendingNodePtr,
							      0,
							      FLAG_INCOMPLETE,
							      thisParserPtr);
					    smiFree($1);
					}
					$$ = objectPtr;
				    } else {
					/*
					 * imported object.
					 */
					importPtr->use++;
					$$ = findObjectByModulenameAndName(
					    importPtr->export.module, $3);
					smiFree($1);
					smiFree($3);
				    }
				}
				if ($$)
				    thisParserPtr->parentNodePtr = $$->nodePtr;
			    }
			}
	|		NUMBER
			{
			    Node *nodePtr;
			    Object *objectPtr;

			    nodePtr = findNodeByParentAndSubid(thisParserPtr->parentNodePtr,
							       $1);
			    if (nodePtr && nodePtr->lastObjectPtr &&
	       		(nodePtr->lastObjectPtr->modulePtr == thisModulePtr)) {
				/*
				 * hopefully, the last defined Object for
				 * this Node is the one we expect.
				 */
				$$ = nodePtr->lastObjectPtr;
			    } else {
				objectPtr = addObject(NULL,
						      thisParserPtr->parentNodePtr,
						      $1,
						      FLAG_INCOMPLETE,
						      thisParserPtr);
				$$ = objectPtr;
			    }
			    thisParserPtr->parentNodePtr = $$->nodePtr;
			}
	|		LOWERCASE_IDENTIFIER '(' NUMBER ')'
			{
			    Object *objectPtr = NULL;
			    Object *oldObjectPtr = NULL;
			    Node *oldNodePtr = NULL;
			    
			    /* TODO: search in local module and
			     *       in imported modules
			     */

			    oldNodePtr = findNodeByParentAndSubid(
				thisParserPtr->parentNodePtr, $3);
			    oldObjectPtr = findObjectByModuleAndName(
                                thisParserPtr->modulePtr, $1);

			    if (oldObjectPtr &&
				((oldObjectPtr->nodePtr->subid != $3) ||
				 (oldObjectPtr->nodePtr->parentPtr != thisParserPtr->parentNodePtr))) {
				smiPrintError(thisParserPtr,
					      ERR_IDENTIFIER_OID_CHANGED,
					      $1);
				smiPrintErrorAtLine(thisParserPtr,
						    ERR_PREVIOUS_DEFINITION,
						    oldObjectPtr->line,
						    oldObjectPtr->export.name);
				objectPtr = addObject($1,
						      thisParserPtr->parentNodePtr,
						      $3, 0, thisParserPtr);
				setObjectDecl(objectPtr,
					      SMI_DECL_IMPL_OBJECT);
				$$ = objectPtr;
				thisParserPtr->parentNodePtr = $$->nodePtr;
			    } else if (oldNodePtr &&
				       oldNodePtr->lastObjectPtr &&
				       oldNodePtr->lastObjectPtr->export.name &&
				       strcmp(oldNodePtr->lastObjectPtr->export.name, $1)) {
				smiPrintError(thisParserPtr,
					      ERR_OIDLABEL_CHANGED,
					      $1, oldNodePtr->lastObjectPtr->export.name);
				smiPrintErrorAtLine(thisParserPtr,
						    ERR_PREVIOUS_DEFINITION,
						    oldNodePtr->lastObjectPtr->line,
						    oldNodePtr->lastObjectPtr->export.name);
				objectPtr = addObject($1,
						      thisParserPtr->parentNodePtr,
						      $3, 0, thisParserPtr);
				setObjectDecl(objectPtr,
					      SMI_DECL_IMPL_OBJECT);
				$$ = objectPtr;
				thisParserPtr->parentNodePtr = $$->nodePtr;
			    } else {
				objectPtr = addObject($1, thisParserPtr->parentNodePtr,
						      $3, 0,
						      thisParserPtr);
				setObjectDecl(objectPtr,
					      SMI_DECL_IMPL_OBJECT);
				$$ = objectPtr;
				thisParserPtr->parentNodePtr = $$->nodePtr;
			    }
			}
	|		moduleName '.' LOWERCASE_IDENTIFIER '(' NUMBER ')'
			{
			    Object *objectPtr = NULL;
			    Object *oldObjectPtr = NULL;
			    Node *oldNodePtr = NULL;
			    char *md;

			    md = smiMalloc(sizeof(char) *
					   (strlen($1) + strlen($3) + 2));
			    sprintf(md, "%s.%s", $1, $3);

			    oldNodePtr = findNodeByParentAndSubid(
				thisParserPtr->parentNodePtr, $5);
			    oldObjectPtr = findObjectByModulenameAndName(
                                $1, $3);

			    if (oldObjectPtr &&
				((oldObjectPtr->nodePtr->subid != $5) ||
				 (oldObjectPtr->nodePtr->parentPtr != thisParserPtr->parentNodePtr))) {
				smiPrintError(thisParserPtr,
					      ERR_ILLEGALLY_QUALIFIED, md);
				smiPrintError(thisParserPtr,
					      ERR_IDENTIFIER_OID_CHANGED,
					      $3);
				smiPrintErrorAtLine(thisParserPtr,
						    ERR_PREVIOUS_DEFINITION,
						    oldObjectPtr->line,
						    oldObjectPtr->export.name);
				objectPtr = addObject($3,
						      thisParserPtr->parentNodePtr,
						      $5, 0, thisParserPtr);
				setObjectDecl(objectPtr,
					      SMI_DECL_IMPL_OBJECT);
				$$ = objectPtr;
				thisParserPtr->parentNodePtr = $$->nodePtr;
			    } else if (oldNodePtr &&
				       oldNodePtr->lastObjectPtr &&
				       strcmp(oldNodePtr->lastObjectPtr->export.name, $3)) {
				smiPrintError(thisParserPtr,
					      ERR_ILLEGALLY_QUALIFIED, md);
				smiPrintError(thisParserPtr,
					      ERR_OIDLABEL_CHANGED,
					      $3, oldNodePtr->lastObjectPtr->export.name);
				smiPrintErrorAtLine(thisParserPtr,
						    ERR_PREVIOUS_DEFINITION,
						    oldNodePtr->lastObjectPtr->line,
						    oldNodePtr->lastObjectPtr->export.name);
				objectPtr = addObject($3,
						      thisParserPtr->parentNodePtr,
						      $5, 0, thisParserPtr);
				setObjectDecl(objectPtr,
					      SMI_DECL_IMPL_OBJECT);
				$$ = objectPtr;
				thisParserPtr->parentNodePtr = $$->nodePtr;
			    } else {
				smiPrintError(thisParserPtr,
					      ERR_ILLEGALLY_QUALIFIED, md);
				objectPtr = addObject($3, thisParserPtr->parentNodePtr,
						      $5, 0,
						      thisParserPtr);
				setObjectDecl(objectPtr,
					      SMI_DECL_IMPL_OBJECT);
				$$ = objectPtr;
				thisParserPtr->parentNodePtr = $$->nodePtr;
			    }
			    smiFree(md);
			}
	;

objectIdentifier_defval: subidentifiers_defval
			{ $$ = NULL; }
        ;		/* TODO */

subidentifiers_defval:	subidentifier_defval
			{ $$ = 0; }
	|		subidentifiers_defval subidentifier_defval
			{ $$ = 0; }
        ;		/* TODO */

subidentifier_defval:	LOWERCASE_IDENTIFIER '(' NUMBER ')'
			{ $$ = 0; }
	|		NUMBER
			{ $$ = 0; }
	;		/* TODO */

objectGroupClause:	LOWERCASE_IDENTIFIER
			{
			    thisParserPtr->firstStatementLine = thisParserPtr->line;
			    thisParserPtr->currentDecl = SMI_DECL_OBJECTGROUP;
			    
			    checkNameLen(thisParserPtr, $1,
					 ERR_OIDNAME_32, ERR_OIDNAME_64);
			    smiCheckObjectName(thisParserPtr,
					       thisModulePtr, $1);
			}
			OBJECT_GROUP
			{
			    Import *importPtr;
			    
			    if (thisModulePtr->export.language == SMI_LANGUAGE_UNKNOWN)
				thisModulePtr->export.language = SMI_LANGUAGE_SMIV2;

			    importPtr = findImportByName("OBJECT-GROUP",
							 thisModulePtr);
			    if (importPtr) {
				importPtr->use++;
			    } else {
				smiPrintError(thisParserPtr,
					      ERR_MACRO_NOT_IMPORTED,
					      "OBJECT-GROUP", "SNMPv2-CONF");
			    }
			}
			ObjectGroupObjectsPart
			STATUS Status
			DESCRIPTION Text
			{
			    checkDescr(thisParserPtr, $9);
			}
			ReferPart
			COLON_COLON_EQUAL '{' objectIdentifier '}'
			{
			    Object *objectPtr;
			    
			    objectPtr = $14;

			    smiCheckObjectReuse(thisParserPtr, $1, &objectPtr);

			    objectPtr = setObjectName(objectPtr, $1, thisParserPtr);
			    setObjectDecl(objectPtr, SMI_DECL_OBJECTGROUP);
			    setObjectLine(objectPtr, thisParserPtr->firstStatementLine,
					  thisParserPtr);
			    addObjectFlags(objectPtr, FLAG_REGISTERED);
			    deleteObjectFlags(objectPtr, FLAG_INCOMPLETE);
			    setObjectStatus(objectPtr, $7);
			    setObjectDescription(objectPtr, $9, thisParserPtr);
			    if ($11) {
				setObjectReference(objectPtr, $11, thisParserPtr);
			    }
			    setObjectAccess(objectPtr,
					    SMI_ACCESS_NOT_ACCESSIBLE);
			    setObjectList(objectPtr, $5);
			    $$ = 0;
			}
	;

notificationGroupClause: LOWERCASE_IDENTIFIER
			{
			    thisParserPtr->firstStatementLine = thisParserPtr->line;
			    thisParserPtr->currentDecl = SMI_DECL_NOTIFICATIONGROUP;
			    
			    checkNameLen(thisParserPtr, $1,
					 ERR_OIDNAME_32, ERR_OIDNAME_64);
			    smiCheckObjectName(thisParserPtr,
					       thisModulePtr, $1);
			}
			NOTIFICATION_GROUP
			{
			    Import *importPtr;
                            if (thisParserPtr->modulePtr->export.language == SMI_LANGUAGE_SPPI)
                                smiPrintError(thisParserPtr, ERR_SMI_CONSTRUCT_IN_PIB, "NOTIFICATION-GROUP");
			    
			    if (thisModulePtr->export.language == SMI_LANGUAGE_UNKNOWN)
				thisModulePtr->export.language = SMI_LANGUAGE_SMIV2;

			    importPtr = findImportByName("NOTIFICATION-GROUP",
							 thisModulePtr);
			    if (importPtr) {
				importPtr->use++;
			    } else {
				smiPrintError(thisParserPtr,
					      ERR_MACRO_NOT_IMPORTED,
					      "NOTIFICATION-GROUP",
					      "SNMPv2-CONF");
			    }
			}
			NotificationsPart
			STATUS Status
			DESCRIPTION Text
			{
			    checkDescr(thisParserPtr, $9);
			}
			ReferPart
			COLON_COLON_EQUAL '{' objectIdentifier '}'
			{
			    Object *objectPtr;
			    
			    objectPtr = $14;

			    smiCheckObjectReuse(thisParserPtr, $1, &objectPtr);

			    objectPtr = setObjectName(objectPtr, $1, thisParserPtr);
			    setObjectDecl(objectPtr,
					  SMI_DECL_NOTIFICATIONGROUP);
			    setObjectLine(objectPtr, thisParserPtr->firstStatementLine,
					  thisParserPtr);
			    addObjectFlags(objectPtr, FLAG_REGISTERED);
			    deleteObjectFlags(objectPtr, FLAG_INCOMPLETE);
			    setObjectStatus(objectPtr, $7);
			    setObjectDescription(objectPtr, $9, thisParserPtr);
			    if ($11) {
				setObjectReference(objectPtr, $11, thisParserPtr);
			    }
			    setObjectAccess(objectPtr,
					    SMI_ACCESS_NOT_ACCESSIBLE);
			    setObjectList(objectPtr, $5);
			    $$ = 0;
			}
	;

moduleComplianceClause:	LOWERCASE_IDENTIFIER
			{
			    thisParserPtr->firstStatementLine = thisParserPtr->line;
			    thisParserPtr->currentDecl = SMI_DECL_MODULECOMPLIANCE;
			    
			    checkNameLen(thisParserPtr, $1,
					 ERR_OIDNAME_32, ERR_OIDNAME_64);
			    smiCheckObjectName(thisParserPtr,
					       thisModulePtr, $1);
			}
			MODULE_COMPLIANCE
			{
			    Import *importPtr;
			    
			    if (thisModulePtr->export.language == SMI_LANGUAGE_UNKNOWN)
				thisModulePtr->export.language = SMI_LANGUAGE_SMIV2;
			    importPtr = findImportByName("MODULE-COMPLIANCE",
							 thisModulePtr);
			    if (importPtr) {
				importPtr->use++;
			    } else {
				smiPrintError(thisParserPtr,
					      ERR_MACRO_NOT_IMPORTED,
					      "MODULE-COMPLIANCE",
					      "SNMPv2-CONF");
			    }
			}
			STATUS Status
			DESCRIPTION Text
			{
			    checkDescr(thisParserPtr, $8);
			}
			ReferPart
			ComplianceModulePart
			COLON_COLON_EQUAL '{' objectIdentifier '}'
			{
			    Object *objectPtr;
			    Option *optionPtr;
			    Refinement *refinementPtr;
			    List *listPtr;
			    
			    objectPtr = $14;

			    smiCheckObjectReuse(thisParserPtr, $1, &objectPtr);

			    setObjectName(objectPtr, $1, thisParserPtr);
			    setObjectDecl(objectPtr,
					  SMI_DECL_MODULECOMPLIANCE);
			    setObjectLine(objectPtr, thisParserPtr->firstStatementLine,
					  thisParserPtr);
			    addObjectFlags(objectPtr, FLAG_REGISTERED);
			    deleteObjectFlags(objectPtr, FLAG_INCOMPLETE);
			    setObjectStatus(objectPtr, $6);
			    setObjectDescription(objectPtr, $8, thisParserPtr);
			    if ($10) {
				setObjectReference(objectPtr, $10, thisParserPtr);
			    }
			    setObjectAccess(objectPtr,
					    SMI_ACCESS_NOT_ACCESSIBLE);
			    setObjectList(objectPtr, $11.mandatorylistPtr);
			    objectPtr->optionlistPtr = $11.optionlistPtr;
			    objectPtr->refinementlistPtr =
				                          $11.refinementlistPtr;

			    if ($11.optionlistPtr) {
				for (listPtr = $11.optionlistPtr;
				     listPtr;
				     listPtr = listPtr->nextPtr) {
				    optionPtr = ((Option *)(listPtr->ptr));
				    optionPtr->compliancePtr = objectPtr;
				}
			    }

			    /*
			     * Dirty: Fake the types' names in the
			     * refinement list:
			     * ``<compliancename>+<objecttypename>+type''
			     * ``<compliancename>+<objecttypename>+writetype''
			     */
			    if ($11.refinementlistPtr) {
				for (listPtr = $11.refinementlistPtr;
				     listPtr;
				     listPtr = listPtr->nextPtr) {
				    refinementPtr =
					((Refinement *)(listPtr->ptr));
				    refinementPtr->compliancePtr = objectPtr;
				}
			    }

			    $$ = 0;
			}
	;

ComplianceModulePart:	ComplianceModules
			{
			    $$ = $1;
			}
	;

ComplianceModules:	ComplianceModule
			{
			    $$ = $1;
			}
	|		ComplianceModules ComplianceModule
			{
			    List *listPtr;
			    
			    /* concatenate lists in $1 and $2 */
			    if ($1.mandatorylistPtr) {
				for (listPtr = $1.mandatorylistPtr;
				     listPtr->nextPtr;
				     listPtr = listPtr->nextPtr);
				listPtr->nextPtr = $2.mandatorylistPtr;
				$$.mandatorylistPtr = $1.mandatorylistPtr;
			    } else {
				$$.mandatorylistPtr = $2.mandatorylistPtr;
			    }
			    if ($1.optionlistPtr) {
				for (listPtr = $1.optionlistPtr;
				     listPtr->nextPtr;
				     listPtr = listPtr->nextPtr);
				listPtr->nextPtr = $2.optionlistPtr;
				$$.optionlistPtr = $1.optionlistPtr;
			    } else {
				$$.optionlistPtr = $2.optionlistPtr;
			    }
			    if ($1.refinementlistPtr) {
				for (listPtr = $1.refinementlistPtr;
				     listPtr->nextPtr;
				     listPtr = listPtr->nextPtr);
				listPtr->nextPtr = $2.refinementlistPtr;
				$$.refinementlistPtr = $1.refinementlistPtr;
			    } else {
				$$.refinementlistPtr = $2.refinementlistPtr;
			    }
			}
	;

ComplianceModule:	MODULE ComplianceModuleName
			{
			    /*
			     * Remember the module. SMIv2 is broken by
			     * design to allow subsequent clauses to
			     * refer identifiers that are not
			     * imported.  Although, SMIv2 does not
			     * require, we will fake it by inserting
			     * appropriate imports.
			     */
			    if ($2 == thisModulePtr)
				thisParserPtr->complianceModulePtr = NULL;
			    else
				thisParserPtr->complianceModulePtr = $2;
			}
			MandatoryPart
			CompliancePart
			{
			    $$.mandatorylistPtr = $4;
			    $$.optionlistPtr = $5.optionlistPtr;
			    $$.refinementlistPtr = $5.refinementlistPtr;
			    if (thisParserPtr->complianceModulePtr) {
				checkImports(thisParserPtr->complianceModulePtr,
					     thisParserPtr);
				thisParserPtr->complianceModulePtr = NULL;
			    }
			}
	;

ComplianceModuleName:	UPPERCASE_IDENTIFIER objectIdentifier
			{
			    $$ = findModuleByName($1);
			    /* TODO: handle objectIdentifier */
			    if (!$$) {
				$$ = loadModule($1, thisParserPtr);
			    }
			    smiFree($1);
			}
	|		UPPERCASE_IDENTIFIER
			{
			    $$ = findModuleByName($1);
			    if (!$$) {
				$$ = loadModule($1, thisParserPtr);
			    }
			    smiFree($1);
			}
	|		/* empty, only if contained in MIB module */
			/* TODO: RFC 1904 looks a bit different, is this ok? */
			{
			    $$ = thisModulePtr;
			}
	;

MandatoryPart:		MANDATORY_GROUPS '{' MandatoryGroups '}'
			{
			    $$ = $3;
			}
	|		/* empty */
			{
			    $$ = NULL;
			}
	;

MandatoryGroups:	MandatoryGroup
			{
			    $$ = smiMalloc(sizeof(List));
			    $$->ptr = $1;
			    $$->nextPtr = NULL;
			}
	|		MandatoryGroups ',' MandatoryGroup
			{
			    List *p, *pp;
			    
			    p = smiMalloc(sizeof(List));
			    p->ptr = $3;
			    p->nextPtr = NULL;
			    for (pp = $1; pp->nextPtr; pp = pp->nextPtr);
			    pp->nextPtr = p;
			    $$ = $1;
			}
	;

MandatoryGroup:		objectIdentifier
			{
			    /* TODO: check that objectIdentifier is
			       found, is defined in thisParserPtr->complianceModulePtr,
			       and is a group node. */
			    Import *importPtr;

			    $$ = $1;
			    if (thisParserPtr->complianceModulePtr) {
				$$ = findObjectByModuleAndName(
				                           thisParserPtr->complianceModulePtr,
							   $1->export.name);
			    }
			    if (thisParserPtr->complianceModulePtr && $1->export.name) {
				importPtr = findImportByModulenameAndName(
				    thisParserPtr->complianceModulePtr->export.name,
				    $1->export.name, thisModulePtr);
				if (importPtr)
				    importPtr->use++;
			    }
			}
	;

CompliancePart:		Compliances
			{
			    $$.mandatorylistPtr = NULL;
			    $$.optionlistPtr = $1.optionlistPtr;
			    $$.refinementlistPtr = $1.refinementlistPtr;
			}
	|		/* empty */
			{
			    $$.mandatorylistPtr = NULL;
			    $$.optionlistPtr = NULL;
			    $$.refinementlistPtr = NULL;
			}
	;

Compliances:		Compliance
			{
			    $$ = $1;
			}
	|		Compliances Compliance
			{
			    List *listPtr;
			    int stop;
			    
			    $$.mandatorylistPtr = NULL;

			    /* check for duplicates in optionlist */
			    stop = 0;
			    if ($2.optionlistPtr) {
				for (listPtr = $1.optionlistPtr; listPtr;
				     listPtr = listPtr->nextPtr) {
				    if (((Option *)listPtr->ptr)->objectPtr ==
					((Option *)$2.optionlistPtr->ptr)->objectPtr) {
					smiPrintError(thisParserPtr,
						      ERR_OPTIONALGROUP_ALREADY_EXISTS,
						      ((Option *)$2.optionlistPtr->ptr)->objectPtr->export.name);
					stop = 1;
					$$.optionlistPtr = $1.optionlistPtr;
				    }
				}
			    }
	
                            /* concatenate optionlists */
			    if ($1.optionlistPtr) {
				for (listPtr = $1.optionlistPtr;
				     listPtr->nextPtr;
				     listPtr = listPtr->nextPtr);
				if (!stop) {
				    listPtr->nextPtr = $2.optionlistPtr;
				}
				$$.optionlistPtr = $1.optionlistPtr;
			    } else {
				$$.optionlistPtr = $2.optionlistPtr;
			    }

			    /* check for duplicates in refinementlist */
			    stop = 0;
			    if ($2.refinementlistPtr) {
				for (listPtr = $1.refinementlistPtr; listPtr;
				     listPtr = listPtr->nextPtr) {
				    if (((Refinement *)listPtr->ptr)->objectPtr ==
					((Refinement *)$2.refinementlistPtr->ptr)->objectPtr) {
					smiPrintError(thisParserPtr,
						      ERR_REFINEMENT_ALREADY_EXISTS,
						      ((Refinement *)$2.refinementlistPtr->ptr)->objectPtr->export.name);
					stop = 1;
					$$.refinementlistPtr = $1.refinementlistPtr;
				    }
				}
			    }
			    
                            /* concatenate refinementlists */
			    if ($1.refinementlistPtr) {
				for (listPtr = $1.refinementlistPtr;
				     listPtr->nextPtr;
				     listPtr = listPtr->nextPtr);
				if (!stop) {
				    listPtr->nextPtr = $2.refinementlistPtr;
				}
				$$.refinementlistPtr = $1.refinementlistPtr;
			    } else {
				$$.refinementlistPtr = $2.refinementlistPtr;
			    }
			}
	;

Compliance:		ComplianceGroup
			{
			    $$.mandatorylistPtr = NULL;
			    $$.optionlistPtr = $1;
			    $$.refinementlistPtr = NULL;
			}
	|		ComplianceObject
			{
			    $$.mandatorylistPtr = NULL;
			    $$.optionlistPtr = NULL;
			    $$.refinementlistPtr = $1;
			}
	;

ComplianceGroup:	GROUP
			{
			    thisParserPtr->firstNestedStatementLine = thisParserPtr->line;
			}
			objectIdentifier
			DESCRIPTION Text
			{
			    Import *importPtr;
			    
			    if (thisParserPtr->complianceModulePtr && $3->export.name) {
				importPtr = findImportByModulenameAndName(
						    thisParserPtr->complianceModulePtr->export.name,
						    $3->export.name,
						    thisModulePtr);
				if (importPtr)
				    importPtr->use++;
			    }

			    checkDescr(thisParserPtr, $5);
			    
			    $$ = smiMalloc(sizeof(List));
			    $$->nextPtr = NULL;
			    $$->ptr = smiMalloc(sizeof(Option));
			    ((Option *)($$->ptr))->line = thisParserPtr->firstNestedStatementLine;
			    ((Option *)($$->ptr))->objectPtr = $3;
			    if (! (thisModulePtr->flags & SMI_FLAG_NODESCR)) {
				((Option *)($$->ptr))->export.description = $5;
			    } else {
				smiFree($5);
			    }
			}
	;

ComplianceObject:	OBJECT
			{
			    thisParserPtr->firstNestedStatementLine = thisParserPtr->line;
			}
			ObjectName
			SyntaxPart
			WriteSyntaxPart                 /* modified for SPPI */
			AccessPart                      /* modified for SPPI */
			DESCRIPTION Text
			{
			    Import *importPtr;

			    if (thisParserPtr->complianceModulePtr && $3->export.name) {
				importPtr = findImportByModulenameAndName(
						    thisParserPtr->complianceModulePtr->export.name,
						    $3->export.name,
						    thisModulePtr);
				if (importPtr) 
				    importPtr->use++;
			    }

			    checkDescr(thisParserPtr, $8);
			    
			    thisParserPtr->flags &= ~FLAG_CREATABLE;
			    $$ = smiMalloc(sizeof(List));
			    $$->nextPtr = NULL;
			    $$->ptr = smiMalloc(sizeof(Refinement));
			    ((Refinement *)($$->ptr))->line =
				thisParserPtr->firstNestedStatementLine;
			    ((Refinement *)($$->ptr))->objectPtr = $3;
			    ((Refinement *)($$->ptr))->typePtr = $4;
			    if ($4) {
				$4->parentPtr = $3->typePtr;
			    }
			    ((Refinement *)($$->ptr))->writetypePtr = $5;
			    if ($5) {
				$5->parentPtr = $3->typePtr;
			    }
			    ((Refinement *)($$->ptr))->export.access = $6;
			    if (! (thisParserPtr->flags & SMI_FLAG_NODESCR)) {
				((Refinement *)($$->ptr))->export.description = $8;
			    } else {
				smiFree($8);
			    }
			}
	;

SyntaxPart:		SYNTAX Syntax
			{
			    if ($2->export.name) {
				$$ = duplicateType($2, 0, thisParserPtr);
			    } else {
				$$ = $2;
			    }
			}
	|		/* empty */
			{
			    $$ = NULL;
			}
	;

WriteSyntaxPart:	WRITE_SYNTAX WriteSyntax
			{
                            /* must not be present in PIBs */
                            if (thisParserPtr->modulePtr->export.language == SMI_LANGUAGE_SPPI)
                                smiPrintError(thisParserPtr, ERR_SMI_CONSTRUCT_IN_PIB, "WRITE-SYNTAX");
			    if ($2->export.name) {
				$$ = duplicateType($2, 0, thisParserPtr);
			    } else {
				$$ = $2;
			    }
			}
	|		/* empty */
			{
			    $$ = NULL;
			}
	;

WriteSyntax:		Syntax
			{
			    $$ = $1;
			}
	;

AccessPart:		MIN_ACCESS Access
			{
                            if (thisParserPtr->modulePtr->export.language == SMI_LANGUAGE_SPPI)
                                smiPrintError(thisParserPtr, ERR_SMI_CONSTRUCT_IN_PIB, "MIN-ACCESS");
			    $$ = $2;
			}
        |               PIB_MIN_ACCESS Access
                        {
                            if (thisParserPtr->modulePtr->export.language != SMI_LANGUAGE_SPPI)
                                smiPrintError(thisParserPtr, ERR_SPPI_CONSTRUCT_IN_MIB, "PIB-MIN-ACCESS");
                            if ($2 == SMI_ACCESS_REPORT_ONLY)
                                smiPrintError(thisParserPtr, ERR_REPORT_ONLY_IN_PIB_MIN_ACCESS);
                            $$ = $2;
                        }
	|		/* empty */
			{
			    $$ = SMI_ACCESS_UNKNOWN;
			}
	;

agentCapabilitiesClause: LOWERCASE_IDENTIFIER
			{
			    thisParserPtr->firstStatementLine = thisParserPtr->line;
			    thisParserPtr->currentDecl = SMI_DECL_AGENTCAPABILITIES;
			    
			    checkNameLen(thisParserPtr, $1,
					 ERR_OIDNAME_32, ERR_OIDNAME_64);
			    smiCheckObjectName(thisParserPtr,
					       thisModulePtr, $1);
			}
			AGENT_CAPABILITIES
			{
			    Import *importPtr;
                            if (thisParserPtr->modulePtr->export.language == SMI_LANGUAGE_SPPI)
                                smiPrintError(thisParserPtr, ERR_SMI_CONSTRUCT_IN_PIB, "AGENT-CAAPABILITIES");
			    
			    if (thisModulePtr->export.language == SMI_LANGUAGE_UNKNOWN)
				thisModulePtr->export.language = SMI_LANGUAGE_SMIV2;

			    importPtr = findImportByName("AGENT-CAPABILITIES",
							 thisModulePtr);
			    if (importPtr) {
				importPtr->use++;
			    } else {
				smiPrintError(thisParserPtr,
					      ERR_MACRO_NOT_IMPORTED,
					      "AGENT-CAPABILITIES",
					      "SNMPv2-CONF");
			    }
			}
			PRODUCT_RELEASE Text
			STATUS Status_Capabilities
			DESCRIPTION Text
			{
			    checkDescr(thisParserPtr, $10);
			}
			ReferPart
			ModulePart_Capabilities
			COLON_COLON_EQUAL '{' objectIdentifier '}'
			{
			    Object *objectPtr;
			    
			    objectPtr = $16;
			    
			    smiCheckObjectReuse(thisParserPtr, $1, &objectPtr);

			    setObjectName(objectPtr, $1, thisParserPtr);
			    setObjectDecl(objectPtr,
					  SMI_DECL_AGENTCAPABILITIES);
			    setObjectLine(objectPtr, thisParserPtr->firstStatementLine,
					  thisParserPtr);
			    addObjectFlags(objectPtr, FLAG_REGISTERED);
			    deleteObjectFlags(objectPtr, FLAG_INCOMPLETE);
			    setObjectStatus(objectPtr, $8);
			    setObjectDescription(objectPtr, $10, thisParserPtr);
			    if ($12) {
				setObjectReference(objectPtr, $12, thisParserPtr);
			    }
			    setObjectAccess(objectPtr,
					    SMI_ACCESS_NOT_ACCESSIBLE);
				/*
				 * TODO: PRODUCT_RELEASE Text
				 * TODO: ModulePart_Capabilities
				 */
			    $$ = 0;
			}
	;

ModulePart_Capabilities: Modules_Capabilities
			{ $$ = 0; }
	|		/* empty */
			{ $$ = 0; }
	;

Modules_Capabilities:	Module_Capabilities
			{ $$ = 0; }
	|		Modules_Capabilities Module_Capabilities
			{ $$ = 0; }
	;

Module_Capabilities:	SUPPORTS ModuleName_Capabilities
			{
			    /*
			     * Remember the module. SMIv2 is broken by
			     * design to allow subsequent clauses to
			     * refer identifiers that are not
			     * imported.  Although, SMIv2 does not
			     * require, we will fake it by inserting
			     * appropriate imports.
			     */
			    if ($2 == thisModulePtr)
				thisParserPtr->capabilitiesModulePtr = NULL;
			    else
				thisParserPtr->capabilitiesModulePtr = $2;
			}
			INCLUDES '{' CapabilitiesGroups '}'
			VariationPart
			{
			    if (thisParserPtr->capabilitiesModulePtr) {
				checkImports(thisParserPtr->capabilitiesModulePtr,
					     thisParserPtr);
				thisParserPtr->capabilitiesModulePtr = NULL;
			    }
			    $$ = 0;
			}
	;

CapabilitiesGroups:	CapabilitiesGroup
			{
#if 0
			    $$ = smiMalloc(sizeof(List));
			    $$->ptr = $1;
			    $$->nextPtr = NULL;
#else
			    $$ = NULL;
#endif
			}
	|		CapabilitiesGroups ',' CapabilitiesGroup
			{
#if 0
			    List *p, *pp;
			    
			    p = smiMalloc(sizeof(List));
			    p->ptr = $3;
			    p->nextPtr = NULL;
			    for (pp = $1; pp->nextPtr; pp = pp->nextPtr);
			    pp->nextPtr = p;
			    $$ = $1;
#else
			    $$ = NULL;
#endif
			}
	;

CapabilitiesGroup:	objectIdentifier
			{
			    $$ = NULL;
			}
	;

ModuleName_Capabilities: UPPERCASE_IDENTIFIER objectIdentifier
			{
			    $$ = findModuleByName($1);
			    /* TODO: handle objectIdentifier */
			    if (!$$) {
				$$ = loadModule($1, thisParserPtr);
			    }
			    smiFree($1);
			}
	|		UPPERCASE_IDENTIFIER
			{
			    $$ = findModuleByName($1);
			    if (!$$) {
				$$ = loadModule($1, thisParserPtr);
			    }
			    smiFree($1);
			}
	;

VariationPart:		Variations
			{ $$ = 0; }
	|		/* empty */
			{ $$ = 0; }
	;

Variations:		Variation
			{ $$ = 0; }
	|		Variations Variation
			{ $$ = 0; }
	;

Variation:		VARIATION ObjectName
			{
			    if ($2) {
				variationkind = $2->export.nodekind;
			    } else {
				variationkind = SMI_NODEKIND_UNKNOWN;
			    }
			}
			SyntaxPart
			{
			    if (variationkind == SMI_NODEKIND_NOTIFICATION) {
				smiPrintError(thisParserPtr,
				      ERR_NOTIFICATION_VARIATION_SYNTAX);
			    }
			}
			WriteSyntaxPart
			{
			    if (variationkind == SMI_NODEKIND_NOTIFICATION) {
				smiPrintError(thisParserPtr,
				      ERR_NOTIFICATION_VARIATION_WRITESYNTAX);
			    }
			}
			VariationAccessPart
			CreationPart
			{
			    if (variationkind == SMI_NODEKIND_NOTIFICATION) {
				smiPrintError(thisParserPtr,
				      ERR_NOTIFICATION_VARIATION_CREATION);
			    }
			}
			DefValPart
			{
			    if (variationkind == SMI_NODEKIND_NOTIFICATION) {
				smiPrintError(thisParserPtr,
				      ERR_NOTIFICATION_VARIATION_DEFVAL);
			    } else if ($11) {
				adjustDefval(thisParserPtr,
					     $11, $2->typePtr,
					     thisParserPtr->line);
				smiCheckValueType(thisParserPtr,
						  $11, $2->typePtr,
						  thisParserPtr->line);
			    }
			}
			DESCRIPTION Text
			{
			    thisParserPtr->flags &= ~FLAG_CREATABLE;
			    $$ = 0;
			    variationkind = SMI_NODEKIND_UNKNOWN;

			    checkDescr(thisParserPtr, $14);
			}
	;

VariationAccessPart:	ACCESS VariationAccess
			{ $$ = $2; }
	|		/* empty */
			{ $$ = 0; }
	;

VariationAccess:	LOWERCASE_IDENTIFIER
			{
			    if (!strcmp($1, "not-implemented")) {
				$$ = SMI_ACCESS_NOT_IMPLEMENTED;
			    } else if (!strcmp($1, "accessible-for-notify")) {
				if (variationkind ==
				    SMI_NODEKIND_NOTIFICATION) {
				    smiPrintError(thisParserPtr,
				     ERR_INVALID_NOTIFICATION_VARIATION_ACCESS,
					       $1);
				    $$ = SMI_ACCESS_UNKNOWN;
				} else {
				    $$ = SMI_ACCESS_NOTIFY;
				}
			    } else if (!strcmp($1, "read-only")) {
				if (variationkind ==
				    SMI_NODEKIND_NOTIFICATION) {
				    smiPrintError(thisParserPtr,
				     ERR_INVALID_NOTIFICATION_VARIATION_ACCESS,
					       $1);
				    $$ = SMI_ACCESS_UNKNOWN;
				} else {
				    $$ = SMI_ACCESS_READ_ONLY;
				}
			    } else if (!strcmp($1, "read-write")) {
				if (variationkind ==
				    SMI_NODEKIND_NOTIFICATION) {
				    smiPrintError(thisParserPtr,
				     ERR_INVALID_NOTIFICATION_VARIATION_ACCESS,
					       $1);
				    $$ = SMI_ACCESS_UNKNOWN;
				} else {
				    $$ = SMI_ACCESS_READ_WRITE;
				}
			    } else if (!strcmp($1, "read-create")) {
				if (variationkind ==
				    SMI_NODEKIND_NOTIFICATION) {
				    smiPrintError(thisParserPtr,
				     ERR_INVALID_NOTIFICATION_VARIATION_ACCESS,
					       $1);
				    $$ = SMI_ACCESS_UNKNOWN;
				} else {
				    $$ = SMI_ACCESS_READ_WRITE;
				}
			    } else if (!strcmp($1, "write-only")) {
				if (variationkind ==
				    SMI_NODEKIND_NOTIFICATION) {
				    smiPrintError(thisParserPtr,
				     ERR_INVALID_NOTIFICATION_VARIATION_ACCESS,
					       $1);
				    $$ = SMI_ACCESS_UNKNOWN;
				} else {
				    $$ = SMI_ACCESS_READ_WRITE; /* TODO */
				    smiPrintError(thisParserPtr,
					       ERR_SMIV2_WRITE_ONLY);
				}
			    } else {
				smiPrintError(thisParserPtr,
					      ERR_INVALID_VARIATION_ACCESS,
					      $1);
				$$ = SMI_ACCESS_UNKNOWN;
			    }
			}
        ;

CreationPart:		CREATION_REQUIRES '{' Cells '}'
			{ $$ = 0; }
	|		/* empty */
			{ $$ = 0; }
	;

Cells:			Cell
			{ $$ = 0; }
	|		Cells ',' Cell
			{ $$ = 0; }
	;

Cell:			ObjectName
			{ $$ = 0; }
	;

%%

#endif
