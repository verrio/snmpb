/*
 * common.c --
 *
 *      Common functionality.
 *
 * Copyright (c) 1999-2002 Frank Strauss, Technical University of Braunschweig.
 *
 * See the file "COPYING" for information on usage and redistribution
 * of this file, and for a DISCLAIMER OF ALL WARRANTIES.
 *
 * @(#) $Id: data.c 12198 2009-01-05 08:37:07Z schoenw $
 */

#include <config.h>

#include <string.h>
#include <errno.h>
#include <ctype.h>
#include <sys/types.h>
#if !defined(_MSC_VER) && !defined(__MINGW32__)
#include <sys/wait.h>
#endif
#ifdef HAVE_UNISTD_H
#include <unistd.h>
#endif
#ifdef HAVE_WIN_H
#include "win.h"
#endif

#include "common.h"

char* smiGetModulePath(const char *modulename)
{
    char	*path = NULL, *dir, *smipath, *newmodulename;
    char	sep[2];
    int         i;
    
    static const char *ext[] = {
        "", ".my", ".smiv1", ".smiv2", ".sming", ".mib", ".txt", ".yang", NULL
    };
    
    if ((!modulename) || !strlen(modulename)) {
        return NULL;
    }

    if (!smiIsPath(modulename)) {
        /*
         * A plain modulename. Lookup the path along SMIPATH...
         */
        if (!smiHandle->path) {
            return NULL;
        }
	
        smipath = smiStrdup(smiHandle->path);
        sep[0] = PATH_SEPARATOR; sep[1] = 0;
        for (dir = strtok(smipath, sep);
             dir; dir = strtok(NULL, sep)) {
            for (i = 0; ext[i]; i++) {
                smiAsprintf(&path, "%s%c%s%s", dir, DIR_SEPARATOR, modulename, ext[i]);
                if (! access(path, R_OK)) {
                    break;
                }
                smiFree(path);
            }
            if (ext[i]) break;
            
            newmodulename = smiStrdup(modulename);
            for (i = 0; newmodulename[i]; i++) {
                newmodulename[i] = tolower(newmodulename[i]);
            }
            for (i = 0; ext[i]; i++) {
                smiAsprintf(&path, "%s%c%s%s", dir, DIR_SEPARATOR,
                    newmodulename, ext[i]);
                if (! access(path, R_OK)) {
                    break;
                }
                smiFree(path);
            }
            smiFree(newmodulename);
            if (ext[i]) break;            

            path = NULL;
        }
        smiFree(smipath);
    } else {
	/*
	 * A full path. Take it.
	 */
        path = smiStrdup(modulename);
    }

#if !defined(_MSC_VER) && !defined(__MINGW32__)
    if (!path && smiHandle->cache && smiHandle->cacheProg) {
        /*
	 * Not found in the path; now try to fetch & cache the module.
	 */
        int  pid;
        char *argv[4];
        char *cmd;
        int  status;
        smiAsprintf(&path, "%s%c%s", smiHandle->cache, DIR_SEPARATOR, modulename);
        if (access(path, R_OK)) {
            smiAsprintf(&cmd, "%s %s", smiHandle->cacheProg, modulename);
            pid = fork();
            if (pid != -1) {
            if (!pid) {
                argv[0] = "sh"; argv[1] = "-c"; argv[2] = cmd; argv[3] = 0;
                execv("/bin/sh", argv);
                exit(127);
            }
            waitpid(pid, &status, 0);
            }
            smiFree(cmd);
            if (access(path, R_OK)) {
            smiFree(path);
            path = NULL;
            }
        }
    }
#endif    
    return path;
}


SmiLanguage smiGuessFileLanguage(FILE *file)
{
    int i, c, ret = 0;
    
    while ((c = fgetc(file))) {
        if (c == '-' || isupper(c)) {
            ret = SMI_LANGUAGE_SMIV2;
            break;
        } else if (c == '/' || c == 'm' || c == 's')  {
            i = c;
            while ((c = fgetc(file))) {
		/*
		 * Check for statement termination:
		 *   "};" means sming
		 *   "}"  means yang
		 */
                if (i == '}' && c == ';') {
                    ret = SMI_LANGUAGE_SMING;
                    break;
                } else if(i == '}' && c != ';') {
                    ret = SMI_LANGUAGE_YANG;
                    break;
                } else if (c == EOF) {
		    return SMI_LANGUAGE_UNKNOWN;
                }
                i = c;
            }
            break;
        } else if (c == EOF || ! isspace(c)) {
            return SMI_LANGUAGE_UNKNOWN;
        }
    }
    rewind(file);
    return ret;
}


SmiLanguage smiGuessModuleLanguage(const char *modulename)
{
    char *path = NULL;
    FILE *file;
    SmiLanguage lang;
    
    path = smiGetModulePath(modulename);
    if (! path) {
        return SMI_LANGUAGE_UNKNOWN;
    }
    
    file = fopen(path, "r");
    if (! file) {
        return SMI_LANGUAGE_UNKNOWN;
    }

    lang = smiGuessFileLanguage(file);
    fclose(file);
    return lang;
}
