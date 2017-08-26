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
/* Line 2053 of yacc.c  */
#line 276 "parser-yang.y"

    int             rc;                 /* >=0: ok, <0: error    */
    char            *text;


/* Line 2053 of yacc.c  */
#line 154 "parser-yang.tab.h"
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
