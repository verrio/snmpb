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
/* Line 1285 of yacc.c.  */
#line 197 "parser-sming.tab.h"
# define yystype YYSTYPE /* obsolescent; will be withdrawn */
# define YYSTYPE_IS_DECLARED 1
# define YYSTYPE_IS_TRIVIAL 1
#endif





