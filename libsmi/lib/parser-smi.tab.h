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
     DOT_DOT = 258,
     COLON_COLON_EQUAL = 259,
     UPPERCASE_IDENTIFIER = 260,
     LOWERCASE_IDENTIFIER = 261,
     NUMBER = 262,
     NEGATIVENUMBER = 263,
     NUMBER64 = 264,
     NEGATIVENUMBER64 = 265,
     BIN_STRING = 266,
     HEX_STRING = 267,
     QUOTED_STRING = 268,
     ACCESS = 269,
     AGENT_CAPABILITIES = 270,
     APPLICATION = 271,
     AUGMENTS = 272,
     BEGIN_ = 273,
     BITS = 274,
     CHOICE = 275,
     CONTACT_INFO = 276,
     CREATION_REQUIRES = 277,
     COUNTER32 = 278,
     COUNTER64 = 279,
     DEFINITIONS = 280,
     DEFVAL = 281,
     DESCRIPTION = 282,
     DISPLAY_HINT = 283,
     END = 284,
     ENTERPRISE = 285,
     EXPORTS = 286,
     EXTENDS = 287,
     FROM = 288,
     GROUP = 289,
     GAUGE32 = 290,
     IDENTIFIER = 291,
     IMPLICIT = 292,
     IMPLIED = 293,
     IMPORTS = 294,
     INCLUDES = 295,
     INDEX = 296,
     INSTALL_ERRORS = 297,
     INTEGER = 298,
     INTEGER32 = 299,
     INTEGER64 = 300,
     IPADDRESS = 301,
     LAST_UPDATED = 302,
     MACRO = 303,
     MANDATORY_GROUPS = 304,
     MAX_ACCESS = 305,
     MIN_ACCESS = 306,
     MODULE = 307,
     MODULE_COMPLIANCE = 308,
     MODULE_IDENTITY = 309,
     NOT_ACCESSIBLE = 310,
     NOTIFICATIONS = 311,
     NOTIFICATION_GROUP = 312,
     NOTIFICATION_TYPE = 313,
     OBJECT = 314,
     OBJECT_GROUP = 315,
     OBJECT_IDENTITY = 316,
     OBJECT_TYPE = 317,
     OBJECTS = 318,
     OCTET = 319,
     OF = 320,
     ORGANIZATION = 321,
     OPAQUE = 322,
     PIB_ACCESS = 323,
     PIB_DEFINITIONS = 324,
     PIB_INDEX = 325,
     PIB_MIN_ACCESS = 326,
     PIB_REFERENCES = 327,
     PIB_TAG = 328,
     POLICY_ACCESS = 329,
     PRODUCT_RELEASE = 330,
     REFERENCE = 331,
     REVISION = 332,
     SEQUENCE = 333,
     SIZE = 334,
     STATUS = 335,
     STRING = 336,
     SUBJECT_CATEGORIES = 337,
     SUPPORTS = 338,
     SYNTAX = 339,
     TEXTUAL_CONVENTION = 340,
     TIMETICKS = 341,
     TRAP_TYPE = 342,
     UNIQUENESS = 343,
     UNITS = 344,
     UNIVERSAL = 345,
     UNSIGNED32 = 346,
     UNSIGNED64 = 347,
     VALUE = 348,
     VARIABLES = 349,
     VARIATION = 350,
     WRITE_SYNTAX = 351
   };
#endif
/* Tokens.  */
#define DOT_DOT 258
#define COLON_COLON_EQUAL 259
#define UPPERCASE_IDENTIFIER 260
#define LOWERCASE_IDENTIFIER 261
#define NUMBER 262
#define NEGATIVENUMBER 263
#define NUMBER64 264
#define NEGATIVENUMBER64 265
#define BIN_STRING 266
#define HEX_STRING 267
#define QUOTED_STRING 268
#define ACCESS 269
#define AGENT_CAPABILITIES 270
#define APPLICATION 271
#define AUGMENTS 272
#define BEGIN_ 273
#define BITS 274
#define CHOICE 275
#define CONTACT_INFO 276
#define CREATION_REQUIRES 277
#define COUNTER32 278
#define COUNTER64 279
#define DEFINITIONS 280
#define DEFVAL 281
#define DESCRIPTION 282
#define DISPLAY_HINT 283
#define END 284
#define ENTERPRISE 285
#define EXPORTS 286
#define EXTENDS 287
#define FROM 288
#define GROUP 289
#define GAUGE32 290
#define IDENTIFIER 291
#define IMPLICIT 292
#define IMPLIED 293
#define IMPORTS 294
#define INCLUDES 295
#define INDEX 296
#define INSTALL_ERRORS 297
#define INTEGER 298
#define INTEGER32 299
#define INTEGER64 300
#define IPADDRESS 301
#define LAST_UPDATED 302
#define MACRO 303
#define MANDATORY_GROUPS 304
#define MAX_ACCESS 305
#define MIN_ACCESS 306
#define MODULE 307
#define MODULE_COMPLIANCE 308
#define MODULE_IDENTITY 309
#define NOT_ACCESSIBLE 310
#define NOTIFICATIONS 311
#define NOTIFICATION_GROUP 312
#define NOTIFICATION_TYPE 313
#define OBJECT 314
#define OBJECT_GROUP 315
#define OBJECT_IDENTITY 316
#define OBJECT_TYPE 317
#define OBJECTS 318
#define OCTET 319
#define OF 320
#define ORGANIZATION 321
#define OPAQUE 322
#define PIB_ACCESS 323
#define PIB_DEFINITIONS 324
#define PIB_INDEX 325
#define PIB_MIN_ACCESS 326
#define PIB_REFERENCES 327
#define PIB_TAG 328
#define POLICY_ACCESS 329
#define PRODUCT_RELEASE 330
#define REFERENCE 331
#define REVISION 332
#define SEQUENCE 333
#define SIZE 334
#define STATUS 335
#define STRING 336
#define SUBJECT_CATEGORIES 337
#define SUPPORTS 338
#define SYNTAX 339
#define TEXTUAL_CONVENTION 340
#define TIMETICKS 341
#define TRAP_TYPE 342
#define UNIQUENESS 343
#define UNITS 344
#define UNIVERSAL 345
#define UNSIGNED32 346
#define UNSIGNED64 347
#define VALUE 348
#define VARIABLES 349
#define VARIATION 350
#define WRITE_SYNTAX 351




#if ! defined YYSTYPE && ! defined YYSTYPE_IS_DECLARED
typedef union YYSTYPE
#line 1288 "parser-smi.y"
{
    char           *text;	  		/* scanned quoted text       */
    char           *id;				/* identifier name           */
    int            err;				/* actually just a dummy     */
    time_t	   date;			/* a date value              */
    Object         *objectPtr;			/* object identifier         */
    SmiStatus      status;			/* a STATUS value            */
    SmiAccess      access;			/* an ACCESS value           */
    Type           *typePtr;
    List           *listPtr;			/* SEQUENCE and INDEX lists  */
    NamedNumber    *namedNumberPtr;		/* BITS or enum item         */
    Range          *rangePtr;			/* type restricting range    */
    SmiValue	   *valuePtr;
    SmiUnsigned32  unsigned32;			/*                           */
    SmiInteger32   integer32;			/*                           */
    SmiUnsigned64  unsigned64;			/*                           */
    SmiInteger64   integer64;			/*                           */
    struct Compl   compl;
    struct Index   index;
    Module	   *modulePtr;
    SubjectCategories *subjectCategoriesPtr;
}
/* Line 1489 of yacc.c.  */
#line 264 "parser-smi.tab.h"
	YYSTYPE;
# define yystype YYSTYPE /* obsolescent; will be withdrawn */
# define YYSTYPE_IS_DECLARED 1
# define YYSTYPE_IS_TRIVIAL 1
#endif



