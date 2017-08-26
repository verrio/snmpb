/*
 * yang-check.c --
 *
 *      This module contains YANG (semantics) checks 
 *
 * Copyright (c) 2000 Frank Strauss, Technical University of Braunschweig.
 *
 * See the file "COPYING" for information on usage and redistribution
 * of this file, and for a DISCLAIMER OF ALL WARRANTIES.
 *
 * Authors: Kaloyan Kanev, Siarhei Kuryla
 * @(#) $Id: check.c 10751 2008-11-06 22:05:48Z schoenw $
 */


#include <config.h>

#ifdef BACKEND_YANG

#include "yang.h"

#include <string.h>
#include <errno.h>
#include <ctype.h>

#include "yang-data.h"
#include <sys/types.h>

#ifdef HAVE_UNISTD_H
#include <unistd.h>
#endif
#ifdef HAVE_WIN_H
#include "win.h"
#endif

#ifdef HAVE_DMALLOC_H
#include <dmalloc.h>
#endif

#include "yang-data.h"
#include "yang-check.h"
#include "yang-complex-types.h"


/*
 * Current parser defined in parser-yang. Workaround - can't include data.h
 */
extern Parser *currentParser;


static int isPossibleAugmentTargetNode(YangDecl kind) {
    return kind == YANG_DECL_CONTAINER ||
           kind == YANG_DECL_LIST ||
           kind == YANG_DECL_CHOICE ||
           kind == YANG_DECL_CASE ||
           kind == YANG_DECL_INPUT ||
           kind == YANG_DECL_OUTPUT ||
           kind == YANG_DECL_NOTIFICATION ||
           kind == YANG_DECL_INSTANCE ||
           kind == YANG_DECL_INSTANCE_LIST;
}

static int isSchemaNode(YangDecl kind) {
    return kind == YANG_DECL_CONTAINER ||
           kind == YANG_DECL_LEAF ||
           kind == YANG_DECL_LEAF_LIST ||
           kind == YANG_DECL_LIST ||
           kind == YANG_DECL_CHOICE ||
           kind == YANG_DECL_CASE ||
           kind == YANG_DECL_RPC ||
           kind == YANG_DECL_INPUT ||
           kind == YANG_DECL_OUTPUT ||
           kind == YANG_DECL_ANYXML ||
           kind == YANG_DECL_NOTIFICATION ||
           kind == YANG_DECL_INSTANCE ||
           kind == YANG_DECL_INSTANCE_LIST;
}

void yangValidateInclude(_YangNode *module, _YangNode *extModule) {
    _YangNode* node;
    if (!extModule) return;
    node = findChildNodeByType(extModule, YANG_DECL_BELONGS_TO);
    if (node) {
        if (module->export.nodeKind == YANG_DECL_MODULE) {
            if (strcmp(node->export.value, module->export.value)) {
                smiPrintError(currentParser, ERR_SUBMODULE_BELONGS_TO_ANOTHER_MODULE, extModule->export.value, module->export.value);
            }
        } else if (module->export.nodeKind == YANG_DECL_SUBMODULE) {
            _YangNode* node1 = findChildNodeByType(module, YANG_DECL_BELONGS_TO);
            if (strcmp(node->export.value, node1->export.value)) {
                smiPrintError(currentParser, ERR_SUBMODULE_BELONGS_TO_ANOTHER_MODULE, extModule->export.value, node1->export.value);
            }            
        }
    } else {
        smiPrintError(currentParser, ERR_SUBMODULE_BELONGS_TO_ANOTHER_MODULE, extModule->export.value, module->export.value);
    }
}

typedef enum YangIdentifierGroup {
     YANG_IDGR_NONE = 0,
     YANG_IDGR_EXTENSION,
     YANG_IDGR_FEATURE,
     YANG_IDGR_IDENTITY,
     YANG_IDGR_TYPEDEF,
     YANG_IDGR_GROUPING,
     YANG_IDGR_NODE,
     YANG_IDGR_CASE,
     YANG_IDGR_COMPLEX_TYPE
} YangIdentifierGroup;

static int getIdentifierGroup(YangDecl kind) {
    if (kind == YANG_DECL_EXTENSION) {
        return YANG_IDGR_EXTENSION;
    } else if (kind == YANG_DECL_FEATURE) {
        return YANG_IDGR_FEATURE;
    } else if (kind == YANG_DECL_IDENTITY) {
        return YANG_IDGR_IDENTITY;
    } else if (kind == YANG_DECL_TYPEDEF) {
        return YANG_IDGR_TYPEDEF;
    } else if (kind == YANG_DECL_GROUPING) {
        return YANG_IDGR_GROUPING;
    } else if (kind == YANG_DECL_LEAF || 
               kind == YANG_DECL_LEAF_LIST || 
               kind == YANG_DECL_LIST || 
               kind == YANG_DECL_CONTAINER ||
               kind == YANG_DECL_CHOICE ||
               kind == YANG_DECL_RPC ||
               kind == YANG_DECL_NOTIFICATION ||
               kind == YANG_DECL_ANYXML ||
               kind == YANG_DECL_INSTANCE ||
               kind == YANG_DECL_INSTANCE_LIST) {
        return YANG_IDGR_NODE;
    } else if (kind == YANG_DECL_CASE) {
        return YANG_IDGR_CASE;
    } else if (kind == YANG_DECL_COMPLEX_TYPE) {
        return YANG_IDGR_COMPLEX_TYPE;
    }
    return YANG_IDGR_NONE;
}

static char* getModuleName(_YangNode* m1) {
    if (m1->export.nodeKind == YANG_DECL_SUBMODULE) {
        _YangNode *belongsPtr = findChildNodeByType(m1, YANG_DECL_BELONGS_TO);
        if (!belongsPtr) {
            return 0;
        }
        return belongsPtr->export.value;
    } else {
        return m1->export.value;
    }
}

static int sameModules(_YangNode* m1, _YangNode* m2) {
    return !strcmp(getModuleName(m1), getModuleName(m2));
}

static int countChildNodesByTypeAndValue(_YangNode *nodePtr, _YangNode *curNode, YangIdentifierGroup group, _YangNode* namespace, char* value) {
    _YangNode *childPtr = NULL;
    int ret = 0;
    for (childPtr = nodePtr->firstChildPtr; childPtr && childPtr != curNode; childPtr = childPtr->nextSiblingPtr) {       
        if (getIdentifierGroup(childPtr->export.nodeKind) == group && !strcmp(childPtr->export.value, value) && sameModules(childPtr->modulePtr, namespace)) {
            ret++;
        }
    }
    return ret;
}

static int countChoiceChildNodesByTypeAndValue(_YangNode *nodePtr, _YangNode *curNode, YangIdentifierGroup group, _YangNode* namespace, char* value) {
    _YangNode *childPtr = NULL;
    int ret = 0;
    for (childPtr = nodePtr->firstChildPtr; childPtr && childPtr != curNode; childPtr = childPtr->nextSiblingPtr) {
        if (childPtr->export.nodeKind == YANG_DECL_CASE) {
            _YangNode *childPtr2 = childPtr->firstChildPtr;
            for (; childPtr2 && childPtr2 != curNode; childPtr2 = childPtr2->nextSiblingPtr) {
                if (getIdentifierGroup(childPtr2->export.nodeKind) == group && !strcmp(childPtr2->export.value, value) && sameModules(childPtr2->modulePtr, namespace)) {
                    ret++;
                }                
            }
            if (childPtr2 == curNode) break;
        } else {
            if (getIdentifierGroup(childPtr->export.nodeKind) == group && !strcmp(childPtr->export.value, value) && sameModules(childPtr->modulePtr, namespace)) {
                ret++;
            }
        }
    }
    return ret;
}

static int validateNodeUniqueness(_YangNode *nodePtr) {
    YangIdentifierGroup ig = getIdentifierGroup(nodePtr->export.nodeKind);
    _YangNode *cur = nodePtr->parentPtr;
    while (cur) {
        if (cur->export.nodeKind == YANG_DECL_CASE) {
            cur = cur->parentPtr;
            if (countChoiceChildNodesByTypeAndValue(cur, nodePtr, ig, nodePtr->modulePtr, nodePtr->export.value)) {
                return 0;
            }
        } else {
            if (countChildNodesByTypeAndValue(cur, nodePtr, ig, nodePtr->modulePtr, nodePtr->export.value)) {
                return 0;
            }            
        }
        cur = cur->parentPtr;
        if (ig == YANG_IDGR_NODE) {
            break;
        }
    }
    /* check with all submodules if it's a top-level definition or not a data defition statement */
    if (ig != YANG_IDGR_NODE || !nodePtr->parentPtr->parentPtr) {
        YangList* submodules = ((_YangModuleInfo*)nodePtr->modulePtr->info)->submodules;
        while (submodules) {
            if (countChildNodesByTypeAndValue(listNode(submodules), nodePtr, ig, nodePtr->modulePtr, nodePtr->export.value)) {
                return 0;
            }                    
            submodules = submodules->next;
        } 
    }
    return 1;
}

static _YangNode* findTargetNode(_YangNode *nodePtr, char* prefix, char* value) {
    _YangNode *childPtr = NULL;
    /* get module name by prefix */
    char* moduleName = NULL;
    if (!prefix) {
        moduleName = getModuleName(nodePtr->modulePtr);
    } else {
        _YangNode *prefixPtr;
        if (nodePtr->modulePtr->export.nodeKind == YANG_DECL_MODULE) {
            prefixPtr = findChildNodeByType(nodePtr->modulePtr, YANG_DECL_PREFIX);
        } else {
            _YangNode* bt = findChildNodeByType(nodePtr->modulePtr, YANG_DECL_BELONGS_TO);
            if (!bt) {
                return NULL;
            }
            prefixPtr = findChildNodeByType(bt, YANG_DECL_PREFIX);
        }
        if (!strcmp(prefixPtr->export.value, prefix)) {
            moduleName = getModuleName(nodePtr->modulePtr);
        } else {
            moduleName = findYangModuleByPrefix(nodePtr->modulePtr, prefix)->export.value;
        }
    }

    for (childPtr = nodePtr->firstChildPtr; childPtr; childPtr = childPtr->nextSiblingPtr) {
        char* childModuleName = getModuleName(childPtr->modulePtr);
        if (isSchemaNode(childPtr->export.nodeKind) 
	    && ((childPtr->export.value && !strcmp(childPtr->export.value, value)) 
		|| childPtr->export.nodeKind == YANG_DECL_INPUT && !strcmp("input", value)
		|| childPtr->export.nodeKind == YANG_DECL_OUTPUT && !strcmp("output", value))
	    && !strcmp(childModuleName, moduleName)) {
            return childPtr;
        }
    }
    return NULL;
}

static int validatePrefixes(YangList *listPtr, char* modulePrefix, int prefixRequired) {
    YangList *c = listPtr;
    while (c) {
        if (listIdentifierRef(c)->prefix && strcmp(listIdentifierRef(c)->prefix, modulePrefix)) {
            return 0;
        }
        if (!listIdentifierRef(c)->prefix && prefixRequired) {
            return 0;
        }
        c = c->next;
    }
    return 1;
}

/*
 * Resolves a node in the current or imported module by an XPath
 * expression.
 */

static _YangNode *resolveXPath(_YangNode *nodePtr, int allowInstance) {
    _YangNode *cur = NULL, *tmpNode;
    YangList *listPtr = getXPathNode(nodePtr->export.value), *tmp;
    if (!listPtr) return NULL;
    
    cur = NULL;
    if (nodePtr->parentPtr->export.nodeKind == YANG_DECL_COMPLEX_TYPE) {
        cur = nodePtr->parentPtr;
    } else if (nodePtr->parentPtr->export.nodeKind == YANG_DECL_INSTANCE ||
                nodePtr->parentPtr->export.nodeKind == YANG_DECL_INSTANCE_LIST) {
        cur = nodePtr->parentPtr;
    } else if (nodePtr->parentPtr->export.nodeKind == YANG_DECL_USES) {
        /* 'uses' substatement */
        /* let's start from the node which is the parent of the 'uses'*/
        if (!validatePrefixes(listPtr, getModuleInfo(nodePtr->modulePtr)->prefix, 0)) {
            freeIdentiferList(listPtr);
            return NULL;            
        }
        cur = nodePtr->parentPtr->parentPtr;
    } else {
        /* 'module' substatement */
        _YangModuleInfo *info = getModuleInfo(nodePtr->modulePtr);
        if (listIdentifierRef(listPtr)->prefix && strcmp(listIdentifierRef(listPtr)->prefix, info->prefix)) {
            cur = findYangModuleByPrefix(nodePtr->modulePtr, listIdentifierRef(listPtr)->prefix);
            if (!cur) {
                freeIdentiferList(listPtr);
                return NULL;
            }
            if (!validatePrefixes(listPtr, listIdentifierRef(listPtr)->prefix, 1)) {
                freeIdentiferList(listPtr);
                return NULL;
            }            
        } else {            
            cur = nodePtr->modulePtr;
            if (!validatePrefixes(listPtr, getModuleInfo(cur)->prefix, 0)) {
                freeIdentiferList(listPtr);
                return NULL;
            }
        }
    }
    tmp = listPtr;
    YangList* submodules = NULL;
    while (listPtr) {
        if ((cur->export.nodeKind == YANG_DECL_INSTANCE || cur->export.nodeKind == YANG_DECL_INSTANCE_LIST) && !allowInstance) {
            return NULL;
        }
        tmpNode = cur;
        cur = findTargetNode(tmpNode, listIdentifierRef(listPtr)->prefix, listIdentifierRef(listPtr)->ident);
        /* try to instantiate instance/instance-list */
        if (!cur && (nodePtr->export.nodeKind == YANG_DECL_AUGMENT || nodePtr->export.nodeKind == YANG_DECL_DEVIATION) &&
                (tmpNode->export.nodeKind == YANG_DECL_INSTANCE || tmpNode->export.nodeKind == YANG_DECL_INSTANCE_LIST)
                && tmpNode->export.status == YANG_PARSING_IN_PROGRESS) {
            expandInstance(tmpNode, 1);
            cur = findTargetNode(tmpNode, listIdentifierRef(listPtr)->prefix, listIdentifierRef(listPtr)->ident);
        }
        if (!cur) {
            if (submodules) {                
                cur = listNode(submodules);
                submodules = submodules->next;
                continue;
            }
            if (tmpNode->export.nodeKind == YANG_DECL_MODULE || tmpNode->export.nodeKind == YANG_DECL_SUBMODULE) {
                submodules = getModuleInfo(tmpNode)->submodules;
                if (submodules) {
                    cur = listNode(submodules);
                    submodules = submodules->next;
                    continue;                
                }
            }
            
            freeIdentiferList(tmp);
            return NULL;
        }
        listPtr = listPtr->next;
    }
    freeIdentiferList(tmp);
    return cur;
}

static int isAllowedStatement(int stmt, int *allowedStmts, int len) {
    int i = 0;
    for (; i < len; i++)
        if (allowedStmts[i] == stmt) {
            return 1;
        }
    return 0;
}

static void applyRefine(_YangNode* target, _YangNode* refinement, int* allowedStmts, int len) {
    _YangNode *child = refinement->firstChildPtr;
    
    while (child) {
        if (!isAllowedStatement(child->export.nodeKind, allowedStmts, len)) {
            smiPrintErrorAtLine(currentParser, ERR_INVALID_REFINE, child->line, yangDeclAsString(target->export.nodeKind), target->export.value, yangDeclAsString(child->export.nodeKind));
        } else {
            if (child->export.nodeKind == YANG_DECL_MUST) {
                copySubtree(target, child, YANG_NODE_REFINED, 0, refinement->line, 0);
            } else if (child->export.nodeKind == YANG_DECL_DESCRIPTION || 
                child->export.nodeKind == YANG_DECL_REFERENCE) {
                /* just skip, because they are not relevant for the future checks */
            } else if (child->export.nodeKind == YANG_DECL_PRESENCE) {
                if (!findChildNodeByType(target, child->export.nodeKind)) {
                    copySubtree(target, child, YANG_NODE_REFINED, 0, refinement->line, 0);
                }
            } else if (child->export.nodeKind == YANG_DECL_CONFIG ||
                    child->export.nodeKind == YANG_DECL_DEFAULT || 
                    child->export.nodeKind == YANG_DECL_MANDATORY || 
                    child->export.nodeKind == YANG_DECL_MIN_ELEMENTS ||
                    child->export.nodeKind == YANG_DECL_MAX_ELEMENTS) {
                _YangNode *oldOne = findChildNodeByType(target, child->export.nodeKind);
                if (oldOne) {
                    smiFree(oldOne->export.value);
                    oldOne->export.value = smiStrdup(child->export.value);
                } else {
                    copySubtree(target, child, YANG_NODE_REFINED, 0, refinement->line, 0);
                    oldOne = child;
                }
                if (oldOne->export.nodeKind == YANG_DECL_CONFIG) {
                    if (!strcmp(oldOne->export.value, "true")) {
                        setConfig(target, YANG_CONFIG_TRUE);
                    } else {  
                        setConfig(target, YANG_CONFIG_FALSE);
                    }
                }
            }
        }
        child = child->nextSiblingPtr;
    }
}

void applyRefinements(_YangNode* node) {
    /* Apply refinements if there are any  */
    _YangNode *child = node->firstChildPtr;
    while (child) {
        if (child->export.nodeKind == YANG_DECL_REFINE) {
            _YangNode* refinement = child;
            if (refinement) {
                _YangNode *targetNodePtr = resolveXPath(refinement, 0);

                if (!targetNodePtr) {
                    smiPrintErrorAtLine(currentParser, ERR_BAD_REFINE_ARG, refinement->line, refinement->export.value);
                } else {
                    /* check whether the target is an inherited node */
                    if (node->export.nodeKind == YANG_DECL_COMPLEX_TYPE && targetNodePtr->nodeType != YANG_NODE_INHERITED) {
                        smiPrintErrorAtLine(currentParser, ERR_CT_REFINE, refinement->line, targetNodePtr->export.value);
                    } else {
                        if (targetNodePtr->export.nodeKind == YANG_DECL_CONTAINER) {
                            int types[] = {YANG_DECL_MUST, YANG_DECL_PRESENCE, YANG_DECL_CONFIG, YANG_DECL_DESCRIPTION, YANG_DECL_REFERENCE};
                            applyRefine(targetNodePtr, refinement, types, 4);
                        } else if (targetNodePtr->export.nodeKind == YANG_DECL_LEAF) {
                            int types[] = {YANG_DECL_MUST, YANG_DECL_DEFAULT, YANG_DECL_CONFIG, YANG_DECL_MANDATORY, YANG_DECL_DESCRIPTION, YANG_DECL_REFERENCE};
                            applyRefine(targetNodePtr, refinement, types, 6);
                        } else if (targetNodePtr->export.nodeKind == YANG_DECL_LEAF_LIST || targetNodePtr->export.nodeKind == YANG_DECL_LIST) {
                            int types[] = {YANG_DECL_MUST, YANG_DECL_CONFIG, YANG_DECL_MIN_ELEMENTS, YANG_DECL_MAX_ELEMENTS, YANG_DECL_DESCRIPTION, YANG_DECL_REFERENCE};
                            applyRefine(targetNodePtr, refinement, types, 6);
                        } else if (targetNodePtr->export.nodeKind == YANG_DECL_CHOICE) {
                            int types[] = {YANG_DECL_DEFAULT, YANG_DECL_CONFIG, YANG_DECL_MANDATORY, YANG_DECL_DESCRIPTION, YANG_DECL_REFERENCE};
                            applyRefine(targetNodePtr, refinement, types, 5);
                        } else if (targetNodePtr->export.nodeKind == YANG_DECL_CASE) {
                            int types[] = {YANG_DECL_DESCRIPTION, YANG_DECL_REFERENCE};
                            applyRefine(targetNodePtr, refinement, types, 2);
                        } else if (targetNodePtr->export.nodeKind == YANG_DECL_ANYXML) {
                            int types[] = {YANG_DECL_CONFIG, YANG_DECL_MANDATORY, YANG_DECL_DESCRIPTION, YANG_DECL_REFERENCE};
                            applyRefine(targetNodePtr, refinement, types, 4);
                        } else if (targetNodePtr->export.nodeKind == YANG_DECL_INSTANCE) {
                            int types[] = {YANG_DECL_CONFIG, YANG_DECL_MANDATORY, YANG_DECL_DESCRIPTION, YANG_DECL_REFERENCE, YANG_DECL_MUST};
                            applyRefine(targetNodePtr, refinement, types, 5);
                        } else if (targetNodePtr->export.nodeKind == YANG_DECL_INSTANCE_LIST) {
                            int types[] = {YANG_DECL_CONFIG, YANG_DECL_DESCRIPTION, YANG_DECL_REFERENCE, YANG_DECL_MUST, YANG_DECL_MIN_ELEMENTS, YANG_DECL_MAX_ELEMENTS};
                            applyRefine(targetNodePtr, refinement, types, 6);
                        }
                    }
                }
            }
        }
        child = child->nextSiblingPtr;
    }
}

/*
 * From the specification:
 *  1. The effect of a "uses" reference to a grouping is that the
 *      nodes defined by the grouping are copied into the current
 *      schema tree and then updated according to the refinement
 *      statements.
 *
 *  2. Once a grouping is defined, it can be referenced in a "uses"
 *      statement (see Section 7.12).  A grouping MUST NOT reference
 *      itself, neither directly nor indirectly through a chain of
 *      other groupings.
 */

static int expandGroupings(_YangNode *node) {
    YangDecl nodeKind;
    if (!node || node->nodeType != YANG_NODE_ORIGINAL) return 0;
    nodeKind = node->export.nodeKind;
    if (nodeKind == YANG_DECL_GROUPING) {
        if (node->info) {
            _YangGroupingInfo *info = (_YangGroupingInfo*)node->info;
            if (info->state == YANG_PARSING_IN_PROGRESS) {
                smiPrintErrorAtLine(currentParser, ERR_CYCLIC_REFERENCE_CHAIN, node->line, yangDeclAsString(node->export.nodeKind), node->export.value);
                return 0;
            }
            return 1;
        }
        _YangGroupingInfo *info = smiMalloc(sizeof(_YangGroupingInfo));
        info->state = YANG_PARSING_IN_PROGRESS;
        node->info = info;
    }
    if (nodeKind == YANG_DECL_USES) {
        _YangIdentifierRefInfo* info = (_YangIdentifierRefInfo*)node->info;
        if (info->resolvedNode) {
            if (expandGroupings(info->resolvedNode)) {            
                _YangNode *refChild = info->resolvedNode->firstChildPtr;
                while (refChild) {
                    if (isDataDefinitionNode(refChild)) {
                        copySubtree(node->parentPtr, refChild, YANG_NODE_EXPANDED_USES, 0, node->line, 0);
                    }
                    refChild = refChild->nextSiblingPtr;
                }
                applyRefinements(node);
            }
        }
    }
    
    _YangNode *child = node->firstChildPtr;
    while (child) {
        expandGroupings(child);
        child = child->nextSiblingPtr;
    }
    if (nodeKind == YANG_DECL_GROUPING) {
        ((_YangGroupingInfo*)node->info)->state = YANG_PARSING_DONE;
    }
    return 1;
}

/*
 * Verifies that all identifiers are unique within all namespaces.
 */

static void uniqueNames(_YangNode* nodePtr) { 
    /* go over all child nodes */
    _YangNode* cur = nodePtr->firstChildPtr;
    while (cur) {
        YangIdentifierGroup yig = getIdentifierGroup(cur->export.nodeKind);
        if (yig > YANG_IDGR_NONE) {            
            if (!validateNodeUniqueness(cur)) {
                if (cur->nodeType == YANG_NODE_EXPANDED_USES) {
                    smiPrintErrorAtLine(currentParser, ERR_DUPLICATED_NODE_WHILE_GROUPING_INSTANTIATION, cur->line, cur->export.value);
                } else if (cur->nodeType == YANG_NODE_EXPANDED_AUGMENT) {
                    smiPrintErrorAtLine(currentParser, ERR_DUPLICATED_NODE_WHILE_AUGMENT_INSTANTIATION, cur->line, cur->export.value);
                } else if (cur->nodeType == YANG_NODE_INHERITED) {
                    smiPrintErrorAtLine(currentParser, ERR_INHERITED_DUPLICATED_IDENTIFIER, cur->line, cur->export.value);
                } else {
                    smiPrintErrorAtLine(currentParser, ERR_DUPLICATED_IDENTIFIER, cur->line, cur->export.value);
                }
            }
        }
        uniqueNames(cur);
        cur = cur->nextSiblingPtr;
    }
}

/*
 * Should be reimplemented in a more efficient way by using some
 * appropriate datastructures, like HashTable
 */

static void uniqueSubmoduleDefinitions(_YangNode* modulePtr) {
    /* validate name uniqueness of the top level definitions in all submodules */
    YangList* submodulePtr = ((_YangModuleInfo*)modulePtr->info)->submodules, *firstSubmodulePtr;
    firstSubmodulePtr = submodulePtr;
    while (submodulePtr) {
        _YangNode* curNodePtr = listNode(submodulePtr)->firstChildPtr;
        while (curNodePtr) {
            YangIdentifierGroup ig = getIdentifierGroup(curNodePtr->export.nodeKind);
            if (ig > YANG_IDGR_NONE) {
               YangList* sPtr = firstSubmodulePtr;
                while (sPtr != submodulePtr) {
                    if (countChildNodesByTypeAndValue(listNode(sPtr), curNodePtr, ig, curNodePtr->modulePtr, curNodePtr->export.value)) {
                        smiPrintErrorAtLine(((Parser*)getModuleInfo(listNode(submodulePtr))->parser), ERR_IDENTIFIER_DEFINED_IN_OTHER_SUBMODLE, curNodePtr->line, curNodePtr->export.value, listNode(sPtr)->export.value);
                    }
                    sPtr = sPtr->next;
                }
            }
            curNodePtr = curNodePtr->nextSiblingPtr;
        }        
        submodulePtr= submodulePtr->next;
    }   
}

static int map[YANG_DECL_LAST];

static void initMap() {
    map[YANG_DECL_UNKNOWN_STATEMENT] = YANG_DECL_EXTENSION;
    map[YANG_DECL_IF_FEATURE] = YANG_DECL_FEATURE;
    map[YANG_DECL_TYPE] = YANG_DECL_TYPEDEF;
    map[YANG_DECL_USES] = YANG_DECL_GROUPING;
    map[YANG_DECL_BASE] = YANG_DECL_IDENTITY;
    /* CT-EXT*/
    map[YANG_DECL_EXTENDS] = YANG_DECL_COMPLEX_TYPE;
    map[YANG_DECL_INSTANCE_TYPE] = YANG_DECL_COMPLEX_TYPE;
}

 /* 
  * Resolves references to extensions, features, defined types,
  * groupings and identities.  Validates whether there are no circular
  * dependencies between these nodes.
  */  

static void resolveReferences(_YangNode* node) {
    YangDecl nodeKind = node->export.nodeKind;
    if (nodeKind == YANG_DECL_UNKNOWN_STATEMENT ||
        nodeKind == YANG_DECL_IF_FEATURE ||
        (nodeKind == YANG_DECL_TYPE && getBuiltInType(node->export.value) == YANG_TYPE_NONE) ||
        nodeKind == YANG_DECL_USES ||
        nodeKind == YANG_DECL_BASE ||
        nodeKind == YANG_DECL_EXTENDS ||
        nodeKind == YANG_DECL_INSTANCE_TYPE) {            
            _YangIdentifierRefInfo* identifierRef = (_YangIdentifierRefInfo*)node->info;
            if (identifierRef && !identifierRef->resolvedNode) {
                _YangNode *reference = resolveReference(node->parentPtr, map[nodeKind], identifierRef->prefix, identifierRef->identifierName);
                if (!reference) {                    
                    smiPrintErrorAtLine(currentParser, ERR_REFERENCE_NOT_RESOLVED, node->line, identifierRef->prefix, identifierRef->identifierName);
                }
                identifierRef->resolvedNode = reference;
                identifierRef->marker = node;
                /* store a base type */
                if (nodeKind == YANG_DECL_TYPE) {
                    node->typeInfo->baseTypeNodePtr = reference;
                }
                if (nodeKind == YANG_DECL_UNKNOWN_STATEMENT) {
                    /* check the argument */
                    _YangNode *argument = findChildNodeByType(reference, YANG_DECL_ARGUMENT);
                    if (argument && !node->export.extra) {
                        smiPrintErrorAtLine(currentParser, ERR_EXPECTED_EXTENSION_ARGUMENT, node->line, node->export.value);
                    } else if (!argument && node->export.extra) {
                        smiPrintErrorAtLine(currentParser, ERR_UNEXPECTED_EXTENSION_ARGUMENT, node->line, node->export.value);
                    }
                } else if (nodeKind == YANG_DECL_IF_FEATURE || nodeKind == YANG_DECL_BASE || nodeKind == YANG_DECL_TYPE ||
                            nodeKind == YANG_DECL_EXTENDS) {
                    /* check whether there is no cyclic reference */
                    if (node->parentPtr->export.nodeKind == map[nodeKind]) {
                        _YangNode *cur = identifierRef->resolvedNode;
                        while (cur) {
                            _YangNode *childRef = findChildNodeByType(cur, nodeKind);
                            /* Skip basic types, they don't have info */
                            if (childRef && childRef->info) {
                                _YangIdentifierRefInfo* info = ((_YangIdentifierRefInfo*)childRef->info);
                                if (!info->marker) {
                                        info->resolvedNode = resolveReference(node->parentPtr, map[nodeKind], info->prefix, info->identifierName);
                                        if (!info->resolvedNode) {
                                            smiPrintErrorAtLine(currentParser, ERR_REFERENCE_NOT_RESOLVED, childRef->line, info->prefix, info->identifierName);
                                        }
                                        /* store a base type */
                                        if (nodeKind == YANG_DECL_TYPE) {
                                            node->typeInfo->baseTypeNodePtr = info->resolvedNode;
                                        }
                                        
                                        info->marker = node;
                                        cur = info->resolvedNode;
                                } else {
                                    if (info->marker == node) {
                                        smiPrintErrorAtLine(currentParser, ERR_CYCLIC_REFERENCE_CHAIN, node->parentPtr->line, yangDeclAsString(map[nodeKind]), node->parentPtr->export.value);
                                        info->loop = 1;
                                    }
                                    break;
                                }                                     
                            } else {
                                break;
                            }
                        }
                    }
                }
            }
    }

    _YangNode *child = node->firstChildPtr;
    while (child) {
        resolveReferences(child);
        child = child->nextSiblingPtr;
    }
}

/*
 * Expands all augment statements.
 */
void expandAugment(_YangNode* node, int allowInstance) {
    /* resolve XPath to get the target node */
   _YangNode *targetNodePtr = resolveXPath(node, allowInstance);

   if (!targetNodePtr || (!allowInstance && (targetNodePtr->export.nodeKind == YANG_DECL_INSTANCE ||
                targetNodePtr->export.nodeKind == YANG_DECL_INSTANCE_LIST))) {
       smiPrintErrorAtLine(currentParser, ERR_BAD_AUGMENT_ARG, node->line, node->export.value);
   } else {
        /* check whether the target node is of required type */
        if (!isPossibleAugmentTargetNode(targetNodePtr->export.nodeKind)) {
            smiPrintErrorAtLine(currentParser, ERR_WRONG_AUGMENT_TARGET_NODE, node->line, node->export.value);
            return;
        }
        /* expand augment */
        _YangNode *child = node->firstChildPtr;

        /*
         *  From the specification:
         *  If the target node of the “augment” is in another module,
         *  then nodes added by the augmentation MUST NOT be mandatory nodes.
         */
        int isAnotherModule = 1;
        if (!strcmp(getModuleInfo(targetNodePtr->modulePtr)->prefix, getModuleInfo(node->modulePtr)->prefix)) {
            isAnotherModule = 0;
        }
        while (child) {
            if (isAnotherModule) {
                if (isMandatory(child)) {
                    smiPrintErrorAtLine(currentParser, ERR_AUGMENTATION_BY_MANDATORY_NODE, node->line);
                    child = child->nextSiblingPtr;
                    continue;
                }

                YangIdentifierGroup ig = getIdentifierGroup(child->export.nodeKind);
                if (ig != YANG_IDGR_NONE && countChildNodesByTypeAndValue(targetNodePtr, NULL, ig, child->modulePtr, child->export.value)) {
                    smiPrintErrorAtLine(currentParser, ERR_DUPLICATED_NODE_WHILE_AUGMENT_INSTANTIATION, child->line, child->export.value);
                    child = child->nextSiblingPtr;
                    continue;
                }
            }

            /*
             * If the target node is a container, list, case, input, output, or notification node,
             * the "container", "leaf", "list", "leaf-list", "uses", and "choice" statements can be used within the "augment" statement.
             * If the target node is a choice node, the "case" statement can be used within the "augment" statement.
             */
            if (child->export.nodeKind == YANG_DECL_ANYXML) {
                smiPrintErrorAtLine(currentParser, ERR_NODE_KIND_NOT_ALLOWED, child->line, yangDeclAsString(YANG_DECL_ANYXML), child->export.value, yangDeclAsString(targetNodePtr->export.nodeKind), targetNodePtr->export.value);
            } else if (isDataDefNode(child)) {
                if (targetNodePtr->export.nodeKind == YANG_DECL_CHOICE) {
                    smiPrintErrorAtLine(currentParser, ERR_NODE_KIND_NOT_ALLOWED, child->line, yangDeclAsString(child->export.nodeKind), child->export.value, yangDeclAsString(targetNodePtr->export.nodeKind), targetNodePtr->export.value);
                }
                /*
                 *  If the target node is in the external module we should check whether adding this node does not break uniqueness
                 *  (because all imported modules have been already validated)
                 */
                copySubtree(targetNodePtr, child, YANG_NODE_EXPANDED_AUGMENT, isAnotherModule, 0, 0);
            } else if (child->export.nodeKind == YANG_DECL_CASE) {
                if (targetNodePtr->export.nodeKind != YANG_DECL_CHOICE) {
                    smiPrintErrorAtLine(currentParser, ERR_NODE_KIND_NOT_ALLOWED, child->line, yangDeclAsString(child->export.nodeKind), child->export.value, yangDeclAsString(targetNodePtr->export.nodeKind), targetNodePtr->export.value);
                }
                copySubtree(targetNodePtr, child, YANG_NODE_EXPANDED_AUGMENT, isAnotherModule, 0, 0);
            }
            child = child->nextSiblingPtr;
        }
   }
}

static void expandAugments(_YangNode* node) {
    _YangNode *child = node->firstChildPtr;
    while (child) {
        expandAugments(child);
        child = child->nextSiblingPtr;
    }
    if (node->export.nodeKind == YANG_DECL_AUGMENT
	&& (node->parentPtr->export.nodeKind != YANG_DECL_INSTANCE && 
            node->parentPtr->export.nodeKind != YANG_DECL_INSTANCE_LIST)) {
        expandAugment(node, 0);
    }
}

static int isSingleton(_YangNode* node) {
    int kind = node->export.nodeKind;
    return kind == YANG_DECL_CONFIG ||
           kind == YANG_DECL_UNITS ||
           kind == YANG_DECL_UNIQUE ||
           kind == YANG_DECL_MANDATORY ||
           kind == YANG_DECL_MIN_ELEMENTS ||
           kind == YANG_DECL_MAX_ELEMENTS ||
           kind == YANG_DECL_DEFAULT;
}

static int isValidDeviation(int nodeType, int deviationNodeType) {
    switch (deviationNodeType) {
        case YANG_DECL_TYPE:
            if (nodeType == YANG_DECL_LEAF || nodeType == YANG_DECL_LEAF_LIST) {
                return 1;
            }
            break;
        case YANG_DECL_UNITS:
            if (nodeType == YANG_DECL_LEAF || nodeType == YANG_DECL_LEAF_LIST) {
                return 1;
            }
            break;
        case YANG_DECL_DEFAULT:
            if (nodeType == YANG_DECL_LEAF || nodeType == YANG_DECL_CHOICE) {
                return 1;
            }
            break;
        case YANG_DECL_CONFIG:
            if (nodeType == YANG_DECL_LEAF || nodeType == YANG_DECL_CHOICE
                    || nodeType == YANG_DECL_CONTAINER || nodeType == YANG_DECL_LIST
                    || nodeType == YANG_DECL_LEAF_LIST || nodeType == YANG_DECL_INSTANCE ||
                    nodeType == YANG_DECL_INSTANCE_LIST) {
                return 1;
            }
            break;
        case YANG_DECL_MANDATORY:
            if (nodeType == YANG_DECL_LEAF || nodeType == YANG_DECL_CHOICE || nodeType == YANG_DECL_INSTANCE) {
                return 1;
            }
            break;
        case YANG_DECL_MIN_ELEMENTS:
            if (nodeType == YANG_DECL_LIST || nodeType == YANG_DECL_LEAF_LIST ||
                    nodeType == YANG_DECL_INSTANCE_LIST) {
                return 1;
            }
            break;
        case YANG_DECL_MAX_ELEMENTS:
            if (nodeType == YANG_DECL_LIST || nodeType == YANG_DECL_LEAF_LIST ||
                    nodeType == YANG_DECL_INSTANCE_LIST) {
                return 1;
            }
            break;
        case YANG_DECL_UNIQUE:
            if (nodeType == YANG_DECL_LIST) {
                return 1;
            }
            break;
        case YANG_DECL_MUST:
            if (nodeType == YANG_DECL_LEAF || nodeType == YANG_DECL_CHOICE
                    || nodeType == YANG_DECL_CONTAINER || nodeType == YANG_DECL_LIST
                    || nodeType == YANG_DECL_LEAF_LIST || nodeType == YANG_DECL_INSTANCE ||
                    nodeType == YANG_DECL_INSTANCE_LIST) {
                return 1;
            }
            break;
    }
    return 0;
}

static void expandDeviation(_YangNode* node) {
    /* resolve XPath to get the target node */
    _YangNode *targetNodePtr = resolveXPath(node, 1);

    if (!targetNodePtr) {
        smiPrintErrorAtLine(currentParser, ERR_BAD_DEVIATION_ARG, node->line, node->export.value);
    } else {
        _YangNode* deviateNode = findChildNodeByType(node, YANG_DECL_DEVIATE);
        if (!strcmp(deviateNode->export.value, "not-supported")) {
            return;
        }

        _YangNode *child = deviateNode->firstChildPtr;
        while (child) {
            if (!strcmp(deviateNode->export.value, "add")) {
                if (isSingleton(child)) {
                    _YangNode* existingNode = findChildNodeByType(targetNodePtr, child->export.nodeKind);
                    if (existingNode) {
                        smiPrintErrorAtLine(currentParser, ERR_BAD_DEVIATION_ADD, child->line, yangDeclAsString(child->export.nodeKind));
                        return;
                    }
                }
                if (!isValidDeviation(targetNodePtr->export.nodeKind, child->export.nodeKind)) {
                    smiPrintErrorAtLine(currentParser, ERR_BAD_DEVIATION_TYPE, child->line, yangDeclAsString(child->export.nodeKind), yangDeclAsString(targetNodePtr->export.nodeKind));
                    return;
                }
                copySubtree(targetNodePtr, child, YANG_NODE_DEVIATION, 0, child->line, 0);
            } else if (!strcmp(deviateNode->export.value, "delete")) {
                _YangNode* old = findChildNodeByTypeAndValue(targetNodePtr, child->export.nodeKind, child->export.value);
                if (!old) {
                    smiPrintErrorAtLine(currentParser, ERR_BAD_DEVIATION_DEL, child->line, yangDeclAsString(child->export.nodeKind));
                } else {
                    removeYangNode(targetNodePtr, old);
                }
            } else  if (!strcmp(deviateNode->export.value, "replace")) {
                _YangNode* old = findChildNodeByType(targetNodePtr, child->export.nodeKind);
                if (!old) {
                    smiPrintErrorAtLine(currentParser, ERR_BAD_DEVIATION_DEL, child->line, yangDeclAsString(child->export.nodeKind));
                } else {
                    removeYangNode(targetNodePtr, old);
                    copySubtree(targetNodePtr, child, YANG_NODE_DEVIATION, 0, child->line, 0);
                }
            }
            child = child->nextSiblingPtr;
        }
    }
}

/*
 * From the specification:
 * If a node has "config" "false", no node underneath it can have
 * "config" set to "true".  If a "config" statement is present for
 * any node in the input, output or notification tree, it is ignored.
 */

static void validateConfigProperties(_YangNode *nodePtr, YangConfig configValue, int ignore) {
    int ignoreFlag = ignore;
    if (nodePtr->export.nodeKind != YANG_DECL_GROUPING) {
        if (ignore) {
            if (nodePtr->export.nodeKind == YANG_DECL_CONFIG) {
                smiPrintErrorAtLine(currentParser, ERR_IGNORED_CONFIG, nodePtr->line);
            }
        } else if (configValue == YANG_CONFIG_FALSE) {
            if (nodePtr->export.config == YANG_CONFIG_TRUE) {
                smiPrintErrorAtLine(currentParser, ERR_INVALID_CONFIG, nodePtr->line, nodePtr->export.value);
            }
            nodePtr->export.config = YANG_CONFIG_FALSE;
        } else if (configValue == YANG_CONFIG_TRUE) {
            if (nodePtr->export.config == YANG_CONFIG_DEFAULT) {
                nodePtr->export.config = YANG_CONFIG_TRUE;
            }
        }
        if (nodePtr->export.nodeKind == YANG_DECL_INPUT ||
            nodePtr->export.nodeKind == YANG_DECL_OUTPUT ||
            nodePtr->export.nodeKind == YANG_DECL_NOTIFICATION) {
            ignoreFlag = 1;
        }
    }
    _YangNode *childPtr = NULL;
    for (childPtr = nodePtr->firstChildPtr; childPtr; childPtr = childPtr->nextSiblingPtr) {
        validateConfigProperties(childPtr, nodePtr->export.config, ignoreFlag);
    }
}

static void validateLists(_YangNode *nodePtr) {
    if (nodePtr->export.nodeKind == YANG_DECL_LIST) {
        /*
         *  From the specification:
         *  The "key" statement, which MUST be present if the list represents configuration, and MAY be present otherwise, 
         *  takes as an argument a string which specifies a space separated list of leaf identifiers of this list.  
         *  Each such leaf identifier MUST refer to a child leaf of the list.  A leaf that is part of the key can be of any built-in or derived type, except it MUST NOT be the built-in type "empty". 
         *  All key leafs in a list MUST have the same value for their "config"  as the list itself. 
         */
        _YangNode *key = findChildNodeByType(nodePtr, YANG_DECL_KEY);
        if (yangIsTrueConf(nodePtr->export.config)) {
            if (!key) {
                smiPrintErrorAtLine(currentParser, ERR_KEY_REQUIRED, nodePtr->line, nodePtr->export.value);
            }
        }        
        if (key) {
            YangList *keys = (YangList*)key->info;
            while (keys) {
                _YangNode *leafPtr = findChildNodeByTypeAndValue(nodePtr, YANG_DECL_LEAF, listIdentifierRef(keys)->ident);
                if (!leafPtr) {
                    smiPrintErrorAtLine(currentParser, ERR_INVALID_KEY_REFERENCE, key->line, listIdentifierRef(keys)->ident);
                } else {
                    _YangNode *type = findChildNodeByType(leafPtr, YANG_DECL_TYPE);
                    if (!strcmp(type->export.value, "empty")) {
                        smiPrintErrorAtLine(currentParser, ERR_EMPTY_KEY, key->line, leafPtr->export.value);
                    }
                    if (yangIsTrueConf(nodePtr->export.config) != yangIsTrueConf(leafPtr->export.config)) {
                        smiPrintErrorAtLine(currentParser, ERR_INVALID_KEY_LEAF_CONFIG_VALUE, key->line, leafPtr->export.value, nodePtr->export.value);
                    }
                }
                keys = keys->next;
            }
        }

        /*
         *  From the specification:  
         *  The "unique" statement takes as an argument a string which contains a space separated list of schema node identifiers, 
         *  which MUST be given in the descendant form. Each such schema node identifier MUST refer to a leaf. 
         *  If one of the referenced leafs represents configuration data, then all of the referenced leafs MUST represent configuration data. 
         */
        _YangNode *childPtr = nodePtr->firstChildPtr;
        while (childPtr) {            
            if (childPtr->export.nodeKind == YANG_DECL_UNIQUE) {
                _YangList* l = (_YangList*)childPtr->info;
                int configNodeCount = 0, stateNodeCount = 0;
                while (l) {                    
                    YangList* il = (YangList*)l->data;
                    _YangNode* cur = nodePtr;
                    while (il) {
                        cur = cur->firstChildPtr;                        
                        while (cur) {
                            if (isDataDefNode(cur) && !strcmp(cur->export.value, listIdentifierRef(il)->ident)) break;
                            cur = cur->nextSiblingPtr;
                        }
                        if (!cur) {
                            smiPrintErrorAtLine(currentParser, ERR_INVALID_UNIQUE_REFERENCE, childPtr->line, l->additionalInfo);
                            break;
                        }
                        il = il->next;
                    }
                    if (cur && cur->export.nodeKind != YANG_DECL_LEAF) {
                        smiPrintErrorAtLine(currentParser, ERR_INVALID_UNIQUE_REFERENCE, childPtr->line, l->additionalInfo);
                        break;
                    } else if (cur) {
                        if (yangIsTrueConf(cur->export.config)) {
                            configNodeCount++;
                        } else {
                            stateNodeCount++;
                        }
                    }                    
                    l = l->next;
                }
                /*
                 *  If one of the referenced leafs represents configuration data, 
                 *  then all of the referenced leafs MUST represent configuration data.
                 */                
                if (configNodeCount && stateNodeCount) {
                    smiPrintErrorAtLine(currentParser, ERR_MUST_BE_CONFIG, childPtr->line);
                }
            }
            childPtr = childPtr->nextSiblingPtr;
        }
    }
    _YangNode *childPtr = NULL;
    for (childPtr = nodePtr->firstChildPtr; childPtr; childPtr = childPtr->nextSiblingPtr) {
        validateLists(childPtr);
    }    
}

static void validateDefaultStatements(_YangNode *nodePtr) {
    YangDecl nodeKind = nodePtr->export.nodeKind;
    if (nodeKind == YANG_DECL_DEFAULT) {
        YangDecl parentKind = nodePtr->parentPtr->export.nodeKind;

        /* An empty type cannot have a default value. */
        if (parentKind == YANG_DECL_LEAF || parentKind == YANG_DECL_TYPEDEF) {
            _YangNode* typePtr = findChildNodeByType(nodePtr->parentPtr, YANG_DECL_TYPE);
            if (getBuiltInType(typePtr->export.value) == YANG_TYPE_EMPTY) {
                smiPrintErrorAtLine(currentParser, ERR_DEFAULT_NOT_ALLOWED, nodePtr->line);
            }
        }

        /* The "default" statement of the leaf and choice MUST NOT be present where "mandatory" is true. */
        if (parentKind == YANG_DECL_CHOICE || parentKind == YANG_DECL_LEAF) {
            _YangNode* mandatory = findChildNodeByType(nodePtr->parentPtr, YANG_DECL_MANDATORY);
            if (mandatory && !strcmp(mandatory->export.value, "true")) {
                smiPrintErrorAtLine(currentParser, ERR_IVALIDE_DEFAULT, nodePtr->line);
            }
        }
        
        /*
         * The argument of the choice is the identifier of the "case" statement.
         * There MUST NOT be any mandatory nodes directly under the default case. 
         */
        if (parentKind == YANG_DECL_CHOICE) {
            _YangNode *defaultCase = findChildNodeByTypeAndValue(nodePtr->parentPtr, YANG_DECL_CASE, nodePtr->export.value);
            if (!defaultCase) {
                smiPrintErrorAtLine(currentParser, ERR_IVALIDE_DEFAULT_CASE, nodePtr->line, nodePtr->export.value);
            } else {
               /* Mandatory nodes:
                * 1. A leaf or choice node with a "mandatory" statement with the value "true".
                * 2. A list or leaf-list node with a "min-elements" statement with a value greater than zero.
                * 3. A container node without a "presence" statement.
                */                
                _YangNode *childPtr = NULL;
                _YangNode *mandatory = NULL;
                for (childPtr = defaultCase->firstChildPtr; childPtr; childPtr = childPtr->nextSiblingPtr) {
                    mandatory = findChildNodeByTypeAndValue(childPtr, YANG_DECL_MANDATORY, "true");
                    if (!mandatory) {
                        mandatory = findChildNodeByType(childPtr, YANG_DECL_PRESENCE);
                    }
                    if (!mandatory) {
                        mandatory = findChildNodeByType(childPtr, YANG_DECL_MIN_ELEMENTS);
                        if (mandatory->export.value[0] == '0') {
                            mandatory = NULL;
                        }
                    }
                    if (mandatory) break;                   
                }        
                smiPrintErrorAtLine(currentParser, ERR_MANDATORY_NODE_UNDER_DEFAULT_CASE, nodePtr->line, defaultCase->export.value, nodePtr->parentPtr->export.value);
            }
        }
    }
    _YangNode *childPtr = NULL;
    for (childPtr = nodePtr->firstChildPtr; childPtr; childPtr = childPtr->nextSiblingPtr) {
        validateDefaultStatements(childPtr);
    }        
}

static void typeHandler(_YangNode* nodePtr) {
    if (nodePtr->nodeType != YANG_NODE_ORIGINAL) return;
    /* resolve built-in type */
    _YangNode* curNode = nodePtr;
    while (curNode->typeInfo && curNode->typeInfo->baseTypeNodePtr != NULL) {
        curNode = findChildNodeByType(curNode->typeInfo->baseTypeNodePtr, YANG_DECL_TYPE);
        if (curNode == nodePtr) {
            /* loop */
            return;
        }
    }

    if (nodePtr->typeInfo && curNode->typeInfo) {
	nodePtr->typeInfo->builtinType = curNode->typeInfo->builtinType;
    }

    /* Validate union subtypes.
       A member type can be of any built-in or derived type, except it MUST NOT be one of the built-in types "empty" or "leafref". */
    if (nodePtr->parentPtr->export.nodeKind == YANG_DECL_TYPE) {        
        if (nodePtr->typeInfo->builtinType == YANG_TYPE_EMPTY) {
            smiPrintErrorAtLine(currentParser, ERR_INVALID_UNION_TYPE, nodePtr->line, "empty");
        } else if (nodePtr->typeInfo->builtinType == YANG_TYPE_LEAFREF) {
            smiPrintErrorAtLine(currentParser, ERR_INVALID_UNION_TYPE, nodePtr->line, "leafref");
        }
    }

    if (nodePtr->typeInfo && !nodePtr->typeInfo->baseTypeNodePtr) {
        switch (nodePtr->typeInfo->builtinType) {
            case YANG_TYPE_ENUMERATION:
                if (!findChildNodeByType(nodePtr, YANG_DECL_ENUM)) {
                    smiPrintErrorAtLine(currentParser, ERR_CHILD_REQUIRED, nodePtr->line, "enumeration", "enum");
                }
                break;
            case YANG_TYPE_BITS:
                if (!findChildNodeByType(nodePtr, YANG_DECL_BIT)) {
                    smiPrintErrorAtLine(currentParser, ERR_CHILD_REQUIRED, nodePtr->line, "bits", "bit");
                }
                break;
            case YANG_TYPE_LEAFREF:
                if (!findChildNodeByType(nodePtr, YANG_DECL_PATH)) {
                    smiPrintErrorAtLine(currentParser, ERR_CHILD_REQUIRED, nodePtr->line, "leafref", "path");
                }
                break;
            case YANG_TYPE_IDENTITY:
                if (!findChildNodeByType(nodePtr, YANG_DECL_BASE)) {
                    smiPrintErrorAtLine(currentParser, ERR_CHILD_REQUIRED, nodePtr->line, "identityref", "base");
                }
                break;
            case YANG_TYPE_UNION:
                if (!findChildNodeByType(nodePtr, YANG_DECL_TYPE)) {
                    smiPrintErrorAtLine(currentParser, ERR_CHILD_REQUIRED, nodePtr->line, "union", "type");
                }
                break;
            case YANG_TYPE_DECIMAL64:
                if (!findChildNodeByType(nodePtr, YANG_DECL_FRACTION_DIGITS)) {
                    smiPrintErrorAtLine(currentParser, ERR_CHILD_REQUIRED, nodePtr->line, "decimal64", "fraction-digits");
                }
                break;
        }
    }

    curNode = nodePtr->firstChildPtr;
    while (curNode) {
        switch (curNode->export.nodeKind) {
            case YANG_DECL_RANGE:
                if (nodePtr->typeInfo
		    && !isNumericalType(nodePtr->typeInfo->builtinType)) {
                   smiPrintErrorAtLine(currentParser, ERR_RESTRICTION_NOT_ALLOWED, curNode->line, "range");
                }
                break;
            case YANG_DECL_FRACTION_DIGITS:
                if (nodePtr->typeInfo
		    && nodePtr->typeInfo->builtinType != YANG_TYPE_DECIMAL64) {
                    smiPrintErrorAtLine(currentParser, ERR_RESTRICTION_NOT_ALLOWED, curNode->line, "fraction-digits");
                }
                break;
            case YANG_DECL_LENGTH:
                if (nodePtr->typeInfo
		    && nodePtr->typeInfo->builtinType != YANG_TYPE_STRING
		    && nodePtr->typeInfo->builtinType != YANG_TYPE_BINARY) {
                    smiPrintErrorAtLine(currentParser, ERR_RESTRICTION_NOT_ALLOWED, curNode->line, "length");
                }
                break;
            case YANG_DECL_PATTERN:
                if (nodePtr->typeInfo
		    && nodePtr->typeInfo->builtinType != YANG_TYPE_STRING) {
                    smiPrintErrorAtLine(currentParser, ERR_RESTRICTION_NOT_ALLOWED, curNode->line, "pattern");
                }
                break;
            case YANG_DECL_ENUM:
                if (nodePtr->typeInfo
		    && nodePtr->typeInfo->builtinType != YANG_TYPE_ENUMERATION) {
                    smiPrintErrorAtLine(currentParser, ERR_RESTRICTION_NOT_ALLOWED, curNode->line, "enum");
                }
                break;
            case YANG_DECL_BIT:
                if (nodePtr->typeInfo
		    && nodePtr->typeInfo->builtinType != YANG_TYPE_BITS) {
                    smiPrintErrorAtLine(currentParser, ERR_RESTRICTION_NOT_ALLOWED, curNode->line, "bit");
                }
                break;
            case YANG_DECL_PATH:
                if (nodePtr->typeInfo
		    && nodePtr->typeInfo->builtinType != YANG_TYPE_LEAFREF) {
                    smiPrintErrorAtLine(currentParser, ERR_RESTRICTION_NOT_ALLOWED, curNode->line, "path");
                }
                break;
            case YANG_DECL_BASE:
                if (nodePtr->typeInfo
		    && nodePtr->typeInfo->builtinType != YANG_TYPE_IDENTITY) {
                    smiPrintErrorAtLine(currentParser, ERR_RESTRICTION_NOT_ALLOWED, curNode->line, "base");
                }
                break;
            case YANG_DECL_TYPE:
                if (nodePtr->typeInfo
		    && nodePtr->typeInfo->builtinType != YANG_TYPE_UNION) {
                    smiPrintErrorAtLine(currentParser, ERR_RESTRICTION_NOT_ALLOWED, curNode->line, "union");
                }
                break;
            default:
                break;
        }
        curNode = curNode->nextSiblingPtr;
    }
}


static int isInList(int value, int* list) {
    int i;
    for (i = 1; i <= list[0]; i++) {
        if (value == list[i]) {
            return 1;
        }
    }
    return 0;
}

static void _iterate(_YangNode *nodePtr, void* handler, int* nodeKindList) {
    if (isInList(nodePtr->export.nodeKind, nodeKindList)) {
        void (*handlerPtr)(_YangNode*);
        handlerPtr = handler;
        handlerPtr(nodePtr);
    }
    _YangNode *childPtr = NULL;
    for (childPtr = nodePtr->firstChildPtr; childPtr; childPtr = childPtr->nextSiblingPtr) {
        _iterate(childPtr, handler, nodeKindList);
    }
}
/*
 * The last argument should be the YANG_DECL_UNKNOWN value
 */
static void iterate(_YangNode *nodePtr, void* handler, ...) {
    int cnt = 0, value;
    va_list ap;
    
    va_start(ap, handler);    
    while ((value = va_arg(ap, int)) != YANG_DECL_UNKNOWN) {
        cnt++;
    }
    va_start(ap, handler);
    int *nodeKindList = smiMalloc((cnt + 1) * sizeof(int));
    nodeKindList[0] = cnt;
    cnt = 0;
    while ((value = va_arg(ap, int)) != YANG_DECL_UNKNOWN) {
        cnt++;
        nodeKindList[cnt] = value;
    }
    va_end(ap);
    _iterate(nodePtr, handler, nodeKindList);
}

void yangSemanticAnalysis(_YangNode *module) {
    getModuleInfo(module)->originalModule = copyModule(module);
    initMap();
    resolveReferences(module);
    /* complex types */
    /* complex types */
    iterate(module, keyValidation, YANG_DECL_KEY, YANG_DECL_UNKNOWN);
    iterate(module, abstractValidation, YANG_DECL_COMPLEX_TYPE, YANG_DECL_UNKNOWN);
    iterate(module, expandInheritance, YANG_DECL_COMPLEX_TYPE, YANG_DECL_UNKNOWN);
    /* END complex types */

    expandGroupings(module);
    /* complex types */
    expandInstance(module, 0);
    /* END complex types */
    expandAugments(module);

    iterate(module, expandDeviation, YANG_DECL_DEVIATION, YANG_DECL_UNKNOWN);

    /*
     *  module - a pointer to the root node of the module;
     *  1      - current config value;
     *  0      - don't ignore config value (it may be used for 'input', 'output' or 'notification' nodes, for which the 'config' value should be ignored;
     */
    validateConfigProperties(module, YANG_CONFIG_TRUE, 0);
    validateDefaultStatements(module);

    validateLists(module);

    /* complex types */
    iterate(module, instanceListValidation, YANG_DECL_INSTANCE_LIST, YANG_DECL_UNKNOWN);
    /* END complex types */



    uniqueNames(module);
    uniqueSubmoduleDefinitions(module);
    /* validate type restrictions */
    iterate(module, typeHandler, YANG_DECL_TYPE, YANG_DECL_UNKNOWN);
}

#endif

