/*
 * win.h --
 *
 *      Some helper functions to make libsmi compile with vc++ for win32.
 *
 * Copyright (c) 2000 E. Schoenfelder, Gaertner Datensysteme Braunschweig.
 * Copyright (c) 2000 J. Schoenwaelder, Technical University of Braunschweig.
 *
 * See the file "COPYING" for information on usage and redistribution
 * of this file, and for a DISCLAIMER OF ALL WARRANTIES.
 *
 * @(#) $Id: win.h 1867 2004-10-06 13:45:31Z strauss $
 */

#ifndef _WIN_H
#define _WIN_H

#include <ctype.h>
#include <limits.h>
#include <io.h>

/*
 * The Win32 API provides use with stricmp(), which is a string
 * lowercase compare function. This should be equivalent with
 * strcasecmp().
 */

#define strcasecmp stricmp

/*
 * The access() function exists in the Win32 API, but there are no
 * defines for the mode parameter. So we provided them here.
 */

#ifndef F_OK
#define F_OK	00
#endif
#ifndef W_OK
#define W_OK	02
#endif
#ifndef R_OK
#define R_OK	04
#endif

/*
 * other function prototypes 
 */
#if ! defined(__GNUC__) && defined(__STDC__)
int __cdecl fileno(FILE *); 
#endif


/*
 * isascii() is a non __STDC__ extension needed when __STDC__ is defined in
 * Win32 environment.
 */
#if defined(__STDC__)
#ifndef isascii
#define isascii(_c)   ( (unsigned)(_c) < 0x80 )
#endif
#endif

#endif /* _WIN_H */
