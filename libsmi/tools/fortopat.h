/*
 * fortopat.h --
 *
 *      Header file for format string to xsd pattern conversion.
 *
 * Copyright (c) 2001 J. Schoenwaelder, Technical University of Braunschweig.
 *           (c) 2002 T. Klie, Technical University of Braunschweig.
 *           (c) 2002 F. Strauss, Technical University of Braunschweig.
 *           (c) 2007 T. Klie, Technical University of Braunschweig.
 *	     (c) 2007 J. Schoenwaelder, Jacobs University Bremen.
 *
 * See the file "COPYING" for information on usage and redistribution
 * of this file, and for a DISCLAIMER OF ALL WARRANTIES.
 *
 * @(#) $Id: rea.h 7289 2007-09-28 06:48:22Z schoenw $
 */

#ifndef _FORTOPAT_H
#define _FORTOPAT_H

#include <config.h>

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>
#ifdef HAVE_WIN_H
#include "win.h"
#endif

#include "smi.h"
#include "smidump.h"

/*
 * Convert the octet string format string in fmt to an xsd pattern
 * with the ranges given by range. The pattern is returned as an
 * allocated string and must be xfree()d by the caller. On failure,
 * a NULL pointer will be returned.
 */

extern char* smiFormatToPattern(const char *format, SmiRange *range);

#endif
