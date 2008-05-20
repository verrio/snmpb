/* A Bison parser, made by GNU Bison 2.3.  */

/* Skeleton implementation for Bison's Yacc-like parsers in C

   Copyright (C) 1984, 1989, 1990, 2000, 2001, 2002, 2003, 2004, 2005, 2006
   Free Software Foundation, Inc.

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
   Foundation, Inc., 51 Franklin Street, Fifth Floor,
   Boston, MA 02110-1301, USA.  */

/* As a special exception, you may create a larger work that contains
   part or all of the Bison parser skeleton and distribute that work
   under terms of your choice, so long as that work isn't itself a
   parser generator using the skeleton or a modified version thereof
   as a parser skeleton.  Alternatively, if you modify or redistribute
   the parser skeleton itself, you may (at your option) remove this
   special exception, which will cause the skeleton and the resulting
   Bison output files to be licensed under the GNU General Public
   License without this special exception.

   This special exception was added by the Free Software Foundation in
   version 2.2 of Bison.  */

/* C LALR(1) parser skeleton written by Richard Stallman, by
   simplifying the original so-called "semantic" parser.  */

/* All symbols defined below should begin with yy or YY, to avoid
   infringing on user name space.  This should be done even for local
   variables, as they might otherwise be expanded by user macros.
   There are some unavoidable exceptions within include files to
   define necessary library symbols; they are noted "INFRINGES ON
   USER NAME SPACE" below.  */

/* Identify Bison output.  */
#define YYBISON 1

/* Bison version.  */
#define YYBISON_VERSION "2.3"

/* Skeleton name.  */
#define YYSKELETON_NAME "yacc.c"

/* Pure parsers.  */
#define YYPURE 1

/* Using locations.  */
#define YYLSP_NEEDED 0

/* Substitute the variable and function names.  */
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
     DOT = 258,
     DOT_DOT = 259,
     COLON_COLON = 260,
     ucIdentifier = 261,
     lcIdentifier = 262,
     floatValue = 263,
     textSegment = 264,
     decimalNumber = 265,
     hexadecimalNumber = 266,
     OID = 267,
     moduleKeyword = 268,
     importKeyword = 269,
     revisionKeyword = 270,
     identityKeyword = 271,
     oidKeyword = 272,
     dateKeyword = 273,
     organizationKeyword = 274,
     contactKeyword = 275,
     descriptionKeyword = 276,
     referenceKeyword = 277,
     extensionKeyword = 278,
     typedefKeyword = 279,
     typeKeyword = 280,
     formatKeyword = 281,
     unitsKeyword = 282,
     statusKeyword = 283,
     accessKeyword = 284,
     defaultKeyword = 285,
     impliedKeyword = 286,
     indexKeyword = 287,
     augmentsKeyword = 288,
     reordersKeyword = 289,
     sparseKeyword = 290,
     expandsKeyword = 291,
     createKeyword = 292,
     membersKeyword = 293,
     objectsKeyword = 294,
     mandatoryKeyword = 295,
     optionalKeyword = 296,
     refineKeyword = 297,
     abnfKeyword = 298,
     OctetStringKeyword = 299,
     ObjectIdentifierKeyword = 300,
     Integer32Keyword = 301,
     Unsigned32Keyword = 302,
     Integer64Keyword = 303,
     Unsigned64Keyword = 304,
     Float32Keyword = 305,
     Float64Keyword = 306,
     Float128Keyword = 307,
     BitsKeyword = 308,
     EnumerationKeyword = 309,
     currentKeyword = 310,
     deprecatedKeyword = 311,
     obsoleteKeyword = 312,
     readonlyKeyword = 313,
     readwriteKeyword = 314,
     parentKeyword = 315,
     classKeyword = 316,
     extendsKeyword = 317,
     attributeKeyword = 318,
     uniqueKeyword = 319,
     eventKeyword = 320,
     PointerKeyword = 321,
     eventonlyKeyword = 322,
     neginfKeyword = 323,
     posinfKeyword = 324,
     snanKeyword = 325,
     qnanKeyword = 326
   };
#endif
/* Tokens.  */
#define DOT 258
#define DOT_DOT 259
#define COLON_COLON 260
#define ucIdentifier 261
#define lcIdentifier 262
#define floatValue 263
#define textSegment 264
#define decimalNumber 265
#define hexadecimalNumber 266
#define OID 267
#define moduleKeyword 268
#define importKeyword 269
#define revisionKeyword 270
#define identityKeyword 271
#define oidKeyword 272
#define dateKeyword 273
#define organizationKeyword 274
#define contactKeyword 275
#define descriptionKeyword 276
#define referenceKeyword 277
#define extensionKeyword 278
#define typedefKeyword 279
#define typeKeyword 280
#define formatKeyword 281
#define unitsKeyword 282
#define statusKeyword 283
#define accessKeyword 284
#define defaultKeyword 285
#define impliedKeyword 286
#define indexKeyword 287
#define augmentsKeyword 288
#define reordersKeyword 289
#define sparseKeyword 290
#define expandsKeyword 291
#define createKeyword 292
#define membersKeyword 293
#define objectsKeyword 294
#define mandatoryKeyword 295
#define optionalKeyword 296
#define refineKeyword 297
#define abnfKeyword 298
#define OctetStringKeyword 299
#define ObjectIdentifierKeyword 300
#define Integer32Keyword 301
#define Unsigned32Keyword 302
#define Integer64Keyword 303
#define Unsigned64Keyword 304
#define Float32Keyword 305
#define Float64Keyword 306
#define Float128Keyword 307
#define BitsKeyword 308
#define EnumerationKeyword 309
#define currentKeyword 310
#define deprecatedKeyword 311
#define obsoleteKeyword 312
#define readonlyKeyword 313
#define readwriteKeyword 314
#define parentKeyword 315
#define classKeyword 316
#define extendsKeyword 317
#define attributeKeyword 318
#define uniqueKeyword 319
#define eventKeyword 320
#define PointerKeyword 321
#define eventonlyKeyword 322
#define neginfKeyword 323
#define posinfKeyword 324
#define snanKeyword 325
#define qnanKeyword 326




/* Copy the first part of user declarations.  */
#line 14 "parser-sming.y"


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

/* Enabling the token table.  */
#ifndef YYTOKEN_TABLE
# define YYTOKEN_TABLE 0
#endif

#if ! defined YYSTYPE && ! defined YYSTYPE_IS_DECLARED
typedef union YYSTYPE
#line 360 "parser-sming.y"
{
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
/* Line 187 of yacc.c.  */
#line 600 "parser-sming.c"
	YYSTYPE;
# define yystype YYSTYPE /* obsolescent; will be withdrawn */
# define YYSTYPE_IS_DECLARED 1
# define YYSTYPE_IS_TRIVIAL 1
#endif



/* Copy the second part of user declarations.  */


/* Line 216 of yacc.c.  */
#line 613 "parser-sming.c"

#ifdef short
# undef short
#endif

#ifdef YYTYPE_UINT8
typedef YYTYPE_UINT8 yytype_uint8;
#else
typedef unsigned char yytype_uint8;
#endif

#ifdef YYTYPE_INT8
typedef YYTYPE_INT8 yytype_int8;
#elif (defined __STDC__ || defined __C99__FUNC__ \
     || defined __cplusplus || defined _MSC_VER)
typedef signed char yytype_int8;
#else
typedef short int yytype_int8;
#endif

#ifdef YYTYPE_UINT16
typedef YYTYPE_UINT16 yytype_uint16;
#else
typedef unsigned short int yytype_uint16;
#endif

#ifdef YYTYPE_INT16
typedef YYTYPE_INT16 yytype_int16;
#else
typedef short int yytype_int16;
#endif

#ifndef YYSIZE_T
# ifdef __SIZE_TYPE__
#  define YYSIZE_T __SIZE_TYPE__
# elif defined size_t
#  define YYSIZE_T size_t
# elif ! defined YYSIZE_T && (defined __STDC__ || defined __C99__FUNC__ \
     || defined __cplusplus || defined _MSC_VER)
#  include <stddef.h> /* INFRINGES ON USER NAME SPACE */
#  define YYSIZE_T size_t
# else
#  define YYSIZE_T unsigned int
# endif
#endif

#define YYSIZE_MAXIMUM ((YYSIZE_T) -1)

#ifndef YY_
# if YYENABLE_NLS
#  if ENABLE_NLS
#   include <libintl.h> /* INFRINGES ON USER NAME SPACE */
#   define YY_(msgid) dgettext ("bison-runtime", msgid)
#  endif
# endif
# ifndef YY_
#  define YY_(msgid) msgid
# endif
#endif

/* Suppress unused-variable warnings by "using" E.  */
#if ! defined lint || defined __GNUC__
# define YYUSE(e) ((void) (e))
#else
# define YYUSE(e) /* empty */
#endif

/* Identity function, used to suppress warnings about constant conditions.  */
#ifndef lint
# define YYID(n) (n)
#else
#if (defined __STDC__ || defined __C99__FUNC__ \
     || defined __cplusplus || defined _MSC_VER)
static int
YYID (int i)
#else
static int
YYID (i)
    int i;
#endif
{
  return i;
}
#endif

#if ! defined yyoverflow || YYERROR_VERBOSE

/* The parser invokes alloca or malloc; define the necessary symbols.  */

# ifdef YYSTACK_USE_ALLOCA
#  if YYSTACK_USE_ALLOCA
#   ifdef __GNUC__
#    define YYSTACK_ALLOC __builtin_alloca
#   elif defined __BUILTIN_VA_ARG_INCR
#    include <alloca.h> /* INFRINGES ON USER NAME SPACE */
#   elif defined _AIX
#    define YYSTACK_ALLOC __alloca
#   elif defined _MSC_VER
#    include <malloc.h> /* INFRINGES ON USER NAME SPACE */
#    define alloca _alloca
#   else
#    define YYSTACK_ALLOC alloca
#    if ! defined _ALLOCA_H && ! defined _STDLIB_H && (defined __STDC__ || defined __C99__FUNC__ \
     || defined __cplusplus || defined _MSC_VER)
#     include <stdlib.h> /* INFRINGES ON USER NAME SPACE */
#     ifndef _STDLIB_H
#      define _STDLIB_H 1
#     endif
#    endif
#   endif
#  endif
# endif

# ifdef YYSTACK_ALLOC
   /* Pacify GCC's `empty if-body' warning.  */
#  define YYSTACK_FREE(Ptr) do { /* empty */; } while (YYID (0))
#  ifndef YYSTACK_ALLOC_MAXIMUM
    /* The OS might guarantee only one guard page at the bottom of the stack,
       and a page size can be as small as 4096 bytes.  So we cannot safely
       invoke alloca (N) if N exceeds 4096.  Use a slightly smaller number
       to allow for a few compiler-allocated temporary stack slots.  */
#   define YYSTACK_ALLOC_MAXIMUM 4032 /* reasonable circa 2006 */
#  endif
# else
#  define YYSTACK_ALLOC YYMALLOC
#  define YYSTACK_FREE YYFREE
#  ifndef YYSTACK_ALLOC_MAXIMUM
#   define YYSTACK_ALLOC_MAXIMUM YYSIZE_MAXIMUM
#  endif
#  if (defined __cplusplus && ! defined _STDLIB_H \
       && ! ((defined YYMALLOC || defined malloc) \
	     && (defined YYFREE || defined free)))
#   include <stdlib.h> /* INFRINGES ON USER NAME SPACE */
#   ifndef _STDLIB_H
#    define _STDLIB_H 1
#   endif
#  endif
#  ifndef YYMALLOC
#   define YYMALLOC malloc
#   if ! defined malloc && ! defined _STDLIB_H && (defined __STDC__ || defined __C99__FUNC__ \
     || defined __cplusplus || defined _MSC_VER)
void *malloc (YYSIZE_T); /* INFRINGES ON USER NAME SPACE */
#   endif
#  endif
#  ifndef YYFREE
#   define YYFREE free
#   if ! defined free && ! defined _STDLIB_H && (defined __STDC__ || defined __C99__FUNC__ \
     || defined __cplusplus || defined _MSC_VER)
void free (void *); /* INFRINGES ON USER NAME SPACE */
#   endif
#  endif
# endif
#endif /* ! defined yyoverflow || YYERROR_VERBOSE */


#if (! defined yyoverflow \
     && (! defined __cplusplus \
	 || (defined YYSTYPE_IS_TRIVIAL && YYSTYPE_IS_TRIVIAL)))

/* A type that is properly aligned for any stack member.  */
union yyalloc
{
  yytype_int16 yyss;
  YYSTYPE yyvs;
  };

/* The size of the maximum gap between one aligned stack and the next.  */
# define YYSTACK_GAP_MAXIMUM (sizeof (union yyalloc) - 1)

/* The size of an array large to enough to hold all stacks, each with
   N elements.  */
# define YYSTACK_BYTES(N) \
     ((N) * (sizeof (yytype_int16) + sizeof (YYSTYPE)) \
      + YYSTACK_GAP_MAXIMUM)

/* Copy COUNT objects from FROM to TO.  The source and destination do
   not overlap.  */
# ifndef YYCOPY
#  if defined __GNUC__ && 1 < __GNUC__
#   define YYCOPY(To, From, Count) \
      __builtin_memcpy (To, From, (Count) * sizeof (*(From)))
#  else
#   define YYCOPY(To, From, Count)		\
      do					\
	{					\
	  YYSIZE_T yyi;				\
	  for (yyi = 0; yyi < (Count); yyi++)	\
	    (To)[yyi] = (From)[yyi];		\
	}					\
      while (YYID (0))
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
    while (YYID (0))

#endif

/* YYFINAL -- State number of the termination state.  */
#define YYFINAL  3
/* YYLAST -- Last index in YYTABLE.  */
#define YYLAST   542

/* YYNTOKENS -- Number of terminals.  */
#define YYNTOKENS  80
/* YYNNTS -- Number of nonterminals.  */
#define YYNNTS  175
/* YYNRULES -- Number of rules.  */
#define YYNRULES  271
/* YYNRULES -- Number of states.  */
#define YYNSTATES  545

/* YYTRANSLATE(YYLEX) -- Bison symbol number corresponding to YYLEX.  */
#define YYUNDEFTOK  2
#define YYMAXUTOK   326

#define YYTRANSLATE(YYX)						\
  ((unsigned int) (YYX) <= YYMAXUTOK ? yytranslate[YYX] : YYUNDEFTOK)

/* YYTRANSLATE[YYLEX] -- Bison symbol number corresponding to YYLEX.  */
static const yytype_uint8 yytranslate[] =
{
       0,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
      75,    76,     2,     2,    78,    79,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,    74,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,    72,    77,    73,     2,     2,     2,     2,
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
      65,    66,    67,    68,    69,    70,    71
};

#if YYDEBUG
/* YYPRHS[YYN] -- Index of the first RHS symbol of rule number YYN in
   YYRHS.  */
static const yytype_uint16 yyprhs[] =
{
       0,     0,     3,     6,     7,     9,    11,    14,    17,    18,
      19,    20,    21,    22,    50,    51,    53,    55,    58,    61,
      62,    63,    64,    65,    66,    85,    86,    88,    90,    93,
      96,    97,    98,    99,   100,   101,   102,   103,   104,   130,
     131,   133,   135,   138,   141,   142,   143,   144,   145,   146,
     147,   170,   171,   173,   175,   178,   181,   182,   183,   184,
     185,   186,   187,   188,   189,   190,   217,   218,   220,   222,
     225,   228,   229,   230,   231,   232,   249,   250,   252,   254,
     257,   260,   261,   262,   276,   277,   279,   281,   284,   287,
     299,   300,   302,   304,   307,   310,   311,   312,   313,   314,
     315,   334,   340,   346,   352,   358,   364,   365,   368,   374,
     375,   378,   384,   385,   388,   394,   395,   398,   406,   408,
     412,   413,   416,   422,   423,   426,   432,   433,   436,   442,
     444,   446,   448,   449,   452,   458,   459,   462,   468,   469,
     472,   478,   479,   482,   488,   490,   492,   494,   496,   499,
     501,   504,   507,   510,   513,   516,   519,   522,   525,   528,
     529,   533,   536,   538,   541,   544,   547,   550,   553,   556,
     559,   562,   565,   566,   570,   573,   576,   577,   580,   582,
     584,   585,   588,   595,   596,   598,   600,   603,   608,   611,
     612,   614,   619,   620,   623,   630,   631,   633,   635,   638,
     643,   646,   647,   649,   654,   656,   658,   660,   662,   663,
     666,   672,   678,   682,   683,   685,   687,   690,   695,   703,
     707,   708,   710,   712,   715,   720,   726,   728,   732,   733,
     735,   737,   740,   745,   747,   749,   753,   755,   759,   761,
     764,   765,   767,   769,   772,   775,   777,   779,   781,   783,
     785,   788,   790,   792,   795,   797,   799,   801,   803,   807,
     809,   811,   813,   815,   817,   819,   822,   824,   826,   827,
     828,   829
};

/* YYRHS -- A `-1'-separated list of the rules' RHS.  */
static const yytype_int16 yyrhs[] =
{
      81,     0,    -1,   253,    82,    -1,    -1,    83,    -1,    84,
      -1,    83,    84,    -1,    85,   253,    -1,    -1,    -1,    -1,
      -1,    -1,    13,   252,     6,    86,   253,    72,   254,   143,
     165,   254,    87,   166,   254,    88,   186,   254,    89,   187,
      90,   149,    91,   153,   100,   112,    73,   253,    74,    -1,
      -1,    92,    -1,    93,    -1,    92,    93,    -1,    94,   254,
      -1,    -1,    -1,    -1,    -1,    -1,    23,   252,     7,    95,
     253,    72,   254,   171,    96,   185,    97,   187,    98,   189,
      99,    73,   253,    74,    -1,    -1,   101,    -1,   102,    -1,
     101,   102,    -1,   103,   254,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    24,   252,     6,   104,   253,    72,
     254,   162,   254,   105,   178,   106,   167,   107,   169,   108,
     171,   109,   185,   110,   187,   111,    73,   253,    74,    -1,
      -1,   113,    -1,   114,    -1,   113,   114,    -1,   115,   254,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    61,   252,     6,
     116,   253,    72,   254,   176,   117,   122,   173,   118,   135,
     171,   119,   185,   120,   187,   121,    73,   253,    74,    -1,
      -1,   123,    -1,   124,    -1,   123,   124,    -1,   125,   254,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      63,   252,     7,   126,   253,    72,   254,   163,   127,   180,
     128,   178,   129,   167,   130,   169,   131,   171,   132,   185,
     133,   187,   134,    73,   253,    74,    -1,    -1,   136,    -1,
     137,    -1,   136,   137,    -1,   138,   254,    -1,    -1,    -1,
      -1,    -1,    65,   252,     7,   139,   253,    72,   254,   171,
     140,   185,   141,   187,   142,    73,   253,    74,    -1,    -1,
     144,    -1,   145,    -1,   144,   145,    -1,   146,   254,    -1,
      -1,    -1,    14,   252,     6,   147,   253,    75,   253,   226,
     148,   253,    76,   253,    74,    -1,    -1,   150,    -1,   151,
      -1,   150,   151,    -1,   152,   254,    -1,    15,   253,    72,
     254,   164,   254,   186,   254,    73,   253,    74,    -1,    -1,
     154,    -1,   155,    -1,   154,   155,    -1,   156,   254,    -1,
      -1,    -1,    -1,    -1,    -1,    16,   252,     7,   157,   253,
      72,   254,   183,   158,   171,   159,   185,   160,   187,   161,
      73,   253,    74,    -1,    25,   252,   191,   253,    74,    -1,
      25,   252,   192,   253,    74,    -1,    18,   252,   242,   253,
      74,    -1,    19,   252,   238,   253,    74,    -1,    20,   252,
     238,   253,    74,    -1,    -1,   168,   254,    -1,    26,   252,
     243,   253,    74,    -1,    -1,   170,   254,    -1,    27,   252,
     244,   253,    74,    -1,    -1,   172,   254,    -1,    28,   252,
     247,   253,    74,    -1,    -1,   174,   254,    -1,    64,   252,
      75,   175,    76,   253,    74,    -1,   251,    -1,     7,   232,
     251,    -1,    -1,   177,   254,    -1,    62,   252,     6,   253,
      74,    -1,    -1,   179,   254,    -1,    30,   252,   245,   253,
      74,    -1,    -1,   181,   254,    -1,    29,   252,   182,   253,
      74,    -1,    58,    -1,    59,    -1,    67,    -1,    -1,   184,
     254,    -1,    60,   252,     7,   253,    74,    -1,    -1,   186,
     254,    -1,    21,   252,   238,   253,    74,    -1,    -1,   188,
     254,    -1,    22,   252,   238,   253,    74,    -1,    -1,   190,
     254,    -1,    43,   252,   238,   253,    74,    -1,   193,    -1,
     197,    -1,   195,    -1,   198,    -1,    44,   201,    -1,    45,
      -1,    46,   201,    -1,    47,   201,    -1,    48,   201,    -1,
      49,   201,    -1,    50,   209,    -1,    51,   209,    -1,    52,
     209,    -1,    66,   218,    -1,    54,   220,    -1,    -1,    53,
     194,   220,    -1,    44,   201,    -1,    45,    -1,    46,   201,
      -1,    47,   201,    -1,    48,   201,    -1,    49,   201,    -1,
      50,   209,    -1,    51,   209,    -1,    52,   209,    -1,    66,
     218,    -1,    54,   220,    -1,    -1,    53,   196,   220,    -1,
     236,   199,    -1,   236,   199,    -1,    -1,   253,   200,    -1,
     202,    -1,   210,    -1,    -1,   253,   202,    -1,    75,   253,
     206,   203,   253,    76,    -1,    -1,   204,    -1,   205,    -1,
     204,   205,    -1,   253,    77,   253,   206,    -1,   250,   207,
      -1,    -1,   208,    -1,   253,     4,   253,   250,    -1,    -1,
     253,   210,    -1,    75,   253,   214,   211,   253,    76,    -1,
      -1,   212,    -1,   213,    -1,   212,   213,    -1,   253,    77,
     253,   214,    -1,     8,   215,    -1,    -1,   216,    -1,   253,
       4,   253,     8,    -1,    68,    -1,    69,    -1,    71,    -1,
      70,    -1,    -1,   253,   219,    -1,    75,   253,   237,   253,
      76,    -1,    75,   253,   221,   253,    76,    -1,   225,   222,
     251,    -1,    -1,   223,    -1,   224,    -1,   223,   224,    -1,
     253,    78,   253,   225,    -1,     7,   253,    75,   253,   250,
     253,    76,    -1,   235,   227,   251,    -1,    -1,   228,    -1,
     229,    -1,   228,   229,    -1,   253,    78,   253,   235,    -1,
      75,   253,   231,   253,    76,    -1,   251,    -1,     7,   232,
     251,    -1,    -1,   233,    -1,   234,    -1,   233,   234,    -1,
     253,    78,   253,     7,    -1,     6,    -1,     7,    -1,     6,
       5,     6,    -1,     6,    -1,     6,     5,     7,    -1,     7,
      -1,     9,   239,    -1,    -1,   240,    -1,   241,    -1,   240,
     241,    -1,   253,     9,    -1,     9,    -1,     9,    -1,     9,
      -1,   230,    -1,    10,    -1,    79,    10,    -1,    11,    -1,
       8,    -1,    79,     8,    -1,   217,    -1,   238,    -1,   237,
      -1,   246,    -1,     6,     5,    12,    -1,    12,    -1,    55,
      -1,    56,    -1,    57,    -1,    11,    -1,    10,    -1,    79,
      10,    -1,   248,    -1,   249,    -1,    -1,    -1,    -1,    -1
};

/* YYRLINE[YYN] -- source line where rule number YYN was defined.  */
static const yytype_uint16 yyrline[] =
{
       0,   611,   611,   622,   625,   631,   635,   650,   665,   695,
     702,   709,   716,   664,   736,   739,   749,   753,   769,   784,
     793,   799,   806,   813,   783,   827,   830,   840,   844,   860,
     875,   889,   909,   932,   941,   947,   953,   960,   874,   978,
     981,   991,   995,  1011,  1026,  1042,  1048,  1091,  1097,  1104,
    1025,  1118,  1121,  1131,  1135,  1151,  1166,  1180,  1191,  1211,
    1228,  1244,  1256,  1261,  1266,  1165,  1279,  1282,  1292,  1296,
    1312,  1327,  1333,  1338,  1343,  1326,  1355,  1358,  1364,  1368,
    1383,  1398,  1403,  1397,  1430,  1433,  1439,  1443,  1458,  1472,
    1482,  1485,  1495,  1499,  1515,  1530,  1538,  1544,  1550,  1557,
    1529,  1568,  1574,  1580,  1586,  1592,  1599,  1602,  1608,  1615,
    1618,  1624,  1631,  1634,  1640,  1647,  1650,  1656,  1662,  1670,
    1682,  1685,  1691,  1702,  1705,  1711,  1718,  1721,  1727,  1733,
    1737,  1741,  1748,  1751,  1757,  1767,  1770,  1776,  1783,  1786,
    1792,  1799,  1802,  1808,  1816,  1820,  1826,  1830,  1837,  1852,
    1856,  1871,  1886,  1901,  1916,  1931,  1946,  1961,  1972,  1988,
    1987,  2016,  2028,  2034,  2047,  2060,  2073,  2086,  2099,  2112,
    2125,  2134,  2148,  2147,  2173,  2187,  2220,  2223,  2229,  2233,
    2240,  2243,  2249,  2259,  2262,  2268,  2274,  2287,  2293,  2308,
    2311,  2317,  2324,  2327,  2333,  2343,  2346,  2352,  2358,  2371,
    2377,  2394,  2397,  2403,  2410,  2414,  2418,  2422,  2429,  2432,
    2441,  2447,  2453,  2463,  2466,  2472,  2478,  2493,  2499,  2507,
    2516,  2519,  2525,  2531,  2544,  2550,  2556,  2560,  2570,  2573,
    2579,  2585,  2598,  2604,  2608,  2614,  2625,  2631,  2640,  2646,
    2666,  2669,  2675,  2679,  2689,  2695,  2701,  2707,  2720,  2740,
    2778,  2798,  2835,  2883,  2924,  2965,  2978,  3011,  3025,  3034,
    3041,  3045,  3049,  3055,  3061,  3069,  3082,  3086,  3097,  3104,
    3110,  3116
};
#endif

#if YYDEBUG || YYERROR_VERBOSE || YYTOKEN_TABLE
/* YYTNAME[SYMBOL-NUM] -- String name of the symbol SYMBOL-NUM.
   First, the terminals, then, starting at YYNTOKENS, nonterminals.  */
static const char *const yytname[] =
{
  "$end", "error", "$undefined", "DOT", "DOT_DOT", "COLON_COLON",
  "ucIdentifier", "lcIdentifier", "floatValue", "textSegment",
  "decimalNumber", "hexadecimalNumber", "OID", "moduleKeyword",
  "importKeyword", "revisionKeyword", "identityKeyword", "oidKeyword",
  "dateKeyword", "organizationKeyword", "contactKeyword",
  "descriptionKeyword", "referenceKeyword", "extensionKeyword",
  "typedefKeyword", "typeKeyword", "formatKeyword", "unitsKeyword",
  "statusKeyword", "accessKeyword", "defaultKeyword", "impliedKeyword",
  "indexKeyword", "augmentsKeyword", "reordersKeyword", "sparseKeyword",
  "expandsKeyword", "createKeyword", "membersKeyword", "objectsKeyword",
  "mandatoryKeyword", "optionalKeyword", "refineKeyword", "abnfKeyword",
  "OctetStringKeyword", "ObjectIdentifierKeyword", "Integer32Keyword",
  "Unsigned32Keyword", "Integer64Keyword", "Unsigned64Keyword",
  "Float32Keyword", "Float64Keyword", "Float128Keyword", "BitsKeyword",
  "EnumerationKeyword", "currentKeyword", "deprecatedKeyword",
  "obsoleteKeyword", "readonlyKeyword", "readwriteKeyword",
  "parentKeyword", "classKeyword", "extendsKeyword", "attributeKeyword",
  "uniqueKeyword", "eventKeyword", "PointerKeyword", "eventonlyKeyword",
  "neginfKeyword", "posinfKeyword", "snanKeyword", "qnanKeyword", "'{'",
  "'}'", "';'", "'('", "')'", "'|'", "','", "'-'", "$accept", "smingFile",
  "moduleStatement_optsep_0n", "moduleStatement_optsep_1n",
  "moduleStatement_optsep", "moduleStatement", "@1", "@2", "@3", "@4",
  "@5", "extensionStatement_stmtsep_0n", "extensionStatement_stmtsep_1n",
  "extensionStatement_stmtsep", "extensionStatement", "@6", "@7", "@8",
  "@9", "@10", "typedefStatement_stmtsep_0n",
  "typedefStatement_stmtsep_1n", "typedefStatement_stmtsep",
  "typedefStatement", "@11", "@12", "@13", "@14", "@15", "@16", "@17",
  "@18", "classStatement_stmtsep_0n", "classStatement_stmtsep_1n",
  "classStatement_stmtsep", "classStatement", "@19", "@20", "@21", "@22",
  "@23", "@24", "attributeStatement_stmtsep_0n",
  "attributeStatement_stmtsep_1n", "attributeStatement_stmtsep",
  "attributeStatement", "@25", "@26", "@27", "@28", "@29", "@30", "@31",
  "@32", "@33", "eventStatement_stmtsep_0n", "eventStatement_stmtsep_1n",
  "eventStatement_stmtsep", "eventStatement", "@34", "@35", "@36", "@37",
  "importStatement_stmtsep_0n", "importStatement_stmtsep_1n",
  "importStatement_stmtsep", "importStatement", "@38", "@39",
  "revisionStatement_stmtsep_0n", "revisionStatement_stmtsep_1n",
  "revisionStatement_stmtsep", "revisionStatement",
  "identityStatement_stmtsep_0n", "identityStatement_stmtsep_1n",
  "identityStatement_stmtsep", "identityStatement", "@40", "@41", "@42",
  "@43", "@44", "typedefTypeStatement", "attributeTypeStatement",
  "dateStatement", "organizationStatement", "contactStatement",
  "formatStatement_stmtsep_01", "formatStatement",
  "unitsStatement_stmtsep_01", "unitsStatement",
  "statusStatement_stmtsep_01", "statusStatement",
  "uniqueStatement_stmtsep_01", "uniqueStatement", "uniqueSpec",
  "extendsStatement_stmtsep_01", "extendsStatement",
  "defaultStatement_stmtsep_01", "defaultStatement",
  "accessStatement_stmtsep_01", "accessStatement", "access",
  "parentStatement_stmtsep_01", "parentStatement",
  "descriptionStatement_stmtsep_01", "descriptionStatement",
  "referenceStatement_stmtsep_01", "referenceStatement",
  "abnfStatement_stmtsep_01", "abnfStatement",
  "refinedBaseType_refinedType", "attribute_refinedBaseType_refinedType",
  "refinedBaseType", "@45", "attribute_refinedBaseType", "@46",
  "refinedType", "attribute_refinedType", "optsep_anySpec_01", "anySpec",
  "optsep_numberSpec_01", "numberSpec", "furtherNumberElement_0n",
  "furtherNumberElement_1n", "furtherNumberElement", "numberElement",
  "numberUpperLimit_01", "numberUpperLimit", "optsep_floatSpec_01",
  "floatSpec", "furtherFloatElement_0n", "furtherFloatElement_1n",
  "furtherFloatElement", "floatElement", "floatUpperLimit_01",
  "floatUpperLimit", "specialFloatValue", "optsep_pointerRestr_01",
  "pointerRestr", "bitsOrEnumerationSpec", "bitsOrEnumerationList",
  "furtherBitsOrEnumerationItem_0n", "furtherBitsOrEnumerationItem_1n",
  "furtherBitsOrEnumerationItem", "bitsOrEnumerationItem",
  "identifierList", "furtherIdentifier_0n", "furtherIdentifier_1n",
  "furtherIdentifier", "bitsValue", "bitsList", "furtherLcIdentifier_0n",
  "furtherLcIdentifier_1n", "furtherLcIdentifier", "identifier",
  "qucIdentifier", "qlcIdentifier", "text", "optsep_textSegment_0n",
  "optsep_textSegment_1n", "optsep_textSegment", "date", "format", "units",
  "anyValue", "qOID", "status", "number", "negativeNumber", "signedNumber",
  "optsep_comma_01", "sep", "optsep", "stmtsep", 0
};
#endif

# ifdef YYPRINT
/* YYTOKNUM[YYLEX-NUM] -- Internal token number corresponding to
   token YYLEX-NUM.  */
static const yytype_uint16 yytoknum[] =
{
       0,   256,   257,   258,   259,   260,   261,   262,   263,   264,
     265,   266,   267,   268,   269,   270,   271,   272,   273,   274,
     275,   276,   277,   278,   279,   280,   281,   282,   283,   284,
     285,   286,   287,   288,   289,   290,   291,   292,   293,   294,
     295,   296,   297,   298,   299,   300,   301,   302,   303,   304,
     305,   306,   307,   308,   309,   310,   311,   312,   313,   314,
     315,   316,   317,   318,   319,   320,   321,   322,   323,   324,
     325,   326,   123,   125,    59,    40,    41,   124,    44,    45
};
# endif

/* YYR1[YYN] -- Symbol number of symbol that rule YYN derives.  */
static const yytype_uint8 yyr1[] =
{
       0,    80,    81,    82,    82,    83,    83,    84,    86,    87,
      88,    89,    90,    85,    91,    91,    92,    92,    93,    95,
      96,    97,    98,    99,    94,   100,   100,   101,   101,   102,
     104,   105,   106,   107,   108,   109,   110,   111,   103,   112,
     112,   113,   113,   114,   116,   117,   118,   119,   120,   121,
     115,   122,   122,   123,   123,   124,   126,   127,   128,   129,
     130,   131,   132,   133,   134,   125,   135,   135,   136,   136,
     137,   139,   140,   141,   142,   138,   143,   143,   144,   144,
     145,   147,   148,   146,   149,   149,   150,   150,   151,   152,
     153,   153,   154,   154,   155,   157,   158,   159,   160,   161,
     156,   162,   163,   164,   165,   166,   167,   167,   168,   169,
     169,   170,   171,   171,   172,   173,   173,   174,   175,   175,
     176,   176,   177,   178,   178,   179,   180,   180,   181,   182,
     182,   182,   183,   183,   184,   185,   185,   186,   187,   187,
     188,   189,   189,   190,   191,   191,   192,   192,   193,   193,
     193,   193,   193,   193,   193,   193,   193,   193,   193,   194,
     193,   195,   195,   195,   195,   195,   195,   195,   195,   195,
     195,   195,   196,   195,   197,   198,   199,   199,   200,   200,
     201,   201,   202,   203,   203,   204,   204,   205,   206,   207,
     207,   208,   209,   209,   210,   211,   211,   212,   212,   213,
     214,   215,   215,   216,   217,   217,   217,   217,   218,   218,
     219,   220,   221,   222,   222,   223,   223,   224,   225,   226,
     227,   227,   228,   228,   229,   230,   231,   231,   232,   232,
     233,   233,   234,   235,   235,   236,   236,   237,   237,   238,
     239,   239,   240,   240,   241,   242,   243,   244,   245,   245,
     245,   245,   245,   245,   245,   245,   245,   245,   246,   246,
     247,   247,   247,   248,   248,   249,   250,   250,   251,   252,
     253,   254
};

/* YYR2[YYN] -- Number of symbols composing right hand side of rule YYN.  */
static const yytype_uint8 yyr2[] =
{
       0,     2,     2,     0,     1,     1,     2,     2,     0,     0,
       0,     0,     0,    27,     0,     1,     1,     2,     2,     0,
       0,     0,     0,     0,    18,     0,     1,     1,     2,     2,
       0,     0,     0,     0,     0,     0,     0,     0,    25,     0,
       1,     1,     2,     2,     0,     0,     0,     0,     0,     0,
      22,     0,     1,     1,     2,     2,     0,     0,     0,     0,
       0,     0,     0,     0,     0,    26,     0,     1,     1,     2,
       2,     0,     0,     0,     0,    16,     0,     1,     1,     2,
       2,     0,     0,    13,     0,     1,     1,     2,     2,    11,
       0,     1,     1,     2,     2,     0,     0,     0,     0,     0,
      18,     5,     5,     5,     5,     5,     0,     2,     5,     0,
       2,     5,     0,     2,     5,     0,     2,     7,     1,     3,
       0,     2,     5,     0,     2,     5,     0,     2,     5,     1,
       1,     1,     0,     2,     5,     0,     2,     5,     0,     2,
       5,     0,     2,     5,     1,     1,     1,     1,     2,     1,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     0,
       3,     2,     1,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     0,     3,     2,     2,     0,     2,     1,     1,
       0,     2,     6,     0,     1,     1,     2,     4,     2,     0,
       1,     4,     0,     2,     6,     0,     1,     1,     2,     4,
       2,     0,     1,     4,     1,     1,     1,     1,     0,     2,
       5,     5,     3,     0,     1,     1,     2,     4,     7,     3,
       0,     1,     1,     2,     4,     5,     1,     3,     0,     1,
       1,     2,     4,     1,     1,     3,     1,     3,     1,     2,
       0,     1,     1,     2,     2,     1,     1,     1,     1,     1,
       2,     1,     1,     2,     1,     1,     1,     1,     3,     1,
       1,     1,     1,     1,     1,     2,     1,     1,     0,     0,
       0,     0
};

/* YYDEFACT[STATE-NAME] -- Default rule to reduce with in state
   STATE-NUM when YYTABLE doesn't specify something else to do.  Zero
   means the default is an error.  */
static const yytype_uint16 yydefact[] =
{
     270,     0,     3,     1,   269,     2,     4,     5,   270,     0,
       6,     7,     8,   270,     0,   271,    76,   269,     0,    77,
      78,   271,     0,   269,   271,    79,    80,    81,     0,     9,
     270,   240,   270,     0,     0,   239,   241,   242,     0,     0,
     269,   271,   270,   243,   244,   104,     0,    10,     0,   270,
       0,   233,   234,    82,   220,     0,   269,   271,   270,   268,
     221,   222,     0,   105,     0,    11,     0,   219,   223,   270,
     270,   138,   270,     0,     0,   269,    12,   271,     0,   224,
     137,     0,    84,   139,    83,   270,   270,    14,    85,    86,
     271,     0,     0,   269,    90,    15,    16,   271,    87,    88,
     140,   271,     0,   269,    25,    91,    92,   271,    17,    18,
       0,    19,     0,   269,    39,    26,    27,   271,    93,    94,
     269,   271,   270,    95,     0,   269,     0,    40,    41,   271,
      28,    29,     0,     0,     0,   270,    30,     0,   270,    42,
      43,   245,   270,   271,   271,     0,   270,    44,     0,     0,
       0,   112,   271,     0,   270,    13,   103,   270,   269,    20,
     271,   132,   271,     0,     0,     0,   135,   113,   269,    96,
     271,     0,   271,    89,   260,   261,   262,   270,    21,   271,
       0,   112,   133,   269,   271,   120,     0,   138,   136,   270,
      97,     0,    31,   269,    45,   271,   114,    22,     0,   135,
     236,   180,   149,   180,   180,   180,   180,   192,   192,   192,
     159,     0,   208,   270,   144,   145,   176,   123,     0,    51,
     121,   141,   134,    98,     0,   148,     0,   150,   151,   152,
     153,   154,     0,   155,   156,     0,   270,   158,   157,     0,
       0,   174,     0,   269,    32,   271,   270,   269,   115,    52,
      53,   271,   269,    23,   271,   138,   235,   270,   181,   270,
     193,   160,     0,   270,   209,   101,   270,   177,   178,   179,
       0,   106,   124,     0,     0,   269,    46,   271,    54,    55,
       0,     0,   142,    99,     0,     0,   270,   270,   213,     0,
       0,     0,   238,   252,   249,   251,   259,   204,   205,   207,
     206,   270,     0,   254,   248,   256,   255,   270,   257,   269,
      33,   271,   122,    56,     0,    66,   116,   270,   270,     0,
     264,   263,     0,   183,   266,   267,   189,   201,   195,     0,
       0,   268,   214,   215,     0,     0,   270,     0,   268,   253,
     250,     0,     0,   109,   107,   270,   268,   269,   112,    67,
      68,   271,     0,     0,   270,   265,   270,   184,   185,     0,
     188,   190,     0,   200,   202,     0,   270,   196,   197,     0,
     270,   211,   212,   216,   270,     0,     0,   237,   258,   228,
     270,   226,   125,   246,   270,   269,    34,   271,     0,   228,
       0,   118,     0,    47,    69,    70,   143,    24,     0,     0,
     186,   270,   270,   270,     0,   198,   270,     0,     0,   210,
     268,   229,   230,     0,     0,     0,     0,   112,   110,   271,
     268,   270,    71,   135,   100,   182,     0,     0,     0,   194,
       0,   270,   217,   227,   231,   270,   225,   108,   247,   270,
      35,     0,   119,     0,   270,    48,   187,   191,   203,   199,
       0,     0,     0,   135,   269,    57,   117,     0,   138,   218,
     232,   111,    36,     0,   126,   271,    49,   138,   180,   162,
     180,   180,   180,   180,   192,   192,   192,   172,     0,   208,
     270,   146,   147,   176,   269,    58,   271,   112,     0,    37,
     161,   163,   164,   165,   166,   167,   168,   169,     0,   171,
     170,     0,   175,     0,   123,   127,    72,   270,     0,   173,
     102,   129,   130,   131,   270,    59,   135,     0,   270,     0,
     106,    73,    50,     0,   128,    60,   138,    38,   109,    74,
      61,     0,   112,   270,    62,     0,   135,    75,    63,   138,
      64,     0,   270,     0,    65
};

/* YYDEFGOTO[NTERM-NUM].  */
static const yytype_int16 yydefgoto[] =
{
      -1,     1,     5,     6,     7,     8,    13,    33,    50,    71,
      82,    94,    95,    96,    97,   122,   166,   187,   221,   281,
     114,   115,   116,   117,   146,   217,   271,   343,   417,   453,
     467,   508,   126,   127,   128,   129,   154,   219,   315,   423,
     458,   488,   248,   249,   250,   251,   345,   464,   504,   520,
     528,   532,   536,   539,   541,   348,   349,   350,   351,   444,
     516,   526,   531,    18,    19,    20,    21,    30,    58,    87,
      88,    89,    90,   104,   105,   106,   107,   135,   181,   199,
     255,   319,   184,   455,   121,    24,    41,   310,   311,   386,
     387,   159,   160,   276,   277,   390,   194,   195,   244,   245,
     485,   486,   514,   169,   170,   178,   179,    76,    77,   253,
     254,   213,   480,   214,   235,   481,   498,   215,   482,   241,
     267,   225,   258,   356,   357,   358,   323,   360,   361,   231,
     260,   366,   367,   368,   328,   363,   364,   303,   238,   264,
     237,   287,   331,   332,   333,   288,    53,    59,    60,    61,
     304,   380,   410,   411,   412,    54,   216,   305,    32,    35,
      36,    37,   142,   384,   439,   307,   308,   177,   324,   325,
     326,    67,     9,   226,    16
};

/* YYPACT[STATE-NUM] -- Index in YYTABLE of the portion describing
   STATE-NUM.  */
#define YYPACT_NINF -391
static const yytype_int16 yypact[] =
{
    -391,    11,     1,  -391,  -391,  -391,     1,  -391,  -391,    35,
    -391,  -391,  -391,  -391,   -54,  -391,    30,  -391,    34,    30,
    -391,  -391,    51,  -391,  -391,  -391,  -391,  -391,    50,  -391,
    -391,    54,  -391,    44,   -10,  -391,    54,  -391,    57,    -7,
    -391,  -391,  -391,  -391,  -391,  -391,    50,  -391,    40,  -391,
      47,  -391,  -391,  -391,     9,    -3,  -391,  -391,  -391,  -391,
       9,  -391,    10,  -391,    50,  -391,     2,  -391,  -391,  -391,
    -391,    60,  -391,    40,    15,  -391,  -391,  -391,    18,  -391,
    -391,    50,    64,  -391,  -391,  -391,  -391,    70,    64,  -391,
    -391,    22,    27,  -391,    67,    70,  -391,  -391,  -391,  -391,
    -391,  -391,    90,  -391,    78,    67,  -391,  -391,  -391,  -391,
      85,  -391,    97,  -391,    45,    78,  -391,  -391,  -391,  -391,
    -391,  -391,  -391,  -391,   103,  -391,    37,    45,  -391,  -391,
    -391,  -391,   104,    47,    42,  -391,  -391,   109,  -391,  -391,
    -391,  -391,  -391,  -391,  -391,    46,  -391,  -391,    43,    52,
      55,    88,  -391,    48,  -391,  -391,  -391,  -391,  -391,  -391,
    -391,    59,  -391,    58,    53,    -5,    47,  -391,  -391,  -391,
    -391,    98,  -391,  -391,  -391,  -391,  -391,  -391,  -391,  -391,
     117,    88,  -391,  -391,  -391,    69,    62,    60,  -391,  -391,
    -391,   447,  -391,  -391,  -391,  -391,  -391,  -391,    63,    47,
     124,    65,  -391,    65,    65,    65,    65,    65,    65,    65,
    -391,    66,    65,  -391,  -391,  -391,    65,   102,   128,    76,
    -391,   100,  -391,  -391,   138,  -391,    74,  -391,  -391,  -391,
    -391,  -391,    75,  -391,  -391,    66,  -391,  -391,  -391,    77,
      79,  -391,    83,  -391,  -391,  -391,  -391,  -391,    87,    76,
    -391,  -391,  -391,  -391,  -391,    60,  -391,  -391,  -391,  -391,
    -391,  -391,   152,  -391,  -391,  -391,  -391,  -391,  -391,  -391,
     386,   134,  -391,    89,   154,  -391,  -391,  -391,  -391,  -391,
      50,    92,  -391,  -391,     5,   159,  -391,  -391,     9,    49,
      12,   157,  -391,  -391,  -391,  -391,  -391,  -391,  -391,  -391,
    -391,  -391,    25,  -391,  -391,  -391,  -391,  -391,  -391,  -391,
    -391,  -391,  -391,  -391,    93,   106,  -391,  -391,  -391,    99,
    -391,  -391,   163,   101,  -391,  -391,   171,   171,   101,   107,
     105,  -391,     9,  -391,   108,   174,  -391,    17,   177,  -391,
    -391,   111,   178,   164,  -391,  -391,   183,  -391,    88,   106,
    -391,  -391,   118,   119,  -391,  -391,  -391,   101,  -391,   120,
    -391,  -391,   190,  -391,  -391,   191,  -391,   101,  -391,   121,
    -391,  -391,  -391,  -391,  -391,   189,   125,  -391,  -391,     9,
    -391,  -391,  -391,  -391,  -391,  -391,  -391,  -391,   130,     9,
     127,  -391,   193,  -391,  -391,  -391,  -391,  -391,   131,   135,
    -391,  -391,  -391,  -391,   139,  -391,  -391,     5,   152,  -391,
    -391,     9,  -391,   126,   141,   132,   201,    88,  -391,  -391,
    -391,  -391,  -391,    47,  -391,  -391,     5,     5,   206,  -391,
     159,  -391,  -391,  -391,  -391,  -391,  -391,  -391,  -391,  -391,
    -391,   194,  -391,   144,  -391,  -391,  -391,  -391,  -391,  -391,
     145,   213,   148,    47,  -391,  -391,  -391,   151,    60,  -391,
    -391,  -391,  -391,   475,   195,  -391,  -391,    60,    65,  -391,
      65,    65,    65,    65,    65,    65,    65,  -391,    66,    65,
    -391,  -391,  -391,    65,  -391,  -391,  -391,    88,   153,  -391,
    -391,  -391,  -391,  -391,  -391,  -391,  -391,  -391,    66,  -391,
    -391,   155,  -391,   -19,   102,  -391,  -391,  -391,   160,  -391,
    -391,  -391,  -391,  -391,  -391,  -391,    47,   158,  -391,   161,
     134,  -391,  -391,   166,  -391,  -391,    60,  -391,   164,  -391,
    -391,   168,    88,  -391,  -391,   169,    47,  -391,  -391,    60,
    -391,   172,  -391,   173,  -391
};

/* YYPGOTO[NTERM-NUM].  */
static const yytype_int16 yypgoto[] =
{
    -391,  -391,  -391,  -391,   219,  -391,  -391,  -391,  -391,  -391,
    -391,  -391,  -391,   133,  -391,  -391,  -391,  -391,  -391,  -391,
    -391,  -391,   136,  -391,  -391,  -391,  -391,  -391,  -391,  -391,
    -391,  -391,  -391,  -391,   115,  -391,  -391,  -391,  -391,  -391,
    -391,  -391,  -391,  -391,    -1,  -391,  -391,  -391,  -391,  -391,
    -391,  -391,  -391,  -391,  -391,  -391,  -391,   -99,  -391,  -391,
    -391,  -391,  -391,  -391,  -391,   233,  -391,  -391,  -391,  -391,
    -391,   165,  -391,  -391,  -391,   149,  -391,  -391,  -391,  -391,
    -391,  -391,  -391,  -391,  -391,  -391,  -391,  -264,  -391,  -268,
    -391,  -179,  -391,  -391,  -391,  -391,  -391,  -391,  -239,  -391,
    -391,  -391,  -391,  -391,  -391,  -192,   -38,  -178,  -391,  -391,
    -391,  -391,  -391,  -391,  -391,  -391,  -391,  -391,  -391,  -216,
    -391,  -130,    26,  -391,  -391,   -88,  -156,  -391,  -391,  -203,
      32,  -391,  -391,   -92,  -154,  -391,  -391,  -391,  -202,  -391,
    -234,  -391,  -391,  -391,   -53,  -126,  -391,  -391,  -391,   218,
    -391,  -391,  -106,  -391,  -127,   212,  -173,     3,   -43,  -391,
    -391,   255,  -391,  -391,  -391,  -391,  -391,  -391,  -391,  -391,
    -390,  -312,   -13,     0,     4
};

/* YYTABLE[YYPACT[STATE-NUM]].  What to do in state STATE-NUM.  If
   positive, shift that token.  If negative, reduce the rule which
   number is the opposite.  If zero, do what YYDEFACT says.
   If YYTABLE_NINF, syntax error.  */
#define YYTABLE_NINF -271
static const yytype_int16 yytable[] =
{
       2,   261,   190,    49,    22,   233,   234,   223,    11,   197,
      28,     3,    57,    14,     4,   320,   321,   431,    15,   372,
     327,    70,   320,   321,   377,    26,   381,    46,    29,   378,
      34,    38,    39,   339,   391,   340,    38,   447,    85,   511,
     512,    12,    48,    64,    17,    47,    51,    52,   513,    55,
     174,   175,   176,    23,    62,   335,   292,    27,    66,    31,
      62,    65,    81,  -270,    40,    42,    44,    45,    56,    73,
      74,    63,    78,   227,   228,   229,   230,   283,    72,    86,
     102,    83,    75,   103,   322,    91,    92,  -270,    69,    80,
     112,   322,    84,    93,    99,   143,   100,   111,   433,   101,
     124,   109,   113,   120,   123,   110,   125,   132,   442,   136,
     138,   119,   137,   141,   144,   147,   158,   155,   152,   168,
     162,   131,   134,   183,   189,   133,   156,   173,   157,   224,
     172,   193,   243,   140,   246,   145,   196,   222,   148,   247,
    -270,   236,   149,   252,   256,   165,   153,   150,   151,   257,
     259,   275,   263,   265,   163,   180,   161,   164,   266,   286,
     309,   313,   337,   312,   167,   318,   171,   327,   346,   393,
     191,   347,   354,   355,   182,  -270,   185,   186,  -270,   375,
     218,   371,   370,   188,   379,   382,   374,   383,   192,   198,
     389,   385,   396,   397,   402,   403,   377,   401,   406,   220,
     422,   409,   419,   421,   435,   424,   437,   232,   232,   232,
     438,   425,   239,   240,   448,   429,   242,   436,   456,   454,
     460,   459,   461,   465,   484,    10,   507,   306,   108,   510,
     270,   445,   522,   518,   274,   524,   262,   317,   440,   280,
     527,   533,   139,   537,   499,   542,   273,   544,   278,   272,
     394,   130,    25,    98,   118,   279,   525,   284,   282,   285,
     530,   462,   314,   289,   509,   515,   290,   502,   268,   400,
     446,   495,   496,   497,   269,   405,   449,   500,    68,   373,
     466,   316,   432,   420,   434,    79,   329,   330,   334,   489,
     483,    43,   336,     0,     0,     0,   342,     0,     0,     0,
       0,   338,     0,     0,     0,     0,     0,   341,   506,     0,
       0,     0,     0,     0,     0,   344,     0,   352,   353,     0,
       0,     0,     0,   359,   521,     0,   362,   365,   369,     0,
       0,     0,   334,     0,   392,     0,   376,     0,   490,     0,
     491,   492,   493,   494,   538,   388,     0,     0,   529,     0,
       0,     0,     0,   534,   398,   395,   399,   359,     0,     0,
       0,   540,     0,     0,     0,     0,   404,   369,     0,     0,
     407,     0,   416,     0,   408,     0,     0,     0,     0,   413,
     414,     0,     0,     0,   415,     0,     0,     0,     0,   413,
       0,   418,   291,   292,   293,    31,   294,   295,   296,     0,
       0,   426,   427,   428,     0,     0,   430,     0,     0,     0,
       0,   413,     0,     0,     0,     0,     0,     0,     0,     0,
       0,   443,     0,   441,     0,     0,     0,     0,     0,     0,
       0,   450,     0,     0,     0,   451,     0,     0,     0,   452,
       0,   463,     0,     0,   457,     0,     0,     0,     0,     0,
       0,     0,     0,   200,   297,   298,   299,   300,     0,     0,
       0,   301,     0,     0,     0,   302,     0,     0,     0,   487,
       0,   503,     0,     0,   232,   232,   232,     0,     0,   239,
     501,   200,     0,   242,     0,     0,     0,     0,     0,     0,
     505,   201,   202,   203,   204,   205,   206,   207,   208,   209,
     210,   211,     0,     0,     0,     0,     0,   517,     0,     0,
       0,     0,     0,   212,   519,     0,     0,     0,   523,   468,
     469,   470,   471,   472,   473,   474,   475,   476,   477,   478,
       0,     0,     0,   535,     0,     0,     0,     0,     0,     0,
       0,   479,   543
};

static const yytype_int16 yycheck[] =
{
       0,   235,   181,    46,    17,   208,   209,   199,     8,   187,
      23,     0,    50,    13,    13,    10,    11,   407,    72,   331,
       8,    64,    10,    11,     7,    21,   338,    40,    24,    12,
      30,    31,    32,     8,   346,    10,    36,   427,    81,    58,
      59,     6,    42,    56,    14,    41,     6,     7,    67,    49,
      55,    56,    57,    19,    54,     6,     7,     6,    58,     9,
      60,    57,    75,     9,    20,    75,     9,    74,    21,    69,
      70,    74,    72,   203,   204,   205,   206,   255,    76,    15,
      93,    77,    22,    16,    79,    85,    86,    78,    78,    74,
     103,    79,    74,    23,    90,   133,    74,     7,   410,    72,
     113,    97,    24,    18,     7,   101,    61,   120,   420,     6,
      73,   107,   125,     9,    72,     6,    28,    74,    72,    60,
      72,   117,   122,    25,     7,   121,    74,    74,    73,     5,
      72,    62,    30,   129,     6,   135,    74,    74,   138,    63,
      75,    75,   142,    43,     6,   158,   146,   143,   144,    75,
      75,    64,    75,    74,   154,   168,   152,   157,    75,     7,
      26,     7,     5,    74,   160,    73,   162,     8,    75,   348,
     183,    65,    73,    10,   170,     4,   172,   177,    77,     5,
     193,    76,    75,   179,     7,    74,    78,     9,   184,   189,
       7,    27,    74,    74,     4,     4,     7,    77,    77,   195,
       7,    76,    72,    76,    78,    74,    74,   207,   208,   209,
       9,    76,   212,   213,     8,    76,   216,    76,    74,    25,
       7,    76,    74,    72,    29,     6,    73,   270,    95,    74,
     243,   423,    74,    73,   247,    74,   236,   280,   417,   252,
      74,    73,   127,    74,   478,    73,   246,    74,   249,   245,
     349,   115,    19,    88,   105,   251,   520,   257,   254,   259,
     528,   453,   275,   263,   498,   504,   266,   483,   242,   357,
     426,   474,   475,   476,   242,   367,   430,   479,    60,   332,
     458,   277,   408,   389,   411,    73,   286,   287,   288,   467,
     463,    36,   289,    -1,    -1,    -1,   309,    -1,    -1,    -1,
      -1,   301,    -1,    -1,    -1,    -1,    -1,   307,   487,    -1,
      -1,    -1,    -1,    -1,    -1,   311,    -1,   317,   318,    -1,
      -1,    -1,    -1,   323,   516,    -1,   326,   327,   328,    -1,
      -1,    -1,   332,    -1,   347,    -1,   336,    -1,   468,    -1,
     470,   471,   472,   473,   536,   345,    -1,    -1,   526,    -1,
      -1,    -1,    -1,   532,   354,   351,   356,   357,    -1,    -1,
      -1,   539,    -1,    -1,    -1,    -1,   366,   367,    -1,    -1,
     370,    -1,   385,    -1,   374,    -1,    -1,    -1,    -1,   379,
     380,    -1,    -1,    -1,   384,    -1,    -1,    -1,    -1,   389,
      -1,   387,     6,     7,     8,     9,    10,    11,    12,    -1,
      -1,   401,   402,   403,    -1,    -1,   406,    -1,    -1,    -1,
      -1,   411,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,   421,    -1,   419,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,   431,    -1,    -1,    -1,   435,    -1,    -1,    -1,   439,
      -1,   454,    -1,    -1,   444,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,     6,    68,    69,    70,    71,    -1,    -1,
      -1,    75,    -1,    -1,    -1,    79,    -1,    -1,    -1,   465,
      -1,   484,    -1,    -1,   474,   475,   476,    -1,    -1,   479,
     480,     6,    -1,   483,    -1,    -1,    -1,    -1,    -1,    -1,
     486,    44,    45,    46,    47,    48,    49,    50,    51,    52,
      53,    54,    -1,    -1,    -1,    -1,    -1,   507,    -1,    -1,
      -1,    -1,    -1,    66,   514,    -1,    -1,    -1,   518,    44,
      45,    46,    47,    48,    49,    50,    51,    52,    53,    54,
      -1,    -1,    -1,   533,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    66,   542
};

/* YYSTOS[STATE-NUM] -- The (internal number of the) accessing
   symbol of state STATE-NUM.  */
static const yytype_uint8 yystos[] =
{
       0,    81,   253,     0,    13,    82,    83,    84,    85,   252,
      84,   253,     6,    86,   253,    72,   254,    14,   143,   144,
     145,   146,   252,    19,   165,   145,   254,     6,   252,   254,
     147,     9,   238,    87,   253,   239,   240,   241,   253,   253,
      20,   166,    75,   241,     9,    74,   252,   254,   253,   238,
      88,     6,     7,   226,   235,   253,    21,   186,   148,   227,
     228,   229,   253,    74,   252,   254,   253,   251,   229,    78,
     238,    89,    76,   253,   253,    22,   187,   188,   253,   235,
      74,   252,    90,   254,    74,   238,    15,   149,   150,   151,
     152,   253,   253,    23,    91,    92,    93,    94,   151,   254,
      74,    72,   252,    16,   153,   154,   155,   156,    93,   254,
     254,     7,   252,    24,   100,   101,   102,   103,   155,   254,
      18,   164,    95,     7,   252,    61,   112,   113,   114,   115,
     102,   254,   252,   254,   253,   157,     6,   252,    73,   114,
     254,     9,   242,   186,    72,   253,   104,     6,   253,   253,
     254,   254,    72,   253,   116,    74,    74,    73,    28,   171,
     172,   254,    72,   253,   253,   252,    96,   254,    60,   183,
     184,   254,    72,    74,    55,    56,    57,   247,   185,   186,
     252,   158,   254,    25,   162,   254,   253,    97,   254,     7,
     171,   252,   254,    62,   176,   177,    74,   187,   253,   159,
       6,    44,    45,    46,    47,    48,    49,    50,    51,    52,
      53,    54,    66,   191,   193,   197,   236,   105,   252,   117,
     254,    98,    74,   185,     5,   201,   253,   201,   201,   201,
     201,   209,   253,   209,   209,   194,    75,   220,   218,   253,
     253,   199,   253,    30,   178,   179,     6,    63,   122,   123,
     124,   125,    43,   189,   190,   160,     6,    75,   202,    75,
     210,   220,   253,    75,   219,    74,    75,   200,   202,   210,
     252,   106,   254,   253,   252,    64,   173,   174,   124,   254,
     252,    99,   254,   187,   253,   253,     7,   221,   225,   253,
     253,     6,     7,     8,    10,    11,    12,    68,    69,    70,
      71,    75,    79,   217,   230,   237,   238,   245,   246,    26,
     167,   168,    74,     7,   252,   118,   254,   238,    73,   161,
      10,    11,    79,   206,   248,   249,   250,     8,   214,   253,
     253,   222,   223,   224,   253,     6,   237,     5,   253,     8,
      10,   253,   252,   107,   254,   126,    75,    65,   135,   136,
     137,   138,   253,   253,    73,    10,   203,   204,   205,   253,
     207,   208,   253,   215,   216,   253,   211,   212,   213,   253,
      75,    76,   251,   224,    78,     5,   253,     7,    12,     7,
     231,   251,    74,     9,   243,    27,   169,   170,   253,     7,
     175,   251,   252,   171,   137,   254,    74,    74,   253,   253,
     205,    77,     4,     4,   253,   213,    77,   253,   253,    76,
     232,   233,   234,   253,   253,   253,   252,   108,   254,    72,
     232,    76,     7,   119,    74,    76,   253,   253,   253,    76,
     253,   250,   225,   251,   234,    78,    76,    74,     9,   244,
     171,   254,   251,   253,   139,   185,   206,   250,     8,   214,
     253,   253,   253,   109,    25,   163,    74,   253,   120,    76,
       7,    74,   185,   252,   127,    72,   187,   110,    44,    45,
      46,    47,    48,    49,    50,    51,    52,    53,    54,    66,
     192,   195,   198,   236,    29,   180,   181,   254,   121,   187,
     201,   201,   201,   201,   201,   209,   209,   209,   196,   220,
     218,   253,   199,   252,   128,   254,   171,    73,   111,   220,
      74,    58,    59,    67,   182,   178,   140,   253,    73,   253,
     129,   185,    74,   253,    74,   167,   141,    74,   130,   187,
     169,   142,   131,    73,   171,   253,   132,    74,   185,   133,
     187,   134,    73,   253,    74
};

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
      YYPOPSTACK (1);						\
      goto yybackup;						\
    }								\
  else								\
    {								\
      yyerror (YY_("syntax error: cannot back up")); \
      YYERROR;							\
    }								\
while (YYID (0))


#define YYTERROR	1
#define YYERRCODE	256


/* YYLLOC_DEFAULT -- Set CURRENT to span from RHS[1] to RHS[N].
   If N is 0, then set CURRENT to the empty location which ends
   the previous symbol: RHS[0] (always defined).  */

#define YYRHSLOC(Rhs, K) ((Rhs)[K])
#ifndef YYLLOC_DEFAULT
# define YYLLOC_DEFAULT(Current, Rhs, N)				\
    do									\
      if (YYID (N))                                                    \
	{								\
	  (Current).first_line   = YYRHSLOC (Rhs, 1).first_line;	\
	  (Current).first_column = YYRHSLOC (Rhs, 1).first_column;	\
	  (Current).last_line    = YYRHSLOC (Rhs, N).last_line;		\
	  (Current).last_column  = YYRHSLOC (Rhs, N).last_column;	\
	}								\
      else								\
	{								\
	  (Current).first_line   = (Current).last_line   =		\
	    YYRHSLOC (Rhs, 0).last_line;				\
	  (Current).first_column = (Current).last_column =		\
	    YYRHSLOC (Rhs, 0).last_column;				\
	}								\
    while (YYID (0))
#endif


/* YY_LOCATION_PRINT -- Print the location on the stream.
   This macro was not mandated originally: define only if we know
   we won't break user code: when these are the locations we know.  */

#ifndef YY_LOCATION_PRINT
# if YYLTYPE_IS_TRIVIAL
#  define YY_LOCATION_PRINT(File, Loc)			\
     fprintf (File, "%d.%d-%d.%d",			\
	      (Loc).first_line, (Loc).first_column,	\
	      (Loc).last_line,  (Loc).last_column)
# else
#  define YY_LOCATION_PRINT(File, Loc) ((void) 0)
# endif
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
} while (YYID (0))

# define YY_SYMBOL_PRINT(Title, Type, Value, Location)			  \
do {									  \
  if (yydebug)								  \
    {									  \
      YYFPRINTF (stderr, "%s ", Title);					  \
      yy_symbol_print (stderr,						  \
		  Type, Value); \
      YYFPRINTF (stderr, "\n");						  \
    }									  \
} while (YYID (0))


/*--------------------------------.
| Print this symbol on YYOUTPUT.  |
`--------------------------------*/

/*ARGSUSED*/
#if (defined __STDC__ || defined __C99__FUNC__ \
     || defined __cplusplus || defined _MSC_VER)
static void
yy_symbol_value_print (FILE *yyoutput, int yytype, YYSTYPE const * const yyvaluep)
#else
static void
yy_symbol_value_print (yyoutput, yytype, yyvaluep)
    FILE *yyoutput;
    int yytype;
    YYSTYPE const * const yyvaluep;
#endif
{
  if (!yyvaluep)
    return;
# ifdef YYPRINT
  if (yytype < YYNTOKENS)
    YYPRINT (yyoutput, yytoknum[yytype], *yyvaluep);
# else
  YYUSE (yyoutput);
# endif
  switch (yytype)
    {
      default:
	break;
    }
}


/*--------------------------------.
| Print this symbol on YYOUTPUT.  |
`--------------------------------*/

#if (defined __STDC__ || defined __C99__FUNC__ \
     || defined __cplusplus || defined _MSC_VER)
static void
yy_symbol_print (FILE *yyoutput, int yytype, YYSTYPE const * const yyvaluep)
#else
static void
yy_symbol_print (yyoutput, yytype, yyvaluep)
    FILE *yyoutput;
    int yytype;
    YYSTYPE const * const yyvaluep;
#endif
{
  if (yytype < YYNTOKENS)
    YYFPRINTF (yyoutput, "token %s (", yytname[yytype]);
  else
    YYFPRINTF (yyoutput, "nterm %s (", yytname[yytype]);

  yy_symbol_value_print (yyoutput, yytype, yyvaluep);
  YYFPRINTF (yyoutput, ")");
}

/*------------------------------------------------------------------.
| yy_stack_print -- Print the state stack from its BOTTOM up to its |
| TOP (included).                                                   |
`------------------------------------------------------------------*/

#if (defined __STDC__ || defined __C99__FUNC__ \
     || defined __cplusplus || defined _MSC_VER)
static void
yy_stack_print (yytype_int16 *bottom, yytype_int16 *top)
#else
static void
yy_stack_print (bottom, top)
    yytype_int16 *bottom;
    yytype_int16 *top;
#endif
{
  YYFPRINTF (stderr, "Stack now");
  for (; bottom <= top; ++bottom)
    YYFPRINTF (stderr, " %d", *bottom);
  YYFPRINTF (stderr, "\n");
}

# define YY_STACK_PRINT(Bottom, Top)				\
do {								\
  if (yydebug)							\
    yy_stack_print ((Bottom), (Top));				\
} while (YYID (0))


/*------------------------------------------------.
| Report that the YYRULE is going to be reduced.  |
`------------------------------------------------*/

#if (defined __STDC__ || defined __C99__FUNC__ \
     || defined __cplusplus || defined _MSC_VER)
static void
yy_reduce_print (YYSTYPE *yyvsp, int yyrule)
#else
static void
yy_reduce_print (yyvsp, yyrule)
    YYSTYPE *yyvsp;
    int yyrule;
#endif
{
  int yynrhs = yyr2[yyrule];
  int yyi;
  unsigned long int yylno = yyrline[yyrule];
  YYFPRINTF (stderr, "Reducing stack by rule %d (line %lu):\n",
	     yyrule - 1, yylno);
  /* The symbols being reduced.  */
  for (yyi = 0; yyi < yynrhs; yyi++)
    {
      fprintf (stderr, "   $%d = ", yyi + 1);
      yy_symbol_print (stderr, yyrhs[yyprhs[yyrule] + yyi],
		       &(yyvsp[(yyi + 1) - (yynrhs)])
		       		       );
      fprintf (stderr, "\n");
    }
}

# define YY_REDUCE_PRINT(Rule)		\
do {					\
  if (yydebug)				\
    yy_reduce_print (yyvsp, Rule); \
} while (YYID (0))

/* Nonzero means print parse trace.  It is left uninitialized so that
   multiple parsers can coexist.  */
int yydebug;
#else /* !YYDEBUG */
# define YYDPRINTF(Args)
# define YY_SYMBOL_PRINT(Title, Type, Value, Location)
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
   YYSTACK_ALLOC_MAXIMUM < YYSTACK_BYTES (YYMAXDEPTH)
   evaluated with infinite-precision integer arithmetic.  */

#ifndef YYMAXDEPTH
# define YYMAXDEPTH 10000
#endif



#if YYERROR_VERBOSE

# ifndef yystrlen
#  if defined __GLIBC__ && defined _STRING_H
#   define yystrlen strlen
#  else
/* Return the length of YYSTR.  */
#if (defined __STDC__ || defined __C99__FUNC__ \
     || defined __cplusplus || defined _MSC_VER)
static YYSIZE_T
yystrlen (const char *yystr)
#else
static YYSIZE_T
yystrlen (yystr)
    const char *yystr;
#endif
{
  YYSIZE_T yylen;
  for (yylen = 0; yystr[yylen]; yylen++)
    continue;
  return yylen;
}
#  endif
# endif

# ifndef yystpcpy
#  if defined __GLIBC__ && defined _STRING_H && defined _GNU_SOURCE
#   define yystpcpy stpcpy
#  else
/* Copy YYSRC to YYDEST, returning the address of the terminating '\0' in
   YYDEST.  */
#if (defined __STDC__ || defined __C99__FUNC__ \
     || defined __cplusplus || defined _MSC_VER)
static char *
yystpcpy (char *yydest, const char *yysrc)
#else
static char *
yystpcpy (yydest, yysrc)
    char *yydest;
    const char *yysrc;
#endif
{
  char *yyd = yydest;
  const char *yys = yysrc;

  while ((*yyd++ = *yys++) != '\0')
    continue;

  return yyd - 1;
}
#  endif
# endif

# ifndef yytnamerr
/* Copy to YYRES the contents of YYSTR after stripping away unnecessary
   quotes and backslashes, so that it's suitable for yyerror.  The
   heuristic is that double-quoting is unnecessary unless the string
   contains an apostrophe, a comma, or backslash (other than
   backslash-backslash).  YYSTR is taken from yytname.  If YYRES is
   null, do not copy; instead, return the length of what the result
   would have been.  */
static YYSIZE_T
yytnamerr (char *yyres, const char *yystr)
{
  if (*yystr == '"')
    {
      YYSIZE_T yyn = 0;
      char const *yyp = yystr;

      for (;;)
	switch (*++yyp)
	  {
	  case '\'':
	  case ',':
	    goto do_not_strip_quotes;

	  case '\\':
	    if (*++yyp != '\\')
	      goto do_not_strip_quotes;
	    /* Fall through.  */
	  default:
	    if (yyres)
	      yyres[yyn] = *yyp;
	    yyn++;
	    break;

	  case '"':
	    if (yyres)
	      yyres[yyn] = '\0';
	    return yyn;
	  }
    do_not_strip_quotes: ;
    }

  if (! yyres)
    return yystrlen (yystr);

  return yystpcpy (yyres, yystr) - yyres;
}
# endif

/* Copy into YYRESULT an error message about the unexpected token
   YYCHAR while in state YYSTATE.  Return the number of bytes copied,
   including the terminating null byte.  If YYRESULT is null, do not
   copy anything; just return the number of bytes that would be
   copied.  As a special case, return 0 if an ordinary "syntax error"
   message will do.  Return YYSIZE_MAXIMUM if overflow occurs during
   size calculation.  */
static YYSIZE_T
yysyntax_error (char *yyresult, int yystate, int yychar)
{
  int yyn = yypact[yystate];

  if (! (YYPACT_NINF < yyn && yyn <= YYLAST))
    return 0;
  else
    {
      int yytype = YYTRANSLATE (yychar);
      YYSIZE_T yysize0 = yytnamerr (0, yytname[yytype]);
      YYSIZE_T yysize = yysize0;
      YYSIZE_T yysize1;
      int yysize_overflow = 0;
      enum { YYERROR_VERBOSE_ARGS_MAXIMUM = 5 };
      char const *yyarg[YYERROR_VERBOSE_ARGS_MAXIMUM];
      int yyx;

# if 0
      /* This is so xgettext sees the translatable formats that are
	 constructed on the fly.  */
      YY_("syntax error, unexpected %s");
      YY_("syntax error, unexpected %s, expecting %s");
      YY_("syntax error, unexpected %s, expecting %s or %s");
      YY_("syntax error, unexpected %s, expecting %s or %s or %s");
      YY_("syntax error, unexpected %s, expecting %s or %s or %s or %s");
# endif
      char *yyfmt;
      char const *yyf;
      static char const yyunexpected[] = "syntax error, unexpected %s";
      static char const yyexpecting[] = ", expecting %s";
      static char const yyor[] = " or %s";
      char yyformat[sizeof yyunexpected
		    + sizeof yyexpecting - 1
		    + ((YYERROR_VERBOSE_ARGS_MAXIMUM - 2)
		       * (sizeof yyor - 1))];
      char const *yyprefix = yyexpecting;

      /* Start YYX at -YYN if negative to avoid negative indexes in
	 YYCHECK.  */
      int yyxbegin = yyn < 0 ? -yyn : 0;

      /* Stay within bounds of both yycheck and yytname.  */
      int yychecklim = YYLAST - yyn + 1;
      int yyxend = yychecklim < YYNTOKENS ? yychecklim : YYNTOKENS;
      int yycount = 1;

      yyarg[0] = yytname[yytype];
      yyfmt = yystpcpy (yyformat, yyunexpected);

      for (yyx = yyxbegin; yyx < yyxend; ++yyx)
	if (yycheck[yyx + yyn] == yyx && yyx != YYTERROR)
	  {
	    if (yycount == YYERROR_VERBOSE_ARGS_MAXIMUM)
	      {
		yycount = 1;
		yysize = yysize0;
		yyformat[sizeof yyunexpected - 1] = '\0';
		break;
	      }
	    yyarg[yycount++] = yytname[yyx];
	    yysize1 = yysize + yytnamerr (0, yytname[yyx]);
	    yysize_overflow |= (yysize1 < yysize);
	    yysize = yysize1;
	    yyfmt = yystpcpy (yyfmt, yyprefix);
	    yyprefix = yyor;
	  }

      yyf = YY_(yyformat);
      yysize1 = yysize + yystrlen (yyf);
      yysize_overflow |= (yysize1 < yysize);
      yysize = yysize1;

      if (yysize_overflow)
	return YYSIZE_MAXIMUM;

      if (yyresult)
	{
	  /* Avoid sprintf, as that infringes on the user's name space.
	     Don't have undefined behavior even if the translation
	     produced a string with the wrong number of "%s"s.  */
	  char *yyp = yyresult;
	  int yyi = 0;
	  while ((*yyp = *yyf) != '\0')
	    {
	      if (*yyp == '%' && yyf[1] == 's' && yyi < yycount)
		{
		  yyp += yytnamerr (yyp, yyarg[yyi++]);
		  yyf += 2;
		}
	      else
		{
		  yyp++;
		  yyf++;
		}
	    }
	}
      return yysize;
    }
}
#endif /* YYERROR_VERBOSE */


/*-----------------------------------------------.
| Release the memory associated to this symbol.  |
`-----------------------------------------------*/

/*ARGSUSED*/
#if (defined __STDC__ || defined __C99__FUNC__ \
     || defined __cplusplus || defined _MSC_VER)
static void
yydestruct (const char *yymsg, int yytype, YYSTYPE *yyvaluep)
#else
static void
yydestruct (yymsg, yytype, yyvaluep)
    const char *yymsg;
    int yytype;
    YYSTYPE *yyvaluep;
#endif
{
  YYUSE (yyvaluep);

  if (!yymsg)
    yymsg = "Deleting";
  YY_SYMBOL_PRINT (yymsg, yytype, yyvaluep, yylocationp);

  switch (yytype)
    {

      default:
	break;
    }
}


/* Prevent warnings from -Wmissing-prototypes.  */

#ifdef YYPARSE_PARAM
#if defined __STDC__ || defined __cplusplus
int yyparse (void *YYPARSE_PARAM);
#else
int yyparse ();
#endif
#else /* ! YYPARSE_PARAM */
#if defined __STDC__ || defined __cplusplus
int yyparse (void);
#else
int yyparse ();
#endif
#endif /* ! YYPARSE_PARAM */






/*----------.
| yyparse.  |
`----------*/

#ifdef YYPARSE_PARAM
#if (defined __STDC__ || defined __C99__FUNC__ \
     || defined __cplusplus || defined _MSC_VER)
int
yyparse (void *YYPARSE_PARAM)
#else
int
yyparse (YYPARSE_PARAM)
    void *YYPARSE_PARAM;
#endif
#else /* ! YYPARSE_PARAM */
#if (defined __STDC__ || defined __C99__FUNC__ \
     || defined __cplusplus || defined _MSC_VER)
int
yyparse (void)
#else
int
yyparse ()

#endif
#endif
{
  /* The look-ahead symbol.  */
int yychar;

/* The semantic value of the look-ahead symbol.  */
YYSTYPE yylval;

/* Number of syntax errors so far.  */
int yynerrs;

  int yystate;
  int yyn;
  int yyresult;
  /* Number of tokens to shift before error messages enabled.  */
  int yyerrstatus;
  /* Look-ahead token as an internal (translated) token number.  */
  int yytoken = 0;
#if YYERROR_VERBOSE
  /* Buffer for error messages, and its allocated size.  */
  char yymsgbuf[128];
  char *yymsg = yymsgbuf;
  YYSIZE_T yymsg_alloc = sizeof yymsgbuf;
#endif

  /* Three stacks and their tools:
     `yyss': related to states,
     `yyvs': related to semantic values,
     `yyls': related to locations.

     Refer to the stacks thru separate pointers, to allow yyoverflow
     to reallocate them elsewhere.  */

  /* The state stack.  */
  yytype_int16 yyssa[YYINITDEPTH];
  yytype_int16 *yyss = yyssa;
  yytype_int16 *yyssp;

  /* The semantic value stack.  */
  YYSTYPE yyvsa[YYINITDEPTH];
  YYSTYPE *yyvs = yyvsa;
  YYSTYPE *yyvsp;



#define YYPOPSTACK(N)   (yyvsp -= (N), yyssp -= (N))

  YYSIZE_T yystacksize = YYINITDEPTH;

  /* The variables used to return semantic value and location from the
     action routines.  */
  YYSTYPE yyval;


  /* The number of symbols on the RHS of the reduced rule.
     Keep to zero when no symbol should be popped.  */
  int yylen = 0;

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
     have just been pushed.  So pushing a state here evens the stacks.  */
  yyssp++;

 yysetstate:
  *yyssp = yystate;

  if (yyss + yystacksize - 1 <= yyssp)
    {
      /* Get the current used size of the three stacks, in elements.  */
      YYSIZE_T yysize = yyssp - yyss + 1;

#ifdef yyoverflow
      {
	/* Give user a chance to reallocate the stack.  Use copies of
	   these so that the &'s don't force the real ones into
	   memory.  */
	YYSTYPE *yyvs1 = yyvs;
	yytype_int16 *yyss1 = yyss;


	/* Each stack pointer address is followed by the size of the
	   data in use in that stack, in bytes.  This used to be a
	   conditional around just the two extra args, but that might
	   be undefined if yyoverflow is a macro.  */
	yyoverflow (YY_("memory exhausted"),
		    &yyss1, yysize * sizeof (*yyssp),
		    &yyvs1, yysize * sizeof (*yyvsp),

		    &yystacksize);

	yyss = yyss1;
	yyvs = yyvs1;
      }
#else /* no yyoverflow */
# ifndef YYSTACK_RELOCATE
      goto yyexhaustedlab;
# else
      /* Extend the stack our own way.  */
      if (YYMAXDEPTH <= yystacksize)
	goto yyexhaustedlab;
      yystacksize *= 2;
      if (YYMAXDEPTH < yystacksize)
	yystacksize = YYMAXDEPTH;

      {
	yytype_int16 *yyss1 = yyss;
	union yyalloc *yyptr =
	  (union yyalloc *) YYSTACK_ALLOC (YYSTACK_BYTES (yystacksize));
	if (! yyptr)
	  goto yyexhaustedlab;
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

  /* Do appropriate processing given the current state.  Read a
     look-ahead token if we need one and don't already have one.  */

  /* First try to decide what to do without reference to look-ahead token.  */
  yyn = yypact[yystate];
  if (yyn == YYPACT_NINF)
    goto yydefault;

  /* Not known => get a look-ahead token if don't already have one.  */

  /* YYCHAR is either YYEMPTY or YYEOF or a valid look-ahead symbol.  */
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
      YY_SYMBOL_PRINT ("Next token is", yytoken, &yylval, &yylloc);
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

  /* Count tokens shifted since error; after three, turn off error
     status.  */
  if (yyerrstatus)
    yyerrstatus--;

  /* Shift the look-ahead token.  */
  YY_SYMBOL_PRINT ("Shifting", yytoken, &yylval, &yylloc);

  /* Discard the shifted token unless it is eof.  */
  if (yychar != YYEOF)
    yychar = YYEMPTY;

  yystate = yyn;
  *++yyvsp = yylval;

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
#line 612 "parser-sming.y"
    {
			    /*
			     * Return the number of successfully
			     * parsed modules.
			     */
			    (yyval.rc) = (yyvsp[(2) - (2)].rc);
			;}
    break;

  case 3:
#line 622 "parser-sming.y"
    {
			    (yyval.rc) = 0;
			;}
    break;

  case 4:
#line 626 "parser-sming.y"
    {
			    (yyval.rc) = (yyvsp[(1) - (1)].rc);
			;}
    break;

  case 5:
#line 632 "parser-sming.y"
    {
			    (yyval.rc) = (yyvsp[(1) - (1)].rc);
			;}
    break;

  case 6:
#line 636 "parser-sming.y"
    {
			    /*
			     * Sum up the number of successfully parsed
			     * modules or return -1, if at least one
			     * module failed.
			     */
			    if (((yyvsp[(1) - (2)].rc) >= 0) && ((yyvsp[(2) - (2)].rc) >= 0)) {
				(yyval.rc) = (yyvsp[(1) - (2)].rc) + (yyvsp[(2) - (2)].rc);
			    } else {
				(yyval.rc) = -1;
			    }
			;}
    break;

  case 7:
#line 651 "parser-sming.y"
    {
			    /*
			     * If we got a (Module *) return rc == 1,
			     * otherwise parsing failed (rc == -1).
			     */
			    if ((yyvsp[(1) - (2)].modulePtr) != NULL) {
				(yyval.rc) = 1;
			    } else {
				(yyval.rc) = -1;
			    }
			;}
    break;

  case 8:
#line 665 "parser-sming.y"
    {
			    thisParserPtr->modulePtr = findModuleByName((yyvsp[(3) - (3)].text));
			    if (!thisParserPtr->modulePtr) {
				thisParserPtr->modulePtr =
				    addModule((yyvsp[(3) - (3)].text),
					      smiStrdup(thisParserPtr->path),
					      0,
					      thisParserPtr);
			    } else {
			        smiPrintError(thisParserPtr,
					      ERR_MODULE_ALREADY_LOADED,
					      (yyvsp[(3) - (3)].text));
				free((yyvsp[(3) - (3)].text));
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
#line 695 "parser-sming.y"
    {
			    if ((yyvsp[(9) - (10)].text)) {
				setModuleOrganization(thisParserPtr->modulePtr,
						      (yyvsp[(9) - (10)].text));
			    }
			;}
    break;

  case 10:
#line 702 "parser-sming.y"
    {
			    if ((yyvsp[(12) - (13)].text)) {
				setModuleContactInfo(thisParserPtr->modulePtr,
						     (yyvsp[(12) - (13)].text));
			    }
			;}
    break;

  case 11:
#line 709 "parser-sming.y"
    {
			    if ((yyvsp[(15) - (16)].text)) {
				setModuleDescription(thisParserPtr->modulePtr,
						     (yyvsp[(15) - (16)].text), thisParserPtr);
			    }
			;}
    break;

  case 12:
#line 716 "parser-sming.y"
    {
			    if ((yyvsp[(18) - (18)].text)) {
				setModuleReference(thisParserPtr->modulePtr,
						   (yyvsp[(18) - (18)].text), thisParserPtr);
			    }
			;}
    break;

  case 13:
#line 728 "parser-sming.y"
    {
			    checkTypes(thisParserPtr, thisModulePtr);
			    
			    (yyval.modulePtr) = thisModulePtr;
			;}
    break;

  case 14:
#line 736 "parser-sming.y"
    {
			    (yyval.rc) = 0;
			;}
    break;

  case 15:
#line 740 "parser-sming.y"
    {
			    /*
			     * Return the number of successfully
			     * parsed extension statements.
			     */
			    (yyval.rc) = (yyvsp[(1) - (1)].rc);
			;}
    break;

  case 16:
#line 750 "parser-sming.y"
    {
			    (yyval.rc) = (yyvsp[(1) - (1)].rc);
			;}
    break;

  case 17:
#line 755 "parser-sming.y"
    {
			    /*
			     * Sum up the number of successfully parsed
			     * extensions or return -1, if at least one
			     * failed.
			     */
			    if (((yyvsp[(1) - (2)].rc) >= 0) && ((yyvsp[(2) - (2)].rc) >= 0)) {
				(yyval.rc) = (yyvsp[(1) - (2)].rc) + (yyvsp[(2) - (2)].rc);
			    } else {
				(yyval.rc) = -1;
			    }
			;}
    break;

  case 18:
#line 770 "parser-sming.y"
    {
			    /*
			     * If we got a (Type *) return rc == 1,
			     * otherwise parsing failed (rc == -1).
			     */
			    if ((yyvsp[(1) - (2)].macroPtr)) {
				(yyval.rc) = 1;
			    } else {
				(yyval.rc) = -1;
			    }
			;}
    break;

  case 19:
#line 784 "parser-sming.y"
    {
			    macroIdentifier = (yyvsp[(3) - (3)].text);
			    macroPtr = addMacro(macroIdentifier,
						0,
						thisParserPtr);
			    setMacroDecl(macroPtr, SMI_DECL_EXTENSION);
			;}
    break;

  case 20:
#line 793 "parser-sming.y"
    {
			    if (macroPtr && (yyvsp[(8) - (8)].status)) {
				setMacroStatus(macroPtr, (yyvsp[(8) - (8)].status));
			    }
			;}
    break;

  case 21:
#line 799 "parser-sming.y"
    {
			    if (macroPtr && (yyvsp[(10) - (10)].text)) {
				setMacroDescription(macroPtr, (yyvsp[(10) - (10)].text),
						    thisParserPtr);
			    }
			;}
    break;

  case 22:
#line 806 "parser-sming.y"
    {
			    if (macroPtr && (yyvsp[(12) - (12)].text)) {
				setMacroReference(macroPtr, (yyvsp[(12) - (12)].text),
						  thisParserPtr);
			    }
			;}
    break;

  case 23:
#line 813 "parser-sming.y"
    {
			   if (macroPtr && (yyvsp[(14) - (14)].text)) {
				setMacroAbnf(macroPtr, (yyvsp[(14) - (14)].text),
						  thisParserPtr);
			   }
			;}
    break;

  case 24:
#line 820 "parser-sming.y"
    {
			    (yyval.macroPtr) = 0;
			    macroPtr = NULL;
			;}
    break;

  case 25:
#line 827 "parser-sming.y"
    {
			    (yyval.rc) = 0;
			;}
    break;

  case 26:
#line 831 "parser-sming.y"
    {
			    /*
			     * Return the number of successfully
			     * parsed typedef statements.
			     */
			    (yyval.rc) = (yyvsp[(1) - (1)].rc);
			;}
    break;

  case 27:
#line 841 "parser-sming.y"
    {
			    (yyval.rc) = (yyvsp[(1) - (1)].rc);
			;}
    break;

  case 28:
#line 846 "parser-sming.y"
    {
			    /*
			     * Sum up the number of successfully parsed
			     * typedefs or return -1, if at least one
			     * failed.
			     */
			    if (((yyvsp[(1) - (2)].rc) >= 0) && ((yyvsp[(2) - (2)].rc) >= 0)) {
				(yyval.rc) = (yyvsp[(1) - (2)].rc) + (yyvsp[(2) - (2)].rc);
			    } else {
				(yyval.rc) = -1;
			    }
			;}
    break;

  case 29:
#line 861 "parser-sming.y"
    {
			    /*
			     * If we got a (Type *) return rc == 1,
			     * otherwise parsing failed (rc == -1).
			     */
			    if ((yyvsp[(1) - (2)].typePtr)) {
				(yyval.rc) = 1;
			    } else {
				(yyval.rc) = -1;
			    }
			;}
    break;

  case 30:
#line 875 "parser-sming.y"
    {
			    typeIdentifier = (yyvsp[(3) - (3)].text);
			    /* 
			     *check for duplicate names in the module 
			     */
			    if(typePtr = 
			    	findType(typeIdentifier, thisParserPtr,thisModulePtr)) 
					if( typePtr->modulePtr == thisParserPtr->modulePtr)
					       smiPrintError(thisParserPtr,
										ERR_DUPLICATE_TYPE_NAME,
					      				typeIdentifier);
			;}
    break;

  case 31:
#line 889 "parser-sming.y"
    {
			    if ((yyvsp[(8) - (9)].typePtr)) {
				if ((yyvsp[(8) - (9)].typePtr)->export.name) {
				    /*
				     * If the exact type has been found
				     * and no new Type structure has been
				     * created, we have to create a
				     * duplicate now.
				     */
				    typePtr = duplicateType((yyvsp[(8) - (9)].typePtr), 0,
							    thisParserPtr);
				} else {
				    typePtr = (yyvsp[(8) - (9)].typePtr);
				}
				typePtr = setTypeName(typePtr, typeIdentifier);
				setTypeDecl(typePtr, SMI_DECL_TYPEDEF);
			    }
			    defaultBasetype = typePtr->export.basetype;
			;}
    break;

  case 32:
#line 909 "parser-sming.y"
    {
			    if (typePtr && (yyvsp[(11) - (11)].valuePtr)) {
			    	if(typePtr->export.basetype == 
			    					SMI_BASETYPE_ENUM) 
			    					//check ENUM value for correctness
    				{
    					if((yyvsp[(11) - (11)].valuePtr)->len)
    	 				if(namedNumberPtr = findNamedNumberByName(typePtr,
    	 												 (yyvsp[(11) - (11)].valuePtr)->value.ptr)){
    	 					smiFree((yyvsp[(11) - (11)].valuePtr));
    	 					(yyvsp[(11) - (11)].valuePtr) = &namedNumberPtr->export.value;
    	 				}
    	 				else{ smiPrintError(thisParserPtr,
					      		ERR_ENUM_NAME_NOT_DEFINED,
					      		(yyvsp[(11) - (11)].valuePtr)->value.ptr);
					    }
    	 			}
    	 			//NOTE that the bits default value is set in the anyval
    	 			//rule
				setTypeValue(typePtr, (yyvsp[(11) - (11)].valuePtr));
			    }
			;}
    break;

  case 33:
#line 932 "parser-sming.y"
    {
			    if (typePtr && (yyvsp[(13) - (13)].text)
				&& smiCheckFormat(thisParserPtr,
						  typePtr->export.basetype,
						  (yyvsp[(13) - (13)].text), 0)) {
				setTypeFormat(typePtr, (yyvsp[(13) - (13)].text));
			    }
			;}
    break;

  case 34:
#line 941 "parser-sming.y"
    {
			    if (typePtr && (yyvsp[(15) - (15)].text)) {
				setTypeUnits(typePtr, (yyvsp[(15) - (15)].text));
			    }
			;}
    break;

  case 35:
#line 947 "parser-sming.y"
    {
			    if (typePtr && (yyvsp[(17) - (17)].status)) {
				setTypeStatus(typePtr, (yyvsp[(17) - (17)].status));
			    }
			;}
    break;

  case 36:
#line 953 "parser-sming.y"
    {
			    if (typePtr && (yyvsp[(19) - (19)].text)) {
				setTypeDescription(typePtr, (yyvsp[(19) - (19)].text),
						   thisParserPtr);
			    }
			;}
    break;

  case 37:
#line 960 "parser-sming.y"
    {
			    if (typePtr && (yyvsp[(21) - (21)].text)) {
				setTypeReference(typePtr, (yyvsp[(21) - (21)].text), thisParserPtr);
			    }
			;}
    break;

  case 38:
#line 966 "parser-sming.y"
    {
			    smiCheckNamedNumberSubtyping(thisParserPtr , typePtr);
			    smiCheckNamedNumbersOrder(thisParserPtr , typePtr);
			    smiCheckNamedNumberRedefinition(thisParserPtr , typePtr);
			    (yyval.typePtr) = 0;
			    typePtr = NULL;
			    free(typeIdentifier);
			    defaultBasetype = SMI_BASETYPE_UNKNOWN;
			;}
    break;

  case 39:
#line 978 "parser-sming.y"
    {
			    (yyval.rc) = 0;
			;}
    break;

  case 40:
#line 982 "parser-sming.y"
    {
			    /*
			     * Return the number of successfully
			     * parsed typedef statements.
			     */
			    (yyval.rc) = (yyvsp[(1) - (1)].rc);
			;}
    break;

  case 41:
#line 992 "parser-sming.y"
    {
			    (yyval.rc) = (yyvsp[(1) - (1)].rc);
			;}
    break;

  case 42:
#line 997 "parser-sming.y"
    {
			    /*
			     * Sum up the number of successfully parsed
			     * classes or return -1, if at least one
			     * failed.
			     */
			    if (((yyvsp[(1) - (2)].rc) >= 0) && ((yyvsp[(2) - (2)].rc) >= 0)) {
				(yyval.rc) = (yyvsp[(1) - (2)].rc) + (yyvsp[(2) - (2)].rc);
			    } else {
				(yyval.rc) = -1;
			    }
			;}
    break;

  case 43:
#line 1012 "parser-sming.y"
    {
			    /*
			     * If we got a (Type *) return rc == 1,
			     * otherwise parsing failed (rc == -1).
			     */
			    if ((yyvsp[(1) - (2)].classPtr)) {
				(yyval.rc) = 1;
			    } else {
				(yyval.rc) = -1;
			    }
			;}
    break;

  case 44:
#line 1026 "parser-sming.y"
    {
			    classIdentifier = (yyvsp[(3) - (3)].text);
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
			;}
    break;

  case 45:
#line 1042 "parser-sming.y"
    {
				if(classPtr && (yyvsp[(8) - (8)].classPtr))
					classPtr->parentPtr = (yyvsp[(8) - (8)].classPtr);
			;}
    break;

  case 46:
#line 1048 "parser-sming.y"
    {
				List *tmpList;
				Attribute *tmpAttribute;
				if(classPtr && (yyvsp[(11) - (11)].listPtr))
				{
					
					//Check for "magic" value #@# that defines
					//scalar class. See NOTE after Class definitino in data.h
					if(!strcmp((char*)((yyvsp[(11) - (11)].listPtr)->ptr),"#@#"))
					{	
						classPtr->uniqueList = (List*)malloc(sizeof(List));
						classPtr->uniqueList->ptr = classPtr;
						classPtr->uniqueList->nextPtr = NULL;
						smiFree((yyvsp[(11) - (11)].listPtr));
					}
					else
					{
						tmpList = (yyvsp[(11) - (11)].listPtr);
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
						
						classPtr->uniqueList = (yyvsp[(11) - (11)].listPtr);
					}
				}
			;}
    break;

  case 47:
#line 1091 "parser-sming.y"
    {
			    if (classPtr && (yyvsp[(14) - (14)].status)) {
				setClassStatus(classPtr, (yyvsp[(14) - (14)].status));
			    }
			;}
    break;

  case 48:
#line 1097 "parser-sming.y"
    {
			    if (classPtr && (yyvsp[(16) - (16)].text)) {
				setClassDescription(classPtr, (yyvsp[(16) - (16)].text),
						    thisParserPtr);
			    }
			;}
    break;

  case 49:
#line 1104 "parser-sming.y"
    {
			    if (classPtr && (yyvsp[(18) - (18)].text)) {
				setClassReference(classPtr, (yyvsp[(18) - (18)].text),
						  thisParserPtr);
			    }
			;}
    break;

  case 50:
#line 1111 "parser-sming.y"
    {
			    (yyval.classPtr) = 0;
			    classPtr = NULL;
			;}
    break;

  case 51:
#line 1118 "parser-sming.y"
    {
			    (yyval.rc) = 0;
			;}
    break;

  case 52:
#line 1122 "parser-sming.y"
    {
			    /*
			     * Return the number of successfully
			     * parsed typedef statements.
			     */
			    (yyval.rc) = (yyvsp[(1) - (1)].rc);
			;}
    break;

  case 53:
#line 1132 "parser-sming.y"
    {
			    (yyval.rc) = (yyvsp[(1) - (1)].rc);
			;}
    break;

  case 54:
#line 1137 "parser-sming.y"
    {
			    /*
			     * Sum up the number of successfully parsed
			     * attributes or return -1, if at least one
			     * failed.
			     */
			    if (((yyvsp[(1) - (2)].rc) >= 0) && ((yyvsp[(2) - (2)].rc) >= 0)) {
				(yyval.rc) = (yyvsp[(1) - (2)].rc) + (yyvsp[(2) - (2)].rc);
			    } else {
				(yyval.rc) = -1;
			    }
			;}
    break;

  case 55:
#line 1152 "parser-sming.y"
    {
			    /*
			     * If we got a (Type *) return rc == 1,
			     * otherwise parsing failed (rc == -1).
			     */
			    if ((yyvsp[(1) - (2)].attributePtr)) {
				(yyval.rc) = 1;
			    } else {
				(yyval.rc) = -1;
			    }
			;}
    break;

  case 56:
#line 1166 "parser-sming.y"
    {
						attributeIdentifier = (yyvsp[(3) - (3)].text);
						attributePtr = (Attribute*)smiGetAttribute(&(classPtr->export),
												attributeIdentifier);
						if(attributePtr)
						{
							smiPrintError(thisParserPtr,
										ERR_DUPLICATE_ATTRIBUTE_NAME,
					      				attributeIdentifier);
					      	attributePtr = NULL;
						}					
					;}
    break;

  case 57:
#line 1180 "parser-sming.y"
    {
			    		if ((yyvsp[(8) - (8)].attributePtr)) {
				    		attributePtr = (yyvsp[(8) - (8)].attributePtr);
							setAttributeName(attributePtr, 
											attributeIdentifier);
							setAttributeDecl(attributePtr, 
												SMI_DECL_ATTRIBUTE);
							defaultBasetype = attributePtr->export.basetype;
			    		}
					;}
    break;

  case 58:
#line 1191 "parser-sming.y"
    {
						if((yyvsp[(10) - (10)].access) && attributePtr){
						/* check whether there's access for class reference, 
						   which is not allowed */
							if(attributePtr->parentClassPtr)
								smiPrintError(thisParserPtr,
										ERR_ATTRIBUTE_CLASS_ACCESS,
					      							attributeIdentifier);
					      	else {
					      		setAttributeAccess(attributePtr,(yyvsp[(10) - (10)].access));
					      	}
						}
						else if(attributePtr && attributePtr->parentTypePtr)
						{
							smiPrintError(thisParserPtr,
										ERR_ATTRIBUTE_MISSING_ACCESS,
					      							attributeIdentifier);
						}
					;}
    break;

  case 59:
#line 1211 "parser-sming.y"
    {
						if((yyvsp[(12) - (12)].valuePtr) && attributePtr){
							if(attributePtr->parentClassPtr)
								smiPrintError(thisParserPtr,
										ERR_ATTRIBUTE_CLASS_DEFAULT,
					      							attributeIdentifier);
					      	else{
					      		attributePtr->export.value = *(yyvsp[(12) - (12)].valuePtr);
					      	}
						}
						else if(attributePtr)
						{
							attributePtr->export.value.basetype = 
													SMI_BASETYPE_UNKNOWN;
						}
					;}
    break;

  case 60:
#line 1228 "parser-sming.y"
    {
			   			if((yyvsp[(14) - (14)].text) && attributePtr){
							if(attributePtr->parentClassPtr)
								smiPrintError(thisParserPtr,
										ERR_ATTRIBUTE_CLASS_FORMAT,
					      							attributeIdentifier);
					      	else{
					      		if (smiCheckFormat(thisParserPtr,
						  				attributePtr->export.basetype,(yyvsp[(14) - (14)].text), 0)) 
						  		{
									attributePtr->export.format = (yyvsp[(14) - (14)].text);
					      		}
					      	}
						}
			    	;}
    break;

  case 61:
#line 1244 "parser-sming.y"
    {
			    		if((yyvsp[(16) - (16)].text) && attributePtr){
							if(attributePtr->parentClassPtr)
								smiPrintError(thisParserPtr,
										ERR_ATTRIBUTE_CLASS_UNITS,
					      							attributeIdentifier);
					      	else{
									attributePtr->export.units = (yyvsp[(16) - (16)].text);
					      	}
						}
					;}
    break;

  case 62:
#line 1256 "parser-sming.y"
    {
			   			if (attributePtr && (yyvsp[(18) - (18)].status)) 
							attributePtr->export.status = (yyvsp[(18) - (18)].status);
					;}
    break;

  case 63:
#line 1261 "parser-sming.y"
    {
						if (attributePtr && (yyvsp[(20) - (20)].text)) 
							attributePtr->export.description = (yyvsp[(20) - (20)].text);
					;}
    break;

  case 64:
#line 1266 "parser-sming.y"
    {
						if (attributePtr && (yyvsp[(22) - (22)].text)) 
							attributePtr->export.reference = (yyvsp[(22) - (22)].text);
			    	;}
    break;

  case 65:
#line 1271 "parser-sming.y"
    {
						(yyval.attributePtr) = attributePtr;
						attributePtr = NULL;
						defaultBasetype = SMI_BASETYPE_UNKNOWN;
					;}
    break;

  case 66:
#line 1279 "parser-sming.y"
    {
			    (yyval.rc) = 0;
			;}
    break;

  case 67:
#line 1283 "parser-sming.y"
    {
			    /*
			     * Return the number of successfully
			     * parsed event statements.
			     */
			    (yyval.rc) = (yyvsp[(1) - (1)].rc);
			;}
    break;

  case 68:
#line 1293 "parser-sming.y"
    {
			    (yyval.rc) = (yyvsp[(1) - (1)].rc);
			;}
    break;

  case 69:
#line 1298 "parser-sming.y"
    {
			    /*
			     * Sum up the number of successfully parsed
			     * events or return -1, if at least one
			     * failed.
			     */
			    if (((yyvsp[(1) - (2)].rc) >= 0) && ((yyvsp[(2) - (2)].rc) >= 0)) {
				(yyval.rc) = (yyvsp[(1) - (2)].rc) + (yyvsp[(2) - (2)].rc);
			    } else {
				(yyval.rc) = -1;
			    }
			;}
    break;

  case 70:
#line 1313 "parser-sming.y"
    {
			    /*
			     * If we got a (Type *) return rc == 1,
			     * otherwise parsing failed (rc == -1).
			     */
			    if ((yyvsp[(1) - (2)].eventPtr)) {
				(yyval.rc) = 1;
			    } else {
				(yyval.rc) = -1;
			    }
			;}
    break;

  case 71:
#line 1327 "parser-sming.y"
    {
				//TODO check for repeated names
				eventPtr=addEvent((yyvsp[(3) - (3)].text),classPtr,thisParserPtr);
			;}
    break;

  case 72:
#line 1333 "parser-sming.y"
    {
				if((yyvsp[(8) - (8)].status) && eventPtr)
				eventPtr->export.status = (yyvsp[(8) - (8)].status);
			;}
    break;

  case 73:
#line 1338 "parser-sming.y"
    {
				if((yyvsp[(10) - (10)].text) && eventPtr)
				eventPtr->export.description = (yyvsp[(10) - (10)].text);
			;}
    break;

  case 74:
#line 1343 "parser-sming.y"
    {
				if((yyvsp[(12) - (12)].text) && eventPtr)
				eventPtr->export.reference = (yyvsp[(12) - (12)].text);
			;}
    break;

  case 75:
#line 1348 "parser-sming.y"
    {
				(yyval.eventPtr) = eventPtr;
				eventPtr = NULL;
			;}
    break;

  case 76:
#line 1355 "parser-sming.y"
    {
			    (yyval.rc) = 0;
			;}
    break;

  case 77:
#line 1359 "parser-sming.y"
    {
			    (yyval.rc) = (yyvsp[(1) - (1)].rc);
			;}
    break;

  case 78:
#line 1365 "parser-sming.y"
    {
			    (yyval.rc) = (yyvsp[(1) - (1)].rc);
			;}
    break;

  case 79:
#line 1369 "parser-sming.y"
    {
			    /*
			     * Sum up the number of successfully parsed
			     * imports or return -1, if at least one
			     * module failed.
			     */
			    if (((yyvsp[(1) - (2)].rc) >= 0) && ((yyvsp[(2) - (2)].rc) >= 0)) {
				(yyval.rc) = (yyvsp[(1) - (2)].rc) + (yyvsp[(2) - (2)].rc);
			    } else {
				(yyval.rc) = -1;
			    }
			;}
    break;

  case 80:
#line 1384 "parser-sming.y"
    {
			    /*
			     * If we got an (Object *) return rc == 1,
			     * otherwise parsing failed (rc == -1).
			     */
			    if ((yyvsp[(1) - (2)].listPtr)) {
				(yyval.rc) = 1;
			    } else {
				(yyval.rc) = -1;
			    }
			;}
    break;

  case 81:
#line 1398 "parser-sming.y"
    {
			    importModulename = smiStrdup((yyvsp[(3) - (3)].text));
			;}
    break;

  case 82:
#line 1403 "parser-sming.y"
    {
			    List *listPtr, *nextPtr;
			    
			    for (listPtr = (yyvsp[(8) - (8)].listPtr); listPtr; listPtr = nextPtr) {
				addImport(listPtr->ptr, thisParserPtr);
				thisParserPtr->modulePtr->
				                      numImportedIdentifiers++;
				nextPtr = listPtr->nextPtr;
				free(listPtr);
			    }
			;}
    break;

  case 83:
#line 1415 "parser-sming.y"
    {
			    Module *modulePtr;
			    char *s = importModulename;

			    modulePtr = findModuleByName(s);
			    if (!modulePtr) {
				modulePtr = loadModule(s, thisParserPtr);
			    }
			    checkImports(modulePtr, thisParserPtr);
			    free(s);
			    (yyval.listPtr) = NULL;
			;}
    break;

  case 84:
#line 1430 "parser-sming.y"
    {
			    (yyval.rc) = 0;
			;}
    break;

  case 85:
#line 1434 "parser-sming.y"
    {
			    (yyval.rc) = (yyvsp[(1) - (1)].rc);
			;}
    break;

  case 86:
#line 1440 "parser-sming.y"
    {
			    (yyval.rc) = (yyvsp[(1) - (1)].rc);
			;}
    break;

  case 87:
#line 1444 "parser-sming.y"
    {
			    /*
			     * Sum up the number of successfully parsed
			     * revisions or return -1, if at least one
			     * module failed.
			     */
			    if (((yyvsp[(1) - (2)].rc) >= 0) && ((yyvsp[(2) - (2)].rc) >= 0)) {
				(yyval.rc) = (yyvsp[(1) - (2)].rc) + (yyvsp[(2) - (2)].rc);
			    } else {
				(yyval.rc) = -1;
			    }
			;}
    break;

  case 88:
#line 1459 "parser-sming.y"
    {
			       /*
				* If we got a (Revision *) return rc == 1,
				* otherwise parsing failed (rc == -1).
				*/
			       if ((yyvsp[(1) - (2)].revisionPtr)) {
				(yyval.rc) = 1;
			       } else {
				   (yyval.rc) = -1;
			       }
			   ;}
    break;

  case 89:
#line 1476 "parser-sming.y"
    {
			    (yyval.revisionPtr) = addRevision((yyvsp[(5) - (11)].date), (yyvsp[(7) - (11)].text), thisParserPtr);
			;}
    break;

  case 90:
#line 1482 "parser-sming.y"
    {
			    (yyval.rc) = 0;
			;}
    break;

  case 91:
#line 1486 "parser-sming.y"
    {
			    /*
			     * Return the number of successfully
			     * parsed identity statements.
			     */
			    (yyval.rc) = (yyvsp[(1) - (1)].rc);
			;}
    break;

  case 92:
#line 1496 "parser-sming.y"
    {
			    (yyval.rc) = (yyvsp[(1) - (1)].rc);
			;}
    break;

  case 93:
#line 1501 "parser-sming.y"
    {
			    /*
			     * Sum up the number of successfully parsed
			     * identities or return -1, if at least one
			     * failed.
			     */
			    if (((yyvsp[(1) - (2)].rc) >= 0) && ((yyvsp[(2) - (2)].rc) >= 0)) {
				(yyval.rc) = (yyvsp[(1) - (2)].rc) + (yyvsp[(2) - (2)].rc);
			    } else {
				(yyval.rc) = -1;
			    }
			;}
    break;

  case 94:
#line 1516 "parser-sming.y"
    {
			    /*
			     * If we got a (Type *) return rc == 1,
			     * otherwise parsing failed (rc == -1).
			     */
			    if ((yyvsp[(1) - (2)].identityPtr)) {
				(yyval.rc) = 1;
			    } else {
				(yyval.rc) = -1;
			    }
			;}
    break;

  case 95:
#line 1530 "parser-sming.y"
    {
				identityIdentifier = (yyvsp[(3) - (3)].text);
				identityPtr = addIdentity(identityIdentifier,
						thisParserPtr);
			    setIdentityDecl(identityPtr, SMI_DECL_IDENTITY);
			;}
    break;

  case 96:
#line 1538 "parser-sming.y"
    {
				if(identityPtr && (yyvsp[(8) - (8)].identityPtr)) {
					setIdentityParent(identityPtr,(yyvsp[(8) - (8)].identityPtr));
				}
			;}
    break;

  case 97:
#line 1544 "parser-sming.y"
    {
			    if (identityPtr && (yyvsp[(10) - (10)].status)) {
				setIdentityStatus(identityPtr, (yyvsp[(10) - (10)].status));
			    }
			;}
    break;

  case 98:
#line 1550 "parser-sming.y"
    {
				if (identityPtr && (yyvsp[(12) - (12)].text)) {
				setIdentityDescription(identityPtr, (yyvsp[(12) - (12)].text),
						    			thisParserPtr);
			    }
			;}
    break;

  case 99:
#line 1557 "parser-sming.y"
    {
				setIdentityReference(identityPtr, (yyvsp[(14) - (14)].text), 
									 thisParserPtr)
			;}
    break;

  case 100:
#line 1562 "parser-sming.y"
    {
			    (yyval.identityPtr) = 0;
			    identityPtr = NULL;
			;}
    break;

  case 101:
#line 1569 "parser-sming.y"
    {
			    (yyval.typePtr) = (yyvsp[(3) - (5)].typePtr);
			;}
    break;

  case 102:
#line 1575 "parser-sming.y"
    {
			    (yyval.attributePtr) = (yyvsp[(3) - (5)].attributePtr);
			;}
    break;

  case 103:
#line 1581 "parser-sming.y"
    {
			    (yyval.date) = (yyvsp[(3) - (5)].date);
			;}
    break;

  case 104:
#line 1587 "parser-sming.y"
    {
			    (yyval.text) = (yyvsp[(3) - (5)].text);
			;}
    break;

  case 105:
#line 1593 "parser-sming.y"
    {
			    (yyval.text) = (yyvsp[(3) - (5)].text);
			;}
    break;

  case 106:
#line 1599 "parser-sming.y"
    {
			    (yyval.text) = NULL;
			;}
    break;

  case 107:
#line 1603 "parser-sming.y"
    {
			    (yyval.text) = (yyvsp[(1) - (2)].text);
			;}
    break;

  case 108:
#line 1609 "parser-sming.y"
    {
			    (yyval.text) = (yyvsp[(3) - (5)].text);
			;}
    break;

  case 109:
#line 1615 "parser-sming.y"
    {
			    (yyval.text) = NULL;
			;}
    break;

  case 110:
#line 1619 "parser-sming.y"
    {
			    (yyval.text) = (yyvsp[(1) - (2)].text);
			;}
    break;

  case 111:
#line 1625 "parser-sming.y"
    {
			    (yyval.text) = (yyvsp[(3) - (5)].text);
			;}
    break;

  case 112:
#line 1631 "parser-sming.y"
    {
			    (yyval.status) = SMI_STATUS_CURRENT;
			;}
    break;

  case 113:
#line 1635 "parser-sming.y"
    {
			    (yyval.status) = (yyvsp[(1) - (2)].status);
			;}
    break;

  case 114:
#line 1641 "parser-sming.y"
    {
			    (yyval.status) = (yyvsp[(3) - (5)].status);
			;}
    break;

  case 115:
#line 1647 "parser-sming.y"
    {
			    (yyval.listPtr) = NULL;
			;}
    break;

  case 116:
#line 1651 "parser-sming.y"
    {
			    (yyval.listPtr) = (yyvsp[(1) - (2)].listPtr);
			;}
    break;

  case 117:
#line 1657 "parser-sming.y"
    {
			    (yyval.listPtr) = (yyvsp[(4) - (7)].listPtr);
			;}
    break;

  case 118:
#line 1663 "parser-sming.y"
    {
				(yyval.listPtr) = smiMalloc(sizeof(List));
			    (yyval.listPtr)->ptr = "#@#"; //used to indicate that unique
			    				 //statement is present and empty
			    				 //i.e. the class is scalar
			    (yyval.listPtr)->nextPtr = NULL;
			;}
    break;

  case 119:
#line 1671 "parser-sming.y"
    {
			    (yyval.listPtr) = smiMalloc(sizeof(List));
			    (yyval.listPtr)->ptr = (yyvsp[(1) - (3)].text);
			    (yyval.listPtr)->nextPtr = (yyvsp[(2) - (3)].listPtr);
			;}
    break;

  case 120:
#line 1682 "parser-sming.y"
    {
			    (yyval.classPtr) = NULL;
			;}
    break;

  case 121:
#line 1686 "parser-sming.y"
    {
			    (yyval.classPtr) = (yyvsp[(1) - (2)].classPtr);
			;}
    break;

  case 122:
#line 1692 "parser-sming.y"
    {
			    (yyval.classPtr) = findClass((yyvsp[(3) - (5)].text), thisParserPtr,thisModulePtr);
			    if(!(yyval.classPtr))smiPrintError(thisParserPtr,
					      ERR_UNKNOWN_CLASS,
					      (yyvsp[(3) - (5)].text));
			    
			;}
    break;

  case 123:
#line 1702 "parser-sming.y"
    {
			    (yyval.valuePtr) = NULL;
			;}
    break;

  case 124:
#line 1706 "parser-sming.y"
    {
			    (yyval.valuePtr) = (yyvsp[(1) - (2)].valuePtr);
			;}
    break;

  case 125:
#line 1712 "parser-sming.y"
    {
			    (yyval.valuePtr) = (yyvsp[(3) - (5)].valuePtr);
			;}
    break;

  case 126:
#line 1718 "parser-sming.y"
    {
			    (yyval.access) = SMI_ACCESS_UNKNOWN;
			;}
    break;

  case 127:
#line 1722 "parser-sming.y"
    {
			    (yyval.access) = (yyvsp[(1) - (2)].access);
			;}
    break;

  case 128:
#line 1728 "parser-sming.y"
    {
			    (yyval.access) = (yyvsp[(3) - (5)].access);
			;}
    break;

  case 129:
#line 1734 "parser-sming.y"
    {
				(yyval.access) = SMI_ACCESS_READ_ONLY;
			;}
    break;

  case 130:
#line 1738 "parser-sming.y"
    {
				(yyval.access) = SMI_ACCESS_READ_WRITE;
			;}
    break;

  case 131:
#line 1742 "parser-sming.y"
    {
				(yyval.access) = SMI_ACCESS_EVENT_ONLY;
			;}
    break;

  case 132:
#line 1748 "parser-sming.y"
    {
			    (yyval.identityPtr) = NULL;
			;}
    break;

  case 133:
#line 1752 "parser-sming.y"
    {
			    (yyval.identityPtr) = (yyvsp[(1) - (2)].identityPtr);
			;}
    break;

  case 134:
#line 1758 "parser-sming.y"
    {
			    (yyval.identityPtr) = findIdentity((yyvsp[(3) - (5)].text), thisParserPtr, thisModulePtr);
			    if(!(yyval.identityPtr))smiPrintError(thisParserPtr,
					      ERR_IDENTITY_PARENT_NOT_FOUND,
					      (yyvsp[(3) - (5)].text));
			;}
    break;

  case 135:
#line 1767 "parser-sming.y"
    {
			    (yyval.text) = NULL;
			;}
    break;

  case 136:
#line 1771 "parser-sming.y"
    {
			    (yyval.text) = (yyvsp[(1) - (2)].text);
			;}
    break;

  case 137:
#line 1777 "parser-sming.y"
    {
			    (yyval.text) = (yyvsp[(3) - (5)].text);
			;}
    break;

  case 138:
#line 1783 "parser-sming.y"
    {
			    (yyval.text) = NULL;
			;}
    break;

  case 139:
#line 1787 "parser-sming.y"
    {
			    (yyval.text) = (yyvsp[(1) - (2)].text);
			;}
    break;

  case 140:
#line 1793 "parser-sming.y"
    {
			    (yyval.text) = (yyvsp[(3) - (5)].text);
			;}
    break;

  case 141:
#line 1799 "parser-sming.y"
    {
			    (yyval.text) = NULL;
			;}
    break;

  case 142:
#line 1803 "parser-sming.y"
    {
			    (yyval.text) = (yyvsp[(1) - (2)].text);
			;}
    break;

  case 143:
#line 1809 "parser-sming.y"
    {
			    (yyval.text) = (yyvsp[(3) - (5)].text);
			;}
    break;

  case 144:
#line 1817 "parser-sming.y"
    {
			    (yyval.typePtr) = (yyvsp[(1) - (1)].typePtr);
			;}
    break;

  case 145:
#line 1821 "parser-sming.y"
    {
			    (yyval.typePtr) = (yyvsp[(1) - (1)].typePtr);
			;}
    break;

  case 146:
#line 1827 "parser-sming.y"
    {
			    (yyval.attributePtr) = (yyvsp[(1) - (1)].attributePtr);
			;}
    break;

  case 147:
#line 1831 "parser-sming.y"
    {
			    (yyval.attributePtr) = (yyvsp[(1) - (1)].attributePtr);
			;}
    break;

  case 148:
#line 1838 "parser-sming.y"
    {
			    List *p;
			    
			    if (!(yyvsp[(2) - (2)].listPtr)) {
				(yyval.typePtr) = smiHandle->typeOctetStringPtr;
			    } else {
				(yyval.typePtr) = duplicateType(smiHandle->typeOctetStringPtr, 0,
						   thisParserPtr);
				setTypeParent((yyval.typePtr), smiHandle->typeOctetStringPtr);
				setTypeList((yyval.typePtr), (yyvsp[(2) - (2)].listPtr));
				for (p = (yyvsp[(2) - (2)].listPtr); p; p = p->nextPtr)
				    ((Range *)p->ptr)->typePtr = (yyval.typePtr);
			    }
			;}
    break;

  case 149:
#line 1853 "parser-sming.y"
    {
			    (yyval.typePtr) = smiHandle->typeObjectIdentifierPtr;
			;}
    break;

  case 150:
#line 1857 "parser-sming.y"
    {
			    List *p;
			    
			    if (!(yyvsp[(2) - (2)].listPtr)) {
				(yyval.typePtr) = smiHandle->typeInteger32Ptr;
			    } else {
				(yyval.typePtr) = duplicateType(smiHandle->typeInteger32Ptr, 0,
						   thisParserPtr);
				setTypeParent((yyval.typePtr), smiHandle->typeInteger32Ptr);
				setTypeList((yyval.typePtr), (yyvsp[(2) - (2)].listPtr));
				for (p = (yyvsp[(2) - (2)].listPtr); p; p = p->nextPtr)
				    ((Range *)p->ptr)->typePtr = (yyval.typePtr);
			    }
			;}
    break;

  case 151:
#line 1872 "parser-sming.y"
    {
			    List *p;
			    
			    if (!(yyvsp[(2) - (2)].listPtr)) {
				(yyval.typePtr) = smiHandle->typeUnsigned32Ptr;
			    } else {
				(yyval.typePtr) = duplicateType(smiHandle->typeUnsigned32Ptr, 0,
						   thisParserPtr);
				setTypeParent((yyval.typePtr), smiHandle->typeUnsigned32Ptr);
				setTypeList((yyval.typePtr), (yyvsp[(2) - (2)].listPtr));
				for (p = (yyvsp[(2) - (2)].listPtr); p; p = p->nextPtr)
				    ((Range *)p->ptr)->typePtr = (yyval.typePtr);
			    }
			;}
    break;

  case 152:
#line 1887 "parser-sming.y"
    {
			    List *p;
			    
			    if (!(yyvsp[(2) - (2)].listPtr)) {
				(yyval.typePtr) = smiHandle->typeInteger64Ptr;
			    } else {
				(yyval.typePtr) = duplicateType(smiHandle->typeInteger64Ptr, 0,
						   thisParserPtr);
				setTypeParent((yyval.typePtr), smiHandle->typeInteger64Ptr);
				setTypeList((yyval.typePtr), (yyvsp[(2) - (2)].listPtr));
				for (p = (yyvsp[(2) - (2)].listPtr); p; p = p->nextPtr)
				    ((Range *)p->ptr)->typePtr = (yyval.typePtr);
			    }
			;}
    break;

  case 153:
#line 1902 "parser-sming.y"
    {
			    List *p;
			    
			    if (!(yyvsp[(2) - (2)].listPtr)) {
				(yyval.typePtr) = smiHandle->typeUnsigned64Ptr;
			    } else {
				(yyval.typePtr) = duplicateType(smiHandle->typeUnsigned64Ptr, 0,
						   thisParserPtr);
				setTypeParent((yyval.typePtr), smiHandle->typeUnsigned64Ptr);
				setTypeList((yyval.typePtr), (yyvsp[(2) - (2)].listPtr));
				for (p = (yyvsp[(2) - (2)].listPtr); p; p = p->nextPtr)
				    ((Range *)p->ptr)->typePtr = (yyval.typePtr);
			    }
			;}
    break;

  case 154:
#line 1917 "parser-sming.y"
    {
			    List *p;
			    
			    if (!(yyvsp[(2) - (2)].listPtr)) {
				(yyval.typePtr) = smiHandle->typeFloat32Ptr;
			    } else {
				(yyval.typePtr) = duplicateType(smiHandle->typeFloat32Ptr, 0,
						   thisParserPtr);
				setTypeParent((yyval.typePtr), smiHandle->typeFloat32Ptr);
				setTypeList((yyval.typePtr), (yyvsp[(2) - (2)].listPtr));
				for (p = (yyvsp[(2) - (2)].listPtr); p; p = p->nextPtr)
				    ((Range *)p->ptr)->typePtr = (yyval.typePtr);
			    }
			;}
    break;

  case 155:
#line 1932 "parser-sming.y"
    {
			    List *p;
			    
			    if (!(yyvsp[(2) - (2)].listPtr)) {
				(yyval.typePtr) = smiHandle->typeFloat64Ptr;
			    } else {
				(yyval.typePtr) = duplicateType(smiHandle->typeFloat64Ptr, 0,
						   thisParserPtr);
				setTypeParent((yyval.typePtr), smiHandle->typeFloat64Ptr);
				setTypeList((yyval.typePtr), (yyvsp[(2) - (2)].listPtr));
				for (p = (yyvsp[(2) - (2)].listPtr); p; p = p->nextPtr)
				    ((Range *)p->ptr)->typePtr = (yyval.typePtr);
			    }
			;}
    break;

  case 156:
#line 1947 "parser-sming.y"
    {
			    List *p;
			    
			    if (!(yyvsp[(2) - (2)].listPtr)) {
				(yyval.typePtr) = smiHandle->typeFloat128Ptr;
			    } else {
				(yyval.typePtr) = duplicateType(smiHandle->typeFloat128Ptr, 0,
						   thisParserPtr);
				setTypeParent((yyval.typePtr), smiHandle->typeFloat128Ptr);
				setTypeList((yyval.typePtr), (yyvsp[(2) - (2)].listPtr));
				for (p = (yyvsp[(2) - (2)].listPtr); p; p = p->nextPtr)
				    ((Range *)p->ptr)->typePtr = (yyval.typePtr);
			    }
			;}
    break;

  case 157:
#line 1962 "parser-sming.y"
    {
			    if (!(yyvsp[(2) - (2)].listPtr)) {
				(yyval.typePtr) = smiHandle->typePointerPtr;
			    } else {
				(yyval.typePtr) = duplicateType(smiHandle->typePointerPtr, 0,
						   thisParserPtr);
				setTypeParent((yyval.typePtr), smiHandle->typePointerPtr);
				setTypeList((yyval.typePtr), (yyvsp[(2) - (2)].listPtr));				
				}
			;}
    break;

  case 158:
#line 1973 "parser-sming.y"
    {
			    List *p;
			    
			    if (!(yyvsp[(2) - (2)].listPtr)) {
				(yyval.typePtr) = smiHandle->typeEnumPtr;
			    } else {
				(yyval.typePtr) = duplicateType(smiHandle->typeEnumPtr, 0,
						   thisParserPtr);
				setTypeParent((yyval.typePtr), smiHandle->typeEnumPtr);
				setTypeList((yyval.typePtr), (yyvsp[(2) - (2)].listPtr));
				for (p = (yyvsp[(2) - (2)].listPtr); p; p = p->nextPtr)
				    ((NamedNumber *)p->ptr)->typePtr = (yyval.typePtr);
			    }
			;}
    break;

  case 159:
#line 1988 "parser-sming.y"
    {
				bitsFlag = 1; //Since Enum elements can be 
							  //negative we must make sure
							  //that bits is not negative,
							  //so we raise bitsFlag and
							  //give error if there is
							  //negative value 
			;}
    break;

  case 160:
#line 1997 "parser-sming.y"
    {
			    List *p;
			    
			    if (!(yyvsp[(3) - (3)].listPtr)) {
				(yyval.typePtr) = smiHandle->typeBitsPtr;
			    } else {
				(yyval.typePtr) = duplicateType(smiHandle->typeBitsPtr, 0,
						   thisParserPtr);
				setTypeParent((yyval.typePtr), smiHandle->typeBitsPtr);
				setTypeList((yyval.typePtr), (yyvsp[(3) - (3)].listPtr));
				for (p = (yyvsp[(3) - (3)].listPtr); p; p = p->nextPtr)
				    ((NamedNumber *)p->ptr)->typePtr = (yyval.typePtr);
			    }
			    
			    bitsFlag = 0;//reset flag
			;}
    break;

  case 161:
#line 2017 "parser-sming.y"
    {
			    List *p;
			    (yyval.attributePtr) = duplicateTypeToAttribute(smiHandle->typeOctetStringPtr,
												classPtr, thisParserPtr);
				setAttributeParentType((yyval.attributePtr), smiHandle->typeOctetStringPtr);
			    if ((yyvsp[(2) - (2)].listPtr)) {
					setAttributeList((yyval.attributePtr), (yyvsp[(2) - (2)].listPtr));
					for (p = (yyvsp[(2) - (2)].listPtr); p; p = p->nextPtr)
				    	((Range *)p->ptr)->typePtr = (Type*)(yyval.attributePtr);
			    }
			;}
    break;

  case 162:
#line 2029 "parser-sming.y"
    {
			    (yyval.attributePtr) = duplicateTypeToAttribute(
			    	smiHandle->typeObjectIdentifierPtr, classPtr, thisParserPtr);
			   	setAttributeParentType((yyval.attributePtr), smiHandle->typeObjectIdentifierPtr);
			;}
    break;

  case 163:
#line 2035 "parser-sming.y"
    {
			    List *p;
			    
				(yyval.attributePtr) = duplicateTypeToAttribute(smiHandle->typeInteger32Ptr,
													classPtr, thisParserPtr);
				setAttributeParentType((yyval.attributePtr), smiHandle->typeInteger32Ptr);
			    if ((yyvsp[(2) - (2)].listPtr)) {
					setAttributeList((yyval.attributePtr), (yyvsp[(2) - (2)].listPtr));
					for (p = (yyvsp[(2) - (2)].listPtr); p; p = p->nextPtr)
				    	((Range *)p->ptr)->typePtr = (Type*)(yyval.attributePtr);
			    }
			;}
    break;

  case 164:
#line 2048 "parser-sming.y"
    {
			    List *p;
			    
				(yyval.attributePtr) = duplicateTypeToAttribute(smiHandle->typeUnsigned32Ptr,
													classPtr, thisParserPtr);
				setAttributeParentType((yyval.attributePtr), smiHandle->typeUnsigned32Ptr);
			    if ((yyvsp[(2) - (2)].listPtr)) {
					setAttributeList((yyval.attributePtr), (yyvsp[(2) - (2)].listPtr));
					for (p = (yyvsp[(2) - (2)].listPtr); p; p = p->nextPtr)
				    	((Range *)p->ptr)->typePtr = (Type*)(yyval.attributePtr);
			    }
			;}
    break;

  case 165:
#line 2061 "parser-sming.y"
    {
			   List *p;
			    
				(yyval.attributePtr) = duplicateTypeToAttribute(smiHandle->typeInteger64Ptr,
													classPtr, thisParserPtr);
				setAttributeParentType((yyval.attributePtr), smiHandle->typeInteger64Ptr);
			    if ((yyvsp[(2) - (2)].listPtr)) {
					setAttributeList((yyval.attributePtr), (yyvsp[(2) - (2)].listPtr));
					for (p = (yyvsp[(2) - (2)].listPtr); p; p = p->nextPtr)
				    	((Range *)p->ptr)->typePtr = (Type*)(yyval.attributePtr);
			    }
			;}
    break;

  case 166:
#line 2074 "parser-sming.y"
    {
			   List *p;
			    
				(yyval.attributePtr) = duplicateTypeToAttribute(smiHandle->typeUnsigned64Ptr,
													classPtr, thisParserPtr);
				setAttributeParentType((yyval.attributePtr), smiHandle->typeUnsigned64Ptr);
			    if ((yyvsp[(2) - (2)].listPtr)) {
					setAttributeList((yyval.attributePtr), (yyvsp[(2) - (2)].listPtr));
					for (p = (yyvsp[(2) - (2)].listPtr); p; p = p->nextPtr)
				    	((Range *)p->ptr)->typePtr = (Type*)(yyval.attributePtr);
			    }
			;}
    break;

  case 167:
#line 2087 "parser-sming.y"
    {
			    List *p;
			    
				(yyval.attributePtr) = duplicateTypeToAttribute(smiHandle->typeFloat32Ptr,
													classPtr, thisParserPtr);
				setAttributeParentType((yyval.attributePtr), smiHandle->typeFloat32Ptr);
			    if ((yyvsp[(2) - (2)].listPtr)) {
					setAttributeList((yyval.attributePtr), (yyvsp[(2) - (2)].listPtr));
					for (p = (yyvsp[(2) - (2)].listPtr); p; p = p->nextPtr)
				    	((Range *)p->ptr)->typePtr = (Type*)(yyval.attributePtr);
			    }
			;}
    break;

  case 168:
#line 2100 "parser-sming.y"
    {
			   List *p;
			    
				(yyval.attributePtr) = duplicateTypeToAttribute(smiHandle->typeFloat64Ptr,
													classPtr, thisParserPtr);
				setAttributeParentType((yyval.attributePtr), smiHandle->typeFloat64Ptr);
			    if ((yyvsp[(2) - (2)].listPtr)) {
					setAttributeList((yyval.attributePtr), (yyvsp[(2) - (2)].listPtr));
					for (p = (yyvsp[(2) - (2)].listPtr); p; p = p->nextPtr)
				    	((Range *)p->ptr)->typePtr = (Type*)(yyval.attributePtr);
			    }
			;}
    break;

  case 169:
#line 2113 "parser-sming.y"
    {
			    List *p;
			    
			    (yyval.attributePtr) = duplicateTypeToAttribute(smiHandle->typeFloat128Ptr,
							  classPtr, thisParserPtr);
			    setAttributeParentType((yyval.attributePtr), smiHandle->typeFloat128Ptr);
			    if ((yyvsp[(2) - (2)].listPtr)) {
				setAttributeList((yyval.attributePtr), (yyvsp[(2) - (2)].listPtr));
				for (p = (yyvsp[(2) - (2)].listPtr); p; p = p->nextPtr)
				    ((Range *)p->ptr)->typePtr = (Type*)(yyval.attributePtr);
			    }
			;}
    break;

  case 170:
#line 2126 "parser-sming.y"
    {
			    (yyval.attributePtr) = duplicateTypeToAttribute(smiHandle->typePointerPtr,
							  classPtr, thisParserPtr);
			    setAttributeParentType((yyval.attributePtr), smiHandle->typePointerPtr);
			    if ((yyvsp[(2) - (2)].listPtr)) {
				setAttributeList((yyval.attributePtr), (yyvsp[(2) - (2)].listPtr));
			    }
			;}
    break;

  case 171:
#line 2135 "parser-sming.y"
    {
			    List *p;
			    
			    (yyval.attributePtr) = duplicateTypeToAttribute(smiHandle->typeEnumPtr,
													classPtr, thisParserPtr);
				setAttributeParentType((yyval.attributePtr), smiHandle->typeEnumPtr);
			    if ((yyvsp[(2) - (2)].listPtr)) {
					setAttributeList((yyval.attributePtr), (yyvsp[(2) - (2)].listPtr));
					for (p = (yyvsp[(2) - (2)].listPtr); p; p = p->nextPtr)
				    	((NamedNumber *)p->ptr)->typePtr = (Type*)(yyval.attributePtr);
			    }
			;}
    break;

  case 172:
#line 2148 "parser-sming.y"
    {
				bitsFlag = 1; //Since Enum elements can be 
							  //negative we must make sure
							  //that bits is not negative,
							  //so we raise bitsFlag and
							  //give error if there is
							  //negative value 
			;}
    break;

  case 173:
#line 2157 "parser-sming.y"
    {
			    List *p;
			    
			    (yyval.attributePtr) = duplicateTypeToAttribute(smiHandle->typeBitsPtr,
							  classPtr, thisParserPtr);
			    setAttributeParentType((yyval.attributePtr), smiHandle->typeBitsPtr);
			    if ((yyvsp[(1) - (3)].rc)) {
				setAttributeList((yyval.attributePtr), (yyvsp[(1) - (3)].rc));
				for (p = (yyvsp[(1) - (3)].rc); p; p = p->nextPtr)
				    ((NamedNumber *)(p->ptr))->typePtr = (Type*)(yyval.attributePtr);
			    }
			    
			    bitsFlag = 0; /* reset flag */
			;}
    break;

  case 174:
#line 2174 "parser-sming.y"
    {
			    typePtr = findType((yyvsp[(1) - (2)].text), thisParserPtr,
					       thisModulePtr);
			    if (typePtr && (yyvsp[(2) - (2)].listPtr)) {
				typePtr = duplicateType(typePtr, 0,
							thisParserPtr);
				setTypeList(typePtr, (yyvsp[(2) - (2)].listPtr));
			    }

			    (yyval.typePtr) = typePtr;
			;}
    break;

  case 175:
#line 2188 "parser-sming.y"
    {
			    Class *tmp;
			    typePtr = findType((yyvsp[(1) - (2)].text), thisParserPtr,
					       thisModulePtr);
			    if (typePtr && (yyvsp[(2) - (2)].listPtr)) {
				attributePtr = duplicateTypeToAttribute(typePtr,
											classPtr, thisParserPtr);
				setAttributeList(attributePtr, (yyvsp[(2) - (2)].listPtr));
				setAttributeParentType(attributePtr, typePtr);
			    } else if( typePtr ){
			    	attributePtr = duplicateTypeToAttribute(typePtr,
											classPtr, thisParserPtr);
					setAttributeParentType(attributePtr, typePtr);
				} else if((yyvsp[(2) - (2)].listPtr)){
					smiPrintError(thisParserPtr, ERR_UNKNOWN_TYPE, (yyvsp[(1) - (2)].text));
					attributePtr = NULL;
				} else if (tmp = findClass((yyvsp[(1) - (2)].text),
										 thisParserPtr,thisModulePtr)){
					attributePtr = addAttribute((yyvsp[(1) - (2)].text), classPtr, thisParserPtr);
					setAttributeParentClass(attributePtr, tmp);
				} else {
					attributePtr = NULL;
					smiPrintError(thisParserPtr, 
											ERR_UNKNOWN_TYPE_OR_CLASS, (yyvsp[(1) - (2)].text));
				}
				

			    (yyval.attributePtr) = attributePtr;
			;}
    break;

  case 176:
#line 2220 "parser-sming.y"
    {
			    (yyval.listPtr) = NULL;
			;}
    break;

  case 177:
#line 2224 "parser-sming.y"
    {
			    (yyval.listPtr) = (yyvsp[(2) - (2)].listPtr);
			;}
    break;

  case 178:
#line 2230 "parser-sming.y"
    {
			    (yyval.listPtr) = (yyvsp[(1) - (1)].listPtr);
			;}
    break;

  case 179:
#line 2234 "parser-sming.y"
    {
			    (yyval.listPtr) = (yyvsp[(1) - (1)].listPtr);
			;}
    break;

  case 180:
#line 2240 "parser-sming.y"
    {
			    (yyval.listPtr) = NULL;
			;}
    break;

  case 181:
#line 2244 "parser-sming.y"
    {
			    (yyval.listPtr) = (yyvsp[(2) - (2)].listPtr);
			;}
    break;

  case 182:
#line 2251 "parser-sming.y"
    {
			    (yyval.listPtr) = smiMalloc(sizeof(List));
			    (yyval.listPtr)->ptr = (yyvsp[(3) - (6)].rangePtr);
			    (yyval.listPtr)->nextPtr = (yyvsp[(4) - (6)].listPtr);
			;}
    break;

  case 183:
#line 2259 "parser-sming.y"
    {
			    (yyval.listPtr) = NULL;
			;}
    break;

  case 184:
#line 2263 "parser-sming.y"
    {
			    (yyval.listPtr) = (yyvsp[(1) - (1)].listPtr);
			;}
    break;

  case 185:
#line 2269 "parser-sming.y"
    {
			    (yyval.listPtr) = smiMalloc(sizeof(List));
			    (yyval.listPtr)->ptr = (yyvsp[(1) - (1)].rangePtr);
			    (yyval.listPtr)->nextPtr = NULL;
			;}
    break;

  case 186:
#line 2275 "parser-sming.y"
    {
			    List *p, *pp;
			    
			    p = smiMalloc(sizeof(List));
			    p->ptr = (yyvsp[(2) - (2)].rangePtr);
			    p->nextPtr = NULL;
			    for (pp = (yyvsp[(1) - (2)].listPtr); pp->nextPtr; pp = pp->nextPtr);
			    pp->nextPtr = p;
			    (yyval.listPtr) = (yyvsp[(1) - (2)].listPtr);
			;}
    break;

  case 187:
#line 2288 "parser-sming.y"
    {
			    (yyval.rangePtr) = (yyvsp[(4) - (4)].rangePtr);
			;}
    break;

  case 188:
#line 2294 "parser-sming.y"
    {
			    (yyval.rangePtr) = smiMalloc(sizeof(Range));
			    (yyval.rangePtr)->export.minValue = *(yyvsp[(1) - (2)].valuePtr);
			    if ((yyvsp[(2) - (2)].valuePtr)) {
				(yyval.rangePtr)->export.maxValue = *(yyvsp[(2) - (2)].valuePtr);
				smiFree((yyvsp[(2) - (2)].valuePtr));
			    } else {
				(yyval.rangePtr)->export.maxValue = *(yyvsp[(1) - (2)].valuePtr);
			    }
			    smiFree((yyvsp[(1) - (2)].valuePtr));
			;}
    break;

  case 189:
#line 2308 "parser-sming.y"
    {
			    (yyval.valuePtr) = NULL;
			;}
    break;

  case 190:
#line 2312 "parser-sming.y"
    {
			    (yyval.valuePtr) = (yyvsp[(1) - (1)].valuePtr);
			;}
    break;

  case 191:
#line 2318 "parser-sming.y"
    {
			    (yyval.valuePtr) = (yyvsp[(4) - (4)].valuePtr);
			;}
    break;

  case 192:
#line 2324 "parser-sming.y"
    {
			    (yyval.listPtr) = NULL;
			;}
    break;

  case 193:
#line 2328 "parser-sming.y"
    {
			    (yyval.listPtr) = (yyvsp[(2) - (2)].listPtr);
			;}
    break;

  case 194:
#line 2335 "parser-sming.y"
    {
			    (yyval.listPtr) = smiMalloc(sizeof(List));
			    (yyval.listPtr)->ptr = (yyvsp[(3) - (6)].rangePtr);
			    (yyval.listPtr)->nextPtr = (yyvsp[(4) - (6)].listPtr);
			;}
    break;

  case 195:
#line 2343 "parser-sming.y"
    {
			    (yyval.listPtr) = NULL;
			;}
    break;

  case 196:
#line 2347 "parser-sming.y"
    {
			    (yyval.listPtr) = (yyvsp[(1) - (1)].listPtr);
			;}
    break;

  case 197:
#line 2353 "parser-sming.y"
    {
			    (yyval.listPtr) = smiMalloc(sizeof(List));
			    (yyval.listPtr)->ptr = (yyvsp[(1) - (1)].rangePtr);
			    (yyval.listPtr)->nextPtr = NULL;
			;}
    break;

  case 198:
#line 2359 "parser-sming.y"
    {
			    List *p, *pp;
			    
			    p = smiMalloc(sizeof(List));
			    p->ptr = (yyvsp[(2) - (2)].rangePtr);
			    p->nextPtr = NULL;
			    for (pp = (yyvsp[(1) - (2)].listPtr); pp->nextPtr; pp = pp->nextPtr);
			    pp->nextPtr = p;
			    (yyval.listPtr) = (yyvsp[(1) - (2)].listPtr);
			;}
    break;

  case 199:
#line 2372 "parser-sming.y"
    {
			    (yyval.rangePtr) = (yyvsp[(4) - (4)].rangePtr);
			;}
    break;

  case 200:
#line 2378 "parser-sming.y"
    {
			    (yyval.rangePtr) = smiMalloc(sizeof(Range));
			    (yyval.rangePtr)->export.minValue.basetype = SMI_BASETYPE_FLOAT64;
			    (yyval.rangePtr)->export.minValue.value.float64 = strtod((yyvsp[(1) - (2)].text), NULL);
			    if ((yyvsp[(2) - (2)].text)) {
				(yyval.rangePtr)->export.maxValue.basetype =
				                          SMI_BASETYPE_FLOAT64;
				(yyval.rangePtr)->export.maxValue.value.float64 =
				                              strtod((yyvsp[(2) - (2)].text), NULL);
			    } else {
				(yyval.rangePtr)->export.maxValue = (yyval.rangePtr)->export.minValue;
			    }
			;}
    break;

  case 201:
#line 2394 "parser-sming.y"
    {
			    (yyval.text) = NULL;
			;}
    break;

  case 202:
#line 2398 "parser-sming.y"
    {
			    (yyval.text) = (yyvsp[(1) - (1)].text);
			;}
    break;

  case 203:
#line 2404 "parser-sming.y"
    {
			    (yyval.text) = (yyvsp[(4) - (4)].text);
			;}
    break;

  case 204:
#line 2411 "parser-sming.y"
    {
				(yyval.text)="-inf";
			;}
    break;

  case 205:
#line 2415 "parser-sming.y"
    {
				(yyval.text)="+inf";
			;}
    break;

  case 206:
#line 2419 "parser-sming.y"
    {
				(yyval.text)="nan";
			;}
    break;

  case 207:
#line 2423 "parser-sming.y"
    {
				(yyval.text)="nan";
			;}
    break;

  case 208:
#line 2429 "parser-sming.y"
    {
			    (yyval.listPtr) = NULL;
			;}
    break;

  case 209:
#line 2433 "parser-sming.y"
    {
			    NamedNumber *nn =(NamedNumber*)smiMalloc(sizeof(NamedNumber));
			    (yyval.listPtr) = smiMalloc(sizeof(List));
			    (yyval.listPtr)->ptr = nn;
			    nn->export.name = (yyvsp[(2) - (2)].text);
			;}
    break;

  case 210:
#line 2442 "parser-sming.y"
    {
			    (yyval.text) = (yyvsp[(3) - (5)].text);
			;}
    break;

  case 211:
#line 2448 "parser-sming.y"
    {
			    (yyval.listPtr) = (yyvsp[(3) - (5)].listPtr);
			;}
    break;

  case 212:
#line 2455 "parser-sming.y"
    {
			    (yyval.listPtr) = smiMalloc(sizeof(List));
			    (yyval.listPtr)->ptr = (yyvsp[(1) - (3)].namedNumberPtr);
			    (yyval.listPtr)->nextPtr = (yyvsp[(2) - (3)].listPtr);
			;}
    break;

  case 213:
#line 2463 "parser-sming.y"
    {
			    (yyval.listPtr) = NULL;
			;}
    break;

  case 214:
#line 2467 "parser-sming.y"
    {
			    (yyval.listPtr) = (yyvsp[(1) - (1)].listPtr);
			;}
    break;

  case 215:
#line 2473 "parser-sming.y"
    {
			    (yyval.listPtr) = smiMalloc(sizeof(List));
			    (yyval.listPtr)->ptr = (yyvsp[(1) - (1)].namedNumberPtr);
			    (yyval.listPtr)->nextPtr = NULL;
			;}
    break;

  case 216:
#line 2480 "parser-sming.y"
    {
			    List *p, *pp;
			    
			    p = smiMalloc(sizeof(List));
			    p->ptr = (yyvsp[(2) - (2)].namedNumberPtr);
			    p->nextPtr = NULL;
			    for (pp = (yyvsp[(1) - (2)].listPtr); pp->nextPtr; pp = pp->nextPtr);
			    pp->nextPtr = p;
			    (yyval.listPtr) = (yyvsp[(1) - (2)].listPtr);
			;}
    break;

  case 217:
#line 2494 "parser-sming.y"
    {
			    (yyval.namedNumberPtr) = (yyvsp[(4) - (4)].namedNumberPtr);
			;}
    break;

  case 218:
#line 2500 "parser-sming.y"
    {
			    (yyval.namedNumberPtr) = smiMalloc(sizeof(NamedNumber));
			    (yyval.namedNumberPtr)->export.name = (yyvsp[(1) - (7)].text);
			    (yyval.namedNumberPtr)->export.value = *(yyvsp[(5) - (7)].valuePtr);
			;}
    break;

  case 219:
#line 2508 "parser-sming.y"
    {
			    (yyval.listPtr) = smiMalloc(sizeof(List));
			    (yyval.listPtr)->ptr = (yyvsp[(1) - (3)].text);
			    (yyval.listPtr)->nextPtr = (yyvsp[(2) - (3)].listPtr);
			;}
    break;

  case 220:
#line 2516 "parser-sming.y"
    {
			    (yyval.listPtr) = NULL;
			;}
    break;

  case 221:
#line 2520 "parser-sming.y"
    {
			    (yyval.listPtr) = (yyvsp[(1) - (1)].listPtr);
			;}
    break;

  case 222:
#line 2526 "parser-sming.y"
    {
			    (yyval.listPtr) = smiMalloc(sizeof(List));
			    (yyval.listPtr)->ptr = (yyvsp[(1) - (1)].text);
			    (yyval.listPtr)->nextPtr = NULL;
			;}
    break;

  case 223:
#line 2532 "parser-sming.y"
    {
			    List *p, *pp;
			    
			    p = smiMalloc(sizeof(List));
			    p->ptr = (yyvsp[(2) - (2)].text);
			    p->nextPtr = NULL;
			    for (pp = (yyvsp[(1) - (2)].listPtr); pp->nextPtr; pp = pp->nextPtr);
			    pp->nextPtr = p;
			    (yyval.listPtr) = (yyvsp[(1) - (2)].listPtr);
			;}
    break;

  case 224:
#line 2545 "parser-sming.y"
    {
			    (yyval.text) = (yyvsp[(4) - (4)].text);
			;}
    break;

  case 225:
#line 2551 "parser-sming.y"
    {
			    (yyval.listPtr) = (yyvsp[(3) - (5)].listPtr);
			;}
    break;

  case 226:
#line 2557 "parser-sming.y"
    {
			    (yyval.listPtr) = NULL;
			;}
    break;

  case 227:
#line 2561 "parser-sming.y"
    {
			    (yyval.listPtr) = smiMalloc(sizeof(List));
			    (yyval.listPtr)->ptr = (yyvsp[(1) - (3)].text);
			    (yyval.listPtr)->nextPtr = (yyvsp[(2) - (3)].listPtr);
			;}
    break;

  case 228:
#line 2570 "parser-sming.y"
    {
			    (yyval.listPtr) = NULL;
			;}
    break;

  case 229:
#line 2574 "parser-sming.y"
    {
			    (yyval.listPtr) = (yyvsp[(1) - (1)].listPtr);
			;}
    break;

  case 230:
#line 2580 "parser-sming.y"
    {
			    (yyval.listPtr) = smiMalloc(sizeof(List));
			    (yyval.listPtr)->ptr = (yyvsp[(1) - (1)].text);
			    (yyval.listPtr)->nextPtr = NULL;
			;}
    break;

  case 231:
#line 2586 "parser-sming.y"
    {
			    List *p, *pp;
			    
			    p = smiMalloc(sizeof(List));
			    p->ptr = (yyvsp[(2) - (2)].text);
			    p->nextPtr = NULL;
			    for (pp = (yyvsp[(1) - (2)].listPtr); pp->nextPtr; pp = pp->nextPtr);
			    pp->nextPtr = p;
			    (yyval.listPtr) = (yyvsp[(1) - (2)].listPtr);
			;}
    break;

  case 232:
#line 2599 "parser-sming.y"
    {
			    (yyval.text) = (yyvsp[(4) - (4)].text);
			;}
    break;

  case 233:
#line 2605 "parser-sming.y"
    {
			    (yyval.text) = (yyvsp[(1) - (1)].text);
			;}
    break;

  case 234:
#line 2609 "parser-sming.y"
    {
			    (yyval.text) = (yyvsp[(1) - (1)].text);
			;}
    break;

  case 235:
#line 2615 "parser-sming.y"
    {
			    char *s;

			    s = smiMalloc(strlen((yyvsp[(1) - (3)].text)) +
					    strlen((yyvsp[(3) - (3)].text)) + 3);
			    sprintf(s, "%s::%s", (yyvsp[(1) - (3)].text), (yyvsp[(3) - (3)].text));
			    (yyval.text) = s;
			    free((yyvsp[(1) - (3)].text));
			    free((yyvsp[(3) - (3)].text));
			;}
    break;

  case 236:
#line 2626 "parser-sming.y"
    {
			    (yyval.text) = (yyvsp[(1) - (1)].text);
			;}
    break;

  case 237:
#line 2632 "parser-sming.y"
    {
			    char *s;

			    s = smiMalloc(strlen((yyvsp[(1) - (3)].text)) +
					  strlen((yyvsp[(3) - (3)].text)) + 3);
			    sprintf(s, "%s::%s", (yyvsp[(1) - (3)].text), (yyvsp[(3) - (3)].text));
			    (yyval.text) = s;
			;}
    break;

  case 238:
#line 2641 "parser-sming.y"
    {
			    (yyval.text) = (yyvsp[(1) - (1)].text);
			;}
    break;

  case 239:
#line 2647 "parser-sming.y"
    {
			    int len;
			    if ((yyvsp[(2) - (2)].text)) {
				(yyval.text) = smiMalloc(strlen((yyvsp[(1) - (2)].text)) + strlen((yyvsp[(2) - (2)].text)) + 1);
				strcpy((yyval.text), (yyvsp[(1) - (2)].text));
				strcat((yyval.text), (yyvsp[(2) - (2)].text));
				free((yyvsp[(1) - (2)].text));
				free((yyvsp[(2) - (2)].text));
			    } else {
				(yyval.text) = smiStrdup((yyvsp[(1) - (2)].text));
			    }
			    len = strlen((yyval.text));
			    while (len > 0 && (yyval.text)[len-1] == '\n') {
				(yyval.text)[--len] = 0;
			    }
			;}
    break;

  case 240:
#line 2666 "parser-sming.y"
    {
			    (yyval.text) = NULL;
			;}
    break;

  case 241:
#line 2670 "parser-sming.y"
    {
			    (yyval.text) = (yyvsp[(1) - (1)].text);
			;}
    break;

  case 242:
#line 2676 "parser-sming.y"
    {
			    (yyval.text) = (yyvsp[(1) - (1)].text);
			;}
    break;

  case 243:
#line 2680 "parser-sming.y"
    {
			    (yyval.text) = smiMalloc(strlen((yyvsp[(1) - (2)].text)) + strlen((yyvsp[(2) - (2)].text)) + 1);
			    strcpy((yyval.text), (yyvsp[(1) - (2)].text));
			    strcat((yyval.text), (yyvsp[(2) - (2)].text));
			    free((yyvsp[(1) - (2)].text));
			    free((yyvsp[(2) - (2)].text));
			;}
    break;

  case 244:
#line 2690 "parser-sming.y"
    {
			    (yyval.text) = smiStrdup((yyvsp[(2) - (2)].text));
			;}
    break;

  case 245:
#line 2696 "parser-sming.y"
    {
			    (yyval.date) = checkDate(thisParserPtr, (yyvsp[(1) - (1)].text));
			;}
    break;

  case 246:
#line 2702 "parser-sming.y"
    {
			    (yyval.text) = smiStrdup((yyvsp[(1) - (1)].text));
			;}
    break;

  case 247:
#line 2708 "parser-sming.y"
    {
			    (yyval.text) = smiStrdup((yyvsp[(1) - (1)].text));
			;}
    break;

  case 248:
#line 2721 "parser-sming.y"
    {
			    if (defaultBasetype == SMI_BASETYPE_BITS) {
				(yyval.valuePtr) = smiMalloc(sizeof(SmiValue));
				(yyval.valuePtr)->basetype = SMI_BASETYPE_BITS;
				(yyval.valuePtr)->value.ptr = (void*)((yyvsp[(1) - (1)].listPtr));
				//set the bits value in the value.integer32
				if(typePtr){
					createBitsValue((yyval.valuePtr),typePtr);
				}
				else if(attributePtr){
				createBitsValue((yyval.valuePtr),
					(Type*)smiGetAttributeParentType(&(attributePtr->export)));
				}
			    } else {
				smiPrintError(thisParserPtr,
					      ERR_UNEXPECTED_VALUETYPE);
				(yyval.valuePtr) = NULL;
			    }
			;}
    break;

  case 249:
#line 2741 "parser-sming.y"
    {
			    /* Note: might also be an OID or signed */
			    switch (defaultBasetype) {
			    case SMI_BASETYPE_UNSIGNED32:
				(yyval.valuePtr) = smiMalloc(sizeof(SmiValue));
				(yyval.valuePtr)->basetype = SMI_BASETYPE_UNSIGNED32;
				(yyval.valuePtr)->value.unsigned32 = strtoul((yyvsp[(1) - (1)].text), NULL, 10);
				break;
			    case SMI_BASETYPE_UNSIGNED64:
				(yyval.valuePtr) = smiMalloc(sizeof(SmiValue));
				(yyval.valuePtr)->basetype = SMI_BASETYPE_UNSIGNED64;
				(yyval.valuePtr)->value.unsigned64 = strtoull((yyvsp[(1) - (1)].text), NULL, 10);
				break;
			    case SMI_BASETYPE_INTEGER32:
				(yyval.valuePtr) = smiMalloc(sizeof(SmiValue));
				(yyval.valuePtr)->basetype = SMI_BASETYPE_INTEGER32;
				(yyval.valuePtr)->value.integer32 = strtol((yyvsp[(1) - (1)].text), NULL, 10);
				break;
			    case SMI_BASETYPE_INTEGER64:
				(yyval.valuePtr) = smiMalloc(sizeof(SmiValue));
				(yyval.valuePtr)->basetype = SMI_BASETYPE_INTEGER64;
				(yyval.valuePtr)->value.integer64 = strtoll((yyvsp[(1) - (1)].text), NULL, 10);
				break;
			    case SMI_BASETYPE_OBJECTIDENTIFIER:
				(yyval.valuePtr) = smiMalloc(sizeof(SmiValue));
				(yyval.valuePtr)->basetype = SMI_BASETYPE_OBJECTIDENTIFIER;
				(yyval.valuePtr)->len = strlen((yyvsp[(1) - (1)].text));
				(yyval.valuePtr)->value.ptr = smiStrdup((yyvsp[(1) - (1)].text));
				/* TODO */
				break;
			    default:
				smiPrintError(thisParserPtr,
					      ERR_UNEXPECTED_VALUETYPE);
				(yyval.valuePtr) = NULL;
				break;
			    }
			;}
    break;

  case 250:
#line 2779 "parser-sming.y"
    {
			    switch (defaultBasetype) {
			    case SMI_BASETYPE_INTEGER32:
				(yyval.valuePtr) = smiMalloc(sizeof(SmiValue));
				(yyval.valuePtr)->basetype = SMI_BASETYPE_INTEGER32;
				(yyval.valuePtr)->value.integer32 = - strtol((yyvsp[(2) - (2)].text), NULL, 10);
				break;
			    case SMI_BASETYPE_INTEGER64:
				(yyval.valuePtr) = smiMalloc(sizeof(SmiValue));
				(yyval.valuePtr)->basetype = SMI_BASETYPE_INTEGER64;
				(yyval.valuePtr)->value.integer64 = - strtoll((yyvsp[(2) - (2)].text), NULL, 10);
				break;
			    default:
				smiPrintError(thisParserPtr,
					      ERR_UNEXPECTED_VALUETYPE);
				(yyval.valuePtr) = NULL;
				break;
			    }
			;}
    break;

  case 251:
#line 2799 "parser-sming.y"
    {
			    /* Note: might also be an octet string */
			    switch (defaultBasetype) {
			    case SMI_BASETYPE_UNSIGNED32:
				(yyval.valuePtr) = smiMalloc(sizeof(SmiValue));
				(yyval.valuePtr)->basetype = SMI_BASETYPE_UNSIGNED32;
				(yyval.valuePtr)->value.unsigned32 = strtoul((yyvsp[(1) - (1)].text), NULL, 16);
				break;
			    case SMI_BASETYPE_UNSIGNED64:
				(yyval.valuePtr) = smiMalloc(sizeof(SmiValue));
				(yyval.valuePtr)->basetype = SMI_BASETYPE_UNSIGNED64;
				(yyval.valuePtr)->value.unsigned64 = strtoull((yyvsp[(1) - (1)].text), NULL, 16);
				break;
			    case SMI_BASETYPE_INTEGER32:
				(yyval.valuePtr) = smiMalloc(sizeof(SmiValue));
				(yyval.valuePtr)->basetype = SMI_BASETYPE_INTEGER32;
				(yyval.valuePtr)->value.integer32 = strtol((yyvsp[(1) - (1)].text), NULL, 16);
				break;
			    case SMI_BASETYPE_INTEGER64:
				(yyval.valuePtr) = smiMalloc(sizeof(SmiValue));
				(yyval.valuePtr)->basetype = SMI_BASETYPE_INTEGER64;
				(yyval.valuePtr)->value.integer64 = strtoll((yyvsp[(1) - (1)].text), NULL, 16);
				break;
			    case SMI_BASETYPE_OCTETSTRING:
				(yyval.valuePtr) = smiMalloc(sizeof(SmiValue));
				(yyval.valuePtr)->basetype = SMI_BASETYPE_OCTETSTRING;
				(yyval.valuePtr)->value.ptr = hexToStr((yyvsp[(1) - (1)].text),strlen((yyvsp[(1) - (1)].text)));
				(yyval.valuePtr)->len = strlen((yyval.valuePtr)->value.ptr);
				break;
			    default:
				smiPrintError(thisParserPtr,
					      ERR_UNEXPECTED_VALUETYPE);
				(yyval.valuePtr) = NULL;
				break;
			    }
			;}
    break;

  case 252:
#line 2836 "parser-sming.y"
    {
			    /* Note: might also be an OID */
			    switch (defaultBasetype) {
			    case SMI_BASETYPE_FLOAT32:
			    	(yyval.valuePtr) = smiMalloc(sizeof(SmiValue));
				(yyval.valuePtr)->basetype = SMI_BASETYPE_FLOAT32;
				(yyval.valuePtr)->value.float32 = strtof((yyvsp[(1) - (1)].text),NULL);
				if(errno == ERANGE){
					smiPrintError(thisParserPtr, 
					ERR_FLOAT_OVERFLOW, (yyvsp[(1) - (1)].text));
					errno = 0;
				}
				break;
			    case SMI_BASETYPE_FLOAT64:
			    	(yyval.valuePtr) = smiMalloc(sizeof(SmiValue));
				(yyval.valuePtr)->basetype = SMI_BASETYPE_FLOAT64;
				(yyval.valuePtr)->value.float64 = strtod((yyvsp[(1) - (1)].text),NULL);
				if(errno == ERANGE){
					smiPrintError(thisParserPtr, 
					ERR_FLOAT_OVERFLOW, (yyvsp[(1) - (1)].text));
					errno = 0;
				}
				break;
			    case SMI_BASETYPE_FLOAT128:
			    	(yyval.valuePtr) = smiMalloc(sizeof(SmiValue));
				(yyval.valuePtr)->basetype = SMI_BASETYPE_FLOAT128;
				(yyval.valuePtr)->value.float128 = strtold((yyvsp[(1) - (1)].text),NULL);
				if(errno == ERANGE){
					smiPrintError(thisParserPtr, 
					ERR_FLOAT_OVERFLOW, (yyvsp[(1) - (1)].text));
					errno = 0;
				}
				break;		
			    case SMI_BASETYPE_OBJECTIDENTIFIER:
				(yyval.valuePtr) = smiMalloc(sizeof(SmiValue));
				(yyval.valuePtr)->basetype = SMI_BASETYPE_OBJECTIDENTIFIER;
				(yyval.valuePtr)->value.ptr = smiMalloc(strlen((yyvsp[(1) - (1)].text))+1);
				strcpy((yyval.valuePtr)->value.ptr,(yyvsp[(1) - (1)].text));
				(yyval.valuePtr)->len = strlen((yyval.valuePtr)->value.ptr);
				break;
			    default:
				smiPrintError(thisParserPtr,
					      ERR_UNEXPECTED_VALUETYPE);
				(yyval.valuePtr) = NULL;
				break;
			    }
			;}
    break;

  case 253:
#line 2884 "parser-sming.y"
    {
		
			    switch (defaultBasetype) {
			    case SMI_BASETYPE_FLOAT32:
			    	(yyval.valuePtr) = smiMalloc(sizeof(SmiValue));
				(yyval.valuePtr)->basetype = SMI_BASETYPE_FLOAT32;
				(yyval.valuePtr)->value.float32 = - strtof((yyvsp[(2) - (2)].text),NULL);
				if(errno == ERANGE){
					smiPrintError(thisParserPtr, 
					ERR_FLOAT_OVERFLOW, (yyvsp[(2) - (2)].text));
					errno = 0;
				}
				break;
			    case SMI_BASETYPE_FLOAT64:
			    	(yyval.valuePtr) = smiMalloc(sizeof(SmiValue));
				(yyval.valuePtr)->basetype = SMI_BASETYPE_FLOAT64;
				(yyval.valuePtr)->value.float64 = - strtof((yyvsp[(2) - (2)].text),NULL);
				if(errno == ERANGE){
					smiPrintError(thisParserPtr, 
					ERR_FLOAT_OVERFLOW, (yyvsp[(2) - (2)].text));
					errno = 0;
				}
				break;
			    case SMI_BASETYPE_FLOAT128:
			    	(yyval.valuePtr) = smiMalloc(sizeof(SmiValue));
				(yyval.valuePtr)->basetype = SMI_BASETYPE_FLOAT128;
				(yyval.valuePtr)->value.float128 = - strtof((yyvsp[(2) - (2)].text),NULL);
				if(errno == ERANGE){
					smiPrintError(thisParserPtr, 
					ERR_FLOAT_OVERFLOW, (yyvsp[(2) - (2)].text));
					errno = 0;
				}
				break;		
			    default:
				smiPrintError(thisParserPtr,
					      ERR_UNEXPECTED_VALUETYPE);
				(yyval.valuePtr) = NULL;
				break;
			    }
			;}
    break;

  case 254:
#line 2925 "parser-sming.y"
    {
				  /* Note: might also be an OID */
			    switch (defaultBasetype) {
			    case SMI_BASETYPE_FLOAT32:
			    	(yyval.valuePtr) = smiMalloc(sizeof(SmiValue));
				(yyval.valuePtr)->basetype = SMI_BASETYPE_FLOAT32;
				(yyval.valuePtr)->value.float32 = strtof((yyvsp[(1) - (1)].text),NULL);
				if(errno == ERANGE){
					smiPrintError(thisParserPtr, 
					ERR_FLOAT_OVERFLOW, (yyvsp[(1) - (1)].text));
					errno = 0;
				}
				break;
			    case SMI_BASETYPE_FLOAT64:
			    	(yyval.valuePtr) = smiMalloc(sizeof(SmiValue));
				(yyval.valuePtr)->basetype = SMI_BASETYPE_FLOAT64;
				(yyval.valuePtr)->value.float64 = strtod((yyvsp[(1) - (1)].text),NULL);
				if(errno == ERANGE){
					smiPrintError(thisParserPtr, 
					ERR_FLOAT_OVERFLOW, (yyvsp[(1) - (1)].text));
					errno = 0;
				}
				break;
			    case SMI_BASETYPE_FLOAT128:
			    	(yyval.valuePtr) = smiMalloc(sizeof(SmiValue));
				(yyval.valuePtr)->basetype = SMI_BASETYPE_FLOAT128;
				(yyval.valuePtr)->value.float128 = strtold((yyvsp[(1) - (1)].text),NULL);
				if(errno == ERANGE){
					smiPrintError(thisParserPtr, 
					ERR_FLOAT_OVERFLOW, (yyvsp[(1) - (1)].text));
					errno = 0;
				}
				break;
				default:
				smiPrintError(thisParserPtr,
					      ERR_UNEXPECTED_VALUETYPE);
				(yyval.valuePtr) = NULL;
				break;
			    }	
			;}
    break;

  case 255:
#line 2966 "parser-sming.y"
    {
			    if (defaultBasetype == SMI_BASETYPE_OCTETSTRING) {
				(yyval.valuePtr) = smiMalloc(sizeof(SmiValue));
				(yyval.valuePtr)->basetype = SMI_BASETYPE_OCTETSTRING;
				(yyval.valuePtr)->value.ptr = (yyvsp[(1) - (1)].text);
				(yyval.valuePtr)->len = strlen((yyvsp[(1) - (1)].text));
			    } else {
				smiPrintError(thisParserPtr,
					      ERR_UNEXPECTED_VALUETYPE);
				(yyval.valuePtr) = NULL;
			    }
			;}
    break;

  case 256:
#line 2979 "parser-sming.y"
    {
			    /* Note: might be an Enumeration item or OID */
			    /* TODO: convert if it's an oid? */
			    switch (defaultBasetype) {
			    case SMI_BASETYPE_ENUM:
				(yyval.valuePtr) = smiMalloc(sizeof(SmiValue));
				(yyval.valuePtr)->basetype = SMI_BASETYPE_ENUM;
				(yyval.valuePtr)->value.ptr = (yyvsp[(1) - (1)].text);
				(yyval.valuePtr)->len = strlen((yyvsp[(1) - (1)].text));
				/* TODO: XXX convert to int */
				break;
			    case SMI_BASETYPE_OBJECTIDENTIFIER:
				(yyval.valuePtr) = smiMalloc(sizeof(SmiValue));
				(yyval.valuePtr)->basetype = SMI_BASETYPE_OBJECTIDENTIFIER;
				(yyval.valuePtr)->value.ptr = (yyvsp[(1) - (1)].text);
				(yyval.valuePtr)->len = strlen((yyvsp[(1) - (1)].text));
				/* TODO: XXX convert to oid if found */
				break;
				case SMI_BASETYPE_POINTER:
				(yyval.valuePtr) = smiMalloc(sizeof(SmiValue));
				(yyval.valuePtr)->basetype = SMI_BASETYPE_OBJECTIDENTIFIER;
				(yyval.valuePtr)->value.ptr = (yyvsp[(1) - (1)].text);
				(yyval.valuePtr)->len = strlen((yyvsp[(1) - (1)].text));
				/* TODO: XXX check if valid reference found */
				break;
			    default:
				smiPrintError(thisParserPtr,
					      ERR_UNEXPECTED_VALUETYPE);
				(yyval.valuePtr) = NULL;
				break;
			    }
			;}
    break;

  case 257:
#line 3012 "parser-sming.y"
    {
			    if (defaultBasetype == SMI_BASETYPE_OBJECTIDENTIFIER){
			    (yyval.valuePtr) = smiMalloc(sizeof(SmiValue));
				(yyval.valuePtr)->basetype = SMI_BASETYPE_OBJECTIDENTIFIER;
				(yyval.valuePtr)->value.ptr = (yyvsp[(1) - (1)].text);
				(yyval.valuePtr)->len = strlen((yyval.valuePtr)->value.ptr);
			    }
			    else
			    	smiPrintError(thisParserPtr,
					      ERR_UNEXPECTED_VALUETYPE);
			;}
    break;

  case 258:
#line 3026 "parser-sming.y"
    {
			    char *s;

			    s = smiMalloc(strlen((yyvsp[(1) - (3)].text)) +
					  strlen((yyvsp[(3) - (3)].text)) + 3);
			    sprintf(s, "%s::%s", (yyvsp[(1) - (3)].text), (yyvsp[(3) - (3)].text));
			    (yyval.text) = s;
			;}
    break;

  case 259:
#line 3035 "parser-sming.y"
    {
				(yyval.text) = smiMalloc(strlen((yyvsp[(1) - (1)].text))+1);
				strcpy((yyval.text),(yyvsp[(1) - (1)].text));
			;}
    break;

  case 260:
#line 3042 "parser-sming.y"
    {
			    (yyval.status) = SMI_STATUS_CURRENT;
			;}
    break;

  case 261:
#line 3046 "parser-sming.y"
    {
			    (yyval.status) = SMI_STATUS_DEPRECATED;
			;}
    break;

  case 262:
#line 3050 "parser-sming.y"
    {
			    (yyval.status) = SMI_STATUS_OBSOLETE;
			;}
    break;

  case 263:
#line 3056 "parser-sming.y"
    {
			    (yyval.valuePtr) = smiMalloc(sizeof(SmiValue));
			    (yyval.valuePtr)->basetype = SMI_BASETYPE_UNSIGNED64;
			    (yyval.valuePtr)->value.unsigned64 = strtoull((yyvsp[(1) - (1)].text), NULL, 16);
			;}
    break;

  case 264:
#line 3062 "parser-sming.y"
    {
			    (yyval.valuePtr) = smiMalloc(sizeof(SmiValue));
			    (yyval.valuePtr)->basetype = SMI_BASETYPE_UNSIGNED64;
			    (yyval.valuePtr)->value.unsigned64 = strtoull((yyvsp[(1) - (1)].text), NULL, 10);
			;}
    break;

  case 265:
#line 3070 "parser-sming.y"
    {
			    if(bitsFlag){
			    smiPrintError(thisParserPtr,
					      ERR_BITS_NUMBER_NEGATIVE);
			    (yyval.valuePtr) = NULL;
			    }
			    (yyval.valuePtr) = smiMalloc(sizeof(SmiValue));
			    (yyval.valuePtr)->basetype = SMI_BASETYPE_INTEGER64;
			    (yyval.valuePtr)->value.integer64 = - strtoll((yyvsp[(2) - (2)].text), NULL, 10);
			;}
    break;

  case 266:
#line 3083 "parser-sming.y"
    {
			    (yyval.valuePtr) = (yyvsp[(1) - (1)].valuePtr);
			;}
    break;

  case 267:
#line 3087 "parser-sming.y"
    {
			    (yyval.valuePtr) = (yyvsp[(1) - (1)].valuePtr);
			;}
    break;

  case 268:
#line 3097 "parser-sming.y"
    {
			    (yyval.rc) = 0;
			;}
    break;

  case 269:
#line 3104 "parser-sming.y"
    {
			    (yyval.rc) = 0;
			;}
    break;

  case 270:
#line 3110 "parser-sming.y"
    {
			    (yyval.rc) = 0;
			;}
    break;

  case 271:
#line 3116 "parser-sming.y"
    {
			    (yyval.rc) = 0;
			;}
    break;


/* Line 1267 of yacc.c.  */
#line 5383 "parser-sming.c"
      default: break;
    }
  YY_SYMBOL_PRINT ("-> $$ =", yyr1[yyn], &yyval, &yyloc);

  YYPOPSTACK (yylen);
  yylen = 0;
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
#if ! YYERROR_VERBOSE
      yyerror (YY_("syntax error"));
#else
      {
	YYSIZE_T yysize = yysyntax_error (0, yystate, yychar);
	if (yymsg_alloc < yysize && yymsg_alloc < YYSTACK_ALLOC_MAXIMUM)
	  {
	    YYSIZE_T yyalloc = 2 * yysize;
	    if (! (yysize <= yyalloc && yyalloc <= YYSTACK_ALLOC_MAXIMUM))
	      yyalloc = YYSTACK_ALLOC_MAXIMUM;
	    if (yymsg != yymsgbuf)
	      YYSTACK_FREE (yymsg);
	    yymsg = (char *) YYSTACK_ALLOC (yyalloc);
	    if (yymsg)
	      yymsg_alloc = yyalloc;
	    else
	      {
		yymsg = yymsgbuf;
		yymsg_alloc = sizeof yymsgbuf;
	      }
	  }

	if (0 < yysize && yysize <= yymsg_alloc)
	  {
	    (void) yysyntax_error (yymsg, yystate, yychar);
	    yyerror (yymsg);
	  }
	else
	  {
	    yyerror (YY_("syntax error"));
	    if (yysize != 0)
	      goto yyexhaustedlab;
	  }
      }
#endif
    }



  if (yyerrstatus == 3)
    {
      /* If just tried and failed to reuse look-ahead token after an
	 error, discard it.  */

      if (yychar <= YYEOF)
	{
	  /* Return failure if at end of input.  */
	  if (yychar == YYEOF)
	    YYABORT;
	}
      else
	{
	  yydestruct ("Error: discarding",
		      yytoken, &yylval);
	  yychar = YYEMPTY;
	}
    }

  /* Else will try to reuse look-ahead token after shifting the error
     token.  */
  goto yyerrlab1;


/*---------------------------------------------------.
| yyerrorlab -- error raised explicitly by YYERROR.  |
`---------------------------------------------------*/
yyerrorlab:

  /* Pacify compilers like GCC when the user code never invokes
     YYERROR and the label yyerrorlab therefore never appears in user
     code.  */
  if (/*CONSTCOND*/ 0)
     goto yyerrorlab;

  /* Do not reclaim the symbols of the rule which action triggered
     this YYERROR.  */
  YYPOPSTACK (yylen);
  yylen = 0;
  YY_STACK_PRINT (yyss, yyssp);
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


      yydestruct ("Error: popping",
		  yystos[yystate], yyvsp);
      YYPOPSTACK (1);
      yystate = *yyssp;
      YY_STACK_PRINT (yyss, yyssp);
    }

  if (yyn == YYFINAL)
    YYACCEPT;

  *++yyvsp = yylval;


  /* Shift the error token.  */
  YY_SYMBOL_PRINT ("Shifting", yystos[yyn], yyvsp, yylsp);

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
/*-------------------------------------------------.
| yyexhaustedlab -- memory exhaustion comes here.  |
`-------------------------------------------------*/
yyexhaustedlab:
  yyerror (YY_("memory exhausted"));
  yyresult = 2;
  /* Fall through.  */
#endif

yyreturn:
  if (yychar != YYEOF && yychar != YYEMPTY)
     yydestruct ("Cleanup: discarding lookahead",
		 yytoken, &yylval);
  /* Do not reclaim the symbols of the rule which action triggered
     this YYABORT or YYACCEPT.  */
  YYPOPSTACK (yylen);
  YY_STACK_PRINT (yyss, yyssp);
  while (yyssp != yyss)
    {
      yydestruct ("Cleanup: popping",
		  yystos[*yyssp], yyvsp);
      YYPOPSTACK (1);
    }
#ifndef yyoverflow
  if (yyss != yyssa)
    YYSTACK_FREE (yyss);
#endif
#if YYERROR_VERBOSE
  if (yymsg != yymsgbuf)
    YYSTACK_FREE (yymsg);
#endif
  /* Make sure YYID is used.  */
  return YYID (yyresult);
}


#line 3121 "parser-sming.y"


#endif
			

