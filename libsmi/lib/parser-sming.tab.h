/* A Bison parser, made by GNU Bison 2.7.12-4996.  */

/* Bison interface for Yacc-like parsers in C
   
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

#ifndef YY_SMING_PARSER_SMING_TAB_H_INCLUDED
# define YY_SMING_PARSER_SMING_TAB_H_INCLUDED
/* Enabling traces.  */
#ifndef YYDEBUG
# define YYDEBUG 1
#endif
#if YYDEBUG
extern int smingdebug;
#endif

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


#if ! defined YYSTYPE && ! defined YYSTYPE_IS_DECLARED
typedef union YYSTYPE
{
/* Line 2053 of yacc.c  */
#line 359 "parser-sming.y"

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


/* Line 2053 of yacc.c  */
#line 155 "parser-sming.tab.h"
} YYSTYPE;
# define YYSTYPE_IS_TRIVIAL 1
# define yystype YYSTYPE /* obsolescent; will be withdrawn */
# define YYSTYPE_IS_DECLARED 1
#endif


#ifdef YYPARSE_PARAM
#if defined __STDC__ || defined __cplusplus
int smingparse (void *YYPARSE_PARAM);
#else
int smingparse ();
#endif
#else /* ! YYPARSE_PARAM */
#if defined __STDC__ || defined __cplusplus
int smingparse (struct Parser *parserPtr);
#else
int smingparse ();
#endif
#endif /* ! YYPARSE_PARAM */

#endif /* !YY_SMING_PARSER_SMING_TAB_H_INCLUDED  */
