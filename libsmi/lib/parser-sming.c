/* A Bison parser, made by GNU Bison 1.875d.  */

/* Skeleton parser for Yacc-like parsing with Bison,
   Copyright (C) 1984, 1989, 1990, 2000, 2001, 2002, 2003, 2004 Free Software Foundation, Inc.

   This program is free software; you can redistribute it and/or modify
   it under the terms of the GNU General Public License as published by
   the Free Software Foundation; either version 2, or (at your option)
   any later version.

   This program is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY; without even the implied warranty of
   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
   GNU General Public License for more details.

   You should have received a copy of the GNU General Public License
   along with this program; if not, write to the Free Software
   Foundation, Inc., 59 Temple Place - Suite 330,
   Boston, MA 02111-1307, USA.  */

/* As a special exception, when this file is copied by Bison into a
   Bison output file, you may use that output file without restriction.
   This special exception was added by the Free Software Foundation
   in version 1.24 of Bison.  */

/* Written by Richard Stallman by simplifying the original so called
   ``semantic'' parser.  */

/* All symbols defined below should begin with yy or YY, to avoid
   infringing on user name space.  This should be done even for local
   variables, as they might otherwise be expanded by user macros.
   There are some unavoidable exceptions within include files to
   define necessary library symbols; they are noted "INFRINGES ON
   USER NAME SPACE" below.  */

/* Identify Bison output.  */
#define YYBISON 1

/* Skeleton name.  */
#define YYSKELETON_NAME "yacc.c"

/* Pure parsers.  */
#define YYPURE 1

/* Using locations.  */
#define YYLSP_NEEDED 0

/* If NAME_PREFIX is specified substitute the variables and functions
   names.  */
#define yyparse smingparse
#define yylex   sminglex
#define yyerror smingerror
#define yylval  sminglval
#define yychar  smingchar
#define yydebug smingdebug
#define yynerrs smingnerrs


/* Tokens.  */
#ifndef YYTOKENTYPE
# define YYTOKENTYPE
   /* Put the tokens into the symbol table, so that GDB and other debuggers
      know about them.  */
   enum yytokentype {
     DOT_DOT = 258,
     COLON_COLON = 259,
     ucIdentifier = 260,
     lcIdentifier = 261,
     floatValue = 262,
     textSegment = 263,
     decimalNumber = 264,
     hexadecimalNumber = 265,
     moduleKeyword = 266,
     importKeyword = 267,
     revisionKeyword = 268,
     identityKeyword = 269,
     oidKeyword = 270,
     dateKeyword = 271,
     organizationKeyword = 272,
     contactKeyword = 273,
     descriptionKeyword = 274,
     referenceKeyword = 275,
     extensionKeyword = 276,
     typedefKeyword = 277,
     typeKeyword = 278,
     writetypeKeyword = 279,
     nodeKeyword = 280,
     scalarKeyword = 281,
     tableKeyword = 282,
     columnKeyword = 283,
     rowKeyword = 284,
     notificationKeyword = 285,
     groupKeyword = 286,
     complianceKeyword = 287,
     formatKeyword = 288,
     unitsKeyword = 289,
     statusKeyword = 290,
     accessKeyword = 291,
     defaultKeyword = 292,
     impliedKeyword = 293,
     indexKeyword = 294,
     augmentsKeyword = 295,
     reordersKeyword = 296,
     sparseKeyword = 297,
     expandsKeyword = 298,
     createKeyword = 299,
     membersKeyword = 300,
     objectsKeyword = 301,
     mandatoryKeyword = 302,
     optionalKeyword = 303,
     refineKeyword = 304,
     abnfKeyword = 305,
     OctetStringKeyword = 306,
     ObjectIdentifierKeyword = 307,
     Integer32Keyword = 308,
     Unsigned32Keyword = 309,
     Integer64Keyword = 310,
     Unsigned64Keyword = 311,
     Float32Keyword = 312,
     Float64Keyword = 313,
     Float128Keyword = 314,
     BitsKeyword = 315,
     EnumerationKeyword = 316,
     currentKeyword = 317,
     deprecatedKeyword = 318,
     obsoleteKeyword = 319,
     noaccessKeyword = 320,
     notifyonlyKeyword = 321,
     readonlyKeyword = 322,
     readwriteKeyword = 323,
     readcreateKeyword = 324
   };
#endif
#define DOT_DOT 258
#define COLON_COLON 259
#define ucIdentifier 260
#define lcIdentifier 261
#define floatValue 262
#define textSegment 263
#define decimalNumber 264
#define hexadecimalNumber 265
#define moduleKeyword 266
#define importKeyword 267
#define revisionKeyword 268
#define identityKeyword 269
#define oidKeyword 270
#define dateKeyword 271
#define organizationKeyword 272
#define contactKeyword 273
#define descriptionKeyword 274
#define referenceKeyword 275
#define extensionKeyword 276
#define typedefKeyword 277
#define typeKeyword 278
#define writetypeKeyword 279
#define nodeKeyword 280
#define scalarKeyword 281
#define tableKeyword 282
#define columnKeyword 283
#define rowKeyword 284
#define notificationKeyword 285
#define groupKeyword 286
#define complianceKeyword 287
#define formatKeyword 288
#define unitsKeyword 289
#define statusKeyword 290
#define accessKeyword 291
#define defaultKeyword 292
#define impliedKeyword 293
#define indexKeyword 294
#define augmentsKeyword 295
#define reordersKeyword 296
#define sparseKeyword 297
#define expandsKeyword 298
#define createKeyword 299
#define membersKeyword 300
#define objectsKeyword 301
#define mandatoryKeyword 302
#define optionalKeyword 303
#define refineKeyword 304
#define abnfKeyword 305
#define OctetStringKeyword 306
#define ObjectIdentifierKeyword 307
#define Integer32Keyword 308
#define Unsigned32Keyword 309
#define Integer64Keyword 310
#define Unsigned64Keyword 311
#define Float32Keyword 312
#define Float64Keyword 313
#define Float128Keyword 314
#define BitsKeyword 315
#define EnumerationKeyword 316
#define currentKeyword 317
#define deprecatedKeyword 318
#define obsoleteKeyword 319
#define noaccessKeyword 320
#define notifyonlyKeyword 321
#define readonlyKeyword 322
#define readwriteKeyword 323
#define readcreateKeyword 324




/* Copy the first part of user declarations.  */
#line 14 "parser-sming.y"


#include <config.h>
    
#ifdef BACKEND_SMING

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



static char *typeIdentifier, *macroIdentifier, *nodeIdentifier,
	    *scalarIdentifier, *tableIdentifier, *rowIdentifier,
	    *columnIdentifier, *notificationIdentifier,
	    *groupIdentifier, *complianceIdentifier;
static char *importModulename = NULL;
static Object *moduleObjectPtr = NULL;
static Object *nodeObjectPtr = NULL;
static Object *scalarObjectPtr = NULL;
static Object *tableObjectPtr = NULL;
static Object *rowObjectPtr = NULL;
static Object *columnObjectPtr = NULL;
static Object *notificationObjectPtr = NULL;
static Object *groupObjectPtr = NULL;
static Object *complianceObjectPtr = NULL;
static Type *typePtr = NULL;
static Macro *macroPtr = NULL;
static SmiBasetype defaultBasetype = SMI_BASETYPE_UNKNOWN;


#define SMI_EPOCH	631152000	/* 01 Jan 1990 00:00:00 */ 
 

static Type *
findType(spec, parserPtr, modulePtr)
    char *spec;
    Parser *parserPtr;
    Module *modulePtr;
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
 

			    
static Object *
findObject(spec, parserPtr, modulePtr)
    char *spec;
    Parser *parserPtr;
    Module *modulePtr;
{
    Object *objectPtr;
    Import *importPtr;
    char *module, *object;

    object = strstr(spec, "::");
    if (!object) {
	objectPtr = findObjectByModuleAndName(modulePtr, spec);
	if (!objectPtr) {
	    importPtr = findImportByName(spec, modulePtr);
	    if (importPtr) {
	     objectPtr = findObjectByModulenameAndName(importPtr->export.module,
							  spec);
	    }
	}
    } else {
	module = strtok(spec, ":");
	object = &object[2];
	objectPtr = findObjectByModulenameAndName(module, object);
    }
    return objectPtr;
}



static void
checkObjects(Parser *parserPtr, Module *modulePtr)
{
    Object *objectPtr;
    Node *nodePtr;
    int i;

    for (objectPtr = modulePtr->firstObjectPtr;
	 objectPtr; objectPtr = objectPtr->nextPtr) {

	/*
	 * Check whether groups only contain scalars, columns and
	 * notifications.
	 */

	if (objectPtr->export.nodekind == SMI_NODEKIND_GROUP) {
	    smiCheckGroupMembers(parserPtr, objectPtr);
	}

	/*
	 * Complain about empty description clauses.
	 */

	if (! parserPtr->flags & SMI_FLAG_NODESCR
	    && objectPtr->export.nodekind != SMI_NODEKIND_UNKNOWN
	    && objectPtr->export.nodekind != SMI_NODEKIND_NODE
	    && (! objectPtr->export.description
		|| ! objectPtr->export.description[0])) {
	    smiPrintErrorAtLine(parserPtr, ERR_EMPTY_DESCRIPTION,
				objectPtr->line, objectPtr->export.name);
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
		smiCheckAugment(parserPtr, objectPtr);
		break;
	    case SMI_INDEX_REORDER:	/* TODO */
		break;
	    case SMI_INDEX_SPARSE:	/* TODO */
		break;
	    case SMI_INDEX_EXPAND:	/* TODO */
		break;
	    default:
		break;
	    }
	}
	
	/*
	 * Set the oidlen/oid values that are not yet correct.
	 */
				
	if (objectPtr->export.oidlen == 0) {
	    if (objectPtr->nodePtr->oidlen == 0) {
		for (nodePtr = objectPtr->nodePtr, i = 1;
		     nodePtr->parentPtr != smiHandle->rootNodePtr;
		     nodePtr = nodePtr->parentPtr, i++);
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
	
	/*
	 * Determine the longest common OID prefix of all nodes.
	 */
	
	if (!thisModulePtr->prefixNodePtr) {
	    thisModulePtr->prefixNodePtr = objectPtr->nodePtr;
	} else {
	    for (i = 0; i < thisModulePtr->prefixNodePtr->oidlen; i++) {
		if (thisModulePtr->prefixNodePtr->oid[i] !=
		    objectPtr->nodePtr->oid[i]) {
		    thisModulePtr->prefixNodePtr =
			findNodeByOid(i, thisModulePtr->prefixNodePtr->oid);
		    break;
		}
	    }
	}
    }

    for (objectPtr = modulePtr->firstObjectPtr;
	 objectPtr; objectPtr = objectPtr->nextPtr) {

	smiCheckDefault(parserPtr, objectPtr);
	
    	/*
	 * Check whether all objects and notifications arecontained in at
	 * least one conformance group (RFC 2580 3.3 and 4.1).
	 */

	smiCheckGroupMembership(parserPtr, objectPtr);
    }
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

			    


/* Enabling traces.  */
#ifndef YYDEBUG
# define YYDEBUG 1
#endif

/* Enabling verbose error messages.  */
#ifdef YYERROR_VERBOSE
# undef YYERROR_VERBOSE
# define YYERROR_VERBOSE 1
#else
# define YYERROR_VERBOSE 0
#endif

#if ! defined (YYSTYPE) && ! defined (YYSTYPE_IS_DECLARED)
#line 385 "parser-sming.y"
typedef union YYSTYPE {
    char           *id;				/* identifier name           */
    int            rc;				/* >=0: ok, <0: error        */
    time_t	   date;			/* a date value		     */
    char	   *text;
    Module	   *modulePtr;
    Node	   *nodePtr;
    Object	   *objectPtr;
    Macro	   *macroPtr;
    Type	   *typePtr;
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
} YYSTYPE;
/* Line 191 of yacc.c.  */
#line 598 "parser-sming.c"
# define yystype YYSTYPE /* obsolescent; will be withdrawn */
# define YYSTYPE_IS_DECLARED 1
# define YYSTYPE_IS_TRIVIAL 1
#endif



/* Copy the second part of user declarations.  */


/* Line 214 of yacc.c.  */
#line 610 "parser-sming.c"

#if ! defined (yyoverflow) || YYERROR_VERBOSE

# ifndef YYFREE
#  define YYFREE free
# endif
# ifndef YYMALLOC
#  define YYMALLOC malloc
# endif

/* The parser invokes alloca or malloc; define the necessary symbols.  */

# ifdef YYSTACK_USE_ALLOCA
#  if YYSTACK_USE_ALLOCA
#   define YYSTACK_ALLOC alloca
#  endif
# else
#  if defined (alloca) || defined (_ALLOCA_H)
#   define YYSTACK_ALLOC alloca
#  else
#   ifdef __GNUC__
#    define YYSTACK_ALLOC __builtin_alloca
#   endif
#  endif
# endif

# ifdef YYSTACK_ALLOC
   /* Pacify GCC's `empty if-body' warning. */
#  define YYSTACK_FREE(Ptr) do { /* empty */; } while (0)
# else
#  if defined (__STDC__) || defined (__cplusplus)
#   include <stdlib.h> /* INFRINGES ON USER NAME SPACE */
#   define YYSIZE_T size_t
#  endif
#  define YYSTACK_ALLOC YYMALLOC
#  define YYSTACK_FREE YYFREE
# endif
#endif /* ! defined (yyoverflow) || YYERROR_VERBOSE */


#if (! defined (yyoverflow) \
     && (! defined (__cplusplus) \
	 || (defined (YYSTYPE_IS_TRIVIAL) && YYSTYPE_IS_TRIVIAL)))

/* A type that is properly aligned for any stack member.  */
union yyalloc
{
  short int yyss;
  YYSTYPE yyvs;
  };

/* The size of the maximum gap between one aligned stack and the next.  */
# define YYSTACK_GAP_MAXIMUM (sizeof (union yyalloc) - 1)

/* The size of an array large to enough to hold all stacks, each with
   N elements.  */
# define YYSTACK_BYTES(N) \
     ((N) * (sizeof (short int) + sizeof (YYSTYPE))			\
      + YYSTACK_GAP_MAXIMUM)

/* Copy COUNT objects from FROM to TO.  The source and destination do
   not overlap.  */
# ifndef YYCOPY
#  if defined (__GNUC__) && 1 < __GNUC__
#   define YYCOPY(To, From, Count) \
      __builtin_memcpy (To, From, (Count) * sizeof (*(From)))
#  else
#   define YYCOPY(To, From, Count)		\
      do					\
	{					\
	  register YYSIZE_T yyi;		\
	  for (yyi = 0; yyi < (Count); yyi++)	\
	    (To)[yyi] = (From)[yyi];		\
	}					\
      while (0)
#  endif
# endif

/* Relocate STACK from its old location to the new one.  The
   local variables YYSIZE and YYSTACKSIZE give the old and new number of
   elements in the stack, and YYPTR gives the new location of the
   stack.  Advance YYPTR to a properly aligned location for the next
   stack.  */
# define YYSTACK_RELOCATE(Stack)					\
    do									\
      {									\
	YYSIZE_T yynewbytes;						\
	YYCOPY (&yyptr->Stack, Stack, yysize);				\
	Stack = &yyptr->Stack;						\
	yynewbytes = yystacksize * sizeof (*Stack) + YYSTACK_GAP_MAXIMUM; \
	yyptr += yynewbytes / sizeof (*yyptr);				\
      }									\
    while (0)

#endif

#if defined (__STDC__) || defined (__cplusplus)
   typedef signed char yysigned_char;
#else
   typedef short int yysigned_char;
#endif

/* YYFINAL -- State number of the termination state. */
#define YYFINAL  3
/* YYLAST -- Last index in YYTABLE.  */
#define YYLAST   1342

/* YYNTOKENS -- Number of terminals. */
#define YYNTOKENS  79
/* YYNNTS -- Number of nonterminals. */
#define YYNNTS  237
/* YYNRULES -- Number of rules. */
#define YYNRULES  334
/* YYNRULES -- Number of states. */
#define YYNSTATES  766

/* YYTRANSLATE(YYLEX) -- Bison symbol number corresponding to YYLEX.  */
#define YYUNDEFTOK  2
#define YYMAXUTOK   324

#define YYTRANSLATE(YYX) 						\
  ((unsigned int) (YYX) <= YYMAXUTOK ? yytranslate[YYX] : YYUNDEFTOK)

/* YYTRANSLATE[YYLEX] -- Bison symbol number corresponding to YYLEX.  */
static const unsigned char yytranslate[] =
{
       0,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
      73,    74,     2,     2,    76,    77,    78,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,    72,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,    70,    75,    71,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     1,     2,     3,     4,
       5,     6,     7,     8,     9,    10,    11,    12,    13,    14,
      15,    16,    17,    18,    19,    20,    21,    22,    23,    24,
      25,    26,    27,    28,    29,    30,    31,    32,    33,    34,
      35,    36,    37,    38,    39,    40,    41,    42,    43,    44,
      45,    46,    47,    48,    49,    50,    51,    52,    53,    54,
      55,    56,    57,    58,    59,    60,    61,    62,    63,    64,
      65,    66,    67,    68,    69
};

#if YYDEBUG
/* YYPRHS[YYN] -- Index of the first RHS symbol of rule number YYN in
   YYRHS.  */
static const unsigned short int yyprhs[] =
{
       0,     0,     3,     6,     7,     9,    11,    14,    17,    18,
      19,    20,    21,    22,    53,    54,    56,    58,    61,    64,
      65,    66,    67,    68,    69,    88,    89,    91,    93,    96,
      99,   100,   101,   102,   103,   104,   105,   106,   107,   133,
     134,   136,   138,   141,   144,   146,   148,   150,   151,   152,
     153,   154,   155,   175,   176,   177,   178,   179,   180,   181,
     182,   183,   184,   185,   218,   219,   220,   221,   222,   223,
     246,   247,   248,   249,   250,   251,   252,   253,   280,   282,
     285,   288,   289,   290,   291,   292,   293,   294,   295,   296,
     297,   298,   331,   332,   334,   336,   339,   342,   343,   344,
     345,   346,   347,   348,   371,   372,   374,   376,   379,   382,
     383,   384,   385,   386,   387,   388,   412,   413,   415,   417,
     420,   423,   424,   425,   426,   427,   428,   429,   430,   431,
     458,   459,   461,   463,   466,   469,   470,   471,   485,   486,
     488,   490,   493,   496,   508,   509,   512,   518,   524,   525,
     528,   534,   535,   538,   544,   546,   548,   550,   552,   554,
     565,   571,   584,   590,   603,   604,   607,   608,   611,   615,
     621,   627,   633,   639,   640,   643,   649,   650,   653,   659,
     660,   663,   669,   670,   673,   679,   680,   683,   689,   690,
     693,   699,   700,   703,   709,   710,   713,   719,   729,   730,
     733,   743,   744,   747,   757,   758,   760,   762,   765,   768,
     779,   780,   782,   784,   787,   790,   804,   806,   808,   811,
     813,   816,   819,   822,   825,   828,   831,   834,   837,   840,
     843,   844,   847,   849,   851,   852,   855,   862,   863,   865,
     867,   870,   875,   878,   879,   881,   886,   887,   890,   897,
     898,   900,   902,   905,   910,   913,   914,   916,   921,   927,
     931,   932,   934,   936,   939,   944,   952,   956,   957,   959,
     961,   964,   969,   973,   974,   976,   978,   981,   986,   992,
     994,   998,   999,  1001,  1003,  1006,  1011,  1013,  1015,  1019,
    1021,  1025,  1027,  1030,  1031,  1033,  1035,  1038,  1041,  1043,
    1045,  1047,  1049,  1051,  1054,  1056,  1058,  1060,  1062,  1065,
    1070,  1072,  1074,  1076,  1078,  1080,  1082,  1084,  1087,  1089,
    1091,  1092,  1094,  1096,  1099,  1102,  1104,  1106,  1108,  1111,
    1113,  1115,  1116,  1117,  1118
};

/* YYRHS -- A `-1'-separated list of the rules' RHS. */
static const short int yyrhs[] =
{
      80,     0,    -1,   314,    81,    -1,    -1,    82,    -1,    83,
      -1,    82,    83,    -1,    84,   314,    -1,    -1,    -1,    -1,
      -1,    -1,    11,   313,     5,    85,   314,    70,   315,   191,
     219,   315,    86,   220,   315,    87,   232,   315,    88,   233,
      89,   197,   201,    90,    99,   111,   159,   169,   179,    71,
     314,    72,    -1,    -1,    91,    -1,    92,    -1,    91,    92,
      -1,    93,   315,    -1,    -1,    -1,    -1,    -1,    -1,    21,
     313,     6,    94,   314,    70,   315,   225,    95,   231,    96,
     233,    97,   235,    98,    71,   314,    72,    -1,    -1,   100,
      -1,   101,    -1,   100,   101,    -1,   102,   315,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    22,   313,     5,
     103,   314,    70,   315,   203,   315,   104,   229,   105,   221,
     106,   223,   107,   225,   108,   231,   109,   233,   110,    71,
     314,    72,    -1,    -1,   112,    -1,   113,    -1,   112,   113,
      -1,   114,   315,    -1,   115,    -1,   121,    -1,   132,    -1,
      -1,    -1,    -1,    -1,    -1,    25,   313,     6,   116,   314,
      70,   315,   217,   315,   117,   225,   118,   231,   119,   233,
     120,    71,   314,    72,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    26,   313,     6,   122,   314,
      70,   315,   217,   315,   123,   205,   315,   124,   228,   315,
     125,   229,   126,   221,   127,   223,   128,   225,   129,   232,
     315,   130,   233,   131,    71,   314,    72,    -1,    -1,    -1,
      -1,    -1,    -1,    27,   313,     6,   133,   314,    70,   315,
     217,   315,   134,   225,   135,   232,   315,   136,   233,   137,
     138,   315,    71,   314,    72,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    29,   313,     6,   139,   314,    70,   315,
     217,   315,   140,   208,   315,   141,   215,   142,   225,   143,
     232,   315,   144,   233,   145,   146,    71,   314,    72,    -1,
     147,    -1,   146,   147,    -1,   148,   315,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    28,   313,
       6,   149,   314,    70,   315,   217,   315,   150,   205,   315,
     151,   228,   315,   152,   229,   153,   221,   154,   223,   155,
     225,   156,   232,   315,   157,   233,   158,    71,   314,    72,
      -1,    -1,   160,    -1,   161,    -1,   160,   161,    -1,   162,
     315,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    30,   313,
       6,   163,   314,    70,   315,   217,   315,   164,   238,   165,
     225,   166,   232,   315,   167,   233,   168,    71,   314,    72,
      -1,    -1,   170,    -1,   171,    -1,   170,   171,    -1,   172,
     315,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    31,   313,
       6,   173,   314,    70,   315,   217,   315,   174,   237,   315,
     175,   225,   176,   232,   315,   177,   233,   178,    71,   314,
      72,    -1,    -1,   180,    -1,   181,    -1,   180,   181,    -1,
     182,   315,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    32,   313,     6,   183,   314,    70,   315,   217,   315,
     184,   225,   185,   232,   315,   186,   233,   187,   240,   188,
     242,   189,   246,   190,    71,   314,    72,    -1,    -1,   192,
      -1,   193,    -1,   192,   193,    -1,   194,   315,    -1,    -1,
      -1,    12,   313,     5,   195,   314,    73,   314,   277,   196,
     314,    74,   314,    72,    -1,    -1,   198,    -1,   199,    -1,
     198,   199,    -1,   200,   315,    -1,    13,   314,    70,   315,
     218,   315,   232,   315,    71,   314,    72,    -1,    -1,   202,
     315,    -1,    14,   313,     6,   314,    72,    -1,    23,   313,
     250,   314,    72,    -1,    -1,   205,   315,    -1,    23,   313,
     250,   314,    72,    -1,    -1,   207,   315,    -1,    24,   313,
     250,   314,    72,    -1,   209,    -1,   210,    -1,   211,    -1,
     212,    -1,   213,    -1,    39,   214,   314,    73,   314,   281,
     314,    74,   314,    72,    -1,    40,   313,   292,   314,    72,
      -1,    41,   313,   292,   214,   314,    73,   314,   281,   314,
      74,   314,    72,    -1,    42,   313,   292,   314,    72,    -1,
      43,   313,   292,   214,   314,    73,   314,   281,   314,    74,
     314,    72,    -1,    -1,   313,    38,    -1,    -1,   216,   315,
      -1,    44,   314,    72,    -1,    15,   313,   303,   314,    72,
      -1,    16,   313,   297,   314,    72,    -1,    17,   313,   293,
     314,    72,    -1,    18,   313,   293,   314,    72,    -1,    -1,
     222,   315,    -1,    33,   313,   298,   314,    72,    -1,    -1,
     224,   315,    -1,    34,   313,   299,   314,    72,    -1,    -1,
     226,   315,    -1,    35,   313,   301,   314,    72,    -1,    -1,
     228,   315,    -1,    36,   313,   302,   314,    72,    -1,    -1,
     230,   315,    -1,    37,   313,   300,   314,    72,    -1,    -1,
     232,   315,    -1,    19,   313,   293,   314,    72,    -1,    -1,
     234,   315,    -1,    20,   313,   293,   314,    72,    -1,    -1,
     236,   315,    -1,    50,   313,   293,   314,    72,    -1,    45,
     314,    73,   314,   281,   314,    74,   314,    72,    -1,    -1,
     239,   315,    -1,    46,   314,    73,   314,   281,   314,    74,
     314,    72,    -1,    -1,   241,   315,    -1,    47,   314,    73,
     314,   281,   314,    74,   314,    72,    -1,    -1,   243,    -1,
     244,    -1,   243,   244,    -1,   245,   315,    -1,    48,   313,
     292,   314,    70,   232,   315,    71,   314,    72,    -1,    -1,
     247,    -1,   248,    -1,   247,   248,    -1,   249,   315,    -1,
      49,   313,   292,   314,    70,   204,   206,   227,   232,   315,
      71,   314,    72,    -1,   251,    -1,   252,    -1,    51,   255,
      -1,    52,    -1,    53,   255,    -1,    54,   255,    -1,    55,
     255,    -1,    56,   255,    -1,    57,   263,    -1,    58,   263,
      -1,    59,   263,    -1,    61,   271,    -1,    60,   271,    -1,
     291,   253,    -1,    -1,   314,   254,    -1,   256,    -1,   264,
      -1,    -1,   314,   256,    -1,    73,   314,   260,   257,   314,
      74,    -1,    -1,   258,    -1,   259,    -1,   258,   259,    -1,
     314,    75,   314,   260,    -1,   311,   261,    -1,    -1,   262,
      -1,   314,     3,   314,   311,    -1,    -1,   314,   264,    -1,
      73,   314,   268,   265,   314,    74,    -1,    -1,   266,    -1,
     267,    -1,   266,   267,    -1,   314,    75,   314,   268,    -1,
       7,   269,    -1,    -1,   270,    -1,   314,     3,   314,     7,
      -1,    73,   314,   272,   314,    74,    -1,   276,   273,   312,
      -1,    -1,   274,    -1,   275,    -1,   274,   275,    -1,   314,
      76,   314,   276,    -1,     6,   314,    73,   314,   309,   314,
      74,    -1,   290,   278,   312,    -1,    -1,   279,    -1,   280,
      -1,   279,   280,    -1,   314,    76,   314,   290,    -1,   292,
     282,   312,    -1,    -1,   283,    -1,   284,    -1,   283,   284,
      -1,   314,    76,   314,   292,    -1,    73,   314,   286,   314,
      74,    -1,   312,    -1,     6,   287,   312,    -1,    -1,   288,
      -1,   289,    -1,   288,   289,    -1,   314,    76,   314,     6,
      -1,     5,    -1,     6,    -1,     5,     4,     5,    -1,     5,
      -1,     5,     4,     6,    -1,     6,    -1,     8,   294,    -1,
      -1,   295,    -1,   296,    -1,   295,   296,    -1,   314,     8,
      -1,     8,    -1,     8,    -1,     8,    -1,   285,    -1,     9,
      -1,    77,     9,    -1,    10,    -1,     7,    -1,   293,    -1,
     292,    -1,   292,   306,    -1,   308,    78,   308,   306,    -1,
      62,    -1,    63,    -1,    64,    -1,    65,    -1,    66,    -1,
      67,    -1,    68,    -1,   304,   305,    -1,   292,    -1,   308,
      -1,    -1,   306,    -1,   307,    -1,   306,   307,    -1,    78,
     308,    -1,     9,    -1,    10,    -1,     9,    -1,    77,     9,
      -1,   309,    -1,   310,    -1,    -1,    -1,    -1,    -1
};

/* YYRLINE[YYN] -- source line where rule number YYN was defined.  */
static const unsigned short int yyrline[] =
{
       0,   665,   665,   676,   679,   685,   689,   704,   719,   749,
     756,   763,   774,   718,   852,   855,   865,   869,   885,   900,
     909,   915,   922,   929,   899,   940,   943,   953,   957,   973,
     988,   993,  1012,  1018,  1027,  1033,  1039,  1046,   987,  1061,
    1064,  1074,  1078,  1094,  1108,  1109,  1110,  1114,  1119,  1133,
    1139,  1153,  1113,  1176,  1181,  1194,  1209,  1215,  1221,  1230,
    1236,  1242,  1249,  1175,  1265,  1270,  1285,  1291,  1298,  1264,
    1314,  1319,  1334,  1358,  1368,  1374,  1381,  1313,  1396,  1404,
    1419,  1434,  1439,  1452,  1467,  1473,  1479,  1488,  1494,  1500,
    1507,  1433,  1523,  1526,  1536,  1540,  1556,  1571,  1576,  1590,
    1606,  1612,  1619,  1570,  1634,  1637,  1647,  1651,  1666,  1681,
    1686,  1698,  1714,  1720,  1727,  1680,  1742,  1745,  1755,  1759,
    1775,  1790,  1795,  1809,  1815,  1822,  1829,  1845,  1861,  1789,
    1886,  1889,  1895,  1899,  1914,  1929,  1934,  1928,  1961,  1964,
    1970,  1974,  1989,  2003,  2013,  2016,  2022,  2029,  2038,  2041,
    2047,  2055,  2058,  2064,  2071,  2072,  2073,  2074,  2075,  2078,
    2099,  2115,  2137,  2153,  2176,  2179,  2186,  2189,  2195,  2201,
    2207,  2213,  2219,  2226,  2229,  2235,  2242,  2245,  2251,  2258,
    2261,  2267,  2274,  2277,  2283,  2290,  2293,  2299,  2306,  2309,
    2315,  2322,  2325,  2331,  2338,  2341,  2347,  2353,  2361,  2364,
    2370,  2378,  2381,  2387,  2395,  2398,  2404,  2410,  2424,  2430,
    2444,  2447,  2453,  2459,  2472,  2478,  2508,  2512,  2518,  2533,
    2537,  2552,  2567,  2582,  2597,  2612,  2627,  2642,  2657,  2674,
    2689,  2692,  2698,  2702,  2709,  2712,  2718,  2728,  2731,  2737,
    2743,  2756,  2762,  2777,  2780,  2786,  2793,  2796,  2802,  2812,
    2815,  2821,  2827,  2840,  2846,  2863,  2866,  2872,  2878,  2884,
    2894,  2897,  2903,  2909,  2924,  2930,  2939,  2948,  2951,  2957,
    2963,  2976,  3025,  3034,  3037,  3043,  3049,  3062,  3068,  3074,
    3078,  3087,  3090,  3096,  3102,  3115,  3121,  3125,  3141,  3152,
    3158,  3169,  3175,  3195,  3198,  3204,  3208,  3218,  3224,  3230,
    3236,  3249,  3261,  3302,  3322,  3328,  3334,  3347,  3371,  3376,
    3383,  3387,  3391,  3397,  3401,  3405,  3409,  3415,  3444,  3482,
    3489,  3492,  3499,  3503,  3513,  3522,  3528,  3534,  3542,  3550,
    3554,  3565,  3572,  3578,  3584
};
#endif

#if YYDEBUG || YYERROR_VERBOSE
/* YYTNME[SYMBOL-NUM] -- String name of the symbol SYMBOL-NUM.
   First, the terminals, then, starting at YYNTOKENS, nonterminals. */
static const char *const yytname[] =
{
  "$end", "error", "$undefined", "DOT_DOT", "COLON_COLON", "ucIdentifier",
  "lcIdentifier", "floatValue", "textSegment", "decimalNumber",
  "hexadecimalNumber", "moduleKeyword", "importKeyword", "revisionKeyword",
  "identityKeyword", "oidKeyword", "dateKeyword", "organizationKeyword",
  "contactKeyword", "descriptionKeyword", "referenceKeyword",
  "extensionKeyword", "typedefKeyword", "typeKeyword", "writetypeKeyword",
  "nodeKeyword", "scalarKeyword", "tableKeyword", "columnKeyword",
  "rowKeyword", "notificationKeyword", "groupKeyword", "complianceKeyword",
  "formatKeyword", "unitsKeyword", "statusKeyword", "accessKeyword",
  "defaultKeyword", "impliedKeyword", "indexKeyword", "augmentsKeyword",
  "reordersKeyword", "sparseKeyword", "expandsKeyword", "createKeyword",
  "membersKeyword", "objectsKeyword", "mandatoryKeyword",
  "optionalKeyword", "refineKeyword", "abnfKeyword", "OctetStringKeyword",
  "ObjectIdentifierKeyword", "Integer32Keyword", "Unsigned32Keyword",
  "Integer64Keyword", "Unsigned64Keyword", "Float32Keyword",
  "Float64Keyword", "Float128Keyword", "BitsKeyword", "EnumerationKeyword",
  "currentKeyword", "deprecatedKeyword", "obsoleteKeyword",
  "noaccessKeyword", "notifyonlyKeyword", "readonlyKeyword",
  "readwriteKeyword", "readcreateKeyword", "'{'", "'}'", "';'", "'('",
  "')'", "'|'", "','", "'-'", "'.'", "$accept", "smingFile",
  "moduleStatement_optsep_0n", "moduleStatement_optsep_1n",
  "moduleStatement_optsep", "moduleStatement", "@1", "@2", "@3", "@4",
  "@5", "extensionStatement_stmtsep_0n", "extensionStatement_stmtsep_1n",
  "extensionStatement_stmtsep", "extensionStatement", "@6", "@7", "@8",
  "@9", "@10", "typedefStatement_stmtsep_0n",
  "typedefStatement_stmtsep_1n", "typedefStatement_stmtsep",
  "typedefStatement", "@11", "@12", "@13", "@14", "@15", "@16", "@17",
  "@18", "anyObjectStatement_stmtsep_0n", "anyObjectStatement_stmtsep_1n",
  "anyObjectStatement_stmtsep", "anyObjectStatement", "nodeStatement",
  "@19", "@20", "@21", "@22", "@23", "scalarStatement", "@24", "@25",
  "@26", "@27", "@28", "@29", "@30", "@31", "@32", "@33", "tableStatement",
  "@34", "@35", "@36", "@37", "@38", "rowStatement", "@39", "@40", "@41",
  "@42", "@43", "@44", "@45", "columnStatement_stmtsep_1n",
  "columnStatement_stmtsep", "columnStatement", "@46", "@47", "@48", "@49",
  "@50", "@51", "@52", "@53", "@54", "@55",
  "notificationStatement_stmtsep_0n", "notificationStatement_stmtsep_1n",
  "notificationStatement_stmtsep", "notificationStatement", "@56", "@57",
  "@58", "@59", "@60", "@61", "groupStatement_stmtsep_0n",
  "groupStatement_stmtsep_1n", "groupStatement_stmtsep", "groupStatement",
  "@62", "@63", "@64", "@65", "@66", "@67",
  "complianceStatement_stmtsep_0n", "complianceStatement_stmtsep_1n",
  "complianceStatement_stmtsep", "complianceStatement", "@68", "@69",
  "@70", "@71", "@72", "@73", "@74", "@75", "importStatement_stmtsep_0n",
  "importStatement_stmtsep_1n", "importStatement_stmtsep",
  "importStatement", "@76", "@77", "revisionStatement_stmtsep_0n",
  "revisionStatement_stmtsep_1n", "revisionStatement_stmtsep",
  "revisionStatement", "identityStatement_stmtsep_01", "identityStatement",
  "typedefTypeStatement", "typeStatement_stmtsep_01", "typeStatement",
  "writetypeStatement_stmtsep_01", "writetypeStatement",
  "anyIndexStatement", "indexStatement", "augmentsStatement",
  "reordersStatement", "sparseStatement", "expandsStatement",
  "sep_impliedKeyword_01", "createStatement_stmtsep_01", "createStatement",
  "oidStatement", "dateStatement", "organizationStatement",
  "contactStatement", "formatStatement_stmtsep_01", "formatStatement",
  "unitsStatement_stmtsep_01", "unitsStatement",
  "statusStatement_stmtsep_01", "statusStatement",
  "accessStatement_stmtsep_01", "accessStatement",
  "defaultStatement_stmtsep_01", "defaultStatement",
  "descriptionStatement_stmtsep_01", "descriptionStatement",
  "referenceStatement_stmtsep_01", "referenceStatement",
  "abnfStatement_stmtsep_01", "abnfStatement", "membersStatement",
  "objectsStatement_stmtsep_01", "objectsStatement",
  "mandatoryStatement_stmtsep_01", "mandatoryStatement",
  "optionalStatement_stmtsep_0n", "optionalStatement_stmtsep_1n",
  "optionalStatement_stmtsep", "optionalStatement",
  "refineStatement_stmtsep_0n", "refineStatement_stmtsep_1n",
  "refineStatement_stmtsep", "refineStatement",
  "refinedBaseType_refinedType", "refinedBaseType", "refinedType",
  "optsep_anySpec_01", "anySpec", "optsep_numberSpec_01", "numberSpec",
  "furtherNumberElement_0n", "furtherNumberElement_1n",
  "furtherNumberElement", "numberElement", "numberUpperLimit_01",
  "numberUpperLimit", "optsep_floatSpec_01", "floatSpec",
  "furtherFloatElement_0n", "furtherFloatElement_1n",
  "furtherFloatElement", "floatElement", "floatUpperLimit_01",
  "floatUpperLimit", "bitsOrEnumerationSpec", "bitsOrEnumerationList",
  "furtherBitsOrEnumerationItem_0n", "furtherBitsOrEnumerationItem_1n",
  "furtherBitsOrEnumerationItem", "bitsOrEnumerationItem",
  "identifierList", "furtherIdentifier_0n", "furtherIdentifier_1n",
  "furtherIdentifier", "qlcIdentifierList", "furtherQlcIdentifier_0n",
  "furtherQlcIdentifier_1n", "furtherQlcIdentifier", "bitsValue",
  "bitsList", "furtherLcIdentifier_0n", "furtherLcIdentifier_1n",
  "furtherLcIdentifier", "identifier", "qucIdentifier", "qlcIdentifier",
  "text", "optsep_textSegment_0n", "optsep_textSegment_1n",
  "optsep_textSegment", "date", "format", "units", "anyValue", "status",
  "access", "objectIdentifier", "qlcIdentifier_subid", "dot_subid_0127",
  "dot_subid_1n", "dot_subid", "subid", "number", "negativeNumber",
  "signedNumber", "optsep_comma_01", "sep", "optsep", "stmtsep", 0
};
#endif

# ifdef YYPRINT
/* YYTOKNUM[YYLEX-NUM] -- Internal token number corresponding to
   token YYLEX-NUM.  */
static const unsigned short int yytoknum[] =
{
       0,   256,   257,   258,   259,   260,   261,   262,   263,   264,
     265,   266,   267,   268,   269,   270,   271,   272,   273,   274,
     275,   276,   277,   278,   279,   280,   281,   282,   283,   284,
     285,   286,   287,   288,   289,   290,   291,   292,   293,   294,
     295,   296,   297,   298,   299,   300,   301,   302,   303,   304,
     305,   306,   307,   308,   309,   310,   311,   312,   313,   314,
     315,   316,   317,   318,   319,   320,   321,   322,   323,   324,
     123,   125,    59,    40,    41,   124,    44,    45,    46
};
# endif

/* YYR1[YYN] -- Symbol number of symbol that rule YYN derives.  */
static const unsigned short int yyr1[] =
{
       0,    79,    80,    81,    81,    82,    82,    83,    85,    86,
      87,    88,    89,    84,    90,    90,    91,    91,    92,    94,
      95,    96,    97,    98,    93,    99,    99,   100,   100,   101,
     103,   104,   105,   106,   107,   108,   109,   110,   102,   111,
     111,   112,   112,   113,   114,   114,   114,   116,   117,   118,
     119,   120,   115,   122,   123,   124,   125,   126,   127,   128,
     129,   130,   131,   121,   133,   134,   135,   136,   137,   132,
     139,   140,   141,   142,   143,   144,   145,   138,   146,   146,
     147,   149,   150,   151,   152,   153,   154,   155,   156,   157,
     158,   148,   159,   159,   160,   160,   161,   163,   164,   165,
     166,   167,   168,   162,   169,   169,   170,   170,   171,   173,
     174,   175,   176,   177,   178,   172,   179,   179,   180,   180,
     181,   183,   184,   185,   186,   187,   188,   189,   190,   182,
     191,   191,   192,   192,   193,   195,   196,   194,   197,   197,
     198,   198,   199,   200,   201,   201,   202,   203,   204,   204,
     205,   206,   206,   207,   208,   208,   208,   208,   208,   209,
     210,   211,   212,   213,   214,   214,   215,   215,   216,   217,
     218,   219,   220,   221,   221,   222,   223,   223,   224,   225,
     225,   226,   227,   227,   228,   229,   229,   230,   231,   231,
     232,   233,   233,   234,   235,   235,   236,   237,   238,   238,
     239,   240,   240,   241,   242,   242,   243,   243,   244,   245,
     246,   246,   247,   247,   248,   249,   250,   250,   251,   251,
     251,   251,   251,   251,   251,   251,   251,   251,   251,   252,
     253,   253,   254,   254,   255,   255,   256,   257,   257,   258,
     258,   259,   260,   261,   261,   262,   263,   263,   264,   265,
     265,   266,   266,   267,   268,   269,   269,   270,   271,   272,
     273,   273,   274,   274,   275,   276,   277,   278,   278,   279,
     279,   280,   281,   282,   282,   283,   283,   284,   285,   286,
     286,   287,   287,   288,   288,   289,   290,   290,   291,   291,
     292,   292,   293,   294,   294,   295,   295,   296,   297,   298,
     299,   300,   300,   300,   300,   300,   300,   300,   300,   300,
     301,   301,   301,   302,   302,   302,   302,   303,   304,   304,
     305,   305,   306,   306,   307,   308,   309,   309,   310,   311,
     311,   312,   313,   314,   315
};

/* YYR2[YYN] -- Number of symbols composing right hand side of rule YYN.  */
static const unsigned char yyr2[] =
{
       0,     2,     2,     0,     1,     1,     2,     2,     0,     0,
       0,     0,     0,    30,     0,     1,     1,     2,     2,     0,
       0,     0,     0,     0,    18,     0,     1,     1,     2,     2,
       0,     0,     0,     0,     0,     0,     0,     0,    25,     0,
       1,     1,     2,     2,     1,     1,     1,     0,     0,     0,
       0,     0,    19,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,    32,     0,     0,     0,     0,     0,    22,
       0,     0,     0,     0,     0,     0,     0,    26,     1,     2,
       2,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,    32,     0,     1,     1,     2,     2,     0,     0,     0,
       0,     0,     0,    22,     0,     1,     1,     2,     2,     0,
       0,     0,     0,     0,     0,    23,     0,     1,     1,     2,
       2,     0,     0,     0,     0,     0,     0,     0,     0,    26,
       0,     1,     1,     2,     2,     0,     0,    13,     0,     1,
       1,     2,     2,    11,     0,     2,     5,     5,     0,     2,
       5,     0,     2,     5,     1,     1,     1,     1,     1,    10,
       5,    12,     5,    12,     0,     2,     0,     2,     3,     5,
       5,     5,     5,     0,     2,     5,     0,     2,     5,     0,
       2,     5,     0,     2,     5,     0,     2,     5,     0,     2,
       5,     0,     2,     5,     0,     2,     5,     9,     0,     2,
       9,     0,     2,     9,     0,     1,     1,     2,     2,    10,
       0,     1,     1,     2,     2,    13,     1,     1,     2,     1,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       0,     2,     1,     1,     0,     2,     6,     0,     1,     1,
       2,     4,     2,     0,     1,     4,     0,     2,     6,     0,
       1,     1,     2,     4,     2,     0,     1,     4,     5,     3,
       0,     1,     1,     2,     4,     7,     3,     0,     1,     1,
       2,     4,     3,     0,     1,     1,     2,     4,     5,     1,
       3,     0,     1,     1,     2,     4,     1,     1,     3,     1,
       3,     1,     2,     0,     1,     1,     2,     2,     1,     1,
       1,     1,     1,     2,     1,     1,     1,     1,     2,     4,
       1,     1,     1,     1,     1,     1,     1,     2,     1,     1,
       0,     1,     1,     2,     2,     1,     1,     1,     2,     1,
       1,     0,     0,     0,     0
};

/* YYDEFACT[STATE-NAME] -- Default rule to reduce with in state
   STATE-NUM when YYTABLE doesn't specify something else to do.  Zero
   means the default is an error.  */
static const unsigned short int yydefact[] =
{
     333,     0,     3,     1,   332,     2,     4,     5,   333,     0,
       6,     7,     8,   333,     0,   334,   130,   332,     0,   131,
     132,   334,     0,   332,   334,   133,   134,   135,     0,     9,
     333,   293,   333,     0,     0,   292,   294,   295,     0,     0,
     332,   334,   333,   296,   297,   171,     0,    10,     0,   333,
       0,   286,   287,   136,   267,     0,   332,   334,   333,   331,
     268,   269,     0,   172,     0,    11,     0,   266,   270,   333,
     333,   191,   333,     0,     0,   332,    12,   334,     0,   271,
     190,     0,   138,   192,   137,   333,   333,   144,   139,   140,
     334,     0,     0,   332,    14,   334,   141,   142,   193,   334,
       0,   332,    25,    15,    16,   334,   145,     0,   333,     0,
     332,    39,    26,    27,   334,    17,    18,   332,   334,     0,
      19,     0,   332,   332,   332,    92,    40,    41,   334,    44,
      45,    46,    28,    29,     0,     0,   146,   333,    30,     0,
       0,     0,   332,   104,    93,    94,   334,    42,    43,   298,
     333,   334,     0,   333,    47,    53,    64,     0,   332,   116,
     105,   106,   334,    95,    96,     0,     0,   334,     0,   333,
     333,   333,    97,     0,   332,     0,   117,   118,   334,   107,
     108,   170,   333,   179,   334,     0,     0,     0,   333,   109,
       0,   333,   119,   120,     0,   332,    20,   334,     0,   334,
     334,   334,     0,   333,   121,     0,   143,     0,   188,   180,
     332,   334,     0,     0,     0,   334,     0,   333,    13,   310,
     311,   312,   333,    21,   334,     0,    31,   332,   334,   334,
     334,     0,   334,     0,     0,   191,   189,   289,   234,   219,
     234,   234,   234,   234,   246,   246,   246,     0,     0,   333,
     216,   217,   230,   185,     0,    48,    54,    65,   334,     0,
     334,   181,    22,     0,   218,     0,   220,   221,   222,   223,
     224,     0,   225,   226,   333,   228,   227,     0,   229,     0,
     332,    32,   334,     0,   291,   325,   318,   333,   320,   319,
     179,     0,   179,    98,   334,     0,   194,   288,   333,   235,
     333,   247,     0,   147,   333,   231,   232,   233,     0,   173,
     186,     0,     0,     0,   317,   321,   322,    49,   332,   334,
      66,   198,   110,   334,   332,    23,   334,     0,     0,   333,
     333,   260,     0,   305,   302,   304,   333,     0,   301,   307,
     306,   333,     0,   332,    33,   334,   290,   169,   324,   323,
     188,     0,    55,     0,   333,    99,   334,     0,   122,     0,
       0,   195,   327,   326,     0,   237,   329,   330,   243,   255,
     249,     0,     0,   331,   261,   262,     0,   331,   303,   308,
       0,     0,     0,   176,   174,    50,   333,     0,   334,     0,
     179,   199,   333,   334,   179,   333,   333,   328,   333,   238,
     239,     0,   242,   244,     0,   254,   256,     0,   333,   250,
     251,     0,   333,   258,   259,   263,   333,   281,   333,   279,
     187,     0,   299,   333,   332,    34,   334,   191,     0,   332,
     334,    67,   333,   100,     0,   111,   123,     0,     0,     0,
     240,   333,   333,   333,     0,   252,   333,     0,     0,   331,
     282,   283,     0,     0,   309,     0,     0,   179,   177,    51,
     150,     0,    56,   191,     0,     0,   333,   179,     0,   196,
      24,   236,     0,     0,     0,   248,     0,   333,   264,   280,
     284,   333,   278,   175,   300,   333,    35,     0,   313,   314,
     315,   316,   333,   185,    68,   333,   273,   334,     0,   112,
     334,   241,   245,   257,   253,     0,     0,     0,   188,   333,
       0,    57,     0,     0,   331,   274,   275,     0,   101,   333,
       0,   124,   265,   285,   178,    36,     0,   184,   173,   332,
     334,   333,   272,   276,   333,   191,     0,   334,   191,   191,
      52,    58,     0,     0,     0,     0,   102,   333,   113,   125,
      37,   176,    70,   333,   200,   277,     0,     0,   191,   201,
       0,    59,   333,     0,   333,   197,   114,   333,   126,   334,
     333,   179,     0,    69,     0,     0,     0,   204,   202,     0,
      60,   334,   103,   333,   333,   332,   127,   205,   206,   334,
      38,     0,     0,     0,     0,     0,   210,   207,   208,   334,
     334,   115,   333,   333,   332,   128,   211,   212,   334,    61,
      71,     0,     0,     0,     0,   213,   214,   191,     0,   333,
       0,   333,   333,    62,   164,   332,   332,   332,   332,   334,
     154,   155,   156,   157,   158,     0,   334,     0,     0,     0,
     333,     0,     0,     0,     0,     0,    72,   203,     0,   148,
     129,   333,     0,   165,   333,   164,   333,   164,   166,   333,
     151,   334,     0,   333,     0,   333,     0,   333,   333,    73,
     334,     0,   332,   182,   334,   149,    63,     0,   160,     0,
     162,     0,     0,   179,   167,   209,     0,     0,   334,   152,
     333,   333,   333,   168,    74,   333,   334,   183,     0,     0,
       0,     0,     0,     0,   333,   333,   333,   334,   153,   333,
       0,     0,     0,    75,     0,   159,   333,   333,   191,   215,
       0,     0,    76,   161,   163,     0,   332,     0,    78,   334,
       0,   333,    79,    80,    81,     0,   333,    77,     0,   334,
       0,   334,    82,     0,   334,    83,     0,   334,    84,   185,
      85,   173,    86,   176,    87,   179,    88,     0,   334,    89,
     191,    90,     0,   333,     0,    91
};

/* YYDEFGOTO[NTERM-NUM]. */
static const short int yydefgoto[] =
{
      -1,     1,     5,     6,     7,     8,    13,    33,    50,    71,
      82,   102,   103,   104,   105,   137,   208,   235,   296,   360,
     111,   112,   113,   114,   153,   253,   309,   383,   457,   508,
     539,   560,   125,   126,   127,   128,   129,   169,   290,   350,
     427,   487,   130,   170,   291,   387,   493,   528,   551,   571,
     591,   617,   639,   131,   171,   292,   353,   463,   512,   530,
     562,   618,   658,   683,   701,   718,   725,   727,   728,   729,
     736,   743,   746,   749,   751,   753,   755,   757,   760,   762,
     143,   144,   145,   146,   188,   321,   390,   465,   535,   556,
     159,   160,   161,   162,   203,   357,   467,   520,   558,   575,
     175,   176,   177,   178,   217,   394,   468,   538,   559,   577,
     596,   614,    18,    19,    20,    21,    30,    58,    87,    88,
      89,    90,    94,    95,   211,   660,   319,   673,   674,   629,
     630,   631,   632,   633,   634,   640,   669,   670,   228,   118,
      24,    41,   344,   345,   425,   426,   196,   197,   687,   430,
     281,   282,   223,   224,    76,    77,   325,   326,   393,   355,
     356,   568,   569,   586,   587,   588,   589,   605,   606,   607,
     608,   249,   250,   251,   278,   305,   264,   299,   398,   399,
     400,   365,   402,   403,   270,   301,   408,   409,   410,   370,
     405,   406,   275,   330,   373,   374,   375,   331,    53,    59,
      60,    61,   495,   514,   515,   516,   338,   418,   449,   450,
     451,    54,   252,   496,    32,    35,    36,    37,   150,   423,
     485,   341,   222,   492,   287,   288,   314,   315,   316,   289,
     366,   367,   368,    67,   641,   265,    16
};

/* YYPACT[STATE-NUM] -- Index in YYTABLE of the portion describing
   STATE-NUM.  */
#define YYPACT_NINF -649
static const short int yypact[] =
{
    -649,    26,    28,  -649,  -649,  -649,    28,  -649,  -649,    32,
    -649,  -649,  -649,  -649,   -20,  -649,    47,  -649,    50,    47,
    -649,  -649,    91,  -649,  -649,  -649,  -649,  -649,    92,  -649,
    -649,    94,  -649,    81,    31,  -649,    94,  -649,    95,    37,
    -649,  -649,  -649,  -649,  -649,  -649,    92,  -649,    42,  -649,
      93,  -649,  -649,  -649,    38,    44,  -649,  -649,  -649,  -649,
      38,  -649,    52,  -649,    92,  -649,    41,  -649,  -649,  -649,
    -649,   110,  -649,    42,    59,  -649,  -649,  -649,    60,  -649,
    -649,    92,   121,  -649,  -649,  -649,  -649,   122,   121,  -649,
    -649,    63,    72,  -649,   123,  -649,  -649,  -649,  -649,  -649,
     137,  -649,   124,   123,  -649,  -649,  -649,   129,  -649,   141,
    -649,    30,   124,  -649,  -649,  -649,  -649,  -649,  -649,    76,
    -649,   144,  -649,  -649,  -649,   125,    30,  -649,  -649,  -649,
    -649,  -649,  -649,  -649,   146,    93,  -649,  -649,  -649,   150,
     151,   152,  -649,   130,   125,  -649,  -649,  -649,  -649,  -649,
    -649,  -649,    82,  -649,  -649,  -649,  -649,   154,  -649,   131,
     130,  -649,  -649,  -649,  -649,    90,    96,  -649,    98,  -649,
    -649,  -649,  -649,   158,  -649,   101,   131,  -649,  -649,  -649,
    -649,  -649,  -649,   138,  -649,   106,   107,   108,  -649,  -649,
     159,  -649,  -649,  -649,   109,  -649,  -649,  -649,   143,  -649,
    -649,  -649,   113,  -649,  -649,   114,  -649,    11,    93,  -649,
    -649,  -649,   164,   164,   164,  -649,   115,  -649,  -649,  -649,
    -649,  -649,  -649,  -649,  -649,    66,  -649,  -649,  -649,  -649,
    -649,   164,  -649,   117,   118,   110,  -649,   176,   119,  -649,
     119,   119,   119,   119,   119,   119,   119,   120,   120,  -649,
    -649,  -649,   119,   157,    35,  -649,  -649,  -649,  -649,   164,
    -649,  -649,  -649,   190,  -649,   126,  -649,  -649,  -649,  -649,
    -649,   127,  -649,  -649,  -649,  -649,  -649,   132,  -649,   128,
    -649,  -649,  -649,   192,  -649,  -649,  -649,  -649,   136,  -649,
     138,   174,   138,  -649,  -649,   164,   148,  -649,  -649,  -649,
    -649,  -649,   196,  -649,  -649,  -649,  -649,  -649,    56,   172,
    -649,   200,   147,   199,  -649,   136,  -649,  -649,  -649,  -649,
    -649,   163,  -649,  -649,  -649,  -649,  -649,    18,   203,  -649,
    -649,    38,    36,  -649,   140,  -649,  -649,   206,  -649,   136,
    -649,  -649,   142,  -649,  -649,  -649,  -649,  -649,  -649,  -649,
      93,    66,  -649,    93,  -649,  -649,  -649,   171,  -649,    92,
     153,  -649,  -649,  -649,   212,   155,  -649,  -649,   220,   220,
     155,   156,   160,  -649,    38,  -649,   149,   222,  -649,   136,
     161,   199,   218,   197,  -649,  -649,  -649,   201,  -649,   162,
     138,  -649,  -649,  -649,   138,  -649,  -649,  -649,  -649,   155,
    -649,   165,  -649,  -649,   229,  -649,  -649,   233,  -649,   155,
    -649,   166,  -649,  -649,  -649,  -649,  -649,    38,  -649,  -649,
    -649,   136,  -649,  -649,  -649,  -649,  -649,   110,   167,  -649,
    -649,  -649,  -649,  -649,   169,  -649,  -649,   175,   178,   177,
    -649,  -649,  -649,  -649,   179,  -649,  -649,    43,   196,  -649,
      38,  -649,   180,   181,   136,   182,   230,   138,  -649,  -649,
    -649,    23,  -649,   110,    78,    93,  -649,   138,    93,  -649,
    -649,  -649,    18,    18,   241,  -649,   203,  -649,  -649,  -649,
    -649,  -649,  -649,  -649,  -649,  -649,  -649,   187,  -649,  -649,
    -649,  -649,  -649,   157,  -649,  -649,    38,  -649,    78,  -649,
    -649,  -649,  -649,  -649,  -649,   185,   254,   189,    93,  -649,
     191,  -649,   235,   188,  -649,    38,  -649,   193,  -649,  -649,
      93,  -649,  -649,  -649,  -649,  -649,   194,  -649,   172,  -649,
    -649,  -649,  -649,  -649,  -649,   110,   198,  -649,   110,   110,
    -649,  -649,   261,   202,   204,    78,  -649,  -649,  -649,  -649,
    -649,   197,  -649,  -649,  -649,  -649,   207,   205,   110,   221,
     208,  -649,  -649,   209,  -649,  -649,  -649,  -649,  -649,  -649,
    -649,   138,   210,  -649,   211,   213,   216,   223,  -649,   214,
    -649,  -649,  -649,  -649,  -649,  -649,  -649,   223,  -649,  -649,
    -649,    93,   164,   219,    78,    78,   226,  -649,  -649,  -649,
    -649,  -649,  -649,  -649,  -649,  -649,   226,  -649,  -649,  -649,
    -649,   225,   215,    78,   224,  -649,  -649,   110,    39,  -649,
      93,  -649,  -649,  -649,   232,  -649,  -649,  -649,  -649,  -649,
    -649,  -649,  -649,  -649,  -649,   231,  -649,   227,   234,   236,
    -649,   244,    78,    78,    78,    78,  -649,  -649,   237,   174,
    -649,  -649,   217,  -649,  -649,   232,  -649,   232,   248,  -649,
     269,  -649,   238,  -649,   239,  -649,   240,  -649,  -649,  -649,
    -649,   242,  -649,   201,  -649,  -649,  -649,    78,  -649,   228,
    -649,   247,   245,   138,  -649,  -649,    66,    93,  -649,  -649,
    -649,  -649,  -649,  -649,  -649,  -649,  -649,  -649,   250,    78,
      78,    93,   249,   251,  -649,  -649,  -649,  -649,  -649,  -649,
     253,   252,   258,  -649,   255,  -649,  -649,  -649,   110,  -649,
     256,   262,  -649,  -649,  -649,   266,  -649,     6,  -649,  -649,
     290,  -649,  -649,  -649,  -649,   263,  -649,  -649,   243,  -649,
     164,  -649,  -649,   174,  -649,  -649,   201,  -649,  -649,   157,
    -649,   172,  -649,   197,  -649,   138,  -649,    93,  -649,  -649,
     110,  -649,   268,  -649,   270,  -649
};

/* YYPGOTO[NTERM-NUM].  */
static const short int yypgoto[] =
{
    -649,  -649,  -649,  -649,   296,  -649,  -649,  -649,  -649,  -649,
    -649,  -649,  -649,   246,  -649,  -649,  -649,  -649,  -649,  -649,
    -649,  -649,   259,  -649,  -649,  -649,  -649,  -649,  -649,  -649,
    -649,  -649,  -649,  -649,   183,  -649,  -649,  -649,  -649,  -649,
    -649,  -649,  -649,  -649,  -649,  -649,  -649,  -649,  -649,  -649,
    -649,  -649,  -649,  -649,  -649,  -649,  -649,  -649,  -649,  -649,
    -649,  -649,  -649,  -649,  -649,  -649,  -649,  -649,  -394,  -649,
    -649,  -649,  -649,  -649,  -649,  -649,  -649,  -649,  -649,  -649,
    -649,  -649,   257,  -649,  -649,  -649,  -649,  -649,  -649,  -649,
    -649,  -649,   184,  -649,  -649,  -649,  -649,  -649,  -649,  -649,
    -649,  -649,   170,  -649,  -649,  -649,  -649,  -649,  -649,  -649,
    -649,  -649,  -649,  -649,   321,  -649,  -649,  -649,  -649,  -649,
     260,  -649,  -649,  -649,  -649,  -649,  -633,  -649,  -649,  -649,
    -649,  -649,  -649,  -649,  -649,  -622,  -649,  -649,  -208,  -649,
    -649,  -649,  -524,  -649,  -540,  -649,  -283,  -649,  -649,  -648,
    -492,  -649,  -333,   -30,  -220,  -649,  -649,  -649,  -649,  -649,
    -649,  -649,  -649,  -649,  -649,  -244,  -649,  -649,  -649,  -261,
    -649,  -349,  -649,  -649,  -649,  -649,  -102,    68,  -649,  -649,
     -49,  -121,  -649,  -649,  -153,    73,  -649,  -649,   -56,  -120,
    -649,  -649,   111,  -649,  -649,  -649,   -17,   -90,  -649,  -649,
    -649,   300,  -488,  -649,  -649,  -154,  -649,  -649,  -649,  -649,
     -88,   291,  -649,  -240,   -43,  -649,  -649,   327,  -649,  -649,
    -649,  -649,  -649,  -649,  -649,  -649,  -649,  -320,  -303,  -284,
     -81,  -649,  -106,  -355,   435,     0,   584
};

/* YYTABLE[YYPACT[STATE-NUM]].  What to do in state STATE-NUM.  If
   positive, shift that token.  If negative, reduce the rule which
   number is the opposite.  If zero, do what YYDEFACT says.
   If YYTABLE_NINF, syntax error.  */
#define YYTABLE_NINF -334
static const short int yytable[] =
{
       2,   511,   386,    49,   541,   229,   230,   317,    11,   320,
     519,   561,   349,    14,   286,   262,   661,   385,   414,   379,
      57,    70,   419,   258,   342,   688,     3,   362,   363,   348,
      34,    38,    39,   665,   726,   667,    38,    12,    85,     4,
     283,   284,    48,   369,   285,   362,   363,    51,    52,    55,
      15,   294,   362,   363,    62,   122,   123,   124,    66,    17,
      62,   283,   284,   333,    31,   334,   335,    23,   339,    73,
      74,   237,    78,   219,   220,   221,   349,   731,   624,   625,
     626,   627,   628,   283,   284,    91,    92,   323,   488,   489,
     490,   491,   272,   273,   479,   364,    27,   421,   747,    40,
      31,   454,  -333,    44,    42,   151,   602,   433,   119,    45,
     744,   436,    56,   364,  -333,    72,    63,   238,   239,   240,
     241,   242,   243,   244,   245,   246,   247,   248,    69,   336,
      75,    80,    84,   337,    86,    98,    93,   152,   266,   267,
     268,   269,    99,   108,   101,   117,   110,   120,   136,   138,
     165,   349,   167,   168,   149,   142,   154,   155,   156,   532,
     172,   158,   181,   174,   189,   204,   210,   182,   184,   185,
     186,   187,   191,   195,   486,   525,   199,   200,   201,   227,
     263,   206,   194,   215,   499,   232,   218,   260,   202,   690,
     261,   205,  -333,   274,   280,   297,   311,   318,   324,   298,
     300,   304,   329,   216,   303,   343,   346,   459,   285,   354,
     369,   705,   706,   754,   313,   378,   392,   233,  -325,   347,
     381,   397,   234,  -333,   396,   416,   422,   752,   417,   412,
    -333,   424,   442,   420,   413,   432,   443,   429,   484,   460,
     441,   446,   466,   494,   271,   271,   271,   469,   503,   277,
     470,   471,   279,   475,   483,   482,   481,   750,   509,   522,
     523,   524,   531,   527,   529,   340,   540,   552,   567,   534,
    -332,   585,   547,   553,   302,   604,   554,   565,   564,   570,
     581,   573,   653,   582,   583,   620,   590,   312,   580,   584,
     663,   601,   668,   672,   726,   622,   734,   649,   327,   619,
     328,   691,    10,   647,   332,   555,   650,   651,   659,   147,
     676,   678,   680,   739,   685,   546,   395,   693,   549,   550,
     692,   708,   709,   388,   704,   715,   716,   719,   723,   371,
     372,   376,   717,   732,   724,   737,   377,   695,   566,   763,
      25,   380,   765,   597,   179,   615,   192,   306,    96,   115,
     440,   501,   307,   445,   389,   603,   504,   415,   478,   276,
      68,   533,   480,    43,    79,   401,   477,   502,   404,   407,
     411,   132,     0,   621,   376,     0,     0,     0,     0,     0,
       0,     0,     0,     0,   600,     0,   428,     0,     0,     0,
       0,     0,   434,     0,     0,   437,   438,   623,   439,   401,
     694,   163,   654,   655,   656,   657,     0,     0,   444,   411,
       0,     0,   447,     0,     0,     0,   448,   452,   453,     0,
       0,     0,     0,   455,     0,     0,     0,     0,     0,     0,
       0,     0,   464,     0,     0,   497,     0,     0,   500,     9,
       0,   472,   473,   474,     0,     0,   476,     0,     0,     0,
     452,     0,    22,     0,     0,     0,     0,     0,    28,     0,
       0,     0,     0,     0,     0,     0,   498,     0,     0,     0,
       0,     0,   756,     0,     0,    46,     0,   505,     0,     0,
       0,   506,     0,     0,     0,   507,     0,     0,     0,     0,
     537,    64,   510,     0,     0,   513,   517,     0,   722,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,   526,
      81,     0,     0,     0,     0,   517,     0,     0,     0,   536,
       0,     0,     0,     0,     0,     0,     0,     0,   100,     0,
       0,   544,   741,     0,   545,     0,   109,     0,     0,     0,
     761,     0,     0,     0,     0,   121,     0,   557,     0,     0,
       0,     0,   134,   563,     0,     0,     0,   139,   140,   141,
       0,   599,   572,     0,   574,     0,     0,   576,     0,     0,
     579,     0,     0,     0,     0,     0,     0,   157,     0,     0,
       0,     0,     0,   593,   594,     0,     0,     0,     0,     0,
     636,     0,     0,   173,     0,     0,     0,     0,     0,     0,
       0,     0,   611,   612,     0,    26,     0,     0,    29,   190,
       0,     0,     0,     0,     0,     0,     0,     0,     0,   635,
       0,   637,   638,     0,     0,    47,     0,     0,     0,     0,
     207,     0,     0,     0,     0,     0,     0,     0,     0,     0,
     652,    65,     0,     0,     0,   225,     0,     0,     0,     0,
       0,   662,     0,     0,   664,     0,   666,   696,     0,   671,
       0,    83,   254,   677,     0,   679,     0,   681,   682,     0,
       0,   707,     0,     0,    97,     0,     0,     0,     0,   106,
       0,     0,     0,   107,     0,     0,     0,     0,     0,   116,
     698,   699,   700,     0,     0,   702,     0,     0,   133,     0,
       0,     0,   135,     0,   710,   711,   712,     0,     0,   714,
       0,     0,   148,     0,     0,   308,   720,   721,     0,     0,
       0,     0,     0,     0,     0,     0,     0,   758,     0,     0,
     164,   735,     0,     0,     0,   166,   738,     0,     0,     0,
       0,     0,     0,     0,     0,     0,   180,     0,     0,     0,
       0,   183,     0,   351,     0,     0,     0,     0,     0,   359,
       0,     0,   193,   764,     0,     0,     0,     0,   198,     0,
       0,     0,     0,     0,     0,     0,     0,     0,   382,     0,
       0,   209,     0,   212,   213,   214,     0,     0,     0,     0,
       0,     0,     0,     0,     0,   226,     0,     0,     0,   231,
       0,     0,     0,     0,     0,     0,     0,     0,   236,     0,
       0,     0,   255,   256,   257,     0,   259,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,   293,     0,   295,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,   456,
       0,     0,     0,     0,   461,     0,   310,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,   322,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,   352,     0,     0,     0,   358,     0,     0,
     361,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,   384,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
     391,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,   542,     0,     0,     0,     0,     0,
       0,     0,   431,     0,     0,     0,     0,   435,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
     458,     0,     0,     0,   462,     0,     0,     0,     0,     0,
     595,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,   613,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
     642,   643,   644,   645,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,   518,     0,     0,   521,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,   686,     0,     0,
       0,     0,     0,     0,   543,     0,     0,     0,     0,     0,
       0,   548,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,   578,     0,     0,     0,     0,     0,     0,
       0,   730,     0,     0,     0,   592,     0,     0,     0,     0,
       0,     0,     0,   598,     0,     0,     0,     0,     0,     0,
       0,     0,     0,   609,   610,     0,     0,     0,     0,     0,
       0,     0,   616,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,   646,     0,     0,     0,     0,     0,     0,
     648,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,   675,     0,     0,     0,     0,
       0,     0,     0,     0,   684,     0,     0,     0,   689,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,   697,     0,     0,     0,     0,     0,     0,     0,
     703,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,   713,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,   733,     0,     0,     0,     0,     0,     0,
       0,     0,     0,   740,     0,   742,     0,     0,   745,     0,
       0,   748,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,   759
};

static const short int yycheck[] =
{
       0,   493,   351,    46,   528,   213,   214,   290,     8,   292,
     498,   551,   315,    13,   254,   235,   649,   350,   373,   339,
      50,    64,   377,   231,   308,   673,     0,     9,    10,   313,
      30,    31,    32,   655,    28,   657,    36,     5,    81,    11,
       5,     6,    42,     7,     9,     9,    10,     5,     6,    49,
      70,   259,     9,    10,    54,    25,    26,    27,    58,    12,
      60,     5,     6,     7,     8,     9,    10,    17,   308,    69,
      70,     5,    72,    62,    63,    64,   379,    71,    39,    40,
      41,    42,    43,     5,     6,    85,    86,   295,    65,    66,
      67,    68,   245,   246,   449,    77,     5,   381,   746,    18,
       8,   421,     8,     8,    73,   135,   594,   390,   108,    72,
     743,   394,    19,    77,    76,    74,    72,    51,    52,    53,
      54,    55,    56,    57,    58,    59,    60,    61,    76,    73,
      20,    72,    72,    77,    13,    72,    14,   137,   240,   241,
     242,   243,    70,     6,    21,    16,    22,     6,    72,     5,
     150,   454,    70,   153,     8,    30,     6,     6,     6,   514,
       6,    31,    72,    32,     6,     6,    23,    71,    70,   169,
     170,   171,    71,    35,   457,   508,    70,    70,    70,    15,
       4,    72,   182,    70,   467,    70,    72,    70,   188,   677,
      72,   191,    73,    73,    37,     5,     4,    23,    50,    73,
      73,    73,     6,   203,    72,    33,     6,   427,     9,    46,
       7,   699,   700,   753,    78,     9,    45,   217,    78,    72,
      78,     9,   222,     3,    71,    76,     8,   751,     6,    73,
      75,    34,     3,    72,    74,    73,     3,    36,     8,    72,
      75,    75,    73,   463,   244,   245,   246,    72,     7,   249,
      72,    74,   252,    74,    72,    74,    76,   749,    71,    74,
       6,    72,    74,    72,    29,   308,    72,     6,    47,    76,
      38,    48,    74,    71,   274,    49,    72,    72,    71,    71,
      70,    72,    38,    72,    71,    70,    72,   287,   571,    73,
      73,    72,    44,    24,    28,    71,     6,    70,   298,    74,
     300,    73,     6,    72,   304,   545,    72,    71,    71,   126,
      72,    72,    72,    70,    72,   535,   359,    72,   538,   539,
      73,    72,    71,   353,    74,    72,    74,    72,    72,   329,
     330,   331,    74,   727,    72,    72,   336,   686,   558,    71,
      19,   341,    72,   587,   160,   606,   176,   279,    88,   103,
     399,   472,   279,   409,   354,   595,   476,   374,   448,   248,
      60,   515,   450,    36,    73,   365,   447,   473,   368,   369,
     370,   112,    -1,   613,   374,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,   592,    -1,   386,    -1,    -1,    -1,
      -1,    -1,   392,    -1,    -1,   395,   396,   617,   398,   399,
     683,   144,   642,   643,   644,   645,    -1,    -1,   408,   409,
      -1,    -1,   412,    -1,    -1,    -1,   416,   417,   418,    -1,
      -1,    -1,    -1,   423,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,   432,    -1,    -1,   465,    -1,    -1,   468,     4,
      -1,   441,   442,   443,    -1,    -1,   446,    -1,    -1,    -1,
     450,    -1,    17,    -1,    -1,    -1,    -1,    -1,    23,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,   466,    -1,    -1,    -1,
      -1,    -1,   755,    -1,    -1,    40,    -1,   477,    -1,    -1,
      -1,   481,    -1,    -1,    -1,   485,    -1,    -1,    -1,    -1,
     520,    56,   492,    -1,    -1,   495,   496,    -1,   718,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,   509,
      75,    -1,    -1,    -1,    -1,   515,    -1,    -1,    -1,   519,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    93,    -1,
      -1,   531,   740,    -1,   534,    -1,   101,    -1,    -1,    -1,
     760,    -1,    -1,    -1,    -1,   110,    -1,   547,    -1,    -1,
      -1,    -1,   117,   553,    -1,    -1,    -1,   122,   123,   124,
      -1,   591,   562,    -1,   564,    -1,    -1,   567,    -1,    -1,
     570,    -1,    -1,    -1,    -1,    -1,    -1,   142,    -1,    -1,
      -1,    -1,    -1,   583,   584,    -1,    -1,    -1,    -1,    -1,
     620,    -1,    -1,   158,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,   602,   603,    -1,    21,    -1,    -1,    24,   174,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,   619,
      -1,   621,   622,    -1,    -1,    41,    -1,    -1,    -1,    -1,
     195,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
     640,    57,    -1,    -1,    -1,   210,    -1,    -1,    -1,    -1,
      -1,   651,    -1,    -1,   654,    -1,   656,   687,    -1,   659,
      -1,    77,   227,   663,    -1,   665,    -1,   667,   668,    -1,
      -1,   701,    -1,    -1,    90,    -1,    -1,    -1,    -1,    95,
      -1,    -1,    -1,    99,    -1,    -1,    -1,    -1,    -1,   105,
     690,   691,   692,    -1,    -1,   695,    -1,    -1,   114,    -1,
      -1,    -1,   118,    -1,   704,   705,   706,    -1,    -1,   709,
      -1,    -1,   128,    -1,    -1,   280,   716,   717,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,   757,    -1,    -1,
     146,   731,    -1,    -1,    -1,   151,   736,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,   162,    -1,    -1,    -1,
      -1,   167,    -1,   318,    -1,    -1,    -1,    -1,    -1,   324,
      -1,    -1,   178,   763,    -1,    -1,    -1,    -1,   184,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,   343,    -1,
      -1,   197,    -1,   199,   200,   201,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,   211,    -1,    -1,    -1,   215,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,   224,    -1,
      -1,    -1,   228,   229,   230,    -1,   232,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,   258,    -1,   260,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,   424,
      -1,    -1,    -1,    -1,   429,    -1,   282,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,   294,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,   319,    -1,    -1,    -1,   323,    -1,    -1,
     326,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,   345,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
     356,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,   529,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,   388,    -1,    -1,    -1,    -1,   393,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
     426,    -1,    -1,    -1,   430,    -1,    -1,    -1,    -1,    -1,
     585,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,   604,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
     625,   626,   627,   628,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,   497,    -1,    -1,   500,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,   672,    -1,    -1,
      -1,    -1,    -1,    -1,   530,    -1,    -1,    -1,    -1,    -1,
      -1,   537,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,   569,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,   726,    -1,    -1,    -1,   581,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,   589,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,   599,   600,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,   608,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,   629,    -1,    -1,    -1,    -1,    -1,    -1,
     636,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,   661,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,   670,    -1,    -1,    -1,   674,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,   688,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
     696,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,   707,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,   729,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,   739,    -1,   741,    -1,    -1,   744,    -1,
      -1,   747,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,   758
};

/* YYSTOS[STATE-NUM] -- The (internal number of the) accessing
   symbol of state STATE-NUM.  */
static const unsigned short int yystos[] =
{
       0,    80,   314,     0,    11,    81,    82,    83,    84,   313,
      83,   314,     5,    85,   314,    70,   315,    12,   191,   192,
     193,   194,   313,    17,   219,   193,   315,     5,   313,   315,
     195,     8,   293,    86,   314,   294,   295,   296,   314,   314,
      18,   220,    73,   296,     8,    72,   313,   315,   314,   293,
      87,     5,     6,   277,   290,   314,    19,   232,   196,   278,
     279,   280,   314,    72,   313,   315,   314,   312,   280,    76,
     293,    88,    74,   314,   314,    20,   233,   234,   314,   290,
      72,   313,    89,   315,    72,   293,    13,   197,   198,   199,
     200,   314,   314,    14,   201,   202,   199,   315,    72,    70,
     313,    21,    90,    91,    92,    93,   315,   315,     6,   313,
      22,    99,   100,   101,   102,    92,   315,    16,   218,   314,
       6,   313,    25,    26,    27,   111,   112,   113,   114,   115,
     121,   132,   101,   315,   313,   315,    72,    94,     5,   313,
     313,   313,    30,   159,   160,   161,   162,   113,   315,     8,
     297,   232,   314,   103,     6,     6,     6,   313,    31,   169,
     170,   171,   172,   161,   315,   314,   315,    70,   314,   116,
     122,   133,     6,   313,    32,   179,   180,   181,   182,   171,
     315,    72,    71,   315,    70,   314,   314,   314,   163,     6,
     313,    71,   181,   315,   314,    35,   225,   226,   315,    70,
      70,    70,   314,   173,     6,   314,    72,   313,    95,   315,
      23,   203,   315,   315,   315,    70,   314,   183,    72,    62,
      63,    64,   301,   231,   232,   313,   315,    15,   217,   217,
     217,   315,    70,   314,   314,    96,   315,     5,    51,    52,
      53,    54,    55,    56,    57,    58,    59,    60,    61,   250,
     251,   252,   291,   104,   313,   315,   315,   315,   217,   315,
      70,    72,   233,     4,   255,   314,   255,   255,   255,   255,
     263,   314,   263,   263,    73,   271,   271,   314,   253,   314,
      37,   229,   230,     5,     6,     9,   292,   303,   304,   308,
     117,   123,   134,   315,   217,   315,    97,     5,    73,   256,
      73,   264,   314,    72,    73,   254,   256,   264,   313,   105,
     315,     4,   314,    78,   305,   306,   307,   225,    23,   205,
     225,   164,   315,   217,    50,   235,   236,   314,   314,     6,
     272,   276,   314,     7,     9,    10,    73,    77,   285,   292,
     293,   300,   308,    33,   221,   222,     6,    72,   308,   307,
     118,   313,   315,   135,    46,   238,   239,   174,   315,   313,
      98,   315,     9,    10,    77,   260,   309,   310,   311,     7,
     268,   314,   314,   273,   274,   275,   314,   314,     9,   306,
     314,    78,   313,   106,   315,   231,   250,   124,   232,   314,
     165,   315,    45,   237,   184,   293,    71,     9,   257,   258,
     259,   314,   261,   262,   314,   269,   270,   314,   265,   266,
     267,   314,    73,    74,   312,   275,    76,     6,   286,   312,
      72,   308,     8,   298,    34,   223,   224,   119,   314,    36,
     228,   315,    73,   225,   314,   315,   225,   314,   314,   314,
     259,    75,     3,     3,   314,   267,    75,   314,   314,   287,
     288,   289,   314,   314,   306,   314,   313,   107,   315,   233,
      72,   313,   315,   136,   314,   166,    73,   175,   185,    72,
      72,    74,   314,   314,   314,    74,   314,   309,   276,   312,
     289,    76,    74,    72,     8,   299,   225,   120,    65,    66,
      67,    68,   302,   125,   233,   281,   292,   232,   314,   225,
     232,   260,   311,     7,   268,   314,   314,   314,   108,    71,
     314,   229,   137,   314,   282,   283,   284,   314,   315,   281,
     176,   315,    74,     6,    72,   231,   314,    72,   126,    29,
     138,    74,   312,   284,    76,   167,   314,   232,   186,   109,
      72,   221,   313,   315,   314,   314,   233,    74,   315,   233,
     233,   127,     6,    71,    72,   292,   168,   314,   177,   187,
     110,   223,   139,   314,    71,    72,   233,    47,   240,   241,
      71,   128,   314,    72,   314,   178,   314,   188,   315,   314,
     225,    70,    72,    71,    73,    48,   242,   243,   244,   245,
      72,   129,   315,   314,   314,   313,   189,   244,   315,   232,
     217,    72,   281,   292,    49,   246,   247,   248,   249,   315,
     315,   314,   314,   313,   190,   248,   315,   130,   140,    74,
      70,   292,    71,   233,    39,    40,    41,    42,    43,   208,
     209,   210,   211,   212,   213,   314,   232,   314,   314,   131,
     214,   313,   313,   313,   313,   313,   315,    72,   315,    70,
      72,    71,   314,    38,   292,   292,   292,   292,   141,    71,
     204,   205,   314,    73,   314,   214,   314,   214,    44,   215,
     216,   314,    24,   206,   207,   315,    72,   314,    72,   314,
      72,   314,   314,   142,   315,    72,   313,   227,   228,   315,
     281,    73,    73,    72,   225,   250,   232,   315,   314,   314,
     314,   143,   314,   315,    74,   281,   281,   232,    72,    71,
     314,   314,   314,   315,   314,    72,    74,    74,   144,    72,
     314,   314,   233,    72,    72,   145,    28,   146,   147,   148,
     313,    71,   147,   315,     6,   314,   149,    72,   314,    70,
     315,   217,   315,   150,   205,   315,   151,   228,   315,   152,
     229,   153,   221,   154,   223,   155,   225,   156,   232,   315,
     157,   233,   158,    71,   314,    72
};

#if ! defined (YYSIZE_T) && defined (__SIZE_TYPE__)
# define YYSIZE_T __SIZE_TYPE__
#endif
#if ! defined (YYSIZE_T) && defined (size_t)
# define YYSIZE_T size_t
#endif
#if ! defined (YYSIZE_T)
# if defined (__STDC__) || defined (__cplusplus)
#  include <stddef.h> /* INFRINGES ON USER NAME SPACE */
#  define YYSIZE_T size_t
# endif
#endif
#if ! defined (YYSIZE_T)
# define YYSIZE_T unsigned int
#endif

#define yyerrok		(yyerrstatus = 0)
#define yyclearin	(yychar = YYEMPTY)
#define YYEMPTY		(-2)
#define YYEOF		0

#define YYACCEPT	goto yyacceptlab
#define YYABORT		goto yyabortlab
#define YYERROR		goto yyerrorlab


/* Like YYERROR except do call yyerror.  This remains here temporarily
   to ease the transition to the new meaning of YYERROR, for GCC.
   Once GCC version 2 has supplanted version 1, this can go.  */

#define YYFAIL		goto yyerrlab

#define YYRECOVERING()  (!!yyerrstatus)

#define YYBACKUP(Token, Value)					\
do								\
  if (yychar == YYEMPTY && yylen == 1)				\
    {								\
      yychar = (Token);						\
      yylval = (Value);						\
      yytoken = YYTRANSLATE (yychar);				\
      YYPOPSTACK;						\
      goto yybackup;						\
    }								\
  else								\
    { 								\
      yyerror ("syntax error: cannot back up");\
      YYERROR;							\
    }								\
while (0)

#define YYTERROR	1
#define YYERRCODE	256

/* YYLLOC_DEFAULT -- Compute the default location (before the actions
   are run).  */

#ifndef YYLLOC_DEFAULT
# define YYLLOC_DEFAULT(Current, Rhs, N)		\
   ((Current).first_line   = (Rhs)[1].first_line,	\
    (Current).first_column = (Rhs)[1].first_column,	\
    (Current).last_line    = (Rhs)[N].last_line,	\
    (Current).last_column  = (Rhs)[N].last_column)
#endif

/* YYLEX -- calling `yylex' with the right arguments.  */

#ifdef YYLEX_PARAM
# define YYLEX yylex (&yylval, YYLEX_PARAM)
#else
# define YYLEX yylex (&yylval)
#endif

/* Enable debugging if requested.  */
#if YYDEBUG

# ifndef YYFPRINTF
#  include <stdio.h> /* INFRINGES ON USER NAME SPACE */
#  define YYFPRINTF fprintf
# endif

# define YYDPRINTF(Args)			\
do {						\
  if (yydebug)					\
    YYFPRINTF Args;				\
} while (0)

# define YYDSYMPRINT(Args)			\
do {						\
  if (yydebug)					\
    yysymprint Args;				\
} while (0)

# define YYDSYMPRINTF(Title, Token, Value, Location)		\
do {								\
  if (yydebug)							\
    {								\
      YYFPRINTF (stderr, "%s ", Title);				\
      yysymprint (stderr, 					\
                  Token, Value);	\
      YYFPRINTF (stderr, "\n");					\
    }								\
} while (0)

/*------------------------------------------------------------------.
| yy_stack_print -- Print the state stack from its BOTTOM up to its |
| TOP (included).                                                   |
`------------------------------------------------------------------*/

#if defined (__STDC__) || defined (__cplusplus)
static void
yy_stack_print (short int *bottom, short int *top)
#else
static void
yy_stack_print (bottom, top)
    short int *bottom;
    short int *top;
#endif
{
  YYFPRINTF (stderr, "Stack now");
  for (/* Nothing. */; bottom <= top; ++bottom)
    YYFPRINTF (stderr, " %d", *bottom);
  YYFPRINTF (stderr, "\n");
}

# define YY_STACK_PRINT(Bottom, Top)				\
do {								\
  if (yydebug)							\
    yy_stack_print ((Bottom), (Top));				\
} while (0)


/*------------------------------------------------.
| Report that the YYRULE is going to be reduced.  |
`------------------------------------------------*/

#if defined (__STDC__) || defined (__cplusplus)
static void
yy_reduce_print (int yyrule)
#else
static void
yy_reduce_print (yyrule)
    int yyrule;
#endif
{
  int yyi;
  unsigned int yylno = yyrline[yyrule];
  YYFPRINTF (stderr, "Reducing stack by rule %d (line %u), ",
             yyrule - 1, yylno);
  /* Print the symbols being reduced, and their result.  */
  for (yyi = yyprhs[yyrule]; 0 <= yyrhs[yyi]; yyi++)
    YYFPRINTF (stderr, "%s ", yytname [yyrhs[yyi]]);
  YYFPRINTF (stderr, "-> %s\n", yytname [yyr1[yyrule]]);
}

# define YY_REDUCE_PRINT(Rule)		\
do {					\
  if (yydebug)				\
    yy_reduce_print (Rule);		\
} while (0)

/* Nonzero means print parse trace.  It is left uninitialized so that
   multiple parsers can coexist.  */
int yydebug;
#else /* !YYDEBUG */
# define YYDPRINTF(Args)
# define YYDSYMPRINT(Args)
# define YYDSYMPRINTF(Title, Token, Value, Location)
# define YY_STACK_PRINT(Bottom, Top)
# define YY_REDUCE_PRINT(Rule)
#endif /* !YYDEBUG */


/* YYINITDEPTH -- initial size of the parser's stacks.  */
#ifndef	YYINITDEPTH
# define YYINITDEPTH 200
#endif

/* YYMAXDEPTH -- maximum size the stacks can grow to (effective only
   if the built-in stack extension method is used).

   Do not make this value too large; the results are undefined if
   SIZE_MAX < YYSTACK_BYTES (YYMAXDEPTH)
   evaluated with infinite-precision integer arithmetic.  */

#if defined (YYMAXDEPTH) && YYMAXDEPTH == 0
# undef YYMAXDEPTH
#endif

#ifndef YYMAXDEPTH
# define YYMAXDEPTH 10000
#endif



#if YYERROR_VERBOSE

# ifndef yystrlen
#  if defined (__GLIBC__) && defined (_STRING_H)
#   define yystrlen strlen
#  else
/* Return the length of YYSTR.  */
static YYSIZE_T
#   if defined (__STDC__) || defined (__cplusplus)
yystrlen (const char *yystr)
#   else
yystrlen (yystr)
     const char *yystr;
#   endif
{
  register const char *yys = yystr;

  while (*yys++ != '\0')
    continue;

  return yys - yystr - 1;
}
#  endif
# endif

# ifndef yystpcpy
#  if defined (__GLIBC__) && defined (_STRING_H) && defined (_GNU_SOURCE)
#   define yystpcpy stpcpy
#  else
/* Copy YYSRC to YYDEST, returning the address of the terminating '\0' in
   YYDEST.  */
static char *
#   if defined (__STDC__) || defined (__cplusplus)
yystpcpy (char *yydest, const char *yysrc)
#   else
yystpcpy (yydest, yysrc)
     char *yydest;
     const char *yysrc;
#   endif
{
  register char *yyd = yydest;
  register const char *yys = yysrc;

  while ((*yyd++ = *yys++) != '\0')
    continue;

  return yyd - 1;
}
#  endif
# endif

#endif /* !YYERROR_VERBOSE */



#if YYDEBUG
/*--------------------------------.
| Print this symbol on YYOUTPUT.  |
`--------------------------------*/

#if defined (__STDC__) || defined (__cplusplus)
static void
yysymprint (FILE *yyoutput, int yytype, YYSTYPE *yyvaluep)
#else
static void
yysymprint (yyoutput, yytype, yyvaluep)
    FILE *yyoutput;
    int yytype;
    YYSTYPE *yyvaluep;
#endif
{
  /* Pacify ``unused variable'' warnings.  */
  (void) yyvaluep;

  if (yytype < YYNTOKENS)
    {
      YYFPRINTF (yyoutput, "token %s (", yytname[yytype]);
# ifdef YYPRINT
      YYPRINT (yyoutput, yytoknum[yytype], *yyvaluep);
# endif
    }
  else
    YYFPRINTF (yyoutput, "nterm %s (", yytname[yytype]);

  switch (yytype)
    {
      default:
        break;
    }
  YYFPRINTF (yyoutput, ")");
}

#endif /* ! YYDEBUG */
/*-----------------------------------------------.
| Release the memory associated to this symbol.  |
`-----------------------------------------------*/

#if defined (__STDC__) || defined (__cplusplus)
static void
yydestruct (int yytype, YYSTYPE *yyvaluep)
#else
static void
yydestruct (yytype, yyvaluep)
    int yytype;
    YYSTYPE *yyvaluep;
#endif
{
  /* Pacify ``unused variable'' warnings.  */
  (void) yyvaluep;

  switch (yytype)
    {

      default:
        break;
    }
}


/* Prevent warnings from -Wmissing-prototypes.  */

#ifdef YYPARSE_PARAM
# if defined (__STDC__) || defined (__cplusplus)
int yyparse (void *YYPARSE_PARAM);
# else
int yyparse ();
# endif
#else /* ! YYPARSE_PARAM */
#if defined (__STDC__) || defined (__cplusplus)
int yyparse (void);
#else
int yyparse ();
#endif
#endif /* ! YYPARSE_PARAM */






/*----------.
| yyparse.  |
`----------*/

#ifdef YYPARSE_PARAM
# if defined (__STDC__) || defined (__cplusplus)
int yyparse (void *YYPARSE_PARAM)
# else
int yyparse (YYPARSE_PARAM)
  void *YYPARSE_PARAM;
# endif
#else /* ! YYPARSE_PARAM */
#if defined (__STDC__) || defined (__cplusplus)
int
yyparse (void)
#else
int
yyparse ()

#endif
#endif
{
  /* The lookahead symbol.  */
int yychar;

/* The semantic value of the lookahead symbol.  */
YYSTYPE yylval;

/* Number of syntax errors so far.  */
int yynerrs;

  register int yystate;
  register int yyn;
  int yyresult;
  /* Number of tokens to shift before error messages enabled.  */
  int yyerrstatus;
  /* Lookahead token as an internal (translated) token number.  */
  int yytoken = 0;

  /* Three stacks and their tools:
     `yyss': related to states,
     `yyvs': related to semantic values,
     `yyls': related to locations.

     Refer to the stacks thru separate pointers, to allow yyoverflow
     to reallocate them elsewhere.  */

  /* The state stack.  */
  short int yyssa[YYINITDEPTH];
  short int *yyss = yyssa;
  register short int *yyssp;

  /* The semantic value stack.  */
  YYSTYPE yyvsa[YYINITDEPTH];
  YYSTYPE *yyvs = yyvsa;
  register YYSTYPE *yyvsp;



#define YYPOPSTACK   (yyvsp--, yyssp--)

  YYSIZE_T yystacksize = YYINITDEPTH;

  /* The variables used to return semantic value and location from the
     action routines.  */
  YYSTYPE yyval;


  /* When reducing, the number of symbols on the RHS of the reduced
     rule.  */
  int yylen;

  YYDPRINTF ((stderr, "Starting parse\n"));

  yystate = 0;
  yyerrstatus = 0;
  yynerrs = 0;
  yychar = YYEMPTY;		/* Cause a token to be read.  */

  /* Initialize stack pointers.
     Waste one element of value and location stack
     so that they stay on the same level as the state stack.
     The wasted elements are never initialized.  */

  yyssp = yyss;
  yyvsp = yyvs;


  goto yysetstate;

/*------------------------------------------------------------.
| yynewstate -- Push a new state, which is found in yystate.  |
`------------------------------------------------------------*/
 yynewstate:
  /* In all cases, when you get here, the value and location stacks
     have just been pushed. so pushing a state here evens the stacks.
     */
  yyssp++;

 yysetstate:
  *yyssp = yystate;

  if (yyss + yystacksize - 1 <= yyssp)
    {
      /* Get the current used size of the three stacks, in elements.  */
      YYSIZE_T yysize = yyssp - yyss + 1;

#ifdef yyoverflow
      {
	/* Give user a chance to reallocate the stack. Use copies of
	   these so that the &'s don't force the real ones into
	   memory.  */
	YYSTYPE *yyvs1 = yyvs;
	short int *yyss1 = yyss;


	/* Each stack pointer address is followed by the size of the
	   data in use in that stack, in bytes.  This used to be a
	   conditional around just the two extra args, but that might
	   be undefined if yyoverflow is a macro.  */
	yyoverflow ("parser stack overflow",
		    &yyss1, yysize * sizeof (*yyssp),
		    &yyvs1, yysize * sizeof (*yyvsp),

		    &yystacksize);

	yyss = yyss1;
	yyvs = yyvs1;
      }
#else /* no yyoverflow */
# ifndef YYSTACK_RELOCATE
      goto yyoverflowlab;
# else
      /* Extend the stack our own way.  */
      if (YYMAXDEPTH <= yystacksize)
	goto yyoverflowlab;
      yystacksize *= 2;
      if (YYMAXDEPTH < yystacksize)
	yystacksize = YYMAXDEPTH;

      {
	short int *yyss1 = yyss;
	union yyalloc *yyptr =
	  (union yyalloc *) YYSTACK_ALLOC (YYSTACK_BYTES (yystacksize));
	if (! yyptr)
	  goto yyoverflowlab;
	YYSTACK_RELOCATE (yyss);
	YYSTACK_RELOCATE (yyvs);

#  undef YYSTACK_RELOCATE
	if (yyss1 != yyssa)
	  YYSTACK_FREE (yyss1);
      }
# endif
#endif /* no yyoverflow */

      yyssp = yyss + yysize - 1;
      yyvsp = yyvs + yysize - 1;


      YYDPRINTF ((stderr, "Stack size increased to %lu\n",
		  (unsigned long int) yystacksize));

      if (yyss + yystacksize - 1 <= yyssp)
	YYABORT;
    }

  YYDPRINTF ((stderr, "Entering state %d\n", yystate));

  goto yybackup;

/*-----------.
| yybackup.  |
`-----------*/
yybackup:

/* Do appropriate processing given the current state.  */
/* Read a lookahead token if we need one and don't already have one.  */
/* yyresume: */

  /* First try to decide what to do without reference to lookahead token.  */

  yyn = yypact[yystate];
  if (yyn == YYPACT_NINF)
    goto yydefault;

  /* Not known => get a lookahead token if don't already have one.  */

  /* YYCHAR is either YYEMPTY or YYEOF or a valid lookahead symbol.  */
  if (yychar == YYEMPTY)
    {
      YYDPRINTF ((stderr, "Reading a token: "));
      yychar = YYLEX;
    }

  if (yychar <= YYEOF)
    {
      yychar = yytoken = YYEOF;
      YYDPRINTF ((stderr, "Now at end of input.\n"));
    }
  else
    {
      yytoken = YYTRANSLATE (yychar);
      YYDSYMPRINTF ("Next token is", yytoken, &yylval, &yylloc);
    }

  /* If the proper action on seeing token YYTOKEN is to reduce or to
     detect an error, take that action.  */
  yyn += yytoken;
  if (yyn < 0 || YYLAST < yyn || yycheck[yyn] != yytoken)
    goto yydefault;
  yyn = yytable[yyn];
  if (yyn <= 0)
    {
      if (yyn == 0 || yyn == YYTABLE_NINF)
	goto yyerrlab;
      yyn = -yyn;
      goto yyreduce;
    }

  if (yyn == YYFINAL)
    YYACCEPT;

  /* Shift the lookahead token.  */
  YYDPRINTF ((stderr, "Shifting token %s, ", yytname[yytoken]));

  /* Discard the token being shifted unless it is eof.  */
  if (yychar != YYEOF)
    yychar = YYEMPTY;

  *++yyvsp = yylval;


  /* Count tokens shifted since error; after three, turn off error
     status.  */
  if (yyerrstatus)
    yyerrstatus--;

  yystate = yyn;
  goto yynewstate;


/*-----------------------------------------------------------.
| yydefault -- do the default action for the current state.  |
`-----------------------------------------------------------*/
yydefault:
  yyn = yydefact[yystate];
  if (yyn == 0)
    goto yyerrlab;
  goto yyreduce;


/*-----------------------------.
| yyreduce -- Do a reduction.  |
`-----------------------------*/
yyreduce:
  /* yyn is the number of a rule to reduce with.  */
  yylen = yyr2[yyn];

  /* If YYLEN is nonzero, implement the default value of the action:
     `$$ = $1'.

     Otherwise, the following line sets YYVAL to garbage.
     This behavior is undocumented and Bison
     users should not rely upon it.  Assigning to YYVAL
     unconditionally makes the parser a bit smaller, and it avoids a
     GCC warning that YYVAL may be used uninitialized.  */
  yyval = yyvsp[1-yylen];


  YY_REDUCE_PRINT (yyn);
  switch (yyn)
    {
        case 2:
#line 666 "parser-sming.y"
    {
			    /*
			     * Return the number of successfully
			     * parsed modules.
			     */
			    yyval.rc = yyvsp[0].rc;
			;}
    break;

  case 3:
#line 676 "parser-sming.y"
    {
			    yyval.rc = 0;
			;}
    break;

  case 4:
#line 680 "parser-sming.y"
    {
			    yyval.rc = yyvsp[0].rc;
			;}
    break;

  case 5:
#line 686 "parser-sming.y"
    {
			    yyval.rc = yyvsp[0].rc;
			;}
    break;

  case 6:
#line 690 "parser-sming.y"
    {
			    /*
			     * Sum up the number of successfully parsed
			     * modules or return -1, if at least one
			     * module failed.
			     */
			    if ((yyvsp[-1].rc >= 0) && (yyvsp[0].rc >= 0)) {
				yyval.rc = yyvsp[-1].rc + yyvsp[0].rc;
			    } else {
				yyval.rc = -1;
			    }
			;}
    break;

  case 7:
#line 705 "parser-sming.y"
    {
			    /*
			     * If we got a (Module *) return rc == 1,
			     * otherwise parsing failed (rc == -1).
			     */
			    if (yyvsp[-1].modulePtr != NULL) {
				yyval.rc = 1;
			    } else {
				yyval.rc = -1;
			    }
			;}
    break;

  case 8:
#line 719 "parser-sming.y"
    {
			    thisParserPtr->modulePtr = findModuleByName(yyvsp[0].text);
			    if (!thisParserPtr->modulePtr) {
				thisParserPtr->modulePtr =
				    addModule(yyvsp[0].text,
					      smiStrdup(thisParserPtr->path),
					      0,
					      thisParserPtr);
			    } else {
			        smiPrintError(thisParserPtr,
					      ERR_MODULE_ALREADY_LOADED,
					      yyvsp[0].text);
				free(yyvsp[0].text);
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
			;}
    break;

  case 9:
#line 749 "parser-sming.y"
    {
			    if (yyvsp[-1].text) {
				setModuleOrganization(thisParserPtr->modulePtr,
						      yyvsp[-1].text);
			    }
			;}
    break;

  case 10:
#line 756 "parser-sming.y"
    {
			    if (yyvsp[-1].text) {
				setModuleContactInfo(thisParserPtr->modulePtr,
						     yyvsp[-1].text);
			    }
			;}
    break;

  case 11:
#line 763 "parser-sming.y"
    {
			    if (yyvsp[-1].text) {
				setModuleDescription(thisParserPtr->modulePtr,
						     yyvsp[-1].text, thisParserPtr);
				if (moduleObjectPtr) {
				    setObjectDescription(moduleObjectPtr, yyvsp[-1].text,
							 thisParserPtr);
				}
			    }
			;}
    break;

  case 12:
#line 774 "parser-sming.y"
    {
			    if (yyvsp[0].text) {
				setModuleReference(thisParserPtr->modulePtr,
						   yyvsp[0].text, thisParserPtr);
				if (moduleObjectPtr) {
				    setObjectReference(moduleObjectPtr, yyvsp[0].text,
						       thisParserPtr);
				}
			    }
			;}
    break;

  case 13:
#line 793 "parser-sming.y"
    {
			    List *listPtr;
			    Object *objectPtr;
			    
			    /*
			     * Walk through the index structs of all table
			     * rows of this module and convert their
			     * labelstrings to (Object *). This is the
			     * case for index column lists
			     * (indexPtr->listPtr[]->ptr), index related
			     * rows (indexPtr->rowPtr) and create lists
			     * (listPtr[]->ptr).
			     */
			    while (thisParserPtr->firstIndexlabelPtr) {
				/* adjust indexPtr->listPtr elements */
				for (listPtr =
					 ((Object *)(thisParserPtr->
						     firstIndexlabelPtr->
						     ptr))->listPtr;
				     listPtr; listPtr = listPtr->nextPtr) {
				    objectPtr = findObject(listPtr->ptr,
							   thisParserPtr,
							   thisModulePtr);
				    listPtr->ptr = objectPtr;
				}
				/* adjust relatedPtr */
				if (((Object *)
				    (thisParserPtr->firstIndexlabelPtr->ptr))->
				    relatedPtr) {
				    objectPtr = findObject(
					((Object *)(thisParserPtr->
						    firstIndexlabelPtr->ptr))->
					relatedPtr,
					thisParserPtr,
			                thisModulePtr);
				    ((Object *)(thisParserPtr->
						firstIndexlabelPtr->ptr))->
					relatedPtr = objectPtr;
				}
				listPtr =
				    thisParserPtr->firstIndexlabelPtr->nextPtr;
				free(thisParserPtr->firstIndexlabelPtr);
				thisParserPtr->firstIndexlabelPtr = listPtr;
			    }

			    /*
			     * Is there a node that matches the `identity'
			     * statement?
			     */
			    
			    checkObjects(thisParserPtr, thisModulePtr);
			    checkTypes(thisParserPtr, thisModulePtr);
			    
			    yyval.modulePtr = thisModulePtr;
			    moduleObjectPtr = NULL;
			;}
    break;

  case 14:
#line 852 "parser-sming.y"
    {
			    yyval.rc = 0;
			;}
    break;

  case 15:
#line 856 "parser-sming.y"
    {
			    /*
			     * Return the number of successfully
			     * parsed extension statements.
			     */
			    yyval.rc = yyvsp[0].rc;
			;}
    break;

  case 16:
#line 866 "parser-sming.y"
    {
			    yyval.rc = yyvsp[0].rc;
			;}
    break;

  case 17:
#line 871 "parser-sming.y"
    {
			    /*
			     * Sum up the number of successfully parsed
			     * extensions or return -1, if at least one
			     * failed.
			     */
			    if ((yyvsp[-1].rc >= 0) && (yyvsp[0].rc >= 0)) {
				yyval.rc = yyvsp[-1].rc + yyvsp[0].rc;
			    } else {
				yyval.rc = -1;
			    }
			;}
    break;

  case 18:
#line 886 "parser-sming.y"
    {
			    /*
			     * If we got a (Type *) return rc == 1,
			     * otherwise parsing failed (rc == -1).
			     */
			    if (yyvsp[-1].macroPtr) {
				yyval.rc = 1;
			    } else {
				yyval.rc = -1;
			    }
			;}
    break;

  case 19:
#line 900 "parser-sming.y"
    {
			    macroIdentifier = yyvsp[0].text;
			    macroPtr = addMacro(macroIdentifier,
						0,
						thisParserPtr);
			    setMacroDecl(macroPtr, SMI_DECL_EXTENSION);
			;}
    break;

  case 20:
#line 909 "parser-sming.y"
    {
			    if (macroPtr && yyvsp[0].status) {
				setMacroStatus(macroPtr, yyvsp[0].status);
			    }
			;}
    break;

  case 21:
#line 915 "parser-sming.y"
    {
			    if (macroPtr && yyvsp[0].text) {
				setMacroDescription(macroPtr, yyvsp[0].text,
						    thisParserPtr);
			    }
			;}
    break;

  case 22:
#line 922 "parser-sming.y"
    {
			    if (macroPtr && yyvsp[0].text) {
				setMacroReference(macroPtr, yyvsp[0].text,
						  thisParserPtr);
			    }
			;}
    break;

  case 23:
#line 929 "parser-sming.y"
    {
			;}
    break;

  case 24:
#line 932 "parser-sming.y"
    {
			    yyval.macroPtr = 0;
			    macroPtr = NULL;
			    free(macroIdentifier);
			;}
    break;

  case 25:
#line 940 "parser-sming.y"
    {
			    yyval.rc = 0;
			;}
    break;

  case 26:
#line 944 "parser-sming.y"
    {
			    /*
			     * Return the number of successfully
			     * parsed typedef statements.
			     */
			    yyval.rc = yyvsp[0].rc;
			;}
    break;

  case 27:
#line 954 "parser-sming.y"
    {
			    yyval.rc = yyvsp[0].rc;
			;}
    break;

  case 28:
#line 959 "parser-sming.y"
    {
			    /*
			     * Sum up the number of successfully parsed
			     * typedefs or return -1, if at least one
			     * failed.
			     */
			    if ((yyvsp[-1].rc >= 0) && (yyvsp[0].rc >= 0)) {
				yyval.rc = yyvsp[-1].rc + yyvsp[0].rc;
			    } else {
				yyval.rc = -1;
			    }
			;}
    break;

  case 29:
#line 974 "parser-sming.y"
    {
			    /*
			     * If we got a (Type *) return rc == 1,
			     * otherwise parsing failed (rc == -1).
			     */
			    if (yyvsp[-1].typePtr) {
				yyval.rc = 1;
			    } else {
				yyval.rc = -1;
			    }
			;}
    break;

  case 30:
#line 988 "parser-sming.y"
    {
			    typeIdentifier = yyvsp[0].text;
			;}
    break;

  case 31:
#line 993 "parser-sming.y"
    {
			    if (yyvsp[-1].typePtr) {
				if (yyvsp[-1].typePtr->export.name) {
				    /*
				     * If the exact type has been found
				     * and no new Type structure has been
				     * created, we have to create a
				     * duplicate now.
				     */
				    typePtr = duplicateType(yyvsp[-1].typePtr, 0,
							    thisParserPtr);
				} else {
				    typePtr = yyvsp[-1].typePtr;
				}
				typePtr = setTypeName(typePtr, typeIdentifier);
				setTypeDecl(typePtr, SMI_DECL_TYPEDEF);
			    }
			;}
    break;

  case 32:
#line 1012 "parser-sming.y"
    {
			    if (typePtr && yyvsp[0].valuePtr) {
				setTypeValue(typePtr, yyvsp[0].valuePtr);
			    }
			;}
    break;

  case 33:
#line 1018 "parser-sming.y"
    {
			    if (typePtr && yyvsp[0].text
				&& smiCheckFormat(thisParserPtr,
						  typePtr->export.basetype,
						  yyvsp[0].text, 0)) {
				setTypeFormat(typePtr, yyvsp[0].text);
			    }
			;}
    break;

  case 34:
#line 1027 "parser-sming.y"
    {
			    if (typePtr && yyvsp[0].text) {
				setTypeUnits(typePtr, yyvsp[0].text);
			    }
			;}
    break;

  case 35:
#line 1033 "parser-sming.y"
    {
			    if (typePtr && yyvsp[0].status) {
				setTypeStatus(typePtr, yyvsp[0].status);
			    }
			;}
    break;

  case 36:
#line 1039 "parser-sming.y"
    {
			    if (typePtr && yyvsp[0].text) {
				setTypeDescription(typePtr, yyvsp[0].text,
						   thisParserPtr);
			    }
			;}
    break;

  case 37:
#line 1046 "parser-sming.y"
    {
			    if (typePtr && yyvsp[0].text) {
				setTypeReference(typePtr, yyvsp[0].text, thisParserPtr);
			    }
			;}
    break;

  case 38:
#line 1052 "parser-sming.y"
    {
			    yyval.typePtr = 0;
			    typePtr = NULL;
			    free(typeIdentifier);
			    defaultBasetype = SMI_BASETYPE_UNKNOWN;
			;}
    break;

  case 39:
#line 1061 "parser-sming.y"
    {
			    yyval.rc = 0;
			;}
    break;

  case 40:
#line 1065 "parser-sming.y"
    {
			    /*
			     * Return the number of successfully
			     * parsed object declaring statements.
			     */
			    yyval.rc = yyvsp[0].rc;
			;}
    break;

  case 41:
#line 1075 "parser-sming.y"
    {
			    yyval.rc = yyvsp[0].rc;
			;}
    break;

  case 42:
#line 1080 "parser-sming.y"
    {
			    /*
			     * Sum up the number of successfully parsed
			     * statements or return -1, if at least one
			     * module failed.
			     */
			    if ((yyvsp[-1].rc >= 0) && (yyvsp[0].rc >= 0)) {
				yyval.rc = yyvsp[-1].rc + yyvsp[0].rc;
			    } else {
				yyval.rc = -1;
			    }
			;}
    break;

  case 43:
#line 1095 "parser-sming.y"
    {
			    /*
			     * If we got an (Object *) return rc == 1,
			     * otherwise parsing failed (rc == -1).
			     */
			    if (yyvsp[-1].objectPtr) {
				yyval.rc = 1;
			    } else {
				yyval.rc = -1;
			    }
			;}
    break;

  case 47:
#line 1114 "parser-sming.y"
    {
			    nodeIdentifier = yyvsp[0].text;
			;}
    break;

  case 48:
#line 1119 "parser-sming.y"
    {
			    if (yyvsp[-1].nodePtr) {
				nodeObjectPtr = addObject(nodeIdentifier,
							  yyvsp[-1].nodePtr->parentPtr,
							  yyvsp[-1].nodePtr->subid,
							  0, thisParserPtr);
				setObjectDecl(nodeObjectPtr, SMI_DECL_NODE);
				setObjectNodekind(nodeObjectPtr,
						  SMI_NODEKIND_NODE);
				setObjectAccess(nodeObjectPtr,
						SMI_ACCESS_NOT_ACCESSIBLE);
			    }
			;}
    break;

  case 49:
#line 1133 "parser-sming.y"
    {
			    if (nodeObjectPtr) {
				setObjectStatus(nodeObjectPtr, yyvsp[0].status);
			    }
			;}
    break;

  case 50:
#line 1139 "parser-sming.y"
    {
			    if (nodeObjectPtr && yyvsp[0].text) {
				setObjectDescription(nodeObjectPtr, yyvsp[0].text,
						     thisParserPtr);
				/*
				 * If the node has a description, it gets
				 * registered. This is used to distinguish
				 * between SMIv2 OBJECT-IDENTITY macros and
				 * non-registering ASN.1 value assignments.
				 */
				addObjectFlags(nodeObjectPtr, FLAG_REGISTERED);
			    }
			;}
    break;

  case 51:
#line 1153 "parser-sming.y"
    {
			    if (nodeObjectPtr && yyvsp[0].text) {
				setObjectReference(nodeObjectPtr, yyvsp[0].text,
						   thisParserPtr);
			    }
			;}
    break;

  case 52:
#line 1160 "parser-sming.y"
    {
			    if (thisParserPtr->identityObjectName &&
				!strcmp(thisParserPtr->identityObjectName,
					nodeIdentifier)) {
				setModuleIdentityObject(
				    thisParserPtr->modulePtr, nodeObjectPtr);
				thisParserPtr->identityObjectName = NULL;
			    }
			    
			    yyval.objectPtr = nodeObjectPtr;
			    nodeObjectPtr = NULL;
			    free(nodeIdentifier);
			;}
    break;

  case 53:
#line 1176 "parser-sming.y"
    {
			    scalarIdentifier = yyvsp[0].text;
			;}
    break;

  case 54:
#line 1181 "parser-sming.y"
    {
			    if (yyvsp[-1].nodePtr) {
				scalarObjectPtr = addObject(scalarIdentifier,
							    yyvsp[-1].nodePtr->parentPtr,
							    yyvsp[-1].nodePtr->subid,
							    0, thisParserPtr);
				setObjectDecl(scalarObjectPtr,
					      SMI_DECL_SCALAR);
				setObjectNodekind(scalarObjectPtr,
						  SMI_NODEKIND_SCALAR);
			    }
			;}
    break;

  case 55:
#line 1194 "parser-sming.y"
    {
			    if (scalarObjectPtr && yyvsp[-1].typePtr) {
				setObjectType(scalarObjectPtr, yyvsp[-1].typePtr);
				defaultBasetype = yyvsp[-1].typePtr->export.basetype;
				if (!(yyvsp[-1].typePtr->export.name)) {
				    /*
				     * An inlined type.
				     */
#if 0 /* export implicitly defined types by the node's lowercase name */
				    setTypeName(yyvsp[-1].typePtr, scalarIdentifier);
#endif
				}
			    }
			;}
    break;

  case 56:
#line 1209 "parser-sming.y"
    {
			    if (scalarObjectPtr) {
				setObjectAccess(scalarObjectPtr, yyvsp[-1].access);
			    }
			;}
    break;

  case 57:
#line 1215 "parser-sming.y"
    {
			    if (scalarObjectPtr && yyvsp[0].valuePtr) {
				setObjectValue(scalarObjectPtr, yyvsp[0].valuePtr);
			    }
			;}
    break;

  case 58:
#line 1221 "parser-sming.y"
    {
			    if (scalarObjectPtr && yyvsp[0].text
				&& smiCheckFormat(thisParserPtr,
						  yyvsp[-8].typePtr->export.basetype,
						  yyvsp[0].text, 0)) {
				setObjectFormat(scalarObjectPtr, yyvsp[0].text);
			    }
			;}
    break;

  case 59:
#line 1230 "parser-sming.y"
    {
			    if (scalarObjectPtr && yyvsp[0].text) {
				setObjectUnits(scalarObjectPtr, yyvsp[0].text);
			    }
			;}
    break;

  case 60:
#line 1236 "parser-sming.y"
    {
			    if (scalarObjectPtr) {
				setObjectStatus(scalarObjectPtr, yyvsp[0].status);
			    }
			;}
    break;

  case 61:
#line 1242 "parser-sming.y"
    {
			    if (scalarObjectPtr && yyvsp[-1].text) {
				setObjectDescription(scalarObjectPtr, yyvsp[-1].text,
						     thisParserPtr);
			    }
			;}
    break;

  case 62:
#line 1249 "parser-sming.y"
    {
			    if (scalarObjectPtr && yyvsp[0].text) {
				setObjectReference(scalarObjectPtr, yyvsp[0].text,
						   thisParserPtr);
			    }
			;}
    break;

  case 63:
#line 1256 "parser-sming.y"
    {
			    yyval.objectPtr = scalarObjectPtr;
			    scalarObjectPtr = NULL;
			    free(scalarIdentifier);
			    defaultBasetype = SMI_BASETYPE_UNKNOWN;
			;}
    break;

  case 64:
#line 1265 "parser-sming.y"
    {
			    tableIdentifier = yyvsp[0].text;
			;}
    break;

  case 65:
#line 1270 "parser-sming.y"
    {
			    if (yyvsp[-1].nodePtr) {
				tableObjectPtr = addObject(tableIdentifier,
							   yyvsp[-1].nodePtr->parentPtr,
							   yyvsp[-1].nodePtr->subid,
							   0, thisParserPtr);
				setObjectDecl(tableObjectPtr,
					      SMI_DECL_TABLE);
				setObjectNodekind(tableObjectPtr,
						  SMI_NODEKIND_TABLE);
				setObjectAccess(tableObjectPtr,
						SMI_ACCESS_NOT_ACCESSIBLE);
			    }
			;}
    break;

  case 66:
#line 1285 "parser-sming.y"
    {
			    if (tableObjectPtr) {
				setObjectStatus(tableObjectPtr, yyvsp[0].status);
			    }
			;}
    break;

  case 67:
#line 1291 "parser-sming.y"
    {
			    if (tableObjectPtr && yyvsp[-1].text) {
				setObjectDescription(tableObjectPtr, yyvsp[-1].text,
						     thisParserPtr);
			    }
			;}
    break;

  case 68:
#line 1298 "parser-sming.y"
    {
			    if (tableObjectPtr && yyvsp[0].text) {
				setObjectReference(tableObjectPtr, yyvsp[0].text,
						   thisParserPtr);
			    }
			;}
    break;

  case 69:
#line 1306 "parser-sming.y"
    {
			    yyval.objectPtr = tableObjectPtr;
			    tableObjectPtr = NULL;
			    free(tableIdentifier);
			;}
    break;

  case 70:
#line 1314 "parser-sming.y"
    {
			    rowIdentifier = yyvsp[0].text;
			;}
    break;

  case 71:
#line 1319 "parser-sming.y"
    {
			    if (yyvsp[-1].nodePtr) {
				rowObjectPtr = addObject(rowIdentifier,
							 yyvsp[-1].nodePtr->parentPtr,
							 yyvsp[-1].nodePtr->subid,
							 0, thisParserPtr);
				setObjectDecl(rowObjectPtr,
					      SMI_DECL_ROW);
				setObjectNodekind(rowObjectPtr,
						  SMI_NODEKIND_ROW);
				setObjectAccess(rowObjectPtr,
						SMI_ACCESS_NOT_ACCESSIBLE);
			    }
			;}
    break;

  case 72:
#line 1334 "parser-sming.y"
    {
			    List *listPtr;
			    
			    if (rowObjectPtr &&
				(yyvsp[-1].index.indexkind != SMI_INDEX_UNKNOWN)) {
				setObjectIndexkind(rowObjectPtr,
						   yyvsp[-1].index.indexkind);
				setObjectImplied(rowObjectPtr, yyvsp[-1].index.implied);
				setObjectRelated(rowObjectPtr, yyvsp[-1].index.rowPtr);
				setObjectList(rowObjectPtr, yyvsp[-1].index.listPtr);

				/*
				 * Add this row object to the list of rows
				 * that have to be converted when the whole
				 * module has been parsed. See the end of
				 * the moduleStatement rule above.
				 */
				listPtr = smiMalloc(sizeof(List));
				listPtr->ptr = rowObjectPtr;
			  listPtr->nextPtr = thisParserPtr->firstIndexlabelPtr;
				thisParserPtr->firstIndexlabelPtr = listPtr;
			    }
			;}
    break;

  case 73:
#line 1358 "parser-sming.y"
    {
			    if (rowObjectPtr) {
				if (yyvsp[0].rc) {
				    addObjectFlags(rowObjectPtr,
						   FLAG_CREATABLE);
				    setObjectCreate(rowObjectPtr, 1);
				}
			    }
			;}
    break;

  case 74:
#line 1368 "parser-sming.y"
    {
			    if (rowObjectPtr) {
				setObjectStatus(rowObjectPtr, yyvsp[0].status);
			    }
			;}
    break;

  case 75:
#line 1374 "parser-sming.y"
    {
			    if (rowObjectPtr && yyvsp[-1].text) {
				setObjectDescription(rowObjectPtr, yyvsp[-1].text,
						     thisParserPtr);
			    }
			;}
    break;

  case 76:
#line 1381 "parser-sming.y"
    {
			    if (rowObjectPtr && yyvsp[0].text) {
				setObjectReference(rowObjectPtr, yyvsp[0].text,
						   thisParserPtr);
			    }
			;}
    break;

  case 77:
#line 1389 "parser-sming.y"
    {
			    yyval.objectPtr = rowObjectPtr;
			    rowObjectPtr = NULL;
			    free(rowIdentifier);
			;}
    break;

  case 78:
#line 1397 "parser-sming.y"
    {
			    /*
			     * Return the number of successfully
			     * parsed column statements.
			     */
			    yyval.rc = yyvsp[0].rc;
			;}
    break;

  case 79:
#line 1405 "parser-sming.y"
    {
			    /*
			     * Sum up the number of successfully parsed
			     * columns or return -1, if at least one
			     * module failed.
			     */
			    if ((yyvsp[-1].rc >= 0) && (yyvsp[0].rc >= 0)) {
				yyval.rc = yyvsp[-1].rc + yyvsp[0].rc;
			    } else {
				yyval.rc = -1;
			    }
			;}
    break;

  case 80:
#line 1420 "parser-sming.y"
    {
			    /*
			     * If we got an (Object *) return rc == 1,
			     * otherwise parsing failed (rc == -1).
			     */
			    if (yyvsp[-1].objectPtr) {
				yyval.rc = 1;
			    } else {
				yyval.rc = -1;
			    }
			;}
    break;

  case 81:
#line 1434 "parser-sming.y"
    {
			    columnIdentifier = yyvsp[0].text;
			;}
    break;

  case 82:
#line 1439 "parser-sming.y"
    {
			    if (yyvsp[-1].nodePtr) {
				columnObjectPtr = addObject(columnIdentifier,
							    yyvsp[-1].nodePtr->parentPtr,
							    yyvsp[-1].nodePtr->subid,
							    0, thisParserPtr);
				setObjectDecl(columnObjectPtr,
					      SMI_DECL_COLUMN);
				setObjectNodekind(columnObjectPtr,
						  SMI_NODEKIND_COLUMN);
			    }
			;}
    break;

  case 83:
#line 1452 "parser-sming.y"
    {
			    if (columnObjectPtr && yyvsp[-1].typePtr) {
				setObjectType(columnObjectPtr, yyvsp[-1].typePtr);
				defaultBasetype = yyvsp[-1].typePtr->export.basetype;
				if (!(yyvsp[-1].typePtr->export.name)) {
				    /*
				     * An inlined type.
				     */
#if 0 /* export implicitly defined types by the node's lowercase name */
				    setTypeName(yyvsp[-1].typePtr, columnIdentifier);
#endif
				}
			    }
			;}
    break;

  case 84:
#line 1467 "parser-sming.y"
    {
			    if (columnObjectPtr) {
				setObjectAccess(columnObjectPtr, yyvsp[-1].access);
			    }
			;}
    break;

  case 85:
#line 1473 "parser-sming.y"
    {
			    if (columnObjectPtr && yyvsp[0].valuePtr) {
				setObjectValue(columnObjectPtr, yyvsp[0].valuePtr);
			    }
			;}
    break;

  case 86:
#line 1479 "parser-sming.y"
    {
			    if (columnObjectPtr && yyvsp[0].text
                                && smiCheckFormat(thisParserPtr,
						  yyvsp[-8].typePtr->export.basetype,
						  yyvsp[0].text, 0)) {
				setObjectFormat(columnObjectPtr, yyvsp[0].text);
			    }
			;}
    break;

  case 87:
#line 1488 "parser-sming.y"
    {
			    if (columnObjectPtr && yyvsp[0].text) {
				setObjectUnits(columnObjectPtr, yyvsp[0].text);
			    }
			;}
    break;

  case 88:
#line 1494 "parser-sming.y"
    {
			    if (columnObjectPtr) {
				setObjectStatus(columnObjectPtr, yyvsp[0].status);
			    }
			;}
    break;

  case 89:
#line 1500 "parser-sming.y"
    {
			    if (columnObjectPtr && yyvsp[-1].text) {
				setObjectDescription(columnObjectPtr, yyvsp[-1].text,
						     thisParserPtr);
			    }
			;}
    break;

  case 90:
#line 1507 "parser-sming.y"
    {
			    if (columnObjectPtr && yyvsp[0].text) {
				setObjectReference(columnObjectPtr, yyvsp[0].text,
						   thisParserPtr);
			    }
			;}
    break;

  case 91:
#line 1514 "parser-sming.y"
    {
			    yyval.objectPtr = columnObjectPtr;
			    columnObjectPtr = NULL;
			    free(columnIdentifier);
			    defaultBasetype = SMI_BASETYPE_UNKNOWN;
			;}
    break;

  case 92:
#line 1523 "parser-sming.y"
    {
			    yyval.rc = 0;
			;}
    break;

  case 93:
#line 1527 "parser-sming.y"
    {
			    /*
			     * Return the number of successfully
			     * parsed notification statements.
			     */
			    yyval.rc = yyvsp[0].rc;
			;}
    break;

  case 94:
#line 1537 "parser-sming.y"
    {
			    yyval.rc = yyvsp[0].rc;
			;}
    break;

  case 95:
#line 1542 "parser-sming.y"
    {
			    /*
			     * Sum up the number of successfully parsed
			     * notifications or return -1, if at least one
			     * module failed.
			     */
			    if ((yyvsp[-1].rc >= 0) && (yyvsp[0].rc >= 0)) {
				yyval.rc = yyvsp[-1].rc + yyvsp[0].rc;
			    } else {
				yyval.rc = -1;
			    }
			;}
    break;

  case 96:
#line 1557 "parser-sming.y"
    {
			    /*
			     * If we got an (Object *) return rc == 1,
			     * otherwise parsing failed (rc == -1).
			     */
			    if (yyvsp[-1].objectPtr) {
				yyval.rc = 1;
			    } else {
				yyval.rc = -1;
			    }
			;}
    break;

  case 97:
#line 1571 "parser-sming.y"
    {
			    notificationIdentifier = yyvsp[0].text;
			;}
    break;

  case 98:
#line 1576 "parser-sming.y"
    {
			    if (yyvsp[-1].nodePtr) {
				notificationObjectPtr =
				    addObject(notificationIdentifier,
					      yyvsp[-1].nodePtr->parentPtr,
					      yyvsp[-1].nodePtr->subid,
					      0, thisParserPtr);
				setObjectDecl(notificationObjectPtr,
					      SMI_DECL_NOTIFICATION);
				setObjectNodekind(notificationObjectPtr,
						  SMI_NODEKIND_NOTIFICATION);
			    }
			;}
    break;

  case 99:
#line 1590 "parser-sming.y"
    {
			    List *listPtr;
			    Object *objectPtr;
			    
			    if (notificationObjectPtr && yyvsp[0].listPtr) {
				for (listPtr = yyvsp[0].listPtr; listPtr;
				     listPtr = listPtr->nextPtr) {
				    objectPtr = findObject(listPtr->ptr,
							   thisParserPtr,
							   thisModulePtr);
				    listPtr->ptr = objectPtr;
				}
				setObjectList(notificationObjectPtr, yyvsp[0].listPtr);
			    }
			;}
    break;

  case 100:
#line 1606 "parser-sming.y"
    {
			    if (notificationObjectPtr) {
				setObjectStatus(notificationObjectPtr, yyvsp[0].status);
			    }
			;}
    break;

  case 101:
#line 1612 "parser-sming.y"
    {
			    if (notificationObjectPtr && yyvsp[-1].text) {
				setObjectDescription(notificationObjectPtr,
						     yyvsp[-1].text, thisParserPtr);
			    }
			;}
    break;

  case 102:
#line 1619 "parser-sming.y"
    {
			    if (notificationObjectPtr && yyvsp[0].text) {
				setObjectReference(notificationObjectPtr, yyvsp[0].text,
						   thisParserPtr);
			    }
			;}
    break;

  case 103:
#line 1626 "parser-sming.y"
    {
			    yyval.objectPtr = notificationObjectPtr;
			    notificationObjectPtr = NULL;
			    free(notificationIdentifier);
			;}
    break;

  case 104:
#line 1634 "parser-sming.y"
    {
			    yyval.rc = 0;
			;}
    break;

  case 105:
#line 1638 "parser-sming.y"
    {
			    /*
			     * Return the number of successfully
			     * parsed group statements.
			     */
			    yyval.rc = yyvsp[0].rc;
			;}
    break;

  case 106:
#line 1648 "parser-sming.y"
    {
			    yyval.rc = yyvsp[0].rc;
			;}
    break;

  case 107:
#line 1652 "parser-sming.y"
    {
			    /*
			     * Sum up the number of successfully parsed
			     * groups or return -1, if at least one
			     * module failed.
			     */
			    if ((yyvsp[-1].rc >= 0) && (yyvsp[0].rc >= 0)) {
				yyval.rc = yyvsp[-1].rc + yyvsp[0].rc;
			    } else {
				yyval.rc = -1;
			    }
			;}
    break;

  case 108:
#line 1667 "parser-sming.y"
    {
			    /*
			     * If we got an (Object *) return rc == 1,
			     * otherwise parsing failed (rc == -1).
			     */
			    if (yyvsp[-1].objectPtr) {
				yyval.rc = 1;
			    } else {
				yyval.rc = -1;
			    }
			;}
    break;

  case 109:
#line 1681 "parser-sming.y"
    {
			    groupIdentifier = yyvsp[0].text;
			;}
    break;

  case 110:
#line 1686 "parser-sming.y"
    {
			    if (yyvsp[-1].nodePtr) {
				groupObjectPtr = addObject(groupIdentifier,
							   yyvsp[-1].nodePtr->parentPtr,
							   yyvsp[-1].nodePtr->subid,
							   0, thisParserPtr);
				setObjectDecl(groupObjectPtr, SMI_DECL_GROUP);
				setObjectNodekind(groupObjectPtr,
						  SMI_NODEKIND_GROUP);
			    }
			;}
    break;

  case 111:
#line 1698 "parser-sming.y"
    {
			    List *listPtr;
			    Object *objectPtr;
			    
			    if (groupObjectPtr && yyvsp[-1].listPtr) {
				for (listPtr = yyvsp[-1].listPtr; listPtr;
				     listPtr = listPtr->nextPtr) {
				    objectPtr = findObject(listPtr->ptr,
							   thisParserPtr,
							   thisModulePtr);
				    listPtr->ptr = objectPtr;
				}
				setObjectList(groupObjectPtr, yyvsp[-1].listPtr);
			    }
			;}
    break;

  case 112:
#line 1714 "parser-sming.y"
    {
			    if (groupObjectPtr) {
				setObjectStatus(groupObjectPtr, yyvsp[0].status);
			    }
			;}
    break;

  case 113:
#line 1720 "parser-sming.y"
    {
			    if (groupObjectPtr && yyvsp[-1].text) {
				setObjectDescription(groupObjectPtr, yyvsp[-1].text,
						     thisParserPtr);
			    }
			;}
    break;

  case 114:
#line 1727 "parser-sming.y"
    {
			    if (groupObjectPtr && yyvsp[0].text) {
				setObjectReference(groupObjectPtr, yyvsp[0].text,
						   thisParserPtr);
			    }
			;}
    break;

  case 115:
#line 1734 "parser-sming.y"
    {
			    yyval.objectPtr = groupObjectPtr;
			    groupObjectPtr = NULL;
			    free(groupIdentifier);
			;}
    break;

  case 116:
#line 1742 "parser-sming.y"
    {
			    yyval.rc = 0;
			;}
    break;

  case 117:
#line 1746 "parser-sming.y"
    {
			    /*
			     * Return the number of successfully
			     * parsed compliance statements.
			     */
			    yyval.rc = yyvsp[0].rc;
			;}
    break;

  case 118:
#line 1756 "parser-sming.y"
    {
			    yyval.rc = yyvsp[0].rc;
			;}
    break;

  case 119:
#line 1761 "parser-sming.y"
    {
			    /*
			     * Sum up the number of successfully parsed
			     * compliances or return -1, if at least one
			     * module failed.
			     */
			    if ((yyvsp[-1].rc >= 0) && (yyvsp[0].rc >= 0)) {
				yyval.rc = yyvsp[-1].rc + yyvsp[0].rc;
			    } else {
				yyval.rc = -1;
			    }
			;}
    break;

  case 120:
#line 1776 "parser-sming.y"
    {
			    /*
			     * If we got an (Object *) return rc == 1,
			     * otherwise parsing failed (rc == -1).
			     */
			    if (yyvsp[-1].objectPtr) {
				yyval.rc = 1;
			    } else {
				yyval.rc = -1;
			    }
			;}
    break;

  case 121:
#line 1790 "parser-sming.y"
    {
			    complianceIdentifier = yyvsp[0].text;
			;}
    break;

  case 122:
#line 1795 "parser-sming.y"
    {
			    if (yyvsp[-1].nodePtr) {
				complianceObjectPtr =
				    addObject(complianceIdentifier,
					      yyvsp[-1].nodePtr->parentPtr,
					      yyvsp[-1].nodePtr->subid,
					      0, thisParserPtr);
				setObjectDecl(complianceObjectPtr,
					      SMI_DECL_COMPLIANCE);
				setObjectNodekind(complianceObjectPtr,
						  SMI_NODEKIND_COMPLIANCE);
			    }
			;}
    break;

  case 123:
#line 1809 "parser-sming.y"
    {
			    if (complianceObjectPtr) {
				setObjectStatus(complianceObjectPtr, yyvsp[0].status);
			    }
			;}
    break;

  case 124:
#line 1815 "parser-sming.y"
    {
			    if (complianceObjectPtr && yyvsp[-1].text) {
				setObjectDescription(complianceObjectPtr, yyvsp[-1].text,
						     thisParserPtr);
			    }
			;}
    break;

  case 125:
#line 1822 "parser-sming.y"
    {
			    if (complianceObjectPtr && yyvsp[0].text) {
				setObjectReference(complianceObjectPtr, yyvsp[0].text,
						   thisParserPtr);
			    }
			;}
    break;

  case 126:
#line 1829 "parser-sming.y"
    {
			    List *listPtr;
			    Object *objectPtr;
			    
			    if (complianceObjectPtr && yyvsp[0].listPtr) {
				for (listPtr = yyvsp[0].listPtr; listPtr;
				     listPtr = listPtr->nextPtr) {
				    objectPtr = findObject(listPtr->ptr,
							   thisParserPtr,
							   thisModulePtr);
				    listPtr->ptr = objectPtr;
				}
				setObjectList(complianceObjectPtr, yyvsp[0].listPtr);
			    }
			;}
    break;

  case 127:
#line 1845 "parser-sming.y"
    {
			    Option *optionPtr;
			    List *listPtr;
			    
			    complianceObjectPtr->optionlistPtr = yyvsp[0].listPtr;
			    if (yyvsp[0].listPtr) {
				for (listPtr = yyvsp[0].listPtr;
				     listPtr;
				     listPtr = listPtr->nextPtr) {
				    optionPtr = ((Option *)(listPtr->ptr));
				    optionPtr->compliancePtr =
					complianceObjectPtr;
				}
			    }
			;}
    break;

  case 128:
#line 1861 "parser-sming.y"
    {
			    Refinement *refinementPtr;
			    List *listPtr;
			    
			    complianceObjectPtr->refinementlistPtr = yyvsp[0].listPtr;
			    if (yyvsp[0].listPtr) {
				for (listPtr = yyvsp[0].listPtr;
				     listPtr;
				     listPtr = listPtr->nextPtr) {
				    refinementPtr =
					((Refinement *)(listPtr->ptr));
				    refinementPtr->compliancePtr =
					complianceObjectPtr;
				}
			    }
			;}
    break;

  case 129:
#line 1878 "parser-sming.y"
    {
			    yyval.objectPtr = complianceObjectPtr;
			    complianceObjectPtr = NULL;
			    free(complianceIdentifier);
			;}
    break;

  case 130:
#line 1886 "parser-sming.y"
    {
			    yyval.rc = 0;
			;}
    break;

  case 131:
#line 1890 "parser-sming.y"
    {
			    yyval.rc = yyvsp[0].rc;
			;}
    break;

  case 132:
#line 1896 "parser-sming.y"
    {
			    yyval.rc = yyvsp[0].rc;
			;}
    break;

  case 133:
#line 1900 "parser-sming.y"
    {
			    /*
			     * Sum up the number of successfully parsed
			     * imports or return -1, if at least one
			     * module failed.
			     */
			    if ((yyvsp[-1].rc >= 0) && (yyvsp[0].rc >= 0)) {
				yyval.rc = yyvsp[-1].rc + yyvsp[0].rc;
			    } else {
				yyval.rc = -1;
			    }
			;}
    break;

  case 134:
#line 1915 "parser-sming.y"
    {
			    /*
			     * If we got an (Object *) return rc == 1,
			     * otherwise parsing failed (rc == -1).
			     */
			    if (yyvsp[-1].listPtr) {
				yyval.rc = 1;
			    } else {
				yyval.rc = -1;
			    }
			;}
    break;

  case 135:
#line 1929 "parser-sming.y"
    {
			    importModulename = smiStrdup(yyvsp[0].text);
			;}
    break;

  case 136:
#line 1934 "parser-sming.y"
    {
			    List *listPtr, *nextPtr;
			    
			    for (listPtr = yyvsp[0].listPtr; listPtr; listPtr = nextPtr) {
				addImport(listPtr->ptr, thisParserPtr);
				thisParserPtr->modulePtr->
				                      numImportedIdentifiers++;
				nextPtr = listPtr->nextPtr;
				free(listPtr);
			    }
			;}
    break;

  case 137:
#line 1946 "parser-sming.y"
    {
			    Module *modulePtr;
			    char *s = importModulename;

			    modulePtr = findModuleByName(s);
			    if (!modulePtr) {
				modulePtr = loadModule(s, thisParserPtr);
			    }
			    checkImports(modulePtr, thisParserPtr);
			    free(s);
			    yyval.listPtr = NULL;
			;}
    break;

  case 138:
#line 1961 "parser-sming.y"
    {
			    yyval.rc = 0;
			;}
    break;

  case 139:
#line 1965 "parser-sming.y"
    {
			    yyval.rc = yyvsp[0].rc;
			;}
    break;

  case 140:
#line 1971 "parser-sming.y"
    {
			    yyval.rc = yyvsp[0].rc;
			;}
    break;

  case 141:
#line 1975 "parser-sming.y"
    {
			    /*
			     * Sum up the number of successfully parsed
			     * revisions or return -1, if at least one
			     * module failed.
			     */
			    if ((yyvsp[-1].rc >= 0) && (yyvsp[0].rc >= 0)) {
				yyval.rc = yyvsp[-1].rc + yyvsp[0].rc;
			    } else {
				yyval.rc = -1;
			    }
			;}
    break;

  case 142:
#line 1990 "parser-sming.y"
    {
			       /*
				* If we got a (Revision *) return rc == 1,
				* otherwise parsing failed (rc == -1).
				*/
			       if (yyvsp[-1].revisionPtr) {
				yyval.rc = 1;
			       } else {
				   yyval.rc = -1;
			       }
			   ;}
    break;

  case 143:
#line 2007 "parser-sming.y"
    {
			    yyval.revisionPtr = addRevision(yyvsp[-6].date, yyvsp[-4].text, thisParserPtr);
			;}
    break;

  case 144:
#line 2013 "parser-sming.y"
    {
			    yyval.rc = 0;
			;}
    break;

  case 145:
#line 2017 "parser-sming.y"
    {
			    yyval.rc = 1;
			;}
    break;

  case 146:
#line 2023 "parser-sming.y"
    {
			    thisParserPtr->identityObjectName = yyvsp[-2].text;
			    yyval.rc = 1;
			;}
    break;

  case 147:
#line 2032 "parser-sming.y"
    {
			    yyval.typePtr = yyvsp[-2].typePtr;
			;}
    break;

  case 148:
#line 2038 "parser-sming.y"
    {
			    yyval.typePtr = NULL;
			;}
    break;

  case 149:
#line 2042 "parser-sming.y"
    {
			    yyval.typePtr = yyvsp[-1].typePtr;
			;}
    break;

  case 150:
#line 2049 "parser-sming.y"
    {
			    yyval.typePtr = yyvsp[-2].typePtr;
			;}
    break;

  case 151:
#line 2055 "parser-sming.y"
    {
			    yyval.typePtr = NULL;
			;}
    break;

  case 152:
#line 2059 "parser-sming.y"
    {
			    yyval.typePtr = yyvsp[-1].typePtr;
			;}
    break;

  case 153:
#line 2066 "parser-sming.y"
    {
			    yyval.typePtr = yyvsp[-2].typePtr;
			;}
    break;

  case 159:
#line 2080 "parser-sming.y"
    {
			    if (yyvsp[-8].rc) {
				yyval.index.implied = 1;
			    } else {
				yyval.index.implied = 0;
			    }
			    yyval.index.indexkind = SMI_INDEX_INDEX;
			    yyval.index.listPtr = yyvsp[-4].listPtr;
			    yyval.index.rowPtr = NULL;
			    /*
			     * NOTE: at this point $$->listPtr and $$-rowPtr
			     * contain identifier strings. Converstion to
			     * (Object *)'s must be delayed till the whole
			     * module is parsed, since even in SMIng index
			     * clauses can contain forward references.
			     */
			;}
    break;

  case 160:
#line 2100 "parser-sming.y"
    {
			    yyval.index.implied = 0;
			    yyval.index.indexkind = SMI_INDEX_AUGMENT;
			    yyval.index.listPtr = NULL;
			    yyval.index.rowPtr = (void *)yyvsp[-2].text;
			    /*
			     * NOTE: at this point $$->listPtr and $$-rowPtr
			     * contain identifier strings. Converstion to
			     * (Object *)'s must be delayed till the whole
			     * module is parsed, since even in SMIng index
			     * clauses can contain forward references.
			     */
			;}
    break;

  case 161:
#line 2118 "parser-sming.y"
    {
			    if (yyvsp[-8].rc) {
				yyval.index.implied = 1;
			    } else {
				yyval.index.implied = 0;
			    }
			    yyval.index.indexkind = SMI_INDEX_REORDER;
			    yyval.index.listPtr = yyvsp[-4].listPtr;
			    /*
			     * NOTE: at this point $$->listPtr and $$-rowPtr
			     * contain identifier strings. Converstion to
			     * (Object *)'s must be delayed till the whole
			     * module is parsed, since even in SMIng index
			     * clauses can contain forward references.
			     */
			    yyval.index.rowPtr = (void *)yyvsp[-9].text;
			;}
    break;

  case 162:
#line 2138 "parser-sming.y"
    {
			    yyval.index.implied = 0;
			    yyval.index.indexkind = SMI_INDEX_SPARSE;
			    yyval.index.listPtr = NULL;
			    yyval.index.rowPtr = (void *)yyvsp[-2].text;
			    /*
			     * NOTE: at this point $$->listPtr and $$-rowPtr
			     * contain identifier strings. Converstion to
			     * (Object *)'s must be delayed till the whole
			     * module is parsed, since even in SMIng index
			     * clauses can contain forward references.
			     */
			;}
    break;

  case 163:
#line 2156 "parser-sming.y"
    {
			    if (yyvsp[-8].rc) {
				yyval.index.implied = 1;
			    } else {
				yyval.index.implied = 0;
			    }
			    yyval.index.indexkind = SMI_INDEX_EXPAND;
			    yyval.index.listPtr = yyvsp[-4].listPtr;
			    /*
			     * NOTE: at this point $$->listPtr and $$-rowPtr
			     * contain identifier strings. Converstion to
			     * (Object *)'s must be delayed till the whole
			     * module is parsed, since even in SMIng index
			     * clauses can contain forward references.
			     */
			    yyval.index.rowPtr = (void *)yyvsp[-9].text;
			;}
    break;

  case 164:
#line 2176 "parser-sming.y"
    {
			    yyval.rc = 0;
			;}
    break;

  case 165:
#line 2180 "parser-sming.y"
    {
			    yyval.rc = 1;
			;}
    break;

  case 166:
#line 2186 "parser-sming.y"
    {
			    yyval.rc = 0;
			;}
    break;

  case 167:
#line 2190 "parser-sming.y"
    {
			    yyval.rc = 1;
			;}
    break;

  case 168:
#line 2196 "parser-sming.y"
    {
			    yyval.rc = 0;
			;}
    break;

  case 169:
#line 2202 "parser-sming.y"
    {
			    yyval.nodePtr = yyvsp[-2].nodePtr;
			;}
    break;

  case 170:
#line 2208 "parser-sming.y"
    {
			    yyval.date = yyvsp[-2].date;
			;}
    break;

  case 171:
#line 2214 "parser-sming.y"
    {
			    yyval.text = yyvsp[-2].text;
			;}
    break;

  case 172:
#line 2220 "parser-sming.y"
    {
			    yyval.text = yyvsp[-2].text;
			;}
    break;

  case 173:
#line 2226 "parser-sming.y"
    {
			    yyval.text = NULL;
			;}
    break;

  case 174:
#line 2230 "parser-sming.y"
    {
			    yyval.text = yyvsp[-1].text;
			;}
    break;

  case 175:
#line 2236 "parser-sming.y"
    {
			    yyval.text = yyvsp[-2].text;
			;}
    break;

  case 176:
#line 2242 "parser-sming.y"
    {
			    yyval.text = NULL;
			;}
    break;

  case 177:
#line 2246 "parser-sming.y"
    {
			    yyval.text = yyvsp[-1].text;
			;}
    break;

  case 178:
#line 2252 "parser-sming.y"
    {
			    yyval.text = yyvsp[-2].text;
			;}
    break;

  case 179:
#line 2258 "parser-sming.y"
    {
			    yyval.status = SMI_STATUS_CURRENT;
			;}
    break;

  case 180:
#line 2262 "parser-sming.y"
    {
			    yyval.status = yyvsp[-1].status;
			;}
    break;

  case 181:
#line 2268 "parser-sming.y"
    {
			    yyval.status = yyvsp[-2].status;
			;}
    break;

  case 182:
#line 2274 "parser-sming.y"
    {
			    yyval.access = SMI_ACCESS_UNKNOWN;
			;}
    break;

  case 183:
#line 2278 "parser-sming.y"
    {
			    yyval.access = yyvsp[-1].access;
			;}
    break;

  case 184:
#line 2284 "parser-sming.y"
    {
			    yyval.access = yyvsp[-2].access;
			;}
    break;

  case 185:
#line 2290 "parser-sming.y"
    {
			    yyval.valuePtr = NULL;
			;}
    break;

  case 186:
#line 2294 "parser-sming.y"
    {
			    yyval.valuePtr = yyvsp[-1].valuePtr;
			;}
    break;

  case 187:
#line 2300 "parser-sming.y"
    {
			    yyval.valuePtr = yyvsp[-2].valuePtr;
			;}
    break;

  case 188:
#line 2306 "parser-sming.y"
    {
			    yyval.text = NULL;
			;}
    break;

  case 189:
#line 2310 "parser-sming.y"
    {
			    yyval.text = yyvsp[-1].text;
			;}
    break;

  case 190:
#line 2316 "parser-sming.y"
    {
			    yyval.text = yyvsp[-2].text;
			;}
    break;

  case 191:
#line 2322 "parser-sming.y"
    {
			    yyval.text = NULL;
			;}
    break;

  case 192:
#line 2326 "parser-sming.y"
    {
			    yyval.text = yyvsp[-1].text;
			;}
    break;

  case 193:
#line 2332 "parser-sming.y"
    {
			    yyval.text = yyvsp[-2].text;
			;}
    break;

  case 194:
#line 2338 "parser-sming.y"
    {
			    yyval.text = NULL;
			;}
    break;

  case 195:
#line 2342 "parser-sming.y"
    {
			    yyval.text = yyvsp[-1].text;
			;}
    break;

  case 196:
#line 2348 "parser-sming.y"
    {
			    yyval.text = yyvsp[-2].text;
			;}
    break;

  case 197:
#line 2355 "parser-sming.y"
    {
			    yyval.listPtr = yyvsp[-4].listPtr;
			;}
    break;

  case 198:
#line 2361 "parser-sming.y"
    {
			    yyval.listPtr = NULL;
			;}
    break;

  case 199:
#line 2365 "parser-sming.y"
    {
			    yyval.listPtr = yyvsp[-1].listPtr;
			;}
    break;

  case 200:
#line 2372 "parser-sming.y"
    {
			    yyval.listPtr = yyvsp[-4].listPtr;
			;}
    break;

  case 201:
#line 2378 "parser-sming.y"
    {
			    yyval.listPtr = NULL;
			;}
    break;

  case 202:
#line 2382 "parser-sming.y"
    {
			    yyval.listPtr = yyvsp[-1].listPtr;
			;}
    break;

  case 203:
#line 2389 "parser-sming.y"
    {
			    yyval.listPtr = yyvsp[-4].listPtr;
			;}
    break;

  case 204:
#line 2395 "parser-sming.y"
    {
			    yyval.listPtr = NULL;
			;}
    break;

  case 205:
#line 2399 "parser-sming.y"
    {
			    yyval.listPtr = yyvsp[0].listPtr;
			;}
    break;

  case 206:
#line 2405 "parser-sming.y"
    {
			    yyval.listPtr = smiMalloc(sizeof(List));
			    yyval.listPtr->ptr = yyvsp[0].optionPtr;
			    yyval.listPtr->nextPtr = NULL;
			;}
    break;

  case 207:
#line 2412 "parser-sming.y"
    {
			    List *p, *pp;
			    
			    p = smiMalloc(sizeof(List));
			    p->ptr = yyvsp[0].optionPtr;
			    p->nextPtr = NULL;
			    for (pp = yyvsp[-1].listPtr; pp->nextPtr; pp = pp->nextPtr);
			    pp->nextPtr = p;
			    yyval.listPtr = yyvsp[-1].listPtr;
			;}
    break;

  case 208:
#line 2425 "parser-sming.y"
    {
			       yyval.optionPtr = yyvsp[-1].optionPtr;
			   ;}
    break;

  case 209:
#line 2434 "parser-sming.y"
    {
			    yyval.optionPtr = smiMalloc(sizeof(Option));
			    yyval.optionPtr->objectPtr = findObject(yyvsp[-7].text,
						       thisParserPtr,
						       thisModulePtr);
			    yyval.optionPtr->export.description = smiStrdup(yyvsp[-4].text);
			;}
    break;

  case 210:
#line 2444 "parser-sming.y"
    {
			    yyval.listPtr = NULL;
			;}
    break;

  case 211:
#line 2448 "parser-sming.y"
    {
			    yyval.listPtr = yyvsp[0].listPtr;
			;}
    break;

  case 212:
#line 2454 "parser-sming.y"
    {
			    yyval.listPtr = smiMalloc(sizeof(List));
			    yyval.listPtr->ptr = yyvsp[0].refinementPtr;
			    yyval.listPtr->nextPtr = NULL;
			;}
    break;

  case 213:
#line 2460 "parser-sming.y"
    {
			    List *p, *pp;
			    
			    p = smiMalloc(sizeof(List));
			    p->ptr = yyvsp[0].refinementPtr;
			    p->nextPtr = NULL;
			    for (pp = yyvsp[-1].listPtr; pp->nextPtr; pp = pp->nextPtr);
			    pp->nextPtr = p;
			    yyval.listPtr = yyvsp[-1].listPtr;
			;}
    break;

  case 214:
#line 2473 "parser-sming.y"
    {
			    yyval.refinementPtr = yyvsp[-1].refinementPtr;
			;}
    break;

  case 215:
#line 2484 "parser-sming.y"
    {
			    yyval.refinementPtr = smiMalloc(sizeof(Refinement));
			    yyval.refinementPtr->objectPtr = findObject(yyvsp[-10].text,
						       thisParserPtr,
						       thisModulePtr);
			    if (yyvsp[-7].typePtr) {
				yyval.refinementPtr->typePtr = duplicateType(yyvsp[-7].typePtr, 0,
							    thisParserPtr);
				yyval.refinementPtr->typePtr->listPtr = yyvsp[-7].typePtr->listPtr;
			    } else {
				yyval.refinementPtr->typePtr = NULL;
			    }
			    if (yyvsp[-6].typePtr) {
				yyval.refinementPtr->writetypePtr =
				    duplicateType(yyvsp[-6].typePtr, 0, thisParserPtr);
				yyval.refinementPtr->writetypePtr->listPtr = yyvsp[-6].typePtr->listPtr;
			    } else {
				yyval.refinementPtr->writetypePtr = NULL;
			    }
			    yyval.refinementPtr->export.access = yyvsp[-5].access;
			    yyval.refinementPtr->export.description = smiStrdup(yyvsp[-4].text);
			;}
    break;

  case 216:
#line 2509 "parser-sming.y"
    {
			    yyval.typePtr = yyvsp[0].typePtr;
			;}
    break;

  case 217:
#line 2513 "parser-sming.y"
    {
			    yyval.typePtr = yyvsp[0].typePtr;
			;}
    break;

  case 218:
#line 2519 "parser-sming.y"
    {
			    List *p;
			    
			    if (!yyvsp[0].listPtr) {
				yyval.typePtr = smiHandle->typeOctetStringPtr;
			    } else {
				yyval.typePtr = duplicateType(smiHandle->typeOctetStringPtr, 0,
						   thisParserPtr);
				setTypeParent(yyval.typePtr, smiHandle->typeOctetStringPtr);
				setTypeList(yyval.typePtr, yyvsp[0].listPtr);
				for (p = yyvsp[0].listPtr; p; p = p->nextPtr)
				    ((Range *)p->ptr)->typePtr = yyval.typePtr;
			    }
			;}
    break;

  case 219:
#line 2534 "parser-sming.y"
    {
			    yyval.typePtr = smiHandle->typeObjectIdentifierPtr;
			;}
    break;

  case 220:
#line 2538 "parser-sming.y"
    {
			    List *p;
			    
			    if (!yyvsp[0].listPtr) {
				yyval.typePtr = smiHandle->typeInteger32Ptr;
			    } else {
				yyval.typePtr = duplicateType(smiHandle->typeInteger32Ptr, 0,
						   thisParserPtr);
				setTypeParent(yyval.typePtr, smiHandle->typeInteger32Ptr);
				setTypeList(yyval.typePtr, yyvsp[0].listPtr);
				for (p = yyvsp[0].listPtr; p; p = p->nextPtr)
				    ((Range *)p->ptr)->typePtr = yyval.typePtr;
			    }
			;}
    break;

  case 221:
#line 2553 "parser-sming.y"
    {
			    List *p;
			    
			    if (!yyvsp[0].listPtr) {
				yyval.typePtr = smiHandle->typeUnsigned32Ptr;
			    } else {
				yyval.typePtr = duplicateType(smiHandle->typeUnsigned32Ptr, 0,
						   thisParserPtr);
				setTypeParent(yyval.typePtr, smiHandle->typeUnsigned32Ptr);
				setTypeList(yyval.typePtr, yyvsp[0].listPtr);
				for (p = yyvsp[0].listPtr; p; p = p->nextPtr)
				    ((Range *)p->ptr)->typePtr = yyval.typePtr;
			    }
			;}
    break;

  case 222:
#line 2568 "parser-sming.y"
    {
			    List *p;
			    
			    if (!yyvsp[0].listPtr) {
				yyval.typePtr = smiHandle->typeInteger64Ptr;
			    } else {
				yyval.typePtr = duplicateType(smiHandle->typeInteger64Ptr, 0,
						   thisParserPtr);
				setTypeParent(yyval.typePtr, smiHandle->typeInteger64Ptr);
				setTypeList(yyval.typePtr, yyvsp[0].listPtr);
				for (p = yyvsp[0].listPtr; p; p = p->nextPtr)
				    ((Range *)p->ptr)->typePtr = yyval.typePtr;
			    }
			;}
    break;

  case 223:
#line 2583 "parser-sming.y"
    {
			    List *p;
			    
			    if (!yyvsp[0].listPtr) {
				yyval.typePtr = smiHandle->typeUnsigned64Ptr;
			    } else {
				yyval.typePtr = duplicateType(smiHandle->typeUnsigned64Ptr, 0,
						   thisParserPtr);
				setTypeParent(yyval.typePtr, smiHandle->typeUnsigned64Ptr);
				setTypeList(yyval.typePtr, yyvsp[0].listPtr);
				for (p = yyvsp[0].listPtr; p; p = p->nextPtr)
				    ((Range *)p->ptr)->typePtr = yyval.typePtr;
			    }
			;}
    break;

  case 224:
#line 2598 "parser-sming.y"
    {
			    List *p;
			    
			    if (!yyvsp[0].listPtr) {
				yyval.typePtr = smiHandle->typeFloat32Ptr;
			    } else {
				yyval.typePtr = duplicateType(smiHandle->typeFloat32Ptr, 0,
						   thisParserPtr);
				setTypeParent(yyval.typePtr, smiHandle->typeFloat32Ptr);
				setTypeList(yyval.typePtr, yyvsp[0].listPtr);
				for (p = yyvsp[0].listPtr; p; p = p->nextPtr)
				    ((Range *)p->ptr)->typePtr = yyval.typePtr;
			    }
			;}
    break;

  case 225:
#line 2613 "parser-sming.y"
    {
			    List *p;
			    
			    if (!yyvsp[0].listPtr) {
				yyval.typePtr = smiHandle->typeFloat64Ptr;
			    } else {
				yyval.typePtr = duplicateType(smiHandle->typeFloat64Ptr, 0,
						   thisParserPtr);
				setTypeParent(yyval.typePtr, smiHandle->typeFloat64Ptr);
				setTypeList(yyval.typePtr, yyvsp[0].listPtr);
				for (p = yyvsp[0].listPtr; p; p = p->nextPtr)
				    ((Range *)p->ptr)->typePtr = yyval.typePtr;
			    }
			;}
    break;

  case 226:
#line 2628 "parser-sming.y"
    {
			    List *p;
			    
			    if (!yyvsp[0].listPtr) {
				yyval.typePtr = smiHandle->typeFloat128Ptr;
			    } else {
				yyval.typePtr = duplicateType(smiHandle->typeFloat128Ptr, 0,
						   thisParserPtr);
				setTypeParent(yyval.typePtr, smiHandle->typeFloat128Ptr);
				setTypeList(yyval.typePtr, yyvsp[0].listPtr);
				for (p = yyvsp[0].listPtr; p; p = p->nextPtr)
				    ((Range *)p->ptr)->typePtr = yyval.typePtr;
			    }
			;}
    break;

  case 227:
#line 2643 "parser-sming.y"
    {
			    List *p;
			    
			    if (!yyvsp[0].listPtr) {
				yyval.typePtr = smiHandle->typeEnumPtr;
			    } else {
				yyval.typePtr = duplicateType(smiHandle->typeEnumPtr, 0,
						   thisParserPtr);
				setTypeParent(yyval.typePtr, smiHandle->typeEnumPtr);
				setTypeList(yyval.typePtr, yyvsp[0].listPtr);
				for (p = yyvsp[0].listPtr; p; p = p->nextPtr)
				    ((NamedNumber *)p->ptr)->typePtr = yyval.typePtr;
			    }
			;}
    break;

  case 228:
#line 2658 "parser-sming.y"
    {
			    List *p;
			    
			    if (!yyvsp[0].listPtr) {
				yyval.typePtr = smiHandle->typeBitsPtr;
			    } else {
				yyval.typePtr = duplicateType(smiHandle->typeBitsPtr, 0,
						   thisParserPtr);
				setTypeParent(yyval.typePtr, smiHandle->typeBitsPtr);
				setTypeList(yyval.typePtr, yyvsp[0].listPtr);
				for (p = yyvsp[0].listPtr; p; p = p->nextPtr)
				    ((NamedNumber *)p->ptr)->typePtr = yyval.typePtr;
			    }
			;}
    break;

  case 229:
#line 2675 "parser-sming.y"
    {
			    typePtr = findType(yyvsp[-1].text, thisParserPtr,
					       thisModulePtr);
			    if (typePtr && yyvsp[0].listPtr) {
				typePtr = duplicateType(typePtr, 0,
							thisParserPtr);
				setTypeList(typePtr, yyvsp[0].listPtr);
			    }

			    yyval.typePtr = typePtr;
			;}
    break;

  case 230:
#line 2689 "parser-sming.y"
    {
			    yyval.listPtr = NULL;
			;}
    break;

  case 231:
#line 2693 "parser-sming.y"
    {
			    yyval.listPtr = yyvsp[0].listPtr;
			;}
    break;

  case 232:
#line 2699 "parser-sming.y"
    {
			    yyval.listPtr = yyvsp[0].listPtr;
			;}
    break;

  case 233:
#line 2703 "parser-sming.y"
    {
			    yyval.listPtr = yyvsp[0].listPtr;
			;}
    break;

  case 234:
#line 2709 "parser-sming.y"
    {
			    yyval.listPtr = NULL;
			;}
    break;

  case 235:
#line 2713 "parser-sming.y"
    {
			    yyval.listPtr = yyvsp[0].listPtr;
			;}
    break;

  case 236:
#line 2720 "parser-sming.y"
    {
			    yyval.listPtr = smiMalloc(sizeof(List));
			    yyval.listPtr->ptr = yyvsp[-3].rangePtr;
			    yyval.listPtr->nextPtr = yyvsp[-2].listPtr;
			;}
    break;

  case 237:
#line 2728 "parser-sming.y"
    {
			    yyval.listPtr = NULL;
			;}
    break;

  case 238:
#line 2732 "parser-sming.y"
    {
			    yyval.listPtr = yyvsp[0].listPtr;
			;}
    break;

  case 239:
#line 2738 "parser-sming.y"
    {
			    yyval.listPtr = smiMalloc(sizeof(List));
			    yyval.listPtr->ptr = yyvsp[0].rangePtr;
			    yyval.listPtr->nextPtr = NULL;
			;}
    break;

  case 240:
#line 2744 "parser-sming.y"
    {
			    List *p, *pp;
			    
			    p = smiMalloc(sizeof(List));
			    p->ptr = yyvsp[0].rangePtr;
			    p->nextPtr = NULL;
			    for (pp = yyvsp[-1].listPtr; pp->nextPtr; pp = pp->nextPtr);
			    pp->nextPtr = p;
			    yyval.listPtr = yyvsp[-1].listPtr;
			;}
    break;

  case 241:
#line 2757 "parser-sming.y"
    {
			    yyval.rangePtr = yyvsp[0].rangePtr;
			;}
    break;

  case 242:
#line 2763 "parser-sming.y"
    {
			    yyval.rangePtr = smiMalloc(sizeof(Range));
			    yyval.rangePtr->export.minValue = *yyvsp[-1].valuePtr;
			    if (yyvsp[0].valuePtr) {
				yyval.rangePtr->export.maxValue = *yyvsp[0].valuePtr;
				smiFree(yyvsp[0].valuePtr);
			    } else {
				yyval.rangePtr->export.maxValue = *yyvsp[-1].valuePtr;
			    }
			    smiFree(yyvsp[-1].valuePtr);
			;}
    break;

  case 243:
#line 2777 "parser-sming.y"
    {
			    yyval.valuePtr = NULL;
			;}
    break;

  case 244:
#line 2781 "parser-sming.y"
    {
			    yyval.valuePtr = yyvsp[0].valuePtr;
			;}
    break;

  case 245:
#line 2787 "parser-sming.y"
    {
			    yyval.valuePtr = yyvsp[0].valuePtr;
			;}
    break;

  case 246:
#line 2793 "parser-sming.y"
    {
			    yyval.listPtr = NULL;
			;}
    break;

  case 247:
#line 2797 "parser-sming.y"
    {
			    yyval.listPtr = yyvsp[0].listPtr;
			;}
    break;

  case 248:
#line 2804 "parser-sming.y"
    {
			    yyval.listPtr = smiMalloc(sizeof(List));
			    yyval.listPtr->ptr = yyvsp[-3].rangePtr;
			    yyval.listPtr->nextPtr = yyvsp[-2].listPtr;
			;}
    break;

  case 249:
#line 2812 "parser-sming.y"
    {
			    yyval.listPtr = NULL;
			;}
    break;

  case 250:
#line 2816 "parser-sming.y"
    {
			    yyval.listPtr = yyvsp[0].listPtr;
			;}
    break;

  case 251:
#line 2822 "parser-sming.y"
    {
			    yyval.listPtr = smiMalloc(sizeof(List));
			    yyval.listPtr->ptr = yyvsp[0].rangePtr;
			    yyval.listPtr->nextPtr = NULL;
			;}
    break;

  case 252:
#line 2828 "parser-sming.y"
    {
			    List *p, *pp;
			    
			    p = smiMalloc(sizeof(List));
			    p->ptr = yyvsp[0].rangePtr;
			    p->nextPtr = NULL;
			    for (pp = yyvsp[-1].listPtr; pp->nextPtr; pp = pp->nextPtr);
			    pp->nextPtr = p;
			    yyval.listPtr = yyvsp[-1].listPtr;
			;}
    break;

  case 253:
#line 2841 "parser-sming.y"
    {
			    yyval.rangePtr = yyvsp[0].rangePtr;
			;}
    break;

  case 254:
#line 2847 "parser-sming.y"
    {
			    yyval.rangePtr = smiMalloc(sizeof(Range));
			    yyval.rangePtr->export.minValue.basetype = SMI_BASETYPE_FLOAT64;
			    yyval.rangePtr->export.minValue.value.float64 = strtod(yyvsp[-1].text, NULL);
			    if (yyvsp[0].text) {
				yyval.rangePtr->export.maxValue.basetype =
				                          SMI_BASETYPE_FLOAT64;
				yyval.rangePtr->export.maxValue.value.float64 =
				                              strtod(yyvsp[0].text, NULL);
			    } else {
				yyval.rangePtr->export.maxValue = yyval.rangePtr->export.minValue;
			    }
			;}
    break;

  case 255:
#line 2863 "parser-sming.y"
    {
			    yyval.text = NULL;
			;}
    break;

  case 256:
#line 2867 "parser-sming.y"
    {
			    yyval.text = yyvsp[0].text;
			;}
    break;

  case 257:
#line 2873 "parser-sming.y"
    {
			    yyval.text = yyvsp[0].text;
			;}
    break;

  case 258:
#line 2879 "parser-sming.y"
    {
			    yyval.listPtr = yyvsp[-2].listPtr;
			;}
    break;

  case 259:
#line 2886 "parser-sming.y"
    {
			    yyval.listPtr = smiMalloc(sizeof(List));
			    yyval.listPtr->ptr = yyvsp[-2].namedNumberPtr;
			    yyval.listPtr->nextPtr = yyvsp[-1].listPtr;
			;}
    break;

  case 260:
#line 2894 "parser-sming.y"
    {
			    yyval.listPtr = NULL;
			;}
    break;

  case 261:
#line 2898 "parser-sming.y"
    {
			    yyval.listPtr = yyvsp[0].listPtr;
			;}
    break;

  case 262:
#line 2904 "parser-sming.y"
    {
			    yyval.listPtr = smiMalloc(sizeof(List));
			    yyval.listPtr->ptr = yyvsp[0].namedNumberPtr;
			    yyval.listPtr->nextPtr = NULL;
			;}
    break;

  case 263:
#line 2911 "parser-sming.y"
    {
			    List *p, *pp;
			    
			    p = smiMalloc(sizeof(List));
			    p->ptr = yyvsp[0].namedNumberPtr;
			    p->nextPtr = NULL;
			    for (pp = yyvsp[-1].listPtr; pp->nextPtr; pp = pp->nextPtr);
			    pp->nextPtr = p;
			    yyval.listPtr = yyvsp[-1].listPtr;
			;}
    break;

  case 264:
#line 2925 "parser-sming.y"
    {
			    yyval.namedNumberPtr = yyvsp[0].namedNumberPtr;
			;}
    break;

  case 265:
#line 2931 "parser-sming.y"
    {
			    yyval.namedNumberPtr = smiMalloc(sizeof(NamedNumber));
			    yyval.namedNumberPtr->export.name = yyvsp[-6].text;
			    yyval.namedNumberPtr->export.value = *yyvsp[-2].valuePtr;
			    smiFree(yyvsp[-2].valuePtr);
			;}
    break;

  case 266:
#line 2940 "parser-sming.y"
    {
			    yyval.listPtr = smiMalloc(sizeof(List));
			    yyval.listPtr->ptr = yyvsp[-2].text;
			    yyval.listPtr->nextPtr = yyvsp[-1].listPtr;
			;}
    break;

  case 267:
#line 2948 "parser-sming.y"
    {
			    yyval.listPtr = NULL;
			;}
    break;

  case 268:
#line 2952 "parser-sming.y"
    {
			    yyval.listPtr = yyvsp[0].listPtr;
			;}
    break;

  case 269:
#line 2958 "parser-sming.y"
    {
			    yyval.listPtr = smiMalloc(sizeof(List));
			    yyval.listPtr->ptr = yyvsp[0].text;
			    yyval.listPtr->nextPtr = NULL;
			;}
    break;

  case 270:
#line 2964 "parser-sming.y"
    {
			    List *p, *pp;
			    
			    p = smiMalloc(sizeof(List));
			    p->ptr = yyvsp[0].text;
			    p->nextPtr = NULL;
			    for (pp = yyvsp[-1].listPtr; pp->nextPtr; pp = pp->nextPtr);
			    pp->nextPtr = p;
			    yyval.listPtr = yyvsp[-1].listPtr;
			;}
    break;

  case 271:
#line 2977 "parser-sming.y"
    {
			    yyval.text = yyvsp[0].text;
			;}
    break;

  case 272:
#line 3026 "parser-sming.y"
    {
			    yyval.listPtr = smiMalloc(sizeof(List));
			    yyval.listPtr->ptr = yyvsp[-2].text;
			    yyval.listPtr->nextPtr = yyvsp[-1].listPtr;
			;}
    break;

  case 273:
#line 3034 "parser-sming.y"
    {
			    yyval.listPtr = NULL;
			;}
    break;

  case 274:
#line 3038 "parser-sming.y"
    {
			    yyval.listPtr = yyvsp[0].listPtr;
			;}
    break;

  case 275:
#line 3044 "parser-sming.y"
    {
			    yyval.listPtr = smiMalloc(sizeof(List));
			    yyval.listPtr->ptr = yyvsp[0].text;
			    yyval.listPtr->nextPtr = NULL;
			;}
    break;

  case 276:
#line 3050 "parser-sming.y"
    {
			    List *p, *pp;
			    
			    p = smiMalloc(sizeof(List));
			    p->ptr = yyvsp[0].text;
			    p->nextPtr = NULL;
			    for (pp = yyvsp[-1].listPtr; pp->nextPtr; pp = pp->nextPtr);
			    pp->nextPtr = p;
			    yyval.listPtr = yyvsp[-1].listPtr;
			;}
    break;

  case 277:
#line 3063 "parser-sming.y"
    {
			    yyval.text = yyvsp[0].text;
			;}
    break;

  case 278:
#line 3069 "parser-sming.y"
    {
			    yyval.listPtr = yyvsp[-2].listPtr;
			;}
    break;

  case 279:
#line 3075 "parser-sming.y"
    {
			    yyval.listPtr = NULL;
			;}
    break;

  case 280:
#line 3079 "parser-sming.y"
    {
			    yyval.listPtr = smiMalloc(sizeof(List));
			    yyval.listPtr->ptr = yyvsp[-2].text;
			    yyval.listPtr->nextPtr = yyvsp[-1].listPtr;
			;}
    break;

  case 281:
#line 3087 "parser-sming.y"
    {
			    yyval.listPtr = NULL;
			;}
    break;

  case 282:
#line 3091 "parser-sming.y"
    {
			    yyval.listPtr = yyvsp[0].listPtr;
			;}
    break;

  case 283:
#line 3097 "parser-sming.y"
    {
			    yyval.listPtr = smiMalloc(sizeof(List));
			    yyval.listPtr->ptr = yyvsp[0].text;
			    yyval.listPtr->nextPtr = NULL;
			;}
    break;

  case 284:
#line 3103 "parser-sming.y"
    {
			    List *p, *pp;
			    
			    p = smiMalloc(sizeof(List));
			    p->ptr = yyvsp[0].text;
			    p->nextPtr = NULL;
			    for (pp = yyvsp[-1].listPtr; pp->nextPtr; pp = pp->nextPtr);
			    pp->nextPtr = p;
			    yyval.listPtr = yyvsp[-1].listPtr;
			;}
    break;

  case 285:
#line 3116 "parser-sming.y"
    {
			    yyval.text = yyvsp[0].text;
			;}
    break;

  case 286:
#line 3122 "parser-sming.y"
    {
			    yyval.text = yyvsp[0].text;
			;}
    break;

  case 287:
#line 3126 "parser-sming.y"
    {
			    yyval.text = yyvsp[0].text;
			;}
    break;

  case 288:
#line 3142 "parser-sming.y"
    {
			    char *s;

			    s = smiMalloc(strlen(yyvsp[-2].text) +
					    strlen(yyvsp[0].text) + 3);
			    sprintf(s, "%s::%s", yyvsp[-2].text, yyvsp[0].text);
			    yyval.text = s;
			    free(yyvsp[-2].text);
			    free(yyvsp[0].text);
			;}
    break;

  case 289:
#line 3153 "parser-sming.y"
    {
			    yyval.text = yyvsp[0].text;
			;}
    break;

  case 290:
#line 3159 "parser-sming.y"
    {
			    char *s;

			    s = smiMalloc(strlen(yyvsp[-2].text) +
					  strlen(yyvsp[0].text) + 3);
			    sprintf(s, "%s::%s", yyvsp[-2].text, yyvsp[0].text);
			    yyval.text = s;
			    free(yyvsp[-2].text);
			    free(yyvsp[0].text);
			;}
    break;

  case 291:
#line 3170 "parser-sming.y"
    {
			    yyval.text = yyvsp[0].text;
			;}
    break;

  case 292:
#line 3176 "parser-sming.y"
    {
			    int len;
			    if (yyvsp[0].text) {
				yyval.text = smiMalloc(strlen(yyvsp[-1].text) + strlen(yyvsp[0].text) + 1);
				strcpy(yyval.text, yyvsp[-1].text);
				strcat(yyval.text, yyvsp[0].text);
				free(yyvsp[-1].text);
				free(yyvsp[0].text);
			    } else {
				yyval.text = smiStrdup(yyvsp[-1].text);
			    }
			    len = strlen(yyval.text);
			    while (len > 0 && yyval.text[len-1] == '\n') {
				yyval.text[--len] = 0;
			    }
			;}
    break;

  case 293:
#line 3195 "parser-sming.y"
    {
			    yyval.text = NULL;
			;}
    break;

  case 294:
#line 3199 "parser-sming.y"
    {
			    yyval.text = yyvsp[0].text;
			;}
    break;

  case 295:
#line 3205 "parser-sming.y"
    {
			    yyval.text = yyvsp[0].text;
			;}
    break;

  case 296:
#line 3209 "parser-sming.y"
    {
			    yyval.text = smiMalloc(strlen(yyvsp[-1].text) + strlen(yyvsp[0].text) + 1);
			    strcpy(yyval.text, yyvsp[-1].text);
			    strcat(yyval.text, yyvsp[0].text);
			    free(yyvsp[-1].text);
			    free(yyvsp[0].text);
			;}
    break;

  case 297:
#line 3219 "parser-sming.y"
    {
			    yyval.text = smiStrdup(yyvsp[0].text);
			;}
    break;

  case 298:
#line 3225 "parser-sming.y"
    {
			    yyval.date = checkDate(thisParserPtr, yyvsp[0].text);
			;}
    break;

  case 299:
#line 3231 "parser-sming.y"
    {
			    yyval.text = smiStrdup(yyvsp[0].text);
			;}
    break;

  case 300:
#line 3237 "parser-sming.y"
    {
			    yyval.text = smiStrdup(yyvsp[0].text);
			;}
    break;

  case 301:
#line 3250 "parser-sming.y"
    {
			    if (defaultBasetype == SMI_BASETYPE_BITS) {
				yyval.valuePtr = smiMalloc(sizeof(SmiValue));
				yyval.valuePtr->basetype = SMI_BASETYPE_BITS;
				yyval.valuePtr->value.ptr = NULL;
			    } else {
				smiPrintError(thisParserPtr,
					      ERR_UNEXPECTED_VALUETYPE);
				yyval.valuePtr = NULL;
			    }
			;}
    break;

  case 302:
#line 3262 "parser-sming.y"
    {
			    /* Note: might also be an OID or signed */
			    switch (defaultBasetype) {
			    case SMI_BASETYPE_UNSIGNED32:
				yyval.valuePtr = smiMalloc(sizeof(SmiValue));
				yyval.valuePtr->basetype = SMI_BASETYPE_UNSIGNED32;
				yyval.valuePtr->value.unsigned32 = strtoul(yyvsp[0].text, NULL, 10);
				break;
			    case SMI_BASETYPE_UNSIGNED64:
				yyval.valuePtr = smiMalloc(sizeof(SmiValue));
				yyval.valuePtr->basetype = SMI_BASETYPE_UNSIGNED64;
				yyval.valuePtr->value.unsigned64 = strtoull(yyvsp[0].text, NULL, 10);
				break;
			    case SMI_BASETYPE_INTEGER32:
				yyval.valuePtr = smiMalloc(sizeof(SmiValue));
				yyval.valuePtr->basetype = SMI_BASETYPE_INTEGER32;
				yyval.valuePtr->value.integer32 = strtol(yyvsp[0].text, NULL, 10);
				break;
			    case SMI_BASETYPE_INTEGER64:
				yyval.valuePtr = smiMalloc(sizeof(SmiValue));
				yyval.valuePtr->basetype = SMI_BASETYPE_INTEGER64;
				yyval.valuePtr->value.integer64 = strtoll(yyvsp[0].text, NULL, 10);
				break;
			    case SMI_BASETYPE_OBJECTIDENTIFIER:
				yyval.valuePtr = smiMalloc(sizeof(SmiValue));
				yyval.valuePtr->basetype = SMI_BASETYPE_OBJECTIDENTIFIER;
				yyval.valuePtr->len = 2;
				yyval.valuePtr->value.oid =
				    smiMalloc(2 * sizeof(SmiSubid));
				yyval.valuePtr->value.oid[0] = 0;
				yyval.valuePtr->value.oid[1] = 0;
				/* TODO */
				break;
			    default:
				smiPrintError(thisParserPtr,
					      ERR_UNEXPECTED_VALUETYPE);
				yyval.valuePtr = NULL;
				break;
			    }
			;}
    break;

  case 303:
#line 3303 "parser-sming.y"
    {
			    switch (defaultBasetype) {
			    case SMI_BASETYPE_INTEGER32:
				yyval.valuePtr = smiMalloc(sizeof(SmiValue));
				yyval.valuePtr->basetype = SMI_BASETYPE_INTEGER32;
				yyval.valuePtr->value.integer32 = - strtol(yyvsp[0].text, NULL, 10);
				break;
			    case SMI_BASETYPE_INTEGER64:
				yyval.valuePtr = smiMalloc(sizeof(SmiValue));
				yyval.valuePtr->basetype = SMI_BASETYPE_INTEGER64;
				yyval.valuePtr->value.integer64 = - strtoll(yyvsp[0].text, NULL, 10);
				break;
			    default:
				smiPrintError(thisParserPtr,
					      ERR_UNEXPECTED_VALUETYPE);
				yyval.valuePtr = NULL;
				break;
			    }
			;}
    break;

  case 304:
#line 3323 "parser-sming.y"
    {
			    /* TODO */
			    /* Note: might also be an octet string */
			    yyval.valuePtr = NULL;
			;}
    break;

  case 305:
#line 3329 "parser-sming.y"
    {
			    /* TODO */
			    /* Note: might also be an OID */
			    yyval.valuePtr = NULL;
			;}
    break;

  case 306:
#line 3335 "parser-sming.y"
    {
			    if (defaultBasetype == SMI_BASETYPE_OCTETSTRING) {
				yyval.valuePtr = smiMalloc(sizeof(SmiValue));
				yyval.valuePtr->basetype = SMI_BASETYPE_OCTETSTRING;
				yyval.valuePtr->value.ptr = yyvsp[0].text;
				yyval.valuePtr->len = strlen(yyvsp[0].text);
			    } else {
				smiPrintError(thisParserPtr,
					      ERR_UNEXPECTED_VALUETYPE);
				yyval.valuePtr = NULL;
			    }
			;}
    break;

  case 307:
#line 3348 "parser-sming.y"
    {
			    /* Note: might be an Enumeration item or OID */
			    /* TODO: convert if it's an oid? */
			    switch (defaultBasetype) {
			    case SMI_BASETYPE_ENUM:
				yyval.valuePtr = smiMalloc(sizeof(SmiValue));
				yyval.valuePtr->basetype = SMI_BASETYPE_ENUM;
				yyval.valuePtr->value.ptr = yyvsp[0].text;
				/* TODO: XXX convert to int */
				break;
			    case SMI_BASETYPE_OBJECTIDENTIFIER:
				yyval.valuePtr = smiMalloc(sizeof(SmiValue));
				yyval.valuePtr->basetype = SMI_BASETYPE_OBJECTIDENTIFIER;
				yyval.valuePtr->value.ptr = yyvsp[0].text;
				/* TODO: XXX convert to oid if found */
				break;
			    default:
				smiPrintError(thisParserPtr,
					      ERR_UNEXPECTED_VALUETYPE);
				yyval.valuePtr = NULL;
				break;
			    }
			;}
    break;

  case 308:
#line 3372 "parser-sming.y"
    {
			    /* TODO */
			    yyval.valuePtr = NULL;
			;}
    break;

  case 309:
#line 3377 "parser-sming.y"
    {
			    /* TODO */
			    yyval.valuePtr = NULL;
			;}
    break;

  case 310:
#line 3384 "parser-sming.y"
    {
			    yyval.status = SMI_STATUS_CURRENT;
			;}
    break;

  case 311:
#line 3388 "parser-sming.y"
    {
			    yyval.status = SMI_STATUS_DEPRECATED;
			;}
    break;

  case 312:
#line 3392 "parser-sming.y"
    {
			    yyval.status = SMI_STATUS_OBSOLETE;
			;}
    break;

  case 313:
#line 3398 "parser-sming.y"
    {
			    yyval.access = SMI_ACCESS_NOT_ACCESSIBLE;
			;}
    break;

  case 314:
#line 3402 "parser-sming.y"
    {
			    yyval.access = SMI_ACCESS_NOTIFY;
			;}
    break;

  case 315:
#line 3406 "parser-sming.y"
    {
			    yyval.access = SMI_ACCESS_READ_ONLY;
			;}
    break;

  case 316:
#line 3410 "parser-sming.y"
    {
			    yyval.access = SMI_ACCESS_READ_WRITE;
			;}
    break;

  case 317:
#line 3416 "parser-sming.y"
    {
			    char *oid = NULL;
			    Node *nodePtr;

			    if (yyvsp[-1].text && yyvsp[0].text) {
				oid = smiMalloc(strlen(yyvsp[-1].text) + strlen(yyvsp[0].text) + 1);
				strcpy(oid, yyvsp[-1].text);
				strcat(oid, yyvsp[0].text);
				free(yyvsp[-1].text);
				free(yyvsp[0].text);
			    } else if (yyvsp[-1].text) {
				oid = smiMalloc(strlen(yyvsp[-1].text) + 1);
				strcpy(oid, yyvsp[-1].text);
				free(yyvsp[-1].text);
			    }
			    
			    if (oid) {
				nodePtr = findNodeByOidString(oid);
				if (!nodePtr) {
				    nodePtr = createNodesByOidString(oid);
				}
				yyval.nodePtr = nodePtr;
			    } else {
				yyval.nodePtr = NULL;
			    }
			;}
    break;

  case 318:
#line 3445 "parser-sming.y"
    {
			    Object *objectPtr;
			    Node *nodePtr;
			    char *s;
			    char ss[20];
			    
			    /* TODO: $1 might be numeric !? */
			    
			    objectPtr = findObject(yyvsp[0].text,
						   thisParserPtr,
						   thisModulePtr);

			    if (objectPtr) {
				/* create OID string */
				nodePtr = objectPtr->nodePtr;
				s = smiMalloc(100);
				sprintf(s, "%u", nodePtr->subid);
				while ((nodePtr->parentPtr) &&
				       (nodePtr->parentPtr != smiHandle->rootNodePtr)) {
				    nodePtr = nodePtr->parentPtr;

				    sprintf(ss, "%u", nodePtr->subid);
				    if (strlen(s) > 80)
					s = smiRealloc(s,
						       strlen(s)+strlen(ss)+2);
				    memmove(&s[strlen(ss)+1], s, strlen(s)+1);
				    strncpy(s, ss, strlen(ss));
				    s[strlen(ss)] = '.';
				}
				yyval.text = smiStrdup(s);
				smiFree(s);
			    } else {
				smiPrintError(thisParserPtr,
					      ERR_UNKNOWN_OIDLABEL, yyvsp[0].text);
				yyval.text = NULL;
			    }
			;}
    break;

  case 319:
#line 3483 "parser-sming.y"
    {
			    yyval.text = yyvsp[0].text;
			;}
    break;

  case 320:
#line 3489 "parser-sming.y"
    {
			    yyval.text = NULL;
			;}
    break;

  case 321:
#line 3493 "parser-sming.y"
    {
			    /* TODO: check upper limit of 127 subids */ 
			    yyval.text = yyvsp[0].text;
			;}
    break;

  case 322:
#line 3500 "parser-sming.y"
    {
			    yyval.text = yyvsp[0].text;
			;}
    break;

  case 323:
#line 3504 "parser-sming.y"
    {
			    yyval.text = smiMalloc(strlen(yyvsp[-1].text) + strlen(yyvsp[0].text) + 1);
			    strcpy(yyval.text, yyvsp[-1].text);
			    strcat(yyval.text, yyvsp[0].text);
			    free(yyvsp[-1].text);
			    free(yyvsp[0].text);
			;}
    break;

  case 324:
#line 3514 "parser-sming.y"
    {
			    yyval.text = smiMalloc(strlen(yyvsp[0].text) + 1 + 1);
			    strcpy(yyval.text, ".");
			    strcat(yyval.text, yyvsp[0].text);
			    free(yyvsp[0].text);
			;}
    break;

  case 325:
#line 3523 "parser-sming.y"
    {
			    yyval.text = smiStrdup(yyvsp[0].text);
			;}
    break;

  case 326:
#line 3529 "parser-sming.y"
    {
			    yyval.valuePtr = smiMalloc(sizeof(SmiValue));
			    yyval.valuePtr->basetype = SMI_BASETYPE_UNSIGNED64;
			    yyval.valuePtr->value.unsigned64 = strtoull(yyvsp[0].text, NULL, 0);
			;}
    break;

  case 327:
#line 3535 "parser-sming.y"
    {
			    yyval.valuePtr = smiMalloc(sizeof(SmiValue));
			    yyval.valuePtr->basetype = SMI_BASETYPE_UNSIGNED64;
			    yyval.valuePtr->value.unsigned64 = strtoull(yyvsp[0].text, NULL, 10);
			;}
    break;

  case 328:
#line 3543 "parser-sming.y"
    {
			    yyval.valuePtr = smiMalloc(sizeof(SmiValue));
			    yyval.valuePtr->basetype = SMI_BASETYPE_INTEGER64;
			    yyval.valuePtr->value.integer64 = - strtoll(yyvsp[0].text, NULL, 10);
			;}
    break;

  case 329:
#line 3551 "parser-sming.y"
    {
			    yyval.valuePtr = yyvsp[0].valuePtr;
			;}
    break;

  case 330:
#line 3555 "parser-sming.y"
    {
			    yyval.valuePtr = yyvsp[0].valuePtr;
			;}
    break;

  case 331:
#line 3565 "parser-sming.y"
    {
			    yyval.rc = 0;
			;}
    break;

  case 332:
#line 3572 "parser-sming.y"
    {
			    yyval.rc = 0;
			;}
    break;

  case 333:
#line 3578 "parser-sming.y"
    {
			    yyval.rc = 0;
			;}
    break;

  case 334:
#line 3584 "parser-sming.y"
    {
			    yyval.rc = 0;
			;}
    break;


    }

/* Line 1010 of yacc.c.  */
#line 5814 "parser-sming.c"

  yyvsp -= yylen;
  yyssp -= yylen;


  YY_STACK_PRINT (yyss, yyssp);

  *++yyvsp = yyval;


  /* Now `shift' the result of the reduction.  Determine what state
     that goes to, based on the state we popped back to and the rule
     number reduced by.  */

  yyn = yyr1[yyn];

  yystate = yypgoto[yyn - YYNTOKENS] + *yyssp;
  if (0 <= yystate && yystate <= YYLAST && yycheck[yystate] == *yyssp)
    yystate = yytable[yystate];
  else
    yystate = yydefgoto[yyn - YYNTOKENS];

  goto yynewstate;


/*------------------------------------.
| yyerrlab -- here on detecting error |
`------------------------------------*/
yyerrlab:
  /* If not already recovering from an error, report this error.  */
  if (!yyerrstatus)
    {
      ++yynerrs;
#if YYERROR_VERBOSE
      yyn = yypact[yystate];

      if (YYPACT_NINF < yyn && yyn < YYLAST)
	{
	  YYSIZE_T yysize = 0;
	  int yytype = YYTRANSLATE (yychar);
	  const char* yyprefix;
	  char *yymsg;
	  int yyx;

	  /* Start YYX at -YYN if negative to avoid negative indexes in
	     YYCHECK.  */
	  int yyxbegin = yyn < 0 ? -yyn : 0;

	  /* Stay within bounds of both yycheck and yytname.  */
	  int yychecklim = YYLAST - yyn;
	  int yyxend = yychecklim < YYNTOKENS ? yychecklim : YYNTOKENS;
	  int yycount = 0;

	  yyprefix = ", expecting ";
	  for (yyx = yyxbegin; yyx < yyxend; ++yyx)
	    if (yycheck[yyx + yyn] == yyx && yyx != YYTERROR)
	      {
		yysize += yystrlen (yyprefix) + yystrlen (yytname [yyx]);
		yycount += 1;
		if (yycount == 5)
		  {
		    yysize = 0;
		    break;
		  }
	      }
	  yysize += (sizeof ("syntax error, unexpected ")
		     + yystrlen (yytname[yytype]));
	  yymsg = (char *) YYSTACK_ALLOC (yysize);
	  if (yymsg != 0)
	    {
	      char *yyp = yystpcpy (yymsg, "syntax error, unexpected ");
	      yyp = yystpcpy (yyp, yytname[yytype]);

	      if (yycount < 5)
		{
		  yyprefix = ", expecting ";
		  for (yyx = yyxbegin; yyx < yyxend; ++yyx)
		    if (yycheck[yyx + yyn] == yyx && yyx != YYTERROR)
		      {
			yyp = yystpcpy (yyp, yyprefix);
			yyp = yystpcpy (yyp, yytname[yyx]);
			yyprefix = " or ";
		      }
		}
	      yyerror (yymsg);
	      YYSTACK_FREE (yymsg);
	    }
	  else
	    yyerror ("syntax error; also virtual memory exhausted");
	}
      else
#endif /* YYERROR_VERBOSE */
	yyerror ("syntax error");
    }



  if (yyerrstatus == 3)
    {
      /* If just tried and failed to reuse lookahead token after an
	 error, discard it.  */

      if (yychar <= YYEOF)
        {
          /* If at end of input, pop the error token,
	     then the rest of the stack, then return failure.  */
	  if (yychar == YYEOF)
	     for (;;)
	       {
		 YYPOPSTACK;
		 if (yyssp == yyss)
		   YYABORT;
		 YYDSYMPRINTF ("Error: popping", yystos[*yyssp], yyvsp, yylsp);
		 yydestruct (yystos[*yyssp], yyvsp);
	       }
        }
      else
	{
	  YYDSYMPRINTF ("Error: discarding", yytoken, &yylval, &yylloc);
	  yydestruct (yytoken, &yylval);
	  yychar = YYEMPTY;

	}
    }

  /* Else will try to reuse lookahead token after shifting the error
     token.  */
  goto yyerrlab1;


/*---------------------------------------------------.
| yyerrorlab -- error raised explicitly by YYERROR.  |
`---------------------------------------------------*/
yyerrorlab:

#ifdef __GNUC__
  /* Pacify GCC when the user code never invokes YYERROR and the label
     yyerrorlab therefore never appears in user code.  */
  if (0)
     goto yyerrorlab;
#endif

  yyvsp -= yylen;
  yyssp -= yylen;
  yystate = *yyssp;
  goto yyerrlab1;


/*-------------------------------------------------------------.
| yyerrlab1 -- common code for both syntax error and YYERROR.  |
`-------------------------------------------------------------*/
yyerrlab1:
  yyerrstatus = 3;	/* Each real token shifted decrements this.  */

  for (;;)
    {
      yyn = yypact[yystate];
      if (yyn != YYPACT_NINF)
	{
	  yyn += YYTERROR;
	  if (0 <= yyn && yyn <= YYLAST && yycheck[yyn] == YYTERROR)
	    {
	      yyn = yytable[yyn];
	      if (0 < yyn)
		break;
	    }
	}

      /* Pop the current state because it cannot handle the error token.  */
      if (yyssp == yyss)
	YYABORT;

      YYDSYMPRINTF ("Error: popping", yystos[*yyssp], yyvsp, yylsp);
      yydestruct (yystos[yystate], yyvsp);
      YYPOPSTACK;
      yystate = *yyssp;
      YY_STACK_PRINT (yyss, yyssp);
    }

  if (yyn == YYFINAL)
    YYACCEPT;

  YYDPRINTF ((stderr, "Shifting error token, "));

  *++yyvsp = yylval;


  yystate = yyn;
  goto yynewstate;


/*-------------------------------------.
| yyacceptlab -- YYACCEPT comes here.  |
`-------------------------------------*/
yyacceptlab:
  yyresult = 0;
  goto yyreturn;

/*-----------------------------------.
| yyabortlab -- YYABORT comes here.  |
`-----------------------------------*/
yyabortlab:
  yyresult = 1;
  goto yyreturn;

#ifndef yyoverflow
/*----------------------------------------------.
| yyoverflowlab -- parser overflow comes here.  |
`----------------------------------------------*/
yyoverflowlab:
  yyerror ("parser stack overflow");
  yyresult = 2;
  /* Fall through.  */
#endif

yyreturn:
#ifndef yyoverflow
  if (yyss != yyssa)
    YYSTACK_FREE (yyss);
#endif
  return yyresult;
}


#line 3589 "parser-sming.y"


#endif
			

