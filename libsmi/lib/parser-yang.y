/*
 * parser-yang.y --
 *
 *      Syntax rules for parsing the YANG MIB module language.
 *
 * Copyright (c) 1999 Frank Strauss, Technical University of Braunschweig.
 *
 * See the file "COPYING" for information on usage and redistribution
 * of this file, and for a DISCLAIMER OF ALL WARRANTIES.
 *
 *  Authors: Kaloyan Kanev, Siarhei Kuryla
 */

%parse-param { struct Parser *parserPtr }
%lex-param { struct Parser *parserPtr }
    
%{

#include <config.h>
    
#ifdef BACKEND_YANG

#define _ISOC99_SOURCE
#include <stdio.h>
#include <errno.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>
#include <time.h>
#include <limits.h>
#include <float.h>

#if defined(_MSC_VER)
#include <malloc.h>
#endif

#include "yang.h"
#include "yang-data.h"
#include "parser-yang.h"
#include "scanner-yang.h"
#include "yang-complex-types.h"
#include "util.h"
#include "error.h"
    
#ifdef HAVE_DMALLOC_H
#include <dmalloc.h>
#endif


#define thisParserPtr      ((Parser *)parserPtr)
#define thisModulePtr      (((Parser *)parserPtr)->yangModulePtr)
#define thisModuleInfoPtr  ((_YangModuleInfo*)((Parser *)parserPtr)->yangModulePtr->info)

#define SMI_EPOCH	631152000	/* 01 Jan 1990 00:00:00 */ 

#define DEBUG

#ifdef DEBUG 
#define debug(args...) fprintf (stderr, args);
#else
#define debug
#endif

    
/*
 * NOTE: The argument lvalp ist not really a void pointer. Unfortunately,
 * we don't know it better at this point. bison generated C code declares
 * YYSTYPE just a few lines below based on the `%union' declaration.
 */
extern int yylex(void *lvalp, Parser *parserPtr);

/* The declaration stack is used to determine what is the parrent
 * statement of the current one. It helps to write generic
 * substatements like the description statement which calls the
 * function setDescription, which in turn checks the current
 * statement type(decl) and uses the a global pointer to the 
 * current parent statement to set the description. 
 */
typedef struct declStack {
    YangDecl decl;
    _YangNode* node;
    struct declStack *down;
} declStack;

static declStack *dStack = NULL;

static void pushDecl(YangDecl decl)
{
    declStack *top = (declStack*)smiMalloc(sizeof(declStack));
    top->down = NULL;
    top->decl = decl;
	
    if(dStack == NULL) {
        dStack = top;
    } else {
        top->down = dStack;
        dStack = top;
    }
}

static void pushNode(_YangNode* node)
{
    declStack *top = (declStack*)smiMalloc(sizeof(declStack));
    top->down = NULL;
    top->node = node;
    if (node) {
        top->decl = node->export.nodeKind;
    }
	
    if(dStack == NULL) {
        dStack = top;
    } else {
        top->down = dStack;
        dStack = top;
    }
}

static void pop()
{
    declStack *top;
    if(dStack != NULL)
    {
        top = dStack;
        dStack = dStack->down;
        free(top);
    }
}

static YangDecl topDecl()
{
    if (dStack == NULL)
    {
        return YANG_DECL_UNKNOWN;
    }
    else
    {
        return dStack->decl;
    }
}

static _YangNode* topNode()
{
    if (dStack == NULL)
    {
        return NULL;
    }
    else
    {
        return dStack->node;
    }
}

Parser *currentParser = NULL;

static _YangNode *node = NULL;

char* getPrefix(char* identifierRef) {
    char* colonIndex = strchr(identifierRef, ':');
    if (!colonIndex) return NULL;
    return smiStrndup(identifierRef, colonIndex - identifierRef);
}

char* getIdentifier(char* identifierRef) {
    char* colonIndex = strchr(identifierRef, ':');
    if (!colonIndex) return smiStrdup(identifierRef);
    return smiStrdup(colonIndex + 1);
}

void checkUnknownStatement() {
    if (topNode()->export.nodeKind == YANG_DECL_UNKNOWN_STATEMENT) {
        _YangNode *childPtr = NULL;
        for (childPtr = topNode()->firstChildPtr; childPtr; childPtr = childPtr->nextSiblingPtr) {
            if (childPtr->export.nodeKind != YANG_DECL_UNKNOWN_STATEMENT) {
                smiPrintErrorAtLine(currentParser, ERR_UNEXPECTED_KEYWORD, childPtr->line, yangDeclAsString(childPtr->export.nodeKind));
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
    if (len == 10 || len == 16) {
        for (i = 0; i < len; i++) {
            if (((i < 4 || i == 5 || i == 6 || i == 8 || i == 9 || i == 11
              || i == 12 || i == 14 || i == 15) && ! isdigit((int)date[i]))
            || ((i == 4 || i == 7) && date[i] != '-')
            || (i == 10 && date[i] != ' ')
            || (i == 13 && date[i] != ':')) {
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
	for (i = 0, p = date, tm.tm_year = 0; i < 4; i++, p++) {
	    tm.tm_year = tm.tm_year * 10 + (*p - '0');
	}
	p++;
	tm.tm_mon = (p[0]-'0') * 10 + (p[1]-'0');
	p += 3;
	tm.tm_mday = (p[0]-'0') * 10 + (p[1]-'0');
	p += 2;
	if (len == 16) {
	    p++;
	    tm.tm_hour = (p[0]-'0') * 10 + (p[1]-'0');
	    p += 3;
	    tm.tm_min = (p[0]-'0') * 10 + (p[1]-'0');
	}
	
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
%start yangFile

/*
 * We call the parser from within the parser when IMPORTing modules,
 * hence we need reentrant parser code. This is a bison feature.
 */
%pure_parser

/*
 * The attributes.
 */
%union {
    int             rc;                 /* >=0: ok, <0: error    */
    char            *text;
}

/*
 * Tokens and their attributes.
 */
%token <text>augmentKeyword
%token <text>belongs_toKeyword
%token <text>choiceKeyword
%token <text>configKeyword
%token <text>contactKeyword
%token <text>containerKeyword
%token <text>defaultKeyword
%token <text>descriptionKeyword
%token <text>fractionDigitsKeyword
%token <text>enumKeyword
%token <text>error_app_tagKeyword
%token <text>error_messageKeyword
%token <text>extensionKeyword
%token <text>groupingKeyword
%token <text>importKeyword
%token <text>includeKeyword
%token <text>keyKeyword
%token <text>leafKeyword
%token <text>leaf_listKeyword
%token <text>lengthKeyword
%token <text>listKeyword
%token <text>mandatoryKeyword
%token <text>max_elementsKeyword
%token <text>min_elementsKeyword
%token <text>moduleKeyword
%token <text>submoduleKeyword
%token <text>mustKeyword
%token <text>namespaceKeyword
%token <text>ordered_byKeyword
%token <text>organizationKeyword
%token <text>prefixKeyword
%token <text>rangeKeyword
%token <text>referenceKeyword
%token <text>patternKeyword
%token <text>revisionKeyword
%token <text>statusKeyword
%token <text>typeKeyword
%token <text>typedefKeyword
%token <text>uniqueKeyword
%token <text>unitsKeyword
%token <text>usesKeyword
%token <text>valueKeyword
%token <text>whenKeyword
%token <text>bitKeyword
%token <text>pathKeyword
%token <text>anyXMLKeyword
%token <text>deprecatedKeyword
%token <text>currentKeyword
%token <text>obsoleteKeyword
%token <text>trueKeyword
%token <text>falseKeyword
%token <text>caseKeyword
%token <text>inputKeyword
%token <text>outputKeyword
%token <text>rpcKeyword
%token <text>notificationKeyword
%token <text>argumentKeyword
%token <text>yangversionKeyword
%token <text>baseKeyword
%token <text>deviationKeyword
%token <text>deviateKeyword
%token <text>featureKeyword
%token <text>identityKeyword
%token <text>ifFeatureKeyword
%token <text>positionKeyword
%token <text>presenceKeyword
%token <text>refineKeyword
%token <text>requireInstanceKeyword
%token <text>yinElementKeyword
%token <text>notSupportedKeyword
%token <text>addKeyword
%token <text>deleteKeyword
%token <text>replaceKeyword

%token <text>complexTypeKeyword
%token <text>extendsKeyword
%token <text>abstractKeyword
%token <text>instanceKeyword
%token <text>instanceListKeyword
%token <text>instanceTypeKeyword

%token <text>identifier
%token <text>identifierRefArg
%token <text>identifierRefArgStr
%token <text>dateString
%token <text>yangVersion
%token <text>qString
%token <text>uqString

%token <text>decimalNumber
%token <text>floatNumber
%token <text>hexNumber

/*
 * Types of non-terminal symbols.
 */

%type <rc>yangFile
%type <rc>moduleStatement
%type <rc>submoduleStatement
%type <rc>commonStatement
%type <rc>dataDefStatement
%type <rc>extensionStatement
%type <rc>extensionStatementBody
%type <rc>extensionSubstatement
%type <rc>extensionSubstatement_0n
%type <rc>argumentStatement
%type <rc>argumentStatementBody
%type <rc>yinElementOptional
%type <rc>linkageStatement
%type <rc>linkageStatement_0n
%type <rc>moduleMetaStatement
%type <rc>moduleMetaStatement_0n
%type <rc>moduleHeaderStatement
%type <rc>moduleHeaderStatement_0n
%type <rc>submoduleHeaderStatement
%type <rc>belongsToStatement
%type <rc>bodyStatement
%type <rc>bodyStatement_0n
%type <rc>containerStatement
%type <rc>containerSubstatement
%type <rc>containerSubstatement_0n
%type <rc>mustStatement
%type <rc>mustSubstatement
%type <rc>mustSubstatement_0n
%type <rc>presenceStatement
%type <rc>organizationStatement
%type <rc>contactStatement
%type <rc>referenceStatement
%type <rc>descriptionStatement
%type <rc>revisionStatement
%type <rc>optionalRevision
%type <rc>revisionStatement_0n
%type <rc>revisionSubstatement_On
%type <rc>revisionSubstatement
%type <rc>importStatement
%type <rc>includeStatement
%type <rc>includeStatementBody
%type <rc>prefixStatement
%type <rc>typedefStatement
%type <rc>typedefSubstatement
%type <rc>typedefSubstatement_0n
%type <rc>typeStatement
%type <rc>optionalTypeBodyStatements
%type <rc>typeBodyStmts
%type <rc>unitsStatement
%type <rc>featureStatement
%type <rc>featureSpec
%type <rc>featureSubstatement_0n
%type <rc>featureSubstatement
%type <rc>ifFeatureStatement
%type <rc>identityStatement
%type <rc>identitySpec
%type <rc>identitySubstatement_0n
%type <rc>identitySubstatement
%type <rc>booleanValue

%type <rc>numRestriction
%type <rc>stringRestriction
%type <rc>stringRestriction_0n
%type <rc>optionalRestrictionSpec
%type <rc>restrictionSpec_0n
%type <rc>restrictionSpec
%type <rc>range
%type <rc>length
%type <text>date
%type <text>string
%type <text>prefix
%type <text>identifierRef
%type <text>identifierStr
%type <rc>enumSpec
%type <rc>enum
%type <rc>enumSubstatementSpec
%type <rc>enumSubstatement_0n
%type <rc>enumSubstatement
%type <rc>valueStatement
%type <rc>leafrefSpec
%type <rc>identityrefSpec
%type <rc>baseStatement
%type <rc>bitsSpec
%type <rc>bitsStatement
%type <rc>bitsSubstatementSpec
%type <rc>bitsSubstatement_0n
%type <rc>bitsSubstatement
%type <rc>positionStatement
%type <rc>unionSpec
%type <rc>stmtEnd
%type <rc>unknownStatement0_n
%type <rc>unknownStatement
%type <rc>complexTypeStatement
%type <rc>instanceStatement
%type <rc>instanceListStatement
%type <rc>instanceTypeStatement
%type <rc>abstractStatement
%type <rc>extendsStatement
%type <rc>errorAppTagStatement
%type <rc>errorMessageStatement
%type <rc>path
%type <rc>requireInstanceStatement
%type <rc>pattern
%type <rc>status
%type <rc>statusStatement
%type <rc>configStatement
%type <rc>defaultStatement
%type <rc>leafStatement
%type <rc>leafSubstatement
%type <rc>leafSubstatement_0n
%type <rc>leaf_listStatement
%type <rc>leaf_listSubstatement
%type <rc>leaf_listSubstatement_0n
%type <rc>listStatement

%type <rc>listSubstatement
%type <rc>listSubstatement_0n
%type <rc>min_elementsStatement
%type <rc>max_elementsStatement
%type <rc>ordered_byStatement
%type <rc>keyStatement
%type <rc>uniqueStatement
%type <rc>choiceStatement
%type <rc>choiceSpec
%type <rc>choiceSubstatement
%type <rc>choiceSubstatement_0n
%type <rc>caseStatement
%type <rc>caseSpec
%type <rc>caseSubstatement
%type <rc>caseSubstatement_0n
%type <rc>caseDataDef
%type <rc>fractionDigitsStatement
%type <rc>groupingStatement
%type <rc>groupingSubstatement
%type <rc>groupingSubstatement_0n
%type <rc>usesStatement
%type <rc>usesSubstatement
%type <rc>usesSubstatement_0n
%type <rc>refineStatement
%type <rc>refineSpec
%type <rc>refineSubstatement
%type <rc>refine
%type <rc>refine_0n
%type <rc>augmentStatement
%type <rc>augmentSubstatement
%type <rc>augmentSubstatement_0n
%type <rc>whenStatement
%type <rc>rpcStatement
%type <rc>rpcSpec
%type <rc>rpcSubstatement
%type <rc>rpcSubstatement_0n
%type <rc>inputStatement
%type <rc>inputOutputSubstatement
%type <rc>inputOutputSubstatement_0n
%type <rc>notificationStatement
%type <rc>notificationSpec
%type <rc>notificationSubstatement
%type <rc>notificationSubstatement_0n
%type <rc>anyXMLStatement
%type <rc>anyXMLSpec
%type <rc>anyXMLSubstatement
%type <rc>anyXMLSubstatement_0n
%type <rc>deviationStatement
%type <rc>deviationSubstatement_0n
%type <rc>deviationSubstatement
%type <rc>deviateNotSupported
%type <rc>deviateAddStatement
%type <rc>deviateAddSpec
%type <rc>deviateAddSubstatement_0n
%type <rc>deviateAddSubstatement
%type <rc>deviateDeleteStatement
%type <rc>deviateDeleteSpec
%type <rc>deviateDeleteSubstatement_0n
%type <rc>deviateDeleteSubstatement
%type <rc>deviateReplaceStatement
%type <rc>deviateReplaceSpec
%type <rc>deviateReplaceSubstatement_0n
%type <rc>deviateReplaceSubstatement
%type <rc>yangVersionStatement
%type <rc>namespaceStatement
%type <rc>mandatoryStatement
%type <rc>outputStatement
%%

/*
 * Yacc rules.
 *
 */


/*
 * One mibFile may contain multiple MIB modules.
 * It's also possible that there's no module in a file.
 */
yangFile:   moduleStatement
        |
            submoduleStatement
        ;

moduleStatement:    moduleKeyword identifierStr
		{
                    currentParser = thisParserPtr;
                    thisParserPtr->yangModulePtr = findYangModuleByName($2, NULL);
                    if (!thisParserPtr->yangModulePtr) {
                        thisParserPtr->yangModulePtr =  addYangNode($2, YANG_DECL_MODULE, NULL);
                    
                        if (smiHandle->firstYangModulePtr) {
                            smiHandle->firstYangModulePtr->nextSiblingPtr = thisModulePtr;
                        } else {
                            smiHandle->firstYangModulePtr = thisModulePtr;
                        }
                    } else {
                        smiPrintError(thisParserPtr, ERR_MODULE_ALREADY_LOADED, $2);
                        free($2);
                        /*
                         * this aborts parsing the whole file,
                         * not only the current module.
                         */
                        YYABORT;
                    }
                    thisModulePtr->info = createModuleInfo(thisModulePtr);
                    pushNode(thisModulePtr);
                }
               '{'
                    stmtSep
                    moduleHeaderStatement_0n
                    linkageStatement_0n
                    moduleMetaStatement_0n
                    revisionStatement_0n
                    bodyStatement_0n
               '}'
                {
                    thisModuleInfoPtr->parsingState  = YANG_PARSING_DONE;
                    pop();
                    yangSemanticAnalysis(thisModulePtr);
                }
	;

submoduleStatement:	submoduleKeyword identifierStr
                    {
                        currentParser = thisParserPtr;
                        thisParserPtr->yangModulePtr = findYangModuleByName($2, NULL);
                        if (!thisParserPtr->yangModulePtr) {
                            thisParserPtr->yangModulePtr =  addYangNode($2, YANG_DECL_SUBMODULE, NULL);
                    
                            if (smiHandle->firstYangModulePtr) {
                                smiHandle->firstYangModulePtr->nextSiblingPtr = thisModulePtr;
                            } else {
                                smiHandle->firstYangModulePtr = thisModulePtr;
                            }
                        } else {
                            smiPrintError(thisParserPtr, ERR_MODULE_ALREADY_LOADED, $2);
                            free($2);
                            /*
                             * this aborts parsing the whole file,
                             * not only the current module.
                             */
                            YYABORT;
                        }
                        currentParser = thisParserPtr;
                        thisModulePtr->info = createModuleInfo(thisModulePtr);
                        pushNode(thisModulePtr);
                    }
                   '{'
                        stmtSep
                        submoduleHeaderStatement
                        linkageStatement_0n
                        moduleMetaStatement_0n
                        revisionStatement_0n
                        bodyStatement_0n
                   '}'
                    {
                        thisModuleInfoPtr->parsingState  = YANG_PARSING_DONE;
                        pop();
                        yangSemanticAnalysis(thisModulePtr);
                    }
            ;

moduleHeaderStatement_0n:   moduleHeaderStatement0_n
                 	{
                            if (!thisModuleInfoPtr->namespace) {
                                smiPrintError(parserPtr, ERR_NAMESPACE_MISSING, NULL);
                            }
                            if (!thisModuleInfoPtr->prefix) {
                                smiPrintError(parserPtr, ERR_PREFIX_MISSING, NULL);
                            }
			}
		;

moduleHeaderStatement0_n:
                |
                    moduleHeaderStatement moduleHeaderStatement0_n
                ;

moduleHeaderStatement:	yangVersionStatement stmtSep
                |
                        namespaceStatement stmtSep
                |
                        prefixStatement stmtSep
		;

moduleMetaStatement_0n:	{}
		|
			moduleMetaStatement_0n moduleMetaStatement stmtSep
		;

moduleMetaStatement:	organizationStatement
		|
			contactStatement 
		|
			descriptionStatement
		|
			referenceStatement
		;

submoduleHeaderStatement:   belongsToStatement stmtSep
                            yangVersionStatement stmtSep
                |
                            yangVersionStatement stmtSep
                            belongsToStatement stmtSep
                |
                            belongsToStatement stmtSep
                ;


belongsToStatement: belongs_toKeyword identifierStr 
                    {
                        node = addYangNode($2, YANG_DECL_BELONGS_TO, topNode());
                        pushNode(node);
                    }
                    '{'
                        stmtSep prefixStatement stmtSep 
                    '}'
                    {
                        pop();
                    }
                    ;

linkageStatement_0n: {}
		|
			linkageStatement_0n linkageStatement
		;

linkageStatement:       includeStatement stmtSep
            |
                        importStatement stmtSep
		;

revisionStatement_0n: {}
		|
			revisionStatement_0n revisionStatement stmtSep
		;

bodyStatement_0n: {}
		|
			bodyStatement_0n bodyStatement stmtSep
		;

bodyStatement:      extensionStatement
                |
                    featureStatement
                |
                    identityStatement
                |
                    typedefStatement
                |
                    groupingStatement
                |
                    dataDefStatement
                |
                    augmentStatement
                |
                    rpcStatement
                |
                    notificationStatement
                |
                    deviationStatement
                ;

dataDefStatement:   containerStatement
                |
                    leafStatement
                |
                    leaf_listStatement
                |
                    listStatement
                |
                    choiceStatement
                |
                    anyXMLStatement
                |
                    usesStatement
                ;

commonStatement:    descriptionStatement
                |
                    statusStatement
                |
                    referenceStatement
                |
                    configStatement
                ;	       

organizationStatement:	organizationKeyword string stmtEnd
			{
                            if (!thisModuleInfoPtr->organization) {
                                node = addYangNode($2, YANG_DECL_ORGANIZATION, topNode());
                                thisModuleInfoPtr->organization = node->export.value;
                            } else {
                                smiPrintError(currentParser, ERR_REDEFINED_ORGANIZATION, NULL);
                            }
			}
                ;

contactStatement:	contactKeyword string stmtEnd
			{
                            if (!thisModuleInfoPtr->contact) {
                                node = addYangNode($2, YANG_DECL_CONTACT, topNode());
                                thisModuleInfoPtr->contact = node->export.value;
                            } else {
                                smiPrintError(currentParser, ERR_REDEFINED_CONTACT, NULL);
                            }
			}
                ;

descriptionStatement:	descriptionKeyword string stmtEnd
			{
                            uniqueNodeKind(topNode(), YANG_DECL_DESCRIPTION);
                            setDescription(topNode(), $2);
                            node = addYangNode($2, YANG_DECL_DESCRIPTION, topNode());
			}
        	;

referenceStatement:	referenceKeyword string stmtEnd
			{
                            uniqueNodeKind(topNode(), YANG_DECL_REFERENCE);
                            setReference(topNode(), $2);
                            node = addYangNode($2, YANG_DECL_REFERENCE, topNode());
			}
        	;

statusStatement:	statusKeyword status stmtEnd
			{
                            uniqueNodeKind(topNode(), YANG_DECL_STATUS);
                            setStatus(topNode(), $2);
                            node = addYangNode(yangStatusAsString($2), YANG_DECL_STATUS, topNode());
			}
        	;

namespaceStatement:	namespaceKeyword string stmtEnd
		  	{
                            if (!thisModuleInfoPtr->namespace) {
                                node = addYangNode($2, YANG_DECL_NAMESPACE, topNode());
                                thisModuleInfoPtr->namespace = node->export.value;
                            } else {
                                smiPrintError(currentParser, ERR_REDEFINED_NAMESPACE, NULL);
                            }
			}
            ;

yangVersionStatement:   yangversionKeyword yangVersion stmtEnd
		  	{
                            if (!thisModuleInfoPtr->version) {
                                node = addYangNode($2, YANG_DECL_YANG_VERSION, topNode());
                                thisModuleInfoPtr->version = node->export.value;
                            } else {
                                smiPrintError(currentParser, ERR_REDEFINED_YANGVERSION, NULL);
                            }
			}
            ;

status:		deprecatedKeyword
		{
                    $$ = YANG_STATUS_DEPRECATED;
		}
	|
		currentKeyword
		{
                    $$ = YANG_STATUS_CURRENT;
		}
	|
		obsoleteKeyword
		{
                    $$ = YANG_STATUS_OBSOLETE;
		}
	;

prefixStatement:	prefixKeyword prefix stmtEnd
			{
                            node = addYangNode($2, YANG_DECL_PREFIX, topNode());
                            switch(topDecl())
                            {
                                case YANG_DECL_MODULE:
                                    if(!thisModuleInfoPtr->prefix)
                                        thisModuleInfoPtr->prefix = node->export.value;
                                    else
                                        smiPrintError(currentParser, ERR_REDEFINED_PREFIX, NULL);
                                    break;
                                case YANG_DECL_BELONGS_TO:
                                    thisModuleInfoPtr->prefix = node->export.value;
                                case YANG_DECL_IMPORT:

                                    break;
                                default:
                                    /* TODO print error */
                                    debug("DEBUGG: OOPS wrong prefix DECL %d, at line %d\n", topDecl(), currentParser->line);
                                    break;
                            }
			}
	;

revisionStatement:	revisionKeyword date ';' 
			{
                            node = addYangNode($2, YANG_DECL_REVISION, topNode());
                        }
                |
                        revisionKeyword date
			{
                            node = addYangNode($2, YANG_DECL_REVISION, topNode());
                            pushNode(node);
			}
			'{'
                            stmtSep
                            revisionSubstatement_On
			'}'
			{
                            pop();
			}
    ;

revisionSubstatement_On: {}
                    |
                       revisionSubstatement_On revisionSubstatement stmtSep
                    ;

revisionSubstatement:
                        descriptionStatement
                    |
                        referenceStatement;

date: 	dateString
	{
            checkDate(currentParser, $1);
	}
	;

importStatement: importKeyword identifierStr
		{
                    node = addYangNode($2, YANG_DECL_IMPORT, topNode());
                    pushNode(node);
		}
		'{'
                    stmtSep
                    prefixStatement stmtSep
                    optionalRevision
		'}'
		{
		    _YangNode* importNode;
                    externalModule(topNode());
                    importNode = topNode();
                    pop();
                    if (topNode() != thisModulePtr) {
                        importNode = topNode();
                        pop();
                    }
                    if (!findChildNodeByType(importNode, YANG_DECL_REVISION)) {
                        smiPrintError(currentParser, ERR_IMPORT_REQUIRES_REVISION);
                    }
		}
        ;

optionalRevision: {}
                    |
                      revisionStatement stmtSep 
                    ;

includeStatement: includeKeyword identifierStr
		{
                    node = addYangNode($2, YANG_DECL_INCLUDE, topNode());
                    pushNode(node);
		}
                includeStatementBody
		{
                    _YangNode *includedModule = externalModule(topNode());
                    yangValidateInclude(thisModulePtr, includedModule);
                    pop();
                    if (topNode() != thisModulePtr) {
                        pop();
                    }
		}
        ;

includeStatementBody:         ';' {}
                |
                    '{'
                        stmtSep
                        optionalRevision
                    '}' {}
                ;

featureStatement: featureKeyword identifierStr
                {
                    node = addYangNode($2, YANG_DECL_FEATURE, topNode());
                    pushNode(node);
                }
                  featureSpec
                {
                    pop();
                }
                ;


featureSpec:    ';' {}
            |   
                '{'
                    stmtSep
                    featureSubstatement_0n
                '}' {}
                ;

featureSubstatement_0n: {}
                |
                   featureSubstatement_0n featureSubstatement stmtSep
                ;

featureSubstatement:    ifFeatureStatement
                    |
                        statusStatement
                    |
                        descriptionStatement
                    |   
                        referenceStatement;

ifFeatureStatement: ifFeatureKeyword identifierRef stmtEnd
                    {
                        node = addYangNode($2, YANG_DECL_IF_FEATURE, topNode());
                        createIdentifierRef(node, getPrefix($2), getIdentifier($2));
                    }
                    ;

identityStatement: identityKeyword identifierStr 
                {
                    node = addYangNode($2, YANG_DECL_IDENTITY, topNode());
                    pushNode(node);
                }
                   identitySpec
                {
                    pop();
                };

identitySpec:    ';' {}
            |   
                '{'
                        stmtSep
                        identitySubstatement_0n
                '}' {}
                ;

identitySubstatement_0n: {}
                |
                   identitySubstatement_0n identitySubstatement stmtSep
                ;

identitySubstatement:   baseStatement
                    |
                        statusStatement
                    |
                        descriptionStatement
                    |   
                        referenceStatement;

typedefStatement:   typedefKeyword identifierStr
                {
                    if (getBuiltInType($2) != YANG_TYPE_NONE) {
                        smiPrintError(thisParserPtr, ERR_ILLEGAL_TYPE_NAME, $2);
                    }
                    node = addYangNode($2, YANG_DECL_TYPEDEF, topNode());
                    pushNode(node);
                }
                '{'
                    stmtSep
                    typedefSubstatement_0n
                '}'
                {                                
                    if (getCardinality(topNode(), YANG_DECL_TYPE) != 1) {
                        smiPrintError(currentParser, ERR_WRONG_CARDINALITY, yangDeclAsString(YANG_DECL_TYPE), "1");
                    }
                    pop();
                }
        ;

typedefSubstatement_0n:	{}
                |
                       typedefSubstatement_0n typedefSubstatement stmtSep
                ;				

typedefSubstatement:
                        typeStatement
                    |
                        unitsStatement
                    |
                        defaultStatement
                    |
                        statusStatement
                    |
                        referenceStatement
                    |
                        descriptionStatement
                ;

typeStatement: typeKeyword identifierRef 
               {
                    node = addYangNode($2, YANG_DECL_TYPE, topNode());
                    createTypeInfo(node);
                    if (getBuiltInType($2) == YANG_TYPE_NONE) {
                        createIdentifierRef(node, getPrefix($2), getIdentifier($2));
                    }
                    pushNode(node);
               }
               optionalTypeBodyStatements
               {
                    pop();
               }
            ;

optionalTypeBodyStatements: ';' {}
                        |   
                            '{'
                                    stmtSep
                                    typeBodyStmts
                            '}' {}
                        ;

typeBodyStmts:  numRestriction
            |
                stringRestriction_0n
            |
                enumSpec
            |
                leafrefSpec
            |
                identityrefSpec
            |
                bitsSpec
            |
                unionSpec
            ;


numRestriction: range stmtSep
                fractionDigitsStatement
            |
                fractionDigitsStatement
                range stmtSep
            |
                fractionDigitsStatement
            |
                range stmtSep
            ;

range:	rangeKeyword string
		{
                    uniqueNodeKind(topNode(), YANG_DECL_RANGE);
                    node = addYangNode($2, YANG_DECL_RANGE, topNode());
                    pushNode(node);
		}
		optionalRestrictionSpec
		{
			pop();
		}
	;

fractionDigitsStatement: fractionDigitsKeyword string stmtEnd
		{
                    uniqueNodeKind(topNode(), YANG_DECL_FRACTION_DIGITS);
                    node = addYangNode($2, YANG_DECL_FRACTION_DIGITS, topNode());
		}
	;

stringRestriction_0n: {}
                |
                    stringRestriction_0n stringRestriction
                ;

stringRestriction: length
                |
                   pattern
                ;

length:	lengthKeyword string
		{
                    uniqueNodeKind(topNode(), YANG_DECL_LENGTH);
                    node = addYangNode($2, YANG_DECL_LENGTH, topNode());
                    pushNode(node);
                }
                optionalRestrictionSpec
                {
			pop();
		}
	;


pattern: patternKeyword string
        {
            node = addYangNode($2, YANG_DECL_PATTERN, topNode());
            pushNode(node);
        }
        optionalRestrictionSpec
        {
            pop();
        }
	;

enumSpec:   enum stmtSep
        |
            enumSpec enum stmtSep
        ;

enum:   enumKeyword string 
        {
            if (strlen($2) == 0 || isWSP($2[0]) || isWSP($2[strlen($2)-1])) {
                smiPrintError(currentParser, ERR_WRONG_ENUM);
            }
            node = findChildNodeByTypeAndValue(topNode(), YANG_DECL_ENUM, $2);
            if (node) {
                smiPrintError(currentParser, ERR_DUPLICATED_ENUM_NAME, $2);
            }
            node = addYangNode($2, YANG_DECL_ENUM, topNode());
            pushNode(node);
        }
        enumSubstatementSpec
        {
            pop();
        }
	;

enumSubstatementSpec: ';' {}
        |
          '{'
                stmtSep
                enumSubstatement_0n
          '}' {}
        ;

enumSubstatement_0n: {}
                |
                    enumSubstatement_0n enumSubstatement stmtSep
                ;

enumSubstatement:    valueStatement
                |
                     statusStatement
                |
                     descriptionStatement
                |
                     referenceStatement
        ;

valueStatement: valueKeyword string stmtEnd 
            {                
                node = addYangNode($2, YANG_DECL_VALUE, topNode());
            }
            ;

optionalRestrictionSpec: ';' {}
            |
                 '{'
                        stmtSep
                        restrictionSpec_0n
                 '}' {}
            ;

restrictionSpec_0n: {}
                |
                    restrictionSpec_0n restrictionSpec stmtSep
                ;

restrictionSpec: descriptionStatement
            |
                 referenceStatement
            |
                 errorMessageStatement
            |
                 errorAppTagStatement
            ;

errorMessageStatement: error_messageKeyword string stmtEnd
            {
                uniqueNodeKind(topNode(), YANG_DECL_ERROR_MESSAGE);
                node = addYangNode($2, YANG_DECL_ERROR_MESSAGE, topNode());
            }
            ;

errorAppTagStatement: error_app_tagKeyword string stmtEnd
            {
                uniqueNodeKind(topNode(), YANG_DECL_ERROR_APP_TAG);
                node = addYangNode($2, YANG_DECL_ERROR_APP_TAG, topNode());
            }
        ;

leafrefSpec: path stmtSep
        |
             path stmtSep
             requireInstanceStatement stmtSep
        |
             requireInstanceStatement stmtSep
             path stmtSep
        ;

path:	pathKeyword string stmtEnd
		{
            uniqueNodeKind(topNode(), YANG_DECL_PATH);
            node = addYangNode($2, YANG_DECL_PATH, topNode());
		}
	;

requireInstanceStatement: requireInstanceKeyword booleanValue stmtEnd
        {
            uniqueNodeKind(topNode(), YANG_DECL_REQUIRE_INSTANCE);
            if ($2 == YANG_BOOLEAN_TRUE) {
                node = addYangNode("true", YANG_DECL_REQUIRE_INSTANCE, topNode());
            } else {
                node = addYangNode("false", YANG_DECL_REQUIRE_INSTANCE, topNode());
            }
        }
        ;

identityrefSpec: baseStatement stmtSep
            |
                 baseStatement stmtSep
                 requireInstanceStatement stmtSep
            |
                 requireInstanceStatement stmtSep
                 baseStatement stmtSep
            ;

baseStatement: baseKeyword identifierRef stmtEnd 
            {
                uniqueNodeKind(topNode(), YANG_DECL_BASE);
                node = addYangNode($2, YANG_DECL_BASE, topNode());
                createIdentifierRef(node, getPrefix($2), getIdentifier($2));
            }
            ;

bitsSpec:   bitsStatement stmtSep
        |
            bitsSpec bitsStatement stmtSep
        ;

bitsStatement: bitKeyword identifier 
            {
                node = findChildNodeByTypeAndValue(topNode(), YANG_DECL_BIT, $2);
                if (node) {
                    smiPrintError(currentParser, ERR_DUPLICATED, "bit", $2);
                }

                node = addYangNode($2, YANG_DECL_BIT, topNode());
                pushNode(node);
            }
            bitsSubstatementSpec
            {
                pop();
            }
            ;

bitsSubstatementSpec:   ';' {}
            |
               '{'
                    stmtSep
                    bitsSubstatement_0n
               '}' {}
            ;

bitsSubstatement_0n: {}
        |
           bitsSubstatement_0n bitsSubstatement stmtSep
        ;

bitsSubstatement:   positionStatement
                |
                    descriptionStatement
                |
                    referenceStatement
                |
                    statusStatement
                ;

positionStatement: positionKeyword string stmtEnd 
                {
		    _YangNode* typePtr, *childPtr;
                    if (!isNonNegativeInteger($2)) {
                        smiPrintError(currentParser, ERR_ARG_VALUE, $2, "non-negative-integer");
                    }                    
                    uniqueNodeKind(topNode(), YANG_DECL_POSITION);

                    /* position values must be unique within the type */
                    typePtr = topNode()->parentPtr;
                    childPtr = typePtr->firstChildPtr;
                    while (childPtr) {
                        if (childPtr->export.nodeKind == YANG_DECL_BIT) {
                            _YangNode* positionPtr = findChildNodeByTypeAndValue(childPtr, YANG_DECL_POSITION, $2);
                            if (positionPtr) {
                                smiPrintError(currentParser, ERR_DUPLICATED, "position", $2);
                            }
                        }
                        childPtr = childPtr->nextSiblingPtr;
                    }

                    node = addYangNode($2, YANG_DECL_POSITION, topNode());
                }
                ;

unionSpec: typeStatement stmtSep
        |
           unionSpec typeStatement stmtSep
        ;

stmtEnd:    ';' {}
         |
            '{'
                    unknownStatement0_n
            '}' {}
        ;

stmtSep: {}
	|
        unknownStatement0_n
	;

 /*
  * Complex-type extension definitions. Nokia Siemens Networks Oy contribution.
  */        
unknownStatement0_n: {}
	|
        unknownStatement unknownStatement0_n
        |
        complexTypeStatement unknownStatement0_n
        |
        instanceStatement unknownStatement0_n
        |
        instanceListStatement unknownStatement0_n
        |
        instanceTypeStatement unknownStatement0_n
        |
        abstractStatement unknownStatement0_n
        |
        extendsStatement unknownStatement0_n
	;

complexTypeStatement: complexTypeKeyword identifierStr
                    {
                        node = addCTExtNode(thisModulePtr, YANG_DECL_COMPLEX_TYPE, 0, topNode(), getPrefix($1), getIdentifier($1), $1, $2);
                        pushNode(node);
                    }
                    complexTypeSpec
                    {
                        checkUnknownStatement();
                        pop();
                    }
	;

complexTypeSpec:  ';' {}
                |
                '{'
                    stmtSep
                    complexTypeSubstatement_0n
                '}'
	;

complexTypeSubstatement_0n: {}
		|
		       complexTypeSubstatement stmtSep complexTypeSubstatement_0n
	;

complexTypeSubstatement:                     
                        groupingStatement
                    |
                        ifFeatureStatement
                    |
                        dataDefStatement
                    |
                        mustStatement
                    |
                        typedefStatement
                    |
                        keyStatement
                    |
                        ordered_byStatement
                    |
                        refineStatement
                    |
                        descriptionStatement
                    |
                        statusStatement
                    |
                        referenceStatement
                    ;

abstractStatement: abstractKeyword trueKeyword stmtEnd
                {
                    node = addCTExtNode(thisModulePtr, YANG_DECL_ABSTRACT, 1, topNode(), getPrefix($1), getIdentifier($1), $1, "true");
                }
            |
                   abstractKeyword falseKeyword stmtEnd
                {
                    node = addCTExtNode(thisModulePtr, YANG_DECL_ABSTRACT, 1, topNode(), getPrefix($1), getIdentifier($1), $1, "false");
                }
            ;

extendsStatement: extendsKeyword identifierRef 
                {
                    node = addCTExtNode(thisModulePtr, YANG_DECL_EXTENDS, 1, topNode(), getPrefix($1), getIdentifier($1), $1, $2);
                    if (node->export.nodeKind == YANG_DECL_EXTENDS) {
                        createIdentifierRef(node, getPrefix($2), getIdentifier($2));
                    }
                    pushNode(node);
                }
                extendsStatementSpec
                {
                     pop();
                }
            ;

extendsStatementSpec: ';' {}
                    |
                        '{'
                            stmtSep
                            extendsSubstatement_0n
                        '}'
                    ;

extendsSubstatement_0n: {}
		|
		       extendsSubstatement stmtSep extendsSubstatement_0n
                ;

extendsSubstatement: descriptionStatement
                |
                    statusStatement
                |
                    referenceStatement
                ;

instanceStatement: instanceKeyword identifierStr
                {
                    node = addCTExtNode(thisModulePtr, YANG_DECL_INSTANCE, 0, topNode(), getPrefix($1), getIdentifier($1), $1, $2);
                    pushNode(node);
                }
                '{'
                    stmtSep
                    instanceSubstatement_0n
                '}'
                {
                    checkUnknownStatement();
                    if (topNode()->export.nodeKind != YANG_DECL_UNKNOWN_STATEMENT && getCardinality(topNode(), YANG_DECL_INSTANCE_TYPE) == 0) {
                        smiPrintErrorAtLine(currentParser, ERR_WRONG_CARDINALITY, topNode()->line, yangDeclAsString(YANG_DECL_INSTANCE_TYPE), "1");
                    }
                    pop();
                }
            ;

instanceSubstatement_0n: {}
		|
		    instanceSubstatement stmtSep instanceSubstatement_0n
                ;

instanceSubstatement:
                        ifFeatureStatement
                    |
                        commonStatement
                    |
                        dataDefStatement
                    |
                        mustStatement
                    |
                        whenStatement
                    |
                        mandatoryStatement
                    |
                        augmentStatement
                ;

instanceListStatement: instanceListKeyword identifierStr
                {
                    node = addCTExtNode(thisModulePtr, YANG_DECL_INSTANCE_LIST, 0, topNode(), getPrefix($1), getIdentifier($1), $1, $2);
                    pushNode(node);
                }
                '{'
                    stmtSep
                    instanceListSubstatement_0n
                '}'
                {
                    checkUnknownStatement();
                    if (topNode()->export.nodeKind != YANG_DECL_UNKNOWN_STATEMENT && getCardinality(topNode(), YANG_DECL_INSTANCE_TYPE) == 0) {
                        smiPrintErrorAtLine(currentParser, ERR_WRONG_CARDINALITY, topNode()->line, yangDeclAsString(YANG_DECL_INSTANCE_TYPE), "1");
                    }
                    pop();
                }
            ;

instanceListSubstatement_0n: {}
		|
		    instanceListSubstatement stmtSep instanceListSubstatement_0n
                ;

instanceListSubstatement:
                        ifFeatureStatement
                    |
                        commonStatement
                    |
                        dataDefStatement
                    |
                        mustStatement
                    |
                        whenStatement
                    |
                        augmentStatement
                    |
                        max_elementsStatement
                    |
                        min_elementsStatement
                    |
                        ordered_byStatement
                ;


instanceTypeStatement: instanceTypeKeyword identifierRef stmtEnd
                    {
                        node = addCTExtNode(thisModulePtr, YANG_DECL_INSTANCE_TYPE, 1, topNode(), getPrefix($1), getIdentifier($1), $1, $2);
                        if (node->export.nodeKind == YANG_DECL_INSTANCE_TYPE) {
                            createIdentifierRef(node, getPrefix($2), getIdentifier($2));
                        }
                    }
	;
 /*
  * End complex-type extension definitions. Nokia Siemens Networks Oy contribution.
  */

unknownStatement:   identifierRefArg 
                    {
                        node = addYangNode($1, YANG_DECL_UNKNOWN_STATEMENT, topNode());
                        createIdentifierRef(node, getPrefix($1), getIdentifier($1));
                        pushNode(node);
                    }
                    stmtEnd 
                    {
                        pop();
                    }
        |                    
                    identifierRefArg string
                    {
                        node = addYangNode($1, YANG_DECL_UNKNOWN_STATEMENT, topNode());
                        createIdentifierRef(node, getPrefix($1), getIdentifier($1));
                        node->export.extra = smiStrdup($2);
                        pushNode(node);
                    }
                    stmtEnd 
                    {
                        pop();
                    }
        ;

containerStatement: containerKeyword identifierStr
			{
                            node = addYangNode($2, YANG_DECL_CONTAINER, topNode());
                            pushNode(node);
			}
                        containerSpec
			{
                            pop();
			}
	;

containerSpec:  ';' {}
            |
                '{'
                    stmtSep
                    containerSubstatement_0n
                '}'
            ;

containerSubstatement_0n: {}
		|
		       containerSubstatement stmtSep containerSubstatement_0n
	;

containerSubstatement:	ifFeatureStatement
                    |
                        commonStatement
                    |
                        dataDefStatement
                    |
                        groupingStatement
                    |
                        mustStatement
                    |
                        whenStatement
                    |
                        presenceStatement
                    |
                        typedefStatement
                    ;

mustStatement: mustKeyword string
		{
                    node = addYangNode($2, YANG_DECL_MUST, topNode());
                    pushNode(node);
		}
		'{'
			mustSubstatement_0n
		'}'
		{
			pop();
		}
	|
		mustKeyword string ';'
		{
                    node = addYangNode($2, YANG_DECL_MUST, topNode());
		}
	;

mustSubstatement_0n: {}
            |
		       mustSubstatement_0n mustSubstatement stmtSep
        ;

mustSubstatement:   errorMessageStatement
                |
                    errorAppTagStatement
                |
                    descriptionStatement
                |
                    referenceStatement
                ;

presenceStatement: presenceKeyword string stmtEnd
                {
                    uniqueNodeKind(topNode(), YANG_DECL_PRESENCE);
                    node = addYangNode($2, YANG_DECL_PRESENCE, topNode());
                }
                ;
			
configStatement: configKeyword trueKeyword stmtEnd
                {
                    uniqueNodeKind(topNode(), YANG_DECL_CONFIG);
                    setConfig(topNode(), YANG_CONFIG_TRUE);
                    node = addYangNode("true", YANG_DECL_CONFIG, topNode());
                }
            |
                    configKeyword falseKeyword stmtEnd
                {
                    uniqueNodeKind(topNode(), YANG_DECL_CONFIG);
                    setConfig(topNode(), YANG_CONFIG_FALSE);
                    node = addYangNode("false", YANG_DECL_CONFIG, topNode());
                }
            ;

mandatoryStatement: mandatoryKeyword trueKeyword stmtEnd
			{
                            uniqueNodeKind(topNode(), YANG_DECL_MANDATORY);
                            node = addYangNode("true", YANG_DECL_MANDATORY, topNode());
			}
                    |
		    	mandatoryKeyword falseKeyword stmtEnd	
			{
                            uniqueNodeKind(topNode(), YANG_DECL_MANDATORY);
                            node = addYangNode("false", YANG_DECL_MANDATORY, topNode());
			}
		;
			
leafStatement: leafKeyword identifierStr
			{
                            node = addYangNode($2, YANG_DECL_LEAF, topNode());
                            pushNode(node);
			}
			'{'
                            stmtSep
                            leafSubstatement_0n
			'}'
			{
                            if (getCardinality(topNode(), YANG_DECL_TYPE) != 1) {
                                smiPrintError(currentParser, ERR_WRONG_CARDINALITY, yangDeclAsString(YANG_DECL_TYPE), "1");
                            }
                            pop();
			}
		;
			
leafSubstatement_0n: {}
            |
                   leafSubstatement_0n leafSubstatement stmtSep
        ;

leafSubstatement:   ifFeatureStatement
                |
                    unitsStatement
                |
                    mustStatement
                |
                    commonStatement
                |
                    mandatoryStatement
                |
                    typeStatement
                |
                    defaultStatement	
                |
                    whenStatement
                ;

leaf_listStatement: leaf_listKeyword identifierStr
			{
                            node = addYangNode($2, YANG_DECL_LEAF_LIST, topNode());
                            pushNode(node);
			}
			'{'
                            stmtSep
                            leaf_listSubstatement_0n
			'}'
			{
                            if (getCardinality(topNode(), YANG_DECL_TYPE) != 1) {
				smiPrintError(currentParser, ERR_WRONG_CARDINALITY, yangDeclAsString(YANG_DECL_TYPE), "1");
                            }
                            pop();
			}
		;
			
leaf_listSubstatement_0n: {}
		|
		       leaf_listSubstatement_0n leaf_listSubstatement stmtSep
        ;

leaf_listSubstatement:	mustStatement
                    |
                        commonStatement
                    |
                        typeStatement
                    |
                        ifFeatureStatement
                    |
                        unitsStatement
                    |	
                        whenStatement
                    |	
                        max_elementsStatement
                    |
                        min_elementsStatement
                    |
                        ordered_byStatement
                    ;
		
listStatement: listKeyword identifierStr
			{
                            node = addYangNode($2, YANG_DECL_LIST, topNode());
                            pushNode(node);
			}
			'{'
                            stmtSep
                            listSubstatement_0n
			'}'
			{
                            pop();
			}
		;

listSubstatement_0n: {}
                |
		       listSubstatement_0n listSubstatement stmtSep
        	;

listSubstatement:   mustStatement
                |
                    commonStatement
                |
                    max_elementsStatement
                |
                    min_elementsStatement
                |
                    ordered_byStatement
                |
                    keyStatement
                |
                    uniqueStatement
                |
                    dataDefStatement
                |
                    typedefStatement
                |
                    groupingStatement
                |
                    whenStatement
                |
                    ifFeatureStatement
                ;

max_elementsStatement: 	max_elementsKeyword string stmtEnd
                    {
                        if (!isNonNegativeInteger($2)) {                            
                            smiPrintError(currentParser, ERR_ARG_VALUE, $2, "non-negative-integer");
                        }
                        uniqueNodeKind(topNode(), YANG_DECL_MAX_ELEMENTS);
                        node = addYangNode($2, YANG_DECL_MAX_ELEMENTS, topNode());
                    }
                ;

min_elementsStatement: 	min_elementsKeyword string stmtEnd
                    {
                        if (!isNonNegativeInteger($2)) {
                            smiPrintError(currentParser, ERR_ARG_VALUE, $2, "non-negative-integer");
                        }
                        uniqueNodeKind(topNode(), YANG_DECL_MIN_ELEMENTS);
                        node = addYangNode($2, YANG_DECL_MIN_ELEMENTS, topNode());
                    }
        		;

ordered_byStatement: 	ordered_byKeyword string stmtEnd
                    {
                        if (!strcmp($2, "user") || !strcmp($2, "system")) {
                        } else {
                            smiPrintError(thisParserPtr, ERR_IVALIDE_ORDERED_BY_VALUE, $2);
                        }
                        uniqueNodeKind(topNode(), YANG_DECL_ORDERED_BY);
                        node = addYangNode($2, YANG_DECL_ORDERED_BY, topNode());
                    }
                ;

keyStatement: keyKeyword string stmtEnd
		{
                    uniqueNodeKind(topNode(), YANG_DECL_KEY);
                    node = addYangNode($2, YANG_DECL_KEY, topNode());
                    node->info = getKeyList($2);
		}
	;
	
uniqueStatement: uniqueKeyword string stmtEnd
		{
		    YangList *il;
                    node = addYangNode($2, YANG_DECL_UNIQUE, topNode());
                    il = getUniqueList($2);
                    node->info = processUniqueList(node, il);
                    freeIdentiferList(il);
		}
	;

choiceStatement: choiceKeyword identifierStr
		{
                    node = addYangNode($2, YANG_DECL_CHOICE, topNode());
                    pushNode(node);
		}
        choiceSpec
		{
                    pop();
		}
		;
 
choiceSpec: ';' {}
        |
            '{'
                stmtSep
                choiceSubstatement_0n			
            '}' {}
        ;

choiceSubstatement_0n: {}
		|
		       choiceSubstatement_0n choiceSubstatement stmtSep
        ;

choiceSubstatement: commonStatement
                  |
                    defaultStatement
                  |
                    whenStatement
                  |
                    ifFeatureStatement
                  |
                    mandatoryStatement
                  |
                    caseStatement
                ;

caseStatement: 	caseKeyword identifierStr
		{
                    node = findChildNodeByTypeAndValue(topNode(), YANG_DECL_CASE, $2);
                    if (node) {
                        smiPrintError(currentParser, ERR_DUPLICATED_CASE_IDENTIFIER, $2);
                    }
                    node = addYangNode($2, YANG_DECL_CASE, topNode());
                    pushNode(node);
		}
                caseSpec
		{
                    pop();
		}	
            |
                containerStatement
            |
                leafStatement
            |
                leaf_listStatement
            |
                listStatement
            |
                anyXMLStatement
            ;

caseSpec:   ';' {}
        |
            '{'
                stmtSep
                caseSubstatement_0n
            '}' {}
        ;

caseSubstatement_0n: {}
            |
                caseSubstatement_0n caseSubstatement  stmtSep
            ;

caseSubstatement:   descriptionStatement
                |
                    statusStatement
                |
                    referenceStatement
                |
                    whenStatement
                |
                    ifFeatureStatement
                |
                    caseDataDef
                ;


caseDataDef:    containerStatement
            |
                leafStatement
            |
                leaf_listStatement
            |
                listStatement
            |
                anyXMLStatement
            |
                usesStatement
            ;


groupingStatement: groupingKeyword identifierStr
		{
                    node = addYangNode($2, YANG_DECL_GROUPING, topNode());
                    pushNode(node);
		}
		'{'
                    stmtSep
                    groupingSubstatement_0n
		'}'
		{
                    pop();
		}
		;

groupingSubstatement_0n: {}
		|
                    groupingSubstatement_0n groupingSubstatement stmtSep
            ;

groupingSubstatement:	statusStatement
                    |
                        descriptionStatement
                    |
                        referenceStatement
                    |
                        dataDefStatement
                    |
                        groupingStatement
                    |
                        typedefStatement
                    ;

usesStatement:  usesKeyword identifierRef
            {
                node = addYangNode($2, YANG_DECL_USES, topNode());
                createIdentifierRef(node, getPrefix($2), getIdentifier($2));
                pushNode(node);
            }
            '{'
                usesSubstatement_0n
            '}'
            {
                pop();
            }
        |
            usesKeyword identifierRef ';'
            {
                node = addYangNode($2, YANG_DECL_USES, topNode());
                createIdentifierRef(node, getPrefix($2), getIdentifier($2));
            }
            ;

usesSubstatement_0n: {}
		|
		       usesSubstatement_0n usesSubstatement stmtSep
        ;

usesSubstatement:   descriptionStatement
                |
                    referenceStatement
                |
                    statusStatement
                |
                    whenStatement
                |   
                    ifFeatureStatement
                | 
                    refineStatement
                |
                    augmentStatement
                    
		;

refineStatement:    refineKeyword string
                    {
                        if (!isDescendantSchemaNodeid($2)) {
                            smiPrintError(thisParserPtr, ERR_DESCEDANT_FORM, $2);
                        }
                        node = addYangNode($2, YANG_DECL_REFINE, topNode());
                        pushNode(node);
                    }
                    refineSpec
                    {
                        pop();
                    }
                    ;

refineSpec: ';' {}
        |   
            '{'
                stmtSep
                refineSubstatement
            '}' {}
        ;

refineSubstatement: refine_0n;

refine_0n:  {}
         |
            refine_0n refine stmtSep;

refine: mustStatement
    |
        presenceStatement
    |
        configStatement
    |
        descriptionStatement
    |
        referenceStatement
    |
        defaultStatement
    |
        mandatoryStatement
    |
        min_elementsStatement
    |
        max_elementsStatement

    ;

augmentStatement: augmentKeyword string 
		{
                    if (topDecl() == YANG_DECL_USES || topDecl() == YANG_DECL_INSTANCE || topDecl() == YANG_DECL_INSTANCE_LIST) {
                        if (!isDescendantSchemaNodeid($2)) {
                            smiPrintError(thisParserPtr, ERR_DESCEDANT_FORM, $2);
                        }
                    } else {
                        if (!isAbsoluteSchemaNodeid($2)) {
                            smiPrintError(thisParserPtr, ERR_ABSOLUTE_FORM, $2);
                        }
                    }
                    node = addYangNode($2, YANG_DECL_AUGMENT, topNode());
                    pushNode(node);
		}
		'{'
                    stmtSep
                    augmentSubstatement_0n
                '}'
		{
                    int count = 0;
		    node = topNode()->firstChildPtr;
                    while (node) {
                        if (node->export.nodeKind == YANG_DECL_CASE ||
                            isDataDefNode(node)) {
                                count++;
                        }
                        node = node->nextSiblingPtr;
                    }
                    if (count == 0) {
                        smiPrintError(thisParserPtr, ERR_DATADEF_NODE_REQUIRED, $2);
                    }

                    pop();
		}
		;

augmentSubstatement_0n:	stmtSep {}
                |
                        augmentSubstatement_0n augmentSubstatement stmtSep
                ;

augmentSubstatement:	whenStatement
                    |
                        ifFeatureStatement
                    |
                        descriptionStatement
                    |
                        referenceStatement
                    |
                        statusStatement
                    |
                        dataDefStatement
                    |
                        caseStatement
                    ;


whenStatement:	whenKeyword string stmtEnd
	    {
                uniqueNodeKind(topNode(), YANG_DECL_WHEN);
                node = addYangNode($2, YANG_DECL_WHEN, topNode());
            }
        ;

rpcStatement: rpcKeyword identifierStr
            {
                node = addYangNode($2, YANG_DECL_RPC, topNode());
                pushNode(node);
            }
            rpcSpec
            {
                pop();
            }
            ;

rpcSpec:    ';' {}
        |
            '{'
                stmtSep
                rpcSubstatement_0n
            '}' {}
        ;

rpcSubstatement_0n: {}
		|
		       rpcSubstatement_0n rpcSubstatement  stmtSep
        	;

rpcSubstatement:    ifFeatureStatement
                |
                    descriptionStatement
                |
                    referenceStatement
                |
                    statusStatement
                |
                    typedefStatement
                |
                    groupingStatement
                |
                    inputStatement
                |
                    outputStatement
            ;

inputStatement: inputKeyword
		{
                    node = addYangNode(NULL, YANG_DECL_INPUT, topNode());
                    pushNode(node);
		}
		'{'
                    stmtSep
                    inputOutputSubstatement_0n
		'}'
		{
                    int numberDataDefStmts = 0;
                    _YangNode *childPtr = topNode()->firstChildPtr;
                    while (childPtr) {
                        if (!(childPtr->export.nodeKind == YANG_DECL_TYPEDEF ||
                            childPtr->export.nodeKind == YANG_DECL_GROUPING)) {
                                numberDataDefStmts++;
                            }
                        childPtr = childPtr->nextSiblingPtr;
                    }
                    if (!numberDataDefStmts) {
                        smiPrintError(thisParserPtr, ERR_DATA_DEF_REQUIRED, "input");
                    }
                    pop();
		}
		;

inputOutputSubstatement_0n:	inputOutputSubstatement stmtSep
                        |
                                inputOutputSubstatement_0n inputOutputSubstatement stmtSep
                        ;

inputOutputSubstatement:    dataDefStatement
                        |
                            groupingStatement
                        |
                            typedefStatement
                        ;

outputStatement: outputKeyword
		{
                    node = addYangNode(NULL, YANG_DECL_OUTPUT, topNode());
                    pushNode(node);
		}
		'{'
                    stmtSep
                    inputOutputSubstatement_0n
		'}'
		{
                    int numberDataDefStmts = 0;
                    _YangNode *childPtr = topNode()->firstChildPtr;
                    while (childPtr) {
                        if (!(childPtr->export.nodeKind == YANG_DECL_TYPEDEF ||
                            childPtr->export.nodeKind == YANG_DECL_GROUPING)) {
                                numberDataDefStmts++;
                            }
                        childPtr = childPtr->nextSiblingPtr;
                    }
                    if (!numberDataDefStmts) {
                        smiPrintError(thisParserPtr, ERR_DATA_DEF_REQUIRED, "input");
                    }
                    pop();
		}
		;


notificationStatement: notificationKeyword identifierStr
                {
                    node = addYangNode($2, YANG_DECL_NOTIFICATION, topNode());
                    pushNode(node);
                }
                notificationSpec
                {
                    pop();
                }
                ;

notificationSpec:   ';' {}
                |
                    '{'
                        stmtSep
                        notificationSubstatement_0n
                    '}' {}
                    ;


notificationSubstatement_0n: {}
		|
		       notificationSubstatement_0n notificationSubstatement stmtSep
                ;

notificationSubstatement:   ifFeatureStatement
                        |
                            descriptionStatement
                        |
                            referenceStatement
                        |
                            statusStatement
                        |	
                            dataDefStatement
                        |
                            groupingStatement
                        |
                            typedefStatement
                        ;

deviationStatement: deviationKeyword string 
                {
                        node = addYangNode($2, YANG_DECL_DEVIATION, topNode());
                        pushNode(node);
                }
                '{'
                    stmtSep
                    deviationSubstatement_0n
                '}'
                {
                    pop();
                }
                ;

deviationSubstatement_0n:   deviationSubstatement stmtSep
                        |
                            deviationSubstatement_0n deviationSubstatement stmtSep;

deviationSubstatement:  descriptionStatement
                    |
                        referenceStatement
                    |
                        deviateNotSupported
                    |
                        deviateAddStatement
                    |
                        deviateDeleteStatement
                    |
                        deviateReplaceStatement
                    ;

deviateNotSupported: deviateKeyword notSupportedKeyword ';'
                    {
                        uniqueNodeKind(topNode(), YANG_DECL_DEVIATE);
                        node = addYangNode("not-supported", YANG_DECL_DEVIATE, topNode());
                    }
                |
                     deviateKeyword notSupportedKeyword '{' stmtSep '}'
                    {
                        uniqueNodeKind(topNode(), YANG_DECL_DEVIATE);
                        node = addYangNode("not-supported", YANG_DECL_DEVIATE, topNode());
                    }
                ;

deviateAddStatement:    deviateKeyword addKeyword 
                    {
                        node = addYangNode("add", YANG_DECL_DEVIATE, topNode());
                        pushNode(node);
                    }
                        deviateAddSpec
                    {
                        pop();
                    }
                    ;

deviateAddSpec: ';' {}
            |
                '{'
                    stmtSep
                    deviateAddSubstatement_0n
                '}' {}
            ;

deviateAddSubstatement_0n: {}
                    |
                        deviateAddSubstatement_0n deviateAddSubstatement stmtSep;

deviateAddSubstatement:     unitsStatement
                    |
                            mustStatement
                    |
                            uniqueStatement
                    |
                            defaultStatement
                    |
                            configStatement
                    |
                            mandatoryStatement
                    |
                            min_elementsStatement
                    |
                            max_elementsStatement
                    ;

deviateDeleteStatement:    deviateKeyword deleteKeyword 
                    {
                        node = addYangNode("delete", YANG_DECL_DEVIATE, topNode());
                        pushNode(node);
                    }
                        deviateDeleteSpec
                    {
                        pop();
                    }
                    ;

deviateDeleteSpec: ';' {}
            |
                '{'
                    stmtSep
                    deviateDeleteSubstatement_0n
                '}' {}
            ;

deviateDeleteSubstatement_0n: {}
                    |
                        deviateDeleteSubstatement_0n deviateDeleteSubstatement stmtSep;

deviateDeleteSubstatement:      unitsStatement
                        |
                                mustStatement
                        |
                                uniqueStatement
                        |
                                defaultStatement;

deviateReplaceStatement: deviateKeyword replaceKeyword 
                    {
                        node = addYangNode("replace", YANG_DECL_DEVIATE, topNode());
                        pushNode(node);
                    }
                        deviateReplaceSpec
                    {
                        pop();
                    }
                    ;

deviateReplaceSpec: ';' {}
            |
                '{'
                    stmtSep
                    deviateReplaceSubstatement_0n
                '}' {}
            ;

deviateReplaceSubstatement_0n: {}
                    |
                        deviateReplaceSubstatement_0n deviateReplaceSubstatement stmtSep;

deviateReplaceSubstatement: typeStatement
                        |
                            unitsStatement
                        |
                            defaultStatement
                        |
                            configStatement
                        |
                            mandatoryStatement
                        |
                            min_elementsStatement
                        |
                            max_elementsStatement
                        ;


anyXMLStatement: anyXMLKeyword identifierStr
		{
            node = addYangNode($2, YANG_DECL_ANYXML, topNode());
            pushNode(node);
		}
        anyXMLSpec
		{
			pop();
		}
		;

anyXMLSpec: ';' {}
        |
            '{'
                stmtSep
                anyXMLSubstatement_0n
            '}' {}
        ;

anyXMLSubstatement_0n: {}
    		|
		       anyXMLSubstatement_0n anyXMLSubstatement stmtSep
            ;

anyXMLSubstatement: commonStatement
                |
                    whenStatement
                |
                    ifFeatureStatement
                |   
                    mustStatement
                |
                    mandatoryStatement
                ;

extensionStatement: extensionKeyword identifierStr
		{
                    node = addYangNode($2, YANG_DECL_EXTENSION, topNode());
                    pushNode(node);
		}
                extensionStatementBody
		{
                    pop();
		}
		;
		
extensionStatementBody:  '{' stmtSep extensionSubstatement_0n '}' {}
                |
                     ';' {}
                ;                       

extensionSubstatement_0n: {}
		|
                    extensionSubstatement_0n extensionSubstatement stmtSep
		;

extensionSubstatement:	argumentStatement
                |
                        statusStatement
                |
                        descriptionStatement
                |
                        referenceStatement
                ;

argumentStatement:  argumentKeyword identifierStr
                    {
                        uniqueNodeKind(topNode(), YANG_DECL_ARGUMENT);
                        node = addYangNode($2, YANG_DECL_ARGUMENT, topNode());
                        pushNode(node);
                    }
                    argumentStatementBody
                    {
                        pop();
                    }
                ;

argumentStatementBody:  '{' stmtSep yinElementOptional '}' {}
                    |
                         ';' {}
                    ;                       

yinElementOptional: {}
                |
                    yinElementKeyword trueKeyword stmtEnd stmtSep
                    {
                        addYangNode("true", YANG_DECL_YIN_ELEMENT, topNode());
                    }
                |
                    yinElementKeyword falseKeyword stmtEnd stmtSep
                    {
                        addYangNode("false", YANG_DECL_YIN_ELEMENT, topNode());
                    }
                ;

unitsStatement: unitsKeyword string stmtEnd
                {
                    uniqueNodeKind(topNode(), YANG_DECL_UNITS);
                    addYangNode($2, YANG_DECL_UNITS, topNode());
                }
                ;

defaultStatement: defaultKeyword string stmtEnd
                {
                    uniqueNodeKind(topNode(), YANG_DECL_DEFAULT);
                    addYangNode($2, YANG_DECL_DEFAULT, topNode());
                }
                ;

prefix:		identifierStr
        ;

identifierRef:  identifierRefArg
            |
                identifierRefArgStr 
            |
                identifierStr
            |
                complexTypeKeyword
            |
                extendsKeyword 
            |
                abstractKeyword
            |
                instanceKeyword
            |
                instanceListKeyword
            |
                instanceTypeKeyword
            ;

identifierStr:    identifier
                | augmentKeyword {$$ = $1;} | belongs_toKeyword {$$ = $1;} | choiceKeyword {$$ = $1;} | configKeyword {$$ = $1;} | contactKeyword {$$ = $1;} | containerKeyword {$$ = $1;} | defaultKeyword {$$ = $1;} | descriptionKeyword {$$ = $1;}
                | enumKeyword {$$ = $1;} | error_app_tagKeyword  {$$ = $1;} | error_messageKeyword  {$$ = $1;} | extensionKeyword  {$$ = $1;} | groupingKeyword  {$$ = $1;} | importKeyword  {$$ = $1;} | includeKeyword  {$$ = $1;} | keyKeyword  {$$ = $1;} 
                | leafKeyword  {$$ = $1;} | leaf_listKeyword  {$$ = $1;} | lengthKeyword  {$$ = $1;} | listKeyword  {$$ = $1;} | mandatoryKeyword  {$$ = $1;} | max_elementsKeyword  {$$ = $1;} | min_elementsKeyword  {$$ = $1;} | moduleKeyword  {$$ = $1;} 
                | submoduleKeyword  {$$ = $1;} | mustKeyword  {$$ = $1;} | namespaceKeyword  {$$ = $1;} | ordered_byKeyword  {$$ = $1;} | organizationKeyword  {$$ = $1;} | prefixKeyword  {$$ = $1;} | rangeKeyword  {$$ = $1;} | referenceKeyword  {$$ = $1;} 
                | patternKeyword  {$$ = $1;} | revisionKeyword  {$$ = $1;} | statusKeyword  {$$ = $1;} | typeKeyword  {$$ = $1;} | typedefKeyword  {$$ = $1;} | uniqueKeyword  {$$ = $1;} | unitsKeyword  {$$ = $1;} | usesKeyword  {$$ = $1;} | valueKeyword  {$$ = $1;} 
                | whenKeyword  {$$ = $1;} | bitKeyword  {$$ = $1;} | pathKeyword  {$$ = $1;} | anyXMLKeyword  {$$ = $1;} | deprecatedKeyword  {$$ = $1;} | currentKeyword  {$$ = $1;} | obsoleteKeyword  {$$ = $1;} | trueKeyword  {$$ = $1;} | falseKeyword  {$$ = $1;} 
                | caseKeyword  {$$ = $1;} | inputKeyword  {$$ = $1;} | outputKeyword  {$$ = $1;} | rpcKeyword  {$$ = $1;} | notificationKeyword  {$$ = $1;} | argumentKeyword  {$$ = $1;} | yangversionKeyword  {$$ = $1;} | baseKeyword  {$$ = $1;}
                | deviationKeyword  {$$ = $1;} | deviateKeyword  {$$ = $1;} | featureKeyword  {$$ = $1;} | identityKeyword  {$$ = $1;} | ifFeatureKeyword  {$$ = $1;} | positionKeyword  {$$ = $1;} | presenceKeyword  {$$ = $1;} | refineKeyword  {$$ = $1;}
                | requireInstanceKeyword  {$$ = $1;} | yinElementKeyword  {$$ = $1;} | notSupportedKeyword  {$$ = $1;} | addKeyword  {$$ = $1;} | deleteKeyword  {$$ = $1;} | replaceKeyword  {$$ = $1;} 
                ;

booleanValue:   trueKeyword
                {
                    $$ = YANG_BOOLEAN_TRUE;
                }
             |
                falseKeyword
                {
                    $$ = YANG_BOOLEAN_FALSE;
                }
             ;

string:     qString
	|
            uqString
	|
            identifierRef
        |
            dateString
        |
            yangVersion

	;
%%

#endif			
