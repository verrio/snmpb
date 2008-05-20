/*
 * dstring.c --
 *
 *	Implementation of the dynamic string abstract data type.
 *
 * Copyright (c) 2006 Juergen Schoenwaelder, International University Bremen.
 *
 * See the file "COPYING" for information on usage and redistribution
 * of this file, and for a DISCLAIMER OF ALL WARRANTIES.
 *
 * @(#) $Id: smilint.c 1867 2004-10-06 13:45:31Z strauss $
 */

#include <config.h>

#include <stdlib.h>
#include <string.h>
#include <stdarg.h>
#include <stdio.h>
#ifdef HAVE_WIN_H
#include "win.h"
#endif

#include "dstring.h"

#if !defined va_copy
# if defined __va_copy
#  define va_copy __va_copy			/* C99 draft proposal */
# else
#  define va_copy(lhs,rhs) (lhs) = (rhs)
# endif
#endif

#if 0
/* These functions should not be needed if inlining works properly. */
char*
dstring_str(dstring_t *ds)
{
    return ds ? ds->str : NULL;
}

size_t
dstring_len(dstring_t *ds)
{
    return ds ? ds->len : 0;
}
#endif

static inline dstring_t*
dstring_grow(dstring_t *ds, size_t len)
{
    if (ds) {
        ds->str = realloc(ds->str, len + 1);
        if (! ds->str) {
            exit(EXIT_FAILURE);
        }
        ds->str[len] = '\0';
        ds->len = len;
    }
    return ds;
}

dstring_t*
dstring_new(void)
{
    dstring_t *ds;

    ds = calloc(1, sizeof(dstring_t));
    if (! ds) {
        exit(EXIT_FAILURE);
    }
    return dstring_grow(ds, 0);
}

dstring_t*
dstring_delete(dstring_t *ds)
{
    if (ds) {
        if (ds->str) free(ds->str);
        free(ds);
    }
    return NULL;
}

dstring_t*
dstring_assign(dstring_t *ds, const char *s)
{
    if (ds && s) {
        ds = dstring_grow(ds, strlen(s));
        strcpy(ds->str, s);
    }
    return ds;
}

dstring_t*
dstring_append(dstring_t *ds, const char *s)
{
    if (ds && s) {
        ds = dstring_grow(ds, ds->len + strlen(s));
        strcat(ds->str, s);
    }
    return ds;
}


dstring_t*
dstring_append_char(dstring_t *ds, const char c)
{
    if (ds) {
        ds = dstring_grow(ds, ds->len + 1);
	ds->str[ds->len-1] = c;
    }
    return ds;
}


dstring_t*
dstring_concat(dstring_t *ds, ...)
{
    va_list ap;
    const char *s;

    if (ds) {
        va_start(ap, ds);
        for (s = va_arg(ap, char*); s; s = va_arg(ap, char*)) {
            ds = dstring_append(ds, s);
        }
        va_end(ap);
    }
    return ds;
}

dstring_t*
dstring_append_printf(dstring_t *ds, const char *format, ...)
{
    va_list ap;

    va_start(ap, format);
    ds = dstring_append_vprintf(ds, format, ap);
    va_end(ap);
    return ds;
}

dstring_t *
dstring_append_vprintf(dstring_t *ds, const char *format, va_list _ap)
{
    int n, o;
    va_list ap;
    
    if (ds) {
	o = ds->len;
        while (1) {
            va_copy(ap, _ap);
            n = vsnprintf(ds->str + o, ds->len+1 - o, format, ap);
            va_end(ap);
            if (n > -1 && n+o <= ds->len) {
                if (n+o < ds->len) {
                    dstring_truncate(ds, n+o);
                }
                return ds;
            }
            if (n > 0) {
                ds = dstring_grow(ds, n + o);	                /* C99 */
            } else {
                ds = dstring_grow(ds, ds->len + ds->len);       /* GLIBC */
            }
        }
    }

    return ds;
}

dstring_t*
dstring_printf(dstring_t *ds, const char *format, ...)
{
    va_list ap;

    va_start(ap, format);
    ds = dstring_vprintf(ds, format, ap);
    va_end(ap);
    return ds;
}

dstring_t*
dstring_vprintf(dstring_t *ds, const char *format, va_list _ap)
{
    int n;
    va_list ap;

    if (ds) {
        while (1) {
            va_copy(ap, _ap);
            n = vsnprintf(ds->str, ds->len+1, format, ap);
            va_end(ap);
            if (n > -1 && n <= ds->len) {
                if (n < ds->len) {
                    dstring_truncate(ds, n);
                }
                return ds;
            }
            if (n > 0) {
                ds = dstring_grow(ds, n);                       /* C99 */
            } else {
                ds = dstring_grow(ds, ds->len + ds->len);       /* GLIBC */
            }
        }
    }
    return ds;
}

dstring_t*
dstring_truncate(dstring_t *ds, int len)
{
    if (ds && len < ds->len) {
        ds = dstring_grow(ds, len);
    }
    return ds;
}

dstring_t*
dstring_expand(dstring_t *ds, int len, char fill)
{
    if (ds && len > ds->len) {
        int i, old = ds->len;
        ds = dstring_grow(ds, len);
        for (i = old; i < len; i++) {
            ds->str[i] = fill;
        }
    }
    return ds;
}
