/*
 * yang.c --
 *
 *      Interface Implementation of YANG.
 *
 * Copyright (c) 1999 Frank Strauss, Technical University of Braunschweig.
 *
 * See the file "COPYING" for information on usage and redistribution
 * of this file, and for a DISCLAIMER OF ALL WARRANTIES.
 *
 * Author: Kaloyan Kanev, Siarhei Kuryla
 * @(#) $Id: data.c 12198 2009-01-05 08:37:07Z schoenw $
 */

#include <config.h>

#ifdef BACKEND_YANG

#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <ctype.h>
#include <sys/types.h>
#include <limits.h>
#ifdef HAVE_UNISTD_H
#include <unistd.h>
#endif
#ifdef HAVE_PWD_H
#include <pwd.h>
#endif

#define ONLY_ORIGINAL 1

#include "common.h"
#include "yang.h"

/*
 * YANG API
 */

/*
 * Stringification functions. We are using verbose switch statements
 * since this allows the compiler to check completeness and we like
 * to handle invalid values gracefully.
 */

char *yangDeclAsString(YangDecl decl)
{
    char *s = "<DECL-UNDEFINED>";

    switch (decl) {
    case YANG_DECL_UNKNOWN:
	s = "<unknown>";
	break;
    case YANG_DECL_MODULE:
	s = "module";
	break;
    case YANG_DECL_SUBMODULE:
	s = "submodule";
	break;
    case YANG_DECL_REVISION:
	s = "revision";
	break;
    case YANG_DECL_IMPORT:
	s = "import";
	break;
    case YANG_DECL_RANGE:
	s = "range";
	break;
    case YANG_DECL_PATTERN:
	s = "pattern";
	break;
    case YANG_DECL_CONTAINER:
	s = "container";
	break;
    case YANG_DECL_MUST:
	s = "must";
	break;
    case YANG_DECL_LEAF:
	s = "leaf";
	break;
    case YANG_DECL_LEAF_LIST:
	s = "leaf-list";
	break;
    case YANG_DECL_LIST:
	s = "list";
	break;
    case YANG_DECL_CASE:
	s = "case";
	break;
    case YANG_DECL_USES:
	s = "uses";
	break;
    case YANG_DECL_AUGMENT:
	s = "augment";
	break;
    case YANG_DECL_GROUPING:
	s = "grouping";
	break;
    case YANG_DECL_CHOICE:
	s = "choice";
	break;
    case YANG_DECL_ARGUMENT:
	s = "argument";
	break;
    case YANG_DECL_RPC:
	s = "rpc";
	break;
    case YANG_DECL_INPUT:
	s = "input";
	break;
    case YANG_DECL_OUTPUT:
	s = "output";
	break;
    case YANG_DECL_ANYXML:
	s = "anyxml";
	break;
    case YANG_DECL_INCLUDE:
	s = "include";
	break;
    case YANG_DECL_ORGANIZATION:
	s = "organization";
	break; 
    case YANG_DECL_CONTACT:
	s = "contact";
	break; 
    case YANG_DECL_NAMESPACE:
	s = "namespace";
	break; 
    case YANG_DECL_YANG_VERSION:
	s = "yang-version";
	break; 
    case YANG_DECL_PREFIX:
	s = "prefix";
	break; 
    case YANG_DECL_TYPEDEF:
	s = "typedef";
	break; 
    case YANG_DECL_PATH:
	s = "path";
	break; 
    case YANG_DECL_LENGTH:
	s = "length";
	break; 
    case YANG_DECL_TYPE:
	s = "type";
	break; 
    case YANG_DECL_ERROR_MESSAGE:
	s = "error-message";
	break; 
    case YANG_DECL_ERROR_APP_TAG:
	s = "error-app-tag";
	break; 
    case YANG_DECL_MANDATORY:
	s = "mandatory";
	break; 
    case YANG_DECL_NOTIFICATION:
	s = "notification";
	break; 
    case YANG_DECL_EXTENSION:
	s = "extension";
	break; 
    case YANG_DECL_BELONGS_TO:
	s = "belongs-to";
	break; 
    case YANG_DECL_YIN_ELEMENT:
	s = "yin-element";
	break; 
    case YANG_DECL_UNKNOWN_STATEMENT:
	s = "unknown-statement";
	break;
    case YANG_DECL_DESCRIPTION:
	s = "description";
	break;
    case YANG_DECL_REFERENCE:
	s = "reference";
	break;
    case YANG_DECL_STATUS:
	s = "status";
	break;
    case YANG_DECL_CONFIG:
	s = "config";
	break;
    case YANG_DECL_ENUM:
	s = "enum";
	break;
    case YANG_DECL_VALUE:
	s = "value";
	break;
    case YANG_DECL_REQUIRE_INSTANCE:
	s = "require-instance";
	break;
    case YANG_DECL_BASE:
	s = "base";
	break;
    case YANG_DECL_BIT:
	s = "bit";
	break;
    case YANG_DECL_POSITION:
	s = "position";
	break;
    case YANG_DECL_UNITS:
	s = "units";
	break;
    case YANG_DECL_DEFAULT:
	s = "default";
	break;
    case YANG_DECL_FEATURE:
	s = "feature";
	break;
    case YANG_DECL_IF_FEATURE:
	s = "if-feature";
	break;
    case YANG_DECL_IDENTITY:
	s = "identity";
	break;
    case YANG_DECL_PRESENCE:
	s = "presence";
	break;
    case YANG_DECL_WHEN:
	s = "when";
	break;
    case YANG_DECL_MIN_ELEMENTS:
	s = "min-elements";
	break;
    case YANG_DECL_MAX_ELEMENTS:
	s = "max-elements";
	break;
    case YANG_DECL_ORDERED_BY:
	s = "ordered-by";
	break;
    case YANG_DECL_KEY:
	s = "key";
	break;
    case YANG_DECL_UNIQUE:
	s = "unique";
	break;
    case YANG_DECL_REFINE:
	s = "refine";
	break;
    case YANG_DECL_DEVIATION:
	s = "deviation";
	break;
    case YANG_DECL_DEVIATE:
	s = "deviate";
	break;
    case YANG_DECL_COMPLEX_TYPE:
	s = "complex-type";
	break;
    case YANG_DECL_ABSTRACT:
	s = "abstract";
	break;
    case YANG_DECL_EXTENDS:
	s = "extends";
	break;
    case YANG_DECL_INSTANCE:
	s = "instance";
	break;
    case YANG_DECL_INSTANCE_LIST:
	s = "instance-list";
	break;
    case YANG_DECL_INSTANCE_TYPE:
	s = "instance-type";
	break;
    case YANG_DECL_FRACTION_DIGITS:
	s = "fraction-digits";
	break;
    case YANG_DECL_SMI_OID:
	s = "smiv2:oid";
	break;
    case YANG_DECL_SMI_DISPLAY_HINT:
	s = "smiv2:display-hint";
	break;
    case YANG_DECL_SMI_DEFAULT:
	s = "smiv2:defval";
	break;
    case YANG_DECL_SMI_MAX_ACCESS:
	s = "smiv2:max-access";
	break;
    case YANG_DECL_SMI_ALIAS:
	s = "smiv2:alias";
	break;
    case YANG_DECL_SMI_IMPLIED:
	s = "smiv2:implied";
	break;
    }
    return s;
}

char *yangStatusAsString(YangStatus status)
{
    char *s = "<STATUS-UNDEFINED>";

    switch (status) {
    case YANG_STATUS_DEFAULT_CURRENT:
	s = "<default-current>";
	break;
    case YANG_STATUS_CURRENT:
	s = "current";
	break;
    case YANG_STATUS_DEPRECATED:
	s = "deprecated";
	break;
    case YANG_STATUS_OBSOLETE:
	s = "obsolete";
	break;
    }
    return s;
}

/*
 * Determines whether a module with a certain name is a YANG module
 */
int yangIsModule(const char* modulename) {
    _YangNode   *modulePtr;
    modulePtr = findYangModuleByName(modulename, NULL);
    
    if (modulePtr) return 0;

    return 1;    
}

YangNode* yangGetModule(char *modulename) {
    if (ONLY_ORIGINAL) {
        return &((getModuleInfo(findYangModuleByName(modulename, NULL))->originalModule)->export);
    } else {
        return &(findYangModuleByName(modulename, NULL)->export);
    }
}

YangNode *yangGetFirstChildNode(YangNode *yangNodePtr) {
    _YangNode *nodePtr = (_YangNode *)yangNodePtr;
    if (!nodePtr) return NULL;
    nodePtr = nodePtr->firstChildPtr;

    if (!nodePtr) {
        return NULL;
    } else {
        return &(nodePtr->export);
    }
}

YangNode *yangGetNextChildNode(YangNode *yangNodePtr) {
    _YangNode *nodePtr = (_YangNode *)yangNodePtr;
    if (!nodePtr) return NULL;
    nodePtr = nodePtr->nextSiblingPtr;
    if (!nodePtr) {
        return NULL;
    } else {
        return &(nodePtr->export);
    }
}

YangNode *yangGetFirstModule(void) {
    if (ONLY_ORIGINAL) {
        return &((getModuleInfo(smiHandle->firstYangModulePtr)->originalModule)->export);
    } else {
        return &(smiHandle->firstYangModulePtr->export);
    }
}

YangNode *yangGetNextModule(YangNode *yangModulePtr) {
    _YangNode *modulePtr = (_YangNode *)yangModulePtr;
    if (ONLY_ORIGINAL) {
        _YangNode* cur = smiHandle->firstYangModulePtr;
        while (cur) {
            if (getModuleInfo(cur)->originalModule == modulePtr) break;
            cur = cur->nextSiblingPtr;
        }    

        if (cur && cur->nextSiblingPtr) {
            return &(getModuleInfo(cur->nextSiblingPtr)->originalModule->export);    
        } else {
            return NULL;
        }
    } else {
        if (modulePtr && modulePtr->nextSiblingPtr) {
            return &(modulePtr->nextSiblingPtr->export);    
        } else {
            return NULL;
        }        
    }
}

int yangIsTrueConf(YangConfig conf) {
    return conf == YANG_CONFIG_TRUE;
}

#endif

