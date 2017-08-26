    /*
 * scanner-yang.l --
 *
 *      Lexical rules for scanning the Yang modules.
 *
 * Copyright (c) 1999 Frank Strauss, Technical University of Braunschweig.
 *
 * See the file "COPYING" for information on usage and redistribution
 * of this file, and for a DISCLAIMER OF ALL WARRANTIES.
 *
 *  Authors: Kaloyan Kanev, Siarhei Kuryla
 */

%option noyywrap

%{

#include <config.h>
    
#ifdef BACKEND_YANG

#include <stdio.h>
#include <string.h>
#include <ctype.h>
#include <errno.h>

#if defined(HAVE_WIN_H)
#include "win.h"
#endif

#include "error.h"
#include "util.h"
#include "parser-yang.h"
#include "parser-yang.tab.h"
#include "scanner-yang.h"

#ifdef HAVE_DMALLOC_H
#include <dmalloc.h>
#endif

/* #define DEBUG */

#ifdef DEBUG 
#define debug(args...) fprintf (stderr, "[%s=%s]\n", args);
#define _debug(args...) fprintf (stderr, "[%s=%s]\n", args, yylval.text);
#else
#define debug
#define _debug
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
yangEnterLexRecursion(file)
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
yangLeaveLexRecursion()
{    
    yy_delete_buffer(YY_CURRENT_BUFFER);
    yy_switch_to_buffer(yybuffer[--lexDepth]);
}
    
%}

%{

char* concatTmp;
int lineAdjust;

void cleanSQString()
{

char *s, *d;		/* the source and destination pointer */
    int column = 0;		/* the current column */
    int newlineflag = 0;	/* we have just passed a newline */
    int cutoffcolumn = 0;	/* cut off white space up to this column */
				/* (computed by caculating the indentation */
				/* of the first column) */

    lineAdjust = 0;
    yytext[yyleng-1] = '\0';
    for (d = yytext, s = yytext+1; s[0] ; s++, d++) {

	if ((s[0] == '\n' && s[1] == '\r')	/* newline sequence */
	    || (s[0] == '\r' && s[1] == '\n')) {
	    lineAdjust += 1;
	    d[0] = '\n';
	    s++;
	    newlineflag = 1;
	    column = 0;
	    if (cutoffcolumn < 0) cutoffcolumn = 0;
	} else if (s[0] == '\n') {		/* simple newline */
	    lineAdjust += 1;
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
		  /*  smiPrintError(parser, ERR_ILLEGAL_CHAR_IN_STRING,
				  s[0], (unsigned char) s[0]);*/
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
        /* clean trailing spaces */
    for (d = yytext, s = yytext; s[0] ; s++, d++) {
	if (isspace((int)(unsigned char)s[0]))
	{
		char *i = s;		
		while(isspace((int)(unsigned char)s[0]) && s[0] !=  '\n')
	 	{
		    s++;
		}
		if(s[0] != '\n')
		{
			while(i != s)
			{
				d[0] = i[0];
				d++;
				i++;
			}
		}
	}
	d[0] = s[0];
    }
    d[0] = '\0';
}

void cleanDQString()
{

    char *s, *d;		/* the source and destination pointer */
    int column = 0;		/* the current column */
    int newlineflag = 0;	/* we have just passed a newline */
    int cutoffcolumn = 0;	/* cut off white space up to this column */
				/* (computed by caculating the indentation */
				/* of the first column) */

    lineAdjust = 0;
    yytext[yyleng-1] = '\0';
    for (d = yytext, s = yytext+1; s[0] ; s++, d++) {

	if ((s[0] == '\n' && s[1] == '\r')	/* newline sequence */
	    || (s[0] == '\r' && s[1] == '\n')) {
	    lineAdjust += 1;
	    d[0] = '\n';
	    s++;
	    newlineflag = 1;
	    column = 0;
	    if (cutoffcolumn < 0) cutoffcolumn = 0;
	} else if (s[0] == '\n') {		/* simple newline */
	    lineAdjust += 1;
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
		  /*  smiPrintError(parser, ERR_ILLEGAL_CHAR_IN_STRING,
				  s[0], (unsigned char) s[0]);*/
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
    /* clean trailing spaces */
    for (d = yytext, s = yytext; s[0] ; s++, d++) {
	if (isspace((int)(unsigned char)s[0]))
	{
		char *i = s;		
		while(isspace((int)(unsigned char)s[0]))
	 	{
		    s++;
		}
		if(s[0] != '\n')
		{
			while(i != s)
			{
				d[0] = i[0];
				d++;
				i++;
			}
		}
	}
	d[0] = s[0];
    }
    d[0] = '\0';
}

int getNumberOfCR() {
   int j, count = 0;
   for (j = 0; j <  yyleng; j++) {
        if (*(yytext + j) == '\n') {
            count++;
        }
   }
   return count;
}

%}

/* RFC 4234 core rules. */
ALPHA           [A-Za-z]
CR              \015 
CRLF            {CR}{LF}

DIGIT           [0-9]
NONZERODIGIT    [1-9]
POSDECIMAL      {NONZERODIGIT}{DIGIT}*
NONNEGDECIMAL   (0|{POSDECIMAL})
DECIMAL         ('-'{NONNEGDECIMAL}|{NONNEGDECIMAL})

HTAB            "\t"
LF              \012
SP              " "
VCHAR           ([\041-\377])
WSP             ({SP}|{HTAB})

/*
 * Lex pattern definitions.
 */
yangVersion     1
date            {DIGIT}{4}"-"{DIGIT}{2}"-"{DIGIT}{2}
identifier      ({ALPHA}|"_")({ALPHA}|{DIGIT}|"_"|"-"|".")*
lineBreak       ({CRLF}|{LF})
unquotedString  ([^\015 \n\t;{}/*]|(\*[^/])|(\/[^*/]))+

delim		([{; \n\t\015\012/])
UTFCHAR 	([_a-zA-Z0-9\200-\377])

currentKeyword      current
deprecatedKeyword   deprecated
obsoleteKeyword     obsolete
falseKeyword        false
trueKeyword         true
notSupported        not-supported
addKeyword          add
deleteKeyword       delete
replaceKeyword      replace

/*
 * Lex state definitions.
 */
%x		ConcatString
%x		FindPlus

%%

 /*
  * Lex rules for comments. We don't use {lineBreak} here because it
  * introduces a trailing context which is slow. To avoid it we add two rules for single line comments.
  */
<INITIAL>(\/){2}({WSP}|{VCHAR})*/{LF} {
}

<FindPlus>(\/){2}({WSP}|{VCHAR})*/{LF} {
}

<ConcatString>(\/){2}({WSP}|{VCHAR})*/{LF} {
}

<INITIAL>(\/){2}({WSP}|{VCHAR})*/{CRLF} {
}

<FindPlus>(\/){2}({WSP}|{VCHAR})*/{CRLF} {
}

<ConcatString>(\/){2}({WSP}|{VCHAR})*/{CRLF} {
}

<INITIAL>"/*"([^\*]|(\*[^/]))*"*/"  {
        thisParser.line += getNumberOfCR();
}

<FindPlus>"/*"([^\*]|(\*[^/]))*"*/"  {
        thisParser.line += getNumberOfCR();
}

<ConcatString>"/*"([^\*]|(\*[^/]))*"*/"  {
        thisParser.line += getNumberOfCR();
}

 /*
  * Lex rules for some special tokens.
  */
<INITIAL>[\{\}\;] {
    return yytext[0];
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
  * Lex rules for statment keywords 
  */

<INITIAL>anyxml/{delim} {
	yylval.text = "anyxml";
	return anyXMLKeyword;
}

<INITIAL>argument/{delim} {
	yylval.text = "argument";
	return argumentKeyword;
}

<INITIAL>augment/{delim} {
	yylval.text = "augment";
	return augmentKeyword;
}

<INITIAL>base/{delim} {
	yylval.text = "base";
	return baseKeyword;
}

<INITIAL>belongs-to/{delim} {
	yylval.text = "belongs-to";
	return belongs_toKeyword;
}

<INITIAL>bit/{delim} {
	yylval.text = "bit";
	return bitKeyword;
}

<INITIAL>case/{delim} {
	yylval.text = "case";
	return caseKeyword;
}

<INITIAL>choice/{delim} {
	yylval.text = "choice";
	return choiceKeyword;
}

<INITIAL>config/{delim} {
	yylval.text = "config";
	return configKeyword;
}

<INITIAL>contact/{delim} {
	yylval.text = "contact";
	return contactKeyword;
}

<INITIAL>container/{delim} {
	yylval.text = "container";
	return containerKeyword;
}

<INITIAL>default/{delim} {
	yylval.text = "default";
	return defaultKeyword;
}

<INITIAL>description/{delim} {
	yylval.text = "description";
	return descriptionKeyword;
}

<INITIAL>enum/{delim} {
	yylval.text = "enum";
	return enumKeyword;
}

<INITIAL>error-app-tag/{delim} {
	yylval.text = "error-app-tag";
	return error_app_tagKeyword;
}

<INITIAL>error-message/{delim} {
	yylval.text = "error-message";
	return error_messageKeyword;
}

<INITIAL>extension/{delim} {
	yylval.text = "extension";
	return extensionKeyword;
}

<INITIAL>deviation/{delim} {
	yylval.text = "deviation";
	return deviationKeyword;
}

<INITIAL>deviate/{delim} {
	yylval.text = "deviate";
	return deviateKeyword;
}

<INITIAL>feature/{delim} {
	yylval.text = "feature";
	return featureKeyword;
}

<INITIAL>fraction-digits/{delim} {
	yylval.text = "fraction-digits";
	return fractionDigitsKeyword;
}


<INITIAL>grouping/{delim} {
	yylval.text = "grouping";
	return groupingKeyword;
}

<INITIAL>identity/{delim} {
	yylval.text = "identity";
	return identityKeyword;
}

<INITIAL>if-feature/{delim} {
	yylval.text = "if-feature";
	return ifFeatureKeyword;
}

<INITIAL>import/{delim} {
	yylval.text = "import";
	return importKeyword;
}

<INITIAL>include/{delim} {
	yylval.text = "include";
	return includeKeyword;
}

<INITIAL>input/{delim} {
	yylval.text = "input";
	return inputKeyword;
}

<INITIAL>key/{delim} {
	yylval.text = "key";
	return keyKeyword;
}

<INITIAL>leaf/{delim} {
	yylval.text = "leaf";
	return leafKeyword;
}

<INITIAL>leaf-list/{delim} {
	yylval.text = "leaf-list";
	return leaf_listKeyword;
}

<INITIAL>length/{delim} {
	yylval.text = "length";
	return lengthKeyword;
}

<INITIAL>list/{delim} {
	yylval.text = "list";
	return listKeyword;
}

<INITIAL>mandatory/{delim} {
	yylval.text = "mandatory";
	return mandatoryKeyword;
}

<INITIAL>max-elements/{delim} {
	yylval.text = "max-elements";
	return max_elementsKeyword;
}

<INITIAL>min-elements/{delim} {
	yylval.text = "min-elements";
	return min_elementsKeyword;
}

<INITIAL>module/{delim} {
	yylval.text = "module";
	return moduleKeyword;
}

<INITIAL>must/{delim} {
	yylval.text = "must";
	return mustKeyword;
}

<INITIAL>namespace/{delim} {
	yylval.text = "namespace";
	return namespaceKeyword;
}

<INITIAL>notification/{delim} {
	yylval.text = "notification";
	return notificationKeyword;
}

<INITIAL>ordered-by/{delim} {
	yylval.text = "ordered-by";
	return ordered_byKeyword;
}

<INITIAL>organization/{delim} {
	yylval.text = "organization";
	return organizationKeyword;
}

<INITIAL>output/{delim} {
	yylval.text = "output";
	return outputKeyword;
}

<INITIAL>path/{delim} {
	yylval.text = "path";
	return pathKeyword;
}

<INITIAL>pattern/{delim} {
	yylval.text = "pattern";
	return patternKeyword;
}

<INITIAL>position/{delim} {
	yylval.text = "position";
	return positionKeyword;
}

<INITIAL>prefix/{delim} {
	yylval.text = "prefix";
	return prefixKeyword;
}

<INITIAL>presence/{delim} {
	yylval.text = "presence";
	return presenceKeyword;
}

<INITIAL>range/{delim} {
	yylval.text = "range";
	return rangeKeyword;
}

<INITIAL>reference/{delim} {
	yylval.text = "reference";
	return referenceKeyword;
}

<INITIAL>refine/{delim} {
	yylval.text = "refine";
	return refineKeyword;
}

<INITIAL>require-instance/{delim} {
	yylval.text = "require-instance";
	return requireInstanceKeyword;
}

<INITIAL>revision/{delim} {
	yylval.text = "revision";
	return revisionKeyword;
}

<INITIAL>rpc/{delim} {
	yylval.text = "rpc";
	return rpcKeyword;
}

<INITIAL>status/{delim} {
	yylval.text = "status";
	return statusKeyword;
}

<INITIAL>submodule/{delim} {
	yylval.text = "submodule";
	return submoduleKeyword;
}

<INITIAL>type/{delim} {
	yylval.text = "type";
	return typeKeyword;
}

<INITIAL>typedef/{delim} {
	yylval.text = "typedef";
	return typedefKeyword;
}

<INITIAL>unique/{delim} {
	yylval.text = "unique";
	return uniqueKeyword;
}

<INITIAL>units/{delim} {
	yylval.text = "units";
	return unitsKeyword;
}

<INITIAL>uses/{delim} {
	yylval.text = "uses";
	return usesKeyword;
}

<INITIAL>value/{delim} {
	yylval.text = "value";
	return valueKeyword;
}

<INITIAL>when/{delim} {
	yylval.text = "when";
	return whenKeyword;
}

<INITIAL>yang-version/{delim} {
	yylval.text = "yang-version";
	return yangversionKeyword;
}

<INITIAL>yin-element/{delim} {
	yylval.text = "yin-element";
	return yinElementKeyword;
}

 /*
  * Complex-type extension definitions. Nokia Siemens Networks Oy contribution.
  */

<INITIAL>{identifier}:complex-type/{delim} {
	yylval.text = smiStrdup(yytext);
	return complexTypeKeyword;
}

<INITIAL>{identifier}:extends/{delim} {
	yylval.text = smiStrdup(yytext);
	return extendsKeyword;
}

<INITIAL>{identifier}:abstract/{delim} {
	yylval.text = smiStrdup(yytext);
	return abstractKeyword;
}

<INITIAL>{identifier}:instance/{delim} {
	yylval.text = smiStrdup(yytext);
	return instanceKeyword;
}

<INITIAL>{identifier}:instance-list/{delim} {
	yylval.text = smiStrdup(yytext);
	return instanceListKeyword;
}

<INITIAL>{identifier}:instance-type/{delim} {
	yylval.text = smiStrdup(yytext);
	return instanceTypeKeyword;
}
 /*
  * Lex rules for other keywords
  */

<INITIAL>({currentKeyword}|\"{currentKeyword}\"|\'{currentKeyword}\')/{delim} {
	yylval.text = yytext;
	return currentKeyword;
}

<INITIAL>({deprecatedKeyword}|\"{deprecatedKeyword}\"|\'{deprecatedKeyword}\')/{delim} {
	yylval.text = yytext;
	return deprecatedKeyword;
}

<INITIAL>({falseKeyword}|\"{falseKeyword}\"|\'{falseKeyword}\')/{delim} {
	yylval.text = yytext;
	return falseKeyword;
}

<INITIAL>({obsoleteKeyword}|\"{obsoleteKeyword}\"|\'{obsoleteKeyword}\')/{delim} {
	yylval.text = yytext;
	return obsoleteKeyword;
}

<INITIAL>({trueKeyword}|\"{trueKeyword}\"|\'{trueKeyword}\')/{delim} {
	yylval.text = yytext;
	return trueKeyword;
}

<INITIAL>({notSupported}|\"{notSupported}\"|\'{notSupported}\')/{delim} {
	yylval.text = yytext;
	return notSupportedKeyword;
}

<INITIAL>({addKeyword}|\"{addKeyword}\"|\'{addKeyword}\')/{delim} {
	yylval.text = yytext;
	return addKeyword;
}

<INITIAL>({deleteKeyword}|\"{deleteKeyword}\"|\'{deleteKeyword}\')/{delim} {
	yylval.text = yytext;
	return deleteKeyword;
}

<INITIAL>({replaceKeyword}|\"{replaceKeyword}\"|\'{replaceKeyword}\')/{delim} {
	yylval.text = yytext;
	return replaceKeyword;
}

 /*
  * Lex rules for identifiers.
  * Note that these may be either single and double quoted or unquoted strings
  */
<INITIAL>{identifier}/{delim} {
    yylval.text = smiStrdup(yytext);
    _debug("uIdentifier");
    return identifier;
}

<INITIAL>(\"{identifier}\"|\'{identifier}\')/{delim} {
    yylval.text = smiStrndup((yytext + 1), yyleng - 2);
    _debug("qIdentifier");
    return identifier;
}

 /*
  * Lex rules for identifier-ref-arg-str.
  * Note that these may be either single and double quoted or unquoted strings
  */
<INITIAL>{identifier}:{identifier}/{delim} {
    yylval.text = smiStrdup(yytext);
    _debug("uIdentifierRefArgStr");
    return identifierRefArg;
}

<INITIAL>(\"{identifier}:{identifier}\"|\'{identifier}:{identifier}\')/{delim} {
    yylval.text = smiStrndup((yytext + 1), yyleng - 2);
    _debug("qIdentifierRefArgStr");
    return identifierRefArgStr;
}

 /*
  * Lex rules for dates.
  * Note that these may be either single and double quoted or unquoted strings
  */
<INITIAL>{date}/{delim} {    
    yylval.text = smiStrdup(yytext);
    _debug("uDate");
    return dateString;
}


<INITIAL>(\"{date}\"|\'{date}\')/{delim} {
    yylval.text = smiStrndup((yytext + 1), yyleng - 2);
    _debug("qDate");
    return dateString;
}

 /*
  * Lex rules for dates.
  * Note that these may be either single and double quoted or unquoted strings
  */
<INITIAL>{yangVersion}/{delim} {    
    yylval.text = smiStrdup(yytext);
    _debug("uYangVersion");
    return yangVersion;
}


<INITIAL>(\"{yangVersion}\"|\'{yangVersion}\')/{delim} {
    yylval.text = smiStrndup((yytext + 1), yyleng - 2);
    _debug("qYangVersion");
    return yangVersion;
}

 /*
  * Lex rules for textSegments.
  */

<INITIAL>\"([^\"]|(\\\"))*\"/{delim} {
   cleanSQString();
   thisParser.line += lineAdjust;
   concatTmp = smiStrdup(yytext);
   BEGIN(FindPlus);
}

<INITIAL>\'([^\'])*\'/{delim} {
    cleanSQString();
    thisParser.line += lineAdjust;
    concatTmp = smiStrdup(yytext);
    BEGIN(FindPlus);
}

<FindPlus>{WSP} {
}

<FindPlus>{lineBreak} {
    thisParser.line++;
}

<FindPlus>\+ {
	BEGIN(ConcatString);
}

 /* TODO: '+' is added temporarily, actually here might be any character,
  * but it's not an error because there is a separate rule for '+' above.
  */
<FindPlus>([^+]) {
	BEGIN(INITIAL);
	yyless(0); /* returns the character of the current token back to the input stream */
	yylval.text = smiStrdup(concatTmp);
	smiFree(concatTmp);
        _debug("qSring");
	return qString;
}

<ConcatString>\"([^\"]|(\\\"))*\" {
	char *tmp;
    	cleanDQString();
	thisParser.line += lineAdjust;
	tmp = (char*)smiMalloc(strlen(concatTmp) + strlen(yytext) + 1);
	strcpy(tmp, concatTmp);
	smiFree(concatTmp);
	concatTmp = tmp;
	strcat(concatTmp, yytext);
	BEGIN(FindPlus);
}

<ConcatString>\'([^\']|(\\\'))*\' {
        char *tmp;
        cleanSQString();
	thisParser.line += lineAdjust;
	tmp = (char*)smiMalloc(strlen(concatTmp) + strlen(yytext) + 2);
	strcpy(tmp, concatTmp);
	smiFree(concatTmp);
	concatTmp = tmp;
	strcat(concatTmp,"\n");
	strcat(concatTmp,yytext);

	BEGIN(FindPlus);
}

<ConcatString>{lineBreak} {
    thisParser.line++;
}

<ConcatString>{WSP} {
}

<INITIAL>{unquotedString}/{delim} {
        yylval.text = smiStrdup(yytext);
        _debug("uString");
        return uqString;
}
%%


#endif
