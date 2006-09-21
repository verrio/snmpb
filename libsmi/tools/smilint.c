/*
 * smilint.c --
 *
 *      MIB module checker main program.
 *
 * Copyright (c) 1999 Frank Strauss, Technical University of Braunschweig.
 *
 * See the file "COPYING" for information on usage and redistribution
 * of this file, and for a DISCLAIMER OF ALL WARRANTIES.
 *
 * @(#) $Id: smilint.c 1867 2004-10-06 13:45:31Z strauss $
 */

#include <config.h>

#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#ifdef HAVE_UNISTD_H
#include <unistd.h>
#endif
#ifdef HAVE_WIN_H
#include "win.h"
#endif

#include "smi.h"
#include "shhopt.h"



/*
 * These are functions that are not officially exported by the libsmi.
 * See the original prototype definitions in lib/error.h.
 */

extern int smiGetErrorSeverity(int id);
extern char* smiGetErrorTag(int id);
extern char* smiGetErrorMsg(int id);
extern char* smiGetErrorDescription(int id);


static int mFlag = 0;	/* show the name for error messages */
static int sFlag = 0;	/* show the severity for error messages */
static int eFlag = 0;	/* print the list of possible error messages */
static int flags;


typedef struct Error {
    int id;
    int severity;
    char *tag;
    char *msg;
    char *description;
    int used;
} Error;


static Error *errors = NULL;


static void fold(FILE *f, int indent, const char *msg)
{
    const char *p, *s;

    if (! msg) {
	fprintf(f, "\n");
	return;
    }

    for (s = msg; *s; s++) {
	for (p = s; *p && *p != '\n'; p++) ;
	if (*p) {
	    fprintf(f, "%.*s\n%*s", p - s, s, indent, "");
	    s = p;
	} else {
	    fprintf(f, "%.*s\n", p - s, s);
	    break;
	}
    }
}



static int compare(const void *v1, const void *v2)
{
    Error *err1 = (Error *) v1;
    Error *err2 = (Error *) v2;

    if (err1->severity < err2->severity) {
	return -1;
    }
    if (err1->severity > err2->severity) {
	return 1;
    }
    return strcmp(err1->msg, err2->msg);
}



static Error* errors_new()
{
    int i, cnt;
    Error *errors;
    
    for (cnt = 0; smiGetErrorSeverity(cnt) >= 0; cnt++) ;
    
    errors = malloc((cnt + 1) * sizeof(Error));
    if (! errors) {
	fprintf(stderr, "smilint: malloc failed - running out of memory\n");
	exit(1);
    }
    memset(errors, 0, (cnt + 1) * sizeof(Error));

    for (i = 0; i < cnt; i++) {
	errors[i].id = i;
	errors[i].severity = smiGetErrorSeverity(i);
	errors[i].tag = smiGetErrorTag(i);
	errors[i].msg = smiGetErrorMsg(i);
	errors[i].description = smiGetErrorDescription(i);
    }

    qsort(errors, cnt, sizeof(Error), compare);

    return errors;
}


static void display_one(FILE *f, Error *error)
{
    const int indent = 12;
    char *type, *tag;

    type = (error->severity <= 3) ? "Error:" : "Warning:";
    tag = (error->tag && strlen(error->tag))
	? error->tag : "<xxx-missing-xxx>";
    fprintf(f, "%-*s %s (level %d%s)\n",
	    indent, type, tag, error->severity & 127,
	    error->severity & 128 ? ", ignored" : "");
    fprintf(f, "%-*s %s\n", indent, "Message:",
	    error->msg ? error->msg : "");
    if (error->description) {
	fprintf(f, "%-*s ", indent, "Description:");
	fold(f, indent + 1, error->description);
    }
}


static void display_all(Error *errors)
{
    int i;
    
    for (i = 0; errors[i].msg; i++) {
	if (i) printf("\n");
	display_one(stdout, errors + i);
    }
}



static void display_used(Error *errors)
{
    int i, n;

    for (i = 0, n = 0; errors[i].msg; i++) {
	if (errors[i].used && errors[i].description) n++;
    }

    if (! n) {
	return;
    }

    fprintf(stderr,
	    "\nAdditional descriptions of some error/warning messages:\n"); 

    for (i = 0; errors[i].msg; i++) {
	if (! errors[i].used || !errors[i].description) continue;
	if (i) fprintf(stderr, "\n");
	display_one(stderr, errors + i);
    }
}



static void usage()
{
    fprintf(stderr,
	    "Usage: smilint [options] [module or path ...]\n"
	    "  -V, --version         show version and license information\n"
	    "  -h, --help            show usage information\n"
	    "  -c, --config=file     load a specific configuration file\n"
	    "  -p, --preload=module  preload <module>\n"
	    "  -e, --error-list      print list of known error messages\n"
	    "  -m, --error-names     print the name of errors in braces\n"
	    "  -s, --severity        print the severity of errors in brackets\n"
	    "  -r, --recursive       print errors also for imported modules\n"
	    "  -l, --level=level     set maximum level of errors and warnings\n"
	    "  -i, --ignore=prefix   ignore errors matching prefix pattern\n"
	    "  -I, --noignore=prefix do not ignore errors matching prefix pattern\n");
}



static void help() { usage(); exit(0); }
static void version() { printf("smilint " SMI_VERSION_STRING "\n"); exit(0); }
static void config(char *filename) { smiReadConfig(filename, "smilint"); }
static void preload(char *module) { smiLoadModule(module); }
static void recursive() { flags |= SMI_FLAG_RECURSIVE; smiSetFlags(flags); }
static void level(int lev) { smiSetErrorLevel(lev); }
static void ignore(char *ign) { smiSetSeverity(ign, 128); }
static void noignore(char *ign) { smiSetSeverity(ign, -1); }



static void
errorHandler(char *path, int line, int severity, char *msg, char *tag)
{
    int i;
    
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

    /* If we are supposed to generate error descriptions, locate this
     * error in our error list and increment its usage counter. Note
     * that we assume that error tags are unique (and we should better
     * check for this somewhere). */

    if (errors) {
	for (i = 0; errors[i].msg; i++) {
	    if (strcmp(errors[i].tag, tag) == 0) {
		errors[i].used++;
		break;
	    }
	}
    }
}



int main(int argc, char *argv[])
{
    int i;

    static optStruct opt[] = {
	/* short long              type        var/func       special       */
	{ 'h', "help",           OPT_FLAG,   help,          OPT_CALLFUNC },
	{ 'V', "version",        OPT_FLAG,   version,       OPT_CALLFUNC },
	{ 'c', "config",         OPT_STRING, config,        OPT_CALLFUNC },
	{ 'p', "preload",        OPT_STRING, preload,       OPT_CALLFUNC },
	{ 'e', "error-list",     OPT_FLAG,   &eFlag,        0 },
	{ 'm', "error-names",    OPT_FLAG,   &mFlag,        0 },
	{ 's', "severity",       OPT_FLAG,   &sFlag,        0 },
	{ 'r', "recursive",      OPT_FLAG,   recursive,     OPT_CALLFUNC },
	{ 'l', "level",          OPT_INT,    level,         OPT_CALLFUNC },
	{ 'i', "ignore",         OPT_STRING, ignore,        OPT_CALLFUNC },
	{ 'I', "noignore",       OPT_STRING, noignore,      OPT_CALLFUNC },
	{ 0, 0, OPT_END, 0, 0 }  /* no more options */
    };
    
    for (i = 1; i < argc; i++)
	if ((strstr(argv[i], "-c") == argv[i]) ||
	    (strstr(argv[i], "--config") == argv[i])) break;
    if (i == argc) 
	smiInit("smilint");
    else
	smiInit(NULL);

    flags = smiGetFlags();
    flags |= SMI_FLAG_ERRORS;
    flags |= SMI_FLAG_NODESCR;
    smiSetFlags(flags);

    optParseOptions(&argc, argv, opt, 0);

    if (eFlag) {
	mFlag = 1;
	errors = errors_new();
    }

    if (sFlag || mFlag) {
	smiSetErrorHandler(errorHandler);
    }

    if (eFlag && argc == 1) {
	if (errors) {
	    display_all(errors);
	    free(errors);
	}
	smiExit();
	return 0;
    }
    
    for (i = 1; i < argc; i++) {
	if (smiLoadModule(argv[i]) == NULL) {
	    fprintf(stderr, "smilint: cannot locate module `%s'\n",
		    argv[i]);
	    smiExit();
	    exit(1);
	}
    }

    if (eFlag) {
	if (errors) {
	    display_used(errors);
	    free(errors);
	}
    }

    smiExit();

    return 0;
}
