/*
 * yang.h --
 *
 *      Interface Definition of YANG (version 2:27:0).
 *
 * Copyright (c) 1999,2000 Frank Strauss, Technical University of Braunschweig.
 *
 * See the file "COPYING" for information on usage and redistribution
 * of this file, and for a DISCLAIMER OF ALL WARRANTIES.
 *
 * Author: Kaloyan Kanev, Siarhei Kuryla
 * @(#) $Id: data.c 12198 2009-01-05 08:37:07Z schoenw $
 */

#ifndef _YANG_H
#define _YANG_H

#include <stdlib.h>
#include <stdarg.h>
#ifdef HAVE_STDINT_H
#include <stdint.h>
#endif
#ifdef HAVE_LIMITS_H
#include "limits.h"
#endif
#include <time.h>

#include <smi.h>


#ifdef __cplusplus
extern "C" {
#endif


/* YangDecl -- type or statement that leads to a definition                  */
typedef enum YangDecl {
    YANG_DECL_UNKNOWN                   = 0,  /* should not occur            */
    YANG_DECL_MODULE                    = 1,
    YANG_DECL_SUBMODULE                 = 2,
    YANG_DECL_REVISION                  = 3,
    YANG_DECL_IMPORT                    = 4,
    YANG_DECL_RANGE                     = 5,
    YANG_DECL_PATTERN                   = 6,
    YANG_DECL_CONTAINER                 = 7,
    YANG_DECL_MUST                      = 8,
    YANG_DECL_LEAF                      = 9,
    YANG_DECL_LEAF_LIST                 = 10,
    YANG_DECL_LIST                      = 11,
    YANG_DECL_CASE                      = 12,
    YANG_DECL_USES                      = 13,
    YANG_DECL_AUGMENT                   = 14,
    YANG_DECL_GROUPING                  = 15,
    YANG_DECL_CHOICE                    = 16,
    YANG_DECL_ARGUMENT                  = 17,
    YANG_DECL_RPC                       = 18,
    YANG_DECL_INPUT                     = 19,
    YANG_DECL_OUTPUT                    = 20,
    YANG_DECL_ANYXML                    = 21,
    YANG_DECL_INCLUDE                   = 22,
    YANG_DECL_ORGANIZATION              = 23, 
    YANG_DECL_CONTACT                   = 24, 
    YANG_DECL_NAMESPACE                 = 25, 
    YANG_DECL_YANG_VERSION              = 26, 
    YANG_DECL_PREFIX                    = 27, 
    YANG_DECL_TYPEDEF                   = 28, 
    YANG_DECL_PATH                      = 29, 
    YANG_DECL_LENGTH                    = 30, 
    YANG_DECL_TYPE                      = 31, 
    YANG_DECL_ERROR_MESSAGE             = 32, 
    YANG_DECL_ERROR_APP_TAG             = 33, 
    YANG_DECL_MANDATORY                 = 34, 
    YANG_DECL_NOTIFICATION              = 35, 
    YANG_DECL_EXTENSION                 = 36, 
    YANG_DECL_BELONGS_TO                = 37, 
    YANG_DECL_YIN_ELEMENT               = 38, 
    YANG_DECL_UNKNOWN_STATEMENT         = 39,  /* user-defined extensions */
    YANG_DECL_DESCRIPTION               = 40,
    YANG_DECL_REFERENCE                 = 41,
    YANG_DECL_STATUS                    = 42,
    YANG_DECL_CONFIG                    = 43,
    YANG_DECL_ENUM                      = 44,
    YANG_DECL_VALUE                     = 45,
    YANG_DECL_REQUIRE_INSTANCE          = 46,
    YANG_DECL_BASE                      = 47,
    YANG_DECL_BIT                       = 48,
    YANG_DECL_POSITION                  = 49,
    YANG_DECL_UNITS                     = 50,
    YANG_DECL_DEFAULT                   = 51,
    YANG_DECL_FEATURE                   = 52,
    YANG_DECL_IF_FEATURE                = 53,
    YANG_DECL_IDENTITY                  = 54,
    YANG_DECL_PRESENCE                  = 55,
    YANG_DECL_WHEN                      = 56,
    YANG_DECL_MIN_ELEMENTS              = 57,
    YANG_DECL_MAX_ELEMENTS              = 58,
    YANG_DECL_ORDERED_BY                = 59,
    YANG_DECL_KEY                       = 60,
    YANG_DECL_UNIQUE                    = 61,
    YANG_DECL_REFINE                    = 62,
    YANG_DECL_DEVIATION                 = 63,
    YANG_DECL_DEVIATE                   = 64,
    YANG_DECL_FRACTION_DIGITS           = 65,
    YANG_DECL_COMPLEX_TYPE              = 66,	/* YANG CT EXTENSION */
    YANG_DECL_ABSTRACT                  = 67,	/* YANG CT EXTENSION */
    YANG_DECL_EXTENDS                   = 68,	/* YANG CT EXTENSION */
    YANG_DECL_INSTANCE                  = 69,	/* YANG CT EXTENSION */
    YANG_DECL_INSTANCE_LIST             = 70,	/* YANG CT EXTENSION */
    YANG_DECL_INSTANCE_TYPE             = 71,	/* YANG CT EXTENSION */
    YANG_DECL_SMI_OID			= 73,	/* YANG SMI EXTENSION */
    YANG_DECL_SMI_DISPLAY_HINT		= 74,	/* YANG SMI EXTENSION */
    YANG_DECL_SMI_DEFAULT		= 75,	/* YANG SMI EXTENSION */
    YANG_DECL_SMI_MAX_ACCESS		= 76,	/* YANG SMI EXTENSION */
    YANG_DECL_SMI_ALIAS			= 77,	/* YANG SMI EXTENSION */
    YANG_DECL_SMI_IMPLIED               = 78,	/* YANG SMI EXTENSION */
    YANG_DECL_LAST				/* end marker - must be last */
} YangDecl;


extern char* yangDeclAsString(YangDecl decl);

typedef char    *YangString;

/* the truth value of Yang config statement */
typedef enum YangConfig {
     YANG_CONFIG_DEFAULT                = 1,    
     YANG_CONFIG_FALSE                  = 2, 
     YANG_CONFIG_TRUE                   = 3
} YangConfig;

typedef enum YangStatus {
    YANG_STATUS_DEFAULT_CURRENT  = 0,
    YANG_STATUS_CURRENT          = 1,
    YANG_STATUS_DEPRECATED       = 2,
    YANG_STATUS_OBSOLETE         = 5  /* for compatibility with SMI */
} YangStatus;

extern char *yangStatusAsString(YangStatus status);

/* the truth value of Yang config statement */
typedef enum YangBoolean {
     YANG_BOOLEAN_FALSE                  = 2,
     YANG_BOOLEAN_TRUE                   = 3
} YangBoolean;


typedef struct YangNode {
    YangString      value;	/* 'name' or 'argument' of the statement */
    YangString      extra;      /* extra data, used to store some additional
				   information, e.g., for the 'unknown'
				   statement */
    YangDecl        nodeKind;
    YangStatus      status;
    YangConfig      config;
    char            *description;
    char            *reference;
} YangNode;

/*
 * YANG API
 */
/*
 *  returns 'true' if the module with the modulename is a YANG module
 */
extern int yangIsModule(const char* modulename);

/*
 * returns a pointer to the struct YangNode for the module with the given name
 * or NULL if the YANG module with the given has not been loaded
 */
extern YangNode *yangGetModule(char *modulename);

/*
 * returns a pointer to the struct YangNode that represents the first child node of the yangNodePtr node
 * or NULL if that node does have child nodes.
 */
extern YangNode *yangGetFirstChildNode(YangNode *yangNodePtr);

/*
 * returns a pointer to the struct YangNode that represents the next sibling node of the yangNodePtr node
 * or NULL if that node does have child nodes.
 */
extern YangNode *yangGetNextChildNode(YangNode *yangNodePtr);

/*
 * returns a pointer to the struct YangNode that represents the first loaded YANG module
 * or NULL if no one YANG module has been loaded.
 */
extern YangNode *yangGetFirstModule(void);

/*
 * returns a pointer to the struct YangNode that represents the YANG module
 * loaded after the module yangModulePtr or NULL there is no module after it.
 */
extern YangNode *yangGetNextModule(YangNode *yangModulePtr);

int yangIsTrueConf(YangConfig conf);

#ifdef __cplusplus
}
#endif

#endif /* _YANG_H */
