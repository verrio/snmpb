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
 * @(#) $Id: util.h 1470 2002-11-13 13:15:03Z strauss $
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

#ifdef HAVE_DMALLOC_H

extern void *_smiMalloc(char *, int, size_t);
extern void *_smiRealloc(char *, int, void *ptr, size_t size);
extern char *_smiStrdup(char *, int, const char *s1);
extern char *_smiStrndup(char *, int, const char *s1, size_t n);
extern void _smiFree(char *, int, void *ptr);

#define	smiMalloc(s)	_smiMalloc(__FILE__, __LINE__, s)
#define	smiRealloc(p,s)	_smiRealloc(__FILE__, __LINE__, p, s)
#define	smiStrdup(s)	_smiStrdup(__FILE__, __LINE__, s)
#define	smiStrndup(s,n)	_smiStrndup(__FILE__, __LINE__, s, n)
#define	smiFree(p)	_smiFree(__FILE__, __LINE__, p)

#else

extern void *smiMalloc(size_t size);

extern void *smiRealloc(void *ptr, size_t size);

extern char *smiStrdup(const char *s1);

extern char *smiStrndup(const char *s1, size_t n);

extern void smiFree(void *ptr);

#endif

extern int smiIsPath(const char *s);

#ifndef HAVE_TIMEGM
time_t timegm(struct tm *tm);
#endif

int smiTypeDerivedFrom(Type *typePtr, Type *parentTypePtr);

#endif /* _UTIL_H */

