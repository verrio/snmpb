/* A Bison parser, made by GNU Bison 2.3.  */

/* Skeleton interface for Bison's Yacc-like parsers in C

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
/* Line 1489 of yacc.c.  */
#line 217 "parser-sming.tab.h"
	YYSTYPE;
# define yystype YYSTYPE /* obsolescent; will be withdrawn */
# define YYSTYPE_IS_DECLARED 1
# define YYSTYPE_IS_TRIVIAL 1
#endif



