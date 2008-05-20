/*
 * scanner-sming.l --
 *
 *      Lexical rules for scanning the SMIng MIB module language.
 *
 * Copyright (c) 1999 Frank Strauss, Technical University of Braunschweig.
 *
 * See the file "COPYING" for information on usage and redistribution
 * of this file, and for a DISCLAIMER OF ALL WARRANTIES.
 *
 * @(#) $Id: scanner-sming.l 7734 2008-02-15 07:49:14Z schoenw $
 */

%option noyywrap

%{

#include <config.h>
    
#ifdef BACKEND_SMING

#include <stdio.h>
#include <string.h>
#include <ctype.h>
#include <errno.h>

#if defined(HAVE_WIN_H)
#include "win.h"
#endif

#include "error.h"
#include "util.h"
#include "parser-sming.h"
#include "parser-sming.tab.h"
#include "scanner-sming.h"

#ifdef HAVE_DMALLOC_H
#include <dmalloc.h>
#endif



/* we need a reentrant parser, so yylex gets arguments */
#if 0
#define YY_DECL int yylex YY_PROTO((YYSTYPE *lvalp, void *parser))
#else
#define YY_DECL int yylex(YYSTYPE *lvalp, void *parser)
#endif



#define thisParser (*(Parser *) parser)



#define MAX_NUMBER		"18446744073709551615" /* max Counter64 */



/*
 * This makes the usual notation when referencing attributes also
 * work with our pure parser code.
 */
#define yylval (*lvalp)


static YY_BUFFER_STATE yybuffer[MAX_LEX_DEPTH];

static int lexDepth = 0; 

 
int
smingEnterLexRecursion(file)
    FILE *file;
{
    if (lexDepth >= MAX_LEX_DEPTH) {
	return (-1);
    }
    yybuffer[lexDepth++] = YY_CURRENT_BUFFER;
    yy_switch_to_buffer(yy_create_buffer(file, YY_BUF_SIZE));
    return (lexDepth);
}


 
void
smingLeaveLexRecursion()
{    
    yy_delete_buffer(YY_CURRENT_BUFFER);
    yy_switch_to_buffer(yybuffer[--lexDepth]);
}



%}



/*
 * Lex pattern definitions.
 */
delim		([^a-zA-Z0-9-])
lineBreak       ("\n"|"\n\015"|"\015\n")
WSP		(" "|"\t")
VCHAR		([\041-\176])
HTAB		"\t"
SP		" "


/*
 * Lex state definitions.
 */
%s		Skipline

%%



 /*
  * Lex rules for comments. Do you use {lineBreak} here because it
  * introduces a trailing context which is (a) slow and (b) causes
  * REJECT to be used.
  */

<INITIAL>"//"({WSP}|{VCHAR})*/"\n" {
}

<INITIAL>"//"({WSP}|{VCHAR})*/"\n\015" {
}

<INITIAL>"//"({WSP}|{VCHAR})*/"\015\n" {
}

 /*
  * Lex rules for some special tokens.
  */

<INITIAL>[\{\}\(\)\;\,\-\.\|] {
    return yytext[0];
}
    
<INITIAL>".." {
    return DOT_DOT;
}

<INITIAL>"."/[^\.] {
    return DOT;
}

<INITIAL>"::" {
    return COLON_COLON;
}

 /*
  * Lex rules for separators.
  */

<INITIAL>{lineBreak} {
    thisParser.line++;
}

<INITIAL>{WSP} {
}

 /*
  * Lex rules for known keywords.
  */

<INITIAL>module/{delim} {
    yylval.id = yytext;
    return moduleKeyword;
}

<INITIAL>import/{delim} {
    yylval.id = yytext;
    return importKeyword;
}

<INITIAL>revision/{delim} {
    yylval.id = yytext;
    return revisionKeyword;
}

<INITIAL>date/{delim} {
    yylval.id = yytext;
    return dateKeyword;
}

<INITIAL>organization/{delim} {
    yylval.id = yytext;
    return organizationKeyword;
}

<INITIAL>contact/{delim} {
    yylval.id = yytext;
    return contactKeyword;
}

<INITIAL>description/{delim} {
    yylval.id = yytext;
    return descriptionKeyword;
}

<INITIAL>reference/{delim} {
    yylval.id = yytext;
    return referenceKeyword;
}

<INITIAL>extension/{delim} {
    yylval.id = yytext;
    return extensionKeyword;
}

<INITIAL>typedef/{delim} {
    yylval.id = yytext;
    return typedefKeyword;
}

<INITIAL>type/{delim} {
    yylval.id = yytext;
    return typeKeyword;
}

<INITIAL>parent/{delim} {
    yylval.id = yytext;
    return parentKeyword;
}

<INITIAL>identity/{delim} {
    yylval.id = yytext;
    return identityKeyword;
}

<INITIAL>class/{delim} {
    yylval.id = yytext;
    return classKeyword;
}

<INITIAL>extends/{delim} {
    yylval.id = yytext;
    return extendsKeyword;
}

<INITIAL>attribute/{delim} {
    yylval.id = yytext;
    return attributeKeyword;
}

<INITIAL>unique/{delim} {
    yylval.id = yytext;
    return uniqueKeyword;
}

<INITIAL>event/{delim} {
    yylval.id = yytext;
    return eventKeyword;
}

<INITIAL>format/{delim} {
    yylval.id = yytext;
    return formatKeyword;
}

<INITIAL>units/{delim} {
    yylval.id = yytext;
    return unitsKeyword;
}

<INITIAL>status/{delim} {
    yylval.id = yytext;
    return statusKeyword;
}

<INITIAL>access/{delim} {
    yylval.id = yytext;
    return accessKeyword;
}

<INITIAL>default/{delim} {
    yylval.id = yytext;
    return defaultKeyword;
}

<INITIAL>abnf/{delim} {
    yylval.id = yytext;
    return abnfKeyword;
}


 /*
  *Base types keywords
  */
<INITIAL>OctetString/{delim} {
    yylval.id = yytext;
    return OctetStringKeyword;
}

<INITIAL>Pointer/{delim} {
    yylval.id = yytext;
    return PointerKeyword;
}

<INITIAL>ObjectIdentifier/{delim} {
    yylval.id = yytext;
    return ObjectIdentifierKeyword;
}

<INITIAL>Integer32/{delim} {
    yylval.id = yytext;
    return Integer32Keyword;
}

<INITIAL>Integer64/{delim} {
    yylval.id = yytext;
    return Integer64Keyword;
}

<INITIAL>Unsigned32/{delim} {
    yylval.id = yytext;
    return Unsigned32Keyword;
}

<INITIAL>Unsigned64/{delim} {
    yylval.id = yytext;
    return Unsigned64Keyword;
}

<INITIAL>Float32/{delim} {
    yylval.id = yytext;
    return Float32Keyword;
}

<INITIAL>Float64/{delim} {
    yylval.id = yytext;
    return Float64Keyword;
}

<INITIAL>Float128/{delim} {
    yylval.id = yytext;
    return Float128Keyword;
}

<INITIAL>Bits/{delim} {
    yylval.id = yytext;
    return BitsKeyword;
}

<INITIAL>Enumeration/{delim} {
    yylval.id = yytext;
    return EnumerationKeyword;
}

 /*
  *Status keywords
  */

<INITIAL>current/{delim} {
    yylval.id = yytext;
    return currentKeyword;
}

<INITIAL>deprecated/{delim} {
    yylval.id = yytext;
    return deprecatedKeyword;
}

<INITIAL>obsolete/{delim} {
    yylval.id = yytext;
    return obsoleteKeyword;
}

 /*
  *Access keywords
  */

<INITIAL>eventonly/{delim} {
    yylval.id = yytext;
    return eventonlyKeyword;
}

<INITIAL>readonly/{delim} {
    yylval.id = yytext;
    return readonlyKeyword;
}

<INITIAL>readwrite/{delim} {
    yylval.id = yytext;
    return readwriteKeyword;
}

 /*
  *Special floating point values' keywords
  */

<INITIAL>neginf/{delim} {
    yylval.id = yytext;
    return neginfKeyword;
}

<INITIAL>posinf/{delim} {
    yylval.id = yytext;
    return posinfKeyword;
}

<INITIAL>snan/{delim} {
    yylval.id = yytext;
    return snanKeyword;
}

<INITIAL>qnan/{delim} {
    yylval.id = yytext;
    return qnanKeyword;
}



 /*
  * Lex rules for identifiers.
  */

 /* e.g. module names: REF: draft,p.12-13 */
<INITIAL>[A-Z](-?[a-zA-Z0-9_]+)*-? {
    if (yytext[yyleng-1] == '-') {
	smiPrintError(parser, ERR_ID_ENDS_IN_HYPHEN, yytext);
    }
    if (yyleng > 64) {
	smiPrintError(parser, ERR_UCIDENTIFIER_64, yytext);
    }
    if (strchr(yytext, '_')) {
        smiPrintError(parser, ERR_UNDERSCORE_IN_IDENTIFIER, yytext);
    }
    yylval.text = smiStrdup(yytext);
    return ucIdentifier;
}

 /* same for lowercase names */
<INITIAL>[a-z](-?[a-zA-Z0-9_]+)*-? {
    if (yytext[yyleng-1] == '-') {
	smiPrintError(parser, ERR_ID_ENDS_IN_HYPHEN, yytext);
    }
    if (yyleng > 64) {
	smiPrintError(parser, ERR_LCIDENTIFIER_64, yytext);
    }
    if (strchr(yytext, '_')) {
        smiPrintError(parser, ERR_UNDERSCORE_IN_IDENTIFIER, yytext);
    }
    yylval.text = smiStrdup(yytext);
    return lcIdentifier;
}

 /*
  * Lex rules for numbers.
  *
  * NOTE: `-' is a separate token. Hence, there are no negative numbers.
  */

<INITIAL>0+/[0-9] {
    smiPrintError(parser, ERR_LEADING_ZEROS);
}

<INITIAL>([1-9][0-9]*|0)/[^0-9] {
    if ((yyleng > sizeof(MAX_NUMBER)-1) ||
	((yyleng == sizeof(MAX_NUMBER)-1) &&
	 (strcmp(yytext, MAX_NUMBER) > 0))) {
	smiPrintError(parser, ERR_NUMBER_TOO_LARGE, yytext);
    }
    yylval.text = yytext;
    return decimalNumber;
}

<INITIAL>0x[0-9a-fA-F]+/[^0-9a-fA-F] {
    if (yyleng % 2) {
	smiPrintError(parser, ERR_HEX_STRING_MUL2, yytext);
    }
    yylval.text = yytext;
    return hexadecimalNumber;
}
 /*
  * Lex rules for floating point values.
  */

<INITIAL>([0-9]*\.[0-9]+([eE][+-]?[0-9]+)?)/[^0-9] {
    yylval.text = yytext;
    return floatValue;
}

 /*
  * Lex rules for OID's.
  */
  
<INITIAL>([0-9A-Za-z]-?)+\.([0-9A-Za-z]-?)+\.([0-9A-Za-z]-?)+(\.([0-9A-Za-z]-?)+)*/([^0-9A-Za-z]) {
    yylval.text = yytext;
    return OID;
}


 /*
  * Lex rules for textSegments.
  */

<INITIAL>\"([^\"]|(\\\"))*\" {
    char *s, *d;		/* the source and destination pointer */
    int column = 0;		/* the current column */
    int newlineflag = 0;	/* we have just passed a newline */
    int cutoffcolumn = 0;	/* cut off white space up to this column */
				/* (computed by caculating the indentation */
				/* of the first column) */

    yytext[yyleng-1] = '\0';
    for (d = yytext, s = yytext+1; s[0]; s++, d++) {

	if ((s[0] == '\n' && s[1] == '\r')	/* newline sequence */
	    || (s[0] == '\r' && s[1] == '\n')) {
	    thisParser.line += 1;
	    d[0] = '\n';
	    s++;
	    newlineflag = 1;
	    column = 0;
	    if (cutoffcolumn < 0) cutoffcolumn = 0;
	} else if (s[0] == '\n') {		/* simple newline */
	    thisParser.line += 1;
	    d[0] = '\n';
	    newlineflag = 1;
	    column = 0;
	    if (cutoffcolumn < 0) cutoffcolumn = 0;
	} else {
	    if (newlineflag && isspace((int)(unsigned char)s[0])) {	/* space after newline */
		if (cutoffcolumn <= 0) {
		    cutoffcolumn -= (s[0] == '\t') ? (8-((column-1) % 8)) : 1;
		}
		column += (s[0] == '\t') ? (8-((column-1) % 8)) : 1;
		if (cutoffcolumn <= 0 || column <= cutoffcolumn) {
		    d--;
		} else {
		    d[0] = s[0];
		    newlineflag = 0;
		}
	    } else {				/* everything else */
		if (! isascii(s[0])) {
		    smiPrintError(parser, ERR_ILLEGAL_CHAR_IN_STRING,
				  s[0], (unsigned char) s[0]);
		}
		d[0] = s[0];
		newlineflag = 0;
		if (cutoffcolumn < 0) {
		    cutoffcolumn *= -1;
		}
	    }
	}
    }
    d[0] = '\0';

    yylval.text = yytext;
    return textSegment;
}

 /*
  * Skip the remainder of the line
  */
<Skipline>.*{lineBreak} {
    thisParser.line++;
    BEGIN(INITIAL);
}

 /*
  * Everything else...
  */

. {
    smiPrintError(parser, ERR_LEX_UNEXPECTED_CHAR);
    BEGIN(Skipline);
}

%%

#endif
