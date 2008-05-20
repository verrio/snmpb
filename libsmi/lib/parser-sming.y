/*
 * parser-sming.y --
 *
 *      Syntax rules for parsing the SMIng MIB module language.
 *
 * Copyright (c) 1999 Frank Strauss, Technical University of Braunschweig.
 *
 * See the file "COPYING" for information on usage and redistribution
 * of this file, and for a DISCLAIMER OF ALL WARRANTIES.
 *
 * @(#) $Id: parser-sming.y 7966 2008-03-27 21:25:52Z schoenw $
 */

%{

#include <config.h>
    
#ifdef BACKEND_SMING

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

#ifdef HAVE_WIN_H
#include "win.h"
#endif

#include "smi.h"
#include "error.h"
#include "parser-sming.h"
#include "scanner-sming.h"
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
 * NOTE: The argument lvalp ist not really a void pointer. Unfortunately,
 * we don't know it better at this point. bison generated C code declares
 * YYSTYPE just a few lines below based on the `%union' declaration.
 */
extern int yylex(void *lvalp, Parser *parserPtr);



static char *typeIdentifier, *macroIdentifier, *identityIdentifier, 
	    *classIdentifier, *attributeIdentifier;
static char *importModulename = NULL;
static Type *typePtr = NULL;
static Macro *macroPtr = NULL;
static Identity *identityPtr = NULL;
static Class	*classPtr = NULL;
static Attribute *attributePtr = NULL;
static Event *eventPtr = NULL;
static SmiBasetype defaultBasetype = SMI_BASETYPE_UNKNOWN;
static NamedNumber *namedNumberPtr = NULL;
static int bitsFlag = 0; /* used to differentiate bits definition from enum*/
static int attributeFlag = 0; /* 
							   *Used to differentiate between attribute and
							   *and typedef to tie the type statement
							   *respectively to class or module.
							   */


#define SMI_EPOCH	631152000	/* 01 Jan 1990 00:00:00 */ 
 

static Type *
findType(char *spec, Parser *parserPtr, Module *modulePtr)
{
    Type *typePtr;
    Import *importPtr;
    char *module, *type;

    type = strstr(spec, "::");
    if (!type) {
	typePtr = findTypeByModuleAndName(modulePtr, spec);
	if (!typePtr) {
	    importPtr = findImportByName(spec, modulePtr);
	    if (importPtr) {
		typePtr = findTypeByModulenameAndName(importPtr->export.module,
						      spec);
	    }
	}
    } else {
	module = strtok(spec, ":");
	type = &type[2];
	typePtr = findTypeByModulenameAndName(module, type);
    }
    return typePtr;
}
 
 
static Identity *
findIdentity(char *spec, Parser *parserPtr, Module *modulePtr)
{
    Identity *identityPtr;
    Import *importPtr;
    char *module, *identity;

    identity = strstr(spec, "::");
    if (!identity) {
	identityPtr = findIdentityByModuleAndName(modulePtr, spec);
	if (!identityPtr) {
	    importPtr = findImportByName(spec, modulePtr);
	    if (importPtr) {
		identityPtr = findIdentityByModulenameAndName(importPtr->export.module,
						      spec);
	    }
	}
    } else {
	module = strtok(spec, ":");
	identity = &identity[2];
	identityPtr = findIdentityByModulenameAndName(module, identity);
    }
    return identityPtr;
}

static Class *
findClass(char *spec, Parser *parserPtr, Module *modulePtr)
{
    Class *classPtr;
    Import *importPtr;
    char *module, *class;

    class = strstr(spec, "::");
    if (!class) {
	classPtr = findClassByModuleAndName(modulePtr, spec);
	if (!classPtr) {
	    importPtr = findImportByName(spec, modulePtr);
	    if (importPtr) {
		classPtr = findClassByModulenameAndName(importPtr->export.module,
						      spec);
	    }
	}
    } else {
	module = strtok(spec, ":");
	class = &class[2];
	classPtr = findClassByModulenameAndName(module, class);
    }
    return classPtr;
}

static void
checkTypes(Parser *parserPtr, Module *modulePtr)
{
    Type *typePtr;
    
    for(typePtr = modulePtr->firstTypePtr;
	typePtr; typePtr = typePtr->nextPtr) {

	/*
	 * Complain about empty description clauses.
	 */

	if (! parserPtr->flags & SMI_FLAG_NODESCR
	    && (! typePtr->export.description
		|| ! typePtr->export.description[0])) {
	    smiPrintErrorAtLine(parserPtr, ERR_EMPTY_DESCRIPTION,
				typePtr->line, typePtr->export.name);
	}
	
	smiCheckNamedNumberRedefinition(parserPtr, typePtr);
	smiCheckNamedNumberSubtyping(parserPtr, typePtr);
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

static char *hexToStr(char *hexStr, int len)
{
   union{
   	char ch;
   	long l;
   } hex2char;
   
   char* res =(char*)malloc(sizeof(char)*len/2+1);
   char* tmp =(char*)malloc(sizeof(char)*3);
   int i,j = 0;

   tmp[2]=0;
   for(i=2; i+1<len; i+=2)
   {
   	tmp[0]= hexStr[i];
   	tmp[1]= hexStr[i+1];
   	
   	hex2char.l = strtol(tmp,NULL,16);
   	res[j] = hex2char.ch;
   	
   	j++;
   }
   
   smiFree(tmp);
   
   res[j]=0;
   return res;
}

static void createBitsValue(SmiValue *valuePtr, Type *typePtr)
{
    List *bitsListPtr, *valueListPtr, *p, *pp, *nextPtr;
    int nBits, bit;
    
    if (valuePtr->basetype != SMI_BASETYPE_BITS)
	return;
	
	bitsListPtr = typePtr->listPtr;
	valueListPtr = (void *)valuePtr->value.ptr;
	for (nBits = 0, p = bitsListPtr; p; p = p->nextPtr) {
	    if (nBits < 1+((NamedNumber *)(p->ptr))->export.value.value.unsigned64) {
		nBits = 1+((NamedNumber *)(p->ptr))->export.value.value.unsigned64;
	    }
	}
	valuePtr->value.ptr = smiMalloc((nBits+7)/8);
	memset(valuePtr->value.ptr, 0, (nBits+7)/8);
	valuePtr->len = (nBits+7)/8;
	for (p = valueListPtr; p;) {
	    for (pp = bitsListPtr; pp; pp = pp->nextPtr) {
		if (!strcmp(p->ptr,
			    ((NamedNumber *)(pp->ptr))->export.name)) {
		    bit = (int)(((NamedNumber *)(pp->ptr))->export.value.value.unsigned64);
		    valuePtr->value.ptr[bit/8] |=
			1 << (7-(bit%8));
		}
	    }
	    smiFree(p->ptr);
	    nextPtr = p->nextPtr;
	    smiFree(p);
	    p = nextPtr;
	}
}

			    
%}

/*
 * The grammars start symbol.
 */
%start smingFile



/*
 * We call the parser from within the parser when IMPORTing modules,
 * hence we need reentrant parser code. This is a bison feature.
 */
%pure_parser



/*
 * The attributes.
 */
%union {
    char           *id;				/* identifier name           */
    int            rc;				/* >=0: ok, <0: error        */
    time_t	   date;			/* a date value		     */
    char	   *text;
    Module	   *modulePtr;
    Node	   *nodePtr;
    Object	   *objectPtr;
    Identity   *identityPtr;
    Macro	   *macroPtr;
    Type	   *typePtr;
    Class	   *classPtr;
    Attribute  *attributePtr;
    Event	   *eventPtr;
    Index	   index;
    Option	   *optionPtr;
    Refinement	   *refinementPtr;
    SmiStatus	   status;
    SmiAccess	   access;
    NamedNumber    *namedNumberPtr;
    Range	   *rangePtr;
    SmiValue	   *valuePtr;
    List	   *listPtr;
    Revision	   *revisionPtr;
}



/*
 * Tokens and their attributes.
 */
%token DOT
%token DOT_DOT
%token COLON_COLON

%token <text>ucIdentifier
%token <text>lcIdentifier

%token <text>floatValue
%token <text>textSegment
%token <text>decimalNumber
%token <text>hexadecimalNumber
%token <text>OID

%token <rc>moduleKeyword
%token <rc>importKeyword
%token <rc>revisionKeyword
%token <rc>identityKeyword
%token <rc>oidKeyword
%token <rc>dateKeyword
%token <rc>organizationKeyword
%token <rc>contactKeyword
%token <rc>descriptionKeyword
%token <rc>referenceKeyword
%token <rc>extensionKeyword
%token <rc>typedefKeyword
%token <rc>typeKeyword
%token <rc>formatKeyword
%token <rc>unitsKeyword
%token <rc>statusKeyword
%token <rc>accessKeyword
%token <rc>defaultKeyword
%token <rc>impliedKeyword
%token <rc>indexKeyword
%token <rc>augmentsKeyword
%token <rc>reordersKeyword
%token <rc>sparseKeyword
%token <rc>expandsKeyword
%token <rc>createKeyword
%token <rc>membersKeyword
%token <rc>objectsKeyword
%token <rc>mandatoryKeyword
%token <rc>optionalKeyword
%token <rc>refineKeyword
%token <rc>abnfKeyword
%token <rc>OctetStringKeyword
%token <rc>ObjectIdentifierKeyword
%token <rc>Integer32Keyword
%token <rc>Unsigned32Keyword
%token <rc>Integer64Keyword
%token <rc>Unsigned64Keyword
%token <rc>Float32Keyword
%token <rc>Float64Keyword
%token <rc>Float128Keyword
%token <rc>BitsKeyword
%token <rc>EnumerationKeyword
%token <rc>currentKeyword
%token <rc>deprecatedKeyword
%token <rc>obsoleteKeyword
%token <rc>readonlyKeyword
%token <rc>readwriteKeyword

%token <rc>parentKeyword
%token <rc>classKeyword
%token <rc>extendsKeyword
%token <rc>attributeKeyword
%token <rc>uniqueKeyword
%token <rc>eventKeyword
%token <rc>PointerKeyword
%token <rc>eventonlyKeyword
%token <rc>neginfKeyword
%token <rc>posinfKeyword
%token <rc>snanKeyword
%token <rc>qnanKeyword


/*
 * Types of non-terminal symbols.
 */

%type <rc>smingFile
%type <rc>moduleStatement_optsep_0n
%type <rc>moduleStatement_optsep_1n
%type <rc>moduleStatement_optsep
%type <modulePtr>moduleStatement
%type <rc>extensionStatement_stmtsep_0n
%type <rc>extensionStatement_stmtsep_1n
%type <rc>extensionStatement_stmtsep
%type <macroPtr>extensionStatement
%type <rc>identityStatement_stmtsep_0n
%type <rc>identityStatement_stmtsep_1n
%type <rc>identityStatement_stmtsep
%type <identityPtr>identityStatement
%type <identityPtr>parentStatement_stmtsep_01
%type <identityPtr>parentStatement
%type <rc>typedefStatement_stmtsep_0n
%type <rc>typedefStatement_stmtsep_1n
%type <rc>typedefStatement_stmtsep
%type <typePtr>typedefStatement
%type <rc>attributeStatement_stmtsep_0n
%type <rc>attributeStatement_stmtsep_1n
%type <rc>attributeStatement_stmtsep
%type <attributePtr>attributeStatement
%type <rc>eventStatement_stmtsep_0n
%type <rc>eventStatement_stmtsep_1n
%type <rc>eventStatement_stmtsep
%type <eventPtr>eventStatement
%type <rc>classStatement_stmtsep_0n
%type <rc>classStatement_stmtsep_1n
%type <rc>classStatement_stmtsep
%type <classPtr>classStatement
%type <classPtr>extendsStatement_stmtsep_01
%type <classPtr>extendsStatement
%type <rc>importStatement_stmtsep_0n
%type <rc>importStatement_stmtsep_1n
%type <rc>importStatement_stmtsep
%type <listPtr>importStatement
%type <rc>revisionStatement_stmtsep_0n
%type <rc>revisionStatement_stmtsep_1n
%type <rc>revisionStatement_stmtsep
%type <revisionPtr>revisionStatement
%type <typePtr>typedefTypeStatement
%type <attributePtr>attributeTypeStatement
%type <date>dateStatement
%type <text>organizationStatement
%type <text>contactStatement
%type <text>formatStatement_stmtsep_01
%type <text>formatStatement
%type <text>unitsStatement_stmtsep_01
%type <text>unitsStatement
%type <status>statusStatement_stmtsep_01
%type <status>statusStatement
%type <valuePtr>defaultStatement_stmtsep_01
%type <valuePtr>defaultStatement
%type <access>accessStatement_stmtsep_01
%type <access>accessStatement
%type <access>access
%type <text>descriptionStatement_stmtsep_01
%type <text>descriptionStatement
%type <text>referenceStatement_stmtsep_01
%type <text>referenceStatement
%type <text>abnfStatement_stmtsep_01
%type <text>abnfStatement
%type <typePtr>refinedBaseType_refinedType
%type <typePtr>refinedBaseType
%type <typePtr>refinedType
%type <attributePtr>attribute_refinedBaseType_refinedType
%type <attributePtr>attribute_refinedBaseType
%type <attributePtr>attribute_refinedType
%type <listPtr>optsep_anySpec_01
%type <listPtr>anySpec
%type <listPtr>optsep_numberSpec_01
%type <listPtr>numberSpec
%type <listPtr>furtherNumberElement_0n
%type <listPtr>furtherNumberElement_1n
%type <rangePtr>furtherNumberElement
%type <rangePtr>numberElement
%type <valuePtr>numberUpperLimit_01
%type <valuePtr>numberUpperLimit
%type <listPtr>optsep_floatSpec_01
%type <listPtr>floatSpec
%type <listPtr>furtherFloatElement_0n
%type <listPtr>furtherFloatElement_1n
%type <rangePtr>furtherFloatElement
%type <rangePtr>floatElement
%type <text>floatUpperLimit_01
%type <text>floatUpperLimit
%type <text>specialFloatValue
%type <listPtr>optsep_pointerRestr_01
%type <text>pointerRestr
%type <listPtr>bitsOrEnumerationSpec
%type <listPtr>bitsOrEnumerationList
%type <listPtr>furtherBitsOrEnumerationItem_0n
%type <listPtr>furtherBitsOrEnumerationItem_1n
%type <namedNumberPtr>furtherBitsOrEnumerationItem
%type <namedNumberPtr>bitsOrEnumerationItem
%type <listPtr>identifierList
%type <listPtr>furtherIdentifier_0n
%type <listPtr>furtherIdentifier_1n
%type <text>furtherIdentifier
%type <listPtr>uniqueStatement_stmtsep_01
%type <listPtr>uniqueStatement
%type <listPtr>uniqueSpec
%type <listPtr>bitsValue
%type <listPtr>bitsList
%type <listPtr>furtherLcIdentifier_0n
%type <listPtr>furtherLcIdentifier_1n
%type <text>furtherLcIdentifier
%type <text>identifier
%type <text>qucIdentifier
%type <text>qlcIdentifier
%type <text>text
%type <text>optsep_textSegment_0n
%type <text>optsep_textSegment_1n
%type <text>optsep_textSegment
%type <date>date
%type <text>format
%type <text>units
%type <valuePtr>anyValue
%type <status>status
%type <valuePtr>number
%type <valuePtr>negativeNumber
%type <valuePtr>signedNumber
%type <rc>optsep_comma_01
%type <rc>sep
%type <rc>optsep
%type <rc>stmtsep
%type <text>qOID

%%

/*
 * Yacc rules.
 *
 */


/*
 * One mibFile may contain multiple MIB modules.
 * It's also possible that there's no module in a file.
 */
smingFile:		optsep moduleStatement_optsep_0n
			{
			    /*
			     * Return the number of successfully
			     * parsed modules.
			     */
			    $$ = $2;
			}
	;

moduleStatement_optsep_0n:	/* empty */
			{
			    $$ = 0;
			}
	|		moduleStatement_optsep_1n
			{
			    $$ = $1;
			}
	;

moduleStatement_optsep_1n:	moduleStatement_optsep
			{
			    $$ = $1;
			}
	|		moduleStatement_optsep_1n moduleStatement_optsep
			{
			    /*
			     * Sum up the number of successfully parsed
			     * modules or return -1, if at least one
			     * module failed.
			     */
			    if (($1 >= 0) && ($2 >= 0)) {
				$$ = $1 + $2;
			    } else {
				$$ = -1;
			    }
			}
        ;

moduleStatement_optsep:	moduleStatement optsep
			{
			    /*
			     * If we got a (Module *) return rc == 1,
			     * otherwise parsing failed (rc == -1).
			     */
			    if ($1 != NULL) {
				$$ = 1;
			    } else {
				$$ = -1;
			    }
			}
        ;

moduleStatement:	moduleKeyword sep ucIdentifier
			{
			    thisParserPtr->modulePtr = findModuleByName($3);
			    if (!thisParserPtr->modulePtr) {
				thisParserPtr->modulePtr =
				    addModule($3,
					      smiStrdup(thisParserPtr->path),
					      0,
					      thisParserPtr);
			    } else {
			        smiPrintError(thisParserPtr,
					      ERR_MODULE_ALREADY_LOADED,
					      $3);
				free($3);
				/*
				 * this aborts parsing the whole file,
				 * not only the current module.
				 */
				YYABORT;
			    }
			    thisModulePtr->export.language = SMI_LANGUAGE_SMING;
			    thisParserPtr->modulePtr->numImportedIdentifiers
				                                           = 0;
			    thisParserPtr->modulePtr->numStatements = 0;
			    thisParserPtr->modulePtr->numModuleIdentities = 0;
			    thisParserPtr->firstIndexlabelPtr = NULL;
			    thisParserPtr->identityObjectName = NULL;
			}
			optsep '{' stmtsep
			importStatement_stmtsep_0n
			organizationStatement stmtsep
			{
			    if ($9) {
				setModuleOrganization(thisParserPtr->modulePtr,
						      $9);
			    }
			}
			contactStatement stmtsep
			{
			    if ($12) {
				setModuleContactInfo(thisParserPtr->modulePtr,
						     $12);
			    }
			}
			descriptionStatement stmtsep
			{
			    if ($15) {
				setModuleDescription(thisParserPtr->modulePtr,
						     $15, thisParserPtr);
			    }
			}
			referenceStatement_stmtsep_01
			{
			    if ($18) {
				setModuleReference(thisParserPtr->modulePtr,
						   $18, thisParserPtr);
			    }
			}
			revisionStatement_stmtsep_0n
			extensionStatement_stmtsep_0n
			identityStatement_stmtsep_0n
			typedefStatement_stmtsep_0n
			classStatement_stmtsep_0n			
			'}' optsep ';'
			{
			    checkTypes(thisParserPtr, thisModulePtr);
			    
			    $$ = thisModulePtr;
			}
	;

extensionStatement_stmtsep_0n: /* empty */
			{
			    $$ = 0;
			}
        |		extensionStatement_stmtsep_1n
			{
			    /*
			     * Return the number of successfully
			     * parsed extension statements.
			     */
			    $$ = $1;
			}
	;

extensionStatement_stmtsep_1n: extensionStatement_stmtsep
			{
			    $$ = $1;
			}
        |		extensionStatement_stmtsep_1n
			extensionStatement_stmtsep
			{
			    /*
			     * Sum up the number of successfully parsed
			     * extensions or return -1, if at least one
			     * failed.
			     */
			    if (($1 >= 0) && ($2 >= 0)) {
				$$ = $1 + $2;
			    } else {
				$$ = -1;
			    }
			}
        ;

extensionStatement_stmtsep: extensionStatement stmtsep
			{
			    /*
			     * If we got a (Type *) return rc == 1,
			     * otherwise parsing failed (rc == -1).
			     */
			    if ($1) {
				$$ = 1;
			    } else {
				$$ = -1;
			    }
			}
        ;

extensionStatement:	extensionKeyword sep lcIdentifier
			{
			    macroIdentifier = $3;
			    macroPtr = addMacro(macroIdentifier,
						0,
						thisParserPtr);
			    setMacroDecl(macroPtr, SMI_DECL_EXTENSION);
			}
			optsep '{' stmtsep
			statusStatement_stmtsep_01
			{
			    if (macroPtr && $8) {
				setMacroStatus(macroPtr, $8);
			    }
			}
			descriptionStatement_stmtsep_01
			{
			    if (macroPtr && $10) {
				setMacroDescription(macroPtr, $10,
						    thisParserPtr);
			    }
			}
			referenceStatement_stmtsep_01
			{
			    if (macroPtr && $12) {
				setMacroReference(macroPtr, $12,
						  thisParserPtr);
			    }
			}
			abnfStatement_stmtsep_01
			{
			   if (macroPtr && $14) {
				setMacroAbnf(macroPtr, $14,
						  thisParserPtr);
			   }
			}
			'}' optsep ';'
			{
			    $$ = 0;
			    macroPtr = NULL;
			}
	;

typedefStatement_stmtsep_0n: /* empty */
			{
			    $$ = 0;
			}
        |		typedefStatement_stmtsep_1n
			{
			    /*
			     * Return the number of successfully
			     * parsed typedef statements.
			     */
			    $$ = $1;
			}
	;

typedefStatement_stmtsep_1n: typedefStatement_stmtsep
			{
			    $$ = $1;
			}
        |		typedefStatement_stmtsep_1n
			typedefStatement_stmtsep
			{
			    /*
			     * Sum up the number of successfully parsed
			     * typedefs or return -1, if at least one
			     * failed.
			     */
			    if (($1 >= 0) && ($2 >= 0)) {
				$$ = $1 + $2;
			    } else {
				$$ = -1;
			    }
			}
        ;

typedefStatement_stmtsep: typedefStatement stmtsep
			{
			    /*
			     * If we got a (Type *) return rc == 1,
			     * otherwise parsing failed (rc == -1).
			     */
			    if ($1) {
				$$ = 1;
			    } else {
				$$ = -1;
			    }
			}
        ;

typedefStatement:	typedefKeyword sep ucIdentifier
			{
			    typeIdentifier = $3;
			    /* 
			     *check for duplicate names in the module 
			     */
			    if(typePtr = 
			    	findType(typeIdentifier, thisParserPtr,thisModulePtr)) 
					if( typePtr->modulePtr == thisParserPtr->modulePtr)
					       smiPrintError(thisParserPtr,
										ERR_DUPLICATE_TYPE_NAME,
					      				typeIdentifier);
			}
			optsep '{' stmtsep
			typedefTypeStatement stmtsep
			{
			    if ($8) {
				if ($8->export.name) {
				    /*
				     * If the exact type has been found
				     * and no new Type structure has been
				     * created, we have to create a
				     * duplicate now.
				     */
				    typePtr = duplicateType($8, 0,
							    thisParserPtr);
				} else {
				    typePtr = $8;
				}
				typePtr = setTypeName(typePtr, typeIdentifier);
				setTypeDecl(typePtr, SMI_DECL_TYPEDEF);
			    }
			    defaultBasetype = typePtr->export.basetype;
			}
			defaultStatement_stmtsep_01
			{
			    if (typePtr && $11) {
			    	if(typePtr->export.basetype == 
			    					SMI_BASETYPE_ENUM) 
			    					//check ENUM value for correctness
    				{
    					if($11->len)
    	 				if(namedNumberPtr = findNamedNumberByName(typePtr,
    	 												 $11->value.ptr)){
    	 					smiFree($11);
    	 					$11 = &namedNumberPtr->export.value;
    	 				}
    	 				else{ smiPrintError(thisParserPtr,
					      		ERR_ENUM_NAME_NOT_DEFINED,
					      		$11->value.ptr);
					    }
    	 			}
    	 			//NOTE that the bits default value is set in the anyval
    	 			//rule
				setTypeValue(typePtr, $11);
			    }
			}
			formatStatement_stmtsep_01
			{
			    if (typePtr && $13
				&& smiCheckFormat(thisParserPtr,
						  typePtr->export.basetype,
						  $13, 0)) {
				setTypeFormat(typePtr, $13);
			    }
			}
			unitsStatement_stmtsep_01
			{
			    if (typePtr && $15) {
				setTypeUnits(typePtr, $15);
			    }
			}
			statusStatement_stmtsep_01
			{
			    if (typePtr && $17) {
				setTypeStatus(typePtr, $17);
			    }
			}
			descriptionStatement_stmtsep_01
			{
			    if (typePtr && $19) {
				setTypeDescription(typePtr, $19,
						   thisParserPtr);
			    }
			}
			referenceStatement_stmtsep_01
			{
			    if (typePtr && $21) {
				setTypeReference(typePtr, $21, thisParserPtr);
			    }
			}
			'}' optsep ';'
			{
			    smiCheckNamedNumberSubtyping(thisParserPtr , typePtr);
			    smiCheckNamedNumbersOrder(thisParserPtr , typePtr);
			    smiCheckNamedNumberRedefinition(thisParserPtr , typePtr);
			    $$ = 0;
			    typePtr = NULL;
			    free(typeIdentifier);
			    defaultBasetype = SMI_BASETYPE_UNKNOWN;
			}
	;
	
classStatement_stmtsep_0n: /* empty */
			{
			    $$ = 0;
			}
        |		classStatement_stmtsep_1n
			{
			    /*
			     * Return the number of successfully
			     * parsed typedef statements.
			     */
			    $$ = $1;
			}
	;

classStatement_stmtsep_1n: classStatement_stmtsep
			{
			    $$ = $1;
			}
        |		classStatement_stmtsep_1n
			classStatement_stmtsep
			{
			    /*
			     * Sum up the number of successfully parsed
			     * classes or return -1, if at least one
			     * failed.
			     */
			    if (($1 >= 0) && ($2 >= 0)) {
				$$ = $1 + $2;
			    } else {
				$$ = -1;
			    }
			}
        ;

classStatement_stmtsep: classStatement stmtsep
			{
			    /*
			     * If we got a (Type *) return rc == 1,
			     * otherwise parsing failed (rc == -1).
			     */
			    if ($1) {
				$$ = 1;
			    } else {
				$$ = -1;
			    }
			}
        ;

classStatement: classKeyword sep ucIdentifier
			{
			    classIdentifier = $3;
			    if(findClassByModuleAndName(thisModulePtr, classIdentifier))
			    {
			    	smiPrintError(thisParserPtr,
										ERR_DUPLICATE_CLASS_NAME,
					      				attributeIdentifier);
			    }
			    else{
			    classPtr = addClass(classIdentifier,
						thisParserPtr);
			    setClassDecl(classPtr, SMI_DECL_CLASS);
			    }
			}
			optsep '{' stmtsep
			extendsStatement_stmtsep_01
			{
				if(classPtr && $8)
					classPtr->parentPtr = $8;
			}
			attributeStatement_stmtsep_0n
			uniqueStatement_stmtsep_01
			{
				List *tmpList;
				Attribute *tmpAttribute;
				if(classPtr && $11)
				{
					
					//Check for "magic" value #@# that defines
					//scalar class. See NOTE after Class definitino in data.h
					if(!strcmp((char*)($11->ptr),"#@#"))
					{	
						classPtr->uniqueList = (List*)malloc(sizeof(List));
						classPtr->uniqueList->ptr = classPtr;
						classPtr->uniqueList->nextPtr = NULL;
						smiFree($11);
					}
					else
					{
						tmpList = $11;
						//convert  all attribute names to atributes
						for(tmpList; tmpList; tmpList=tmpList->nextPtr)
						{
							if(tmpAttribute = 
							(Attribute*)smiGetAttribute(&(classPtr->export),(char*)(tmpList->ptr)))
							{
								smiFree(tmpList->ptr);
								tmpList->ptr = tmpAttribute;
							}
							else
							{
								smiFree(tmpList->ptr);
								tmpList->ptr = NULL;
								smiPrintError(thisParserPtr,
										ERR_ATTRIBUTE_NOT_FOUND,
					      				attributeIdentifier);
							}
						}
						
						classPtr->uniqueList = $11;
					}
				}
			}
			eventStatement_stmtsep_0n
			statusStatement_stmtsep_01
			{
			    if (classPtr && $14) {
				setClassStatus(classPtr, $14);
			    }
			}
			descriptionStatement_stmtsep_01
			{
			    if (classPtr && $16) {
				setClassDescription(classPtr, $16,
						    thisParserPtr);
			    }
			}
			referenceStatement_stmtsep_01
			{
			    if (classPtr && $18) {
				setClassReference(classPtr, $18,
						  thisParserPtr);
			    }
			}
			'}' optsep ';'
			{
			    $$ = 0;
			    classPtr = NULL;
			}
	;
		
attributeStatement_stmtsep_0n: /* empty */
			{
			    $$ = 0;
			}
        |		attributeStatement_stmtsep_1n
			{
			    /*
			     * Return the number of successfully
			     * parsed typedef statements.
			     */
			    $$ = $1;
			}
	;

attributeStatement_stmtsep_1n: attributeStatement_stmtsep
			{
			    $$ = $1;
			}
        |		attributeStatement_stmtsep_1n
			attributeStatement_stmtsep
			{
			    /*
			     * Sum up the number of successfully parsed
			     * attributes or return -1, if at least one
			     * failed.
			     */
			    if (($1 >= 0) && ($2 >= 0)) {
				$$ = $1 + $2;
			    } else {
				$$ = -1;
			    }
			}
        ;

attributeStatement_stmtsep: attributeStatement stmtsep
			{
			    /*
			     * If we got a (Type *) return rc == 1,
			     * otherwise parsing failed (rc == -1).
			     */
			    if ($1) {
				$$ = 1;
			    } else {
				$$ = -1;
			    }
			}
        ;

attributeStatement: attributeKeyword sep lcIdentifier
					{
						attributeIdentifier = $3;
						attributePtr = (Attribute*)smiGetAttribute(&(classPtr->export),
												attributeIdentifier);
						if(attributePtr)
						{
							smiPrintError(thisParserPtr,
										ERR_DUPLICATE_ATTRIBUTE_NAME,
					      				attributeIdentifier);
					      	attributePtr = NULL;
						}					
					}
					optsep '{' stmtsep
					attributeTypeStatement
					{
			    		if ($8) {
				    		attributePtr = $8;
							setAttributeName(attributePtr, 
											attributeIdentifier);
							setAttributeDecl(attributePtr, 
												SMI_DECL_ATTRIBUTE);
							defaultBasetype = attributePtr->export.basetype;
			    		}
					}
					accessStatement_stmtsep_01
					{
						if($10 && attributePtr){
						/* check whether there's access for class reference, 
						   which is not allowed */
							if(attributePtr->parentClassPtr)
								smiPrintError(thisParserPtr,
										ERR_ATTRIBUTE_CLASS_ACCESS,
					      							attributeIdentifier);
					      	else {
					      		setAttributeAccess(attributePtr,$10);
					      	}
						}
						else if(attributePtr && attributePtr->parentTypePtr)
						{
							smiPrintError(thisParserPtr,
										ERR_ATTRIBUTE_MISSING_ACCESS,
					      							attributeIdentifier);
						}
					}
					defaultStatement_stmtsep_01
					{
						if($12 && attributePtr){
							if(attributePtr->parentClassPtr)
								smiPrintError(thisParserPtr,
										ERR_ATTRIBUTE_CLASS_DEFAULT,
					      							attributeIdentifier);
					      	else{
					      		attributePtr->export.value = *$12;
					      	}
						}
						else if(attributePtr)
						{
							attributePtr->export.value.basetype = 
													SMI_BASETYPE_UNKNOWN;
						}
					}
					formatStatement_stmtsep_01
					{
			   			if($14 && attributePtr){
							if(attributePtr->parentClassPtr)
								smiPrintError(thisParserPtr,
										ERR_ATTRIBUTE_CLASS_FORMAT,
					      							attributeIdentifier);
					      	else{
					      		if (smiCheckFormat(thisParserPtr,
						  				attributePtr->export.basetype,$14, 0)) 
						  		{
									attributePtr->export.format = $14;
					      		}
					      	}
						}
			    	}
					unitsStatement_stmtsep_01
					{
			    		if($16 && attributePtr){
							if(attributePtr->parentClassPtr)
								smiPrintError(thisParserPtr,
										ERR_ATTRIBUTE_CLASS_UNITS,
					      							attributeIdentifier);
					      	else{
									attributePtr->export.units = $16;
					      	}
						}
					}
					statusStatement_stmtsep_01
					{
			   			if (attributePtr && $18) 
							attributePtr->export.status = $18;
					}
					descriptionStatement_stmtsep_01
					{
						if (attributePtr && $20) 
							attributePtr->export.description = $20;
					}
					referenceStatement_stmtsep_01
					{
						if (attributePtr && $22) 
							attributePtr->export.reference = $22;
			    	}
					'}' optsep ';'
					{
						$$ = attributePtr;
						attributePtr = NULL;
						defaultBasetype = SMI_BASETYPE_UNKNOWN;
					}
		;

eventStatement_stmtsep_0n: /* empty */
			{
			    $$ = 0;
			}
        |		eventStatement_stmtsep_1n
			{
			    /*
			     * Return the number of successfully
			     * parsed event statements.
			     */
			    $$ = $1;
			}
	;

eventStatement_stmtsep_1n: eventStatement_stmtsep
			{
			    $$ = $1;
			}
        |		eventStatement_stmtsep_1n
			eventStatement_stmtsep
			{
			    /*
			     * Sum up the number of successfully parsed
			     * events or return -1, if at least one
			     * failed.
			     */
			    if (($1 >= 0) && ($2 >= 0)) {
				$$ = $1 + $2;
			    } else {
				$$ = -1;
			    }
			}
        ;

eventStatement_stmtsep: eventStatement stmtsep
			{
			    /*
			     * If we got a (Type *) return rc == 1,
			     * otherwise parsing failed (rc == -1).
			     */
			    if ($1) {
				$$ = 1;
			    } else {
				$$ = -1;
			    }
			}
        ;

eventStatement: eventKeyword sep lcIdentifier
			{
				//TODO check for repeated names
				eventPtr=addEvent($3,classPtr,thisParserPtr);
			}
			optsep '{' stmtsep
			statusStatement_stmtsep_01
			{
				if($8 && eventPtr)
				eventPtr->export.status = $8;
			}
			descriptionStatement_stmtsep_01
			{
				if($10 && eventPtr)
				eventPtr->export.description = $10;
			}
			referenceStatement_stmtsep_01
			{
				if($12 && eventPtr)
				eventPtr->export.reference = $12;
			}
			'}' optsep ';'
			{
				$$ = eventPtr;
				eventPtr = NULL;
			}
		;

importStatement_stmtsep_0n: /* empty */
			{
			    $$ = 0;
			}
	|		importStatement_stmtsep_1n
			{
			    $$ = $1;
			}
        ;

importStatement_stmtsep_1n: importStatement_stmtsep
			{
			    $$ = $1;
			}
        |		importStatement_stmtsep_1n importStatement_stmtsep
			{
			    /*
			     * Sum up the number of successfully parsed
			     * imports or return -1, if at least one
			     * module failed.
			     */
			    if (($1 >= 0) && ($2 >= 0)) {
				$$ = $1 + $2;
			    } else {
				$$ = -1;
			    }
			}
        ;

importStatement_stmtsep: importStatement stmtsep
			{
			    /*
			     * If we got an (Object *) return rc == 1,
			     * otherwise parsing failed (rc == -1).
			     */
			    if ($1) {
				$$ = 1;
			    } else {
				$$ = -1;
			    }
			}
        ;

importStatement:	importKeyword sep ucIdentifier
			{
			    importModulename = smiStrdup($3);
			}
			optsep '(' optsep
			identifierList
			{
			    List *listPtr, *nextPtr;
			    
			    for (listPtr = $8; listPtr; listPtr = nextPtr) {
				addImport(listPtr->ptr, thisParserPtr);
				thisParserPtr->modulePtr->
				                      numImportedIdentifiers++;
				nextPtr = listPtr->nextPtr;
				free(listPtr);
			    }
			}
			optsep ')' optsep ';'
			{
			    Module *modulePtr;
			    char *s = importModulename;

			    modulePtr = findModuleByName(s);
			    if (!modulePtr) {
				modulePtr = loadModule(s, thisParserPtr);
			    }
			    checkImports(modulePtr, thisParserPtr);
			    free(s);
			    $$ = NULL;
			}
        ;

revisionStatement_stmtsep_0n: /* empty */
			{
			    $$ = 0;
			}
        |		revisionStatement_stmtsep_1n
			{
			    $$ = $1;
			}
        ;

revisionStatement_stmtsep_1n: revisionStatement_stmtsep
			{
			    $$ = $1;
			}
        |		revisionStatement_stmtsep_1n revisionStatement_stmtsep
			{
			    /*
			     * Sum up the number of successfully parsed
			     * revisions or return -1, if at least one
			     * module failed.
			     */
			    if (($1 >= 0) && ($2 >= 0)) {
				$$ = $1 + $2;
			    } else {
				$$ = -1;
			    }
			}
        ;

revisionStatement_stmtsep: revisionStatement stmtsep
			   {
			       /*
				* If we got a (Revision *) return rc == 1,
				* otherwise parsing failed (rc == -1).
				*/
			       if ($1) {
				$$ = 1;
			       } else {
				   $$ = -1;
			       }
			   }
        ;

revisionStatement:	revisionKeyword optsep '{' stmtsep
			dateStatement stmtsep
			descriptionStatement stmtsep
			'}' optsep ';'
			{
			    $$ = addRevision($5, $7, thisParserPtr);
			}
        ;

identityStatement_stmtsep_0n: /* empty */
			{
			    $$ = 0;
			}
        |		identityStatement_stmtsep_1n
			{
			    /*
			     * Return the number of successfully
			     * parsed identity statements.
			     */
			    $$ = $1;
			}
	;

identityStatement_stmtsep_1n: identityStatement_stmtsep
			{
			    $$ = $1;
			}
        |		identityStatement_stmtsep_1n
			identityStatement_stmtsep
			{
			    /*
			     * Sum up the number of successfully parsed
			     * identities or return -1, if at least one
			     * failed.
			     */
			    if (($1 >= 0) && ($2 >= 0)) {
				$$ = $1 + $2;
			    } else {
				$$ = -1;
			    }
			}
        ;

identityStatement_stmtsep: identityStatement stmtsep
			{
			    /*
			     * If we got a (Type *) return rc == 1,
			     * otherwise parsing failed (rc == -1).
			     */
			    if ($1) {
				$$ = 1;
			    } else {
				$$ = -1;
			    }
			}
        ;

identityStatement:	identityKeyword sep lcIdentifier
			{
				identityIdentifier = $3;
				identityPtr = addIdentity(identityIdentifier,
						thisParserPtr);
			    setIdentityDecl(identityPtr, SMI_DECL_IDENTITY);
			}
			optsep '{' stmtsep
			parentStatement_stmtsep_01
			{
				if(identityPtr && $8) {
					setIdentityParent(identityPtr,$8);
				}
			}
			statusStatement_stmtsep_01
			{
			    if (identityPtr && $10) {
				setIdentityStatus(identityPtr, $10);
			    }
			}
			descriptionStatement_stmtsep_01
			{
				if (identityPtr && $12) {
				setIdentityDescription(identityPtr, $12,
						    			thisParserPtr);
			    }
			}
			referenceStatement_stmtsep_01
			{
				setIdentityReference(identityPtr, $14, 
									 thisParserPtr)
			}
			'}' optsep ';'
			{
			    $$ = 0;
			    identityPtr = NULL;
			}
	;

typedefTypeStatement:	typeKeyword sep refinedBaseType_refinedType optsep ';'
			{
			    $$ = $3;
			}
	;

attributeTypeStatement:	typeKeyword sep attribute_refinedBaseType_refinedType optsep ';'
			{
			    $$ = $3;
			}
	;

dateStatement:		dateKeyword sep date optsep ';'
			{
			    $$ = $3;
			}
        ;

organizationStatement:	organizationKeyword sep text optsep ';'
			{
			    $$ = $3;
			}
        ;

contactStatement:	contactKeyword sep text optsep ';'
			{
			    $$ = $3;
			}
        ;

formatStatement_stmtsep_01: /* empty */
			{
			    $$ = NULL;
			}
	|		formatStatement stmtsep
			{
			    $$ = $1;
			}
	;

formatStatement:	formatKeyword sep format optsep ';'
			{
			    $$ = $3;
			}
        ;

unitsStatement_stmtsep_01: /* empty */
			{
			    $$ = NULL;
			}
        |		unitsStatement stmtsep
			{
			    $$ = $1;
			}
	;

unitsStatement:		unitsKeyword sep units optsep ';'
			{
			    $$ = $3;
			}
        ;

statusStatement_stmtsep_01: /* empty */
			{
			    $$ = SMI_STATUS_CURRENT;
			}
        |               statusStatement stmtsep
			{
			    $$ = $1;
			}
	;

statusStatement:	statusKeyword sep status optsep ';'
			{
			    $$ = $3;
			}
        ;

uniqueStatement_stmtsep_01:
        	{
			    $$ = NULL;
			}
        |		uniqueStatement stmtsep
			{
			    $$ = $1;
			}
		;
		
uniqueStatement:	uniqueKeyword sep '(' uniqueSpec ')' optsep ';'
			{
			    $$ = $4;
			}
        ;

uniqueSpec:	optsep_comma_01
			{
				$$ = smiMalloc(sizeof(List));
			    $$->ptr = "#@#"; //used to indicate that unique
			    				 //statement is present and empty
			    				 //i.e. the class is scalar
			    $$->nextPtr = NULL;
			}
		|		lcIdentifier furtherLcIdentifier_0n optsep_comma_01
			{
			    $$ = smiMalloc(sizeof(List));
			    $$->ptr = $1;
			    $$->nextPtr = $2;
			}
		;


		
		
extendsStatement_stmtsep_01: /* empty */
			{
			    $$ = NULL;
			}
        |               extendsStatement stmtsep
			{
			    $$ = $1;
			}
		;

extendsStatement:	extendsKeyword sep ucIdentifier optsep ';'
			{
			    $$ = findClass($3, thisParserPtr,thisModulePtr);
			    if(!$$)smiPrintError(thisParserPtr,
					      ERR_UNKNOWN_CLASS,
					      $3);
			    
			}
        ;

defaultStatement_stmtsep_01: /* empty */
			{
			    $$ = NULL;
			}
        |		defaultStatement stmtsep
			{
			    $$ = $1;
			}
	;

defaultStatement:	defaultKeyword sep anyValue optsep ';'
			{
			    $$ = $3;
			}
        ;
        
accessStatement_stmtsep_01: /* empty */
			{
			    $$ = SMI_ACCESS_UNKNOWN;
			}
        |		accessStatement stmtsep
			{
			    $$ = $1;
			}
	;

accessStatement:	accessKeyword sep access optsep ';'
			{
			    $$ = $3;
			}
        ;
        
access:		readonlyKeyword
			{
				$$ = SMI_ACCESS_READ_ONLY;
			}
		|		readwriteKeyword
			{
				$$ = SMI_ACCESS_READ_WRITE;
			}
		|		eventonlyKeyword
			{
				$$ = SMI_ACCESS_EVENT_ONLY;
			}
		;
        
parentStatement_stmtsep_01: /* empty */
			{
			    $$ = NULL;
			}
        |		parentStatement stmtsep
			{
			    $$ = $1;
			}
	;

parentStatement:	parentKeyword sep lcIdentifier optsep ';'
			{
			    $$ = findIdentity($3, thisParserPtr, thisModulePtr);
			    if(!$$)smiPrintError(thisParserPtr,
					      ERR_IDENTITY_PARENT_NOT_FOUND,
					      $3);
			}
        ;

descriptionStatement_stmtsep_01: /* empty */
			{
			    $$ = NULL;
			}
        |               descriptionStatement stmtsep
			{
			    $$ = $1;
			}
	;

descriptionStatement:	descriptionKeyword sep text optsep ';'
			{
			    $$ = $3;
			}
        ;

referenceStatement_stmtsep_01: /* empty */
			{
			    $$ = NULL;
			}
        |		referenceStatement stmtsep
			{
			    $$ = $1;
			}
	;

referenceStatement:	referenceKeyword sep text optsep ';'
			{
			    $$ = $3;
			}
        ;

abnfStatement_stmtsep_01: /* empty */
			{
			    $$ = NULL;
			}
        |		abnfStatement stmtsep
			{
			    $$ = $1;
			}
	;

abnfStatement:		abnfKeyword sep text optsep ';'
			{
			    $$ = $3;
			}
        ;



refinedBaseType_refinedType: refinedBaseType
			{
			    $$ = $1;
			}
        |		refinedType
			{
			    $$ = $1;
			}
	;
	
attribute_refinedBaseType_refinedType: attribute_refinedBaseType
			{
			    $$ = $1;
			}
        |		attribute_refinedType
			{
			    $$ = $1;
			}
	;
	

refinedBaseType:	OctetStringKeyword optsep_numberSpec_01
			{
			    List *p;
			    
			    if (!$2) {
				$$ = smiHandle->typeOctetStringPtr;
			    } else {
				$$ = duplicateType(smiHandle->typeOctetStringPtr, 0,
						   thisParserPtr);
				setTypeParent($$, smiHandle->typeOctetStringPtr);
				setTypeList($$, $2);
				for (p = $2; p; p = p->nextPtr)
				    ((Range *)p->ptr)->typePtr = $$;
			    }
			}
        |		ObjectIdentifierKeyword
			{
			    $$ = smiHandle->typeObjectIdentifierPtr;
			}
	|		Integer32Keyword optsep_numberSpec_01
			{
			    List *p;
			    
			    if (!$2) {
				$$ = smiHandle->typeInteger32Ptr;
			    } else {
				$$ = duplicateType(smiHandle->typeInteger32Ptr, 0,
						   thisParserPtr);
				setTypeParent($$, smiHandle->typeInteger32Ptr);
				setTypeList($$, $2);
				for (p = $2; p; p = p->nextPtr)
				    ((Range *)p->ptr)->typePtr = $$;
			    }
			}
	|		Unsigned32Keyword optsep_numberSpec_01
			{
			    List *p;
			    
			    if (!$2) {
				$$ = smiHandle->typeUnsigned32Ptr;
			    } else {
				$$ = duplicateType(smiHandle->typeUnsigned32Ptr, 0,
						   thisParserPtr);
				setTypeParent($$, smiHandle->typeUnsigned32Ptr);
				setTypeList($$, $2);
				for (p = $2; p; p = p->nextPtr)
				    ((Range *)p->ptr)->typePtr = $$;
			    }
			}
	|		Integer64Keyword optsep_numberSpec_01
			{
			    List *p;
			    
			    if (!$2) {
				$$ = smiHandle->typeInteger64Ptr;
			    } else {
				$$ = duplicateType(smiHandle->typeInteger64Ptr, 0,
						   thisParserPtr);
				setTypeParent($$, smiHandle->typeInteger64Ptr);
				setTypeList($$, $2);
				for (p = $2; p; p = p->nextPtr)
				    ((Range *)p->ptr)->typePtr = $$;
			    }
			}
	|		Unsigned64Keyword optsep_numberSpec_01
			{
			    List *p;
			    
			    if (!$2) {
				$$ = smiHandle->typeUnsigned64Ptr;
			    } else {
				$$ = duplicateType(smiHandle->typeUnsigned64Ptr, 0,
						   thisParserPtr);
				setTypeParent($$, smiHandle->typeUnsigned64Ptr);
				setTypeList($$, $2);
				for (p = $2; p; p = p->nextPtr)
				    ((Range *)p->ptr)->typePtr = $$;
			    }
			}
	|		Float32Keyword optsep_floatSpec_01
			{
			    List *p;
			    
			    if (!$2) {
				$$ = smiHandle->typeFloat32Ptr;
			    } else {
				$$ = duplicateType(smiHandle->typeFloat32Ptr, 0,
						   thisParserPtr);
				setTypeParent($$, smiHandle->typeFloat32Ptr);
				setTypeList($$, $2);
				for (p = $2; p; p = p->nextPtr)
				    ((Range *)p->ptr)->typePtr = $$;
			    }
			}
	|		Float64Keyword optsep_floatSpec_01
			{
			    List *p;
			    
			    if (!$2) {
				$$ = smiHandle->typeFloat64Ptr;
			    } else {
				$$ = duplicateType(smiHandle->typeFloat64Ptr, 0,
						   thisParserPtr);
				setTypeParent($$, smiHandle->typeFloat64Ptr);
				setTypeList($$, $2);
				for (p = $2; p; p = p->nextPtr)
				    ((Range *)p->ptr)->typePtr = $$;
			    }
			}
	|		Float128Keyword optsep_floatSpec_01
			{
			    List *p;
			    
			    if (!$2) {
				$$ = smiHandle->typeFloat128Ptr;
			    } else {
				$$ = duplicateType(smiHandle->typeFloat128Ptr, 0,
						   thisParserPtr);
				setTypeParent($$, smiHandle->typeFloat128Ptr);
				setTypeList($$, $2);
				for (p = $2; p; p = p->nextPtr)
				    ((Range *)p->ptr)->typePtr = $$;
			    }
			}
	|		PointerKeyword optsep_pointerRestr_01
			{
			    if (!$2) {
				$$ = smiHandle->typePointerPtr;
			    } else {
				$$ = duplicateType(smiHandle->typePointerPtr, 0,
						   thisParserPtr);
				setTypeParent($$, smiHandle->typePointerPtr);
				setTypeList($$, $2);				
				}
			}
	|		EnumerationKeyword bitsOrEnumerationSpec
			{
			    List *p;
			    
			    if (!$2) {
				$$ = smiHandle->typeEnumPtr;
			    } else {
				$$ = duplicateType(smiHandle->typeEnumPtr, 0,
						   thisParserPtr);
				setTypeParent($$, smiHandle->typeEnumPtr);
				setTypeList($$, $2);
				for (p = $2; p; p = p->nextPtr)
				    ((NamedNumber *)p->ptr)->typePtr = $$;
			    }
			}
	|		BitsKeyword 
			{
				bitsFlag = 1; //Since Enum elements can be 
							  //negative we must make sure
							  //that bits is not negative,
							  //so we raise bitsFlag and
							  //give error if there is
							  //negative value 
			}
			bitsOrEnumerationSpec
			{
			    List *p;
			    
			    if (!$3) {
				$$ = smiHandle->typeBitsPtr;
			    } else {
				$$ = duplicateType(smiHandle->typeBitsPtr, 0,
						   thisParserPtr);
				setTypeParent($$, smiHandle->typeBitsPtr);
				setTypeList($$, $3);
				for (p = $3; p; p = p->nextPtr)
				    ((NamedNumber *)p->ptr)->typePtr = $$;
			    }
			    
			    bitsFlag = 0;//reset flag
			}
	;
	
	
attribute_refinedBaseType:	OctetStringKeyword optsep_numberSpec_01
			{
			    List *p;
			    $$ = duplicateTypeToAttribute(smiHandle->typeOctetStringPtr,
												classPtr, thisParserPtr);
				setAttributeParentType($$, smiHandle->typeOctetStringPtr);
			    if ($2) {
					setAttributeList($$, $2);
					for (p = $2; p; p = p->nextPtr)
				    	((Range *)p->ptr)->typePtr = (Type*)$$;
			    }
			}
        |		ObjectIdentifierKeyword
			{
			    $$ = duplicateTypeToAttribute(
			    	smiHandle->typeObjectIdentifierPtr, classPtr, thisParserPtr);
			   	setAttributeParentType($$, smiHandle->typeObjectIdentifierPtr);
			}
	|		Integer32Keyword optsep_numberSpec_01
			{
			    List *p;
			    
				$$ = duplicateTypeToAttribute(smiHandle->typeInteger32Ptr,
													classPtr, thisParserPtr);
				setAttributeParentType($$, smiHandle->typeInteger32Ptr);
			    if ($2) {
					setAttributeList($$, $2);
					for (p = $2; p; p = p->nextPtr)
				    	((Range *)p->ptr)->typePtr = (Type*)$$;
			    }
			}
	|		Unsigned32Keyword optsep_numberSpec_01
			{
			    List *p;
			    
				$$ = duplicateTypeToAttribute(smiHandle->typeUnsigned32Ptr,
													classPtr, thisParserPtr);
				setAttributeParentType($$, smiHandle->typeUnsigned32Ptr);
			    if ($2) {
					setAttributeList($$, $2);
					for (p = $2; p; p = p->nextPtr)
				    	((Range *)p->ptr)->typePtr = (Type*)$$;
			    }
			}
	|		Integer64Keyword optsep_numberSpec_01
			{
			   List *p;
			    
				$$ = duplicateTypeToAttribute(smiHandle->typeInteger64Ptr,
													classPtr, thisParserPtr);
				setAttributeParentType($$, smiHandle->typeInteger64Ptr);
			    if ($2) {
					setAttributeList($$, $2);
					for (p = $2; p; p = p->nextPtr)
				    	((Range *)p->ptr)->typePtr = (Type*)$$;
			    }
			}
	|		Unsigned64Keyword optsep_numberSpec_01
			{
			   List *p;
			    
				$$ = duplicateTypeToAttribute(smiHandle->typeUnsigned64Ptr,
													classPtr, thisParserPtr);
				setAttributeParentType($$, smiHandle->typeUnsigned64Ptr);
			    if ($2) {
					setAttributeList($$, $2);
					for (p = $2; p; p = p->nextPtr)
				    	((Range *)p->ptr)->typePtr = (Type*)$$;
			    }
			}
	|		Float32Keyword optsep_floatSpec_01
			{
			    List *p;
			    
				$$ = duplicateTypeToAttribute(smiHandle->typeFloat32Ptr,
													classPtr, thisParserPtr);
				setAttributeParentType($$, smiHandle->typeFloat32Ptr);
			    if ($2) {
					setAttributeList($$, $2);
					for (p = $2; p; p = p->nextPtr)
				    	((Range *)p->ptr)->typePtr = (Type*)$$;
			    }
			}
	|		Float64Keyword optsep_floatSpec_01
			{
			   List *p;
			    
				$$ = duplicateTypeToAttribute(smiHandle->typeFloat64Ptr,
													classPtr, thisParserPtr);
				setAttributeParentType($$, smiHandle->typeFloat64Ptr);
			    if ($2) {
					setAttributeList($$, $2);
					for (p = $2; p; p = p->nextPtr)
				    	((Range *)p->ptr)->typePtr = (Type*)$$;
			    }
			}
	|		Float128Keyword optsep_floatSpec_01
			{
			    List *p;
			    
			    $$ = duplicateTypeToAttribute(smiHandle->typeFloat128Ptr,
							  classPtr, thisParserPtr);
			    setAttributeParentType($$, smiHandle->typeFloat128Ptr);
			    if ($2) {
				setAttributeList($$, $2);
				for (p = $2; p; p = p->nextPtr)
				    ((Range *)p->ptr)->typePtr = (Type*)$$;
			    }
			}
	|		PointerKeyword optsep_pointerRestr_01
			{
			    $$ = duplicateTypeToAttribute(smiHandle->typePointerPtr,
							  classPtr, thisParserPtr);
			    setAttributeParentType($$, smiHandle->typePointerPtr);
			    if ($2) {
				setAttributeList($$, $2);
			    }
			}
	|		EnumerationKeyword bitsOrEnumerationSpec
			{
			    List *p;
			    
			    $$ = duplicateTypeToAttribute(smiHandle->typeEnumPtr,
													classPtr, thisParserPtr);
				setAttributeParentType($$, smiHandle->typeEnumPtr);
			    if ($2) {
					setAttributeList($$, $2);
					for (p = $2; p; p = p->nextPtr)
				    	((NamedNumber *)p->ptr)->typePtr = (Type*)$$;
			    }
			}
	|		BitsKeyword 
			{
				bitsFlag = 1; //Since Enum elements can be 
							  //negative we must make sure
							  //that bits is not negative,
							  //so we raise bitsFlag and
							  //give error if there is
							  //negative value 
			}
			bitsOrEnumerationSpec
			{
			    List *p;
			    
			    $$ = duplicateTypeToAttribute(smiHandle->typeBitsPtr,
							  classPtr, thisParserPtr);
			    setAttributeParentType($$, smiHandle->typeBitsPtr);
			    if ($1) {
				setAttributeList($$, $1);
				for (p = $1; p; p = p->nextPtr)
				    ((NamedNumber *)(p->ptr))->typePtr = (Type*)$$;
			    }
			    
			    bitsFlag = 0; /* reset flag */
			}
	;

refinedType:		qucIdentifier optsep_anySpec_01
			{
			    typePtr = findType($1, thisParserPtr,
					       thisModulePtr);
			    if (typePtr && $2) {
				typePtr = duplicateType(typePtr, 0,
							thisParserPtr);
				setTypeList(typePtr, $2);
			    }

			    $$ = typePtr;
			}
	;
	
attribute_refinedType:		qucIdentifier optsep_anySpec_01
			{
			    Class *tmp;
			    typePtr = findType($1, thisParserPtr,
					       thisModulePtr);
			    if (typePtr && $2) {
				attributePtr = duplicateTypeToAttribute(typePtr,
											classPtr, thisParserPtr);
				setAttributeList(attributePtr, $2);
				setAttributeParentType(attributePtr, typePtr);
			    } else if( typePtr ){
			    	attributePtr = duplicateTypeToAttribute(typePtr,
											classPtr, thisParserPtr);
					setAttributeParentType(attributePtr, typePtr);
				} else if($2){
					smiPrintError(thisParserPtr, ERR_UNKNOWN_TYPE, $1);
					attributePtr = NULL;
				} else if (tmp = findClass($1,
										 thisParserPtr,thisModulePtr)){
					attributePtr = addAttribute($1, classPtr, thisParserPtr);
					setAttributeParentClass(attributePtr, tmp);
				} else {
					attributePtr = NULL;
					smiPrintError(thisParserPtr, 
											ERR_UNKNOWN_TYPE_OR_CLASS, $1);
				}
				

			    $$ = attributePtr;
			}
	;

optsep_anySpec_01:	/* empty */
			{
			    $$ = NULL;
			}
	|		optsep anySpec
			{
			    $$ = $2;
			}
	;

anySpec:		numberSpec
			{
			    $$ = $1;
			}
	|		floatSpec
			{
			    $$ = $1;
			}
	;

optsep_numberSpec_01:	/* empty */
			{
			    $$ = NULL;
			}
	|		optsep numberSpec
			{
			    $$ = $2;
			}
	;

numberSpec:		'(' optsep numberElement furtherNumberElement_0n
			optsep ')'
			{
			    $$ = smiMalloc(sizeof(List));
			    $$->ptr = $3;
			    $$->nextPtr = $4;
			}
        ;

furtherNumberElement_0n:	/* empty */
			{
			    $$ = NULL;
			}
        |		furtherNumberElement_1n
			{
			    $$ = $1;
			}
        ;

furtherNumberElement_1n:	furtherNumberElement
			{
			    $$ = smiMalloc(sizeof(List));
			    $$->ptr = $1;
			    $$->nextPtr = NULL;
			}
        |		furtherNumberElement_1n furtherNumberElement
			{
			    List *p, *pp;
			    
			    p = smiMalloc(sizeof(List));
			    p->ptr = $2;
			    p->nextPtr = NULL;
			    for (pp = $1; pp->nextPtr; pp = pp->nextPtr);
			    pp->nextPtr = p;
			    $$ = $1;
			}
        ;

furtherNumberElement:	optsep '|' optsep numberElement
			{
			    $$ = $4;
			}
	;

numberElement:		signedNumber numberUpperLimit_01
			{
			    $$ = smiMalloc(sizeof(Range));
			    $$->export.minValue = *$1;
			    if ($2) {
				$$->export.maxValue = *$2;
				smiFree($2);
			    } else {
				$$->export.maxValue = *$1;
			    }
			    smiFree($1);
			}
	;

numberUpperLimit_01:	/* empty */
			{
			    $$ = NULL;
			}
        |		numberUpperLimit
			{
			    $$ = $1;
			}
	;

numberUpperLimit:	optsep DOT_DOT optsep signedNumber
			{
			    $$ = $4;
			}
        ;

optsep_floatSpec_01:	/* empty */
			{
			    $$ = NULL;
			}
        |		optsep floatSpec
			{
			    $$ = $2;
			}
	;

floatSpec:		'(' optsep floatElement furtherFloatElement_0n
			optsep ')'
			{
			    $$ = smiMalloc(sizeof(List));
			    $$->ptr = $3;
			    $$->nextPtr = $4;
			}
        ;

furtherFloatElement_0n:	/* empty */
			{
			    $$ = NULL;
			}
        |		furtherFloatElement_1n
			{
			    $$ = $1;
			}
        ;

furtherFloatElement_1n:	furtherFloatElement
			{
			    $$ = smiMalloc(sizeof(List));
			    $$->ptr = $1;
			    $$->nextPtr = NULL;
			}
        |		furtherFloatElement_1n furtherFloatElement
			{
			    List *p, *pp;
			    
			    p = smiMalloc(sizeof(List));
			    p->ptr = $2;
			    p->nextPtr = NULL;
			    for (pp = $1; pp->nextPtr; pp = pp->nextPtr);
			    pp->nextPtr = p;
			    $$ = $1;
			}
        ;

furtherFloatElement:	optsep '|' optsep floatElement
			{
			    $$ = $4;
			}
	;

floatElement:		floatValue floatUpperLimit_01
			{
			    $$ = smiMalloc(sizeof(Range));
			    $$->export.minValue.basetype = SMI_BASETYPE_FLOAT64;
			    $$->export.minValue.value.float64 = strtod($1, NULL);
			    if ($2) {
				$$->export.maxValue.basetype =
				                          SMI_BASETYPE_FLOAT64;
				$$->export.maxValue.value.float64 =
				                              strtod($2, NULL);
			    } else {
				$$->export.maxValue = $$->export.minValue;
			    }
			}
	;

floatUpperLimit_01:	/* empty */
			{
			    $$ = NULL;
			}
        |		floatUpperLimit
			{
			    $$ = $1;
			}
	;

floatUpperLimit:	optsep DOT_DOT optsep floatValue
			{
			    $$ = $4;
			}
        ;

specialFloatValue:
			neginfKeyword
			{
				$$="-inf";
			}
		|	posinfKeyword
			{
				$$="+inf";
			}
		|	qnanKeyword
			{
				$$="nan";
			}
		|	snanKeyword
			{
				$$="nan";
			}
	;		
        
optsep_pointerRestr_01:	/* empty */
			{
			    $$ = NULL;
			}
        |		optsep pointerRestr
			{
			    NamedNumber *nn =(NamedNumber*)smiMalloc(sizeof(NamedNumber));
			    $$ = smiMalloc(sizeof(List));
			    $$->ptr = nn;
			    nn->export.name = $2;
			}
	;

pointerRestr:		'(' optsep qlcIdentifier optsep ')'
			{
			    $$ = $3;
			}
    ;

bitsOrEnumerationSpec:	'(' optsep bitsOrEnumerationList optsep ')'
			{
			    $$ = $3;
			}
        ;

bitsOrEnumerationList:	bitsOrEnumerationItem furtherBitsOrEnumerationItem_0n
			optsep_comma_01
			{
			    $$ = smiMalloc(sizeof(List));
			    $$->ptr = $1;
			    $$->nextPtr = $2;
			}
        ;

furtherBitsOrEnumerationItem_0n: /* empty */
			{
			    $$ = NULL;
			}
        |		furtherBitsOrEnumerationItem_1n
			{
			    $$ = $1;
			}
        ;

furtherBitsOrEnumerationItem_1n: furtherBitsOrEnumerationItem
			{
			    $$ = smiMalloc(sizeof(List));
			    $$->ptr = $1;
			    $$->nextPtr = NULL;
			}
        |		furtherBitsOrEnumerationItem_1n
			furtherBitsOrEnumerationItem
			{
			    List *p, *pp;
			    
			    p = smiMalloc(sizeof(List));
			    p->ptr = $2;
			    p->nextPtr = NULL;
			    for (pp = $1; pp->nextPtr; pp = pp->nextPtr);
			    pp->nextPtr = p;
			    $$ = $1;
			}
        ;

	
furtherBitsOrEnumerationItem: optsep ',' optsep bitsOrEnumerationItem
			{
			    $$ = $4;
			}
        ;

bitsOrEnumerationItem:	lcIdentifier optsep '(' optsep signedNumber optsep ')'
			{
			    $$ = smiMalloc(sizeof(NamedNumber));
			    $$->export.name = $1;
			    $$->export.value = *$5;
			}
        ;

identifierList:		identifier furtherIdentifier_0n optsep_comma_01
			{
			    $$ = smiMalloc(sizeof(List));
			    $$->ptr = $1;
			    $$->nextPtr = $2;
			}
        ;

furtherIdentifier_0n:	/* empty */
			{
			    $$ = NULL;
			}
        |		furtherIdentifier_1n
			{
			    $$ = $1;
			}
        ;

furtherIdentifier_1n:	furtherIdentifier
			{
			    $$ = smiMalloc(sizeof(List));
			    $$->ptr = $1;
			    $$->nextPtr = NULL;
			}
        |		furtherIdentifier_1n furtherIdentifier
			{
			    List *p, *pp;
			    
			    p = smiMalloc(sizeof(List));
			    p->ptr = $2;
			    p->nextPtr = NULL;
			    for (pp = $1; pp->nextPtr; pp = pp->nextPtr);
			    pp->nextPtr = p;
			    $$ = $1;
			}
        ;

furtherIdentifier:	optsep ',' optsep identifier
			{
			    $$ = $4;
			}
        ;

bitsValue:		'(' optsep bitsList optsep ')'
			{
			    $$ = $3;
			}
        ;

bitsList:		optsep_comma_01
			{
			    $$ = NULL;
			}
        |		lcIdentifier furtherLcIdentifier_0n optsep_comma_01
			{
			    $$ = smiMalloc(sizeof(List));
			    $$->ptr = $1;
			    $$->nextPtr = $2;
			}
	;

//NOTE used also for unique statement
furtherLcIdentifier_0n:	/* empty */
			{
			    $$ = NULL;
			}
        |		furtherLcIdentifier_1n
			{
			    $$ = $1;
			}
	;

furtherLcIdentifier_1n:	furtherLcIdentifier
			{
			    $$ = smiMalloc(sizeof(List));
			    $$->ptr = $1;
			    $$->nextPtr = NULL;
			}
        |		furtherLcIdentifier_1n furtherLcIdentifier
			{
			    List *p, *pp;
			    
			    p = smiMalloc(sizeof(List));
			    p->ptr = $2;
			    p->nextPtr = NULL;
			    for (pp = $1; pp->nextPtr; pp = pp->nextPtr);
			    pp->nextPtr = p;
			    $$ = $1;
			}
	;

furtherLcIdentifier:	optsep ',' optsep lcIdentifier
			{
			    $$ = $4;
			}
        ;

identifier:		ucIdentifier
			{
			    $$ = $1;
			}
        |		lcIdentifier
			{
			    $$ = $1;
			}
	;

qucIdentifier:		ucIdentifier COLON_COLON ucIdentifier
			{
			    char *s;

			    s = smiMalloc(strlen($1) +
					    strlen($3) + 3);
			    sprintf(s, "%s::%s", $1, $3);
			    $$ = s;
			    free($1);
			    free($3);
			}
        |		ucIdentifier
			{
			    $$ = $1;
			}
	;

qlcIdentifier:		ucIdentifier COLON_COLON lcIdentifier
			{
			    char *s;

			    s = smiMalloc(strlen($1) +
					  strlen($3) + 3);
			    sprintf(s, "%s::%s", $1, $3);
			    $$ = s;
			}
        |		lcIdentifier
			{
			    $$ = $1;
			}
	;

text:			textSegment optsep_textSegment_0n
			{
			    int len;
			    if ($2) {
				$$ = smiMalloc(strlen($1) + strlen($2) + 1);
				strcpy($$, $1);
				strcat($$, $2);
				free($1);
				free($2);
			    } else {
				$$ = smiStrdup($1);
			    }
			    len = strlen($$);
			    while (len > 0 && $$[len-1] == '\n') {
				$$[--len] = 0;
			    }
			}
        ;

optsep_textSegment_0n:	/* empty */
			{
			    $$ = NULL;
			}
        |		optsep_textSegment_1n
			{
			    $$ = $1;
			}
	;

optsep_textSegment_1n:	optsep_textSegment
			{
			    $$ = $1;
			}
        |		optsep_textSegment_1n optsep_textSegment
			{
			    $$ = smiMalloc(strlen($1) + strlen($2) + 1);
			    strcpy($$, $1);
			    strcat($$, $2);
			    free($1);
			    free($2);
			}
	;

optsep_textSegment:	optsep textSegment
			{
			    $$ = smiStrdup($2);
			}
        ;

date:			textSegment
			{
			    $$ = checkDate(thisParserPtr, $1);
			}
        ;

format:			textSegment
			{
			    $$ = smiStrdup($1);
			}
        ;

units:			textSegment
			{
			    $$ = smiStrdup($1);
			}
        ;

/*
 * The type of `anyValue' must be determined from the
 * context. `anyValue' appears only in default value clauses. Hence,
 * we set a global variable defaultBasetype in the object type
 * declaring clause to remember the expected type.  Here, we use this
 * variable to build an SmiValue with the appropriate base type.
 */
anyValue:		bitsValue
			{
			    if (defaultBasetype == SMI_BASETYPE_BITS) {
				$$ = smiMalloc(sizeof(SmiValue));
				$$->basetype = SMI_BASETYPE_BITS;
				$$->value.ptr = (void*)($1);
				//set the bits value in the value.integer32
				if(typePtr){
					createBitsValue($$,typePtr);
				}
				else if(attributePtr){
				createBitsValue($$,
					(Type*)smiGetAttributeParentType(&(attributePtr->export)));
				}
			    } else {
				smiPrintError(thisParserPtr,
					      ERR_UNEXPECTED_VALUETYPE);
				$$ = NULL;
			    }
			}
	|		decimalNumber
			{
			    /* Note: might also be an OID or signed */
			    switch (defaultBasetype) {
			    case SMI_BASETYPE_UNSIGNED32:
				$$ = smiMalloc(sizeof(SmiValue));
				$$->basetype = SMI_BASETYPE_UNSIGNED32;
				$$->value.unsigned32 = strtoul($1, NULL, 10);
				break;
			    case SMI_BASETYPE_UNSIGNED64:
				$$ = smiMalloc(sizeof(SmiValue));
				$$->basetype = SMI_BASETYPE_UNSIGNED64;
				$$->value.unsigned64 = strtoull($1, NULL, 10);
				break;
			    case SMI_BASETYPE_INTEGER32:
				$$ = smiMalloc(sizeof(SmiValue));
				$$->basetype = SMI_BASETYPE_INTEGER32;
				$$->value.integer32 = strtol($1, NULL, 10);
				break;
			    case SMI_BASETYPE_INTEGER64:
				$$ = smiMalloc(sizeof(SmiValue));
				$$->basetype = SMI_BASETYPE_INTEGER64;
				$$->value.integer64 = strtoll($1, NULL, 10);
				break;
			    case SMI_BASETYPE_OBJECTIDENTIFIER:
				$$ = smiMalloc(sizeof(SmiValue));
				$$->basetype = SMI_BASETYPE_OBJECTIDENTIFIER;
				$$->len = strlen($1);
				$$->value.ptr = smiStrdup($1);
				/* TODO */
				break;
			    default:
				smiPrintError(thisParserPtr,
					      ERR_UNEXPECTED_VALUETYPE);
				$$ = NULL;
				break;
			    }
			}
	|		'-' decimalNumber
			{
			    switch (defaultBasetype) {
			    case SMI_BASETYPE_INTEGER32:
				$$ = smiMalloc(sizeof(SmiValue));
				$$->basetype = SMI_BASETYPE_INTEGER32;
				$$->value.integer32 = - strtol($2, NULL, 10);
				break;
			    case SMI_BASETYPE_INTEGER64:
				$$ = smiMalloc(sizeof(SmiValue));
				$$->basetype = SMI_BASETYPE_INTEGER64;
				$$->value.integer64 = - strtoll($2, NULL, 10);
				break;
			    default:
				smiPrintError(thisParserPtr,
					      ERR_UNEXPECTED_VALUETYPE);
				$$ = NULL;
				break;
			    }
			}
	|		hexadecimalNumber
			{
			    /* Note: might also be an octet string */
			    switch (defaultBasetype) {
			    case SMI_BASETYPE_UNSIGNED32:
				$$ = smiMalloc(sizeof(SmiValue));
				$$->basetype = SMI_BASETYPE_UNSIGNED32;
				$$->value.unsigned32 = strtoul($1, NULL, 16);
				break;
			    case SMI_BASETYPE_UNSIGNED64:
				$$ = smiMalloc(sizeof(SmiValue));
				$$->basetype = SMI_BASETYPE_UNSIGNED64;
				$$->value.unsigned64 = strtoull($1, NULL, 16);
				break;
			    case SMI_BASETYPE_INTEGER32:
				$$ = smiMalloc(sizeof(SmiValue));
				$$->basetype = SMI_BASETYPE_INTEGER32;
				$$->value.integer32 = strtol($1, NULL, 16);
				break;
			    case SMI_BASETYPE_INTEGER64:
				$$ = smiMalloc(sizeof(SmiValue));
				$$->basetype = SMI_BASETYPE_INTEGER64;
				$$->value.integer64 = strtoll($1, NULL, 16);
				break;
			    case SMI_BASETYPE_OCTETSTRING:
				$$ = smiMalloc(sizeof(SmiValue));
				$$->basetype = SMI_BASETYPE_OCTETSTRING;
				$$->value.ptr = hexToStr($1,strlen($1));
				$$->len = strlen($$->value.ptr);
				break;
			    default:
				smiPrintError(thisParserPtr,
					      ERR_UNEXPECTED_VALUETYPE);
				$$ = NULL;
				break;
			    }
			}
	|		floatValue
			{
			    /* Note: might also be an OID */
			    switch (defaultBasetype) {
			    case SMI_BASETYPE_FLOAT32:
			    	$$ = smiMalloc(sizeof(SmiValue));
				$$->basetype = SMI_BASETYPE_FLOAT32;
				$$->value.float32 = strtof($1,NULL);
				if(errno == ERANGE){
					smiPrintError(thisParserPtr, 
					ERR_FLOAT_OVERFLOW, $1);
					errno = 0;
				}
				break;
			    case SMI_BASETYPE_FLOAT64:
			    	$$ = smiMalloc(sizeof(SmiValue));
				$$->basetype = SMI_BASETYPE_FLOAT64;
				$$->value.float64 = strtod($1,NULL);
				if(errno == ERANGE){
					smiPrintError(thisParserPtr, 
					ERR_FLOAT_OVERFLOW, $1);
					errno = 0;
				}
				break;
			    case SMI_BASETYPE_FLOAT128:
			    	$$ = smiMalloc(sizeof(SmiValue));
				$$->basetype = SMI_BASETYPE_FLOAT128;
				$$->value.float128 = strtold($1,NULL);
				if(errno == ERANGE){
					smiPrintError(thisParserPtr, 
					ERR_FLOAT_OVERFLOW, $1);
					errno = 0;
				}
				break;		
			    case SMI_BASETYPE_OBJECTIDENTIFIER:
				$$ = smiMalloc(sizeof(SmiValue));
				$$->basetype = SMI_BASETYPE_OBJECTIDENTIFIER;
				$$->value.ptr = smiMalloc(strlen($1)+1);
				strcpy($$->value.ptr,$1);
				$$->len = strlen($$->value.ptr);
				break;
			    default:
				smiPrintError(thisParserPtr,
					      ERR_UNEXPECTED_VALUETYPE);
				$$ = NULL;
				break;
			    }
			}
	|		'-' floatValue
			{
		
			    switch (defaultBasetype) {
			    case SMI_BASETYPE_FLOAT32:
			    	$$ = smiMalloc(sizeof(SmiValue));
				$$->basetype = SMI_BASETYPE_FLOAT32;
				$$->value.float32 = - strtof($2,NULL);
				if(errno == ERANGE){
					smiPrintError(thisParserPtr, 
					ERR_FLOAT_OVERFLOW, $2);
					errno = 0;
				}
				break;
			    case SMI_BASETYPE_FLOAT64:
			    	$$ = smiMalloc(sizeof(SmiValue));
				$$->basetype = SMI_BASETYPE_FLOAT64;
				$$->value.float64 = - strtof($2,NULL);
				if(errno == ERANGE){
					smiPrintError(thisParserPtr, 
					ERR_FLOAT_OVERFLOW, $2);
					errno = 0;
				}
				break;
			    case SMI_BASETYPE_FLOAT128:
			    	$$ = smiMalloc(sizeof(SmiValue));
				$$->basetype = SMI_BASETYPE_FLOAT128;
				$$->value.float128 = - strtof($2,NULL);
				if(errno == ERANGE){
					smiPrintError(thisParserPtr, 
					ERR_FLOAT_OVERFLOW, $2);
					errno = 0;
				}
				break;		
			    default:
				smiPrintError(thisParserPtr,
					      ERR_UNEXPECTED_VALUETYPE);
				$$ = NULL;
				break;
			    }
			}
	|		specialFloatValue
			{
				  /* Note: might also be an OID */
			    switch (defaultBasetype) {
			    case SMI_BASETYPE_FLOAT32:
			    	$$ = smiMalloc(sizeof(SmiValue));
				$$->basetype = SMI_BASETYPE_FLOAT32;
				$$->value.float32 = strtof($1,NULL);
				if(errno == ERANGE){
					smiPrintError(thisParserPtr, 
					ERR_FLOAT_OVERFLOW, $1);
					errno = 0;
				}
				break;
			    case SMI_BASETYPE_FLOAT64:
			    	$$ = smiMalloc(sizeof(SmiValue));
				$$->basetype = SMI_BASETYPE_FLOAT64;
				$$->value.float64 = strtod($1,NULL);
				if(errno == ERANGE){
					smiPrintError(thisParserPtr, 
					ERR_FLOAT_OVERFLOW, $1);
					errno = 0;
				}
				break;
			    case SMI_BASETYPE_FLOAT128:
			    	$$ = smiMalloc(sizeof(SmiValue));
				$$->basetype = SMI_BASETYPE_FLOAT128;
				$$->value.float128 = strtold($1,NULL);
				if(errno == ERANGE){
					smiPrintError(thisParserPtr, 
					ERR_FLOAT_OVERFLOW, $1);
					errno = 0;
				}
				break;
				default:
				smiPrintError(thisParserPtr,
					      ERR_UNEXPECTED_VALUETYPE);
				$$ = NULL;
				break;
			    }	
			}
	|		text
			{
			    if (defaultBasetype == SMI_BASETYPE_OCTETSTRING) {
				$$ = smiMalloc(sizeof(SmiValue));
				$$->basetype = SMI_BASETYPE_OCTETSTRING;
				$$->value.ptr = $1;
				$$->len = strlen($1);
			    } else {
				smiPrintError(thisParserPtr,
					      ERR_UNEXPECTED_VALUETYPE);
				$$ = NULL;
			    }
			}
	|		qlcIdentifier
			{
			    /* Note: might be an Enumeration item or OID */
			    /* TODO: convert if it's an oid? */
			    switch (defaultBasetype) {
			    case SMI_BASETYPE_ENUM:
				$$ = smiMalloc(sizeof(SmiValue));
				$$->basetype = SMI_BASETYPE_ENUM;
				$$->value.ptr = $1;
				$$->len = strlen($1);
				/* TODO: XXX convert to int */
				break;
			    case SMI_BASETYPE_OBJECTIDENTIFIER:
				$$ = smiMalloc(sizeof(SmiValue));
				$$->basetype = SMI_BASETYPE_OBJECTIDENTIFIER;
				$$->value.ptr = $1;
				$$->len = strlen($1);
				/* TODO: XXX convert to oid if found */
				break;
				case SMI_BASETYPE_POINTER:
				$$ = smiMalloc(sizeof(SmiValue));
				$$->basetype = SMI_BASETYPE_OBJECTIDENTIFIER;
				$$->value.ptr = $1;
				$$->len = strlen($1);
				/* TODO: XXX check if valid reference found */
				break;
			    default:
				smiPrintError(thisParserPtr,
					      ERR_UNEXPECTED_VALUETYPE);
				$$ = NULL;
				break;
			    }
			}
	|		qOID
			{
			    if (defaultBasetype == SMI_BASETYPE_OBJECTIDENTIFIER){
			    $$ = smiMalloc(sizeof(SmiValue));
				$$->basetype = SMI_BASETYPE_OBJECTIDENTIFIER;
				$$->value.ptr = $1;
				$$->len = strlen($$->value.ptr);
			    }
			    else
			    	smiPrintError(thisParserPtr,
					      ERR_UNEXPECTED_VALUETYPE);
			}
	;
	
qOID:		ucIdentifier COLON_COLON OID
			{
			    char *s;

			    s = smiMalloc(strlen($1) +
					  strlen($3) + 3);
			    sprintf(s, "%s::%s", $1, $3);
			    $$ = s;
			}
		|	OID
			{
				$$ = smiMalloc(strlen($1)+1);
				strcpy($$,$1);
			}
	;

status:			currentKeyword
			{
			    $$ = SMI_STATUS_CURRENT;
			}
        |		deprecatedKeyword
			{
			    $$ = SMI_STATUS_DEPRECATED;
			}
	|		obsoleteKeyword
			{
			    $$ = SMI_STATUS_OBSOLETE;
			}
	;

number:			hexadecimalNumber
			{
			    $$ = smiMalloc(sizeof(SmiValue));
			    $$->basetype = SMI_BASETYPE_UNSIGNED64;
			    $$->value.unsigned64 = strtoull($1, NULL, 16);
			}
        |		decimalNumber
			{
			    $$ = smiMalloc(sizeof(SmiValue));
			    $$->basetype = SMI_BASETYPE_UNSIGNED64;
			    $$->value.unsigned64 = strtoull($1, NULL, 10);
			}
	;

negativeNumber:		'-' decimalNumber
			{
			    if(bitsFlag){
			    smiPrintError(thisParserPtr,
					      ERR_BITS_NUMBER_NEGATIVE);
			    $$ = NULL;
			    }
			    $$ = smiMalloc(sizeof(SmiValue));
			    $$->basetype = SMI_BASETYPE_INTEGER64;
			    $$->value.integer64 = - strtoll($2, NULL, 10);
			}
        ;

signedNumber:		number
			{
			    $$ = $1;
			}
        |		negativeNumber
			{
			    $$ = $1;
			}
	;

/* decimalNumber, hexadecimalNumber, floatValue */

/* unknown... */

optsep_comma_01:	/* empty */
			{
			    $$ = 0;
			}
/*	|		optsep ',' */
	;

sep:			/* empty, skipped by lexer */
			{
			    $$ = 0;
			}
        ;			

optsep:			/* empty, skipped by lexer */
			{
			    $$ = 0;
			}
        ;			

stmtsep:		/* empty, skipped by lexer */
			{
			    $$ = 0;
			}
        ;			

%%

#endif
			
