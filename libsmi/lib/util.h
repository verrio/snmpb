/*
 * util.h --
 *
 *      Misc utility functions.
 *
 * Copyright (c) 1999 Frank Strauss, Technical University of Braunschweig.
 *
 * See the file "COPYING" for information on usage and redistribution
 * of this file, and for a DISCLAIMER OF ALL WARRANTIES.
 *
 * @(#) $Id: util.h 1675 2010-08-20 06:44:18Z schoenw $
 */

#ifndef _UTIL_H
#define _UTIL_H

#include <stdlib.h>
#include <time.h>

#include "smi.h"
#include "error.h"
#include "parser-smi.h"



#ifdef __CHECKER__
#undef HAVE_TIMEGM
#undef HAVE_STRTOULL
#undef HAVE_STRTOUQ
#undef HAVE_STRTOLL
#undef HAVE_STRTOQ
#endif

/*
 * Make sure we have strtoull() and strtoll() on this system.
 */

#ifndef HAVE_STRTOULL
#ifdef HAVE_STRTOUQ
#define strtoull strtouq
#else
#define strtoull strtoul	/* TODO */
#endif
#endif

#ifndef HAVE_STRTOLL
#ifdef HAVE_STRTOQ
#define strtoll strtoq
#else
#define strtoll strtol
#endif
#endif

extern int smiIsPath(const char *s);

#ifndef HAVE_TIMEGM
time_t timegm(struct tm *tm);
#endif

extern int smiTypeDerivedFrom(Type *typePtr, Type *parentTypePtr);

#endif /* _UTIL_H */

