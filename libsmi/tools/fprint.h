/*
 * fprint.h --
 *
 *      Header-file for commonly used utility functions to produce
 *      nicely indented output.
 *
 * Copyright (c) 1999 Frank Strauss, Technical University of Braunschweig.
 * Copyright (c) 2007 J. Schoenwaelder, Jacobs University Bremen.
 *
 * See the file "COPYING" for information on usage and redistribution
 * of this file, and for a DISCLAIMER OF ALL WARRANTIES.
 *
 * @(#) $Id: fprint.h 7289 2007-09-28 06:48:22Z schoenw $
 */

#ifndef _FPRINT_H
#define _FPRINT_H

#include <stdio.h>
#include <stdarg.h>

extern int fprint_current_column;
extern int fprint_indent_max;
extern int fprint_indent_texts;

extern void fprintInit(int indent_max, int indent_texts);
	
extern void fprint(FILE *f, char *fmt, ...);

extern void fprintSegment(FILE *f, int column, char *string, int length);

extern void fprintWrapped(FILE *f, int column, char *string);

extern void fprintMultilineString(FILE *f, int column, const char *s);

#endif
