/*
 * dstring.h --
 *
 *	This file implements an abstract data type for dynamic strings.
 *
 *	Note: The behavior of the functions that modify the a dynamic
 *	string is undefined if an argument strings points into the
 *	dynamic string itself.
 *
 * Copyright (c) 2006 Juergen Schoenwaelder, International University Bremen.
 *
 * See the file "COPYING" for information on usage and redistribution
 * of this file, and for a DISCLAIMER OF ALL WARRANTIES.
 *
 * @(#) $Id: smilint.c 1867 2004-10-06 13:45:31Z strauss $
 */

#ifndef _DSTRING_H_
#define _DSTRING_H_

#include <stdarg.h>

#ifdef __GNUC__
# define inline /* extern */ static inline
#else 
#ifdef HAVE_WIN_H
# define inline __inline
#endif
#endif

struct dstring {
    char   *str;
    size_t len;
};

typedef struct dstring dstring_t;

inline char*
dstring_str(dstring_t *ds)
{
    return ds ? ds->str : NULL;
}

inline size_t
dstring_len(dstring_t *ds)
{
    return ds ? ds->len : 0;
}

extern dstring_t*
dstring_new(void);

extern dstring_t*
dstring_delete(dstring_t *ds);

extern dstring_t*
dstring_assign(dstring_t *ds, const char *s);

extern dstring_t*
dstring_concat(dstring_t *ds, ...);

extern dstring_t*
dstring_append(dstring_t *ds, const char *s);

extern dstring_t*
dstring_append_char(dstring_t *ds, const char c);

extern dstring_t*
dstring_append_printf(dstring_t *ds, const char *format, ...);

extern dstring_t*
dstring_append_vprintf(dstring_t *ds, const char *format, va_list ap);

extern dstring_t*
dstring_printf(dstring_t *ds, const char *format, ...);

extern dstring_t*
dstring_vprintf(dstring_t *ds, const char *format, va_list ap);

extern dstring_t*
dstring_truncate(dstring_t *ds, int len);

extern dstring_t*
dstring_expand(dstring_t *ds, int len, char fill);

#ifdef __GNUC__
#undef inline
#endif

#endif
