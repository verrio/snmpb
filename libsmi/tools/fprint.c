/*
 * fprint.c --
 *
 *      This file defines commonly used utility functions to produce
 *	nicely indented output.
 *
 * Copyright (c) 1999 Frank Strauss, Technical University of Braunschweig.
 * Copyright (c) 2007 J. Schoenwaelder, Jacobs University Bremen.
 *
 * See the file "COPYING" for information on usage and redistribution
 * of this file, and for a DISCLAIMER OF ALL WARRANTIES.
 *
 * @(#) $Id: fprint.c 8024 2008-04-11 13:25:26Z schoenw $
 */



#include <config.h>

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdarg.h>
#ifdef HAVE_WIN_H
#include "win.h"
#endif

#include "smi.h"
#include "fprint.h"

/*
 * This module keeps internal state. Hence, these functions are not
 * thread safe. Note, however, that multi-threaded concurrent output
 * generation may not be such a wonderful idea anyway - so using
 * static variables may be good enough.
 */


int fprint_current_column = 0;
int fprint_indent_max = 64;
int fprint_indent_texts = 4;



void
fprint(FILE *f, char *fmt, ...)
{
    va_list ap;
    char    *s;
    char    *p;
    
    va_start(ap, fmt);
    fprint_current_column += smiVasprintf(&s, fmt, ap);
    va_end(ap);
    fputs(s, f);
    if ((p = strrchr(s, '\n'))) {
        fprint_current_column = strlen(p) - 1;
    }
    free(s);
}



void
fprintSegment(FILE *f, int column, char *string, int length)
{
    fprint(f, "%*c%s", column, ' ', string);
    if (length) {
	fprint(f, "%*c", length - strlen(string) - column, ' ');
    }
}



void
fprintWrapped(FILE *f, int column, char *string)
{
    if ((fprint_current_column + strlen(string)) > fprint_indent_max) {
	putc('\n', f);
	fprint_current_column = 0;
	fprintSegment(f, column, "", 0);
    }
    fprint(f, "%s", string);
}



void
fprintMultilineString(FILE *f, int column, const char *s)
{
    int i, len;
    
    fprintSegment(f, column - 1 + fprint_indent_texts, "\"", 0);
    if (s) {
	len = strlen(s);
	for (i=0; i < len; i++) {
	    putc(s[i], f);
	    fprint_current_column++;
	    if (s[i] == '\n') {
		fprint_current_column = 0;
		fprintSegment(f, column + fprint_indent_texts, "", 0);
	    }
	}
    }
    putc('\"', f);
    fprint_current_column++;
}
