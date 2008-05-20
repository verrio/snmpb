/*
 * smixlate.c --
 *
 *      Translate OIDs located in the input stream.
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
#include <stdio.h>
#include <string.h>
#include <ctype.h>
#include <assert.h>
#ifdef HAVE_UNISTD_H
#include <unistd.h>
#endif
#ifdef HAVE_WIN_H
#include "win.h"
#endif

#include "smi.h"
#include "shhopt.h"
#include "dstring.h"

static int flags;
static int aFlag = 0;	/* translate all OIDs */
static int fFlag = 0;	/* preserve formatting */

static void translate(dstring_t *token, dstring_t *subst)
{
    SmiNode *smiNode;
    SmiSubid oid[256];
    unsigned int oidlen = 0;
    unsigned int i;
    char *p;

    assert(token && subst);

    dstring_truncate(subst, 0);
    dstring_assign(subst, dstring_str(token));

    for (oidlen = 0, p = strtok(dstring_str(token), ". "); p;
	 oidlen++, p = strtok(NULL, ". ")) {
	oid[oidlen] = strtoul(p, NULL, 0);
    }

    smiNode = smiGetNodeByOID(oidlen, oid);
    if (smiNode &&
	(aFlag
	 || smiNode->nodekind == SMI_NODEKIND_SCALAR
	 || smiNode->nodekind == SMI_NODEKIND_COLUMN
	 || smiNode->nodekind == SMI_NODEKIND_NOTIFICATION
	 || smiNode->nodekind == SMI_NODEKIND_TABLE
	 || smiNode->nodekind == SMI_NODEKIND_ROW)) {
	dstring_assign(subst, smiNode->name);
	for (i = smiNode->oidlen; i < oidlen; i++) {
	    dstring_append_printf(subst, ".%d", oid[i]);
	}
    }
}


static void process(FILE *stream)
{
    int c, space = 0;
    enum { TXT, NUM, NUMDOT, NUMDOTNUM, OID, OIDDOT, EATSPACE } state = TXT;
    dstring_t *token, *subst;

    token = dstring_new();
    subst = dstring_new();
    
    if (! token || ! subst) {
	return;
    }

    /*
     * Shall we require iswhite() or ispunct() before and after the
     * OID?
     *
     * TODO: - translate instance identifier to something meaningful
     *         (e.g. foobar["name",32]) where possible
     *       - generate warnings if instance identifier are incomplete
     *       - provide a reverse translation service (-x) (but this is
     *         more complex since it is unclear how to identify names
     *	     - make the white space magic optional
     */

    while ((c = fgetc(stream)) != EOF) {
	switch (state) {
	case TXT:
	    fputs(dstring_str(token), stdout);
	    dstring_truncate(token, 0);
	    if (isdigit(c) && c >= '0' && c <= '2') {
		dstring_append_char(token, (char) c);
		state = NUM;
	    } else {
		fputc(c, stdout);
		fflush(stdout);
	    }
	    break;
	case NUM:
	    if (isdigit(c)) {
		dstring_append_char(token, (char) c);
	    } else if (c == '.') {
		dstring_append_char(token, (char) c);
		state = NUMDOT;
	    } else {
		dstring_append_char(token, (char) c);
		state = TXT;
	    }
	    break;
	case NUMDOT:
	    if (isdigit(c)) {
		dstring_append_char(token, (char) c);
		state = NUMDOTNUM;
	    } else {
		dstring_append_char(token, (char) c);
		state = TXT;
	    }
	    break;
	case NUMDOTNUM:
	    if (isdigit(c)) {
		dstring_append_char(token, (char) c);
	    } if (c == '.') {
		dstring_append_char(token, (char) c);
		state = OID;
	    } else {
		dstring_append_char(token, (char) c);
		state = TXT;
	    }
	    break;
	case OID:
	    if (isdigit(c)) {
		dstring_append_char(token, (char) c);
	    } else if (c == '.') {
		dstring_append_char(token, (char) c);
		state = OIDDOT;
	    } else {
		translate(token, subst);
		if (fFlag) {
		    if (dstring_len(subst) < dstring_len(token)) {
			dstring_expand(subst, dstring_len(token), ' ');
		    }
		}
		fputs(dstring_str(subst), stdout);
		if (dstring_len(subst) > dstring_len(token)) {
		    space = dstring_len(subst) - dstring_len(token) - 1;
		} else {
		    space = 0;
		}
		if (fFlag && space > 0 && c == ' ') {
		    state = EATSPACE;
		    space--;
		} else {
		    state = TXT;
		    space = 0;
		    fputc(c, stdout);
		}
		dstring_truncate(token, 0);
	    }
	    break;
	case OIDDOT:
	    if (isdigit(c)) {
		dstring_append_char(token, (char) c);
		state = OID;
	    } else {
		translate(token, subst);
		fputs(dstring_str(subst), stdout);
		fputc(c, stdout);
		dstring_truncate(token, 0);
		state = TXT;
	    }
	    break;
	case EATSPACE:
	    if (c == ' ' && space > 0) {
		space--;
	    } else {
		state = TXT;
	    }
	    break;
	}
    }

    if (dstring_len(token)) {
	switch (state) {
	case TXT:
	case NUM:
	case NUMDOT:
	case NUMDOTNUM:
	    fputs(dstring_str(token), stdout);
	    dstring_truncate(token, 0);
	    fputc(c, stdout);
	    fflush(stdout);
	    break;
	case OID:
	    if (isdigit(c)) {
		dstring_append_char(token, (char) c);
	    } else {
		translate(token, subst);
		if (fFlag) {
		    if (dstring_len(subst) < dstring_len(token)) {
			dstring_expand(subst, dstring_len(token), ' ');
		    }
		}
		fputs(dstring_str(subst), stdout);
		if (dstring_len(subst) > dstring_len(token)) {
		    space = dstring_len(subst) - dstring_len(token) - 1;
		} else {
		    space = 0;
		}
		if (fFlag && space > 0 && c == ' ') {
		    space--;
		} else {
		    space = 0;
		    fputc(c, stdout);
		}
		dstring_truncate(token, 0);
	    }
	    break;
	case OIDDOT:
	    if (isdigit(c)) {
		dstring_append_char(token, (char) c);
	    } else {
		translate(token, subst);
		fputs(dstring_str(subst), stdout);
		fputc(c, stdout);
		dstring_truncate(token, 0);
	    }
	    break;
	case EATSPACE:
	    if (c == ' ' && space > 0) {
		space--;
	    } else {
		fputc(c, stdout);
	    }
	    break;
	}
    }
}



static void usage()
{
    fprintf(stderr,
	    "Usage: smixlate [options] [module or path ...]\n"
	    "  -V, --version         show version and license information\n"
	    "  -h, --help            show usage information\n"
	    "  -c, --config=file     load a specific configuration file\n"
	    "  -p, --preload=module  preload <module>\n"
	    "  -r, --recursive       print errors also for imported modules\n"
	    "  -l, --level=level     set maximum level of errors and warnings\n"
	    "  -i, --ignore=prefix   ignore errors matching prefix pattern\n"
	    "  -I, --noignore=prefix do not ignore errors matching prefix pattern\n"
	    "  -a, --all             replace all OIDs (including OID prefixes)\n"
	    "  -f, --format          preserve formatting as much as possible\n");
}



static void help() { usage(); exit(0); }
static void version() { printf("smixlate " SMI_VERSION_STRING "\n"); exit(0); }
static void config(char *filename) { smiReadConfig(filename, "smixlate"); }
static void preload(char *module) { smiLoadModule(module); }
static void recursive() { flags |= SMI_FLAG_RECURSIVE; smiSetFlags(flags); }
static void level(int lev) { smiSetErrorLevel(lev); }
static void ignore(char *ign) { smiSetSeverity(ign, 128); }
static void noignore(char *ign) { smiSetSeverity(ign, -1); }



int main(int argc, char *argv[])
{
    int i;

    static optStruct opt[] = {
	/* short long              type        var/func       special       */
	{ 'a', "all",		 OPT_FLAG,   &aFlag,        0 },
	{ 'f', "format",         OPT_FLAG,   &fFlag,	    0 },
	{ 'h', "help",           OPT_FLAG,   help,          OPT_CALLFUNC },
	{ 'V', "version",        OPT_FLAG,   version,       OPT_CALLFUNC },
	{ 'c', "config",         OPT_STRING, config,        OPT_CALLFUNC },
	{ 'p', "preload",        OPT_STRING, preload,       OPT_CALLFUNC },
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
	smiInit("smixlate");
    else
	smiInit(NULL);

    flags = smiGetFlags();
    flags |= SMI_FLAG_ERRORS;
    flags |= SMI_FLAG_NODESCR;
    smiSetFlags(flags);

    optParseOptions(&argc, argv, opt, 0);

    for (i = 1; i < argc; i++) {
	if (smiLoadModule(argv[i]) == NULL) {
	    fprintf(stderr, "smixlate: cannot locate module `%s'\n",
		    argv[i]);
	    smiExit();
	    exit(1);
	}
    }


    process(stdin);

    smiExit();

    return 0;
}
