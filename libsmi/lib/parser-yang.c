/* A Bison parser, made by GNU Bison 2.7.12-4996.  */

/* Bison implementation for Yacc-like parsers in C
   
      Copyright (C) 1984, 1989-1990, 2000-2013 Free Software Foundation, Inc.
   
   This program is free software: you can redistribute it and/or modify
   it under the terms of the GNU General Public License as published by
   the Free Software Foundation, either version 3 of the License, or
   (at your option) any later version.
   
   This program is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY; without even the implied warranty of
   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
   GNU General Public License for more details.
   
   You should have received a copy of the GNU General Public License
   along with this program.  If not, see <http://www.gnu.org/licenses/>.  */

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
#define YYBISON_VERSION "2.7.12-4996"

/* Skeleton name.  */
#define YYSKELETON_NAME "yacc.c"

/* Pure parsers.  */
#define YYPURE 1

/* Push parsers.  */
#define YYPUSH 0

/* Pull parsers.  */
#define YYPULL 1


/* Substitute the variable and function names.  */
#define yyparse         yangparse
#define yylex           yanglex
#define yyerror         yangerror
#define yylval          yanglval
#define yychar          yangchar
#define yydebug         yangdebug
#define yynerrs         yangnerrs

/* Copy the first part of user declarations.  */
/* Line 371 of yacc.c  */
#line 17 "parser-yang.y"


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


/* Line 371 of yacc.c  */
#line 320 "parser-yang.c"

# ifndef YY_NULL
#  if defined __cplusplus && 201103L <= __cplusplus
#   define YY_NULL nullptr
#  else
#   define YY_NULL 0
#  endif
# endif

/* Enabling verbose error messages.  */
#ifdef YYERROR_VERBOSE
# undef YYERROR_VERBOSE
# define YYERROR_VERBOSE 1
#else
# define YYERROR_VERBOSE 0
#endif

/* In a future release of Bison, this section will be replaced
   by #include "parser-yang.tab.h".  */
#ifndef YY_YANG_PARSER_YANG_TAB_H_INCLUDED
# define YY_YANG_PARSER_YANG_TAB_H_INCLUDED
/* Enabling traces.  */
#ifndef YYDEBUG
# define YYDEBUG 1
#endif
#if YYDEBUG
extern int yangdebug;
#endif

/* Tokens.  */
#ifndef YYTOKENTYPE
# define YYTOKENTYPE
   /* Put the tokens into the symbol table, so that GDB and other debuggers
      know about them.  */
   enum yytokentype {
     augmentKeyword = 258,
     belongs_toKeyword = 259,
     choiceKeyword = 260,
     configKeyword = 261,
     contactKeyword = 262,
     containerKeyword = 263,
     defaultKeyword = 264,
     descriptionKeyword = 265,
     fractionDigitsKeyword = 266,
     enumKeyword = 267,
     error_app_tagKeyword = 268,
     error_messageKeyword = 269,
     extensionKeyword = 270,
     groupingKeyword = 271,
     importKeyword = 272,
     includeKeyword = 273,
     keyKeyword = 274,
     leafKeyword = 275,
     leaf_listKeyword = 276,
     lengthKeyword = 277,
     listKeyword = 278,
     mandatoryKeyword = 279,
     max_elementsKeyword = 280,
     min_elementsKeyword = 281,
     moduleKeyword = 282,
     submoduleKeyword = 283,
     mustKeyword = 284,
     namespaceKeyword = 285,
     ordered_byKeyword = 286,
     organizationKeyword = 287,
     prefixKeyword = 288,
     rangeKeyword = 289,
     referenceKeyword = 290,
     patternKeyword = 291,
     revisionKeyword = 292,
     statusKeyword = 293,
     typeKeyword = 294,
     typedefKeyword = 295,
     uniqueKeyword = 296,
     unitsKeyword = 297,
     usesKeyword = 298,
     valueKeyword = 299,
     whenKeyword = 300,
     bitKeyword = 301,
     pathKeyword = 302,
     anyXMLKeyword = 303,
     deprecatedKeyword = 304,
     currentKeyword = 305,
     obsoleteKeyword = 306,
     trueKeyword = 307,
     falseKeyword = 308,
     caseKeyword = 309,
     inputKeyword = 310,
     outputKeyword = 311,
     rpcKeyword = 312,
     notificationKeyword = 313,
     argumentKeyword = 314,
     yangversionKeyword = 315,
     baseKeyword = 316,
     deviationKeyword = 317,
     deviateKeyword = 318,
     featureKeyword = 319,
     identityKeyword = 320,
     ifFeatureKeyword = 321,
     positionKeyword = 322,
     presenceKeyword = 323,
     refineKeyword = 324,
     requireInstanceKeyword = 325,
     yinElementKeyword = 326,
     notSupportedKeyword = 327,
     addKeyword = 328,
     deleteKeyword = 329,
     replaceKeyword = 330,
     complexTypeKeyword = 331,
     extendsKeyword = 332,
     abstractKeyword = 333,
     instanceKeyword = 334,
     instanceListKeyword = 335,
     instanceTypeKeyword = 336,
     identifier = 337,
     identifierRefArg = 338,
     identifierRefArgStr = 339,
     dateString = 340,
     yangVersion = 341,
     qString = 342,
     uqString = 343,
     decimalNumber = 344,
     floatNumber = 345,
     hexNumber = 346
   };
#endif


#if ! defined YYSTYPE && ! defined YYSTYPE_IS_DECLARED
typedef union YYSTYPE
{
/* Line 387 of yacc.c  */
#line 276 "parser-yang.y"

    int             rc;                 /* >=0: ok, <0: error    */
    char            *text;


/* Line 387 of yacc.c  */
#line 460 "parser-yang.c"
} YYSTYPE;
# define YYSTYPE_IS_TRIVIAL 1
# define yystype YYSTYPE /* obsolescent; will be withdrawn */
# define YYSTYPE_IS_DECLARED 1
#endif


#ifdef YYPARSE_PARAM
#if defined __STDC__ || defined __cplusplus
int yangparse (void *YYPARSE_PARAM);
#else
int yangparse ();
#endif
#else /* ! YYPARSE_PARAM */
#if defined __STDC__ || defined __cplusplus
int yangparse (struct Parser *parserPtr);
#else
int yangparse ();
#endif
#endif /* ! YYPARSE_PARAM */

#endif /* !YY_YANG_PARSER_YANG_TAB_H_INCLUDED  */

/* Copy the second part of user declarations.  */

/* Line 390 of yacc.c  */
#line 487 "parser-yang.c"

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
# if defined YYENABLE_NLS && YYENABLE_NLS
#  if ENABLE_NLS
#   include <libintl.h> /* INFRINGES ON USER NAME SPACE */
#   define YY_(Msgid) dgettext ("bison-runtime", Msgid)
#  endif
# endif
# ifndef YY_
#  define YY_(Msgid) Msgid
# endif
#endif

#ifndef __attribute__
/* This feature is available in gcc versions 2.5 and later.  */
# if (! defined __GNUC__ || __GNUC__ < 2 \
      || (__GNUC__ == 2 && __GNUC_MINOR__ < 5))
#  define __attribute__(Spec) /* empty */
# endif
#endif

/* Suppress unused-variable warnings by "using" E.  */
#if ! defined lint || defined __GNUC__
# define YYUSE(E) ((void) (E))
#else
# define YYUSE(E) /* empty */
#endif


/* Identity function, used to suppress warnings about constant conditions.  */
#ifndef lint
# define YYID(N) (N)
#else
#if (defined __STDC__ || defined __C99__FUNC__ \
     || defined __cplusplus || defined _MSC_VER)
static int
YYID (int yyi)
#else
static int
YYID (yyi)
    int yyi;
#endif
{
  return yyi;
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
#    if ! defined _ALLOCA_H && ! defined EXIT_SUCCESS && (defined __STDC__ || defined __C99__FUNC__ \
     || defined __cplusplus || defined _MSC_VER)
#     include <stdlib.h> /* INFRINGES ON USER NAME SPACE */
      /* Use EXIT_SUCCESS as a witness for stdlib.h.  */
#     ifndef EXIT_SUCCESS
#      define EXIT_SUCCESS 0
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
#  if (defined __cplusplus && ! defined EXIT_SUCCESS \
       && ! ((defined YYMALLOC || defined malloc) \
	     && (defined YYFREE || defined free)))
#   include <stdlib.h> /* INFRINGES ON USER NAME SPACE */
#   ifndef EXIT_SUCCESS
#    define EXIT_SUCCESS 0
#   endif
#  endif
#  ifndef YYMALLOC
#   define YYMALLOC malloc
#   if ! defined malloc && ! defined EXIT_SUCCESS && (defined __STDC__ || defined __C99__FUNC__ \
     || defined __cplusplus || defined _MSC_VER)
void *malloc (YYSIZE_T); /* INFRINGES ON USER NAME SPACE */
#   endif
#  endif
#  ifndef YYFREE
#   define YYFREE free
#   if ! defined free && ! defined EXIT_SUCCESS && (defined __STDC__ || defined __C99__FUNC__ \
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
  yytype_int16 yyss_alloc;
  YYSTYPE yyvs_alloc;
};

/* The size of the maximum gap between one aligned stack and the next.  */
# define YYSTACK_GAP_MAXIMUM (sizeof (union yyalloc) - 1)

/* The size of an array large to enough to hold all stacks, each with
   N elements.  */
# define YYSTACK_BYTES(N) \
     ((N) * (sizeof (yytype_int16) + sizeof (YYSTYPE)) \
      + YYSTACK_GAP_MAXIMUM)

# define YYCOPY_NEEDED 1

/* Relocate STACK from its old location to the new one.  The
   local variables YYSIZE and YYSTACKSIZE give the old and new number of
   elements in the stack, and YYPTR gives the new location of the
   stack.  Advance YYPTR to a properly aligned location for the next
   stack.  */
# define YYSTACK_RELOCATE(Stack_alloc, Stack)				\
    do									\
      {									\
	YYSIZE_T yynewbytes;						\
	YYCOPY (&yyptr->Stack_alloc, Stack, yysize);			\
	Stack = &yyptr->Stack_alloc;					\
	yynewbytes = yystacksize * sizeof (*Stack) + YYSTACK_GAP_MAXIMUM; \
	yyptr += yynewbytes / sizeof (*yyptr);				\
      }									\
    while (YYID (0))

#endif

#if defined YYCOPY_NEEDED && YYCOPY_NEEDED
/* Copy COUNT objects from SRC to DST.  The source and destination do
   not overlap.  */
# ifndef YYCOPY
#  if defined __GNUC__ && 1 < __GNUC__
#   define YYCOPY(Dst, Src, Count) \
      __builtin_memcpy (Dst, Src, (Count) * sizeof (*(Src)))
#  else
#   define YYCOPY(Dst, Src, Count)              \
      do                                        \
        {                                       \
          YYSIZE_T yyi;                         \
          for (yyi = 0; yyi < (Count); yyi++)   \
            (Dst)[yyi] = (Src)[yyi];            \
        }                                       \
      while (YYID (0))
#  endif
# endif
#endif /* !YYCOPY_NEEDED */

/* YYFINAL -- State number of the termination state.  */
#define YYFINAL  81
/* YYLAST -- Last index in YYTABLE.  */
#define YYLAST   1593

/* YYNTOKENS -- Number of terminals.  */
#define YYNTOKENS  95
/* YYNNTS -- Number of nonterminals.  */
#define YYNNTS  237
/* YYNRULES -- Number of rules.  */
#define YYNRULES  617
/* YYNRULES -- Number of states.  */
#define YYNSTATES  941

/* YYTRANSLATE(YYLEX) -- Bison symbol number corresponding to YYLEX.  */
#define YYUNDEFTOK  2
#define YYMAXUTOK   346

#define YYTRANSLATE(YYX)						\
  ((unsigned int) (YYX) <= YYMAXUTOK ? yytranslate[YYX] : YYUNDEFTOK)

/* YYTRANSLATE[YYLEX] -- Bison symbol number corresponding to YYLEX.  */
static const yytype_uint8 yytranslate[] =
{
       0,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,    94,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,    92,     2,    93,     2,     2,     2,     2,
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
      65,    66,    67,    68,    69,    70,    71,    72,    73,    74,
      75,    76,    77,    78,    79,    80,    81,    82,    83,    84,
      85,    86,    87,    88,    89,    90,    91
};

#if YYDEBUG
/* YYPRHS[YYN] -- Index of the first RHS symbol of rule number YYN in
   YYRHS.  */
static const yytype_uint16 yyprhs[] =
{
       0,     0,     3,     5,     7,     8,    20,    21,    33,    35,
      36,    39,    42,    45,    48,    49,    53,    55,    57,    59,
      61,    66,    71,    74,    75,    84,    85,    88,    91,    94,
      95,    99,   100,   104,   106,   108,   110,   112,   114,   116,
     118,   120,   122,   124,   126,   128,   130,   132,   134,   136,
     138,   140,   142,   144,   146,   150,   154,   158,   162,   166,
     170,   174,   176,   178,   180,   184,   188,   189,   197,   198,
     202,   204,   206,   208,   209,   219,   220,   223,   224,   229,
     231,   236,   237,   242,   244,   249,   250,   254,   256,   258,
     260,   262,   266,   267,   272,   274,   279,   280,   284,   286,
     288,   290,   292,   293,   301,   302,   306,   308,   310,   312,
     314,   316,   318,   319,   324,   326,   331,   333,   335,   337,
     339,   341,   343,   345,   349,   353,   355,   358,   359,   364,
     368,   369,   372,   374,   376,   377,   382,   383,   388,   391,
     395,   396,   401,   403,   408,   409,   413,   415,   417,   419,
     421,   425,   427,   432,   433,   437,   439,   441,   443,   445,
     449,   453,   456,   461,   466,   470,   474,   477,   482,   487,
     491,   494,   498,   499,   504,   506,   511,   512,   516,   518,
     520,   522,   524,   528,   531,   535,   537,   541,   542,   544,
     545,   548,   551,   554,   557,   560,   563,   566,   567,   572,
     574,   579,   580,   584,   586,   588,   590,   592,   594,   596,
     598,   600,   602,   604,   606,   610,   614,   615,   620,   622,
     627,   628,   632,   634,   636,   638,   639,   647,   648,   652,
     654,   656,   658,   660,   662,   664,   666,   667,   675,   676,
     680,   682,   684,   686,   688,   690,   692,   694,   696,   698,
     702,   703,   707,   708,   713,   714,   719,   721,   726,   727,
     731,   733,   735,   737,   739,   741,   743,   745,   747,   748,
     755,   759,   760,   764,   766,   768,   770,   772,   776,   780,
     784,   788,   792,   793,   801,   802,   806,   808,   810,   812,
     814,   816,   818,   820,   822,   823,   831,   832,   836,   838,
     840,   842,   844,   846,   848,   850,   852,   854,   855,   863,
     864,   868,   870,   872,   874,   876,   878,   880,   882,   884,
     886,   888,   890,   892,   896,   900,   904,   908,   912,   913,
     918,   920,   925,   926,   930,   932,   934,   936,   938,   940,
     942,   943,   948,   950,   952,   954,   956,   958,   960,   965,
     966,   970,   972,   974,   976,   978,   980,   982,   984,   986,
     988,   990,   992,   994,   995,  1003,  1004,  1008,  1010,  1012,
    1014,  1016,  1018,  1020,  1021,  1028,  1032,  1033,  1037,  1039,
    1041,  1043,  1045,  1047,  1049,  1051,  1052,  1057,  1059,  1064,
    1066,  1067,  1071,  1073,  1075,  1077,  1079,  1081,  1083,  1085,
    1087,  1089,  1090,  1098,  1100,  1104,  1106,  1108,  1110,  1112,
    1114,  1116,  1118,  1122,  1123,  1128,  1130,  1135,  1136,  1140,
    1142,  1144,  1146,  1148,  1150,  1152,  1154,  1156,  1157,  1164,
    1167,  1171,  1173,  1175,  1177,  1178,  1185,  1186,  1191,  1193,
    1198,  1199,  1203,  1205,  1207,  1209,  1211,  1213,  1215,  1217,
    1218,  1226,  1229,  1233,  1235,  1237,  1239,  1241,  1243,  1245,
    1249,  1255,  1256,  1261,  1263,  1268,  1269,  1273,  1275,  1277,
    1279,  1281,  1283,  1285,  1287,  1289,  1290,  1295,  1297,  1302,
    1303,  1307,  1309,  1311,  1313,  1315,  1316,  1321,  1323,  1328,
    1329,  1333,  1335,  1337,  1339,  1341,  1343,  1345,  1347,  1348,
    1353,  1355,  1360,  1361,  1365,  1367,  1369,  1371,  1373,  1375,
    1376,  1381,  1386,  1388,  1389,  1393,  1395,  1397,  1399,  1401,
    1402,  1407,  1412,  1414,  1415,  1420,  1425,  1429,  1433,  1435,
    1437,  1439,  1441,  1443,  1445,  1447,  1449,  1451,  1453,  1455,
    1457,  1459,  1461,  1463,  1465,  1467,  1469,  1471,  1473,  1475,
    1477,  1479,  1481,  1483,  1485,  1487,  1489,  1491,  1493,  1495,
    1497,  1499,  1501,  1503,  1505,  1507,  1509,  1511,  1513,  1515,
    1517,  1519,  1521,  1523,  1525,  1527,  1529,  1531,  1533,  1535,
    1537,  1539,  1541,  1543,  1545,  1547,  1549,  1551,  1553,  1555,
    1557,  1559,  1561,  1563,  1565,  1567,  1569,  1571,  1573,  1575,
    1577,  1579,  1581,  1583,  1585,  1587,  1589,  1591,  1593,  1595,
    1597,  1599,  1601,  1603,  1605,  1607,  1609,  1611
};

/* YYRHS -- A `-1'-separated list of the rules' RHS.  */
static const yytype_int16 yyrhs[] =
{
      96,     0,    -1,    97,    -1,    99,    -1,    -1,    27,   329,
      98,    92,   191,   101,   109,   104,   111,   112,    93,    -1,
      -1,    28,   329,   100,    92,   191,   106,   109,   104,   111,
     112,    93,    -1,   102,    -1,    -1,   103,   102,    -1,   122,
     191,    -1,   121,   191,    -1,   124,   191,    -1,    -1,   104,
     105,   191,    -1,   116,    -1,   117,    -1,   118,    -1,   119,
      -1,   107,   191,   122,   191,    -1,   122,   191,   107,   191,
      -1,   107,   191,    -1,    -1,     4,   329,   108,    92,   191,
     124,   191,    93,    -1,    -1,   109,   110,    -1,   133,   191,
      -1,   130,   191,    -1,    -1,   111,   125,   191,    -1,    -1,
     112,   113,   191,    -1,   316,    -1,   136,    -1,   142,    -1,
     147,    -1,   256,    -1,   114,    -1,   270,    -1,   275,    -1,
     286,    -1,   291,    -1,   216,    -1,   228,    -1,   232,    -1,
     236,    -1,   245,    -1,   311,    -1,   260,    -1,   118,    -1,
     120,    -1,   119,    -1,   226,    -1,    32,   331,   190,    -1,
       7,   331,   190,    -1,    10,   331,   190,    -1,    35,   331,
     190,    -1,    38,   123,   190,    -1,    30,   331,   190,    -1,
      60,    86,   190,    -1,    49,    -1,    50,    -1,    51,    -1,
      33,   327,   190,    -1,    37,   129,    94,    -1,    -1,    37,
     129,   126,    92,   191,   127,    93,    -1,    -1,   127,   128,
     191,    -1,   118,    -1,   119,    -1,    85,    -1,    -1,    17,
     329,   131,    92,   191,   124,   191,   132,    93,    -1,    -1,
     125,   191,    -1,    -1,    18,   329,   134,   135,    -1,    94,
      -1,    92,   191,   132,    93,    -1,    -1,    64,   329,   137,
     138,    -1,    94,    -1,    92,   191,   139,    93,    -1,    -1,
     139,   140,   191,    -1,   141,    -1,   120,    -1,   118,    -1,
     119,    -1,    66,   328,   190,    -1,    -1,    65,   329,   143,
     144,    -1,    94,    -1,    92,   191,   145,    93,    -1,    -1,
     145,   146,   191,    -1,   181,    -1,   120,    -1,   118,    -1,
     119,    -1,    -1,    40,   329,   148,    92,   191,   149,    93,
      -1,    -1,   149,   150,   191,    -1,   151,    -1,   325,    -1,
     326,    -1,   120,    -1,   119,    -1,   118,    -1,    -1,    39,
     328,   152,   153,    -1,    94,    -1,    92,   191,   154,    93,
      -1,   155,    -1,   159,    -1,   165,    -1,   177,    -1,   180,
      -1,   182,    -1,   189,    -1,   156,   191,   158,    -1,   158,
     156,   191,    -1,   158,    -1,   156,   191,    -1,    -1,    34,
     331,   157,   172,    -1,    11,   331,   190,    -1,    -1,   159,
     160,    -1,   161,    -1,   163,    -1,    -1,    22,   331,   162,
     172,    -1,    -1,    36,   331,   164,   172,    -1,   166,   191,
      -1,   165,   166,   191,    -1,    -1,    12,   331,   167,   168,
      -1,    94,    -1,    92,   191,   169,    93,    -1,    -1,   169,
     170,   191,    -1,   171,    -1,   120,    -1,   118,    -1,   119,
      -1,    44,   331,   190,    -1,    94,    -1,    92,   191,   173,
      93,    -1,    -1,   173,   174,   191,    -1,   118,    -1,   119,
      -1,   175,    -1,   176,    -1,    14,   331,   190,    -1,    13,
     331,   190,    -1,   178,   191,    -1,   178,   191,   179,   191,
      -1,   179,   191,   178,   191,    -1,    47,   331,   190,    -1,
      70,   330,   190,    -1,   181,   191,    -1,   181,   191,   179,
     191,    -1,   179,   191,   181,   191,    -1,    61,   328,   190,
      -1,   183,   191,    -1,   182,   183,   191,    -1,    -1,    46,
      82,   184,   185,    -1,    94,    -1,    92,   191,   186,    93,
      -1,    -1,   186,   187,   191,    -1,   188,    -1,   118,    -1,
     119,    -1,   120,    -1,    67,   331,   190,    -1,   151,   191,
      -1,   189,   151,   191,    -1,    94,    -1,    92,   192,    93,
      -1,    -1,   192,    -1,    -1,   213,   192,    -1,   193,   192,
      -1,   204,   192,    -1,   208,   192,    -1,   212,   192,    -1,
     198,   192,    -1,   199,   192,    -1,    -1,    76,   329,   194,
     195,    -1,    94,    -1,    92,   191,   196,    93,    -1,    -1,
     197,   191,   196,    -1,   256,    -1,   141,    -1,   114,    -1,
     221,    -1,   147,    -1,   243,    -1,   242,    -1,   264,    -1,
     118,    -1,   120,    -1,   119,    -1,    78,    52,   190,    -1,
      78,    53,   190,    -1,    -1,    77,   328,   200,   201,    -1,
      94,    -1,    92,   191,   202,    93,    -1,    -1,   203,   191,
     202,    -1,   118,    -1,   120,    -1,   119,    -1,    -1,    79,
     329,   205,    92,   191,   206,    93,    -1,    -1,   207,   191,
     206,    -1,   141,    -1,   115,    -1,   114,    -1,   221,    -1,
     274,    -1,   227,    -1,   270,    -1,    -1,    80,   329,   209,
      92,   191,   210,    93,    -1,    -1,   211,   191,   210,    -1,
     141,    -1,   115,    -1,   114,    -1,   221,    -1,   274,    -1,
     270,    -1,   240,    -1,   241,    -1,   242,    -1,    81,   328,
     190,    -1,    -1,    83,   214,   190,    -1,    -1,    83,   331,
     215,   190,    -1,    -1,     8,   329,   217,   218,    -1,    94,
      -1,    92,   191,   219,    93,    -1,    -1,   220,   191,   219,
      -1,   141,    -1,   115,    -1,   114,    -1,   256,    -1,   221,
      -1,   274,    -1,   225,    -1,   147,    -1,    -1,    29,   331,
     222,    92,   223,    93,    -1,    29,   331,    94,    -1,    -1,
     223,   224,   191,    -1,   175,    -1,   176,    -1,   118,    -1,
     119,    -1,    68,   331,   190,    -1,     6,    52,   190,    -1,
       6,    53,   190,    -1,    24,    52,   190,    -1,    24,    53,
     190,    -1,    -1,    20,   329,   229,    92,   191,   230,    93,
      -1,    -1,   230,   231,   191,    -1,   141,    -1,   325,    -1,
     221,    -1,   115,    -1,   227,    -1,   151,    -1,   326,    -1,
     274,    -1,    -1,    21,   329,   233,    92,   191,   234,    93,
      -1,    -1,   234,   235,   191,    -1,   221,    -1,   115,    -1,
     151,    -1,   141,    -1,   325,    -1,   274,    -1,   240,    -1,
     241,    -1,   242,    -1,    -1,    23,   329,   237,    92,   191,
     238,    93,    -1,    -1,   238,   239,   191,    -1,   221,    -1,
     115,    -1,   240,    -1,   241,    -1,   242,    -1,   243,    -1,
     244,    -1,   114,    -1,   147,    -1,   256,    -1,   274,    -1,
     141,    -1,    25,   331,   190,    -1,    26,   331,   190,    -1,
      31,   331,   190,    -1,    19,   331,   190,    -1,    41,   331,
     190,    -1,    -1,     5,   329,   246,   247,    -1,    94,    -1,
      92,   191,   248,    93,    -1,    -1,   248,   249,   191,    -1,
     115,    -1,   326,    -1,   274,    -1,   141,    -1,   227,    -1,
     250,    -1,    -1,    54,   329,   251,   252,    -1,   216,    -1,
     228,    -1,   232,    -1,   236,    -1,   311,    -1,    94,    -1,
      92,   191,   253,    93,    -1,    -1,   253,   254,   191,    -1,
     118,    -1,   120,    -1,   119,    -1,   274,    -1,   141,    -1,
     255,    -1,   216,    -1,   228,    -1,   232,    -1,   236,    -1,
     311,    -1,   260,    -1,    -1,    16,   329,   257,    92,   191,
     258,    93,    -1,    -1,   258,   259,   191,    -1,   120,    -1,
     118,    -1,   119,    -1,   114,    -1,   256,    -1,   147,    -1,
      -1,    43,   328,   261,    92,   262,    93,    -1,    43,   328,
      94,    -1,    -1,   262,   263,   191,    -1,   118,    -1,   119,
      -1,   120,    -1,   274,    -1,   141,    -1,   264,    -1,   270,
      -1,    -1,    69,   331,   265,   266,    -1,    94,    -1,    92,
     191,   267,    93,    -1,   268,    -1,    -1,   268,   269,   191,
      -1,   221,    -1,   225,    -1,   226,    -1,   118,    -1,   119,
      -1,   326,    -1,   227,    -1,   241,    -1,   240,    -1,    -1,
       3,   331,   271,    92,   191,   272,    93,    -1,   191,    -1,
     272,   273,   191,    -1,   274,    -1,   141,    -1,   118,    -1,
     119,    -1,   120,    -1,   114,    -1,   250,    -1,    45,   331,
     190,    -1,    -1,    57,   329,   276,   277,    -1,    94,    -1,
      92,   191,   278,    93,    -1,    -1,   278,   279,   191,    -1,
     141,    -1,   118,    -1,   119,    -1,   120,    -1,   147,    -1,
     256,    -1,   280,    -1,   284,    -1,    -1,    55,   281,    92,
     191,   282,    93,    -1,   283,   191,    -1,   282,   283,   191,
      -1,   114,    -1,   256,    -1,   147,    -1,    -1,    56,   285,
      92,   191,   282,    93,    -1,    -1,    58,   329,   287,   288,
      -1,    94,    -1,    92,   191,   289,    93,    -1,    -1,   289,
     290,   191,    -1,   141,    -1,   118,    -1,   119,    -1,   120,
      -1,   114,    -1,   256,    -1,   147,    -1,    -1,    62,   331,
     292,    92,   191,   293,    93,    -1,   294,   191,    -1,   293,
     294,   191,    -1,   118,    -1,   119,    -1,   295,    -1,   296,
      -1,   301,    -1,   306,    -1,    63,    72,    94,    -1,    63,
      72,    92,   191,    93,    -1,    -1,    63,    73,   297,   298,
      -1,    94,    -1,    92,   191,   299,    93,    -1,    -1,   299,
     300,   191,    -1,   325,    -1,   221,    -1,   244,    -1,   326,
      -1,   226,    -1,   227,    -1,   241,    -1,   240,    -1,    -1,
      63,    74,   302,   303,    -1,    94,    -1,    92,   191,   304,
      93,    -1,    -1,   304,   305,   191,    -1,   325,    -1,   221,
      -1,   244,    -1,   326,    -1,    -1,    63,    75,   307,   308,
      -1,    94,    -1,    92,   191,   309,    93,    -1,    -1,   309,
     310,   191,    -1,   151,    -1,   325,    -1,   326,    -1,   226,
      -1,   227,    -1,   241,    -1,   240,    -1,    -1,    48,   329,
     312,   313,    -1,    94,    -1,    92,   191,   314,    93,    -1,
      -1,   314,   315,   191,    -1,   115,    -1,   274,    -1,   141,
      -1,   221,    -1,   227,    -1,    -1,    15,   329,   317,   318,
      -1,    92,   191,   319,    93,    -1,    94,    -1,    -1,   319,
     320,   191,    -1,   321,    -1,   120,    -1,   118,    -1,   119,
      -1,    -1,    59,   329,   322,   323,    -1,    92,   191,   324,
      93,    -1,    94,    -1,    -1,    71,    52,   190,   191,    -1,
      71,    53,   190,   191,    -1,    42,   331,   190,    -1,     9,
     331,   190,    -1,   329,    -1,    83,    -1,    84,    -1,   329,
      -1,    76,    -1,    77,    -1,    78,    -1,    79,    -1,    80,
      -1,    81,    -1,    82,    -1,     3,    -1,     4,    -1,     5,
      -1,     6,    -1,     7,    -1,     8,    -1,     9,    -1,    10,
      -1,    12,    -1,    13,    -1,    14,    -1,    15,    -1,    16,
      -1,    17,    -1,    18,    -1,    19,    -1,    20,    -1,    21,
      -1,    22,    -1,    23,    -1,    24,    -1,    25,    -1,    26,
      -1,    27,    -1,    28,    -1,    29,    -1,    30,    -1,    31,
      -1,    32,    -1,    33,    -1,    34,    -1,    35,    -1,    36,
      -1,    37,    -1,    38,    -1,    39,    -1,    40,    -1,    41,
      -1,    42,    -1,    43,    -1,    44,    -1,    45,    -1,    46,
      -1,    47,    -1,    48,    -1,    49,    -1,    50,    -1,    51,
      -1,    52,    -1,    53,    -1,    54,    -1,    55,    -1,    56,
      -1,    57,    -1,    58,    -1,    59,    -1,    60,    -1,    61,
      -1,    62,    -1,    63,    -1,    64,    -1,    65,    -1,    66,
      -1,    67,    -1,    68,    -1,    69,    -1,    70,    -1,    71,
      -1,    72,    -1,    73,    -1,    74,    -1,    75,    -1,    52,
      -1,    53,    -1,    87,    -1,    88,    -1,   328,    -1,    85,
      -1,    86,    -1
};

/* YYRLINE[YYN] -- source line where rule number YYN was defined.  */
static const yytype_uint16 yyrline[] =
{
       0,   575,   575,   577,   581,   580,   620,   619,   659,   670,
     672,   675,   677,   679,   682,   684,   687,   689,   691,   693,
     696,   699,   702,   707,   706,   719,   721,   724,   726,   729,
     731,   734,   736,   739,   741,   743,   745,   747,   749,   751,
     753,   755,   757,   760,   762,   764,   766,   768,   770,   772,
     775,   777,   779,   781,   784,   795,   806,   814,   822,   830,
     841,   852,   857,   862,   868,   892,   898,   897,   911,   913,
     917,   919,   921,   928,   927,   952,   954,   958,   957,   973,
     975,   982,   981,   993,   995,  1001,  1003,  1006,  1008,  1010,
    1012,  1014,  1022,  1021,  1031,  1033,  1039,  1041,  1044,  1046,
    1048,  1050,  1053,  1052,  1072,  1074,  1078,  1080,  1082,  1084,
    1086,  1088,  1092,  1091,  1106,  1108,  1114,  1116,  1118,  1120,
    1122,  1124,  1126,  1130,  1133,  1136,  1138,  1142,  1141,  1153,
    1160,  1162,  1165,  1167,  1171,  1170,  1184,  1183,  1194,  1196,
    1200,  1199,  1217,  1219,  1225,  1227,  1230,  1232,  1234,  1236,
    1239,  1245,  1247,  1253,  1255,  1258,  1260,  1262,  1264,  1267,
    1274,  1281,  1283,  1286,  1290,  1297,  1308,  1310,  1313,  1317,
    1325,  1327,  1331,  1330,  1346,  1348,  1354,  1356,  1359,  1361,
    1363,  1365,  1368,  1393,  1395,  1398,  1400,  1405,  1407,  1413,
    1415,  1417,  1419,  1421,  1423,  1425,  1427,  1431,  1430,  1442,
    1444,  1450,  1452,  1456,  1458,  1460,  1462,  1464,  1466,  1468,
    1470,  1472,  1474,  1476,  1479,  1484,  1491,  1490,  1504,  1506,
    1512,  1514,  1517,  1519,  1521,  1525,  1524,  1542,  1544,  1548,
    1550,  1552,  1554,  1556,  1558,  1560,  1564,  1563,  1581,  1583,
    1587,  1589,  1591,  1593,  1595,  1597,  1599,  1601,  1603,  1607,
    1620,  1619,  1631,  1630,  1644,  1643,  1654,  1656,  1662,  1664,
    1667,  1669,  1671,  1673,  1675,  1677,  1679,  1681,  1685,  1684,
    1696,  1702,  1704,  1707,  1709,  1711,  1713,  1716,  1723,  1730,
    1738,  1744,  1752,  1751,  1768,  1770,  1773,  1775,  1777,  1779,
    1781,  1783,  1785,  1787,  1791,  1790,  1807,  1809,  1812,  1814,
    1816,  1818,  1820,  1822,  1824,  1826,  1828,  1832,  1831,  1845,
    1847,  1850,  1852,  1854,  1856,  1858,  1860,  1862,  1864,  1866,
    1868,  1870,  1872,  1875,  1885,  1895,  1906,  1914,  1925,  1924,
    1935,  1937,  1943,  1945,  1948,  1950,  1952,  1954,  1956,  1958,
    1962,  1961,  1975,  1977,  1979,  1981,  1983,  1986,  1988,  1994,
    1996,  1999,  2001,  2003,  2005,  2007,  2009,  2013,  2015,  2017,
    2019,  2021,  2023,  2028,  2027,  2041,  2043,  2046,  2048,  2050,
    2052,  2054,  2056,  2060,  2059,  2072,  2079,  2081,  2084,  2086,
    2088,  2090,  2092,  2094,  2096,  2101,  2100,  2114,  2116,  2122,
    2124,  2126,  2128,  2130,  2132,  2134,  2136,  2138,  2140,  2142,
    2144,  2149,  2148,  2184,  2186,  2189,  2191,  2193,  2195,  2197,
    2199,  2201,  2205,  2213,  2212,  2223,  2225,  2231,  2233,  2236,
    2238,  2240,  2242,  2244,  2246,  2248,  2250,  2254,  2253,  2279,
    2281,  2284,  2286,  2288,  2292,  2291,  2319,  2318,  2329,  2331,
    2338,  2340,  2343,  2345,  2347,  2349,  2351,  2353,  2355,  2359,
    2358,  2372,  2374,  2376,  2378,  2380,  2382,  2384,  2386,  2389,
    2395,  2403,  2402,  2413,  2415,  2421,  2423,  2425,  2427,  2429,
    2431,  2433,  2435,  2437,  2439,  2443,  2442,  2453,  2455,  2461,
    2463,  2465,  2467,  2469,  2471,  2474,  2473,  2484,  2486,  2492,
    2494,  2496,  2498,  2500,  2502,  2504,  2506,  2508,  2513,  2512,
    2523,  2525,  2531,  2533,  2536,  2538,  2540,  2542,  2544,  2548,
    2547,  2558,  2560,  2563,  2565,  2568,  2570,  2572,  2574,  2578,
    2577,  2589,  2591,  2594,  2596,  2601,  2607,  2614,  2621,  2624,
    2626,  2628,  2630,  2632,  2634,  2636,  2638,  2640,  2643,  2644,
    2644,  2644,  2644,  2644,  2644,  2644,  2644,  2645,  2645,  2645,
    2645,  2645,  2645,  2645,  2645,  2646,  2646,  2646,  2646,  2646,
    2646,  2646,  2646,  2647,  2647,  2647,  2647,  2647,  2647,  2647,
    2647,  2648,  2648,  2648,  2648,  2648,  2648,  2648,  2648,  2648,
    2649,  2649,  2649,  2649,  2649,  2649,  2649,  2649,  2649,  2650,
    2650,  2650,  2650,  2650,  2650,  2650,  2650,  2651,  2651,  2651,
    2651,  2651,  2651,  2651,  2651,  2652,  2652,  2652,  2652,  2652,
    2652,  2655,  2660,  2666,  2668,  2670,  2672,  2674
};
#endif

#if YYDEBUG || YYERROR_VERBOSE || 0
/* YYTNAME[SYMBOL-NUM] -- String name of the symbol SYMBOL-NUM.
   First, the terminals, then, starting at YYNTOKENS, nonterminals.  */
static const char *const yytname[] =
{
  "$end", "error", "$undefined", "augmentKeyword", "belongs_toKeyword",
  "choiceKeyword", "configKeyword", "contactKeyword", "containerKeyword",
  "defaultKeyword", "descriptionKeyword", "fractionDigitsKeyword",
  "enumKeyword", "error_app_tagKeyword", "error_messageKeyword",
  "extensionKeyword", "groupingKeyword", "importKeyword", "includeKeyword",
  "keyKeyword", "leafKeyword", "leaf_listKeyword", "lengthKeyword",
  "listKeyword", "mandatoryKeyword", "max_elementsKeyword",
  "min_elementsKeyword", "moduleKeyword", "submoduleKeyword",
  "mustKeyword", "namespaceKeyword", "ordered_byKeyword",
  "organizationKeyword", "prefixKeyword", "rangeKeyword",
  "referenceKeyword", "patternKeyword", "revisionKeyword", "statusKeyword",
  "typeKeyword", "typedefKeyword", "uniqueKeyword", "unitsKeyword",
  "usesKeyword", "valueKeyword", "whenKeyword", "bitKeyword",
  "pathKeyword", "anyXMLKeyword", "deprecatedKeyword", "currentKeyword",
  "obsoleteKeyword", "trueKeyword", "falseKeyword", "caseKeyword",
  "inputKeyword", "outputKeyword", "rpcKeyword", "notificationKeyword",
  "argumentKeyword", "yangversionKeyword", "baseKeyword",
  "deviationKeyword", "deviateKeyword", "featureKeyword",
  "identityKeyword", "ifFeatureKeyword", "positionKeyword",
  "presenceKeyword", "refineKeyword", "requireInstanceKeyword",
  "yinElementKeyword", "notSupportedKeyword", "addKeyword",
  "deleteKeyword", "replaceKeyword", "complexTypeKeyword",
  "extendsKeyword", "abstractKeyword", "instanceKeyword",
  "instanceListKeyword", "instanceTypeKeyword", "identifier",
  "identifierRefArg", "identifierRefArgStr", "dateString", "yangVersion",
  "qString", "uqString", "decimalNumber", "floatNumber", "hexNumber",
  "'{'", "'}'", "';'", "$accept", "yangFile", "moduleStatement", "$@1",
  "submoduleStatement", "$@2", "moduleHeaderStatement_0n",
  "moduleHeaderStatement0_n", "moduleHeaderStatement",
  "moduleMetaStatement_0n", "moduleMetaStatement",
  "submoduleHeaderStatement", "belongsToStatement", "$@3",
  "linkageStatement_0n", "linkageStatement", "revisionStatement_0n",
  "bodyStatement_0n", "bodyStatement", "dataDefStatement",
  "commonStatement", "organizationStatement", "contactStatement",
  "descriptionStatement", "referenceStatement", "statusStatement",
  "namespaceStatement", "yangVersionStatement", "status",
  "prefixStatement", "revisionStatement", "$@4", "revisionSubstatement_On",
  "revisionSubstatement", "date", "importStatement", "$@5",
  "optionalRevision", "includeStatement", "$@6", "includeStatementBody",
  "featureStatement", "$@7", "featureSpec", "featureSubstatement_0n",
  "featureSubstatement", "ifFeatureStatement", "identityStatement", "$@8",
  "identitySpec", "identitySubstatement_0n", "identitySubstatement",
  "typedefStatement", "$@9", "typedefSubstatement_0n",
  "typedefSubstatement", "typeStatement", "$@10",
  "optionalTypeBodyStatements", "typeBodyStmts", "numRestriction", "range",
  "$@11", "fractionDigitsStatement", "stringRestriction_0n",
  "stringRestriction", "length", "$@12", "pattern", "$@13", "enumSpec",
  "enum", "$@14", "enumSubstatementSpec", "enumSubstatement_0n",
  "enumSubstatement", "valueStatement", "optionalRestrictionSpec",
  "restrictionSpec_0n", "restrictionSpec", "errorMessageStatement",
  "errorAppTagStatement", "leafrefSpec", "path",
  "requireInstanceStatement", "identityrefSpec", "baseStatement",
  "bitsSpec", "bitsStatement", "$@15", "bitsSubstatementSpec",
  "bitsSubstatement_0n", "bitsSubstatement", "positionStatement",
  "unionSpec", "stmtEnd", "stmtSep", "unknownStatement0_n",
  "complexTypeStatement", "$@16", "complexTypeSpec",
  "complexTypeSubstatement_0n", "complexTypeSubstatement",
  "abstractStatement", "extendsStatement", "$@17", "extendsStatementSpec",
  "extendsSubstatement_0n", "extendsSubstatement", "instanceStatement",
  "$@18", "instanceSubstatement_0n", "instanceSubstatement",
  "instanceListStatement", "$@19", "instanceListSubstatement_0n",
  "instanceListSubstatement", "instanceTypeStatement", "unknownStatement",
  "$@20", "$@21", "containerStatement", "$@22", "containerSpec",
  "containerSubstatement_0n", "containerSubstatement", "mustStatement",
  "$@23", "mustSubstatement_0n", "mustSubstatement", "presenceStatement",
  "configStatement", "mandatoryStatement", "leafStatement", "$@24",
  "leafSubstatement_0n", "leafSubstatement", "leaf_listStatement", "$@25",
  "leaf_listSubstatement_0n", "leaf_listSubstatement", "listStatement",
  "$@26", "listSubstatement_0n", "listSubstatement",
  "max_elementsStatement", "min_elementsStatement", "ordered_byStatement",
  "keyStatement", "uniqueStatement", "choiceStatement", "$@27",
  "choiceSpec", "choiceSubstatement_0n", "choiceSubstatement",
  "caseStatement", "$@28", "caseSpec", "caseSubstatement_0n",
  "caseSubstatement", "caseDataDef", "groupingStatement", "$@29",
  "groupingSubstatement_0n", "groupingSubstatement", "usesStatement",
  "$@30", "usesSubstatement_0n", "usesSubstatement", "refineStatement",
  "$@31", "refineSpec", "refineSubstatement", "refine_0n", "refine",
  "augmentStatement", "$@32", "augmentSubstatement_0n",
  "augmentSubstatement", "whenStatement", "rpcStatement", "$@33",
  "rpcSpec", "rpcSubstatement_0n", "rpcSubstatement", "inputStatement",
  "$@34", "inputOutputSubstatement_0n", "inputOutputSubstatement",
  "outputStatement", "$@35", "notificationStatement", "$@36",
  "notificationSpec", "notificationSubstatement_0n",
  "notificationSubstatement", "deviationStatement", "$@37",
  "deviationSubstatement_0n", "deviationSubstatement",
  "deviateNotSupported", "deviateAddStatement", "$@38", "deviateAddSpec",
  "deviateAddSubstatement_0n", "deviateAddSubstatement",
  "deviateDeleteStatement", "$@39", "deviateDeleteSpec",
  "deviateDeleteSubstatement_0n", "deviateDeleteSubstatement",
  "deviateReplaceStatement", "$@40", "deviateReplaceSpec",
  "deviateReplaceSubstatement_0n", "deviateReplaceSubstatement",
  "anyXMLStatement", "$@41", "anyXMLSpec", "anyXMLSubstatement_0n",
  "anyXMLSubstatement", "extensionStatement", "$@42",
  "extensionStatementBody", "extensionSubstatement_0n",
  "extensionSubstatement", "argumentStatement", "$@43",
  "argumentStatementBody", "yinElementOptional", "unitsStatement",
  "defaultStatement", "prefix", "identifierRef", "identifierStr",
  "booleanValue", "string", YY_NULL
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
     325,   326,   327,   328,   329,   330,   331,   332,   333,   334,
     335,   336,   337,   338,   339,   340,   341,   342,   343,   344,
     345,   346,   123,   125,    59
};
# endif

/* YYR1[YYN] -- Symbol number of symbol that rule YYN derives.  */
static const yytype_uint16 yyr1[] =
{
       0,    95,    96,    96,    98,    97,   100,    99,   101,   102,
     102,   103,   103,   103,   104,   104,   105,   105,   105,   105,
     106,   106,   106,   108,   107,   109,   109,   110,   110,   111,
     111,   112,   112,   113,   113,   113,   113,   113,   113,   113,
     113,   113,   113,   114,   114,   114,   114,   114,   114,   114,
     115,   115,   115,   115,   116,   117,   118,   119,   120,   121,
     122,   123,   123,   123,   124,   125,   126,   125,   127,   127,
     128,   128,   129,   131,   130,   132,   132,   134,   133,   135,
     135,   137,   136,   138,   138,   139,   139,   140,   140,   140,
     140,   141,   143,   142,   144,   144,   145,   145,   146,   146,
     146,   146,   148,   147,   149,   149,   150,   150,   150,   150,
     150,   150,   152,   151,   153,   153,   154,   154,   154,   154,
     154,   154,   154,   155,   155,   155,   155,   157,   156,   158,
     159,   159,   160,   160,   162,   161,   164,   163,   165,   165,
     167,   166,   168,   168,   169,   169,   170,   170,   170,   170,
     171,   172,   172,   173,   173,   174,   174,   174,   174,   175,
     176,   177,   177,   177,   178,   179,   180,   180,   180,   181,
     182,   182,   184,   183,   185,   185,   186,   186,   187,   187,
     187,   187,   188,   189,   189,   190,   190,   191,   191,   192,
     192,   192,   192,   192,   192,   192,   192,   194,   193,   195,
     195,   196,   196,   197,   197,   197,   197,   197,   197,   197,
     197,   197,   197,   197,   198,   198,   200,   199,   201,   201,
     202,   202,   203,   203,   203,   205,   204,   206,   206,   207,
     207,   207,   207,   207,   207,   207,   209,   208,   210,   210,
     211,   211,   211,   211,   211,   211,   211,   211,   211,   212,
     214,   213,   215,   213,   217,   216,   218,   218,   219,   219,
     220,   220,   220,   220,   220,   220,   220,   220,   222,   221,
     221,   223,   223,   224,   224,   224,   224,   225,   226,   226,
     227,   227,   229,   228,   230,   230,   231,   231,   231,   231,
     231,   231,   231,   231,   233,   232,   234,   234,   235,   235,
     235,   235,   235,   235,   235,   235,   235,   237,   236,   238,
     238,   239,   239,   239,   239,   239,   239,   239,   239,   239,
     239,   239,   239,   240,   241,   242,   243,   244,   246,   245,
     247,   247,   248,   248,   249,   249,   249,   249,   249,   249,
     251,   250,   250,   250,   250,   250,   250,   252,   252,   253,
     253,   254,   254,   254,   254,   254,   254,   255,   255,   255,
     255,   255,   255,   257,   256,   258,   258,   259,   259,   259,
     259,   259,   259,   261,   260,   260,   262,   262,   263,   263,
     263,   263,   263,   263,   263,   265,   264,   266,   266,   267,
     268,   268,   269,   269,   269,   269,   269,   269,   269,   269,
     269,   271,   270,   272,   272,   273,   273,   273,   273,   273,
     273,   273,   274,   276,   275,   277,   277,   278,   278,   279,
     279,   279,   279,   279,   279,   279,   279,   281,   280,   282,
     282,   283,   283,   283,   285,   284,   287,   286,   288,   288,
     289,   289,   290,   290,   290,   290,   290,   290,   290,   292,
     291,   293,   293,   294,   294,   294,   294,   294,   294,   295,
     295,   297,   296,   298,   298,   299,   299,   300,   300,   300,
     300,   300,   300,   300,   300,   302,   301,   303,   303,   304,
     304,   305,   305,   305,   305,   307,   306,   308,   308,   309,
     309,   310,   310,   310,   310,   310,   310,   310,   312,   311,
     313,   313,   314,   314,   315,   315,   315,   315,   315,   317,
     316,   318,   318,   319,   319,   320,   320,   320,   320,   322,
     321,   323,   323,   324,   324,   324,   325,   326,   327,   328,
     328,   328,   328,   328,   328,   328,   328,   328,   329,   329,
     329,   329,   329,   329,   329,   329,   329,   329,   329,   329,
     329,   329,   329,   329,   329,   329,   329,   329,   329,   329,
     329,   329,   329,   329,   329,   329,   329,   329,   329,   329,
     329,   329,   329,   329,   329,   329,   329,   329,   329,   329,
     329,   329,   329,   329,   329,   329,   329,   329,   329,   329,
     329,   329,   329,   329,   329,   329,   329,   329,   329,   329,
     329,   329,   329,   329,   329,   329,   329,   329,   329,   329,
     329,   330,   330,   331,   331,   331,   331,   331
};

/* YYR2[YYN] -- Number of symbols composing right hand side of rule YYN.  */
static const yytype_uint8 yyr2[] =
{
       0,     2,     1,     1,     0,    11,     0,    11,     1,     0,
       2,     2,     2,     2,     0,     3,     1,     1,     1,     1,
       4,     4,     2,     0,     8,     0,     2,     2,     2,     0,
       3,     0,     3,     1,     1,     1,     1,     1,     1,     1,
       1,     1,     1,     1,     1,     1,     1,     1,     1,     1,
       1,     1,     1,     1,     3,     3,     3,     3,     3,     3,
       3,     1,     1,     1,     3,     3,     0,     7,     0,     3,
       1,     1,     1,     0,     9,     0,     2,     0,     4,     1,
       4,     0,     4,     1,     4,     0,     3,     1,     1,     1,
       1,     3,     0,     4,     1,     4,     0,     3,     1,     1,
       1,     1,     0,     7,     0,     3,     1,     1,     1,     1,
       1,     1,     0,     4,     1,     4,     1,     1,     1,     1,
       1,     1,     1,     3,     3,     1,     2,     0,     4,     3,
       0,     2,     1,     1,     0,     4,     0,     4,     2,     3,
       0,     4,     1,     4,     0,     3,     1,     1,     1,     1,
       3,     1,     4,     0,     3,     1,     1,     1,     1,     3,
       3,     2,     4,     4,     3,     3,     2,     4,     4,     3,
       2,     3,     0,     4,     1,     4,     0,     3,     1,     1,
       1,     1,     3,     2,     3,     1,     3,     0,     1,     0,
       2,     2,     2,     2,     2,     2,     2,     0,     4,     1,
       4,     0,     3,     1,     1,     1,     1,     1,     1,     1,
       1,     1,     1,     1,     3,     3,     0,     4,     1,     4,
       0,     3,     1,     1,     1,     0,     7,     0,     3,     1,
       1,     1,     1,     1,     1,     1,     0,     7,     0,     3,
       1,     1,     1,     1,     1,     1,     1,     1,     1,     3,
       0,     3,     0,     4,     0,     4,     1,     4,     0,     3,
       1,     1,     1,     1,     1,     1,     1,     1,     0,     6,
       3,     0,     3,     1,     1,     1,     1,     3,     3,     3,
       3,     3,     0,     7,     0,     3,     1,     1,     1,     1,
       1,     1,     1,     1,     0,     7,     0,     3,     1,     1,
       1,     1,     1,     1,     1,     1,     1,     0,     7,     0,
       3,     1,     1,     1,     1,     1,     1,     1,     1,     1,
       1,     1,     1,     3,     3,     3,     3,     3,     0,     4,
       1,     4,     0,     3,     1,     1,     1,     1,     1,     1,
       0,     4,     1,     1,     1,     1,     1,     1,     4,     0,
       3,     1,     1,     1,     1,     1,     1,     1,     1,     1,
       1,     1,     1,     0,     7,     0,     3,     1,     1,     1,
       1,     1,     1,     0,     6,     3,     0,     3,     1,     1,
       1,     1,     1,     1,     1,     0,     4,     1,     4,     1,
       0,     3,     1,     1,     1,     1,     1,     1,     1,     1,
       1,     0,     7,     1,     3,     1,     1,     1,     1,     1,
       1,     1,     3,     0,     4,     1,     4,     0,     3,     1,
       1,     1,     1,     1,     1,     1,     1,     0,     6,     2,
       3,     1,     1,     1,     0,     6,     0,     4,     1,     4,
       0,     3,     1,     1,     1,     1,     1,     1,     1,     0,
       7,     2,     3,     1,     1,     1,     1,     1,     1,     3,
       5,     0,     4,     1,     4,     0,     3,     1,     1,     1,
       1,     1,     1,     1,     1,     0,     4,     1,     4,     0,
       3,     1,     1,     1,     1,     0,     4,     1,     4,     0,
       3,     1,     1,     1,     1,     1,     1,     1,     0,     4,
       1,     4,     0,     3,     1,     1,     1,     1,     1,     0,
       4,     4,     1,     0,     3,     1,     1,     1,     1,     0,
       4,     4,     1,     0,     4,     4,     3,     3,     1,     1,
       1,     1,     1,     1,     1,     1,     1,     1,     1,     1,
       1,     1,     1,     1,     1,     1,     1,     1,     1,     1,
       1,     1,     1,     1,     1,     1,     1,     1,     1,     1,
       1,     1,     1,     1,     1,     1,     1,     1,     1,     1,
       1,     1,     1,     1,     1,     1,     1,     1,     1,     1,
       1,     1,     1,     1,     1,     1,     1,     1,     1,     1,
       1,     1,     1,     1,     1,     1,     1,     1,     1,     1,
       1,     1,     1,     1,     1,     1,     1,     1,     1,     1,
       1,     1,     1,     1,     1,     1,     1,     1
};

/* YYDEFACT[STATE-NAME] -- Default reduction number in state STATE-NUM.
   Performed when YYTABLE doesn't specify something else to do.  Zero
   means the default is an error.  */
static const yytype_uint16 yydefact[] =
{
       0,     0,     0,     0,     2,     3,   539,   540,   541,   542,
     543,   544,   545,   546,   547,   548,   549,   550,   551,   552,
     553,   554,   555,   556,   557,   558,   559,   560,   561,   562,
     563,   564,   565,   566,   567,   568,   569,   570,   571,   572,
     573,   574,   575,   576,   577,   578,   579,   580,   581,   582,
     583,   584,   585,   586,   587,   588,   589,   590,   591,   592,
     593,   594,   595,   596,   597,   598,   599,   600,   601,   602,
     603,   604,   605,   606,   607,   608,   609,   610,   538,     4,
       6,     1,     0,     0,   187,   187,     0,     0,     0,     0,
       0,     0,   250,     9,   188,   189,   189,   189,   189,   189,
     189,   189,     0,   197,   532,   533,   534,   535,   536,   537,
     529,   530,   216,   531,     0,     0,   225,   236,     0,   616,
     617,   613,   614,     0,   615,   252,     0,     0,     0,    25,
       8,     9,   187,   187,   187,   191,   195,   196,   192,   193,
     194,   190,     0,    25,   187,   187,     0,     0,   189,   185,
     214,   215,     0,     0,   249,   251,     0,     0,     0,   528,
       0,    14,    10,    12,    11,    13,    23,    14,    22,     0,
     187,   199,   198,   187,   218,   217,     0,   187,   187,   253,
      59,    64,    60,     0,     0,    29,    26,   187,   187,     0,
      29,   187,   187,   201,   220,   186,   227,   238,    73,    77,
       0,     0,     0,     0,   187,    31,    16,    17,    18,    19,
      28,    27,   187,    31,    20,    21,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,   205,   211,   213,   212,   204,   207,     0,   187,    43,
     206,    44,    45,    46,   209,   208,    47,   203,    49,   210,
      48,   222,   224,   223,     0,   187,     0,     0,     0,     0,
     231,   230,    50,    52,    51,   229,     0,   187,   232,    53,
     234,   235,   233,     0,     0,   242,   241,   240,     0,   187,
     243,   246,   247,   248,   245,   244,     0,     0,     0,     0,
       0,     0,    15,     0,     0,   187,     0,     0,   328,   254,
     363,     0,   282,   294,   307,   268,     0,    61,    62,    63,
       0,   102,   373,   498,     0,   385,   200,   201,   219,   220,
     401,     0,     0,     0,     0,     0,   226,   227,     0,     0,
     237,   238,   187,   187,    79,    78,    55,    56,    54,    57,
      72,    66,     0,     0,     0,     0,     0,     0,     5,   187,
      38,    34,    35,    36,    37,    39,    40,    41,    42,    33,
      30,   187,     7,     0,     0,     0,   326,     0,     0,     0,
     270,     0,   325,    58,     0,   375,     0,     0,    91,     0,
     202,   221,     0,   278,   279,   280,   281,   412,   228,   323,
     324,   239,     0,    75,    65,     0,   509,   413,   436,   449,
      81,    92,    32,     0,   187,   330,   329,   187,   256,   255,
     187,   187,   187,   187,   271,   187,   376,   187,   500,   499,
     187,   387,   386,   187,   187,   187,     0,   187,     0,     0,
       0,     0,     0,     0,    24,   332,   258,   365,   284,   296,
     309,     0,   104,     0,   502,   390,   187,    75,    76,    80,
      68,   187,   512,   510,   187,   415,   414,   187,   438,   437,
     187,   187,    83,    82,   187,    94,    93,     0,     0,   262,
     261,   260,   267,     0,   187,   264,   266,   263,   265,     0,
       0,     0,     0,     0,     0,   269,   275,   276,   273,   274,
     187,     0,   374,   378,   379,   380,   382,   187,   383,   384,
     381,     0,     0,   389,   403,     0,     0,     0,   513,   417,
     440,     0,    85,    96,     0,     0,   331,   334,   337,   342,
     338,   343,   344,   345,   187,   339,   336,   346,   335,     0,
     257,   258,   364,   370,   368,   369,   367,   372,   371,   187,
       0,     0,   283,   289,   286,   291,   288,   290,   187,   293,
     287,   292,   295,   299,   301,   300,   298,   187,   304,   305,
     306,   303,   302,     0,   308,   318,   312,   322,   319,   311,
     187,   313,   314,   315,   316,   317,   320,   321,     0,     0,
     272,   103,   111,   110,   109,   187,   106,   107,   108,   377,
     501,   504,   506,   507,   508,   505,   187,   388,   395,   396,
     392,   393,   394,   398,   400,   399,   187,   397,   402,   410,
     407,   408,   409,   406,    43,    44,    45,    46,   411,   187,
     405,    48,    74,    67,    70,    71,   187,     0,     0,     0,
       0,   453,   454,     0,   187,   455,   456,   457,   458,     0,
       0,     0,   340,   333,   277,   259,   366,   112,     0,   285,
     297,     0,   310,   160,   159,   105,   503,   391,   404,    69,
       0,   511,   517,   518,   516,   187,   515,   427,   434,   416,
     420,   421,   422,   419,   423,   424,   187,   425,   426,   439,
     446,   443,   444,   445,   442,   448,   447,   187,     0,   461,
     475,   485,   450,   187,   451,    84,    89,    90,    88,   187,
      87,     0,    95,   100,   101,    99,   187,    98,   527,     0,
       0,   526,   327,   519,   514,     0,     0,   418,   441,   187,
     459,     0,     0,     0,   452,    86,     0,    97,   187,   347,
     341,   187,   114,   113,     0,   187,   187,     0,   187,   463,
     462,   187,   477,   476,   187,   487,   486,   169,   349,   130,
     187,   522,   520,     0,     0,   460,   465,   479,   489,     0,
       0,     0,     0,     0,     0,     0,   187,     0,   116,   187,
     125,   117,   118,   187,   119,   187,   187,   120,   187,   121,
     187,   122,   523,   431,   433,   432,     0,   187,     0,     0,
       0,     0,   348,   351,   353,   352,   355,   357,   358,   359,
     360,   187,   356,   362,   354,   361,     0,   140,   127,   172,
       0,   611,   612,     0,   183,   115,   126,   187,     0,     0,
     131,   132,   133,   187,   138,   161,     0,   166,   187,   170,
     187,     0,     0,   428,   187,   429,   435,   464,   468,   471,
     472,   474,   473,   469,   187,   467,   470,   478,   482,   483,
     187,   481,   484,   488,   491,   494,   495,   497,   496,   187,
     492,   493,   350,   129,     0,     0,     0,   164,   165,   123,
     124,   134,   136,   139,   187,   187,   187,   187,   171,   184,
       0,     0,   521,   430,   466,   480,   490,   187,   142,   141,
     187,   151,   128,   187,   174,   173,     0,     0,   162,   163,
     168,   167,   187,   187,   144,   153,   176,   135,   137,   524,
     525,     0,     0,     0,     0,   143,   148,   149,   147,   187,
     146,   152,   155,   156,   187,   157,   158,     0,   175,   179,
     180,   181,   187,   178,     0,   145,   154,     0,   177,   150,
     182
};

/* YYDEFGOTO[NTERM-NUM].  */
static const yytype_int16 yydefgoto[] =
{
      -1,     3,     4,    82,     5,    83,   129,   130,   131,   185,
     204,   143,   144,   189,   161,   186,   205,   294,   349,   783,
     261,   206,   207,   262,   263,   264,   132,   133,   310,   134,
     295,   395,   507,   626,   341,   187,   286,   426,   188,   287,
     335,   351,   432,   463,   639,   699,   235,   352,   433,   466,
     640,   706,   784,   374,   491,   585,   545,   710,   733,   767,
     768,   769,   865,   770,   771,   820,   821,   896,   822,   897,
     772,   773,   864,   889,   911,   919,   920,   892,   912,   924,
     488,   489,   774,   775,   776,   777,   707,   779,   780,   866,
     895,   913,   932,   933,   781,   150,    93,    94,    95,   146,
     172,   237,   238,    96,    97,   147,   175,   254,   255,    98,
     152,   266,   267,    99,   153,   278,   279,   100,   101,   123,
     156,   239,   364,   409,   473,   474,   240,   371,   441,   490,
     476,   269,   270,   241,   367,   480,   548,   242,   368,   481,
     557,   243,   369,   482,   570,   281,   282,   244,   245,   575,
     246,   363,   406,   467,   524,   525,   709,   730,   759,   801,
     802,   785,   365,   479,   539,   248,   376,   443,   497,   249,
     379,   422,   502,   503,   606,   271,   382,   505,   619,   272,
     356,   429,   456,   628,   676,   677,   715,   786,   787,   678,
     716,   357,   430,   459,   629,   687,   358,   431,   633,   634,
     635,   636,   721,   740,   789,   844,   637,   722,   743,   790,
     850,   638,   723,   746,   791,   859,   250,   377,   419,   501,
     596,   359,   428,   453,   627,   665,   666,   734,   752,   832,
     550,   528,   158,   124,   113,   813,   125
};

/* YYPACT[STATE-NUM] -- Index in YYTABLE of the portion describing
   STATE-NUM.  */
#define YYPACT_NINF -729
static const yytype_int16 yypact[] =
{
       3,  1350,  1350,    18,  -729,  -729,  -729,  -729,  -729,  -729,
    -729,  -729,  -729,  -729,  -729,  -729,  -729,  -729,  -729,  -729,
    -729,  -729,  -729,  -729,  -729,  -729,  -729,  -729,  -729,  -729,
    -729,  -729,  -729,  -729,  -729,  -729,  -729,  -729,  -729,  -729,
    -729,  -729,  -729,  -729,  -729,  -729,  -729,  -729,  -729,  -729,
    -729,  -729,  -729,  -729,  -729,  -729,  -729,  -729,  -729,  -729,
    -729,  -729,  -729,  -729,  -729,  -729,  -729,  -729,  -729,  -729,
    -729,  -729,  -729,  -729,  -729,  -729,  -729,  -729,  -729,  -729,
    -729,  -729,   -67,   -50,   490,   490,  1350,  1268,    97,  1350,
    1350,  1268,  1182,     8,  -729,   490,   490,   490,   490,   490,
     490,   490,    19,  -729,  -729,  -729,  -729,  -729,  -729,  -729,
    -729,  -729,  -729,  -729,   -10,   -10,  -729,  -729,   -10,  -729,
    -729,  -729,  -729,   -10,  -729,  -729,  1182,  1350,   -25,  -729,
    -729,     8,   490,   490,   490,  -729,  -729,  -729,  -729,  -729,
    -729,  -729,  1350,  -729,   490,   490,    64,    93,   490,  -729,
    -729,  -729,   -11,    36,  -729,  -729,   -10,   -10,   -10,  -729,
     -10,   163,  -729,  -729,  -729,  -729,  -729,   163,    72,   134,
     490,  -729,  -729,   490,  -729,  -729,    66,   490,   490,  -729,
    -729,  -729,  -729,  1350,  1350,   393,  -729,   490,   490,    70,
     393,   490,   490,  1421,    59,  -729,  1527,  1457,  -729,  -729,
    1182,  1182,  1182,  1182,   490,   128,  -729,  -729,  -729,  -729,
    -729,  -729,   490,   128,  -729,  -729,  1350,  1350,  1350,  1182,
    1350,  1350,  1350,  1182,  1182,   158,  1350,  1268,  1350,  1268,
    1182,  -729,  -729,  -729,  -729,  -729,  -729,    75,   490,  -729,
    -729,  -729,  -729,  -729,  -729,  -729,  -729,  -729,  -729,  -729,
    -729,  -729,  -729,  -729,    98,   490,  1182,   123,   170,  1182,
    -729,  -729,  -729,  -729,  -729,  -729,   121,   490,  -729,  -729,
    -729,  -729,  -729,  1182,  1182,  -729,  -729,  -729,   125,   490,
    -729,  -729,  -729,  -729,  -729,  -729,   135,   103,   -10,   -10,
     -10,   -10,  -729,   147,   806,   490,   207,   872,  -729,  -729,
    -729,   -10,  -729,  -729,  -729,   150,   -10,  -729,  -729,  -729,
     -10,  -729,   180,  -729,   -10,  -729,  -729,  1421,  -729,    59,
    -729,   -10,   -10,   -10,   -10,   -10,  -729,  1527,   -10,   -10,
    -729,  1457,   490,   490,  -729,  -729,  -729,  -729,  -729,  -729,
    -729,   183,  1350,  1350,  1350,  1182,  1350,  1350,  -729,   490,
    -729,  -729,  -729,  -729,  -729,  -729,  -729,  -729,  -729,  -729,
    -729,   490,  -729,   106,   127,   191,  -729,   201,   214,   237,
    -729,   242,  -729,  -729,   259,  -729,   266,   167,  -729,   176,
    -729,  -729,   285,  -729,  -729,  -729,  -729,  -729,  -729,  -729,
    -729,  -729,   207,   128,  -729,   290,  -729,  -729,  -729,  -729,
    -729,  -729,  -729,   293,   490,  -729,  -729,   490,  -729,  -729,
     490,   490,   490,   490,  -729,   490,  -729,   490,  -729,  -729,
     490,  -729,  -729,   490,   490,   490,   338,   490,   190,   205,
     270,   356,   289,   298,  -729,  -729,  1491,  -729,  -729,  -729,
    -729,   253,  -729,    86,  -729,  -729,   490,   128,  -729,  -729,
    -729,   490,  -729,  -729,   490,  -729,  -729,   490,  -729,  -729,
     490,   490,  -729,  -729,   490,  -729,  -729,  1080,  1182,  -729,
    -729,  -729,  -729,   359,   490,  -729,  -729,  -729,  -729,  1039,
    1100,  1088,   950,  1182,  1182,  -729,  -729,  -729,  -729,  -729,
     490,    48,  -729,  -729,  -729,  -729,  -729,   490,  -729,  -729,
    -729,    85,   360,   504,  -729,   979,   365,   196,  -729,  -729,
    -729,   136,  -729,  -729,  1182,  1350,  -729,  -729,  -729,  -729,
    -729,  -729,  -729,  -729,   490,  -729,  -729,  -729,  -729,   -10,
    -729,  1491,  -729,  -729,  -729,  -729,  -729,  -729,  -729,   490,
    1268,  1182,  -729,  -729,  -729,  -729,  -729,  -729,   490,  -729,
    -729,  -729,  -729,  -729,  -729,  -729,  -729,   490,  -729,  -729,
    -729,  -729,  -729,  1182,  -729,  -729,  -729,  -729,  -729,  -729,
     490,  -729,  -729,  -729,  -729,  -729,  -729,  -729,   -10,   -10,
    -729,  -729,  -729,  -729,  -729,   490,  -729,  -729,  -729,  -729,
    -729,  -729,  -729,  -729,  -729,  -729,   490,  -729,  -729,  -729,
    -729,  -729,  -729,  -729,  -729,  -729,   490,  -729,  -729,  -729,
    -729,  -729,  -729,  -729,  -729,  -729,  -729,  -729,  -729,   490,
    -729,  -729,  -729,  -729,  -729,  -729,   490,   314,   803,  1018,
      43,  -729,  -729,   182,   490,  -729,  -729,  -729,  -729,   333,
     353,   -10,  -729,  -729,  -729,  -729,  -729,  -729,   -10,  -729,
    -729,   -10,  -729,  -729,  -729,  -729,  -729,  -729,  -729,  -729,
    1350,  -729,  -729,  -729,  -729,   490,  -729,  -729,  -729,  -729,
    -729,  -729,  -729,  -729,  -729,  -729,   490,  -729,  -729,  -729,
    -729,  -729,  -729,  -729,  -729,  -729,  -729,   490,   304,  -729,
    -729,  -729,  -729,   490,  -729,  -729,  -729,  -729,  -729,   490,
    -729,  1268,  -729,  -729,  -729,  -729,   490,  -729,  -729,   321,
     335,  -729,  -729,  -729,  -729,   357,   374,  -729,  -729,   490,
    -729,   342,   343,   346,  -729,  -729,   -10,  -729,   490,  -729,
    -729,   490,  -729,  -729,   370,   490,   490,   377,   490,  -729,
    -729,   490,  -729,  -729,   490,  -729,  -729,  -729,  -729,   615,
     490,  -729,  -729,  1132,  1132,  -729,  -729,  -729,  -729,   740,
    1182,  1182,  1182,   385,  1182,   199,   490,   378,  -729,   490,
     456,   -12,   479,   490,  -729,   490,   490,  -729,   490,   448,
     490,   462,   424,  -729,  -729,  -729,   599,   490,   901,   148,
      11,   187,  -729,  -729,  -729,  -729,  -729,  -729,  -729,  -729,
    -729,   490,  -729,  -729,  -729,  -729,   -10,  -729,  -729,  -729,
     -10,  -729,  -729,   -10,  -729,  -729,   493,   490,  1182,  1182,
    -729,  -729,  -729,   490,  -729,   436,     4,   436,   490,  -729,
     490,   292,   414,  -729,   490,  -729,  -729,  -729,  -729,  -729,
    -729,  -729,  -729,  -729,   490,  -729,  -729,  -729,  -729,  -729,
     490,  -729,  -729,  -729,  -729,  -729,  -729,  -729,  -729,   490,
    -729,  -729,  -729,  -729,   390,   394,   411,  -729,  -729,  -729,
    -729,  -729,  -729,  -729,   490,   490,   490,   490,  -729,  -729,
     -10,   -10,  -729,  -729,  -729,  -729,  -729,   490,  -729,  -729,
     490,  -729,  -729,   490,  -729,  -729,   394,   394,  -729,  -729,
    -729,  -729,   490,   490,  -729,  -729,  -729,  -729,  -729,  -729,
    -729,    29,   409,   243,  1182,  -729,  -729,  -729,  -729,   490,
    -729,  -729,  -729,  -729,   490,  -729,  -729,  1182,  -729,  -729,
    -729,  -729,   490,  -729,   -10,  -729,  -729,   -10,  -729,  -729,
    -729
};

/* YYPGOTO[NTERM-NUM].  */
static const yytype_int16 yypgoto[] =
{
    -729,  -729,  -729,  -729,  -729,  -729,  -729,   381,  -729,   348,
    -729,  -729,   347,  -729,   375,  -729,   327,   307,  -729,    53,
     156,  -729,  -729,   -46,    40,  -131,  -729,   -68,  -729,  -291,
    -367,  -729,  -729,  -729,  -729,  -729,  -729,    76,  -729,  -729,
    -729,  -729,  -729,  -729,  -729,  -729,   272,  -729,  -729,  -729,
    -729,  -729,   -32,  -729,  -729,  -729,  -480,  -729,  -729,  -729,
    -729,  -248,  -729,  -292,  -729,  -729,  -729,  -729,  -729,  -729,
    -729,  -247,  -729,  -729,  -729,  -729,  -729,  -495,  -729,  -729,
    -386,  -376,  -729,  -289,  -690,  -729,  -728,  -729,  -241,  -729,
    -729,  -729,  -729,  -729,  -729,   -86,   -85,   379,  -729,  -729,
    -729,   223,  -729,  -729,  -729,  -729,  -729,   225,  -729,  -729,
    -729,   215,  -729,  -729,  -729,   217,  -729,  -729,  -729,  -729,
    -729,  -461,  -729,  -729,    15,  -729,   -71,  -729,  -729,  -729,
      47,  -484,  -458,  -455,  -729,  -729,  -729,  -451,  -729,  -729,
    -729,  -450,  -729,  -729,  -729,  -468,  -404,  -195,    71,  -348,
    -729,  -729,  -729,  -729,  -729,    49,  -729,  -729,  -729,  -729,
    -729,   262,  -729,  -729,  -729,  -207,  -729,  -729,  -729,   114,
    -729,  -729,  -729,  -729,  -729,  -189,  -729,  -729,  -729,  -164,
    -729,  -729,  -729,  -729,  -729,  -729,  -729,  -194,  -380,  -729,
    -729,  -729,  -729,  -729,  -729,  -729,  -729,  -729,  -729,   -72,
    -729,  -729,  -729,  -729,  -729,  -729,  -729,  -729,  -729,  -729,
    -729,  -729,  -729,  -729,  -729,  -729,  -439,  -729,  -729,  -729,
    -729,  -729,  -729,  -729,  -729,  -729,  -729,  -729,  -729,  -729,
    -435,  -476,  -729,   -84,    74,  -729,   -90
};

/* YYTABLE[YYPACT[STATE-NUM]].  What to do in state STATE-NUM.  If
   positive, shift that token.  If negative, reduce the rule which
   number is the opposite.  If YYTABLE_NINF, syntax error.  */
#define YYTABLE_NINF -1
static const yytype_uint16 yytable[] =
{
     102,   555,   283,   112,   551,   361,   519,   118,   284,   520,
     818,   586,   521,   558,   571,   588,   522,   523,    81,   602,
     514,   778,   547,   142,   819,    84,   425,   607,   527,   151,
       1,     2,   154,   285,   145,   604,   157,   155,   126,   201,
     223,   127,    85,   594,   614,   603,   562,   163,   164,   165,
     615,   764,   563,   541,   616,   617,   587,   514,   201,   168,
     169,   160,   234,   253,   203,   701,   621,   225,   128,   201,
     179,   180,   181,   914,   182,    79,    80,   559,   572,   128,
     425,   177,   148,   203,   149,   193,   225,   540,   194,   256,
     541,   257,   196,   197,   203,   201,   201,   225,   876,   605,
     191,   424,   210,   211,   847,   355,   214,   215,   355,   258,
     288,   289,   290,   291,   223,   688,   689,   690,   691,   292,
     203,   203,   915,   225,   225,   268,   280,   296,   178,   301,
     259,   259,   128,   305,   306,   874,   283,   877,   142,   208,
     315,   581,   284,   312,   208,   314,   201,   232,   251,   114,
     115,   229,   229,   317,   257,   230,   170,   514,   171,   195,
     103,   236,   212,   116,   117,   293,   320,   285,   316,   325,
     319,   203,   258,   273,   274,   321,   322,   223,   590,   492,
     183,   184,   327,   328,   329,   173,   234,   174,   253,   563,
     541,   318,   201,   257,   331,   333,   514,   334,   404,   630,
     405,   159,   336,   337,   338,   339,   201,   307,   308,   309,
     360,   258,   273,   274,   326,   366,   166,   203,   330,   407,
     372,   408,   323,   324,   373,   209,   540,   332,   378,   541,
     209,   203,   340,   233,   252,   383,   384,   385,   386,   387,
     127,   837,   389,   390,   370,   630,   231,   392,   393,   260,
     275,   811,   812,   201,   499,   399,   268,   198,   199,   417,
     280,   418,   353,   201,   402,   353,   483,   484,   420,   766,
     421,   232,   478,   251,   375,   692,   403,   394,   203,   500,
     853,   225,   451,   410,   452,   236,   560,   573,   203,   623,
     298,   299,   300,   411,   302,   303,   304,   454,   797,   455,
     311,   830,   313,   526,   798,   839,   412,   855,   799,   800,
     927,   854,   495,   846,   852,   861,   549,   561,   577,   435,
     805,   841,   436,   857,   201,   437,   438,   439,   440,   413,
     442,   840,   444,   856,   414,   445,   928,   595,   446,   447,
     448,   620,   450,   201,   880,   881,   485,   350,   536,   203,
     350,   415,   225,   276,   845,   851,   860,   233,   416,   252,
     584,   504,   457,   201,   458,   475,   508,   478,   203,   509,
     231,   225,   510,   660,   612,   511,   512,   423,   529,   513,
     260,   461,   427,   462,   275,   842,   434,   858,   203,   531,
     464,   225,   465,   578,   579,   486,   719,   493,   720,   229,
     200,   907,   908,   201,   472,   580,   834,   661,   834,   546,
     556,   569,   589,   728,   701,   729,   396,   397,   398,   201,
     400,   401,   483,   484,   641,   202,   695,   731,   203,   732,
     593,   449,   600,   534,   738,   741,   739,   742,   744,   643,
     745,   843,   849,   644,   203,   582,   702,   537,   460,   735,
     568,   648,   530,   597,   646,   247,   647,   598,   622,   610,
     475,   624,   750,   649,   751,   631,   736,   809,   265,   277,
     755,   815,   650,   651,   135,   136,   137,   138,   139,   140,
     141,   487,   887,   494,   888,   652,   890,   276,   891,   469,
     762,   761,   653,   654,   763,   831,   664,   672,   683,   472,
     655,   540,   921,   893,   760,   894,   765,   882,   698,   705,
     257,   656,   162,   514,   201,   190,   192,   213,   167,   535,
     297,   657,   817,   506,   869,   823,   925,   176,   258,   273,
     274,   583,   533,   223,   658,   565,   926,   875,   828,   203,
     380,   659,   388,   599,   381,   611,   645,   625,   391,   694,
     601,   632,   803,   574,   618,   708,   354,   498,   609,   354,
     788,   693,   711,     0,     0,   712,    86,    87,    88,    89,
      90,    91,   468,    92,     0,     0,     0,     0,     0,   247,
     714,   662,   670,   681,   469,     0,     0,   631,     0,   642,
       0,   717,   470,   696,   703,   804,   674,   685,     0,   265,
       0,     0,   718,   277,   216,     0,     0,   217,   724,     0,
       0,     0,     0,     0,   725,   218,     0,   726,     0,   220,
     221,   727,   222,   517,     0,     0,   760,   761,   795,     0,
       0,     0,     0,     0,   737,     0,   543,   553,   566,   226,
     747,     0,   227,   748,     0,     0,   749,   228,     0,   762,
     753,   754,     0,   756,   540,     0,   757,   591,     0,   758,
       0,   763,   764,     0,     0,   782,     0,   663,   671,   682,
     806,   807,   808,   632,   810,     0,   701,     0,     0,   697,
     704,   814,   680,     0,   816,   765,     0,   470,   824,     0,
     825,   826,   833,   827,     0,   829,     0,     0,   477,     0,
       0,     0,   835,     0,     0,     0,     0,     0,   471,     0,
       0,     0,     0,   793,     0,   496,   862,     0,   838,   848,
     863,     0,     0,     0,   867,     0,     0,   868,   871,   872,
       0,     0,   870,     0,   713,     0,     0,     0,   873,   518,
       0,   538,     0,   878,   576,   879,     0,     0,   217,   883,
     201,     0,   544,   554,   567,     0,     0,     0,     0,   884,
     220,   221,     0,   222,     0,   885,     0,     0,     0,     0,
       0,     0,     0,   592,   886,   203,     0,   613,   225,     0,
     918,     0,   931,   227,     0,   259,     0,     0,   228,   898,
     899,   900,   901,   477,   902,   903,     0,     0,     0,   794,
       0,     0,   904,   471,     0,   905,   229,     0,   906,   256,
       0,   216,     0,   201,   217,     0,     0,   909,   910,   218,
       0,   342,   218,     0,   934,     0,   220,   221,     0,   222,
       0,     0,     0,   792,   935,     0,     0,   937,   203,   936,
       0,   225,     0,   226,     0,     0,   226,   938,   939,   227,
       0,   940,     0,     0,   228,     0,     0,     0,   667,   668,
       0,     0,     0,   343,   344,   916,   922,   929,   345,   229,
     346,   347,     0,     0,     0,   256,     0,   216,     0,     0,
     217,     0,     0,     0,     0,     0,     0,   342,   218,     0,
     675,   686,   220,   221,     0,   222,   669,     0,     0,   348,
     673,   684,     0,     0,     0,     0,   216,     0,     0,   217,
       0,   700,   226,     0,     0,   227,     0,   218,     0,     0,
     228,   220,   221,     0,   222,     0,     0,     0,     0,   343,
     344,     0,     0,     0,   345,     0,   346,   347,     0,     0,
       0,   226,     0,     0,   227,     0,     0,     0,     0,   228,
       0,   917,   923,   930,     0,   216,   257,     0,   217,     0,
     201,     0,     0,     0,     0,   362,   218,     0,     0,   219,
     220,   221,     0,   222,     0,   273,   274,     0,     0,   223,
       0,   224,     0,     0,   216,   203,     0,   217,   225,   201,
     226,   563,     0,   227,   836,   259,     0,     0,   228,   220,
     221,     0,   222,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,   203,     0,   229,   225,     0,     0,
       0,     0,   227,   216,   259,     0,   217,   228,   201,     0,
       0,   796,     0,   515,   218,     0,     0,     0,   220,   221,
       0,   222,     0,   564,   216,   229,     0,   217,     0,   201,
       0,     0,     0,   203,     0,   218,   225,     0,   226,   220,
     221,   227,   222,     0,     0,     0,   228,     0,     0,     0,
       0,     0,   608,     0,   203,     0,     0,   225,     0,   226,
       0,     0,   227,     0,   229,     0,   257,   228,   217,   514,
     201,     0,     0,     0,   257,     0,     0,     0,   201,     0,
     220,   221,     0,   222,   258,     0,   257,     0,     0,   514,
     201,   679,     0,   273,   274,   203,     0,   223,   225,   224,
       0,     0,     0,   203,   258,   259,   225,   540,   228,   223,
     541,     0,   532,   259,   515,   203,     0,   216,   225,   540,
     217,     0,   541,     0,     0,   259,   229,     0,   218,     0,
       0,     0,   220,   221,   229,   222,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,   229,     0,     0,     0,
       0,     0,   226,   516,     0,   227,     0,     0,     0,     0,
     228,   552,     0,     0,     0,     6,     7,     8,     9,    10,
      11,    12,    13,   542,    14,    15,    16,    17,    18,    19,
      20,    21,    22,    23,    24,    25,    26,    27,    28,    29,
      30,    31,    32,    33,    34,    35,    36,    37,    38,    39,
      40,    41,    42,    43,    44,    45,    46,    47,    48,    49,
      50,    51,    52,    53,    54,    55,    56,    57,    58,    59,
      60,    61,    62,    63,    64,    65,    66,    67,    68,    69,
      70,    71,    72,    73,    74,    75,    76,    77,   104,   105,
     106,   107,   108,   109,    78,   110,   111,   119,   120,   121,
     122,     6,     7,     8,     9,    10,    11,    12,    13,     0,
      14,    15,    16,    17,    18,    19,    20,    21,    22,    23,
      24,    25,    26,    27,    28,    29,    30,    31,    32,    33,
      34,    35,    36,    37,    38,    39,    40,    41,    42,    43,
      44,    45,    46,    47,    48,    49,    50,    51,    52,    53,
      54,    55,    56,    57,    58,    59,    60,    61,    62,    63,
      64,    65,    66,    67,    68,    69,    70,    71,    72,    73,
      74,    75,    76,    77,   104,   105,   106,   107,   108,   109,
      78,   110,   111,     6,     7,     8,     9,    10,    11,    12,
      13,     0,    14,    15,    16,    17,    18,    19,    20,    21,
      22,    23,    24,    25,    26,    27,    28,    29,    30,    31,
      32,    33,    34,    35,    36,    37,    38,    39,    40,    41,
      42,    43,    44,    45,    46,    47,    48,    49,    50,    51,
      52,    53,    54,    55,    56,    57,    58,    59,    60,    61,
      62,    63,    64,    65,    66,    67,    68,    69,    70,    71,
      72,    73,    74,    75,    76,    77,   216,     0,     0,   217,
       0,   201,    78,     0,     0,     0,     0,   218,     0,     0,
     219,   220,   221,     0,   222,     0,     0,     0,     0,     0,
     223,     0,   224,     0,     0,     0,   203,     0,     0,   225,
     256,   226,   216,   257,   227,   217,     0,   201,     0,   228,
       0,     0,     0,     0,     0,     0,     0,   220,   221,     0,
     222,     0,   273,   274,     0,     0,   223,   229,   224,     0,
     230,     0,   203,     0,     0,   225,   216,   257,     0,   217,
     227,   201,   259,     0,     0,   228,     0,   218,     0,     0,
       0,   220,   221,     0,   222,     0,     0,     0,     0,     0,
     223,     0,     0,   229,     0,     0,   203,     0,     0,   225,
     256,   226,   216,   257,   227,   217,   259,   201,     0,   228,
       0,     0,     0,     0,     0,     0,     0,   220,   221,     0,
     222,   258,     0,     0,     0,     0,   223,   229,     0,   468,
       0,     0,   203,     0,     0,   225,     0,     0,     0,     0,
     227,     0,   259,     0,     0,   228,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,   229
};

#define yypact_value_is_default(Yystate) \
  (!!((Yystate) == (-729)))

#define yytable_value_is_error(Yytable_value) \
  YYID (0)

static const yytype_int16 yycheck[] =
{
      85,   481,   197,    87,   480,   296,   467,    91,   197,   467,
      22,   491,   467,   481,   482,   491,   467,   467,     0,   503,
       9,   749,   480,     4,    36,    92,   393,   503,   467,   115,
      27,    28,   118,   197,   102,   503,   126,   123,    30,    10,
      29,    33,    92,   501,   505,   503,   481,   132,   133,   134,
     505,    47,    41,    42,   505,   505,   491,     9,    10,   144,
     145,    86,   193,   194,    35,    61,   505,    38,    60,    10,
     156,   157,   158,    44,   160,     1,     2,   481,   482,    60,
     447,    92,    92,    35,    94,   170,    38,    39,   173,     3,
      42,     6,   177,   178,    35,    10,    10,    38,   826,   503,
     168,   392,   187,   188,    93,   294,   191,   192,   297,    24,
     200,   201,   202,   203,    29,    72,    73,    74,    75,   204,
      35,    35,    93,    38,    38,   196,   197,   212,    92,   219,
      45,    45,    60,   223,   224,   825,   331,   827,     4,   185,
     230,    93,   331,   227,   190,   229,    10,   193,   194,    52,
      53,    66,    66,   238,     6,    69,    92,     9,    94,    93,
      86,   193,    92,    89,    90,    37,   256,   331,    93,   259,
     255,    35,    24,    25,    26,    52,    53,    29,    93,    93,
      17,    18,   267,   273,   274,    92,   317,    94,   319,    41,
      42,    93,    10,     6,   279,    92,     9,    94,    92,    63,
      94,   127,   288,   289,   290,   291,    10,    49,    50,    51,
     295,    24,    25,    26,    93,   301,   142,    35,    93,    92,
     306,    94,    52,    53,   310,   185,    39,    92,   314,    42,
     190,    35,    85,   193,   194,   321,   322,   323,   324,   325,
      33,    93,   328,   329,    94,    63,   193,   332,   333,   196,
     197,    52,    53,    10,   443,   345,   327,   183,   184,    92,
     331,    94,   294,    10,   349,   297,    13,    14,    92,   749,
      94,   317,   436,   319,    94,    93,   361,    94,    35,   443,
      93,    38,    92,    92,    94,   317,   481,   482,    35,    93,
     216,   217,   218,    92,   220,   221,   222,    92,   759,    94,
     226,   781,   228,   467,   759,   789,    92,   791,   759,   759,
      67,   791,   443,   789,   790,   791,   480,   481,   482,   404,
     759,   789,   407,   791,    10,   410,   411,   412,   413,    92,
     415,   789,   417,   791,    92,   420,    93,   501,   423,   424,
     425,   505,   427,    10,    52,    53,    93,   294,   479,    35,
     297,    92,    38,   197,   789,   790,   791,   317,    92,   319,
     491,   446,    92,    10,    94,   436,   451,   531,    35,   454,
     317,    38,   457,    59,   505,   460,   461,    92,   468,   464,
     327,    92,    92,    94,   331,   789,    93,   791,    35,   474,
      92,    38,    94,   483,   484,   441,    92,   443,    94,    66,
       7,   896,   897,    10,   436,   490,   786,    93,   788,   480,
     481,   482,   497,    92,    61,    94,   342,   343,   344,    10,
     346,   347,    13,    14,   514,    32,    93,    92,    35,    94,
     501,    93,   503,   479,    92,    92,    94,    94,    92,   524,
      94,   789,   790,   529,    35,   491,    93,   479,    92,    92,
     482,   541,    93,    93,   539,   193,   540,   503,    93,   505,
     531,   507,    92,   548,    94,   511,    92,    82,   196,   197,
      93,    93,   557,   563,    95,    96,    97,    98,    99,   100,
     101,   441,    92,   443,    94,   570,    92,   331,    94,   436,
      34,    12,   578,   579,    46,    71,   627,   628,   629,   531,
     585,    39,    93,    92,    11,    94,    70,    93,   639,   640,
       6,   596,   131,     9,    10,   167,   169,   190,   143,   479,
     213,   606,   770,   447,   816,   772,   912,   148,    24,    25,
      26,   491,   479,    29,   619,   482,   912,   826,   779,    35,
     317,   626,   327,   503,   319,   505,   531,   507,   331,   634,
     503,   511,   759,   482,   505,   641,   294,   443,   505,   297,
     754,   633,   648,    -1,    -1,   651,    76,    77,    78,    79,
      80,    81,    68,    83,    -1,    -1,    -1,    -1,    -1,   317,
     665,   627,   628,   629,   531,    -1,    -1,   633,    -1,   515,
      -1,   676,   436,   639,   640,   759,   628,   629,    -1,   327,
      -1,    -1,   687,   331,     5,    -1,    -1,     8,   693,    -1,
      -1,    -1,    -1,    -1,   699,    16,    -1,   701,    -1,    20,
      21,   706,    23,   467,    -1,    -1,    11,    12,   759,    -1,
      -1,    -1,    -1,    -1,   719,    -1,   480,   481,   482,    40,
     726,    -1,    43,   728,    -1,    -1,   731,    48,    -1,    34,
     735,   736,    -1,   738,    39,    -1,   741,   501,    -1,   744,
      -1,    46,    47,    -1,    -1,   750,    -1,   627,   628,   629,
     760,   761,   762,   633,   764,    -1,    61,    -1,    -1,   639,
     640,   766,   629,    -1,   769,    70,    -1,   531,   773,    -1,
     775,   776,    93,   778,    -1,   780,    -1,    -1,   436,    -1,
      -1,    -1,   787,    -1,    -1,    -1,    -1,    -1,   436,    -1,
      -1,    -1,    -1,   759,    -1,   443,   801,    -1,   789,   790,
     806,    -1,    -1,    -1,   810,    -1,    -1,   813,   818,   819,
      -1,    -1,   817,    -1,   660,    -1,    -1,    -1,   823,   467,
      -1,   479,    -1,   828,   482,   830,    -1,    -1,     8,   834,
      10,    -1,   480,   481,   482,    -1,    -1,    -1,    -1,   844,
      20,    21,    -1,    23,    -1,   850,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,   501,   859,    35,    -1,   505,    38,    -1,
     911,    -1,   913,    43,    -1,    45,    -1,    -1,    48,   874,
     875,   876,   877,   531,   880,   881,    -1,    -1,    -1,   759,
      -1,    -1,   887,   531,    -1,   890,    66,    -1,   893,     3,
      -1,     5,    -1,    10,     8,    -1,    -1,   902,   903,    16,
      -1,    15,    16,    -1,   914,    -1,    20,    21,    -1,    23,
      -1,    -1,    -1,    93,   919,    -1,    -1,   927,    35,   924,
      -1,    38,    -1,    40,    -1,    -1,    40,   932,   934,    43,
      -1,   937,    -1,    -1,    48,    -1,    -1,    -1,    55,    56,
      -1,    -1,    -1,    57,    58,   911,   912,   913,    62,    66,
      64,    65,    -1,    -1,    -1,     3,    -1,     5,    -1,    -1,
       8,    -1,    -1,    -1,    -1,    -1,    -1,    15,    16,    -1,
     628,   629,    20,    21,    -1,    23,    93,    -1,    -1,    93,
     628,   629,    -1,    -1,    -1,    -1,     5,    -1,    -1,     8,
      -1,   639,    40,    -1,    -1,    43,    -1,    16,    -1,    -1,
      48,    20,    21,    -1,    23,    -1,    -1,    -1,    -1,    57,
      58,    -1,    -1,    -1,    62,    -1,    64,    65,    -1,    -1,
      -1,    40,    -1,    -1,    43,    -1,    -1,    -1,    -1,    48,
      -1,   911,   912,   913,    -1,     5,     6,    -1,     8,    -1,
      10,    -1,    -1,    -1,    -1,    93,    16,    -1,    -1,    19,
      20,    21,    -1,    23,    -1,    25,    26,    -1,    -1,    29,
      -1,    31,    -1,    -1,     5,    35,    -1,     8,    38,    10,
      40,    41,    -1,    43,    93,    45,    -1,    -1,    48,    20,
      21,    -1,    23,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    35,    -1,    66,    38,    -1,    -1,
      -1,    -1,    43,     5,    45,    -1,     8,    48,    10,    -1,
      -1,   759,    -1,    54,    16,    -1,    -1,    -1,    20,    21,
      -1,    23,    -1,    93,     5,    66,    -1,     8,    -1,    10,
      -1,    -1,    -1,    35,    -1,    16,    38,    -1,    40,    20,
      21,    43,    23,    -1,    -1,    -1,    48,    -1,    -1,    -1,
      -1,    -1,    93,    -1,    35,    -1,    -1,    38,    -1,    40,
      -1,    -1,    43,    -1,    66,    -1,     6,    48,     8,     9,
      10,    -1,    -1,    -1,     6,    -1,    -1,    -1,    10,    -1,
      20,    21,    -1,    23,    24,    -1,     6,    -1,    -1,     9,
      10,    93,    -1,    25,    26,    35,    -1,    29,    38,    31,
      -1,    -1,    -1,    35,    24,    45,    38,    39,    48,    29,
      42,    -1,    93,    45,    54,    35,    -1,     5,    38,    39,
       8,    -1,    42,    -1,    -1,    45,    66,    -1,    16,    -1,
      -1,    -1,    20,    21,    66,    23,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    66,    -1,    -1,    -1,
      -1,    -1,    40,    93,    -1,    43,    -1,    -1,    -1,    -1,
      48,    93,    -1,    -1,    -1,     3,     4,     5,     6,     7,
       8,     9,    10,    93,    12,    13,    14,    15,    16,    17,
      18,    19,    20,    21,    22,    23,    24,    25,    26,    27,
      28,    29,    30,    31,    32,    33,    34,    35,    36,    37,
      38,    39,    40,    41,    42,    43,    44,    45,    46,    47,
      48,    49,    50,    51,    52,    53,    54,    55,    56,    57,
      58,    59,    60,    61,    62,    63,    64,    65,    66,    67,
      68,    69,    70,    71,    72,    73,    74,    75,    76,    77,
      78,    79,    80,    81,    82,    83,    84,    85,    86,    87,
      88,     3,     4,     5,     6,     7,     8,     9,    10,    -1,
      12,    13,    14,    15,    16,    17,    18,    19,    20,    21,
      22,    23,    24,    25,    26,    27,    28,    29,    30,    31,
      32,    33,    34,    35,    36,    37,    38,    39,    40,    41,
      42,    43,    44,    45,    46,    47,    48,    49,    50,    51,
      52,    53,    54,    55,    56,    57,    58,    59,    60,    61,
      62,    63,    64,    65,    66,    67,    68,    69,    70,    71,
      72,    73,    74,    75,    76,    77,    78,    79,    80,    81,
      82,    83,    84,     3,     4,     5,     6,     7,     8,     9,
      10,    -1,    12,    13,    14,    15,    16,    17,    18,    19,
      20,    21,    22,    23,    24,    25,    26,    27,    28,    29,
      30,    31,    32,    33,    34,    35,    36,    37,    38,    39,
      40,    41,    42,    43,    44,    45,    46,    47,    48,    49,
      50,    51,    52,    53,    54,    55,    56,    57,    58,    59,
      60,    61,    62,    63,    64,    65,    66,    67,    68,    69,
      70,    71,    72,    73,    74,    75,     5,    -1,    -1,     8,
      -1,    10,    82,    -1,    -1,    -1,    -1,    16,    -1,    -1,
      19,    20,    21,    -1,    23,    -1,    -1,    -1,    -1,    -1,
      29,    -1,    31,    -1,    -1,    -1,    35,    -1,    -1,    38,
       3,    40,     5,     6,    43,     8,    -1,    10,    -1,    48,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    20,    21,    -1,
      23,    -1,    25,    26,    -1,    -1,    29,    66,    31,    -1,
      69,    -1,    35,    -1,    -1,    38,     5,     6,    -1,     8,
      43,    10,    45,    -1,    -1,    48,    -1,    16,    -1,    -1,
      -1,    20,    21,    -1,    23,    -1,    -1,    -1,    -1,    -1,
      29,    -1,    -1,    66,    -1,    -1,    35,    -1,    -1,    38,
       3,    40,     5,     6,    43,     8,    45,    10,    -1,    48,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    20,    21,    -1,
      23,    24,    -1,    -1,    -1,    -1,    29,    66,    -1,    68,
      -1,    -1,    35,    -1,    -1,    38,    -1,    -1,    -1,    -1,
      43,    -1,    45,    -1,    -1,    48,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    66
};

/* YYSTOS[STATE-NUM] -- The (internal number of the) accessing
   symbol of state STATE-NUM.  */
static const yytype_uint16 yystos[] =
{
       0,    27,    28,    96,    97,    99,     3,     4,     5,     6,
       7,     8,     9,    10,    12,    13,    14,    15,    16,    17,
      18,    19,    20,    21,    22,    23,    24,    25,    26,    27,
      28,    29,    30,    31,    32,    33,    34,    35,    36,    37,
      38,    39,    40,    41,    42,    43,    44,    45,    46,    47,
      48,    49,    50,    51,    52,    53,    54,    55,    56,    57,
      58,    59,    60,    61,    62,    63,    64,    65,    66,    67,
      68,    69,    70,    71,    72,    73,    74,    75,    82,   329,
     329,     0,    98,   100,    92,    92,    76,    77,    78,    79,
      80,    81,    83,   191,   192,   193,   198,   199,   204,   208,
     212,   213,   191,   329,    76,    77,    78,    79,    80,    81,
      83,    84,   328,   329,    52,    53,   329,   329,   328,    85,
      86,    87,    88,   214,   328,   331,    30,    33,    60,   101,
     102,   103,   121,   122,   124,   192,   192,   192,   192,   192,
     192,   192,     4,   106,   107,   122,   194,   200,    92,    94,
     190,   190,   205,   209,   190,   190,   215,   331,   327,   329,
      86,   109,   102,   191,   191,   191,   329,   109,   191,   191,
      92,    94,   195,    92,    94,   201,   192,    92,    92,   190,
     190,   190,   190,    17,    18,   104,   110,   130,   133,   108,
     104,   122,   107,   191,   191,    93,   191,   191,   329,   329,
       7,    10,    32,    35,   105,   111,   116,   117,   118,   119,
     191,   191,    92,   111,   191,   191,     5,     8,    16,    19,
      20,    21,    23,    29,    31,    38,    40,    43,    48,    66,
      69,   114,   118,   119,   120,   141,   147,   196,   197,   216,
     221,   228,   232,   236,   242,   243,   245,   256,   260,   264,
     311,   118,   119,   120,   202,   203,     3,     6,    24,    45,
     114,   115,   118,   119,   120,   141,   206,   207,   221,   226,
     227,   270,   274,    25,    26,   114,   115,   141,   210,   211,
     221,   240,   241,   242,   270,   274,   131,   134,   331,   331,
     331,   331,   191,    37,   112,   125,   191,   112,   329,   329,
     329,   331,   329,   329,   329,   331,   331,    49,    50,    51,
     123,   329,   328,   329,   328,   331,    93,   191,    93,   191,
     331,    52,    53,    52,    53,   331,    93,   191,   331,   331,
      93,   191,    92,    92,    94,   135,   190,   190,   190,   190,
      85,   129,    15,    57,    58,    62,    64,    65,    93,   113,
     114,   136,   142,   147,   256,   270,   275,   286,   291,   316,
     191,   124,    93,   246,   217,   257,   190,   229,   233,   237,
      94,   222,   190,   190,   148,    94,   261,   312,   190,   265,
     196,   202,   271,   190,   190,   190,   190,   190,   206,   190,
     190,   210,   191,   191,    94,   126,   329,   329,   329,   331,
     329,   329,   191,   191,    92,    94,   247,    92,    94,   218,
      92,    92,    92,    92,    92,    92,    92,    92,    94,   313,
      92,    94,   266,    92,   124,   125,   132,    92,   317,   276,
     287,   292,   137,   143,    93,   191,   191,   191,   191,   191,
     191,   223,   191,   262,   191,   191,   191,   191,   191,    93,
     191,    92,    94,   318,    92,    94,   277,    92,    94,   288,
      92,    92,    94,   138,    92,    94,   144,   248,    68,   114,
     115,   141,   147,   219,   220,   221,   225,   256,   274,   258,
     230,   234,   238,    13,    14,    93,   118,   119,   175,   176,
     224,   149,    93,   118,   119,   120,   141,   263,   264,   270,
     274,   314,   267,   268,   191,   272,   132,   127,   191,   191,
     191,   191,   191,   191,     9,    54,    93,   115,   141,   216,
     227,   228,   232,   236,   249,   250,   274,   311,   326,   331,
      93,   191,    93,   114,   118,   119,   120,   147,   256,   259,
      39,    42,    93,   115,   141,   151,   221,   227,   231,   274,
     325,   326,    93,   115,   141,   151,   221,   235,   240,   241,
     242,   274,   325,    41,    93,   114,   115,   141,   147,   221,
     239,   240,   241,   242,   243,   244,   256,   274,   331,   331,
     191,    93,   118,   119,   120,   150,   151,   325,   326,   191,
      93,   115,   141,   221,   227,   274,   315,    93,   118,   119,
     221,   225,   226,   227,   240,   241,   269,   326,    93,   114,
     118,   119,   120,   141,   216,   228,   232,   236,   250,   273,
     274,   311,    93,    93,   118,   119,   128,   319,   278,   289,
      63,   118,   119,   293,   294,   295,   296,   301,   306,   139,
     145,   331,   329,   191,   190,   219,   191,   328,   331,   191,
     191,   331,   191,   190,   190,   191,   191,   191,   191,   191,
      59,    93,   118,   119,   120,   320,   321,    55,    56,    93,
     118,   119,   120,   141,   147,   256,   279,   280,   284,    93,
     114,   118,   119,   120,   141,   147,   256,   290,    72,    73,
      74,    75,    93,   294,   191,    93,   118,   119,   120,   140,
     141,    61,    93,   118,   119,   120,   146,   181,   190,   251,
     152,   190,   190,   329,   191,   281,   285,   191,   191,    92,
      94,   297,   302,   307,   191,   191,   328,   191,    92,    94,
     252,    92,    94,   153,   322,    92,    92,   191,    92,    94,
     298,    92,    94,   303,    92,    94,   308,   190,   191,   191,
      92,    94,   323,   191,   191,    93,   191,   191,   191,   253,
      11,    12,    34,    46,    47,    70,   151,   154,   155,   156,
     158,   159,   165,   166,   177,   178,   179,   180,   181,   182,
     183,   189,   191,   114,   147,   256,   282,   283,   282,   299,
     304,   309,    93,   118,   119,   120,   141,   216,   228,   232,
     236,   254,   255,   260,   274,   311,   331,   331,   331,    82,
     331,    52,    53,   330,   191,    93,   191,   156,    22,    36,
     160,   161,   163,   166,   191,   191,   191,   191,   183,   191,
     151,    71,   324,    93,   283,   191,    93,    93,   221,   226,
     227,   240,   241,   244,   300,   325,   326,    93,   221,   244,
     305,   325,   326,    93,   151,   226,   227,   240,   241,   310,
     325,   326,   191,   190,   167,   157,   184,   190,   190,   158,
     191,   331,   331,   191,   179,   178,   181,   179,   191,   191,
      52,    53,    93,   191,   191,   191,   191,    92,    94,   168,
      92,    94,   172,    92,    94,   185,   162,   164,   191,   191,
     191,   191,   190,   190,   191,   191,   191,   172,   172,   191,
     191,   169,   173,   186,    44,    93,   118,   119,   120,   170,
     171,    93,   118,   119,   174,   175,   176,    67,    93,   118,
     119,   120,   187,   188,   331,   191,   191,   331,   191,   190,
     190
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
   Once GCC version 2 has supplanted version 1, this can go.  However,
   YYFAIL appears to be in use.  Nevertheless, it is formally deprecated
   in Bison 2.4.2's NEWS entry, where a plan to phase it out is
   discussed.  */

#define YYFAIL		goto yyerrlab
#if defined YYFAIL
  /* This is here to suppress warnings from the GCC cpp's
     -Wunused-macros.  Normally we don't worry about that warning, but
     some users do, and we want to make it easy for users to remove
     YYFAIL uses, which will produce warnings from Bison 2.5.  */
#endif

#define YYRECOVERING()  (!!yyerrstatus)

#define YYBACKUP(Token, Value)                                  \
do                                                              \
  if (yychar == YYEMPTY)                                        \
    {                                                           \
      yychar = (Token);                                         \
      yylval = (Value);                                         \
      YYPOPSTACK (yylen);                                       \
      yystate = *yyssp;                                         \
      goto yybackup;                                            \
    }                                                           \
  else                                                          \
    {                                                           \
      yyerror (parserPtr, YY_("syntax error: cannot back up")); \
      YYERROR;							\
    }								\
while (YYID (0))

/* Error token number */
#define YYTERROR	1
#define YYERRCODE	256


/* This macro is provided for backward compatibility. */
#ifndef YY_LOCATION_PRINT
# define YY_LOCATION_PRINT(File, Loc) ((void) 0)
#endif


/* YYLEX -- calling `yylex' with the right arguments.  */
#ifdef YYLEX_PARAM
# define YYLEX yylex (&yylval, YYLEX_PARAM)
#else
# define YYLEX yylex (&yylval, parserPtr)
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
		  Type, Value, parserPtr); \
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
yy_symbol_value_print (FILE *yyoutput, int yytype, YYSTYPE const * const yyvaluep, struct Parser *parserPtr)
#else
static void
yy_symbol_value_print (yyoutput, yytype, yyvaluep, parserPtr)
    FILE *yyoutput;
    int yytype;
    YYSTYPE const * const yyvaluep;
    struct Parser *parserPtr;
#endif
{
  FILE *yyo = yyoutput;
  YYUSE (yyo);
  if (!yyvaluep)
    return;
  YYUSE (parserPtr);
# ifdef YYPRINT
  if (yytype < YYNTOKENS)
    YYPRINT (yyoutput, yytoknum[yytype], *yyvaluep);
# else
  YYUSE (yyoutput);
# endif
  YYUSE (yytype);
}


/*--------------------------------.
| Print this symbol on YYOUTPUT.  |
`--------------------------------*/

#if (defined __STDC__ || defined __C99__FUNC__ \
     || defined __cplusplus || defined _MSC_VER)
static void
yy_symbol_print (FILE *yyoutput, int yytype, YYSTYPE const * const yyvaluep, struct Parser *parserPtr)
#else
static void
yy_symbol_print (yyoutput, yytype, yyvaluep, parserPtr)
    FILE *yyoutput;
    int yytype;
    YYSTYPE const * const yyvaluep;
    struct Parser *parserPtr;
#endif
{
  if (yytype < YYNTOKENS)
    YYFPRINTF (yyoutput, "token %s (", yytname[yytype]);
  else
    YYFPRINTF (yyoutput, "nterm %s (", yytname[yytype]);

  yy_symbol_value_print (yyoutput, yytype, yyvaluep, parserPtr);
  YYFPRINTF (yyoutput, ")");
}

/*------------------------------------------------------------------.
| yy_stack_print -- Print the state stack from its BOTTOM up to its |
| TOP (included).                                                   |
`------------------------------------------------------------------*/

#if (defined __STDC__ || defined __C99__FUNC__ \
     || defined __cplusplus || defined _MSC_VER)
static void
yy_stack_print (yytype_int16 *yybottom, yytype_int16 *yytop)
#else
static void
yy_stack_print (yybottom, yytop)
    yytype_int16 *yybottom;
    yytype_int16 *yytop;
#endif
{
  YYFPRINTF (stderr, "Stack now");
  for (; yybottom <= yytop; yybottom++)
    {
      int yybot = *yybottom;
      YYFPRINTF (stderr, " %d", yybot);
    }
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
yy_reduce_print (YYSTYPE *yyvsp, int yyrule, struct Parser *parserPtr)
#else
static void
yy_reduce_print (yyvsp, yyrule, parserPtr)
    YYSTYPE *yyvsp;
    int yyrule;
    struct Parser *parserPtr;
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
      YYFPRINTF (stderr, "   $%d = ", yyi + 1);
      yy_symbol_print (stderr, yyrhs[yyprhs[yyrule] + yyi],
		       &(yyvsp[(yyi + 1) - (yynrhs)])
		       		       , parserPtr);
      YYFPRINTF (stderr, "\n");
    }
}

# define YY_REDUCE_PRINT(Rule)		\
do {					\
  if (yydebug)				\
    yy_reduce_print (yyvsp, Rule, parserPtr); \
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

/* Copy into *YYMSG, which is of size *YYMSG_ALLOC, an error message
   about the unexpected token YYTOKEN for the state stack whose top is
   YYSSP.

   Return 0 if *YYMSG was successfully written.  Return 1 if *YYMSG is
   not large enough to hold the message.  In that case, also set
   *YYMSG_ALLOC to the required number of bytes.  Return 2 if the
   required number of bytes is too large to store.  */
static int
yysyntax_error (YYSIZE_T *yymsg_alloc, char **yymsg,
                yytype_int16 *yyssp, int yytoken)
{
  YYSIZE_T yysize0 = yytnamerr (YY_NULL, yytname[yytoken]);
  YYSIZE_T yysize = yysize0;
  enum { YYERROR_VERBOSE_ARGS_MAXIMUM = 5 };
  /* Internationalized format string. */
  const char *yyformat = YY_NULL;
  /* Arguments of yyformat. */
  char const *yyarg[YYERROR_VERBOSE_ARGS_MAXIMUM];
  /* Number of reported tokens (one for the "unexpected", one per
     "expected"). */
  int yycount = 0;

  /* There are many possibilities here to consider:
     - Assume YYFAIL is not used.  It's too flawed to consider.  See
       <http://lists.gnu.org/archive/html/bison-patches/2009-12/msg00024.html>
       for details.  YYERROR is fine as it does not invoke this
       function.
     - If this state is a consistent state with a default action, then
       the only way this function was invoked is if the default action
       is an error action.  In that case, don't check for expected
       tokens because there are none.
     - The only way there can be no lookahead present (in yychar) is if
       this state is a consistent state with a default action.  Thus,
       detecting the absence of a lookahead is sufficient to determine
       that there is no unexpected or expected token to report.  In that
       case, just report a simple "syntax error".
     - Don't assume there isn't a lookahead just because this state is a
       consistent state with a default action.  There might have been a
       previous inconsistent state, consistent state with a non-default
       action, or user semantic action that manipulated yychar.
     - Of course, the expected token list depends on states to have
       correct lookahead information, and it depends on the parser not
       to perform extra reductions after fetching a lookahead from the
       scanner and before detecting a syntax error.  Thus, state merging
       (from LALR or IELR) and default reductions corrupt the expected
       token list.  However, the list is correct for canonical LR with
       one exception: it will still contain any token that will not be
       accepted due to an error action in a later state.
  */
  if (yytoken != YYEMPTY)
    {
      int yyn = yypact[*yyssp];
      yyarg[yycount++] = yytname[yytoken];
      if (!yypact_value_is_default (yyn))
        {
          /* Start YYX at -YYN if negative to avoid negative indexes in
             YYCHECK.  In other words, skip the first -YYN actions for
             this state because they are default actions.  */
          int yyxbegin = yyn < 0 ? -yyn : 0;
          /* Stay within bounds of both yycheck and yytname.  */
          int yychecklim = YYLAST - yyn + 1;
          int yyxend = yychecklim < YYNTOKENS ? yychecklim : YYNTOKENS;
          int yyx;

          for (yyx = yyxbegin; yyx < yyxend; ++yyx)
            if (yycheck[yyx + yyn] == yyx && yyx != YYTERROR
                && !yytable_value_is_error (yytable[yyx + yyn]))
              {
                if (yycount == YYERROR_VERBOSE_ARGS_MAXIMUM)
                  {
                    yycount = 1;
                    yysize = yysize0;
                    break;
                  }
                yyarg[yycount++] = yytname[yyx];
                {
                  YYSIZE_T yysize1 = yysize + yytnamerr (YY_NULL, yytname[yyx]);
                  if (! (yysize <= yysize1
                         && yysize1 <= YYSTACK_ALLOC_MAXIMUM))
                    return 2;
                  yysize = yysize1;
                }
              }
        }
    }

  switch (yycount)
    {
# define YYCASE_(N, S)                      \
      case N:                               \
        yyformat = S;                       \
      break
      YYCASE_(0, YY_("syntax error"));
      YYCASE_(1, YY_("syntax error, unexpected %s"));
      YYCASE_(2, YY_("syntax error, unexpected %s, expecting %s"));
      YYCASE_(3, YY_("syntax error, unexpected %s, expecting %s or %s"));
      YYCASE_(4, YY_("syntax error, unexpected %s, expecting %s or %s or %s"));
      YYCASE_(5, YY_("syntax error, unexpected %s, expecting %s or %s or %s or %s"));
# undef YYCASE_
    }

  {
    YYSIZE_T yysize1 = yysize + yystrlen (yyformat);
    if (! (yysize <= yysize1 && yysize1 <= YYSTACK_ALLOC_MAXIMUM))
      return 2;
    yysize = yysize1;
  }

  if (*yymsg_alloc < yysize)
    {
      *yymsg_alloc = 2 * yysize;
      if (! (yysize <= *yymsg_alloc
             && *yymsg_alloc <= YYSTACK_ALLOC_MAXIMUM))
        *yymsg_alloc = YYSTACK_ALLOC_MAXIMUM;
      return 1;
    }

  /* Avoid sprintf, as that infringes on the user's name space.
     Don't have undefined behavior even if the translation
     produced a string with the wrong number of "%s"s.  */
  {
    char *yyp = *yymsg;
    int yyi = 0;
    while ((*yyp = *yyformat) != '\0')
      if (*yyp == '%' && yyformat[1] == 's' && yyi < yycount)
        {
          yyp += yytnamerr (yyp, yyarg[yyi++]);
          yyformat += 2;
        }
      else
        {
          yyp++;
          yyformat++;
        }
  }
  return 0;
}
#endif /* YYERROR_VERBOSE */

/*-----------------------------------------------.
| Release the memory associated to this symbol.  |
`-----------------------------------------------*/

/*ARGSUSED*/
#if (defined __STDC__ || defined __C99__FUNC__ \
     || defined __cplusplus || defined _MSC_VER)
static void
yydestruct (const char *yymsg, int yytype, YYSTYPE *yyvaluep, struct Parser *parserPtr)
#else
static void
yydestruct (yymsg, yytype, yyvaluep, parserPtr)
    const char *yymsg;
    int yytype;
    YYSTYPE *yyvaluep;
    struct Parser *parserPtr;
#endif
{
  YYUSE (yyvaluep);
  YYUSE (parserPtr);

  if (!yymsg)
    yymsg = "Deleting";
  YY_SYMBOL_PRINT (yymsg, yytype, yyvaluep, yylocationp);

  YYUSE (yytype);
}




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
yyparse (struct Parser *parserPtr)
#else
int
yyparse (parserPtr)
    struct Parser *parserPtr;
#endif
#endif
{
/* The lookahead symbol.  */
int yychar;


#if defined __GNUC__ && 407 <= __GNUC__ * 100 + __GNUC_MINOR__
/* Suppress an incorrect diagnostic about yylval being uninitialized.  */
# define YY_IGNORE_MAYBE_UNINITIALIZED_BEGIN \
    _Pragma ("GCC diagnostic push") \
    _Pragma ("GCC diagnostic ignored \"-Wuninitialized\"")\
    _Pragma ("GCC diagnostic ignored \"-Wmaybe-uninitialized\"")
# define YY_IGNORE_MAYBE_UNINITIALIZED_END \
    _Pragma ("GCC diagnostic pop")
#else
/* Default value used for initialization, for pacifying older GCCs
   or non-GCC compilers.  */
static YYSTYPE yyval_default;
# define YY_INITIAL_VALUE(Value) = Value
#endif
#ifndef YY_IGNORE_MAYBE_UNINITIALIZED_BEGIN
# define YY_IGNORE_MAYBE_UNINITIALIZED_BEGIN
# define YY_IGNORE_MAYBE_UNINITIALIZED_END
#endif
#ifndef YY_INITIAL_VALUE
# define YY_INITIAL_VALUE(Value) /* Nothing. */
#endif

/* The semantic value of the lookahead symbol.  */
YYSTYPE yylval YY_INITIAL_VALUE(yyval_default);

    /* Number of syntax errors so far.  */
    int yynerrs;

    int yystate;
    /* Number of tokens to shift before error messages enabled.  */
    int yyerrstatus;

    /* The stacks and their tools:
       `yyss': related to states.
       `yyvs': related to semantic values.

       Refer to the stacks through separate pointers, to allow yyoverflow
       to reallocate them elsewhere.  */

    /* The state stack.  */
    yytype_int16 yyssa[YYINITDEPTH];
    yytype_int16 *yyss;
    yytype_int16 *yyssp;

    /* The semantic value stack.  */
    YYSTYPE yyvsa[YYINITDEPTH];
    YYSTYPE *yyvs;
    YYSTYPE *yyvsp;

    YYSIZE_T yystacksize;

  int yyn;
  int yyresult;
  /* Lookahead token as an internal (translated) token number.  */
  int yytoken = 0;
  /* The variables used to return semantic value and location from the
     action routines.  */
  YYSTYPE yyval;

#if YYERROR_VERBOSE
  /* Buffer for error messages, and its allocated size.  */
  char yymsgbuf[128];
  char *yymsg = yymsgbuf;
  YYSIZE_T yymsg_alloc = sizeof yymsgbuf;
#endif

#define YYPOPSTACK(N)   (yyvsp -= (N), yyssp -= (N))

  /* The number of symbols on the RHS of the reduced rule.
     Keep to zero when no symbol should be popped.  */
  int yylen = 0;

  yyssp = yyss = yyssa;
  yyvsp = yyvs = yyvsa;
  yystacksize = YYINITDEPTH;

  YYDPRINTF ((stderr, "Starting parse\n"));

  yystate = 0;
  yyerrstatus = 0;
  yynerrs = 0;
  yychar = YYEMPTY; /* Cause a token to be read.  */
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
	YYSTACK_RELOCATE (yyss_alloc, yyss);
	YYSTACK_RELOCATE (yyvs_alloc, yyvs);
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

  if (yystate == YYFINAL)
    YYACCEPT;

  goto yybackup;

/*-----------.
| yybackup.  |
`-----------*/
yybackup:

  /* Do appropriate processing given the current state.  Read a
     lookahead token if we need one and don't already have one.  */

  /* First try to decide what to do without reference to lookahead token.  */
  yyn = yypact[yystate];
  if (yypact_value_is_default (yyn))
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
      if (yytable_value_is_error (yyn))
        goto yyerrlab;
      yyn = -yyn;
      goto yyreduce;
    }

  /* Count tokens shifted since error; after three, turn off error
     status.  */
  if (yyerrstatus)
    yyerrstatus--;

  /* Shift the lookahead token.  */
  YY_SYMBOL_PRINT ("Shifting", yytoken, &yylval, &yylloc);

  /* Discard the shifted token.  */
  yychar = YYEMPTY;

  yystate = yyn;
  YY_IGNORE_MAYBE_UNINITIALIZED_BEGIN
  *++yyvsp = yylval;
  YY_IGNORE_MAYBE_UNINITIALIZED_END

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
        case 4:
/* Line 1787 of yacc.c  */
#line 581 "parser-yang.y"
    {
                    currentParser = thisParserPtr;
                    thisParserPtr->yangModulePtr = findYangModuleByName((yyvsp[(2) - (2)].text), NULL);
                    if (!thisParserPtr->yangModulePtr) {
                        thisParserPtr->yangModulePtr =  addYangNode((yyvsp[(2) - (2)].text), YANG_DECL_MODULE, NULL);
                    
                        if (smiHandle->firstYangModulePtr) {
                            smiHandle->firstYangModulePtr->nextSiblingPtr = thisModulePtr;
                        } else {
                            smiHandle->firstYangModulePtr = thisModulePtr;
                        }
                    } else {
                        smiPrintError(thisParserPtr, ERR_MODULE_ALREADY_LOADED, (yyvsp[(2) - (2)].text));
                        free((yyvsp[(2) - (2)].text));
                        /*
                         * this aborts parsing the whole file,
                         * not only the current module.
                         */
                        YYABORT;
                    }
                    thisModulePtr->info = createModuleInfo(thisModulePtr);
                    pushNode(thisModulePtr);
                }
    break;

  case 5:
/* Line 1787 of yacc.c  */
#line 612 "parser-yang.y"
    {
                    thisModuleInfoPtr->parsingState  = YANG_PARSING_DONE;
                    pop();
                    yangSemanticAnalysis(thisModulePtr);
                }
    break;

  case 6:
/* Line 1787 of yacc.c  */
#line 620 "parser-yang.y"
    {
                        currentParser = thisParserPtr;
                        thisParserPtr->yangModulePtr = findYangModuleByName((yyvsp[(2) - (2)].text), NULL);
                        if (!thisParserPtr->yangModulePtr) {
                            thisParserPtr->yangModulePtr =  addYangNode((yyvsp[(2) - (2)].text), YANG_DECL_SUBMODULE, NULL);
                    
                            if (smiHandle->firstYangModulePtr) {
                                smiHandle->firstYangModulePtr->nextSiblingPtr = thisModulePtr;
                            } else {
                                smiHandle->firstYangModulePtr = thisModulePtr;
                            }
                        } else {
                            smiPrintError(thisParserPtr, ERR_MODULE_ALREADY_LOADED, (yyvsp[(2) - (2)].text));
                            free((yyvsp[(2) - (2)].text));
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
    break;

  case 7:
/* Line 1787 of yacc.c  */
#line 652 "parser-yang.y"
    {
                        thisModuleInfoPtr->parsingState  = YANG_PARSING_DONE;
                        pop();
                        yangSemanticAnalysis(thisModulePtr);
                    }
    break;

  case 8:
/* Line 1787 of yacc.c  */
#line 660 "parser-yang.y"
    {
                            if (!thisModuleInfoPtr->namespace) {
                                smiPrintError(parserPtr, ERR_NAMESPACE_MISSING, NULL);
                            }
                            if (!thisModuleInfoPtr->prefix) {
                                smiPrintError(parserPtr, ERR_PREFIX_MISSING, NULL);
                            }
			}
    break;

  case 14:
/* Line 1787 of yacc.c  */
#line 682 "parser-yang.y"
    {}
    break;

  case 23:
/* Line 1787 of yacc.c  */
#line 707 "parser-yang.y"
    {
                        node = addYangNode((yyvsp[(2) - (2)].text), YANG_DECL_BELONGS_TO, topNode());
                        pushNode(node);
                    }
    break;

  case 24:
/* Line 1787 of yacc.c  */
#line 714 "parser-yang.y"
    {
                        pop();
                    }
    break;

  case 25:
/* Line 1787 of yacc.c  */
#line 719 "parser-yang.y"
    {}
    break;

  case 29:
/* Line 1787 of yacc.c  */
#line 729 "parser-yang.y"
    {}
    break;

  case 31:
/* Line 1787 of yacc.c  */
#line 734 "parser-yang.y"
    {}
    break;

  case 54:
/* Line 1787 of yacc.c  */
#line 785 "parser-yang.y"
    {
                            if (!thisModuleInfoPtr->organization) {
                                node = addYangNode((yyvsp[(2) - (3)].text), YANG_DECL_ORGANIZATION, topNode());
                                thisModuleInfoPtr->organization = node->export.value;
                            } else {
                                smiPrintError(currentParser, ERR_REDEFINED_ORGANIZATION, NULL);
                            }
			}
    break;

  case 55:
/* Line 1787 of yacc.c  */
#line 796 "parser-yang.y"
    {
                            if (!thisModuleInfoPtr->contact) {
                                node = addYangNode((yyvsp[(2) - (3)].text), YANG_DECL_CONTACT, topNode());
                                thisModuleInfoPtr->contact = node->export.value;
                            } else {
                                smiPrintError(currentParser, ERR_REDEFINED_CONTACT, NULL);
                            }
			}
    break;

  case 56:
/* Line 1787 of yacc.c  */
#line 807 "parser-yang.y"
    {
                            uniqueNodeKind(topNode(), YANG_DECL_DESCRIPTION);
                            setDescription(topNode(), (yyvsp[(2) - (3)].text));
                            node = addYangNode((yyvsp[(2) - (3)].text), YANG_DECL_DESCRIPTION, topNode());
			}
    break;

  case 57:
/* Line 1787 of yacc.c  */
#line 815 "parser-yang.y"
    {
                            uniqueNodeKind(topNode(), YANG_DECL_REFERENCE);
                            setReference(topNode(), (yyvsp[(2) - (3)].text));
                            node = addYangNode((yyvsp[(2) - (3)].text), YANG_DECL_REFERENCE, topNode());
			}
    break;

  case 58:
/* Line 1787 of yacc.c  */
#line 823 "parser-yang.y"
    {
                            uniqueNodeKind(topNode(), YANG_DECL_STATUS);
                            setStatus(topNode(), (yyvsp[(2) - (3)].rc));
                            node = addYangNode(yangStatusAsString((yyvsp[(2) - (3)].rc)), YANG_DECL_STATUS, topNode());
			}
    break;

  case 59:
/* Line 1787 of yacc.c  */
#line 831 "parser-yang.y"
    {
                            if (!thisModuleInfoPtr->namespace) {
                                node = addYangNode((yyvsp[(2) - (3)].text), YANG_DECL_NAMESPACE, topNode());
                                thisModuleInfoPtr->namespace = node->export.value;
                            } else {
                                smiPrintError(currentParser, ERR_REDEFINED_NAMESPACE, NULL);
                            }
			}
    break;

  case 60:
/* Line 1787 of yacc.c  */
#line 842 "parser-yang.y"
    {
                            if (!thisModuleInfoPtr->version) {
                                node = addYangNode((yyvsp[(2) - (3)].text), YANG_DECL_YANG_VERSION, topNode());
                                thisModuleInfoPtr->version = node->export.value;
                            } else {
                                smiPrintError(currentParser, ERR_REDEFINED_YANGVERSION, NULL);
                            }
			}
    break;

  case 61:
/* Line 1787 of yacc.c  */
#line 853 "parser-yang.y"
    {
                    (yyval.rc) = YANG_STATUS_DEPRECATED;
		}
    break;

  case 62:
/* Line 1787 of yacc.c  */
#line 858 "parser-yang.y"
    {
                    (yyval.rc) = YANG_STATUS_CURRENT;
		}
    break;

  case 63:
/* Line 1787 of yacc.c  */
#line 863 "parser-yang.y"
    {
                    (yyval.rc) = YANG_STATUS_OBSOLETE;
		}
    break;

  case 64:
/* Line 1787 of yacc.c  */
#line 869 "parser-yang.y"
    {
                            node = addYangNode((yyvsp[(2) - (3)].text), YANG_DECL_PREFIX, topNode());
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
    break;

  case 65:
/* Line 1787 of yacc.c  */
#line 893 "parser-yang.y"
    {
                            node = addYangNode((yyvsp[(2) - (3)].text), YANG_DECL_REVISION, topNode());
                        }
    break;

  case 66:
/* Line 1787 of yacc.c  */
#line 898 "parser-yang.y"
    {
                            node = addYangNode((yyvsp[(2) - (2)].text), YANG_DECL_REVISION, topNode());
                            pushNode(node);
			}
    break;

  case 67:
/* Line 1787 of yacc.c  */
#line 906 "parser-yang.y"
    {
                            pop();
			}
    break;

  case 68:
/* Line 1787 of yacc.c  */
#line 911 "parser-yang.y"
    {}
    break;

  case 72:
/* Line 1787 of yacc.c  */
#line 922 "parser-yang.y"
    {
            checkDate(currentParser, (yyvsp[(1) - (1)].text));
	}
    break;

  case 73:
/* Line 1787 of yacc.c  */
#line 928 "parser-yang.y"
    {
                    node = addYangNode((yyvsp[(2) - (2)].text), YANG_DECL_IMPORT, topNode());
                    pushNode(node);
		}
    break;

  case 74:
/* Line 1787 of yacc.c  */
#line 937 "parser-yang.y"
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
    break;

  case 75:
/* Line 1787 of yacc.c  */
#line 952 "parser-yang.y"
    {}
    break;

  case 77:
/* Line 1787 of yacc.c  */
#line 958 "parser-yang.y"
    {
                    node = addYangNode((yyvsp[(2) - (2)].text), YANG_DECL_INCLUDE, topNode());
                    pushNode(node);
		}
    break;

  case 78:
/* Line 1787 of yacc.c  */
#line 963 "parser-yang.y"
    {
                    _YangNode *includedModule = externalModule(topNode());
                    yangValidateInclude(thisModulePtr, includedModule);
                    pop();
                    if (topNode() != thisModulePtr) {
                        pop();
                    }
		}
    break;

  case 79:
/* Line 1787 of yacc.c  */
#line 973 "parser-yang.y"
    {}
    break;

  case 80:
/* Line 1787 of yacc.c  */
#line 978 "parser-yang.y"
    {}
    break;

  case 81:
/* Line 1787 of yacc.c  */
#line 982 "parser-yang.y"
    {
                    node = addYangNode((yyvsp[(2) - (2)].text), YANG_DECL_FEATURE, topNode());
                    pushNode(node);
                }
    break;

  case 82:
/* Line 1787 of yacc.c  */
#line 987 "parser-yang.y"
    {
                    pop();
                }
    break;

  case 83:
/* Line 1787 of yacc.c  */
#line 993 "parser-yang.y"
    {}
    break;

  case 84:
/* Line 1787 of yacc.c  */
#line 998 "parser-yang.y"
    {}
    break;

  case 85:
/* Line 1787 of yacc.c  */
#line 1001 "parser-yang.y"
    {}
    break;

  case 91:
/* Line 1787 of yacc.c  */
#line 1015 "parser-yang.y"
    {
                        node = addYangNode((yyvsp[(2) - (3)].text), YANG_DECL_IF_FEATURE, topNode());
                        createIdentifierRef(node, getPrefix((yyvsp[(2) - (3)].text)), getIdentifier((yyvsp[(2) - (3)].text)));
                    }
    break;

  case 92:
/* Line 1787 of yacc.c  */
#line 1022 "parser-yang.y"
    {
                    node = addYangNode((yyvsp[(2) - (2)].text), YANG_DECL_IDENTITY, topNode());
                    pushNode(node);
                }
    break;

  case 93:
/* Line 1787 of yacc.c  */
#line 1027 "parser-yang.y"
    {
                    pop();
                }
    break;

  case 94:
/* Line 1787 of yacc.c  */
#line 1031 "parser-yang.y"
    {}
    break;

  case 95:
/* Line 1787 of yacc.c  */
#line 1036 "parser-yang.y"
    {}
    break;

  case 96:
/* Line 1787 of yacc.c  */
#line 1039 "parser-yang.y"
    {}
    break;

  case 102:
/* Line 1787 of yacc.c  */
#line 1053 "parser-yang.y"
    {
                    if (getBuiltInType((yyvsp[(2) - (2)].text)) != YANG_TYPE_NONE) {
                        smiPrintError(thisParserPtr, ERR_ILLEGAL_TYPE_NAME, (yyvsp[(2) - (2)].text));
                    }
                    node = addYangNode((yyvsp[(2) - (2)].text), YANG_DECL_TYPEDEF, topNode());
                    pushNode(node);
                }
    break;

  case 103:
/* Line 1787 of yacc.c  */
#line 1064 "parser-yang.y"
    {                                
                    if (getCardinality(topNode(), YANG_DECL_TYPE) != 1) {
                        smiPrintError(currentParser, ERR_WRONG_CARDINALITY, yangDeclAsString(YANG_DECL_TYPE), "1");
                    }
                    pop();
                }
    break;

  case 104:
/* Line 1787 of yacc.c  */
#line 1072 "parser-yang.y"
    {}
    break;

  case 112:
/* Line 1787 of yacc.c  */
#line 1092 "parser-yang.y"
    {
                    node = addYangNode((yyvsp[(2) - (2)].text), YANG_DECL_TYPE, topNode());
                    createTypeInfo(node);
                    if (getBuiltInType((yyvsp[(2) - (2)].text)) == YANG_TYPE_NONE) {
                        createIdentifierRef(node, getPrefix((yyvsp[(2) - (2)].text)), getIdentifier((yyvsp[(2) - (2)].text)));
                    }
                    pushNode(node);
               }
    break;

  case 113:
/* Line 1787 of yacc.c  */
#line 1101 "parser-yang.y"
    {
                    pop();
               }
    break;

  case 114:
/* Line 1787 of yacc.c  */
#line 1106 "parser-yang.y"
    {}
    break;

  case 115:
/* Line 1787 of yacc.c  */
#line 1111 "parser-yang.y"
    {}
    break;

  case 127:
/* Line 1787 of yacc.c  */
#line 1142 "parser-yang.y"
    {
                    uniqueNodeKind(topNode(), YANG_DECL_RANGE);
                    node = addYangNode((yyvsp[(2) - (2)].text), YANG_DECL_RANGE, topNode());
                    pushNode(node);
		}
    break;

  case 128:
/* Line 1787 of yacc.c  */
#line 1148 "parser-yang.y"
    {
			pop();
		}
    break;

  case 129:
/* Line 1787 of yacc.c  */
#line 1154 "parser-yang.y"
    {
                    uniqueNodeKind(topNode(), YANG_DECL_FRACTION_DIGITS);
                    node = addYangNode((yyvsp[(2) - (3)].text), YANG_DECL_FRACTION_DIGITS, topNode());
		}
    break;

  case 130:
/* Line 1787 of yacc.c  */
#line 1160 "parser-yang.y"
    {}
    break;

  case 134:
/* Line 1787 of yacc.c  */
#line 1171 "parser-yang.y"
    {
                    uniqueNodeKind(topNode(), YANG_DECL_LENGTH);
                    node = addYangNode((yyvsp[(2) - (2)].text), YANG_DECL_LENGTH, topNode());
                    pushNode(node);
                }
    break;

  case 135:
/* Line 1787 of yacc.c  */
#line 1177 "parser-yang.y"
    {
			pop();
		}
    break;

  case 136:
/* Line 1787 of yacc.c  */
#line 1184 "parser-yang.y"
    {
            node = addYangNode((yyvsp[(2) - (2)].text), YANG_DECL_PATTERN, topNode());
            pushNode(node);
        }
    break;

  case 137:
/* Line 1787 of yacc.c  */
#line 1189 "parser-yang.y"
    {
            pop();
        }
    break;

  case 140:
/* Line 1787 of yacc.c  */
#line 1200 "parser-yang.y"
    {
            if (strlen((yyvsp[(2) - (2)].text)) == 0 || isWSP((yyvsp[(2) - (2)].text)[0]) || isWSP((yyvsp[(2) - (2)].text)[strlen((yyvsp[(2) - (2)].text))-1])) {
                smiPrintError(currentParser, ERR_WRONG_ENUM);
            }
            node = findChildNodeByTypeAndValue(topNode(), YANG_DECL_ENUM, (yyvsp[(2) - (2)].text));
            if (node) {
                smiPrintError(currentParser, ERR_DUPLICATED_ENUM_NAME, (yyvsp[(2) - (2)].text));
            }
            node = addYangNode((yyvsp[(2) - (2)].text), YANG_DECL_ENUM, topNode());
            pushNode(node);
        }
    break;

  case 141:
/* Line 1787 of yacc.c  */
#line 1212 "parser-yang.y"
    {
            pop();
        }
    break;

  case 142:
/* Line 1787 of yacc.c  */
#line 1217 "parser-yang.y"
    {}
    break;

  case 143:
/* Line 1787 of yacc.c  */
#line 1222 "parser-yang.y"
    {}
    break;

  case 144:
/* Line 1787 of yacc.c  */
#line 1225 "parser-yang.y"
    {}
    break;

  case 150:
/* Line 1787 of yacc.c  */
#line 1240 "parser-yang.y"
    {                
                node = addYangNode((yyvsp[(2) - (3)].text), YANG_DECL_VALUE, topNode());
            }
    break;

  case 151:
/* Line 1787 of yacc.c  */
#line 1245 "parser-yang.y"
    {}
    break;

  case 152:
/* Line 1787 of yacc.c  */
#line 1250 "parser-yang.y"
    {}
    break;

  case 153:
/* Line 1787 of yacc.c  */
#line 1253 "parser-yang.y"
    {}
    break;

  case 159:
/* Line 1787 of yacc.c  */
#line 1268 "parser-yang.y"
    {
                uniqueNodeKind(topNode(), YANG_DECL_ERROR_MESSAGE);
                node = addYangNode((yyvsp[(2) - (3)].text), YANG_DECL_ERROR_MESSAGE, topNode());
            }
    break;

  case 160:
/* Line 1787 of yacc.c  */
#line 1275 "parser-yang.y"
    {
                uniqueNodeKind(topNode(), YANG_DECL_ERROR_APP_TAG);
                node = addYangNode((yyvsp[(2) - (3)].text), YANG_DECL_ERROR_APP_TAG, topNode());
            }
    break;

  case 164:
/* Line 1787 of yacc.c  */
#line 1291 "parser-yang.y"
    {
            uniqueNodeKind(topNode(), YANG_DECL_PATH);
            node = addYangNode((yyvsp[(2) - (3)].text), YANG_DECL_PATH, topNode());
		}
    break;

  case 165:
/* Line 1787 of yacc.c  */
#line 1298 "parser-yang.y"
    {
            uniqueNodeKind(topNode(), YANG_DECL_REQUIRE_INSTANCE);
            if ((yyvsp[(2) - (3)].rc) == YANG_BOOLEAN_TRUE) {
                node = addYangNode("true", YANG_DECL_REQUIRE_INSTANCE, topNode());
            } else {
                node = addYangNode("false", YANG_DECL_REQUIRE_INSTANCE, topNode());
            }
        }
    break;

  case 169:
/* Line 1787 of yacc.c  */
#line 1318 "parser-yang.y"
    {
                uniqueNodeKind(topNode(), YANG_DECL_BASE);
                node = addYangNode((yyvsp[(2) - (3)].text), YANG_DECL_BASE, topNode());
                createIdentifierRef(node, getPrefix((yyvsp[(2) - (3)].text)), getIdentifier((yyvsp[(2) - (3)].text)));
            }
    break;

  case 172:
/* Line 1787 of yacc.c  */
#line 1331 "parser-yang.y"
    {
                node = findChildNodeByTypeAndValue(topNode(), YANG_DECL_BIT, (yyvsp[(2) - (2)].text));
                if (node) {
                    smiPrintError(currentParser, ERR_DUPLICATED, "bit", (yyvsp[(2) - (2)].text));
                }

                node = addYangNode((yyvsp[(2) - (2)].text), YANG_DECL_BIT, topNode());
                pushNode(node);
            }
    break;

  case 173:
/* Line 1787 of yacc.c  */
#line 1341 "parser-yang.y"
    {
                pop();
            }
    break;

  case 174:
/* Line 1787 of yacc.c  */
#line 1346 "parser-yang.y"
    {}
    break;

  case 175:
/* Line 1787 of yacc.c  */
#line 1351 "parser-yang.y"
    {}
    break;

  case 176:
/* Line 1787 of yacc.c  */
#line 1354 "parser-yang.y"
    {}
    break;

  case 182:
/* Line 1787 of yacc.c  */
#line 1369 "parser-yang.y"
    {
		    _YangNode* typePtr, *childPtr;
                    if (!isNonNegativeInteger((yyvsp[(2) - (3)].text))) {
                        smiPrintError(currentParser, ERR_ARG_VALUE, (yyvsp[(2) - (3)].text), "non-negative-integer");
                    }                    
                    uniqueNodeKind(topNode(), YANG_DECL_POSITION);

                    /* position values must be unique within the type */
                    typePtr = topNode()->parentPtr;
                    childPtr = typePtr->firstChildPtr;
                    while (childPtr) {
                        if (childPtr->export.nodeKind == YANG_DECL_BIT) {
                            _YangNode* positionPtr = findChildNodeByTypeAndValue(childPtr, YANG_DECL_POSITION, (yyvsp[(2) - (3)].text));
                            if (positionPtr) {
                                smiPrintError(currentParser, ERR_DUPLICATED, "position", (yyvsp[(2) - (3)].text));
                            }
                        }
                        childPtr = childPtr->nextSiblingPtr;
                    }

                    node = addYangNode((yyvsp[(2) - (3)].text), YANG_DECL_POSITION, topNode());
                }
    break;

  case 185:
/* Line 1787 of yacc.c  */
#line 1398 "parser-yang.y"
    {}
    break;

  case 186:
/* Line 1787 of yacc.c  */
#line 1402 "parser-yang.y"
    {}
    break;

  case 187:
/* Line 1787 of yacc.c  */
#line 1405 "parser-yang.y"
    {}
    break;

  case 189:
/* Line 1787 of yacc.c  */
#line 1413 "parser-yang.y"
    {}
    break;

  case 197:
/* Line 1787 of yacc.c  */
#line 1431 "parser-yang.y"
    {
                        node = addCTExtNode(thisModulePtr, YANG_DECL_COMPLEX_TYPE, 0, topNode(), getPrefix((yyvsp[(1) - (2)].text)), getIdentifier((yyvsp[(1) - (2)].text)), (yyvsp[(1) - (2)].text), (yyvsp[(2) - (2)].text));
                        pushNode(node);
                    }
    break;

  case 198:
/* Line 1787 of yacc.c  */
#line 1436 "parser-yang.y"
    {
                        checkUnknownStatement();
                        pop();
                    }
    break;

  case 199:
/* Line 1787 of yacc.c  */
#line 1442 "parser-yang.y"
    {}
    break;

  case 201:
/* Line 1787 of yacc.c  */
#line 1450 "parser-yang.y"
    {}
    break;

  case 214:
/* Line 1787 of yacc.c  */
#line 1480 "parser-yang.y"
    {
                    node = addCTExtNode(thisModulePtr, YANG_DECL_ABSTRACT, 1, topNode(), getPrefix((yyvsp[(1) - (3)].text)), getIdentifier((yyvsp[(1) - (3)].text)), (yyvsp[(1) - (3)].text), "true");
                }
    break;

  case 215:
/* Line 1787 of yacc.c  */
#line 1485 "parser-yang.y"
    {
                    node = addCTExtNode(thisModulePtr, YANG_DECL_ABSTRACT, 1, topNode(), getPrefix((yyvsp[(1) - (3)].text)), getIdentifier((yyvsp[(1) - (3)].text)), (yyvsp[(1) - (3)].text), "false");
                }
    break;

  case 216:
/* Line 1787 of yacc.c  */
#line 1491 "parser-yang.y"
    {
                    node = addCTExtNode(thisModulePtr, YANG_DECL_EXTENDS, 1, topNode(), getPrefix((yyvsp[(1) - (2)].text)), getIdentifier((yyvsp[(1) - (2)].text)), (yyvsp[(1) - (2)].text), (yyvsp[(2) - (2)].text));
                    if (node->export.nodeKind == YANG_DECL_EXTENDS) {
                        createIdentifierRef(node, getPrefix((yyvsp[(2) - (2)].text)), getIdentifier((yyvsp[(2) - (2)].text)));
                    }
                    pushNode(node);
                }
    break;

  case 217:
/* Line 1787 of yacc.c  */
#line 1499 "parser-yang.y"
    {
                     pop();
                }
    break;

  case 218:
/* Line 1787 of yacc.c  */
#line 1504 "parser-yang.y"
    {}
    break;

  case 220:
/* Line 1787 of yacc.c  */
#line 1512 "parser-yang.y"
    {}
    break;

  case 225:
/* Line 1787 of yacc.c  */
#line 1525 "parser-yang.y"
    {
                    node = addCTExtNode(thisModulePtr, YANG_DECL_INSTANCE, 0, topNode(), getPrefix((yyvsp[(1) - (2)].text)), getIdentifier((yyvsp[(1) - (2)].text)), (yyvsp[(1) - (2)].text), (yyvsp[(2) - (2)].text));
                    pushNode(node);
                }
    break;

  case 226:
/* Line 1787 of yacc.c  */
#line 1533 "parser-yang.y"
    {
                    checkUnknownStatement();
                    if (topNode()->export.nodeKind != YANG_DECL_UNKNOWN_STATEMENT && getCardinality(topNode(), YANG_DECL_INSTANCE_TYPE) == 0) {
                        smiPrintErrorAtLine(currentParser, ERR_WRONG_CARDINALITY, topNode()->line, yangDeclAsString(YANG_DECL_INSTANCE_TYPE), "1");
                    }
                    pop();
                }
    break;

  case 227:
/* Line 1787 of yacc.c  */
#line 1542 "parser-yang.y"
    {}
    break;

  case 236:
/* Line 1787 of yacc.c  */
#line 1564 "parser-yang.y"
    {
                    node = addCTExtNode(thisModulePtr, YANG_DECL_INSTANCE_LIST, 0, topNode(), getPrefix((yyvsp[(1) - (2)].text)), getIdentifier((yyvsp[(1) - (2)].text)), (yyvsp[(1) - (2)].text), (yyvsp[(2) - (2)].text));
                    pushNode(node);
                }
    break;

  case 237:
/* Line 1787 of yacc.c  */
#line 1572 "parser-yang.y"
    {
                    checkUnknownStatement();
                    if (topNode()->export.nodeKind != YANG_DECL_UNKNOWN_STATEMENT && getCardinality(topNode(), YANG_DECL_INSTANCE_TYPE) == 0) {
                        smiPrintErrorAtLine(currentParser, ERR_WRONG_CARDINALITY, topNode()->line, yangDeclAsString(YANG_DECL_INSTANCE_TYPE), "1");
                    }
                    pop();
                }
    break;

  case 238:
/* Line 1787 of yacc.c  */
#line 1581 "parser-yang.y"
    {}
    break;

  case 249:
/* Line 1787 of yacc.c  */
#line 1608 "parser-yang.y"
    {
                        node = addCTExtNode(thisModulePtr, YANG_DECL_INSTANCE_TYPE, 1, topNode(), getPrefix((yyvsp[(1) - (3)].text)), getIdentifier((yyvsp[(1) - (3)].text)), (yyvsp[(1) - (3)].text), (yyvsp[(2) - (3)].text));
                        if (node->export.nodeKind == YANG_DECL_INSTANCE_TYPE) {
                            createIdentifierRef(node, getPrefix((yyvsp[(2) - (3)].text)), getIdentifier((yyvsp[(2) - (3)].text)));
                        }
                    }
    break;

  case 250:
/* Line 1787 of yacc.c  */
#line 1620 "parser-yang.y"
    {
                        node = addYangNode((yyvsp[(1) - (1)].text), YANG_DECL_UNKNOWN_STATEMENT, topNode());
                        createIdentifierRef(node, getPrefix((yyvsp[(1) - (1)].text)), getIdentifier((yyvsp[(1) - (1)].text)));
                        pushNode(node);
                    }
    break;

  case 251:
/* Line 1787 of yacc.c  */
#line 1626 "parser-yang.y"
    {
                        pop();
                    }
    break;

  case 252:
/* Line 1787 of yacc.c  */
#line 1631 "parser-yang.y"
    {
                        node = addYangNode((yyvsp[(1) - (2)].text), YANG_DECL_UNKNOWN_STATEMENT, topNode());
                        createIdentifierRef(node, getPrefix((yyvsp[(1) - (2)].text)), getIdentifier((yyvsp[(1) - (2)].text)));
                        node->export.extra = smiStrdup((yyvsp[(2) - (2)].text));
                        pushNode(node);
                    }
    break;

  case 253:
/* Line 1787 of yacc.c  */
#line 1638 "parser-yang.y"
    {
                        pop();
                    }
    break;

  case 254:
/* Line 1787 of yacc.c  */
#line 1644 "parser-yang.y"
    {
                            node = addYangNode((yyvsp[(2) - (2)].text), YANG_DECL_CONTAINER, topNode());
                            pushNode(node);
			}
    break;

  case 255:
/* Line 1787 of yacc.c  */
#line 1649 "parser-yang.y"
    {
                            pop();
			}
    break;

  case 256:
/* Line 1787 of yacc.c  */
#line 1654 "parser-yang.y"
    {}
    break;

  case 258:
/* Line 1787 of yacc.c  */
#line 1662 "parser-yang.y"
    {}
    break;

  case 268:
/* Line 1787 of yacc.c  */
#line 1685 "parser-yang.y"
    {
                    node = addYangNode((yyvsp[(2) - (2)].text), YANG_DECL_MUST, topNode());
                    pushNode(node);
		}
    break;

  case 269:
/* Line 1787 of yacc.c  */
#line 1692 "parser-yang.y"
    {
			pop();
		}
    break;

  case 270:
/* Line 1787 of yacc.c  */
#line 1697 "parser-yang.y"
    {
                    node = addYangNode((yyvsp[(2) - (3)].text), YANG_DECL_MUST, topNode());
		}
    break;

  case 271:
/* Line 1787 of yacc.c  */
#line 1702 "parser-yang.y"
    {}
    break;

  case 277:
/* Line 1787 of yacc.c  */
#line 1717 "parser-yang.y"
    {
                    uniqueNodeKind(topNode(), YANG_DECL_PRESENCE);
                    node = addYangNode((yyvsp[(2) - (3)].text), YANG_DECL_PRESENCE, topNode());
                }
    break;

  case 278:
/* Line 1787 of yacc.c  */
#line 1724 "parser-yang.y"
    {
                    uniqueNodeKind(topNode(), YANG_DECL_CONFIG);
                    setConfig(topNode(), YANG_CONFIG_TRUE);
                    node = addYangNode("true", YANG_DECL_CONFIG, topNode());
                }
    break;

  case 279:
/* Line 1787 of yacc.c  */
#line 1731 "parser-yang.y"
    {
                    uniqueNodeKind(topNode(), YANG_DECL_CONFIG);
                    setConfig(topNode(), YANG_CONFIG_FALSE);
                    node = addYangNode("false", YANG_DECL_CONFIG, topNode());
                }
    break;

  case 280:
/* Line 1787 of yacc.c  */
#line 1739 "parser-yang.y"
    {
                            uniqueNodeKind(topNode(), YANG_DECL_MANDATORY);
                            node = addYangNode("true", YANG_DECL_MANDATORY, topNode());
			}
    break;

  case 281:
/* Line 1787 of yacc.c  */
#line 1745 "parser-yang.y"
    {
                            uniqueNodeKind(topNode(), YANG_DECL_MANDATORY);
                            node = addYangNode("false", YANG_DECL_MANDATORY, topNode());
			}
    break;

  case 282:
/* Line 1787 of yacc.c  */
#line 1752 "parser-yang.y"
    {
                            node = addYangNode((yyvsp[(2) - (2)].text), YANG_DECL_LEAF, topNode());
                            pushNode(node);
			}
    break;

  case 283:
/* Line 1787 of yacc.c  */
#line 1760 "parser-yang.y"
    {
                            if (getCardinality(topNode(), YANG_DECL_TYPE) != 1) {
                                smiPrintError(currentParser, ERR_WRONG_CARDINALITY, yangDeclAsString(YANG_DECL_TYPE), "1");
                            }
                            pop();
			}
    break;

  case 284:
/* Line 1787 of yacc.c  */
#line 1768 "parser-yang.y"
    {}
    break;

  case 294:
/* Line 1787 of yacc.c  */
#line 1791 "parser-yang.y"
    {
                            node = addYangNode((yyvsp[(2) - (2)].text), YANG_DECL_LEAF_LIST, topNode());
                            pushNode(node);
			}
    break;

  case 295:
/* Line 1787 of yacc.c  */
#line 1799 "parser-yang.y"
    {
                            if (getCardinality(topNode(), YANG_DECL_TYPE) != 1) {
				smiPrintError(currentParser, ERR_WRONG_CARDINALITY, yangDeclAsString(YANG_DECL_TYPE), "1");
                            }
                            pop();
			}
    break;

  case 296:
/* Line 1787 of yacc.c  */
#line 1807 "parser-yang.y"
    {}
    break;

  case 307:
/* Line 1787 of yacc.c  */
#line 1832 "parser-yang.y"
    {
                            node = addYangNode((yyvsp[(2) - (2)].text), YANG_DECL_LIST, topNode());
                            pushNode(node);
			}
    break;

  case 308:
/* Line 1787 of yacc.c  */
#line 1840 "parser-yang.y"
    {
                            pop();
			}
    break;

  case 309:
/* Line 1787 of yacc.c  */
#line 1845 "parser-yang.y"
    {}
    break;

  case 323:
/* Line 1787 of yacc.c  */
#line 1876 "parser-yang.y"
    {
                        if (!isNonNegativeInteger((yyvsp[(2) - (3)].text))) {                            
                            smiPrintError(currentParser, ERR_ARG_VALUE, (yyvsp[(2) - (3)].text), "non-negative-integer");
                        }
                        uniqueNodeKind(topNode(), YANG_DECL_MAX_ELEMENTS);
                        node = addYangNode((yyvsp[(2) - (3)].text), YANG_DECL_MAX_ELEMENTS, topNode());
                    }
    break;

  case 324:
/* Line 1787 of yacc.c  */
#line 1886 "parser-yang.y"
    {
                        if (!isNonNegativeInteger((yyvsp[(2) - (3)].text))) {
                            smiPrintError(currentParser, ERR_ARG_VALUE, (yyvsp[(2) - (3)].text), "non-negative-integer");
                        }
                        uniqueNodeKind(topNode(), YANG_DECL_MIN_ELEMENTS);
                        node = addYangNode((yyvsp[(2) - (3)].text), YANG_DECL_MIN_ELEMENTS, topNode());
                    }
    break;

  case 325:
/* Line 1787 of yacc.c  */
#line 1896 "parser-yang.y"
    {
                        if (!strcmp((yyvsp[(2) - (3)].text), "user") || !strcmp((yyvsp[(2) - (3)].text), "system")) {
                        } else {
                            smiPrintError(thisParserPtr, ERR_IVALIDE_ORDERED_BY_VALUE, (yyvsp[(2) - (3)].text));
                        }
                        uniqueNodeKind(topNode(), YANG_DECL_ORDERED_BY);
                        node = addYangNode((yyvsp[(2) - (3)].text), YANG_DECL_ORDERED_BY, topNode());
                    }
    break;

  case 326:
/* Line 1787 of yacc.c  */
#line 1907 "parser-yang.y"
    {
                    uniqueNodeKind(topNode(), YANG_DECL_KEY);
                    node = addYangNode((yyvsp[(2) - (3)].text), YANG_DECL_KEY, topNode());
                    node->info = getKeyList((yyvsp[(2) - (3)].text));
		}
    break;

  case 327:
/* Line 1787 of yacc.c  */
#line 1915 "parser-yang.y"
    {
		    YangList *il;
                    node = addYangNode((yyvsp[(2) - (3)].text), YANG_DECL_UNIQUE, topNode());
                    il = getUniqueList((yyvsp[(2) - (3)].text));
                    node->info = processUniqueList(node, il);
                    freeIdentiferList(il);
		}
    break;

  case 328:
/* Line 1787 of yacc.c  */
#line 1925 "parser-yang.y"
    {
                    node = addYangNode((yyvsp[(2) - (2)].text), YANG_DECL_CHOICE, topNode());
                    pushNode(node);
		}
    break;

  case 329:
/* Line 1787 of yacc.c  */
#line 1930 "parser-yang.y"
    {
                    pop();
		}
    break;

  case 330:
/* Line 1787 of yacc.c  */
#line 1935 "parser-yang.y"
    {}
    break;

  case 331:
/* Line 1787 of yacc.c  */
#line 1940 "parser-yang.y"
    {}
    break;

  case 332:
/* Line 1787 of yacc.c  */
#line 1943 "parser-yang.y"
    {}
    break;

  case 340:
/* Line 1787 of yacc.c  */
#line 1962 "parser-yang.y"
    {
                    node = findChildNodeByTypeAndValue(topNode(), YANG_DECL_CASE, (yyvsp[(2) - (2)].text));
                    if (node) {
                        smiPrintError(currentParser, ERR_DUPLICATED_CASE_IDENTIFIER, (yyvsp[(2) - (2)].text));
                    }
                    node = addYangNode((yyvsp[(2) - (2)].text), YANG_DECL_CASE, topNode());
                    pushNode(node);
		}
    break;

  case 341:
/* Line 1787 of yacc.c  */
#line 1971 "parser-yang.y"
    {
                    pop();
		}
    break;

  case 347:
/* Line 1787 of yacc.c  */
#line 1986 "parser-yang.y"
    {}
    break;

  case 348:
/* Line 1787 of yacc.c  */
#line 1991 "parser-yang.y"
    {}
    break;

  case 349:
/* Line 1787 of yacc.c  */
#line 1994 "parser-yang.y"
    {}
    break;

  case 363:
/* Line 1787 of yacc.c  */
#line 2028 "parser-yang.y"
    {
                    node = addYangNode((yyvsp[(2) - (2)].text), YANG_DECL_GROUPING, topNode());
                    pushNode(node);
		}
    break;

  case 364:
/* Line 1787 of yacc.c  */
#line 2036 "parser-yang.y"
    {
                    pop();
		}
    break;

  case 365:
/* Line 1787 of yacc.c  */
#line 2041 "parser-yang.y"
    {}
    break;

  case 373:
/* Line 1787 of yacc.c  */
#line 2060 "parser-yang.y"
    {
                node = addYangNode((yyvsp[(2) - (2)].text), YANG_DECL_USES, topNode());
                createIdentifierRef(node, getPrefix((yyvsp[(2) - (2)].text)), getIdentifier((yyvsp[(2) - (2)].text)));
                pushNode(node);
            }
    break;

  case 374:
/* Line 1787 of yacc.c  */
#line 2068 "parser-yang.y"
    {
                pop();
            }
    break;

  case 375:
/* Line 1787 of yacc.c  */
#line 2073 "parser-yang.y"
    {
                node = addYangNode((yyvsp[(2) - (3)].text), YANG_DECL_USES, topNode());
                createIdentifierRef(node, getPrefix((yyvsp[(2) - (3)].text)), getIdentifier((yyvsp[(2) - (3)].text)));
            }
    break;

  case 376:
/* Line 1787 of yacc.c  */
#line 2079 "parser-yang.y"
    {}
    break;

  case 385:
/* Line 1787 of yacc.c  */
#line 2101 "parser-yang.y"
    {
                        if (!isDescendantSchemaNodeid((yyvsp[(2) - (2)].text))) {
                            smiPrintError(thisParserPtr, ERR_DESCEDANT_FORM, (yyvsp[(2) - (2)].text));
                        }
                        node = addYangNode((yyvsp[(2) - (2)].text), YANG_DECL_REFINE, topNode());
                        pushNode(node);
                    }
    break;

  case 386:
/* Line 1787 of yacc.c  */
#line 2109 "parser-yang.y"
    {
                        pop();
                    }
    break;

  case 387:
/* Line 1787 of yacc.c  */
#line 2114 "parser-yang.y"
    {}
    break;

  case 388:
/* Line 1787 of yacc.c  */
#line 2119 "parser-yang.y"
    {}
    break;

  case 390:
/* Line 1787 of yacc.c  */
#line 2124 "parser-yang.y"
    {}
    break;

  case 401:
/* Line 1787 of yacc.c  */
#line 2149 "parser-yang.y"
    {
                    if (topDecl() == YANG_DECL_USES || topDecl() == YANG_DECL_INSTANCE || topDecl() == YANG_DECL_INSTANCE_LIST) {
                        if (!isDescendantSchemaNodeid((yyvsp[(2) - (2)].text))) {
                            smiPrintError(thisParserPtr, ERR_DESCEDANT_FORM, (yyvsp[(2) - (2)].text));
                        }
                    } else {
                        if (!isAbsoluteSchemaNodeid((yyvsp[(2) - (2)].text))) {
                            smiPrintError(thisParserPtr, ERR_ABSOLUTE_FORM, (yyvsp[(2) - (2)].text));
                        }
                    }
                    node = addYangNode((yyvsp[(2) - (2)].text), YANG_DECL_AUGMENT, topNode());
                    pushNode(node);
		}
    break;

  case 402:
/* Line 1787 of yacc.c  */
#line 2166 "parser-yang.y"
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
                        smiPrintError(thisParserPtr, ERR_DATADEF_NODE_REQUIRED, (yyvsp[(2) - (7)].text));
                    }

                    pop();
		}
    break;

  case 403:
/* Line 1787 of yacc.c  */
#line 2184 "parser-yang.y"
    {}
    break;

  case 412:
/* Line 1787 of yacc.c  */
#line 2206 "parser-yang.y"
    {
                uniqueNodeKind(topNode(), YANG_DECL_WHEN);
                node = addYangNode((yyvsp[(2) - (3)].text), YANG_DECL_WHEN, topNode());
            }
    break;

  case 413:
/* Line 1787 of yacc.c  */
#line 2213 "parser-yang.y"
    {
                node = addYangNode((yyvsp[(2) - (2)].text), YANG_DECL_RPC, topNode());
                pushNode(node);
            }
    break;

  case 414:
/* Line 1787 of yacc.c  */
#line 2218 "parser-yang.y"
    {
                pop();
            }
    break;

  case 415:
/* Line 1787 of yacc.c  */
#line 2223 "parser-yang.y"
    {}
    break;

  case 416:
/* Line 1787 of yacc.c  */
#line 2228 "parser-yang.y"
    {}
    break;

  case 417:
/* Line 1787 of yacc.c  */
#line 2231 "parser-yang.y"
    {}
    break;

  case 427:
/* Line 1787 of yacc.c  */
#line 2254 "parser-yang.y"
    {
                    node = addYangNode(NULL, YANG_DECL_INPUT, topNode());
                    pushNode(node);
		}
    break;

  case 428:
/* Line 1787 of yacc.c  */
#line 2262 "parser-yang.y"
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
    break;

  case 434:
/* Line 1787 of yacc.c  */
#line 2292 "parser-yang.y"
    {
                    node = addYangNode(NULL, YANG_DECL_OUTPUT, topNode());
                    pushNode(node);
		}
    break;

  case 435:
/* Line 1787 of yacc.c  */
#line 2300 "parser-yang.y"
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
    break;

  case 436:
/* Line 1787 of yacc.c  */
#line 2319 "parser-yang.y"
    {
                    node = addYangNode((yyvsp[(2) - (2)].text), YANG_DECL_NOTIFICATION, topNode());
                    pushNode(node);
                }
    break;

  case 437:
/* Line 1787 of yacc.c  */
#line 2324 "parser-yang.y"
    {
                    pop();
                }
    break;

  case 438:
/* Line 1787 of yacc.c  */
#line 2329 "parser-yang.y"
    {}
    break;

  case 439:
/* Line 1787 of yacc.c  */
#line 2334 "parser-yang.y"
    {}
    break;

  case 440:
/* Line 1787 of yacc.c  */
#line 2338 "parser-yang.y"
    {}
    break;

  case 449:
/* Line 1787 of yacc.c  */
#line 2359 "parser-yang.y"
    {
                        node = addYangNode((yyvsp[(2) - (2)].text), YANG_DECL_DEVIATION, topNode());
                        pushNode(node);
                }
    break;

  case 450:
/* Line 1787 of yacc.c  */
#line 2367 "parser-yang.y"
    {
                    pop();
                }
    break;

  case 459:
/* Line 1787 of yacc.c  */
#line 2390 "parser-yang.y"
    {
                        uniqueNodeKind(topNode(), YANG_DECL_DEVIATE);
                        node = addYangNode("not-supported", YANG_DECL_DEVIATE, topNode());
                    }
    break;

  case 460:
/* Line 1787 of yacc.c  */
#line 2396 "parser-yang.y"
    {
                        uniqueNodeKind(topNode(), YANG_DECL_DEVIATE);
                        node = addYangNode("not-supported", YANG_DECL_DEVIATE, topNode());
                    }
    break;

  case 461:
/* Line 1787 of yacc.c  */
#line 2403 "parser-yang.y"
    {
                        node = addYangNode("add", YANG_DECL_DEVIATE, topNode());
                        pushNode(node);
                    }
    break;

  case 462:
/* Line 1787 of yacc.c  */
#line 2408 "parser-yang.y"
    {
                        pop();
                    }
    break;

  case 463:
/* Line 1787 of yacc.c  */
#line 2413 "parser-yang.y"
    {}
    break;

  case 464:
/* Line 1787 of yacc.c  */
#line 2418 "parser-yang.y"
    {}
    break;

  case 465:
/* Line 1787 of yacc.c  */
#line 2421 "parser-yang.y"
    {}
    break;

  case 475:
/* Line 1787 of yacc.c  */
#line 2443 "parser-yang.y"
    {
                        node = addYangNode("delete", YANG_DECL_DEVIATE, topNode());
                        pushNode(node);
                    }
    break;

  case 476:
/* Line 1787 of yacc.c  */
#line 2448 "parser-yang.y"
    {
                        pop();
                    }
    break;

  case 477:
/* Line 1787 of yacc.c  */
#line 2453 "parser-yang.y"
    {}
    break;

  case 478:
/* Line 1787 of yacc.c  */
#line 2458 "parser-yang.y"
    {}
    break;

  case 479:
/* Line 1787 of yacc.c  */
#line 2461 "parser-yang.y"
    {}
    break;

  case 485:
/* Line 1787 of yacc.c  */
#line 2474 "parser-yang.y"
    {
                        node = addYangNode("replace", YANG_DECL_DEVIATE, topNode());
                        pushNode(node);
                    }
    break;

  case 486:
/* Line 1787 of yacc.c  */
#line 2479 "parser-yang.y"
    {
                        pop();
                    }
    break;

  case 487:
/* Line 1787 of yacc.c  */
#line 2484 "parser-yang.y"
    {}
    break;

  case 488:
/* Line 1787 of yacc.c  */
#line 2489 "parser-yang.y"
    {}
    break;

  case 489:
/* Line 1787 of yacc.c  */
#line 2492 "parser-yang.y"
    {}
    break;

  case 498:
/* Line 1787 of yacc.c  */
#line 2513 "parser-yang.y"
    {
            node = addYangNode((yyvsp[(2) - (2)].text), YANG_DECL_ANYXML, topNode());
            pushNode(node);
		}
    break;

  case 499:
/* Line 1787 of yacc.c  */
#line 2518 "parser-yang.y"
    {
			pop();
		}
    break;

  case 500:
/* Line 1787 of yacc.c  */
#line 2523 "parser-yang.y"
    {}
    break;

  case 501:
/* Line 1787 of yacc.c  */
#line 2528 "parser-yang.y"
    {}
    break;

  case 502:
/* Line 1787 of yacc.c  */
#line 2531 "parser-yang.y"
    {}
    break;

  case 509:
/* Line 1787 of yacc.c  */
#line 2548 "parser-yang.y"
    {
                    node = addYangNode((yyvsp[(2) - (2)].text), YANG_DECL_EXTENSION, topNode());
                    pushNode(node);
		}
    break;

  case 510:
/* Line 1787 of yacc.c  */
#line 2553 "parser-yang.y"
    {
                    pop();
		}
    break;

  case 511:
/* Line 1787 of yacc.c  */
#line 2558 "parser-yang.y"
    {}
    break;

  case 512:
/* Line 1787 of yacc.c  */
#line 2560 "parser-yang.y"
    {}
    break;

  case 513:
/* Line 1787 of yacc.c  */
#line 2563 "parser-yang.y"
    {}
    break;

  case 519:
/* Line 1787 of yacc.c  */
#line 2578 "parser-yang.y"
    {
                        uniqueNodeKind(topNode(), YANG_DECL_ARGUMENT);
                        node = addYangNode((yyvsp[(2) - (2)].text), YANG_DECL_ARGUMENT, topNode());
                        pushNode(node);
                    }
    break;

  case 520:
/* Line 1787 of yacc.c  */
#line 2584 "parser-yang.y"
    {
                        pop();
                    }
    break;

  case 521:
/* Line 1787 of yacc.c  */
#line 2589 "parser-yang.y"
    {}
    break;

  case 522:
/* Line 1787 of yacc.c  */
#line 2591 "parser-yang.y"
    {}
    break;

  case 523:
/* Line 1787 of yacc.c  */
#line 2594 "parser-yang.y"
    {}
    break;

  case 524:
/* Line 1787 of yacc.c  */
#line 2597 "parser-yang.y"
    {
                        addYangNode("true", YANG_DECL_YIN_ELEMENT, topNode());
                    }
    break;

  case 525:
/* Line 1787 of yacc.c  */
#line 2602 "parser-yang.y"
    {
                        addYangNode("false", YANG_DECL_YIN_ELEMENT, topNode());
                    }
    break;

  case 526:
/* Line 1787 of yacc.c  */
#line 2608 "parser-yang.y"
    {
                    uniqueNodeKind(topNode(), YANG_DECL_UNITS);
                    addYangNode((yyvsp[(2) - (3)].text), YANG_DECL_UNITS, topNode());
                }
    break;

  case 527:
/* Line 1787 of yacc.c  */
#line 2615 "parser-yang.y"
    {
                    uniqueNodeKind(topNode(), YANG_DECL_DEFAULT);
                    addYangNode((yyvsp[(2) - (3)].text), YANG_DECL_DEFAULT, topNode());
                }
    break;

  case 539:
/* Line 1787 of yacc.c  */
#line 2644 "parser-yang.y"
    {(yyval.text) = (yyvsp[(1) - (1)].text);}
    break;

  case 540:
/* Line 1787 of yacc.c  */
#line 2644 "parser-yang.y"
    {(yyval.text) = (yyvsp[(1) - (1)].text);}
    break;

  case 541:
/* Line 1787 of yacc.c  */
#line 2644 "parser-yang.y"
    {(yyval.text) = (yyvsp[(1) - (1)].text);}
    break;

  case 542:
/* Line 1787 of yacc.c  */
#line 2644 "parser-yang.y"
    {(yyval.text) = (yyvsp[(1) - (1)].text);}
    break;

  case 543:
/* Line 1787 of yacc.c  */
#line 2644 "parser-yang.y"
    {(yyval.text) = (yyvsp[(1) - (1)].text);}
    break;

  case 544:
/* Line 1787 of yacc.c  */
#line 2644 "parser-yang.y"
    {(yyval.text) = (yyvsp[(1) - (1)].text);}
    break;

  case 545:
/* Line 1787 of yacc.c  */
#line 2644 "parser-yang.y"
    {(yyval.text) = (yyvsp[(1) - (1)].text);}
    break;

  case 546:
/* Line 1787 of yacc.c  */
#line 2644 "parser-yang.y"
    {(yyval.text) = (yyvsp[(1) - (1)].text);}
    break;

  case 547:
/* Line 1787 of yacc.c  */
#line 2645 "parser-yang.y"
    {(yyval.text) = (yyvsp[(1) - (1)].text);}
    break;

  case 548:
/* Line 1787 of yacc.c  */
#line 2645 "parser-yang.y"
    {(yyval.text) = (yyvsp[(1) - (1)].text);}
    break;

  case 549:
/* Line 1787 of yacc.c  */
#line 2645 "parser-yang.y"
    {(yyval.text) = (yyvsp[(1) - (1)].text);}
    break;

  case 550:
/* Line 1787 of yacc.c  */
#line 2645 "parser-yang.y"
    {(yyval.text) = (yyvsp[(1) - (1)].text);}
    break;

  case 551:
/* Line 1787 of yacc.c  */
#line 2645 "parser-yang.y"
    {(yyval.text) = (yyvsp[(1) - (1)].text);}
    break;

  case 552:
/* Line 1787 of yacc.c  */
#line 2645 "parser-yang.y"
    {(yyval.text) = (yyvsp[(1) - (1)].text);}
    break;

  case 553:
/* Line 1787 of yacc.c  */
#line 2645 "parser-yang.y"
    {(yyval.text) = (yyvsp[(1) - (1)].text);}
    break;

  case 554:
/* Line 1787 of yacc.c  */
#line 2645 "parser-yang.y"
    {(yyval.text) = (yyvsp[(1) - (1)].text);}
    break;

  case 555:
/* Line 1787 of yacc.c  */
#line 2646 "parser-yang.y"
    {(yyval.text) = (yyvsp[(1) - (1)].text);}
    break;

  case 556:
/* Line 1787 of yacc.c  */
#line 2646 "parser-yang.y"
    {(yyval.text) = (yyvsp[(1) - (1)].text);}
    break;

  case 557:
/* Line 1787 of yacc.c  */
#line 2646 "parser-yang.y"
    {(yyval.text) = (yyvsp[(1) - (1)].text);}
    break;

  case 558:
/* Line 1787 of yacc.c  */
#line 2646 "parser-yang.y"
    {(yyval.text) = (yyvsp[(1) - (1)].text);}
    break;

  case 559:
/* Line 1787 of yacc.c  */
#line 2646 "parser-yang.y"
    {(yyval.text) = (yyvsp[(1) - (1)].text);}
    break;

  case 560:
/* Line 1787 of yacc.c  */
#line 2646 "parser-yang.y"
    {(yyval.text) = (yyvsp[(1) - (1)].text);}
    break;

  case 561:
/* Line 1787 of yacc.c  */
#line 2646 "parser-yang.y"
    {(yyval.text) = (yyvsp[(1) - (1)].text);}
    break;

  case 562:
/* Line 1787 of yacc.c  */
#line 2646 "parser-yang.y"
    {(yyval.text) = (yyvsp[(1) - (1)].text);}
    break;

  case 563:
/* Line 1787 of yacc.c  */
#line 2647 "parser-yang.y"
    {(yyval.text) = (yyvsp[(1) - (1)].text);}
    break;

  case 564:
/* Line 1787 of yacc.c  */
#line 2647 "parser-yang.y"
    {(yyval.text) = (yyvsp[(1) - (1)].text);}
    break;

  case 565:
/* Line 1787 of yacc.c  */
#line 2647 "parser-yang.y"
    {(yyval.text) = (yyvsp[(1) - (1)].text);}
    break;

  case 566:
/* Line 1787 of yacc.c  */
#line 2647 "parser-yang.y"
    {(yyval.text) = (yyvsp[(1) - (1)].text);}
    break;

  case 567:
/* Line 1787 of yacc.c  */
#line 2647 "parser-yang.y"
    {(yyval.text) = (yyvsp[(1) - (1)].text);}
    break;

  case 568:
/* Line 1787 of yacc.c  */
#line 2647 "parser-yang.y"
    {(yyval.text) = (yyvsp[(1) - (1)].text);}
    break;

  case 569:
/* Line 1787 of yacc.c  */
#line 2647 "parser-yang.y"
    {(yyval.text) = (yyvsp[(1) - (1)].text);}
    break;

  case 570:
/* Line 1787 of yacc.c  */
#line 2647 "parser-yang.y"
    {(yyval.text) = (yyvsp[(1) - (1)].text);}
    break;

  case 571:
/* Line 1787 of yacc.c  */
#line 2648 "parser-yang.y"
    {(yyval.text) = (yyvsp[(1) - (1)].text);}
    break;

  case 572:
/* Line 1787 of yacc.c  */
#line 2648 "parser-yang.y"
    {(yyval.text) = (yyvsp[(1) - (1)].text);}
    break;

  case 573:
/* Line 1787 of yacc.c  */
#line 2648 "parser-yang.y"
    {(yyval.text) = (yyvsp[(1) - (1)].text);}
    break;

  case 574:
/* Line 1787 of yacc.c  */
#line 2648 "parser-yang.y"
    {(yyval.text) = (yyvsp[(1) - (1)].text);}
    break;

  case 575:
/* Line 1787 of yacc.c  */
#line 2648 "parser-yang.y"
    {(yyval.text) = (yyvsp[(1) - (1)].text);}
    break;

  case 576:
/* Line 1787 of yacc.c  */
#line 2648 "parser-yang.y"
    {(yyval.text) = (yyvsp[(1) - (1)].text);}
    break;

  case 577:
/* Line 1787 of yacc.c  */
#line 2648 "parser-yang.y"
    {(yyval.text) = (yyvsp[(1) - (1)].text);}
    break;

  case 578:
/* Line 1787 of yacc.c  */
#line 2648 "parser-yang.y"
    {(yyval.text) = (yyvsp[(1) - (1)].text);}
    break;

  case 579:
/* Line 1787 of yacc.c  */
#line 2648 "parser-yang.y"
    {(yyval.text) = (yyvsp[(1) - (1)].text);}
    break;

  case 580:
/* Line 1787 of yacc.c  */
#line 2649 "parser-yang.y"
    {(yyval.text) = (yyvsp[(1) - (1)].text);}
    break;

  case 581:
/* Line 1787 of yacc.c  */
#line 2649 "parser-yang.y"
    {(yyval.text) = (yyvsp[(1) - (1)].text);}
    break;

  case 582:
/* Line 1787 of yacc.c  */
#line 2649 "parser-yang.y"
    {(yyval.text) = (yyvsp[(1) - (1)].text);}
    break;

  case 583:
/* Line 1787 of yacc.c  */
#line 2649 "parser-yang.y"
    {(yyval.text) = (yyvsp[(1) - (1)].text);}
    break;

  case 584:
/* Line 1787 of yacc.c  */
#line 2649 "parser-yang.y"
    {(yyval.text) = (yyvsp[(1) - (1)].text);}
    break;

  case 585:
/* Line 1787 of yacc.c  */
#line 2649 "parser-yang.y"
    {(yyval.text) = (yyvsp[(1) - (1)].text);}
    break;

  case 586:
/* Line 1787 of yacc.c  */
#line 2649 "parser-yang.y"
    {(yyval.text) = (yyvsp[(1) - (1)].text);}
    break;

  case 587:
/* Line 1787 of yacc.c  */
#line 2649 "parser-yang.y"
    {(yyval.text) = (yyvsp[(1) - (1)].text);}
    break;

  case 588:
/* Line 1787 of yacc.c  */
#line 2649 "parser-yang.y"
    {(yyval.text) = (yyvsp[(1) - (1)].text);}
    break;

  case 589:
/* Line 1787 of yacc.c  */
#line 2650 "parser-yang.y"
    {(yyval.text) = (yyvsp[(1) - (1)].text);}
    break;

  case 590:
/* Line 1787 of yacc.c  */
#line 2650 "parser-yang.y"
    {(yyval.text) = (yyvsp[(1) - (1)].text);}
    break;

  case 591:
/* Line 1787 of yacc.c  */
#line 2650 "parser-yang.y"
    {(yyval.text) = (yyvsp[(1) - (1)].text);}
    break;

  case 592:
/* Line 1787 of yacc.c  */
#line 2650 "parser-yang.y"
    {(yyval.text) = (yyvsp[(1) - (1)].text);}
    break;

  case 593:
/* Line 1787 of yacc.c  */
#line 2650 "parser-yang.y"
    {(yyval.text) = (yyvsp[(1) - (1)].text);}
    break;

  case 594:
/* Line 1787 of yacc.c  */
#line 2650 "parser-yang.y"
    {(yyval.text) = (yyvsp[(1) - (1)].text);}
    break;

  case 595:
/* Line 1787 of yacc.c  */
#line 2650 "parser-yang.y"
    {(yyval.text) = (yyvsp[(1) - (1)].text);}
    break;

  case 596:
/* Line 1787 of yacc.c  */
#line 2650 "parser-yang.y"
    {(yyval.text) = (yyvsp[(1) - (1)].text);}
    break;

  case 597:
/* Line 1787 of yacc.c  */
#line 2651 "parser-yang.y"
    {(yyval.text) = (yyvsp[(1) - (1)].text);}
    break;

  case 598:
/* Line 1787 of yacc.c  */
#line 2651 "parser-yang.y"
    {(yyval.text) = (yyvsp[(1) - (1)].text);}
    break;

  case 599:
/* Line 1787 of yacc.c  */
#line 2651 "parser-yang.y"
    {(yyval.text) = (yyvsp[(1) - (1)].text);}
    break;

  case 600:
/* Line 1787 of yacc.c  */
#line 2651 "parser-yang.y"
    {(yyval.text) = (yyvsp[(1) - (1)].text);}
    break;

  case 601:
/* Line 1787 of yacc.c  */
#line 2651 "parser-yang.y"
    {(yyval.text) = (yyvsp[(1) - (1)].text);}
    break;

  case 602:
/* Line 1787 of yacc.c  */
#line 2651 "parser-yang.y"
    {(yyval.text) = (yyvsp[(1) - (1)].text);}
    break;

  case 603:
/* Line 1787 of yacc.c  */
#line 2651 "parser-yang.y"
    {(yyval.text) = (yyvsp[(1) - (1)].text);}
    break;

  case 604:
/* Line 1787 of yacc.c  */
#line 2651 "parser-yang.y"
    {(yyval.text) = (yyvsp[(1) - (1)].text);}
    break;

  case 605:
/* Line 1787 of yacc.c  */
#line 2652 "parser-yang.y"
    {(yyval.text) = (yyvsp[(1) - (1)].text);}
    break;

  case 606:
/* Line 1787 of yacc.c  */
#line 2652 "parser-yang.y"
    {(yyval.text) = (yyvsp[(1) - (1)].text);}
    break;

  case 607:
/* Line 1787 of yacc.c  */
#line 2652 "parser-yang.y"
    {(yyval.text) = (yyvsp[(1) - (1)].text);}
    break;

  case 608:
/* Line 1787 of yacc.c  */
#line 2652 "parser-yang.y"
    {(yyval.text) = (yyvsp[(1) - (1)].text);}
    break;

  case 609:
/* Line 1787 of yacc.c  */
#line 2652 "parser-yang.y"
    {(yyval.text) = (yyvsp[(1) - (1)].text);}
    break;

  case 610:
/* Line 1787 of yacc.c  */
#line 2652 "parser-yang.y"
    {(yyval.text) = (yyvsp[(1) - (1)].text);}
    break;

  case 611:
/* Line 1787 of yacc.c  */
#line 2656 "parser-yang.y"
    {
                    (yyval.rc) = YANG_BOOLEAN_TRUE;
                }
    break;

  case 612:
/* Line 1787 of yacc.c  */
#line 2661 "parser-yang.y"
    {
                    (yyval.rc) = YANG_BOOLEAN_FALSE;
                }
    break;


/* Line 1787 of yacc.c  */
#line 5132 "parser-yang.c"
      default: break;
    }
  /* User semantic actions sometimes alter yychar, and that requires
     that yytoken be updated with the new translation.  We take the
     approach of translating immediately before every use of yytoken.
     One alternative is translating here after every semantic action,
     but that translation would be missed if the semantic action invokes
     YYABORT, YYACCEPT, or YYERROR immediately after altering yychar or
     if it invokes YYBACKUP.  In the case of YYABORT or YYACCEPT, an
     incorrect destructor might then be invoked immediately.  In the
     case of YYERROR or YYBACKUP, subsequent parser actions might lead
     to an incorrect destructor call or verbose syntax error message
     before the lookahead is translated.  */
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
  /* Make sure we have latest lookahead translation.  See comments at
     user semantic actions for why this is necessary.  */
  yytoken = yychar == YYEMPTY ? YYEMPTY : YYTRANSLATE (yychar);

  /* If not already recovering from an error, report this error.  */
  if (!yyerrstatus)
    {
      ++yynerrs;
#if ! YYERROR_VERBOSE
      yyerror (parserPtr, YY_("syntax error"));
#else
# define YYSYNTAX_ERROR yysyntax_error (&yymsg_alloc, &yymsg, \
                                        yyssp, yytoken)
      {
        char const *yymsgp = YY_("syntax error");
        int yysyntax_error_status;
        yysyntax_error_status = YYSYNTAX_ERROR;
        if (yysyntax_error_status == 0)
          yymsgp = yymsg;
        else if (yysyntax_error_status == 1)
          {
            if (yymsg != yymsgbuf)
              YYSTACK_FREE (yymsg);
            yymsg = (char *) YYSTACK_ALLOC (yymsg_alloc);
            if (!yymsg)
              {
                yymsg = yymsgbuf;
                yymsg_alloc = sizeof yymsgbuf;
                yysyntax_error_status = 2;
              }
            else
              {
                yysyntax_error_status = YYSYNTAX_ERROR;
                yymsgp = yymsg;
              }
          }
        yyerror (parserPtr, yymsgp);
        if (yysyntax_error_status == 2)
          goto yyexhaustedlab;
      }
# undef YYSYNTAX_ERROR
#endif
    }



  if (yyerrstatus == 3)
    {
      /* If just tried and failed to reuse lookahead token after an
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
		      yytoken, &yylval, parserPtr);
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
      if (!yypact_value_is_default (yyn))
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
		  yystos[yystate], yyvsp, parserPtr);
      YYPOPSTACK (1);
      yystate = *yyssp;
      YY_STACK_PRINT (yyss, yyssp);
    }

  YY_IGNORE_MAYBE_UNINITIALIZED_BEGIN
  *++yyvsp = yylval;
  YY_IGNORE_MAYBE_UNINITIALIZED_END


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

#if !defined yyoverflow || YYERROR_VERBOSE
/*-------------------------------------------------.
| yyexhaustedlab -- memory exhaustion comes here.  |
`-------------------------------------------------*/
yyexhaustedlab:
  yyerror (parserPtr, YY_("memory exhausted"));
  yyresult = 2;
  /* Fall through.  */
#endif

yyreturn:
  if (yychar != YYEMPTY)
    {
      /* Make sure we have latest lookahead translation.  See comments at
         user semantic actions for why this is necessary.  */
      yytoken = YYTRANSLATE (yychar);
      yydestruct ("Cleanup: discarding lookahead",
                  yytoken, &yylval, parserPtr);
    }
  /* Do not reclaim the symbols of the rule which action triggered
     this YYABORT or YYACCEPT.  */
  YYPOPSTACK (yylen);
  YY_STACK_PRINT (yyss, yyssp);
  while (yyssp != yyss)
    {
      yydestruct ("Cleanup: popping",
		  yystos[*yyssp], yyvsp, parserPtr);
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


/* Line 2050 of yacc.c  */
#line 2677 "parser-yang.y"


#endif			
