/*
 * scanner-yang.h --
 *
 *      Definition of lexical tokens of the YANG language.
 *
 * Copyright (c) 1999 Frank Strauss, Technical University of Braunschweig.
 *
 * See the file "COPYING" for information on usage and redistribution
 * of this file, and for a DISCLAIMER OF ALL WARRANTIES.
 *
 * @(#) $Id: scanner-yang.h 973 2000-07-17 09:34:36Z strauss $
 */

#ifndef _SCANNER_YANG_H
#define _SCANNER_YANG_H

#include <stdio.h>

#include "parser-yang.h"

#define YY_NO_UNPUT

extern char *yytext;
#if 0
/* this does not seem to be needed with flex 2.5.35 and in fact conflicts */
extern int yyleng;
#endif

extern int yangEnterLexRecursion(FILE *file);
extern void yangLeaveLexRecursion();

#endif /* _SCANNER_YANG_H */
