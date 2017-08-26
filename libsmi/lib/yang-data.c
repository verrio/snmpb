/*
 * yang-data.c --
 *
 *      Operations on the main YANG data structures.
 *
 * Copyright (c) 1999-2002 Frank Strauss, Technical University of Braunschweig.
 *
 * See the file "COPYING" for information on usage and redistribution
 * of this file, and for a DISCLAIMER OF ALL WARRANTIES.
 *
 * Authors: Kaloyan Kanev, Siarhei Kuryla
 * @(#) $Id: data.c 12198 2009-01-05 08:37:07Z schoenw $
 */

#include <config.h>

#ifdef BACKEND_YANG

#include "yang-data.h"

#include "smi.h"
#include <string.h>

#include "yang.h"
#include "smi.h"
#include "smi.h"

#include <string.h>
#include <errno.h>
#include <ctype.h>
#include <sys/types.h>
#include <string.h>
#if !defined(_MSC_VER) && !defined(__MINGW32__)
#include <sys/wait.h>
#endif
#ifdef HAVE_UNISTD_H
#include <unistd.h>
#endif
#ifdef HAVE_WIN_H
#include "win.h"
#endif

#include "common.h"
#include "smi-data.h"
#include "yang.h"
#include "yang-check.h"
#include "scanner-yang.h"

#ifdef HAVE_DMALLOC_H
#include <dmalloc.h>
#endif

/*
 * Current parser defined in parser-yang. Workaround - can't include data.h
 */
extern Parser *currentParser;

const char* yangBuiltInTypeNames[] = {  "binary",
                                        "bits",
                                        "boolean",
                                        "empty",
                                        "enumeration",
                                        "decimal64",
                                        "union",
                                        "identityref",
                                        "instance-identifier",
                                        "int8",
                                        "int16",
                                        "int32",
                                        "int64",
                                        "leafref",
                                        "string",
                                        "uint8",
                                        "uint16",
                                        "uint32",
                                        "uint64",
					NULL
};


YangBuiltInType getBuiltInType(const char *name) {
    int i;
    for (i = 0; yangBuiltInTypeNames[i]; i++) {
        if (!strcmp(yangBuiltInTypeNames[i], name)) {
            return i;
        }
    }
    return YANG_TYPE_NONE;
}


int isNumericalType(YangBuiltInType type) {
    return type == YANG_TYPE_INT8 ||
           type == YANG_TYPE_INT16 ||
           type == YANG_TYPE_INT32 ||
           type == YANG_TYPE_INT64 ||
           type == YANG_TYPE_UINT8 ||
           type == YANG_TYPE_UINT16 ||
           type == YANG_TYPE_UINT32 ||
           type == YANG_TYPE_UINT64 ||
           type == YANG_TYPE_DECIMAL64;    
}
/*
 * YangNode fields setters
 */
void setConfig(_YangNode *nodePtr, YangConfig config)
{
    nodePtr->export.config = config;
}

void setStatus(_YangNode *nodePtr, YangStatus status)
{
    nodePtr->export.status = status;
}

void setDescription(_YangNode *nodePtr, char *description)
{
    if (nodePtr->export.description) {
        smiFree(nodePtr->export.description);
    }
    nodePtr->export.description = smiStrdup(description);
}

void setReference(_YangNode *nodePtr, char *reference)
{
    if (nodePtr->export.reference) {
        smiFree(nodePtr->export.reference);
    }
    nodePtr->export.reference = smiStrdup(reference);
}

/*
 * Node uniqueness validation
 */
void uniqueNodeKind(_YangNode *nodePtr, YangDecl decl) 
{
    if (findChildNodeByType(nodePtr, decl)) {
        smiPrintError(currentParser, ERR_REDEFINED_ELEMENT,
		      yangDeclAsString(decl));
    }
}


void presenceNodeKind(_YangNode *nodePtr, YangDecl decl) 
{
    if (!findChildNodeByType(nodePtr, decl)) {
        smiPrintError(currentParser, ERR_REQUIRED_ELEMENT,
		      yangDeclAsString(decl));
    }
}

int getCardinality(_YangNode *nodePtr, YangDecl nodeKind) 
{
    _YangNode *childPtr = NULL;
    int ret = 0;
    for (childPtr = nodePtr->firstChildPtr; childPtr; childPtr = childPtr->nextSiblingPtr) {
        if (childPtr->export.nodeKind == nodeKind) {
            ret++;
        }
    }
    return ret;
}

/* ----------------------------------------------------------------------
 *
 *  Utils
 *
 * ----------------------------------------------------------------------
 */
_YangModuleInfo* getModuleInfo(_YangNode* module) {
    return (_YangModuleInfo*)module->info;
}

/*
 *----------------------------------------------------------------------
 *
 * findYangModuleByName --
 *
 *      Lookup a Yang module by the given name.
 *
 * Results:
 *      A pointer to the _YangNode structure or
 *      NULL if it is not found.
 *
 * Side effects:
 *      None.
 *
 *----------------------------------------------------------------------
 */


_YangNode *findYangModuleByName(const char *modulename, char* revision)
{
    _YangNode	*modulePtr;
    
    for (modulePtr = smiHandle->firstYangModulePtr; modulePtr; modulePtr = modulePtr->nextSiblingPtr) {
        if ((modulePtr->export.value) && !strcmp(modulePtr->export.value, modulename)) {
            if (!revision) {
                return (modulePtr);
            } else {
                _YangNode* revisionNodePtr = findChildNodeByType(modulePtr, YANG_DECL_REVISION);
                if (revisionNodePtr) {
                    if (!strcmp(revision, revisionNodePtr->export.value)) {
                        return modulePtr;
                    }
                }
            }
        }
    }
    return (NULL);
}


/*
 *----------------------------------------------------------------------
 *
 * findYangModuleByPrefix --
 *
 *      Lookup the imported Yang module by the given prefix.
 *
 * Results:
 *      A pointer to the _YangNode structure or
 *      NULL if it is not found.
 *
 * Side effects:
 *      None.
 *
 *---------------------------------------------------------------------- */


_YangNode *findYangModuleByPrefix(_YangNode *module, const char *prefix)
{
    YangList *imports = getModuleInfo(module)->imports;    
    for (; imports; imports = imports->next) {
        if (!strcmp(listImport(imports)->prefix, prefix)) {
            return listImport(imports)->modulePtr;
        }
    }
    return (NULL);
}
/*
 *----------------------------------------------------------------------
 *
 * findChildNodeByType --
 *
 *      Lookup a child node by a given type.
 *
 * Results:
 *      A pointer to the _YangNode structure or
 *      NULL if it is not found.
 *
 * Side effects:
 *      None.
 *
 *----------------------------------------------------------------------
 */
_YangNode* findChildNodeByType(_YangNode *nodePtr, YangDecl nodeKind) {
    _YangNode *childPtr = NULL;
    if (nodePtr == NULL) {
        return NULL;
    }
    for (childPtr = nodePtr->firstChildPtr; childPtr; childPtr = childPtr->nextSiblingPtr) {
        if (childPtr->export.nodeKind == nodeKind) {
            return childPtr;
        }
    }
    return NULL;
}

/*
 *----------------------------------------------------------------------
 *
 * findChildNodeByTypeAndValue --
 *
 *      Lookup a child node by a given type and value.
 *
 * Results:
 *      A pointer to the _YangNode structure or
 *      NULL if it is not found.
 *
 * Side effects:
 *      None.
 *
 *----------------------------------------------------------------------
 */
_YangNode* findChildNodeByTypeAndValue(_YangNode *nodePtr, YangDecl nodeKind, char* value) {
    _YangNode *childPtr = NULL;
    for (childPtr = nodePtr->firstChildPtr; childPtr; childPtr = childPtr->nextSiblingPtr) {
        if (childPtr->export.nodeKind == nodeKind && !strcmp(childPtr->export.value, value)) {
            return childPtr;
        }
    }
    return NULL;
}

/*
 *----------------------------------------------------------------------
 *
 * resolveNodeByTypeAndValue
 *
 *      Resolve a node by the given type and argument.
 *
 * Results:
 *      A pointer to the _YangNode structure or
 *      NULL if it is not found.
 *
 * Side effects:
 *      None.
 *
 *----------------------------------------------------------------------
 */
_YangNode* resolveNodeByTypeAndValue(_YangNode *nodePtr, YangDecl nodeKind, char* value, int depth) {
    _YangNode *childPtr = NULL;
    if (depth < 0) return NULL;
    for (childPtr = nodePtr->firstChildPtr; childPtr; childPtr = childPtr->nextSiblingPtr) {
        if (childPtr->export.nodeKind == nodeKind && !strcmp(childPtr->export.value, value)) {
            return childPtr;
        }
    }
    if (nodePtr->parentPtr) {
        _YangNode *ret = resolveNodeByTypeAndValue(nodePtr->parentPtr, nodeKind, value, depth);
        if (ret) return ret;
    } else {
        YangList *submodules = getModuleInfo(nodePtr)->submodules;
        for (; submodules; submodules = submodules->next) {            
            _YangNode *ret = resolveNodeByTypeAndValue(listNode(submodules), nodeKind, value, depth - 1);
            if (ret) return ret;
        }        
    }
    return NULL;
}

/*
 *----------------------------------------------------------------------
 *
 * resolveReference
 *
 *      Resolve a reference to the node with the give type(nodeKind) and argument value (identifierName) 
 *      either in the current or imported module specified by the prefix
 *      in the context of the currentNodePtr node.
 *
 * Results:
 *      A pointer to the _YangNode structure or
 *      NULL if it is not found.
 *
 * Side effects:
 *      None.
 *
 *----------------------------------------------------------------------
 */
_YangNode* resolveReference(_YangNode *currentNodePtr, YangDecl nodeKind, char* prefix, char* identifierName) {
    if (prefix && strcmp(getModuleInfo(currentNodePtr->modulePtr)->prefix, prefix)) {
        /* this is a reference to another module */
        _YangNode *modulePtr = findYangModuleByPrefix(currentNodePtr->modulePtr, prefix);
        if (!modulePtr) return NULL;
        return resolveNodeByTypeAndValue(modulePtr, nodeKind, identifierName, 1);
    } else {
        /* local reference */
        return resolveNodeByTypeAndValue(currentNodePtr, nodeKind, identifierName, 1);
    }
    return NULL;
}

 /*----------------------------------------------------------------------
 *
 * 
 * createModuleInfo --
 *
 *      Attach info to the module.
 *
 * Results:
 *      A pointer to the new _YangModuleInfo structure or
 *	NULL if terminated due to an error.
 *
 *----------------------------------------------------------------------
 */
_YangModuleInfo *createModuleInfo(_YangNode *modulePtr)
{
    Module *module;
    _YangModuleInfo *infoPtr;

    if (! modulePtr) return NULL;
    
    infoPtr = smiMalloc(sizeof(_YangModuleInfo));
    modulePtr->info = infoPtr;
    
    infoPtr->namespace     = NULL;
    infoPtr->prefix        = NULL;
    infoPtr->version       = NULL;
    infoPtr->organization  = NULL;
    infoPtr->contact       = NULL;
    infoPtr->parsingState  = YANG_PARSING_IN_PROGRESS;
    infoPtr->originalModule = NULL;
    infoPtr->submodules    = NULL;

    /* Create a corresponding Module wrapper to maintain interface
     * compatibility */
    
    module = addModule(smiStrdup(modulePtr->export.value),
		       smiStrdup(currentParser ? currentParser->path : ""),
		       0, currentParser);
    module->export.language = SMI_LANGUAGE_YANG;
    if (currentParser) currentParser->modulePtr = module;
    return (infoPtr);
}

void createIdentifierRef(_YangNode *node, char* prefix, char* ident) {
    _YangIdentifierRefInfo *infoPtr = smiMalloc(sizeof(_YangIdentifierRefInfo));
    
    if (prefix) {
        infoPtr->prefix = prefix;
    } else {
        infoPtr->prefix = smiStrdup(getModuleInfo(node->modulePtr)->prefix);
    }
    infoPtr->identifierName = ident;
    infoPtr->resolvedNode = NULL;
    infoPtr->marker = NULL;
    infoPtr->loop = 0;
    
    node->info = infoPtr;
}

void createTypeInfo(_YangNode *node) {
    _YangTypeInfo *infoPtr = smiMalloc(sizeof(_YangTypeInfo));

    infoPtr->builtinType        = getBuiltInType(node->export.value);
    infoPtr->baseTypeNodePtr    = NULL;
    node->typeInfo              = infoPtr;
}

_YangNode *addYangNode(const char *value, YangDecl nodeKind, _YangNode *parentPtr)
{
    _YangNode *node = (_YangNode*) smiMalloc(sizeof(_YangNode));
    node->nodeType              = YANG_NODE_ORIGINAL;
    node->export.value          = smiStrdup(value);
    node->export.nodeKind       = nodeKind;
    node->export.description	= NULL;
    node->export.reference	= NULL;
    node->export.extra  	= NULL;
    node->export.config         = YANG_CONFIG_DEFAULT;
    node->export.status         = YANG_STATUS_DEFAULT_CURRENT;
    node->line                  = currentParser ? currentParser->line : 0;
    
    node->info                  = NULL;    
    node->typeInfo              = NULL;
    
    node->nextSiblingPtr        = NULL;
    node->firstChildPtr         = NULL;
    node->lastChildPtr          = NULL;
    node->parentPtr             = parentPtr;

    if (parentPtr)
    {
        node->modulePtr         = parentPtr->modulePtr;
		
        if(parentPtr->lastChildPtr)
        {
                (parentPtr->lastChildPtr)->nextSiblingPtr = node;
                parentPtr->lastChildPtr = node;
        }
        else /* first child */
        {
                parentPtr->firstChildPtr = node;
                parentPtr->lastChildPtr = node;
        }
    } else {
        /*  it's a module node */
        node->modulePtr         = node;
    }
    return node;
}


/*
 *----------------------------------------------------------------------
 *
 * loadYangModule --
 *
 *      Load a YANG module. If modulename is a plain name, the file is
 *	search along the SMIPATH environment variable. If modulename
 *	contains a `.' or DIR_SEPARATOR it is assumed to be the path.
 *
 *
 *----------------------------------------------------------------------
 */


_YangNode *loadYangModule(const char *modulename, const char * revision, Parser *parserPtr)
{
    Parser      *parser = smiMalloc(sizeof(Parser));
    Parser      *parentParserPtr;
    char	*path = NULL;
    SmiLanguage lang = 0;
    FILE	*file;
    char* name[2], *revisionPart = NULL;
    int index = 1;
    int nameIndex = 0;

    if (revision) {
        smiAsprintf(&name[0], "%s%s", modulename, "%s");
        smiAsprintf(&revisionPart, ".%s", revision);
        index++;
    }
    name[index - 1] = smiStrdup(modulename);
    
    nameIndex = 0;
    while (nameIndex < index) {        
        path = smiGetModulePath(name[nameIndex]);
        if (path && revision) {
            smiAsprintf(&path, "%s%s", path, revisionPart);
        }
        /* module can not be located, the last try */
        if (!path && parserPtr && parserPtr->path) {
            /*  searching for the module at the path where the previous module was found
                it's used by the imported modules */
            int slashIndex = -1;
            int i = 0;
            for (i = strlen(parserPtr->path) - 1; i + 1; i--)
                if (parserPtr->path[i] == DIR_SEPARATOR) {
                    slashIndex = i;
                    break;
                }
            if (slashIndex == -1) {
                smiAsprintf(&path, "%s%s", name[nameIndex], ".yang");
            } else {
                char *dir = (char*) smiMalloc(slashIndex + 2);
                dir[slashIndex + 1] = 0;
                strncpy(dir, parserPtr->path, slashIndex + 1);

                smiAsprintf(&path, "%s%s%s", dir, name[nameIndex], ".yang");

                smiFree(dir);
            }
            if (path && revision) {
                smiAsprintf(&path, path, revisionPart);
            }
        }
        if (path) {
            file = fopen(path, "r");
            if (file) {
                break;
            }
        }        
        nameIndex++;
    }

    if (!path) {
        smiPrintError(parserPtr, ERR_MODULE_NOT_FOUND, modulename);
        return NULL;
    }

    parser->path			= path;
    file = fopen(path, "r");
    if (! file) {
        smiPrintError(parserPtr, ERR_OPENING_INPUTFILE, path, strerror(errno));
        smiFree(path);
        return NULL;
    }
    lang = smiGuessFileLanguage(file);

    if (lang != SMI_LANGUAGE_YANG) {
        smiPrintError(parserPtr, ERR_ILLEGAL_INPUTFILE, path);
        smiFree(path);
        fclose(file);
        return NULL;
    }

#ifdef BACKEND_YANG
	parentParserPtr = smiHandle->parserPtr;
	smiHandle->parserPtr = parser;
    /* 
     *  Initialization of the parser;
     *  In YANG we don't use most of these fields of the Parser
     */
	parser->path			= path;
	parser->flags			= smiHandle->flags;
	parser->modulePtr		= NULL;
	parser->complianceModulePtr	= NULL;
	parser->capabilitiesModulePtr	= NULL;
	parser->currentDecl              = SMI_DECL_UNKNOWN;
	parser->firstStatementLine       = 0;
	parser->firstNestedStatementLine = 0;
	parser->firstRevisionLine        = 0;
        parser->yangModulePtr            = NULL;
	parser->file			= file;

    
	if (yangEnterLexRecursion(parser->file) < 0) {
	    smiPrintError(parser, ERR_MAX_LEX_DEPTH);
	    fclose(parser->file);
	}
	smiDepth++;
	parser->line			= 1;
	yangparse(parser);
	yangLeaveLexRecursion();
	smiDepth--;
	fclose(parser->file);
	smiHandle->parserPtr = parentParserPtr;
    if (parser->yangModulePtr) {
        ((_YangModuleInfo*)(parser->yangModulePtr->info))->conformance = parser->modulePtr->export.conformance;
        ((_YangModuleInfo*)(parser->yangModulePtr->info))->parser = parser;

        return parser->yangModulePtr;
    } else {
        smiFree(path);
        smiFree(parser);
        return NULL;
    }
#else
	smiPrintError(parserPtr, ERR_YANG_NOT_SUPPORTED, path);
	smiFree(path);
        fclose(file);
   
	return NULL;
#endif

    smiFree(path);
    fclose(file);
    return NULL;
}

/*
 *----------------------------------------------------------------------
 *
 * addSubmodule --
 *
 *      Add an included submodule to the module or submodule
 *
 * Side effects:
 *      None.
 *
 *----------------------------------------------------------------------
 */
void addSubmodule(_YangNode *module, _YangNode *submodule) {
    YangList* cur = ((_YangModuleInfo*)module->info)->submodules;
    while (cur) {
        if (listNode(cur) == submodule) return;
        cur = cur->next;
    }
    
    ((_YangModuleInfo*)module->info)->submodules = addElementToList(((_YangModuleInfo*)module->info)->submodules, submodule);

    /* go through all child submodules included by the current submodule and add them to the module as well */
    cur = ((_YangModuleInfo*)submodule->info)->submodules;
    while (cur) {
        addSubmodule(module, listNode(cur));
        cur = cur->next;
    }    
}

/*
 *----------------------------------------------------------------------
 *
 * addImportedModule --
 *
 *      Add an imported module to the module or submodule
 *
 * Side effects:
 *      None.
 *
 *----------------------------------------------------------------------
 */
void addImportedModule(_YangNode *importNode, _YangNode *importedModule) {
    YangList* cur;
    _YangImport *import;
    char* prefix = getModuleInfo(importNode->modulePtr)->prefix;
    char* importPrefix = findChildNodeByType(importNode, YANG_DECL_PREFIX)->export.value;

    if (!strcmp(prefix, importPrefix)) {
        smiPrintError(currentParser, ERR_DUPLICATED_PREFIX, importPrefix);
    }
    
    for (cur = ((_YangModuleInfo*)importNode->modulePtr->info)->imports;
	 cur;  cur = cur->next) {
        if (!strcmp(listImport(cur)->prefix, importPrefix)) {
            smiPrintError(currentParser, ERR_DUPLICATED_PREFIX, importPrefix);
        }
    }

    import = smiMalloc(sizeof(_YangImport));
    import->prefix = importPrefix;
    import->modulePtr = importedModule;
    ((_YangModuleInfo*)importNode->modulePtr->info)->imports = addElementToList(((_YangModuleInfo*)importNode->modulePtr->info)->imports, import);
}

/*
 *----------------------------------------------------------------------
 *
 * externalModule --
 *
 *      Lookup a YANG Module or Submodule by a given name.
 *
 * Results:
 *      A pointer to the _YangModule structure or
 *      NULL if it is not found.
 *
 * Side effects:
 *      None.
 *
 *----------------------------------------------------------------------
 */
_YangNode *externalModule(_YangNode *importNode) {
    /* Check whether the imported module has been already loaded */
    _YangNode *revisionNodePtr = findChildNodeByType(importNode, YANG_DECL_REVISION);
    char* revision = NULL;
    if (revisionNodePtr) {
        revision = revisionNodePtr->export.value;
    }
    _YangNode *importedModule = findYangModuleByName(importNode->export.value, revision);

    /* Check whether there is no cyclic import chain */
    if (importedModule && ((_YangModuleInfo*)importedModule->info)->parsingState == YANG_PARSING_IN_PROGRESS) {
        smiPrintError(currentParser, ERR_CYCLIC_IMPORTS, importNode->modulePtr->export.value, importedModule->export.value);
    }
    
    /* If the module has not been found, lets try to load it */
    if(!importedModule) {
        Parser* tempParser = currentParser;
        importedModule = loadYangModule(importNode->export.value, revision, currentParser);
        currentParser = tempParser;
    }
  
    if(importedModule && !strcmp(importNode->export.value, importedModule->export.value)) {		
        if (importNode->export.nodeKind == YANG_DECL_INCLUDE) {
            addSubmodule(importNode->modulePtr, importedModule);
        } else if (importNode->export.nodeKind == YANG_DECL_IMPORT) {
            addImportedModule(importNode, importedModule);
        }
        return importedModule;
    } else {
        smiPrintError(currentParser, ERR_IMPORT_NOT_FOUND, importNode->export.value);
    }    
    return NULL;
}


_YangNode *copyModule(_YangNode *nodePtr) {
    _YangNode *node;
    _YangNode *childPtr;
    
    if (!nodePtr) return NULL;  
    node = (_YangNode*) smiMalloc(sizeof(_YangNode));
    node->nodeType              = YANG_NODE_ORIGINAL;
    node->export.value          = smiStrdup(nodePtr->export.value);
    node->export.nodeKind       = nodePtr->export.nodeKind;
    node->export.description	= smiStrdup(nodePtr->export.description);
    node->export.reference		= smiStrdup(nodePtr->export.reference);
    node->export.extra  		= nodePtr->export.extra;
    node->export.config         = nodePtr->export.config;
    node->export.status         = nodePtr->export.status;
    node->line                  = 0;
    node->info                  = NULL;    
    node->typeInfo              = NULL;
    node->firstChildPtr         = NULL;
    node->lastChildPtr          = NULL;
    node->parentPtr             = NULL;
    node->modulePtr             = NULL;

    for (childPtr = nodePtr->firstChildPtr;
	 childPtr; childPtr = childPtr->nextSiblingPtr) {
        _YangNode* c = copyModule(childPtr);
        if (!node->firstChildPtr) {
            node->firstChildPtr = c;
            node->lastChildPtr = c;
        } else {
            node->lastChildPtr->nextSiblingPtr = c;            
            node->lastChildPtr = c;
        }
    }
    return node;
    
}

void freeUniqueList(_YangList* listPtr);
/*
 *----------------------------------------------------------------------
 *
 * freeYangNode --
 *
 *      Free a YANG module tree.
 *
 * Side effects:
 *      None.
 *
 *----------------------------------------------------------------------
 */
void freeYangNode(_YangNode *nodePtr) {
    _YangNode *currentNode, *nextNode;

    if (!nodePtr) return;

    /* free only original node's memory, because references hold only pointers to other node's fields */
    if (nodePtr->nodeType == YANG_NODE_ORIGINAL) {
        YangDecl nodeKind = nodePtr->export.nodeKind;
        if (nodePtr->info) {
            if (nodeKind == YANG_DECL_MODULE || nodeKind == YANG_DECL_SUBMODULE) {
                /* Free original tree */
                freeYangNode(getModuleInfo(nodePtr)->originalModule);
                smiFree(((Parser*)getModuleInfo(nodePtr)->parser)->path);
                smiFree(getModuleInfo(nodePtr)->parser);
                
                getModuleInfo(nodePtr)->originalModule = NULL;
                YangList *submodules = getModuleInfo(nodePtr)->submodules;
                while (submodules) {
                    YangList *next = submodules->next;
                    smiFree(submodules);
                    submodules = next;
                }
                YangList *imports = getModuleInfo(nodePtr)->imports;
                while (imports) {
                    YangList *next = imports->next;
                    smiFree(listImport(imports));
                    smiFree(imports);
                    imports = next;
                }        
            }

            if (nodeKind == YANG_DECL_UNKNOWN_STATEMENT ||
                nodeKind == YANG_DECL_IF_FEATURE ||
                nodeKind == YANG_DECL_TYPE ||
                nodeKind == YANG_DECL_USES || 
                nodeKind == YANG_DECL_BASE) {
                    _YangIdentifierRefInfo *info = (_YangIdentifierRefInfo*)nodePtr->info;
                    if (info) {
                        smiFree(info->identifierName);
                        smiFree(info->prefix);
                    }
            }

            if (nodeKind == YANG_DECL_TYPE) {
                smiFree(nodePtr->typeInfo);
            }        
            if (nodeKind == YANG_DECL_KEY) {
                freeIdentiferList(nodePtr->info);
                nodePtr->info = NULL;
            }
            if (nodeKind == YANG_DECL_UNIQUE) {
                freeUniqueList(nodePtr->info);
                nodePtr->info = NULL;
            }

            smiFree(nodePtr->info);
            nodePtr->info = NULL;
        }
       
        smiFree(nodePtr->export.value);
        nodePtr->export.value = NULL;
        smiFree(nodePtr->export.extra);
        nodePtr->export.extra = NULL;
        /*
         *  We don't need to free the memory of 'description' and 'reference', 
         *  because they are reffered from another node and will be released there
         */
        nodePtr->export.description = NULL;
        nodePtr->export.reference = NULL;        
    }

    for (currentNode= nodePtr->firstChildPtr;
	 currentNode; currentNode = nextNode) {
	nextNode = currentNode->nextSiblingPtr;
	freeYangNode(currentNode);
    }

    smiFree(nodePtr);
    nodePtr = NULL;
}


/*
 *----------------------------------------------------------------------
 *
 * removeYangNode --
 *
 *      Remove a Yang Node.
 *
 * Side effects:
 *      None.
 *
 *----------------------------------------------------------------------
 */
int removeYangNode(_YangNode* target, _YangNode* child) {
    _YangNode *cur = target->firstChildPtr;
    if (cur == child) {
        target->firstChildPtr = cur->nextSiblingPtr;
        if (!target->firstChildPtr) {
            target->lastChildPtr = NULL;
        }
        freeYangNode(cur);
        return 1;
    } else {
        while (cur) {
            _YangNode* nextNode = cur->nextSiblingPtr;
            if (nextNode == child) {
                cur->nextSiblingPtr = nextNode->nextSiblingPtr;
                if (!nextNode->nextSiblingPtr) {
                    target->lastChildPtr = cur;
                }
                freeYangNode(nextNode);
                return 1;
            }
            cur = nextNode;
        }
    }
    return 0;
}

/*
 *----------------------------------------------------------------------
 *
 * yangFreeData --
 *
 *      Free YANG all data structures.
 *
 * Side effects:
 *      None.
 *
 *----------------------------------------------------------------------
 */
void yangFreeData() {
    _YangNode	*modulePtr;
    for (modulePtr = smiHandle->firstYangModulePtr; modulePtr; modulePtr = modulePtr->nextSiblingPtr) {
        freeYangNode(modulePtr);
    }    
}

int isDataDefNode(_YangNode* nodePtr) {
    YangDecl kind = nodePtr->export.nodeKind;
    return kind == YANG_DECL_CONTAINER ||
           kind == YANG_DECL_LEAF ||
           kind == YANG_DECL_LEAF_LIST ||
           kind == YANG_DECL_LIST ||
           kind == YANG_DECL_CHOICE ||
           kind == YANG_DECL_ANYXML ||
           kind == YANG_DECL_USES ||
           kind == YANG_DECL_INSTANCE ||
           kind == YANG_DECL_INSTANCE_LIST;
    
}

/*
 *  List functions
 */
YangList *addElementToList(YangList *firstElement, void *data) {
    YangList *listPtr = smiMalloc(sizeof(YangList));    
    listPtr->data = data;
    listPtr->next = firstElement;
    return listPtr;
}

YangList *addLastElementToList(YangList *lastElement, void *data) {
    YangList *listPtr = smiMalloc(sizeof(YangList));
    listPtr->data = data;
    listPtr->next = NULL;
    lastElement->next = listPtr;
    return listPtr;
}

_YangNode  *listNode(YangList *e) {
    return (_YangNode*)e->data;
}

_YangImport  *listImport(YangList *e) {
    return (_YangImport*)e->data;
}

_YangIdentifierRef  *listIdentifierRef(YangList *e) {
    return (_YangIdentifierRef*)e->data;
}

 /*
  * Grammar for simple elements
  */

int isWSP(char ch) {
    return ch == ' ' ||  ch == '\t';
}

int isSeparator(char ch) {
    return isWSP(ch) || ch == '\n';
}

int isAlpha(char ch) {
    return (ch >= 'a' && ch <= 'z') || (ch >= 'A' && ch <= 'Z');
}

int isDigit(char ch) {
    return (ch >= '0' && ch <= '9');
}

int isPositiveInteger(char *s) {
    int i;
    if (!s) return 0;
    if (!(s[0] > '0' && s[0] <= '9')) {
        return 0;
    }
    for (i = 1; i < strlen(s); i++) {
        if (!isDigit(s[i]))
            return 0;
    }
    return 1;
}

int isNonNegativeInteger(char *s) {
    if (!s) return 0;
    if (isPositiveInteger(s) || (s[0] == '0' && strlen(s) == 1)) {
        return 1;
    } else {
        return 0;
    }
   
}

int buildIdentifier(char *s) {
    int ret = 1;
    if (!s || strlen(s) == 0) return 0;
    if (!isAlpha(s[0]) && s[0] != '_') return 0;
    while (ret < strlen(s) && (isAlpha(s[ret]) || isDigit(s[ret]) || s[ret] == '_' || s[ret] == '-' || s[ret] == '.')) {
        ret++;
    }
    return ret;
}

int nodeIdentifier(char* s) {
    int ret;
    
    if (!s || strlen(s) == 0) return 0;
    ret = buildIdentifier(s);
    if (!ret || ret == strlen(s)) return ret;
    if (s[ret] == ':') {
        int ret1 = buildIdentifier(s + ret + 1);
        if (ret1) return ret + ret1 + 1;
    }
    return ret;
}

int absoluteSchemaNodeid(char *s) {
    int ret = 0;
    if (!s || strlen(s) == 0) return 0;
    while (ret < strlen(s)) {
	int cur;
        if (s[ret] != '/') return ret;        
        cur = nodeIdentifier(s + ret + 1);
        if (cur > 0) {
            ret += cur + 1;
        } else {
            return ret;
        }       
    }
    return ret;
}

int descendantSchemaNodeid(char *s) {
    int ret;
    if (!s || strlen(s) == 0) return 0;
    ret = nodeIdentifier(s);
    if (!ret) return 0;
    return ret + absoluteSchemaNodeid(s + ret);
}

int isAbsoluteSchemaNodeid(char *s) {
    if (!s || strlen(s) == 0) return 0;
    return (absoluteSchemaNodeid(s) == strlen(s));
}

int isDescendantSchemaNodeid(char *s) {
    if (!s || strlen(s) == 0) return 0;
    return (descendantSchemaNodeid(s) == strlen(s));
}

void freeIdentiferList(YangList *listPtr) {
    while (listPtr) {
        YangList *tmp = listPtr;
        listPtr = listPtr->next;
        if (listIdentifierRef(tmp)->ident) {
            smiFree(listIdentifierRef(tmp)->ident);
        }
        if (listIdentifierRef(tmp)->prefix) {
            smiFree(listIdentifierRef(tmp)->prefix);
        }
        smiFree(listIdentifierRef(tmp));
        smiFree(tmp);
    }
}


YangList *getXPathNode(char* s) {
    int i = 0;
    YangList *ret = NULL, *prev = NULL;
    if (s[0] == '/') i = 1;
    
    while (i < strlen(s)) {
        int i1 = buildIdentifier(s + i);
        int i2 = 0;
        _YangIdentifierRef *cur = smiMalloc(sizeof(_YangIdentifierRef));
        cur->prefix = NULL;
        if (s[i + i1] == ':') {
            i2 = buildIdentifier(s + i + i1 + 1);
            cur->prefix = smiStrndup(s + i, i1);
            cur->ident  = smiStrndup(s + i + i1 + 1, i2);
            i2 += 1;
        } else {
            cur->ident  = smiStrndup(s + i, i1);
        }
        i += i1 + i2 + 1;
        
        if (ret == NULL) {
            ret = addElementToList(NULL, cur);
            prev = ret;
        } else {
            prev = addLastElementToList(prev, cur);
        }
    }
    return ret;    
}

YangList *getKeyList(char* s) {
    int i = 0;
    YangList *ret = NULL, *item = NULL, *prev = NULL;
    
    while (i < strlen(s)) {
        while (i != 0 && i < strlen(s) && isSeparator(s[i])) {
            i++;
        }
        if (i < strlen(s)) {
            int i1 = buildIdentifier(s + i);
            if (i1) {
                int isNew = 1;
                char* key = smiStrndup(s + i, i1);
                i += i1;
                item = ret;
                while (item) {
                    if (!strcmp(listIdentifierRef(item)->ident, key)) {
                        isNew = 0;
                        break;
                    }
                    item = item->next;
                }
                if (isNew) {
                    _YangIdentifierRef *cur = smiMalloc(sizeof(_YangIdentifierRef));
                    cur->prefix = NULL;
                    cur->ident  = key;

                    if (ret == NULL) {
                        ret = addElementToList(NULL, cur);
                        prev = ret;
                    } else {
                        prev = addLastElementToList(prev, cur);
                    }
                } else {
                    smiPrintError(currentParser, ERR_DUPLICATED_KEY, key);
                    smiFree(key);
                }
            } else {
                freeIdentiferList(ret);
                ret = NULL;
                break;                
            }
        } else {
            freeIdentiferList(ret);
            ret = NULL;
            break;
        }
    }
    if (ret == NULL) {
        smiPrintError(currentParser, ERR_ARG_VALUE, s, "key-arg");
    }
    return ret;    
}


YangList *getUniqueList(char* s) {
    int i = 0;
    YangList *ret = NULL, *item = NULL, *prev = NULL;
    
    while (i < strlen(s)) {
        while (i != 0 && i < strlen(s) && isSeparator(s[i])) {
            i++;
        }
        if (i < strlen(s)) {
            int i1 = descendantSchemaNodeid(s + i);
            if (i1) {
                char* schemaNodeId = smiStrndup(s + i, i1);
                i += i1;
                _YangIdentifierRef *cur = smiMalloc(sizeof(_YangIdentifierRef));
                cur->prefix = NULL;
                cur->ident  = schemaNodeId;

                if (ret == NULL) {
                    ret = addElementToList(NULL, cur);
                    prev = ret;
                } else {
                    prev = addLastElementToList(prev, cur);
                }
            } else {
                freeIdentiferList(ret);
                ret = NULL;
                break;                
            }
        } else {
            freeIdentiferList(ret);
            ret = NULL;
            break;
        }
    }
    /*if (!ret) {
        printf("(null)\n");
    } else {
         _YangIdentifierList *cur = ret;
        while (cur) {
            printf("%s\n", cur->ident);
            cur = cur->next;
        }
        printf("-----------------------------------\n");
    }    */
    if (ret == NULL) {
        smiPrintError(currentParser, ERR_ARG_VALUE, s, "unique-arg");
    }
    return ret;
}

_YangList *createListElement(_YangList *parent) {
    _YangList *ret = smiMalloc(sizeof(_YangList));
    ret->data           = NULL;
    ret->additionalInfo = NULL;
    ret->next           = NULL;
    if (parent) {
        parent->next = ret;        
    }
    return ret;
}

void freeList(_YangList* listPtr) {
    while (listPtr) {
        _YangList *tmp = listPtr;
        listPtr = listPtr->next;
        smiFree(tmp);
    }    
}

void freeUniqueList(_YangList* listPtr) {
    while (listPtr) {
        YangList *il = (YangList*)listPtr->data;
        freeIdentiferList(il);
        smiFree(listPtr->additionalInfo);
        listPtr = listPtr->next;
    }   
}

_YangList* processUniqueList(_YangNode *nodePtr, YangList* il) {
    if (!il) return NULL;
    _YangList* ret = NULL, *prev = NULL;
    YangList* cur = il;    
    while (cur) {
        YangList* path = getXPathNode(listIdentifierRef(cur)->ident), *tmpPath = NULL;
        /* validate prefixes */
        tmpPath = path;
        int isOk = 1;         
        while (tmpPath) {
            if (listIdentifierRef(tmpPath)->prefix && strcmp(listIdentifierRef(tmpPath)->prefix, getModuleInfo(nodePtr->modulePtr)->prefix)) break;
            tmpPath = tmpPath->next;
        }
        if (tmpPath) {
            smiPrintError(currentParser, ERR_INVALID_UNIQUE_REFERENCE, listIdentifierRef(cur)->ident);
            isOk = 0;
        }
        /* validate whether we have already met a reference to the same leaf */
        _YangList* tmp = prev;
        while (tmp) {
            tmpPath = path;
            YangList *ilist = (YangList*)tmp->data;
            while (ilist && tmpPath) {
                if (strcmp(listIdentifierRef(ilist)->ident, listIdentifierRef(tmpPath)->ident)) break;
                ilist = ilist->next;
                tmpPath = tmpPath->next;
            }
            if (!ilist && !tmpPath) {
                isOk = 0;
                smiPrintError(currentParser, ERR_DUPLICATED_LEAF_IN_UNIQUE, listIdentifierRef(cur)->ident);
                break;
            }
            tmp = tmp->next;
        }
        if (isOk) {
            prev = createListElement(prev);
            prev->data = path;
            prev->additionalInfo = smiStrdup(listIdentifierRef(cur)->ident);
            if (!ret) {
                ret = prev;
            }
        } else {
            freeIdentiferList(path);
        }
        cur =  cur->next;
    }
    return ret;
}

int isDataDefinitionNode(_YangNode *node)
{
    YangDecl kind;
    
    if (!node) return 0;
    kind = node->export.nodeKind;
    return (kind == YANG_DECL_CONTAINER ||
            kind == YANG_DECL_LEAF ||
            kind == YANG_DECL_LEAF_LIST ||
            kind == YANG_DECL_LIST ||
            kind == YANG_DECL_CHOICE ||
            kind == YANG_DECL_ANYXML ||
            kind == YANG_DECL_USES ||
            kind == YANG_DECL_INSTANCE ||
            kind == YANG_DECL_INSTANCE_LIST);
}

_YangNode *createReferenceNode(_YangNode *parentPtr, _YangNode *reference, YangNodeType nodeType, int inheritNamespace)
{
    _YangNode *node = (_YangNode*) smiMalloc(sizeof(_YangNode));
    node->nodeType              = nodeType;
    node->export.value          = reference->export.value;
    node->export.nodeKind       = reference->export.nodeKind;
    node->export.config 	= reference->export.config;
    node->export.status 	= reference->export.status;
    node->line                  = reference->line;
    node->export.description	= NULL;
    node->export.reference	= NULL;
    node->export.extra  	= reference->export.extra;
    node->info                  = reference->info;
    node->typeInfo              = NULL;
    node->ctInfo                = NULL;

    node->nextSiblingPtr        = NULL;
    node->firstChildPtr         = NULL;
    node->lastChildPtr          = NULL;
    node->parentPtr             = parentPtr;
    if (!inheritNamespace) {
        node->modulePtr             = parentPtr->modulePtr;
    } else {
        node->modulePtr             = reference->modulePtr;
    }

    if(parentPtr->lastChildPtr)
    {
        (parentPtr->lastChildPtr)->nextSiblingPtr = node;
        parentPtr->lastChildPtr = node;
    }
    else /* first child */
    {
        parentPtr->firstChildPtr = node;
        parentPtr->lastChildPtr = node;
    }
    return node;
}

void copySubtree(_YangNode *destPtr, _YangNode *subtreePtr, YangNodeType nodeType, int skipMandatory, int line, int inheritNamespace) {
    if (skipMandatory && isMandatory(subtreePtr)) {
        smiPrintErrorAtLine(currentParser, ERR_AUGMENTATION_BY_MANDATORY_NODE, subtreePtr->line);
    }
    _YangNode *reference = createReferenceNode(destPtr, subtreePtr, nodeType, inheritNamespace);
    if (line) {
        reference->line = line;
    }
    _YangNode* childPtr = subtreePtr->firstChildPtr;
    while (childPtr) {
        copySubtree(reference, childPtr, nodeType, skipMandatory, line, inheritNamespace);
        childPtr = childPtr->nextSiblingPtr;
    }
}

int isMandatory(_YangNode *nodePtr) {
    _YangNode *mandatory = findChildNodeByType(nodePtr, YANG_DECL_MANDATORY);
    if (mandatory && !strcmp(mandatory->export.value, "true")) {
        return 1;
    }
    return 0;
}

#endif

