/*
 * yang-data.h --
 *
 *      Definitions for the main YANG data structures.
 *
 * Copyright (c) 1999 Frank Strauss, Technical University of Braunschweig.
 *
 * See the file "COPYING" for information on usage and redistribution
 * of this file, and for a DISCLAIMER OF ALL WARRANTIES.
 *
 * Authors: Kaloyan Kanev, Siarhei Kuryla
 * @(#) $Id: data.h 7966 2008-03-27 21:25:52Z schoenw $
 */

#ifndef _YANG_DATA_H
#define _YANG_DATA_H


#include <stdio.h>
#include "yang.h"

extern const char* yangBuiltInTypeNames[];

typedef enum YangBuiltInType {
    YANG_TYPE_NONE                  = -1,
    YANG_TYPE_BINARY                = 0,
    YANG_TYPE_BITS                  = 1,
    YANG_TYPE_BOOLEAN               = 2,
    YANG_TYPE_EMPTY                 = 3,
    YANG_TYPE_ENUMERATION           = 4,
    YANG_TYPE_DECIMAL64             = 5,
    YANG_TYPE_UNION                 = 6,
    YANG_TYPE_IDENTITY              = 7,
    YANG_TYPE_INSTANCE_IDENTIFIER   = 8,
    YANG_TYPE_INT8                  = 9,
    YANG_TYPE_INT16                 = 10,
    YANG_TYPE_INT32                 = 11,
    YANG_TYPE_INT64                 = 12,
    YANG_TYPE_LEAFREF               = 13,
    YANG_TYPE_STRING                = 14,
    YANG_TYPE_UINT8                 = 15,
    YANG_TYPE_UINT16                = 16,
    YANG_TYPE_UINT32                = 17,
    YANG_TYPE_UINT64                = 18
} YangBuiltInType;

typedef enum YangNodeType {
    YANG_NODE_ORIGINAL          = 0,
    YANG_NODE_EXPANDED_USES     = 1,
    YANG_NODE_EXPANDED_AUGMENT  = 2,
    YANG_NODE_REFINED           = 3,
    YANG_NODE_INHERITED         = 4,
    YANG_NODE_INSTANCE          = 5,
    YANG_NODE_DEVIATION         = 6
} YangNodeType;

typedef struct _YangTypeInfo {
    YangBuiltInType   builtinType;
    struct _YangNode   *baseTypeNodePtr;
} _YangTypeInfo;

/* _YangParseState -- reflects the current state of the module processing state      */
typedef enum _YangParsingState {
    YANG_PARSING_IN_PROGRESS       = 0,  /* should not occur            */
    YANG_PARSING_DONE              = 1
} _YangParsingState;

typedef struct _YangComplexTypeInfo {
    _YangParsingState state;
    int labeled;
} _YangComplexTypeInfo;

typedef struct _YangNode {
    YangNode            export;
    YangNodeType        nodeType;
    void                *info;
    int                 line;

    /* used only for type statements */
    struct _YangTypeInfo *typeInfo;

    /* used only for complext-type statements */
    struct _YangComplexTypeInfo *ctInfo;

    struct _YangNode  	*firstChildPtr;
    struct _YangNode  	*lastChildPtr;    
    struct _YangNode  	*nextSiblingPtr;
    struct _YangNode  	*parentPtr;
    struct _YangNode  	*modulePtr;
} _YangNode;

/*
 *  Old code. Should be replaced by the YangList
 */
typedef struct _YangList {
    void                *data;
    void                *additionalInfo;
    struct _YangList    *next;    
} _YangList;


typedef struct _YangModuleInfo {
    char		*prefix;
    char		*version;
    char		*namespace;
    char        *organization;
    char        *contact;
    _YangParsingState parsingState;
    int         conformance;
    /* a module without expantion */
    struct _YangNode     *originalModule;
    struct YangList *submodules;
    struct YangList *imports;
    void                   *parser;
} _YangModuleInfo;

typedef struct _YangIdentifierRefInfo {
    char		*prefix;
    char		*identifierName;
    _YangNode   *resolvedNode;
    _YangNode   *marker;
    int loop;
} _YangIdentifierRefInfo;

typedef struct _YangGroupingInfo {
    _YangParsingState state;
} _YangGroupingInfo;

/*
 * List data structures
 */
typedef struct YangList {
    void  	*data;
    struct YangList *next;
} YangList;


typedef struct _YangImport {
    char                *prefix;
    struct _YangNode  	*modulePtr;
} _YangImport;

typedef struct _YangIdentifierRef {
   char* prefix;
   char* ident;
} _YangIdentifierRef;

YangList    *addElementToList(YangList *firstElement, void *data);

YangList *addLastElementToList(YangList *lastElement, void *data);

_YangNode  *listNode(YangList *e);

_YangImport  *listImport(YangList *e);

_YangIdentifierRef  *listIdentifierRef(YangList *e);

/*
 *  Node and Module functions
 */
_YangNode *addYangNode(const char *value, YangDecl nodeKind, _YangNode *parentPtr);

int removeYangNode(_YangNode* target, _YangNode* child);

_YangModuleInfo *createModuleInfo(_YangNode *modulePtr);

void createTypeInfo(_YangNode *node);

_YangNode *findYangModuleByName(const char *modulename, char* revision);

_YangNode *findYangModuleByPrefix(_YangNode *module, const char *prefix);

_YangNode* findChildNodeByType(_YangNode *nodePtr, YangDecl nodeKind);

_YangNode* findChildNodeByTypeAndValue(_YangNode *nodePtr, YangDecl nodeKind, char* value);

_YangNode* resolveReference(_YangNode *currentNodePtr, YangDecl nodeKind, char* prefix, char* identifierName);

_YangNode *externalModule(_YangNode *importNode);

YangBuiltInType getBuiltInType(const char *name);

int isNumericalType(YangBuiltInType type);
/*
 * YangNode fields setters
 */
void setConfig(_YangNode *nodePtr, YangConfig config);
        
void setStatus(_YangNode *nodePtr, YangStatus status);

void setDescription(_YangNode *nodePtr, char *description);

void setReference(_YangNode *nodePtr, char *reference);

/*
 * Node uniqueness validation
 */
void uniqueNodeKind(_YangNode *nodePtr, YangDecl nodeKind);

void presenceNodeKind(_YangNode *nodePtr, YangDecl nodeKind);

int getCardinality(_YangNode *nodePtr, YangDecl nodeKind);

int isDataDefNode(_YangNode* nodePtr);
/*
 *  XPath
 */
int isAbsoluteSchemaNodeid(char *s);

int isDescendantSchemaNodeid(char *s);

void freeIdentiferList(YangList *listPtr);

int isNonNegativeInteger(char *s);

YangList *getXPathNode(char* s);

YangList *getKeyList(char* s);

YangList *getUniqueList(char* s);

_YangList* processUniqueList(_YangNode *nodePtr, YangList* il);

_YangNode *copyModule(_YangNode *nodePtr);
/*
 *  Free YANG datastructures
 */
void yangFreeData();

_YangModuleInfo* getModuleInfo(_YangNode* module);

int isDataDefinitionNode(_YangNode *node);

_YangNode *createReferenceNode(_YangNode *parentPtr, _YangNode *reference, YangNodeType nodeType, int inheritNamespace);

void copySubtree(_YangNode *destPtr, _YangNode *subtreePtr, YangNodeType nodeType, int skipMandatory, int line, int inheritNamespace);

int isWSP(char ch);

int isMandatory(_YangNode *nodePtr);

#endif /* _YANG_DATA_H */
