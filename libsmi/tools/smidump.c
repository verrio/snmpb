/*
 * smidump.c --
 *
 *      Dump a MIB module conforming to a given format.
 *
 * Copyright (c) 1999 Frank Strauss, Technical University of Braunschweig.
 * Copyright (c) 1999 J. Schoenwaelder, Technical University of Braunschweig.
 *
 * See the file "COPYING" for information on usage and redistribution
 * of this file, and for a DISCLAIMER OF ALL WARRANTIES.
 *
 * @(#) $Id: smidump.c 7870 2008-03-11 19:29:58Z schoenw $
 */

#include <config.h>

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#ifdef HAVE_UNISTD_H
#include <unistd.h>
#endif
#ifdef HAVE_WIN_H
#include "win.h"
#endif

#include "smi.h"
#include "shhopt.h"
#include "smidump.h"



static void help(void);
static void usage(void);
static void version(void);
static void config(char *filename);
static void level(int lev);
static void quiet(void);
static void preload(char *module);
static void unified(void);
static void format(char *form);


static int flags;
static int mFlag = 0;	/* show the name for error messages */
static int sFlag = 0;	/* show the severity for error messages */
static int kFlag = 0;	/* keep going after severe errors */
static SmidumpDriver *driver;
static SmidumpDriver *firstDriver;
static SmidumpDriver *lastDriver;
static SmidumpDriver *defaultDriver;
static char *output;

static int opts;
static optStruct *opt;
static optStruct genericOpt[] = {
    /* short long              type        var/func       special       */
    { 'h', "help",           OPT_FLAG,   help,          OPT_CALLFUNC },
    { 'V', "version",        OPT_FLAG,   version,       OPT_CALLFUNC },
    { 'c', "config",         OPT_STRING, config,        OPT_CALLFUNC },
    { 'l', "level",          OPT_INT,    level,         OPT_CALLFUNC },
    { 'p', "preload",        OPT_STRING, preload,       OPT_CALLFUNC },
    { 'l', "level",          OPT_INT,    level,         OPT_CALLFUNC },
    { 'q', "quiet",          OPT_FLAG,   quiet,         OPT_CALLFUNC },
    { 'm', "error-names",    OPT_FLAG,   &mFlag,        0 },
    { 's', "severity",       OPT_FLAG,   &sFlag,        0 },
    { 'u', "unified",        OPT_FLAG,   unified,       OPT_CALLFUNC },
    { 'f', "format",         OPT_STRING, format,        OPT_CALLFUNC },
    { 'o', "output",         OPT_STRING, &output,       0            },
    { 'k', "keep-going",     OPT_FLAG,	 &kFlag,	0 },
    { 0, 0, OPT_END, 0, 0 }  /* no more options */
};



void *xmalloc(size_t size)
{
    char *m = malloc(size);
    if (! m) {
	fprintf(stderr, "smidump: malloc failed - running out of memory\n");
	exit(1);
    }

    return m;
}



void *xrealloc(void *ptr, size_t size)
{
    char *m = realloc(ptr, size);
    if (! m) {
	fprintf(stderr, "smidump: realloc failed - running out of memory\n");
	exit(1);
    }
    return m;
}



void *xcalloc(size_t nmemb, size_t size)
{
    char *m = calloc(nmemb, size);
    if (! m) {
	fprintf(stderr, "smidump: calloc failed - running out of memory\n");
	exit(1);
    }
    return m;
}



char *xstrdup(const char *s)
{
    char *m = strdup(s);
    if (! m) {
	fprintf(stderr, "smidump: strdup failed - running out of memory\n");
	exit(1);
    }
    return m;
}



void xfree(void *ptr)
{
    free(ptr);
}



void smidumpRegisterDriver(SmidumpDriver *driver)
{
    int i;
    
    if (!firstDriver) {
	firstDriver = driver;
	lastDriver = driver;
    } else {
	lastDriver->next = driver;
	lastDriver = driver;
    }

    for (i = 0; driver->opt && driver->opt[i].type != OPT_END; i++) {
	opt = xrealloc(opt, (opts+1) * sizeof(optStruct));
	memcpy(&opt[opts], &opt[opts-1], sizeof(optStruct));
	opt[opts-1].shortName = 0;
	opt[opts-1].longName  = xmalloc(strlen(driver->name) +
				 strlen(driver->opt[i].name) + 2);
	sprintf(opt[opts-1].longName, "%s-%s",
		driver->name, driver->opt[i].name);
	opt[opts-1].type      = driver->opt[i].type;
	opt[opts-1].arg       = driver->opt[i].arg;
	opt[opts-1].flags     = driver->opt[i].flags;
	opts++;
    }
}



static void formats()
{
    SmidumpDriver *driver = firstDriver;
    
    for (driver = firstDriver; driver; driver = driver->next) {
	fprintf(stderr, "  %-14s: %s\n", driver->name,
 		driver->descr ? driver->descr : "...");
    }
}



static void usage()
{
    int i;
    SmidumpDriver *driver;
    char *value = NULL;
#ifdef _MSC_VER
    #if _MSC_VER >= 1400
/*
   %n in printf is a security vulnerability. Ref:
   http://en.wikipedia.org/wiki/Format_string_vulnerabilities
   MS decided it was important enough to disble it by default. Ref:
   "ms-help://MS.VSCC.v80/MS.MSDN.v80/MS.VisualStudio.v80.en/dv_vccrt/html/77a854ae-5b48-4865-89f4-f2dc5cf80f52.htm
   Calling _set_printf_count_output() stops an invalid parameter crash.
*/
    int printf_state;
    #endif
#endif
    
    fprintf(stderr,
	    "Usage: smidump [options] [module or path ...]\n"
	    "  -V, --version        show version and license information\n"
	    "  -h, --help           show usage information\n"
	    "  -q, --quiet          do not generate any comments\n"
	    "  -c, --config=file    load a specific configuration file\n"
	    "  -p, --preload=module preload <module>\n"
	    "  -l, --level=level    set maximum level of errors and warnings\n"
	    "  -m, --error-names    print the name of errors in braces\n"
	    "  -s, --severity       print the severity of errors in brackets\n"
	    "  -f, --format=format  use <format> when dumping (default %s)\n"
	    "  -o, --output=name    use <name> when creating names for output files\n"
	    "  -u, --unified        print a single unified output of all modules\n"
	    "  -k, --keep-going     continue after serious parse errors\n\n",
	    defaultDriver ? defaultDriver->name : "none");

    fprintf(stderr, "Supported formats are:\n");
    formats();

    for (driver = firstDriver; driver; driver = driver->next) {
	if (! driver->opt) continue;
	fprintf(stderr, "\nSpecific option for the \"%s\" format:\n",
		driver->name);
	for (i = 0; driver->opt && driver->opt[i].type != OPT_END; i++) {
	    int n;
	    switch (driver->opt[i].type) {
	    case OPT_END:
	    case OPT_FLAG:
		value = NULL;
		break;
	    case OPT_STRING:
		value = "string";
		break;
	    case OPT_INT:
	    case OPT_UINT:
	    case OPT_LONG:
	    case OPT_ULONG:
		value = "number";
		break;
	    }
#ifdef _MSC_VER
    #if _MSC_VER >= 1400
	    printf_state=_set_printf_count_output(1);
    #endif
#endif
	    fprintf(stderr, "  --%s-%s%s%s%n",
		    driver->name, driver->opt[i].name,
		    value ? "=" : "",
		    value ? value : "",
		    &n);
#ifdef _MSC_VER
    #if _MSC_VER >= 1400
	    (void)_set_printf_count_output(printf_state);
    #endif
#endif
	    fprintf(stderr, "%*s%s\n",
		    30-n, "",
		    driver->opt[i].descr ? driver->opt[i].descr : "...");
	}
    }
}



static void help() { usage(); exit(0); }
static void version() { printf("smidump " SMI_VERSION_STRING "\n"); exit(0); }
static void config(char *filename) { smiReadConfig(filename, "smidump"); }
static void level(int lev) { smiSetErrorLevel(lev); }
static void quiet() { flags |= SMIDUMP_FLAG_SILENT; }
static void preload(char *module) { smiLoadModule(module); }
static void unified() { flags |= SMIDUMP_FLAG_UNITE; }

static void format(char *form)
{
    for (driver = firstDriver; driver; driver = driver->next) {
	if (strcasecmp(driver->name, form) == 0) {
	    break;
	}
    }
    if (!driver) {
	fprintf(stderr, "smidump: invalid dump format `%s'"
		" - supported formats are:\n", form);
	formats();
	exit(1);
    }
}



static void
errorHandler(char *path, int line, int severity, char *msg, char *tag)
{
    if (path) {
	fprintf(stderr, "%s:%d: ", path, line);
    }
    if (sFlag) {
	fprintf(stderr, "[%d] ", severity);
    }
    if (mFlag) {
	fprintf(stderr, "{%s} ", tag);
    }
    switch (severity) {
    case 4:
    case 5:
	fprintf(stderr, "warning: ");
	break;
    case 6:	
	fprintf(stderr, "info: ");
	break;
    }
    fprintf(stderr, "%s\n", msg);

    if (severity <= 0) {
	exit(1);
    }
}



int main(int argc, char *argv[])
{
    char *modulename;
    SmiModule *smiModule;
    int smiflags, i;
    SmiModule **modv = NULL;
    int modc = 0;

    output = NULL;
    firstDriver = lastDriver = defaultDriver = NULL;

    opts = sizeof(genericOpt) / sizeof(optStruct);
    opt = xmalloc(sizeof(genericOpt));
    memcpy(opt, genericOpt, sizeof(genericOpt));

    initCm();
    initCorba();
#if 0
    initFig();
#endif
    initIdentifiers();
    initImports();
    initJax();
    initMetrics();
    initMosy();
    initNetsnmp();
    initPerl();
    initPython();
    initSming();
    initSmi();                defaultDriver = lastDriver;
    initSppi();
#if 0
    initSql();
#endif
    initScli();
    initSvg();
    initTree();
    initTypes();
    initSizes();
    initXml();
    initXsd();
    initCompliances();
    initYang();
    initBoilerplate();
    
    for (i = 1; i < argc; i++)
	if ((strstr(argv[i], "-c") == argv[i]) ||
	    (strstr(argv[i], "--config") == argv[i])) break;
    if (i == argc) 
	smiInit("smidump");
    else
	smiInit(NULL);

    flags = 0;
    driver = defaultDriver;

    optParseOptions(&argc, argv, opt, 0);

    if (!driver) {
	fprintf(stderr, "smidump: no dump formats registered\n");
	smiExit();
	exit(1);
    }
    
    if (sFlag || mFlag) {
	smiSetErrorHandler(errorHandler);
    }

    smiflags = smiGetFlags();
    smiflags |= SMI_FLAG_ERRORS;
    smiflags |= driver->smiflags;
    smiSetFlags(smiflags);

    if (flags & SMIDUMP_FLAG_UNITE && driver->ignflags & SMIDUMP_DRIVER_CANT_UNITE) {
	fprintf(stderr, "smidump: %s format does not support united output:"
		" ignoring -u\n", driver->name);
	flags = (flags & ~SMIDUMP_FLAG_UNITE);
    }

    if (output && driver->ignflags & SMIDUMP_DRIVER_CANT_OUTPUT) {
	fprintf(stderr, "smidump: %s format does not support output option:"
		" ignoring -o %s\n", driver->name, output);
	output = NULL;
    }

    modv = (SmiModule **) xmalloc((argc) * sizeof(SmiModule *));
    modc = 0;
    
    for (i = 1; i < argc; i++) {
	modulename = smiLoadModule(argv[i]);
	smiModule = modulename ? smiGetModule(modulename) : NULL;
	if (smiModule) {
	    if ((smiModule->conformance) && (smiModule->conformance < 3)) {
		flags |= SMIDUMP_FLAG_ERROR;
		if (! (flags & SMIDUMP_FLAG_SILENT)) {
		    fprintf(stderr,
			    "smidump: module `%s' contains errors, "
			    "expect flawed output\n",
			    argv[i]);
		}
	    }
	    modv[modc++] = smiModule;
	} else {
	    fprintf(stderr, "smidump: cannot locate module `%s'\n",
		    argv[i]);
	}
    }

    if (! (flags & SMIDUMP_FLAG_ERROR) || kFlag) {
	(driver->func)(modc, modv, flags, output);
    } else {
	if (! (flags & SMIDUMP_FLAG_SILENT)) {
	    fprintf(stderr,
		    "smidump: aborting due to severe parsing errors\n"
		    "smidump: use the -k option to force continuation\n");
	}
    }

    smiExit();

    if (modv) xfree(modv);
    
    return 0;
}
