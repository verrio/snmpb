/*
 * smidump.h --
 *
 *      This header contains the entry points into the modules
 *	which dump MIB modules in various output format.
 *
 * Copyright (c) 1999 Frank Strauss, Technical University of Braunschweig.
 * Copyright (c) 1999 J. Schoenwaelder, Technical University of Braunschweig.
 *
 * See the file "COPYING" for information on usage and redistribution
 * of this file, and for a DISCLAIMER OF ALL WARRANTIES.
 *
 * @(#) $Id: smidump.h 7870 2008-03-11 19:29:58Z schoenw $
 */

#ifndef _SMIDUMP_H
#define _SMIDUMP_H

#include "shhopt.h"



/*
 * The following flags can be passed to output drivers in the flags
 * member of the struct above.
 */

#define SMIDUMP_FLAG_SILENT	0x01	/* suppress comments */
#define SMIDUMP_FLAG_UNITE	0x02	/* generated united output */
#define SMIDUMP_FLAG_ERROR	0x04	/* parser hit serious parse errors */


/*
 * Driver capability flags which are used to warn about options not
 * understood by a particular output driver.
 */

#define SMIDUMP_DRIVER_CANT_UNITE	0x02
#define SMIDUMP_DRIVER_CANT_OUTPUT	0x04



/*
 * The data structure which represents a driver specific option.
 * A static array of these options (with the last option's type
 * being OPT_END) is used in SmidumpDriver.
 * The SmidumpDriverOption structure is based on shhopt.h:optStruct.
 */

typedef struct SmidumpDriverOption {
    char *name;
    optArgType type;
    void *arg;
    int flags;
    char *descr;
} SmidumpDriverOption;



/*
 * The data structure which represents the entry point for an output
 * driver. The ignflags contain the driver capabilities as described
 * above.
 */

typedef struct SmidumpDriver {
    char *name;				/* Name of the output driver. */
    void (*func) (int, SmiModule **,	/* Output generating function. */
		  int, char *);
    int smiflags;			/* Flags for the SMI parser. */
    int ignflags;			/* Output driver flags ignored. */
    char *descr;			/* Short description. */
    SmidumpDriverOption *opt;           /* Driver specific options. */
    struct SmidumpDriver *next;
} SmidumpDriver;



/*
 * The entry points for the output drivers that currently exist for
 * smidump.
 */

extern void initSmi(void);
extern void initSming(void);
extern void initSppi(void);
extern void initSvg(void);
extern void initImports(void);
extern void initTypes(void);
extern void initTree(void);
extern void initIdentifiers(void);
extern void initMetrics(void);
extern void initMosy(void);
extern void initXml(void);
extern void initCorba(void);
extern void initCm(void);
extern void initNetsnmp(void);
extern void initJax(void);
extern void initPython(void);
extern void initPerl(void);
extern void initScli(void);
extern void initXsd(void);
extern void initSizes(void);
extern void initCompliances(void);
extern void initYang(void);
extern void initBoilerplate(void);

extern void initSql(void);
extern void initFig(void);


/*
 * The functions are wrappers for the malloc functions which handle
 * memory allocation errors by terminating the program.
 */

extern void *xmalloc(size_t size);
extern void *xrealloc(void *ptr, size_t size);
extern void *xcalloc(size_t nmemb, size_t size);
extern char *xstrdup(const char *s);
extern void xfree(void *ptr);

extern void smidumpRegisterDriver(SmidumpDriver *driver);

#endif /* _SMIDUMP_H */
