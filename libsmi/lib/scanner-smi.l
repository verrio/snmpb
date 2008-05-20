/*
 * scanner-smi.l --
 *
 *      Lexical rules for scanning the SMIv1/v2 MIB module language.
 *
 * Copyright (c) 1999 Frank Strauss, Technical University of Braunschweig.
 *
 * See the file "COPYING" for information on usage and redistribution
 * of this file, and for a DISCLAIMER OF ALL WARRANTIES.
 *
 * @(#) $Id: scanner-smi.l 6539 2007-02-01 08:23:54Z strauss $
 */

%option noyywrap

%{

#include <config.h>
    

#include <stdio.h>
#include <string.h>
#include <ctype.h>
#include <errno.h>

#if defined(HAVE_WIN_H)
#include "win.h"
#endif

#include "error.h"
#include "util.h"
#include "parser-smi.h"
#include "parser-smi.tab.h"
#include "scanner-smi.h"

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


#define MAX_UNSIGNED64		"18446744073709551615"
#define MIN_UNSIGNED64          0
#define MAX_UNSIGNED32		4294967295
#define MIN_UNSIGNED32		0
#define MAX_INTEGER32		2147483647
#define MIN_INTEGER32		-2147483648



/*
 * This makes the usual notation when referencing attributes also
 * work with our pure parser code.
 */
#define yylval (*lvalp)


static YY_BUFFER_STATE yybuffer[MAX_LEX_DEPTH];

static int lexDepth = 0; 

 
int
smiEnterLexRecursion(file)
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
smiLeaveLexRecursion()
{    
    yy_delete_buffer(YY_CURRENT_BUFFER);
    yy_switch_to_buffer(yybuffer[--lexDepth]);
}



%}



/*
 * Lex pattern definitions.
 */
delim		([^a-zA-Z0-9-])|--
enddelim	([^a-zA-Z0-9-])|--|""
eol             ("\n"|"\n\015"|"\015\n"|"\015")



/*
 * Lex state definitions.
 */
%s		Macro
%s		Choice
%s		Exports
%s		Comment
%s		Skipline



%%



 /*
  * Lex rules.
  */

 /*
  * Lex rules for skipping MACRO.
  */

<INITIAL>MACRO {
    BEGIN(Macro);
    yylval.id = yytext;
    return MACRO;
}

<Macro>{eol} {
    thisParser.line++;
}

<Macro>. {
}

<Macro>END/{delim} {
    BEGIN(INITIAL);
    yylval.id = yytext;
    return END;
}

 /*
  * Lex rules for skipping EXPORTS.
  */

<INITIAL>EXPORTS {
    BEGIN(Exports);
    yylval.id = yytext;
    return EXPORTS;
}

<Exports>{eol} {
    thisParser.line++;
}

<Exports>[^\;]* {
}

<Exports>\; {
    BEGIN(INITIAL);
    return yytext[0];
}

 /*
  * Lex rules for skipping CHOICE.
  */

<INITIAL>CHOICE {
    BEGIN(Choice);
    yylval.id = yytext;
    return CHOICE;
}

<Choice>{eol} {
    thisParser.line++;
}

<Choice>[^\}] {
}

<Choice>\} {
    BEGIN(INITIAL);
    return yytext[0];
}

 /*
  * Lex rules for comments.
  */

<INITIAL>"--" {
    BEGIN(Comment);
}

<Comment>"--" {
    thisParser.lcline = thisParser.line;
    BEGIN(INITIAL);
}

 /* Special-case a common error (dashed seperator lines with
  * 4n+1 dashes, meaning a single dash outside the comment).
  * Print the "lexically unexpected" error that you would normally
  * get, and stick in the comment FYI also.
  */
<Comment>"---"{eol} {
    smiPrintError(parser, ERR_COMMENT_TERMINATES);
    smiPrintError(parser, ERR_LEX_UNEXPECTED_CHAR);
    thisParser.line++;
    BEGIN(INITIAL);
}
    
<Comment>{eol} {
    thisParser.line++;
    BEGIN(INITIAL);
}

<Comment>[^-\n\015]* {
}
<Comment>"-" {
}

 /*
  * Lex rules for some special tokens.
  */

<INITIAL>[\[\]\{\}\(\)\:\;\,\-\.\|] {
    return yytext[0];
}
    
<INITIAL>".." {
    return DOT_DOT;
}

<INITIAL>"::=" {
    return COLON_COLON_EQUAL;
}

 /*
  * Lex rules for white space.
  */

<INITIAL>[ \t]* {
}

<INITIAL>{eol} {
    thisParser.line++;
}

 /*
  * Lex rules for known keywords.
  */

 /* SMI(ng) specific */
<INITIAL>ACCESS/{delim} {
    yylval.id = yytext;
    return ACCESS;
}

 /* SMI(ng) specific */
<INITIAL>AGENT-CAPABILITIES/{delim} {
    yylval.id = yytext;
    return AGENT_CAPABILITIES;
}

<INITIAL>APPLICATION/{delim} {
    yylval.id = yytext;
    return APPLICATION;
}

<INITIAL>AUGMENTS/{delim} {
    yylval.id = yytext;
    return AUGMENTS;
}

<INITIAL>BEGIN/{delim} {
    yylval.id = yytext;
    return BEGIN_;
}

<INITIAL>BITS/{delim} {
    yylval.id = yytext;
    return BITS;
}

<INITIAL>CONTACT-INFO/{delim} {
    yylval.id = yytext;
    return CONTACT_INFO;
}

 /* SMI(ng) specific */
<INITIAL>CREATION-REQUIRES/{delim} {
    yylval.id = yytext;
    return CREATION_REQUIRES;
}

 /* SMI(ng) specific */
<INITIAL>Counter32/{delim} {
    yylval.id = yytext;
    return COUNTER32;
}

 /* SMI(ng) specific */
<INITIAL>Counter64/{delim} {
    yylval.id = yytext;
    return COUNTER64;
}

 /* SMI(ng) specific */
<INITIAL>DEFINITIONS/{delim} {
    yylval.id = yytext;
    return DEFINITIONS;
}

<INITIAL>DEFVAL/{delim} {
    yylval.id = yytext;
    return DEFVAL;
}

<INITIAL>DESCRIPTION/{delim} {
    yylval.id = yytext;
    return DESCRIPTION;
}

<INITIAL>DISPLAY-HINT/{delim} {
    yylval.id = yytext;
    return DISPLAY_HINT;
}

<INITIAL>END/{enddelim} {
    yylval.id = yytext;
    return END;
}

 /* SMI(ng) specific */
<INITIAL>ENTERPRISE/{delim} {
    yylval.id = yytext;
    return ENTERPRISE;
}

 /* SPPI specific */
<INITIAL>EXTENDS/{delim} {
    yylval.id = yytext;
    return EXTENDS;
}

<INITIAL>FROM/{delim} {
    yylval.id = yytext;
    return FROM;
}

<INITIAL>GROUP/{delim} {
    yylval.id = yytext;
    return GROUP;
}

 /* SMI(ng) specific */
<INITIAL>Gauge32/{delim} {
    yylval.id = yytext;
    return GAUGE32;
}

<INITIAL>IDENTIFIER/{delim} {
    yylval.id = yytext;
    return IDENTIFIER;
}

<INITIAL>IMPLICIT/{delim} {
    yylval.id = yytext;
    return IMPLICIT;
}

<INITIAL>IMPLIED/{delim} {
    yylval.id = yytext;
    return IMPLIED;
}

<INITIAL>IMPORTS/{delim} {
    yylval.id = yytext;
    return IMPORTS;
}

 /* SMI(ng) specific */
<INITIAL>INCLUDES/{delim} {
    yylval.id = yytext;
    return INCLUDES;
}

<INITIAL>INDEX/{delim} {
    yylval.id = yytext;
    return INDEX;
}

 /* SPPI specific */
<INITIAL>INSTALL-ERRORS/{delim} {
  yylval.id = yytext;
  return INSTALL_ERRORS;
}

<INITIAL>INTEGER/{delim} {
    yylval.id = yytext;
    return INTEGER;
}

<INITIAL>Integer32/{delim} {
    yylval.id = yytext;
    return INTEGER32;
}

 /* SPPI specific */
<INITIAL>Integer64/{delim} {
  yylval.id = yytext;
  return INTEGER64;
}

<INITIAL>IpAddress/{delim} {
    yylval.id = yytext;
    return IPADDRESS;
}

<INITIAL>LAST-UPDATED/{delim} {
    yylval.id = yytext;
    return LAST_UPDATED;
}

<INITIAL>MANDATORY-GROUPS/{delim} {
    yylval.id = yytext;
    return MANDATORY_GROUPS;
}

 /* SMI(ng) specific */
<INITIAL>MAX-ACCESS/{delim} {
    yylval.id = yytext;
    return MAX_ACCESS;
}

 /* SMI(ng) specific */
<INITIAL>MIN-ACCESS/{delim} {
    yylval.id = yytext;
    return MIN_ACCESS;
}

<INITIAL>MODULE/{delim} {
    yylval.id = yytext;
    return MODULE;
}

<INITIAL>MODULE-COMPLIANCE/{delim} {
    yylval.id = yytext;
    return MODULE_COMPLIANCE;
}

<INITIAL>MODULE-IDENTITY/{delim} {
    yylval.id = yytext;
    return MODULE_IDENTITY;
}

 /* SMI(ng) specific */
<INITIAL>NOTIFICATION-GROUP/{delim} {
    yylval.id = yytext;
    return NOTIFICATION_GROUP;
}

 /* SMI(ng) specific */
<INITIAL>NOTIFICATION-TYPE/{delim} {
    yylval.id = yytext;
    return NOTIFICATION_TYPE;
}

 /* SMI(ng) specific */
<INITIAL>NOTIFICATIONS/{delim} {
    yylval.id = yytext;
    return NOTIFICATIONS;
}

<INITIAL>OBJECT/{delim} {
    yylval.id = yytext;
    return OBJECT;
}

<INITIAL>OBJECT-GROUP/{delim} {
    yylval.id = yytext;
    return OBJECT_GROUP;
}

<INITIAL>OBJECT-IDENTITY/{delim} {
    yylval.id = yytext;
    return OBJECT_IDENTITY;
}

<INITIAL>OBJECT-TYPE/{delim} {
    yylval.id = yytext;
    return OBJECT_TYPE;
}

<INITIAL>OBJECTS/{delim} {
    yylval.id = yytext;
    return OBJECTS;
}

<INITIAL>OCTET/{delim} {
    yylval.id = yytext;
    return OCTET;
}

<INITIAL>OF/{delim} {
    yylval.id = yytext;
    return OF;
}

<INITIAL>ORGANIZATION/{delim} {
    yylval.id = yytext;
    return ORGANIZATION;
}

<INITIAL>Opaque/{delim} {
    yylval.id = yytext;
    return OPAQUE;
}

 /* SPPI specific */
<INITIAL>PIB-ACCESS/{delim} {
  yylval.id = yytext;
  return PIB_ACCESS;
}

 /* SPPI specific */
<INITIAL>PIB-DEFINITIONS/{delim} {
    yylval.id = yytext;
    return PIB_DEFINITIONS;
}

 /* SPPI specific */
<INITIAL>PIB-INDEX/{delim} {
  yylval.id = yytext;
  return PIB_INDEX;
}

 /* SPPI specific */
<INITIAL>PIB-MIN-ACCESS/{delim} {
  yylval.id = yytext;
  return PIB_MIN_ACCESS;
}

 /* SPPI specific */
<INITIAL>PIB-REFERENCES/{delim} {
  yylval.id = yytext;
  return PIB_REFERENCES;
}

 /* SPPI specific */
<INITIAL>PIB-TAG/{delim} {
  yylval.id = yytext;
  return PIB_TAG;
}

 /* SPPI specific */
<INITIAL>POLICY-ACCESS/{delim} {
  yylval.id = yytext;
  return POLICY_ACCESS;
}

 /* SMI(ng) specific */
<INITIAL>PRODUCT-RELEASE/{delim} {
    yylval.id = yytext;
    return PRODUCT_RELEASE;
}

<INITIAL>REFERENCE/{delim} {
    yylval.id = yytext;
    return REFERENCE;
}

<INITIAL>REVISION/{delim} {
    yylval.id = yytext;
    return REVISION;
}

<INITIAL>SEQUENCE/{delim} {
    yylval.id = yytext;
    return SEQUENCE;
}

<INITIAL>SIZE/{delim} {
    yylval.id = yytext;
    return SIZE;
}

<INITIAL>STATUS/{delim} {
    yylval.id = yytext;
    return STATUS;
}

<INITIAL>STRING/{delim} {
    yylval.id = yytext;
    return STRING;
}

 /* SPPI specific */
<INITIAL>SUBJECT-CATEGORIES/{delim} {
  yylval.id = yytext;
  return SUBJECT_CATEGORIES;
}

 /* SMI(ng) specific */
<INITIAL>SUPPORTS/{delim} {
    yylval.id = yytext;
    return SUPPORTS;
}

<INITIAL>SYNTAX/{delim} {
    yylval.id = yytext;
    return SYNTAX;
}

<INITIAL>TEXTUAL-CONVENTION/{delim} {
    yylval.id = yytext;
    return TEXTUAL_CONVENTION;
}

<INITIAL>TimeTicks/{delim} {
    yylval.id = yytext;
    return TIMETICKS;
}

 /* SMI(ng) specific */
<INITIAL>TRAP-TYPE/{delim} {
    yylval.id = yytext;
    return TRAP_TYPE;
}

 /* SPPI specific */
<INITIAL>UNIQUENESS/{delim} {
  yylval.id = yytext;
  return UNIQUENESS;
}

<INITIAL>UNITS/{delim} {
    yylval.id = yytext;
    return UNITS;
}

 /* SMI(ng) specific */
<INITIAL>UNIVERSAL/{delim} {
    yylval.id = yytext;
    return UNIVERSAL;
}

<INITIAL>Unsigned32/{delim} {
    yylval.id = yytext;
    return UNSIGNED32;
}

 /* SPPI specific */
<INITIAL>Unsigned64/{delim} {
  yylval.id = yytext;
  return UNSIGNED64;
}

 /* SPPI specific */
<INITIAL>VALUE/{delim} {
  yylval.id = yytext;
  return VALUE;
}

 /* SMI(ng) specific */
<INITIAL>VARIABLES/{delim} {
    yylval.id = yytext;
    return VARIABLES;
}

 /* SMI(ng) specific */
<INITIAL>VARIATION/{delim} {
    yylval.id = yytext;
    return VARIATION;
}

 /* SMI(ng) specific */
<INITIAL>WRITE-SYNTAX/{delim} {
    yylval.id = yytext;
    return WRITE_SYNTAX;
}

 /*
  * Lex rules for forbidden keywords. Do you use {delim} here because it
  * introduces a trailing context which is (a) slow and (b) causes
  * REJECT to be used.
  */

<INITIAL>ABSENT|ANY|BIT|BOOLEAN|BY|COMPONENT|COMPONENTS|DEFAULT|DEFINED|ENUMERATED|EXPLICIT|EXTERNAL|FALSE|MAX|MIN|MINUS-INFINITY|NULL|OPTIONAL|PLUS-INFINITY|PRESENT|PRIVATE|REAL|SET|TAGS|TRUE|WITH/[^a-zA-Z0-9-] {
    smiPrintError(parser, ERR_ILLEGAL_KEYWORD, yytext);
}

<INITIAL>ABSENT|ANY|BIT|BOOLEAN|BY|COMPONENT|COMPONENTS|DEFAULT|DEFINED|ENUMERATED|EXPLICIT|EXTERNAL|FALSE|MAX|MIN|MINUS-INFINITY|NULL|OPTIONAL|PLUS-INFINITY|PRESENT|PRIVATE|REAL|SET|TAGS|TRUE|WITH/-- {
    smiPrintError(parser, ERR_ILLEGAL_KEYWORD, yytext);
}

 /*
  * Lex rules for descriptors.
  */

 /* e.g. module names: REF: draft,p.12-13 */
<INITIAL>[A-Z](-?[a-zA-Z0-9_]+)*-? {
    if (yytext[yyleng-1] == '-') {
	smiPrintError(parser, ERR_ID_ENDS_IN_HYPHEN, yytext);
    }
    if (strchr(yytext, '_')) {
        smiPrintError(parser, ERR_UNDERSCORE_IN_IDENTIFIER, yytext);
    }
    yylval.id = smiStrdup(yytext);
    return UPPERCASE_IDENTIFIER;
}

 /* same for lowercase names */
<INITIAL>[a-z](-?[a-zA-Z0-9_]+)*-? {
    if (yytext[yyleng-1] == '-') {
	smiPrintError(parser, ERR_ID_ENDS_IN_HYPHEN, yytext);
    }
    if (strchr(yytext, '_')) {
        smiPrintError(parser, ERR_UNDERSCORE_IN_IDENTIFIER, yytext);
    }
    yylval.id = smiStrdup(yytext);
    return LOWERCASE_IDENTIFIER;
}

 /*
  * Lex rules for numbers.
  */

<INITIAL>0+/[0-9] {
    smiPrintError(parser, ERR_LEADING_ZEROS);
}

<INITIAL>([1-9][0-9]*|0)/[^0-9] {
    errno = 0;
    yylval.unsigned32 = strtoul(yytext, NULL, 10);
    if (errno == ERANGE) {
        /* TODO: proper handling for SPPI vs SMI modules. */
        if ((thisParser.modulePtr->export.language == SMI_LANGUAGE_SPPI) ||
            (thisParser.modulePtr->export.language == SMI_LANGUAGE_SMIV2)) {
            yylval.unsigned64 = strtoull(yytext, NULL, 10);
            if (errno == ERANGE) {
                /*
                 * Dirty hack - on my system strtoull returns ERANGE for
                 * numbers that definitely are in range for 64 bit types.
                 * Nevertheless strtoull returns the correct value. To
                 * check this we convert the number back to a string.
                 */
                char tmpbuf[24];
                sprintf(tmpbuf, "%llu", yylval.unsigned64);
                if (strcmp(yytext, tmpbuf))
                    smiPrintError(parser, ERR_SPPI_UNSIGNED64_NUMBER_RANGE, yytext);
            }
            return NUMBER64;
        }
        
	/* We don't want to see this error for the SNMPv2-SMI module,
         * neither for COPS-PR-SPPI. */
	if (strcmp(thisParser.modulePtr->export.name, "SNMPv2-SMI") &&
            strcmp(thisParser.modulePtr->export.name, "COPS-PR-SPPI")) {
	    smiPrintError(parser, ERR_SMIV2_UNSIGNED_NUMBER_RANGE, yytext);
	}
    }
    return NUMBER;
}

<INITIAL>-([1-9][0-9]*|0)/[^0-9] {
    errno = 0;
    yylval.integer32 = strtol(yytext, NULL, 10);
    if (errno == ERANGE) {
        /* TODO: proper handling for SPPI vs SMI modules. */
        if (thisParser.modulePtr->export.language == SMI_LANGUAGE_SPPI) {
            yylval.integer64 = strtoll(yytext, NULL, 10);
            if (errno == ERANGE) {
                /*
                 * Dirty hack - on my system strtoull returns ERANGE for
                 * numbers that definitely are in range for 64 bit types.
                 * Nevertheless strtoull returns the correct value. To
                 * check this we convert the number back to a string.
                 */
                char tmpbuf[24];
                sprintf(tmpbuf, "%lld", yylval.integer64);
                if (strcmp(yytext, tmpbuf))
                    smiPrintError(parser, ERR_SPPI_SIGNED64_NUMBER_RANGE, yytext);
            }
            return NEGATIVENUMBER64;
        }
	/* We don't want to see this error for the COPS-PR-SPPI module. */
	if (strcmp(thisParser.modulePtr->export.name, "COPS-PR-SPPI"))
	    smiPrintError(parser, ERR_SMIV2_SIGNED_NUMBER_RANGE, yytext);
    }
    return NEGATIVENUMBER;
}

 /*
  * Lex rules for binary, hexadecimal and quoted strings (RFC 2578 3.1.1)
  */

<INITIAL>\'[01]*\'[bB] {
    yylval.text = yytext + 1;
    yytext[yyleng-2] = '\0';
    if ((yyleng-3) % 8) {
	smiPrintError(parser, ERR_BIN_STRING_MUL8, yylval.text);
    }
    return BIN_STRING;
}

<INITIAL>\'[0123456789AaBbCcDdEeFf]*\'[hH] {
    yylval.text = yytext + 1;
    yytext[yyleng-2] = '\0';
    if ((yyleng-3) % 2) {
	smiPrintError(parser, ERR_HEX_STRING_MUL2, yylval.text);
    }
    return HEX_STRING;
}

<INITIAL>\"[^\"]*\" {
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
	    thisParser.line++;
	    d[0] = '\n';
	    s++;
	    newlineflag = 1;
	    column = 0;
	    if (cutoffcolumn < 0) cutoffcolumn = 0;
	} else if (s[0] == '\n' || s[0] == '\r') {	/* simple newline */
	    thisParser.line++;
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
    return QUOTED_STRING;
}

 /*
  * Skip the remainder of the line
  */
<Skipline>.*{eol} {
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
