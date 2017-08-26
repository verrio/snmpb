/*
 * Copyright (c) 2010, Nokia Siemens Networks Oy
 * All rights reserved.
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions are met:
 *
 *   * Redistributions of source code must retain the above copyright
 *    notice, this list of conditions and the following disclaimer.
 *   * Redistributions in binary form must reproduce the above copyright
 *     notice, this list of conditions and the following disclaimer in
 *     the documentation and/or other materials provided with the
 *     distribution.
 *   * Neither the name of the Nokia Siemens Networks Oy nor the names of its
 *     contributors may be used to endorse or promote products derived
 *     from this software without specific prior written permission.
 *
 * THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS
 * IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED
 * TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A
 * PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT
 * HOLDER OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL,
 * SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED
 * TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR
 * PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF
 * LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING
 * NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS
 * SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
 */

#include <stdio.h>
#include <string.h>

#include "error.h"
#include "yang-data.h"
#include "yang-check.h"
#include "yang-complex-types.h"

/*
 * Current parser defined in parser-yang. Workaround - can't include data.h
 */
extern Parser *currentParser;

static const int parents_complex_type[] = {
    YANG_DECL_MODULE, YANG_DECL_SUBMODULE, YANG_DECL_CONTAINER,
    YANG_DECL_LIST, YANG_DECL_RPC, YANG_DECL_INPUT,
    YANG_DECL_OUTPUT, YANG_DECL_NOTIFICATION
};
static const int parents_complex_type_len = 8;

static const int parents_instance[] = {
    YANG_DECL_MODULE, YANG_DECL_SUBMODULE, YANG_DECL_CONTAINER,
    YANG_DECL_LIST, YANG_DECL_GROUPING, YANG_DECL_INPUT,
    YANG_DECL_OUTPUT, YANG_DECL_NOTIFICATION, YANG_DECL_AUGMENT,
    YANG_DECL_CHOICE, YANG_DECL_CASE, YANG_DECL_COMPLEX_TYPE,
    YANG_DECL_INSTANCE, YANG_DECL_INSTANCE_LIST
};
static const int parents_instance_len = 14;


static int
listContains(int value, const int* list, const int listLen)
{
    int i;
    for (i = 0; i < listLen; i++) {
        if (value == list[i]) {
            return 1;
        }
    }
    return 0;
}

static int
isComplexTypeParent(int parentNode)
{
    return listContains(parentNode,
			parents_complex_type,
			parents_complex_type_len);
}

static int
isInstanceParent(int parentNode)
{
    return listContains(parentNode, parents_instance, parents_instance_len);
}

static int
isInstanceTypeParent(int parentNode)
{
    return (parentNode == YANG_DECL_INSTANCE
	    || parentNode == YANG_DECL_INSTANCE_LIST);
}


/*
 * Create a complex type info
 */
static void createComplexTypeInfo(_YangNode *node) {
    _YangComplexTypeInfo *infoPtr = smiMalloc(sizeof(_YangComplexTypeInfo));

    node->ctInfo             = infoPtr;
    node->ctInfo->labeled    = 0;
}

/*
 * Create a node for the complex type extension
 */
_YangNode* addCTExtNode(_YangNode* module, int nodeType, int isUnique, _YangNode* parent, char* prefix, char* identifier, char* extra, char* value) {
    _YangNode* extModule = findYangModuleByPrefix(module, prefix);
    _YangNode* ret;
    if (!strcmp(extModule->export.value, "complex-types")) {
        if (isUnique) {
            uniqueNodeKind(parent, nodeType);
        }
        ret = addYangNode(value, nodeType, parent);
        ret->export.extra = extra;
        switch (nodeType) {
            case YANG_DECL_COMPLEX_TYPE:
                if (!isComplexTypeParent(parent->export.nodeKind)) {
                    smiPrintError(currentParser, ERR_BAD_PARENT, "complex-type", yangDeclAsString(parent->export.nodeKind));
                }
                createComplexTypeInfo(ret);
                break;
            case YANG_DECL_INSTANCE:
            case YANG_DECL_INSTANCE_LIST:
                if (!isInstanceParent(parent->export.nodeKind)) {
                    smiPrintError(currentParser, ERR_BAD_PARENT, (nodeType == YANG_DECL_INSTANCE ? "instance" : "instance-list"), yangDeclAsString(parent->export.nodeKind));
                }
                break;
            case YANG_DECL_INSTANCE_TYPE:
                if (parent->export.nodeKind == YANG_DECL_TYPE) {
                    if (strcmp(parent->export.value, "instance-identifier")) {
                        smiPrintError(currentParser, ERR_BAD_PARENT, "instance-type", "type with the argument not instance-identifier");
                    }
                } else if (!isInstanceTypeParent(parent->export.nodeKind)) {
                    smiPrintError(currentParser, ERR_BAD_PARENT, "instance-type", yangDeclAsString(parent->export.nodeKind));
                }
                break;
            case YANG_DECL_EXTENDS:
            case YANG_DECL_ABSTRACT:
                if (parent->export.nodeKind != YANG_DECL_COMPLEX_TYPE) {
                    smiPrintError(currentParser, ERR_BAD_PARENT, (nodeType == YANG_DECL_ABSTRACT ? "abstract" : "extends"), yangDeclAsString(parent->export.nodeKind));
                }
                break;
        }
    } else {
        ret = addYangNode(extra, YANG_DECL_UNKNOWN_STATEMENT, parent);
        createIdentifierRef(ret, prefix, identifier);
        ret->export.extra = value;
    }
    return ret;
}

/*
 * Complex type key validation.
 */
void keyValidation(_YangNode* node) {
    _YangNode* nodePtr = node->parentPtr;
    if (nodePtr->export.nodeKind != YANG_DECL_COMPLEX_TYPE) {
        return;
    };
    /* A key statement can only appear in a complex type definition
       if the base complex types do not yet define a key */
    while (1) {
        _YangNode *nodeExtendsPtr = findChildNodeByType(nodePtr, YANG_DECL_EXTENDS);
        if (!nodeExtendsPtr) {
            break;
        }
        nodePtr = ((_YangIdentifierRefInfo*)nodeExtendsPtr->info)->resolvedNode;
        if (!nodePtr) {
            break;
        }
        _YangNode *nodeKeyPtr = findChildNodeByType(nodePtr, YANG_DECL_KEY);
        if (nodeKeyPtr) {
            smiPrintErrorAtLine(currentParser, ERR_CT_KEY_ALREADY_DEFINED, node->line, node->parentPtr->export.value, nodePtr->export.value);
            break;
        }
    }

    nodePtr = node->parentPtr;
    YangList *keys = (YangList*)node->info;
    while (keys) {
        _YangNode *leafPtr = findChildNodeByTypeAndValue(nodePtr, YANG_DECL_LEAF, listIdentifierRef(keys)->ident);
        if (!leafPtr) {
            smiPrintErrorAtLine(currentParser, ERR_INVALID_KEY_REFERENCE, node->line, listIdentifierRef(keys)->ident);
        } else {
            _YangNode *type = findChildNodeByType(leafPtr, YANG_DECL_TYPE);
            while (type->typeInfo->baseTypeNodePtr != NULL) {
                if (((_YangIdentifierRefInfo*)type->info)->loop) {
                    /* loop */
                    break;
                }
                type = findChildNodeByType(type->typeInfo->baseTypeNodePtr, YANG_DECL_TYPE);
            }

            if (!strcmp(type->export.value, "empty")) {
                smiPrintErrorAtLine(currentParser, ERR_EMPTY_KEY, node->line, leafPtr->export.value);
            }
        }
        keys = keys->next;
    }
}

/*
 * Complex type key validation.
 */
void instanceListValidation(_YangNode* node) {
    if (yangIsTrueConf(node->export.config)) {
        _YangNode *nodeTypePtr = findChildNodeByType(node, YANG_DECL_INSTANCE_TYPE);
        if (!nodeTypePtr) {
            return;
        }
        _YangNode *ctPtr = ((_YangIdentifierRefInfo*)nodeTypePtr->info)->resolvedNode;
        if (!ctPtr) {
            return;
        }
        _YangNode *nodeKeyPtr = findChildNodeByType(ctPtr, YANG_DECL_KEY);
        if (!nodeKeyPtr) {
            smiPrintErrorAtLine(currentParser, ERR_INSTANCE_LIST_KEY_REQUIRED, node->line, ctPtr->export.value, node->export.value);
        }
    }
}

/*
 * Check whether complex types which extend an abstract complex type are abstract as well
 */
void abstractValidation(_YangNode* node) {
    _YangNode *nodeAbstractPtr = findChildNodeByType(node, YANG_DECL_ABSTRACT);
    if (nodeAbstractPtr && !strcmp(nodeAbstractPtr->export.value, "true")) {
        _YangNode *cur = node;
        while (cur) {
            _YangNode *abstractPtr = findChildNodeByType(cur, YANG_DECL_ABSTRACT);
            if (!abstractPtr || strcmp(abstractPtr->export.value, "true")) {
                smiPrintErrorAtLine(currentParser, ERR_ABSTRACT, nodeAbstractPtr->line, node->export.value, cur->export.value);
                return;
            }
            _YangNode *extendsPtr = findChildNodeByType(cur, YANG_DECL_EXTENDS);
            if (!extendsPtr) {
                return;
            }
            cur = ((_YangIdentifierRefInfo*)extendsPtr->info)->resolvedNode;
            if (((_YangIdentifierRefInfo*)extendsPtr->info)->loop) {
                /* loop */
                return;
            }
        }
    }
}

/*
 * Expand 'extends' statements
 */
void expandInheritance(_YangNode* node) {
    if (node->ctInfo->state == YANG_PARSING_DONE) {
        return;
    }
    node->ctInfo->state = YANG_PARSING_IN_PROGRESS;
    _YangNode *nodeExtendsPtr = findChildNodeByType(node, YANG_DECL_EXTENDS);
    if (nodeExtendsPtr) {
        if (!((_YangIdentifierRefInfo*)nodeExtendsPtr->info)->loop) {
            _YangNode* extendedCt = ((_YangIdentifierRefInfo*)nodeExtendsPtr->info)->resolvedNode;
            if (extendedCt) {
                expandInheritance(extendedCt);
                _YangNode *refChild = extendedCt->firstChildPtr;
                while (refChild) {
                    if (isDataDefinitionNode(refChild)) {
                        copySubtree(node, refChild, YANG_NODE_INHERITED, 0, nodeExtendsPtr->line, 1);
                    }
                    refChild = refChild->nextSiblingPtr;
                }
            }
            applyRefinements(node);
        }
    }
    node->ctInfo->state = YANG_PARSING_DONE;
}

/*
 * Checks whether the node is nested into a complex type
 */
int isComplexTypeDefinition(_YangNode* node) {
    while (node) {
        if (node->export.nodeKind == YANG_DECL_COMPLEX_TYPE) {
            return 1;
        }
        node = node->parentPtr;
    }
    return 0;
}

/*
 * Expand 'instance/instance-list' statements
 */

void expandInstance(_YangNode* nodePtr, int  forced) {
    int nodeKind = nodePtr->export.nodeKind;
    if (nodeKind == YANG_DECL_COMPLEX_TYPE) {
        return;
    }
    _YangIdentifierRefInfo* typeInfoPtr;
    
    if (nodeKind == YANG_DECL_INSTANCE || nodeKind == YANG_DECL_INSTANCE_LIST) {
        _YangNode* nodeTypePtr = findChildNodeByType(nodePtr, YANG_DECL_INSTANCE_TYPE);
        if (!nodeTypePtr) {
            return;
        }
        typeInfoPtr = (_YangIdentifierRefInfo*)nodeTypePtr->info;
        if (!typeInfoPtr->resolvedNode) {
            return;
        }
        if (typeInfoPtr->resolvedNode->ctInfo->labeled && !forced) {
            /* recursion occurs, stop here */
            nodePtr->export.status = YANG_PARSING_IN_PROGRESS;
            return;
        }
        /* label complex type */
        typeInfoPtr->resolvedNode->ctInfo->labeled = 1;
        
        _YangNode *refChild = typeInfoPtr->resolvedNode->firstChildPtr;
        while (refChild) {
            if (isDataDefinitionNode(refChild)) {
                copySubtree(nodePtr, refChild, YANG_NODE_INSTANCE, 0, typeInfoPtr->resolvedNode->line, 1);
            }
            refChild = refChild->nextSiblingPtr;
        }

        
        nodePtr->export.status = YANG_PARSING_DONE;
    }

    _YangNode *childPtr = NULL;
    for (childPtr = nodePtr->firstChildPtr; childPtr; childPtr = childPtr->nextSiblingPtr) {
        expandInstance(childPtr, 0);
    }
    
    if (nodeKind == YANG_DECL_INSTANCE || nodeKind == YANG_DECL_INSTANCE_LIST) {
        for (childPtr = nodePtr->firstChildPtr; childPtr; childPtr = childPtr->nextSiblingPtr) {
            if (childPtr->export.nodeKind == YANG_DECL_AUGMENT) {
                expandAugment(childPtr, 1);
            }
        }

        /* label complex type */
        typeInfoPtr->resolvedNode->ctInfo->labeled = 0;
    }
}

