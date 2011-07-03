/*
 * smi.c --
 *
 *      Interface Implementation of libsmi.
 *
 * Copyright (c) 1999 Frank Strauss, Technical University of Braunschweig.
 *
 * See the file "COPYING" for information on usage and redistribution
 * of this file, and for a DISCLAIMER OF ALL WARRANTIES.
 *
 * @(#) $Id: smi.c 8071 2008-04-17 11:14:46Z schoenw $
 */

#include <config.h>

#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <ctype.h>
#include <sys/types.h>
#ifdef HAVE_UNISTD_H
#include <unistd.h>
#endif
#ifdef HAVE_PWD_H
#include <pwd.h>
#endif

#include "smi.h"
#include "data.h"
#include "error.h"
#include "util.h"
#include "snprintf.h"

#ifdef BACKEND_SMI
#include "scanner-smi.h"
#include "parser-smi.h"
#endif

#ifdef BACKEND_SMING
#include "scanner-sming.h"
#include "parser-sming.h"
#endif

#ifdef HAVE_DMALLOC_H
#include <dmalloc.h>
#endif



#ifndef MIN
#define MIN(a, b)       ((a) < (b) ? (a) : (b))
#define MAX(a, b)       ((a) < (b) ? (b) : (a))
#endif



const char *smi_library_version = SMI_LIBRARY_VERSION;
const char *smi_version_string = SMI_VERSION_STRING;

Handle *smiHandle = NULL;



/*
 * Internal functions.
 */

static void getModulenameAndName(const char *arg1, const char *arg2,
				 char **module, char **name)
{
    char	    *p;
    int		    l;

    if ((!arg1) && (!arg2)) {
	*module = NULL;
	*name = NULL;
    } else if (!arg2) {
	if (isupper((int)arg1[0])) {
	    if ((p = strstr(arg1, "::"))) {
		/* SMIng style module/label separator */
		*name = smiStrdup(&p[2]);
		l = strcspn(arg1, "::");
		*module = smiStrndup(arg1, l);
	    } else if ((p = strchr(arg1, '!'))) {
		/* old scotty style module/label separator */
		*name = smiStrdup(&p[1]);
		l = strcspn(arg1, "!");
		*module = smiStrndup(arg1, l);
	    } else if ((p = strchr(arg1, '.'))) {
		/* SMIv1/v2 style module/label separator */
		*name = smiStrdup(&p[1]);
		l = strcspn(arg1, ".");
		*module = smiStrndup(arg1, l);
	    } else {
		*name = smiStrdup(arg1);
		*module = smiStrdup("");
	    }
	} else {
	    *name = smiStrdup(arg1);
	    *module = smiStrdup("");
	}
    } else if (!arg1) {
	if (isupper((int)arg2[0])) {
	    if ((p = strstr(arg2, "::"))) {
		/* SMIng style module/label separator */
		*name = smiStrdup(&p[2]);
		l = strcspn(arg2, "::");
		*module = smiStrndup(arg2, l);
	    } else if ((p = strchr(arg2, '!'))) {
		/* old scotty style module/label separator */
		*name = smiStrdup(&p[1]);
		l = strcspn(arg2, "!");
		*module = smiStrndup(arg2, l);
	    } else if ((p = strchr(arg2, '.'))) {
		/* SMIv1/v2 style module/label separator */
		*name = smiStrdup(&p[1]);
		l = strcspn(arg2, ".");
		*module = smiStrndup(arg2, l);
	    } else {
		*name = smiStrdup(arg2);
		*module = smiStrdup("");
	    }
	} else {
	    *name = smiStrdup(arg2);
	    *module = smiStrdup("");
	}
    } else {
	*module = smiStrdup(arg1);
	*name = smiStrdup(arg2);
    }
}



static Node *getNode(unsigned int oidlen, SmiSubid oid[])
{
    Node *nodePtr, *parentPtr;
    unsigned int i;

    for(nodePtr = smiHandle->rootNodePtr, i=0; i < oidlen; i++) {
	parentPtr = nodePtr;
	nodePtr = findNodeByParentAndSubid(parentPtr, oid[i]);
	if (!nodePtr) {
	    return parentPtr;
	}
    }
    
    return nodePtr;
}



static Object *getNextChildObject(Node *startNodePtr, Module *modulePtr,
				  SmiNodekind nodekind)
{
    Node   *nodePtr;
    Object *objectPtr = NULL;

    if (!startNodePtr || !modulePtr)
	return NULL;

    for (nodePtr = startNodePtr; nodePtr; nodePtr = nodePtr->nextPtr) {
	for (objectPtr = nodePtr->firstObjectPtr; objectPtr;
	     objectPtr = objectPtr->nextSameNodePtr) {
	    if (((!modulePtr) || (objectPtr->modulePtr == modulePtr)) &&
		((nodekind == SMI_NODEKIND_ANY) ||
		 (nodekind & objectPtr->export.nodekind))) {
		break;
	    }
	}
	if (objectPtr) break;
	objectPtr = getNextChildObject(nodePtr->firstChildPtr,
				       modulePtr, nodekind);
	if (objectPtr) break;
    }

    return objectPtr;
}



/*
 * Interface Functions.
 */

int smiInit(const char *tag)
{
    char *p, *pp, *tag2;
#ifdef HAVE_PWD_H
    struct passwd *pw;
#endif

    smiHandle = findHandleByName(tag);
    if (smiHandle) {
	return 0;
    }
    smiHandle = addHandle(tag);
    
    smiDepth = 0;

    smiHandle->errorLevel = DEFAULT_ERRORLEVEL;
    smiHandle->errorHandler = smiErrorHandler;
#if !defined(_MSC_VER)
    smiHandle->cache = NULL;
    smiHandle->cacheProg = NULL;
#endif
    
    if (smiInitData()) {
	return -1;
    }

    /*
     * Setup the SMI MIB module search path:
     *  1. set to builtin DEFAULT_SMIPATH
     *  2. read global config file if present (append/prepend/replace)
     *  3. read user config file if present (append/prepend/replace)
     *  4. evaluate SMIPATH env-var if set (append/prepend/replace)
     */

    /* 1. set to builtin DEFAULT_SMIPATH */
    smiHandle->path = smiStrdup(DEFAULT_SMIPATH);

    tag2 = smiStrdup(tag);
    if (tag2) tag2 = strtok(tag2, ":");
    if (tag2) {
	/* 2. read global config file if present (append/prepend/replace) */
	smiReadConfig(DEFAULT_GLOBALCONFIG, tag2);
#ifdef HAVE_PWD_H
	pw = getpwuid(getuid());
	if (pw && pw->pw_dir) {
	    /* 3. read user config file if present (append/prepend/replace) */
	    smiAsprintf(&p, "%s%c%s",
			pw->pw_dir, DIR_SEPARATOR, DEFAULT_USERCONFIG);
	    smiReadConfig(p, tag2);
	    smiFree(p);
	}
#endif
    }
    smiFree(tag2);

    /* 4. evaluate SMIPATH env-var if set (append/prepend/replace) */
    p = getenv("SMIPATH");
    if (p) {
	if (p[0] == PATH_SEPARATOR) {
	    smiAsprintf(&pp, "%s%s", smiHandle->path, p);
	    smiFree(smiHandle->path);
	    smiHandle->path = pp;
	} else if (p[strlen(p)-1] == PATH_SEPARATOR) {
	    smiAsprintf(&pp, "%s%s", p, smiHandle->path);
	    smiFree(smiHandle->path);
	    smiHandle->path = pp;
	} else {
	    smiHandle->path = smiStrdup(p);
	}
    }
    
    if (!smiHandle->path) {
	return -1;
    }

    return 0;
}



void smiExit()
{
    if (!smiHandle)
	return;

    smiFreeData();

    smiFree(smiHandle->path);
#if !defined(_MSC_VER)
    smiFree(smiHandle->cache);
    smiFree(smiHandle->cacheProg);
#endif
    
    removeHandle(smiHandle);
    
    smiHandle = NULL;
    return;
}



char *smiGetPath()
{
    if (smiHandle->path) {
	return smiStrdup(smiHandle->path);
    } else {
	return NULL;
    }
}



int smiSetPath(const char *s)
{
    char *s2;

    if (!smiHandle) smiInit(NULL);

    if (!s) {
	smiFree(smiHandle->path);
	smiHandle->path = NULL;
	return 0;
    }
    
    s2 = smiStrdup(s);
    if (s2) {
	smiFree(smiHandle->path);
	smiHandle->path = s2;
	return 0;
    } else {
	return -1;
    }
    
}



void smiSetSeverity(char *pattern, int severity)
{
    smiSetErrorSeverity(pattern, severity);
}



int smiReadConfig(const char *filename, const char *tag)
{
    FILE *file;
    char buf[201];
    char *cmd, *arg, *s;
    
    file = fopen(filename, "r");
    if (file) {
	while (!feof(file)) {
	    if (!fgets(buf, 200, file)) continue;
	    if ((!strlen(buf)) || (buf[0] == '#')) continue;
	    cmd = strtok(buf, " \t\n\r");
	    if (!cmd) continue;
	    if (cmd[0] == '#') continue;
	    if (cmd[strlen(cmd)-1] == ':') {
		if (!tag) continue;
		cmd[strlen(cmd)-1] = 0;
		if (strcmp(cmd, tag)) continue;
		cmd = strtok(NULL, " \t\n\r");
	    }
	    arg = strtok(NULL, "\t\n\r");
	    if (!strcmp(cmd, "load")) {
		smiLoadModule(arg);
	    } else if (!strcmp(cmd, "path")) {
		if (arg) {
		    if (arg[0] == PATH_SEPARATOR) {
			smiAsprintf(&s, "%s%s", smiHandle->path, arg);
			smiFree(smiHandle->path);
			smiHandle->path = s;
		    } else if (arg[strlen(arg)-1] == PATH_SEPARATOR) {
			smiAsprintf(&s, "%s%s", arg, smiHandle->path);
			smiFree(smiHandle->path);
			smiHandle->path = s;
		    } else {
			smiHandle->path = smiStrdup(arg);
		    }
		}
	    } else if (!strcmp(cmd, "cache")) {
#if !defined(_MSC_VER)
		smiFree(smiHandle->cache);
		smiFree(smiHandle->cacheProg);
#endif
		if (arg && strcmp(arg, "off")) {
#if !defined(_MSC_VER)
		    smiHandle->cache = smiStrdup(arg);
		    arg = strtok(NULL, "\n\r");
		    smiHandle->cacheProg = smiStrdup(arg);
#else
		    smiPrintError(NULL, ERR_CACHE_CONFIG_NOT_SUPPORTED,
				  filename);
#endif
		}
	    } else if (!strcmp(cmd, "level")) {
		smiSetErrorLevel(atoi(arg));
	    } else if (!strcmp(cmd, "hide")) {
		smiSetSeverity(arg, 9);
	    } else {
		smiPrintError(NULL, ERR_UNKNOWN_CONFIG_CMD, cmd, filename);
	    }
	}
	fclose(file);
	return 0;
    }
    return -1;
}



int smiIsLoaded(const char *module)
{
    if (!module)
	return 0;
    
    return isInView(module);
}



char *smiLoadModule(const char *module)
{
    Module *modulePtr;
    
    if (!smiHandle) smiInit(NULL);

    if (smiIsPath(module)) {

	modulePtr = loadModule(module, NULL);

	if (modulePtr) {
	    if (!isInView(modulePtr->export.name)) {
		addView(modulePtr->export.name);
	    }
	    return modulePtr->export.name;
	} else {
	    return NULL;
	}

    } else {
	
	if ((modulePtr = findModuleByName(module))) {
	    /* already loaded. */
	    if (!isInView(module)) {
		addView(module);
	    }
	    return modulePtr->export.name;
	} else {
	    if ((modulePtr = loadModule(module, NULL))) {
		if (!isInView(module)) {
		    addView(module);
		}
		return modulePtr->export.name;
	    } else {
		return NULL;
	    }
	}
    }
}
 
void smiFreeModule(SmiModule *smiModulePtr)
{
    freeModule((Module*)smiModulePtr);
}


void smiSetErrorLevel(int level)
{
    if (!smiHandle) smiInit(NULL);
    
    smiHandle->errorLevel = level;
}



void smiSetFlags(int userflags)
{
    if (!smiHandle) smiInit(NULL);
    
    smiHandle->flags = (smiHandle->flags & ~SMI_FLAG_MASK) | userflags;
}



int smiGetFlags()
{
    if (!smiHandle) smiInit(NULL);
    
    return smiHandle->flags & SMI_FLAG_MASK;
}



SmiModule *smiGetModule(const char *module)
{
    Module	      *modulePtr;
    
    if (!module) {
	return NULL;
    }

    modulePtr = findModuleByName(module);
    
    if (!modulePtr) {
	modulePtr = loadModule(module, NULL);
    }
    
    return &modulePtr->export;
}



SmiModule *smiGetFirstModule()
{
    Module	      *modulePtr;

    for (modulePtr = smiHandle->firstModulePtr;
	 modulePtr && modulePtr->export.name &&
	     (strlen(modulePtr->export.name) == 0);
	 modulePtr = modulePtr->nextPtr);

    return &modulePtr->export;
}



SmiModule *smiGetNextModule(SmiModule *smiModulePtr)
{
    Module	      *modulePtr;
    
    if (!smiModulePtr) {
	return NULL;
    }

    
    for (modulePtr = ((Module *)smiModulePtr)->nextPtr;
	 modulePtr && modulePtr->export.name &&
	     (strlen(modulePtr->export.name) == 0);
	 modulePtr = modulePtr->nextPtr);

    return &modulePtr->export;
}



SmiImport *smiGetFirstImport(SmiModule *smiModulePtr)
{
    if (!smiModulePtr) {
	return NULL;
    }
    
    return &((Module *)smiModulePtr)->firstImportPtr->export;
}



SmiImport *smiGetNextImport(SmiImport *smiImportPtr)
{
    if (!smiImportPtr) {
	return NULL;
    }

    return &((Import *)smiImportPtr)->nextPtr->export;
}



int smiIsImported(SmiModule *smiModulePtr,
		  SmiModule *importedModulePtr,
		  char *importedName)
{
    Import	   *importPtr;
    Module	   *modulePtr;
    char	   *importedModule;
    
    if ((!smiModulePtr) || (!importedName)) {
	return 0;
    }

    modulePtr = (Module *)smiModulePtr;
    
    if (importedModulePtr) {
	importedModule = importedModulePtr->name;
    } else {
	importedModule = NULL;
    }
	
    for (importPtr = modulePtr->firstImportPtr; importPtr;
	 importPtr = importPtr->nextPtr) {
	if ((!strcmp(importedName, importPtr->export.name)) &&
	    ((!importedModule) ||
	     (!strcmp(importedModule, importPtr->export.module)))) {
	    return 1;
	}
    }

    return 0;
}



SmiRevision *smiGetFirstRevision(SmiModule *smiModulePtr)
{
    if (!smiModulePtr) {
	return NULL;
    }
    
    return &((Module *)smiModulePtr)->firstRevisionPtr->export;
}



SmiRevision *smiGetNextRevision(SmiRevision *smiRevisionPtr)
{
    if (!smiRevisionPtr) {
	return NULL;
    }

    return &((Revision *)smiRevisionPtr)->nextPtr->export;
}



int smiGetRevisionLine(SmiRevision *smiRevisionPtr)
{
    return ((Revision *)smiRevisionPtr)->line;
}



SmiType *smiGetType(SmiModule *smiModulePtr, char *type)
{
    Type	    *typePtr = NULL;
    Module	    *modulePtr = NULL;
    char	    *module2, *type2;

    if (!type) {
	return NULL;
    }

    modulePtr = (Module *)smiModulePtr;

    getModulenameAndName(smiModulePtr ? smiModulePtr->name : NULL, type,
			 &module2, &type2);
    
    if (!modulePtr && module2 && strlen(module2)) {
	if (!(modulePtr = findModuleByName(module2))) {
	    modulePtr = loadModule(module2, NULL);
	}
    }

    if (modulePtr) {
	typePtr = findTypeByModuleAndName(modulePtr, type2);
    } else {
	typePtr = findTypeByName(type2);
    }
    
    smiFree(module2);
    smiFree(type2);

    if (!typePtr ||
	typePtr->export.basetype == SMI_BASETYPE_UNKNOWN) {
	return NULL;
    }
    
    return &typePtr->export;
}



SmiType *smiGetFirstType(SmiModule *smiModulePtr)
{
    Type *typePtr;
    
    if (!smiModulePtr) {
	return NULL;
    }
    
    for (typePtr = ((Module *)smiModulePtr)->firstTypePtr; typePtr;
	 typePtr = typePtr->nextPtr) {
	/* loop until we found a `real' type */
	if (typePtr->export.name &&
	    typePtr->export.basetype != SMI_BASETYPE_UNKNOWN) {
	    break;
	}
    }
    
    return &typePtr->export;
}



SmiType *smiGetNextType(SmiType *smiTypePtr)
{
    Type *typePtr;

    if (!smiTypePtr) {
	return NULL;
    }

    for (typePtr = ((Type *)smiTypePtr)->nextPtr; typePtr;
	 typePtr = typePtr->nextPtr) {
	/* loop until we found a `real' type */
	if (typePtr->export.name &&
	    typePtr->export.basetype != SMI_BASETYPE_UNKNOWN) {
	    break;
	}
    }
    
    return &typePtr->export;
}


SmiType *smiGetParentType(SmiType *smiTypePtr)
{
    Type *typePtr;

    if (!smiTypePtr) {
	return NULL;
    }

    typePtr = ((Type *)smiTypePtr)->parentPtr;
    
    if (!typePtr ||
	typePtr->export.basetype == SMI_BASETYPE_UNKNOWN) {
	return NULL;
    }
    
    return &typePtr->export;
}



SmiModule *smiGetTypeModule(SmiType *smiTypePtr)
{
    return &((Type *)smiTypePtr)->modulePtr->export;
}

int smiGetTypeLine(SmiType *smiTypePtr)
{
    return ((Type *)smiTypePtr)->line;
}



SmiNamedNumber *smiGetFirstNamedNumber(SmiType *smiTypePtr)
{
    Type    *typePtr;

    typePtr = (Type *)smiTypePtr;
    
    if ((!typePtr) || (!typePtr->listPtr) ||
	((typePtr->export.basetype != SMI_BASETYPE_ENUM) &&
	 (typePtr->export.basetype != SMI_BASETYPE_BITS) && 
	 (typePtr->export.basetype != SMI_BASETYPE_POINTER))) {
	return NULL;
    }
    
    return &((NamedNumber *)typePtr->listPtr->ptr)->export;
}



SmiNamedNumber *smiGetNextNamedNumber(SmiNamedNumber *smiNamedNumberPtr)
{
    Type  *typePtr;
    List  *listPtr;
    
    if (!smiNamedNumberPtr) {
	return NULL;
    }
    
    typePtr = ((NamedNumber *)smiNamedNumberPtr)->typePtr;

    
    if ((!typePtr) || (!typePtr->listPtr) ||
	((typePtr->export.basetype != SMI_BASETYPE_ENUM) &&
	 (typePtr->export.basetype != SMI_BASETYPE_BITS))) {
	return NULL;
    }

    for (listPtr = typePtr->listPtr; listPtr; listPtr = listPtr->nextPtr) {
	if (((NamedNumber *)(listPtr->ptr))->export.name ==
	                                               smiNamedNumberPtr->name)
	    break;
    }

    if ((!listPtr) || (!listPtr->nextPtr)) {
	return NULL;
    }
	
    return &((NamedNumber *)listPtr->nextPtr->ptr)->export;
}

SmiNamedNumber *smiGetAttributeFirstNamedNumber(SmiAttribute *smiAttributePtr)
{
    Attribute    *attributePtr;

    attributePtr = (Attribute *)smiAttributePtr;
    
    if ((!attributePtr) || (!attributePtr->listPtr) ||
	((attributePtr->export.basetype != SMI_BASETYPE_ENUM) &&
	 (attributePtr->export.basetype != SMI_BASETYPE_BITS) && 
	 (attributePtr->export.basetype != SMI_BASETYPE_POINTER))) {
	return NULL;
    }
    
    return &((NamedNumber *)attributePtr->listPtr->ptr)->export;
}



SmiNamedNumber *smiGetAttributeNextNamedNumber(SmiNamedNumber *smiNamedNumberPtr)
{
    Attribute  *attributePtr;
    List  *listPtr;
    
    if (!smiNamedNumberPtr) {
	return NULL;
    }
    
    attributePtr = (Attribute*)(((NamedNumber *)smiNamedNumberPtr)->typePtr);

    
    if ((!attributePtr) || (!attributePtr->listPtr) ||
	((attributePtr->export.basetype != SMI_BASETYPE_ENUM) &&
	 (attributePtr->export.basetype != SMI_BASETYPE_BITS))) {
	return NULL;
    }

    for (listPtr = attributePtr->listPtr; listPtr; listPtr = listPtr->nextPtr) {
	if (((NamedNumber *)(listPtr->ptr))->export.name ==
	                                               smiNamedNumberPtr->name)
	    break;
    }

    if ((!listPtr) || (!listPtr->nextPtr)) {
	return NULL;
    }
	
    return &((NamedNumber *)listPtr->nextPtr->ptr)->export;
}

SmiRange *smiGetFirstRange(SmiType *smiTypePtr)
{
    Type    *typePtr;

    typePtr = (Type *)smiTypePtr;
    
    if ((!typePtr) || (!typePtr->listPtr) ||
	(typePtr->export.basetype == SMI_BASETYPE_ENUM) ||
	(typePtr->export.basetype == SMI_BASETYPE_BITS)) {
	return NULL;
    }

    return &((Range *)typePtr->listPtr->ptr)->export;
}



SmiRange *smiGetNextRange(SmiRange *smiRangePtr)
{
    Type  *typePtr;
    List  *listPtr;

    if (!smiRangePtr) {
	return NULL;
    }
    
    typePtr = ((Range *)smiRangePtr)->typePtr;

    if ((!typePtr) || (!typePtr->listPtr) ||
	(typePtr->export.basetype == SMI_BASETYPE_ENUM) ||
	(typePtr->export.basetype == SMI_BASETYPE_BITS)) {
	return NULL;
    }
 
    for (listPtr = typePtr->listPtr; listPtr; listPtr = listPtr->nextPtr) {
	if (!memcmp(&((Range *)listPtr->ptr)->export.minValue,
		    &smiRangePtr->minValue, sizeof(struct SmiValue)))
	    break;
    }

    if ((!listPtr) || (!listPtr->nextPtr)) {
	return NULL;
    }
	
    return &((Range *)listPtr->nextPtr->ptr)->export;
}

SmiRange *smiGetAttributeFirstRange(SmiAttribute *smiAttributePtr)
{
    Attribute    *attributePtr;

    attributePtr = (Attribute *)smiAttributePtr;
    
    if ((!attributePtr) || (!attributePtr->listPtr) ||
	(attributePtr->export.basetype == SMI_BASETYPE_ENUM) ||
	(attributePtr->export.basetype == SMI_BASETYPE_BITS)) {
	return NULL;
    }

    return &((Range *)attributePtr->listPtr->ptr)->export;
}



SmiRange *smiGetAttributeNextRange(SmiRange *smiRangePtr)
{
    Attribute  *attributePtr;
    List  *listPtr;

    if (!smiRangePtr) {
	return NULL;
    }
    
    attributePtr = (Attribute*)((Range *)smiRangePtr)->typePtr;

    if ((!attributePtr) || (!attributePtr->listPtr) ||
	(attributePtr->export.basetype == SMI_BASETYPE_ENUM) ||
	(attributePtr->export.basetype == SMI_BASETYPE_BITS)) {
	return NULL;
    }
 
    for (listPtr = attributePtr->listPtr; listPtr; listPtr = listPtr->nextPtr) {
	if (!memcmp(&((Range *)listPtr->ptr)->export.minValue,
		    &smiRangePtr->minValue, sizeof(struct SmiValue)))
	    break;
    }

    if ((!listPtr) || (!listPtr->nextPtr)) {
	return NULL;
    }
	
    return &((Range *)listPtr->nextPtr->ptr)->export;
}


SmiIdentity *smiGetFirstIdentity(SmiModule *smiModulePtr)
{
	if (!smiModulePtr) {
	return NULL;
    }
    
    return ((Module *)smiModulePtr)->firstIdentityPtr ?
	&((Module *)smiModulePtr)->firstIdentityPtr->export : NULL;
    
}

SmiIdentity *smiGetNextIdentity(SmiIdentity *smiIdentityPtr)
{
    if (!smiIdentityPtr) {
	return NULL;
    }

    return ((Identity *)smiIdentityPtr)->nextPtr ?
	&((Identity *)smiIdentityPtr)->nextPtr->export : NULL;
}

SmiModule *smiGetIdentityModule(SmiIdentity *smiIdentityPtr)
{
    return &((Identity *)smiIdentityPtr)->modulePtr->export;
}

SmiIdentity *smiGetParentIdentity(SmiIdentity *smiIdentityPtr)
{
    return (SmiIdentity*)(((Identity *)smiIdentityPtr)->parentPtr);
}

SmiIdentity *smiGetIdentity(SmiModule *smiModulePtr, char *identity)
{
	
	if (!smiModulePtr) {
	return NULL;
    }
    else
    {
    	SmiIdentity *ide; 
    	
    	for(ide = smiGetFirstIdentity(smiModulePtr); 
    		ide;
    		ide = smiGetNextIdentity(ide))
    			if(!strncmp(ide->name,identity,64))return ide;
    		
    	return NULL;
    }
    
}

int smiGetIdentityLine(SmiIdentity *smiIdentityPtr)
{
    return ((Identity *)smiIdentityPtr)->line;
}

	
SmiClass *smiGetFirstClass(SmiModule *smiModulePtr)
{
	if (!smiModulePtr) {
	return NULL;
    }
    
    return ((Module *)smiModulePtr)->firstClassPtr ?
	&((Module *)smiModulePtr)->firstClassPtr->export : NULL;
    
}

SmiClass *smiGetNextClass(SmiClass *smiClassPtr)
{
    if (!smiClassPtr) {
	return NULL;
    }

    return ((Class *)smiClassPtr)->nextPtr ?
	&((Class *)smiClassPtr)->nextPtr->export : NULL;
}

SmiModule *smiGetClassModule(SmiClass *smiClassPtr)
{
    return &((Class *)smiClassPtr)->modulePtr->export;
}

SmiClass *smiGetParentClass(SmiClass *smiClassPtr)
{
    return (SmiClass*)(((Class *)smiClassPtr)->parentPtr);
}

SmiClass *smiGetClass(SmiModule *smiModulePtr, char *class)
{
	
	if (!smiModulePtr) {
	return NULL;
    }
    else
    {
    	SmiClass *cl; 
    	
    	for(cl = smiGetFirstClass(smiModulePtr); 
    		cl;
    		cl = smiGetNextClass(cl))
    			if(!strncmp(cl->name,class,64))return cl;
    		
    	return NULL;
    }
    
}

int smiGetClassLine(SmiClass *smiClassPtr)
{
    return ((Class *)smiClassPtr)->line;
}

SmiAttribute *smiGetFirstAttribute(SmiClass *smiClassPtr)
{
    Attribute *attributePtr;
    
    if (!smiClassPtr) {
	return NULL;
    }
    
  	attributePtr = ((Class *)smiClassPtr)->firstAttributePtr;
    
    return &attributePtr->export;
}

 SmiAttribute *smiGetNextAttribute( SmiAttribute *smiTypePtr)
{
    Attribute *attributePtr;

    if (!smiTypePtr) {
	return NULL;
    }

    attributePtr = ((Attribute *)smiTypePtr)->nextPtr;
    
    return &attributePtr->export;
}

SmiAttribute *smiGetAttribute(SmiClass *smiClassPtr, char *attribute)
{
    Attribute *attributePtr;
    
    if (! smiClassPtr) {
	return NULL;
    }
    
    attributePtr = ((Class *)smiClassPtr)->firstAttributePtr;
    
    for (attributePtr = ((Class *)smiClassPtr)->firstAttributePtr; 
	 attributePtr; attributePtr = attributePtr->nextPtr)
    {
	if (!strncmp(attributePtr->export.name, attribute,64)) {
	    return &attributePtr->export;
	}
    }
    
    /*
     * attribute might belong to the parent so check parent if
     * attribute not found
     */
    
    smiClassPtr = smiGetParentClass(smiClassPtr);
    attributePtr = (Attribute*)smiGetAttribute(smiClassPtr , attribute);
  		
    return &attributePtr->export;
}

SmiType *smiGetAttributeParentType(SmiAttribute *smiAttributePtr)
{
    Type *parentTypePtr;
    
    if (! smiAttributePtr) {
	return NULL;
    }
    
    parentTypePtr = ((Attribute*)smiAttributePtr)->parentTypePtr;
    
    return (parentTypePtr) ? &parentTypePtr->export : NULL;
}

SmiClass *smiGetAttributeParentClass( SmiAttribute *smiAttributePtr)
{
    Class *parentClassPtr;
    
    if (! smiAttributePtr) {
	return NULL;
    }
    
    parentClassPtr = ((Attribute*)smiAttributePtr)->parentClassPtr;

    return parentClassPtr ? &parentClassPtr->export : NULL;
}

SmiAttribute *smiGetFirstUniqueAttribute(SmiClass *smiClassPtr)
{
    Class *classPtr;
    
    if (! smiClassPtr) {
	return NULL;
    }
	
    classPtr = (Class*)smiClassPtr;
	
    if (! classPtr->uniqueList) {
	return NULL;
    }
    
    if (classPtr->uniqueList->ptr == classPtr) {
	return NULL; /* scalar class */
    }

    return (SmiAttribute*)(classPtr->uniqueList->ptr);
}

SmiAttribute *smiGetNextUniqueAttribute( SmiAttribute *smiTypePtr)
{
    Class *classPtr;
    List  *listPtr; 
    
    if (! smiTypePtr) {
	return NULL;
    }
	
    classPtr = ((Attribute*)smiTypePtr)->classPtr;
    
    if (classPtr && classPtr->uniqueList) {
	for (listPtr=classPtr->uniqueList;listPtr; listPtr=listPtr->nextPtr) {
	    if (&((Attribute*)(listPtr->ptr))->export ==  smiTypePtr) {	
		if (listPtr->nextPtr) {
		    return &((Attribute*)(listPtr->nextPtr->ptr))->export;
		}
	    }
	}
    }
    return NULL;
}



int smiGetAttributeLine(SmiAttribute *smiAttributePtr)
{
    return ((Attribute *)smiAttributePtr)->line;
}



int smiIsClassScalar(SmiClass *smiClassPtr)
{
    Class *classPtr;
    
    if (! smiClassPtr) {
	return 0;
    }
	
    classPtr = (Class*)smiClassPtr;
    
    if (! classPtr->uniqueList) {
	return 0;
    }

    return (classPtr->uniqueList->ptr == classPtr);
}



SmiEvent *smiGetFirstEvent(SmiClass *smiClassPtr)
{
    Event *eventPtr;
    
    if (! smiClassPtr) {
	return NULL;
    }
    
    eventPtr = ((Class *)smiClassPtr)->firstEventPtr;
    return &(eventPtr->export);
}



SmiEvent *smiGetNextEvent(SmiEvent *smiEventPtr)
{
    Event *eventPtr;

    if (! smiEventPtr) {
	return NULL;
    }

    eventPtr = ((Event *)smiEventPtr)->nextPtr;
    return &eventPtr->export;
}



int smiGetEventLine(SmiEvent *smiEventPtr)
{
    return ((Event *)smiEventPtr)->line;
}

	

SmiMacro *smiGetMacro(SmiModule *smiModulePtr, char *macro)
{
    Macro	    *macroPtr = NULL;
    Module	    *modulePtr = NULL;
    char	    *module2, *macro2;
    
    if (!macro) {
	return NULL;
    }

    modulePtr = (Module *)smiModulePtr;

    getModulenameAndName(smiModulePtr ? smiModulePtr->name : NULL, macro,
			 &module2, &macro2);
    
    if (!modulePtr && module2 && strlen(module2)) {
	if (!(modulePtr = findModuleByName(module2))) {
	    modulePtr = loadModule(module2, NULL);
	}
    }

    if (modulePtr) {
	macroPtr = findMacroByModuleAndName(modulePtr, macro2);
    } else {
	macroPtr = findMacroByName(macro2);
    }
    
    smiFree(module2);
    smiFree(macro2);
    return macroPtr ? &macroPtr->export : NULL;
}



SmiMacro *smiGetFirstMacro(SmiModule *smiModulePtr)
{
    if (!smiModulePtr) {
	return NULL;
    }
    
    return ((Module *)smiModulePtr)->firstMacroPtr ?
	&((Module *)smiModulePtr)->firstMacroPtr->export : NULL;
}



SmiMacro *smiGetNextMacro(SmiMacro *smiMacroPtr)
{
    if (!smiMacroPtr) {
	return NULL;
    }

    return ((Macro *)smiMacroPtr)->nextPtr ?
	&((Macro *)smiMacroPtr)->nextPtr->export : NULL;
}


SmiModule *smiGetMacroModule(SmiMacro *smiMacroPtr)
{
    return &((Macro *)smiMacroPtr)->modulePtr->export;
}


int smiGetMacroLine(SmiMacro *smiMacroPtr)
{
    return ((Macro *)smiMacroPtr)->line;
}


SmiNode *smiGetNode(SmiModule *smiModulePtr, const char *node)
{
    Object	    *objectPtr = NULL;
    Module	    *modulePtr = NULL;
    Node            *nodePtr;
    char	    *module2, *node2, *p;
    unsigned int    oidlen;
    SmiSubid	    oid[128];
    
    if (!node) {
	return NULL;
    }

    modulePtr = (Module *)smiModulePtr;

    getModulenameAndName(smiModulePtr ? smiModulePtr->name : NULL, node,
			 &module2, &node2);
    
    if (!modulePtr && module2 && strlen(module2)) {
	if (!(modulePtr = findModuleByName(module2))) {
	    modulePtr = loadModule(module2, NULL);
	}
    }

    if (isdigit((int)node2[0])) {
	for (oidlen = 0, p = strtok(node2, ". ");
	     p && oidlen < sizeof(oid)/sizeof(oid[0]);
	     oidlen++, p = strtok(NULL, ". ")) {
	    oid[oidlen] = strtoul(p, NULL, 0);
	}
	if (p) {
            /* the numeric OID is too long */
            return NULL;
	}
	nodePtr = getNode(oidlen, oid);
	if (nodePtr) {
	    if (modulePtr) {
		objectPtr = findObjectByModuleAndNode(modulePtr, nodePtr);
	    } else {
		objectPtr = findObjectByNode(nodePtr);
	    }
	}
    } else {
	p = strtok(node2, ". ");
	if (modulePtr) {
	    objectPtr = findObjectByModuleAndName(modulePtr, p);
	} else {
	    objectPtr = findObjectByName(p);
	}
    }
    
    smiFree(module2);
    smiFree(node2);
    return objectPtr ? &objectPtr->export : NULL;
}



SmiNode *smiGetNodeByOID(unsigned int oidlen, SmiSubid oid[])
{
    Node            *nodePtr;
    Object	    *objectPtr;
    
    if (!oidlen) {
	return NULL;
    }

    nodePtr = getNode(oidlen, oid);

    if (!nodePtr) {
	return NULL;
    }
    
    objectPtr = findObjectByNode(nodePtr);

    return objectPtr ? &objectPtr->export : NULL;
}



SmiNode *smiGetFirstNode(SmiModule *smiModulePtr, SmiNodekind nodekind)
{
    Module *modulePtr;
    Node   *nodePtr = NULL;
    Object *objectPtr;

    if (!smiModulePtr) {
	return NULL;
    }
    
    modulePtr = (Module *)smiModulePtr;

    if (modulePtr && modulePtr->prefixNodePtr) {
	/* start at the common oid prefix of this module */
	nodePtr = modulePtr->prefixNodePtr;
    } else {
	nodePtr = smiHandle->rootNodePtr->firstChildPtr;
    }

    do {
	objectPtr = getNextChildObject(nodePtr, modulePtr, nodekind);
	
	if (objectPtr)
	    return &objectPtr->export;
	
	if (nodePtr->firstChildPtr) {
	    nodePtr = nodePtr->firstChildPtr;
	} else if (nodePtr->nextPtr) {
	    nodePtr = nodePtr->nextPtr;
	} else {
	    for (nodePtr = nodePtr->parentPtr;
		 nodePtr && (nodePtr->parentPtr) && (!nodePtr->nextPtr);
		 nodePtr = nodePtr->parentPtr);
	    if (nodePtr) nodePtr = nodePtr->nextPtr;
	}
    } while (nodePtr);

    return NULL;
}



SmiNode *smiGetNextNode(SmiNode *smiNodePtr, SmiNodekind nodekind)
{
    Module	      *modulePtr;
    Object	      *objectPtr;
    Node	      *nodePtr;
    int               i;
    
    if (!smiNodePtr) {
	return NULL;
    }

    objectPtr = (Object *)smiNodePtr;
    nodePtr = objectPtr->nodePtr;
    modulePtr = objectPtr->modulePtr;

    if (!modulePtr) {
	return NULL;
    }

    if (!nodePtr) {
	return NULL;
    }

    do {
	if (nodePtr->firstChildPtr) {
	    nodePtr = nodePtr->firstChildPtr;
	} else if (nodePtr->nextPtr) {
	    nodePtr = nodePtr->nextPtr;
	} else {
	    for (nodePtr = nodePtr->parentPtr;
		 (nodePtr->parentPtr) && (!nodePtr->nextPtr);
		 nodePtr = nodePtr->parentPtr);
	    nodePtr = nodePtr->nextPtr;
	    /* did we move outside the common oid prefix of this module? */
	    for (i = 0; i < modulePtr->prefixNodePtr->oidlen; i++)
		if ((!nodePtr) || (!nodePtr->oid) ||
		    (nodePtr->oid[i] != modulePtr->prefixNodePtr->oid[i]))
		    return NULL;
	}

	objectPtr = getNextChildObject(nodePtr, modulePtr, nodekind);

	if (objectPtr)
	    return &objectPtr->export;
	
    } while (nodePtr);

    return NULL;
}



SmiNode *smiGetParentNode(SmiNode *smiNodePtr)
{
    Module	      *modulePtr;
    Object	      *objectPtr;
    Import	      *importPtr;
    Node	      *nodePtr;
    
    if (!smiNodePtr) {
	return NULL;
    }

    objectPtr = (Object *)smiNodePtr;
    nodePtr = objectPtr->nodePtr;
    modulePtr = objectPtr->modulePtr;

    if (!nodePtr) {
	return NULL;
    }

    if (nodePtr == smiHandle->rootNodePtr) {
	return NULL;
    }

    nodePtr = nodePtr->parentPtr;
    if (! nodePtr) {
        return NULL;
    }

    /*
     * First, try to find a definition in the same module.
     */
    objectPtr = NULL;
    if (modulePtr) {
	objectPtr = findObjectByModuleAndNode(modulePtr, nodePtr);
    }

    /*
     * If found, check if it's imported. In case, get the original definition.
     */
    if (objectPtr) {
	importPtr = findImportByName(objectPtr->export.name,
				     objectPtr->modulePtr);
	if (importPtr) {
	    objectPtr = findObjectByModulenameAndNode(importPtr->export.module,
						      nodePtr);
	} else {
	    objectPtr = NULL;
	}
    }
    
    /*
     * If not yet found, try to find any definition.
     */
    if (!objectPtr) {
	objectPtr = findObjectByNode(nodePtr);

	if ((!objectPtr) && (nodePtr->parentPtr)) {
	    /* an implicitly created node, e.g. gaga.0 in an object
	     * definition with oid == gaga.0.1.
	     */
	    objectPtr = addObject(SMI_UNKNOWN_LABEL,
				  nodePtr->parentPtr, nodePtr->subid,
				  0, NULL);
	    objectPtr->nodePtr = nodePtr;
	    objectPtr->modulePtr = modulePtr;
	}
    }

    return objectPtr ? &objectPtr->export : NULL;
}



SmiNode *smiGetRelatedNode(SmiNode *smiNodePtr)
{
    if (!smiNodePtr) {
	return NULL;
    }

    return &((Object *)smiNodePtr)->relatedPtr->export;
}



SmiNode *smiGetFirstChildNode(SmiNode *smiNodePtr)
{
    Module	      *modulePtr;
    Object	      *objectPtr;
    Node	      *nodePtr;

    if (!smiNodePtr) {
	return NULL;
    }

    objectPtr = (Object *)smiNodePtr;
    nodePtr = objectPtr->nodePtr;
    modulePtr = objectPtr->modulePtr;

    if (!nodePtr) {
	return NULL;
    }

    nodePtr = nodePtr->firstChildPtr;

    if (!nodePtr) {
	return NULL;
    }

    objectPtr = findObjectByModuleAndNode(modulePtr, nodePtr);
    if (!objectPtr) objectPtr = findObjectByNode(nodePtr);

    return objectPtr ? &objectPtr->export : NULL;
}



SmiNode *smiGetNextChildNode(SmiNode *smiNodePtr)
{
    Module	      *modulePtr;
    Object	      *objectPtr;
    Node	      *nodePtr;
    
    if (!smiNodePtr) {
	return NULL;
    }

    objectPtr = (Object *)smiNodePtr;
    nodePtr = objectPtr->nodePtr;
    modulePtr = objectPtr->modulePtr;

    if (!nodePtr) {
	return NULL;
    }

    nodePtr = nodePtr->nextPtr;

    if (!nodePtr) {
	return NULL;
    }

    objectPtr = findObjectByModuleAndNode(modulePtr, nodePtr);
    if (!objectPtr) objectPtr = findObjectByNode(nodePtr);
    
    return objectPtr ? &objectPtr->export : NULL;
}



SmiNode *smiGetModuleIdentityNode(SmiModule *smiModulePtr)
{
    if (!smiModulePtr) {
	return NULL;
    }

    return &((Module *)smiModulePtr)->objectPtr->export;
}



SmiModule *smiGetNodeModule(SmiNode *smiNodePtr)
{
    return &((Object *)smiNodePtr)->modulePtr->export;
}



SmiType *smiGetNodeType(SmiNode *smiNodePtr)
{
    Type *typePtr;

    typePtr = ((Object *)smiNodePtr)->typePtr;
    
    if (!typePtr ||
	typePtr->export.basetype == SMI_BASETYPE_UNKNOWN) {
	return NULL;
    }
    
    return &typePtr->export;
}



int smiGetNodeLine(SmiNode *smiNodePtr)
{
    return ((Object *)smiNodePtr)->line;
}



SmiElement *smiGetFirstElement(SmiNode *smiNodePtr)
{
    List	      *listPtr;
    
    if (!smiNodePtr) {
	return NULL;
    }

    listPtr = ((Object *)smiNodePtr)->listPtr;

    return (SmiElement *)listPtr;
}



SmiElement *smiGetNextElement(SmiElement *smiElementPtr)
{
    List	      *listPtr;
    
    if (!smiElementPtr) {
	return NULL;
    }

    listPtr = ((List *)smiElementPtr)->nextPtr;
    
    return (SmiElement *)listPtr;
}



SmiNode *smiGetElementNode(SmiElement *smiElementPtr)
{
    if ((Object *)((List *)smiElementPtr)->ptr)
        return &((Object *)((List *)smiElementPtr)->ptr)->export;
    else
        return NULL;
}



SmiOption *smiGetFirstOption(SmiNode *smiComplianceNodePtr)
{
    Object	      *objectPtr;
    
    if (!smiComplianceNodePtr) {
	return NULL;
    }

    objectPtr = (Object *)smiComplianceNodePtr;
    
    if (!objectPtr->optionlistPtr) {
	return NULL;
    }

    if (objectPtr->export.nodekind != SMI_NODEKIND_COMPLIANCE) {
	return NULL;
    }
						     
    return &((Option *)objectPtr->optionlistPtr->ptr)->export;
}



SmiOption *smiGetNextOption(SmiOption *smiOptionPtr)
{
    List	      *listPtr;
    
    if (!smiOptionPtr) {
	return NULL;
    }
						     
    for (listPtr =
	    ((Option *)smiOptionPtr)->compliancePtr->optionlistPtr;
	 listPtr;
	 listPtr = listPtr->nextPtr) {
	if ((Option *)(listPtr->ptr) == (Option *)smiOptionPtr) {
	    if (listPtr->nextPtr) {
		return &((Option *)listPtr->nextPtr->ptr)->export;
	    } else {
		return NULL;
	    }
	}
    }
    
    return NULL;
}



SmiNode *smiGetOptionNode(SmiOption *smiOptionPtr)
{
    return &((Option *)smiOptionPtr)->objectPtr->export;
}



int smiGetOptionLine(SmiOption *smiOptionPtr)
{
    return ((Option *)smiOptionPtr)->line;
}



SmiRefinement *smiGetFirstRefinement(SmiNode *smiComplianceNodePtr)
{
    Object	      *objectPtr;
    
    if (!smiComplianceNodePtr) {
	return NULL;
    }

    objectPtr = (Object *)smiComplianceNodePtr;
    
    if (!objectPtr->refinementlistPtr) {
	return NULL;
    }

    if (objectPtr->export.nodekind != SMI_NODEKIND_COMPLIANCE) {
	return NULL;
    }
						     
    return &((Refinement *)objectPtr->refinementlistPtr->ptr)->export;
}



SmiRefinement *smiGetNextRefinement(SmiRefinement *smiRefinementPtr)
{
    List	      *listPtr;
    
    if (!smiRefinementPtr) {
	return NULL;
    }
						     
    for (listPtr =
	    ((Refinement *)smiRefinementPtr)->compliancePtr->refinementlistPtr;
	 listPtr;
	 listPtr = listPtr->nextPtr) {
	if ((Refinement *)(listPtr->ptr) == (Refinement *)smiRefinementPtr) {
	    if (listPtr->nextPtr) {
		return &((Refinement *)listPtr->nextPtr->ptr)->export;
	    } else {
		return NULL;
	    }
	}
    }
    
    return NULL;
}



SmiNode *smiGetRefinementNode(SmiRefinement *smiRefinementPtr)
{
    return &((Refinement *)smiRefinementPtr)->objectPtr->export;
}



SmiType *smiGetRefinementType(SmiRefinement *smiRefinementPtr)
{
    Type *typePtr;

    typePtr = ((Refinement *)smiRefinementPtr)->typePtr;
    
    if (!typePtr ||
	typePtr->export.basetype == SMI_BASETYPE_UNKNOWN) {
	return NULL;
    }
    
    return &typePtr->export;
}



SmiType *smiGetRefinementWriteType(SmiRefinement *smiRefinementPtr)
{
    Type *typePtr;

    typePtr = ((Refinement *)smiRefinementPtr)->writetypePtr;
    
    if (!typePtr ||
	typePtr->export.basetype == SMI_BASETYPE_UNKNOWN) {
	return NULL;
    }
    
    return &typePtr->export;
}



int smiGetRefinementLine(SmiRefinement *smiRefinementPtr)
{
    return ((Refinement *)smiRefinementPtr)->line;
}



SmiElement *smiGetFirstUniquenessElement(SmiNode *smiNodePtr)
{
    List	      *listPtr;
    
    if (!smiNodePtr) {
	return NULL;
    }

    listPtr = ((Object *)smiNodePtr)->uniquenessPtr;

    return (SmiElement *)listPtr;
}



char *smiRenderOID(unsigned int oidlen, SmiSubid *oid, int flags)
{
    SmiNode *nodePtr = NULL;
    SmiModule *modulePtr = NULL;
    unsigned int i = 0;
    char *ss, *s = NULL;

    if (!oid) {
	if (flags & SMI_RENDER_UNKNOWN) {
	    smiAsprintf(&s, SMI_UNKNOWN_LABEL);
	} else {
	    s = NULL;
	}
	return s;
    }
    
    if (flags & (SMI_RENDER_NAME | SMI_RENDER_QUALIFIED)) {
	int len;
	for (len = oidlen; len; len--) {
	    nodePtr = smiGetNodeByOID(len, oid);
	    if (! nodePtr || nodePtr->name) break;
	}
	if (nodePtr && nodePtr->name) {
	    i = nodePtr->oidlen;
	    if (flags & SMI_RENDER_QUALIFIED) {
		modulePtr = smiGetNodeModule(nodePtr);
	    }
	    if (modulePtr) {
		smiAsprintf(&s, "%s::%s",
			    modulePtr->name, nodePtr->name);
	    } else {
		smiAsprintf(&s, "%s", nodePtr->name);
	    }
	}
    }

    for (; i < oidlen; i++) {
	ss = s;
	smiAsprintf(&s, "%s%s%u", ss ? ss : "", i ? "." : "", oid[i]);
	smiFree(ss);
    }

    if ((!s) && (flags & SMI_RENDER_UNKNOWN)) {
	smiAsprintf(&s, SMI_UNKNOWN_LABEL);
    }
    
    return s;
}



char *smiRenderValue(SmiValue *smiValuePtr, SmiType *smiTypePtr, int flags)
{
    unsigned int i, pfx;
    int j, k, n, have_pfx;
    char *last_fmt, *fmt;
    SmiUnsigned64 vv;
    int xlen;
    SmiNamedNumber *nn;
    char *s, *ss;
    char f[15];
    SmiUnsigned32 v32;
    SmiUnsigned64 v64;
    
    if (!smiValuePtr) {
	if (flags & SMI_RENDER_UNKNOWN) {
	    smiAsprintf(&s, SMI_UNKNOWN_LABEL);
	} else {
	    s = NULL;
	}
	return s;
    }
    
    switch (smiValuePtr->basetype) {
    case SMI_BASETYPE_UNSIGNED32:
	if (!(flags & SMI_RENDER_FORMAT) ||
	    !smiTypePtr || !smiTypePtr->format ||
	    !strlen(smiTypePtr->format) || smiTypePtr->format[0] == 'd') {
	    if (smiTypePtr->format && (strlen(smiTypePtr->format) >= 3) &&
		(smiTypePtr->format[1] == '-')) {
		i = atoi(&smiTypePtr->format[2]);
		if (i < 0) i = 0;
		if (i > 20) i = 20;
		smiAsprintf(&s, "%0*lu.",
			    1 + i,
			    smiValuePtr->value.unsigned32);
		if (s) {
		    for (j = strlen(s) - 1; i > 0; i--, j--) {
			s[j] = s[j-1];
		    }
		    s[j] = '.';
		}
	    } else {
		smiAsprintf(&s, "%lu", smiValuePtr->value.unsigned32);
	    }
	} else if (smiTypePtr->format[0] == 'x') {
	    smiAsprintf(&s, "%lx", smiValuePtr->value.unsigned32);
	} else if (smiTypePtr->format[0] == 'o') {
	    smiAsprintf(&s, "%lo", smiValuePtr->value.unsigned32);
	} else if (smiTypePtr->format[0] == 'b') {
	    for (i = 32 - 1;
		 i > 0 && !(smiValuePtr->value.unsigned32 & (1 << i)); i--);
	    s = smiMalloc(i + 1 + 1);
	    if (s) {
		for (j = 0; i >= 0; i--, j++) {
		    s[j] = smiValuePtr->value.unsigned32 & (1<<i) ? '1' : '0';
		}
		s[j] = 0;
	    }
	}
	break;
    case SMI_BASETYPE_UNSIGNED64:
	if (!(flags & SMI_RENDER_FORMAT) ||
	    !smiTypePtr || !smiTypePtr->format ||
	    !strlen(smiTypePtr->format) || smiTypePtr->format[0] == 'd') {
	    if (smiTypePtr->format && (strlen(smiTypePtr->format) >= 3) &&
		(smiTypePtr->format[1] == '-')) {
		i = atoi(&smiTypePtr->format[2]);
		if (i < 0) i = 0;
		if (i > 20) i = 20;
		sprintf(f, "%%0%s.", UINT64_FORMAT);
		f[2] = '*';
		smiAsprintf(&s, f,
			    1 + i,
			    smiValuePtr->value.unsigned64);
		if (s) {
		    for (j = strlen(s) - 1; i > 0; i--, j--) {
			s[j] = s[j-1];
		    }
		    s[j] = '.';
		}
	    } else {
		smiAsprintf(&s, UINT64_FORMAT, smiValuePtr->value.unsigned64);
	    }
	} else if (smiTypePtr->format[0] == 'x') {
	    strcpy(f, UINT64_FORMAT);
	    f[strlen(f)-1] = 'x';
	    smiAsprintf(&s, f, smiValuePtr->value.unsigned64);
	} else if (smiTypePtr->format[0] == 'o') {
	    strcpy(f, UINT64_FORMAT);
	    f[strlen(f)-1] = 'o';
	    smiAsprintf(&s, f, smiValuePtr->value.unsigned64);
	} else if (smiTypePtr->format[0] == 'b') {
	    for (i = 64 - 1;
		 i > 0 && !(smiValuePtr->value.unsigned64 & (1 << i)); i--);
	    s = smiMalloc(i + 1 + 1);
	    if (s) {
		for (j = 0; i >= 0; i--, j++) {
		    s[j] = smiValuePtr->value.unsigned64 & (1<<i) ? '1' : '0';
		}
		s[j] = 0;
	    }
	}
	break;
    case SMI_BASETYPE_INTEGER32:
	if (!(flags & SMI_RENDER_FORMAT) ||
	    !smiTypePtr || !smiTypePtr->format ||
	    !strlen(smiTypePtr->format) || smiTypePtr->format[0] == 'd') {
	    if (smiTypePtr->format && (strlen(smiTypePtr->format) >= 3) &&
		(smiTypePtr->format[1] == '-')) {
		i = atoi(&smiTypePtr->format[2]);
		if (i < 0) i = 0;
		if (i > 20) i = 20;
		smiAsprintf(&s, "%0*ld.",
			    1 + i + (smiValuePtr->value.integer32 < 0 ? 1 : 0),
			    smiValuePtr->value.integer32);
		if (s) {
		    for (j = strlen(s) - 1; i > 0; i--, j--) {
			s[j] = s[j-1];
		    }
		    s[j] = '.';
		}
	    } else {
		smiAsprintf(&s, "%ld", smiValuePtr->value.integer32);
	    }
	} else if (smiTypePtr->format[0] == 'x') {
	    if (smiValuePtr->value.integer32 >= 0) {
		smiAsprintf(&s, "%lx", smiValuePtr->value.integer32);
	    } else {
		smiAsprintf(&s, "-%lx", - smiValuePtr->value.integer32);
	    }
	} else if (smiTypePtr->format[0] == 'o') {
	    if (smiValuePtr->value.integer32 >= 0) {
		smiAsprintf(&s, "%lo", smiValuePtr->value.integer32);
	    } else {
		smiAsprintf(&s, "-%lo", - smiValuePtr->value.integer32);
	    }
	} else if (smiTypePtr->format[0] == 'b') {
	    if (smiValuePtr->value.integer32 >= 0) {
		v32 = smiValuePtr->value.integer32;
		j = 0;
	    } else {
		v32 = - smiValuePtr->value.integer32;
		j = 1;
	    }
	    for (i = 32 - 1;
		 i > 0 && !(v32 & (1 << i)); i--);
	    s = smiMalloc(i + j + 1 + 1);
	    if (s) {
		s[0] = '-';
		for (; i >= 0; i--, j++) {
		    s[j] = v32 & (1<<i) ? '1' : '0';
		}
		s[j] = 0;
	    }
	}
	break;
    case SMI_BASETYPE_INTEGER64:
	if (!(flags & SMI_RENDER_FORMAT) ||
	    !smiTypePtr || !smiTypePtr->format ||
	    !strlen(smiTypePtr->format) || smiTypePtr->format[0] == 'd') {
	    if (smiTypePtr->format && (strlen(smiTypePtr->format) >= 3) &&
		(smiTypePtr->format[1] == '-')) {
		i = atoi(&smiTypePtr->format[2]);
		if (i < 0) i = 0;
		if (i > 20) i = 20;
		sprintf(f, "%%0%s.", INT64_FORMAT);
		f[2] = '*';
		smiAsprintf(&s, f,
			    1 + i + (smiValuePtr->value.integer64 < 0 ? 1 : 0),
			    smiValuePtr->value.integer64);
		if (s) {
		    for (j = strlen(s) - 1; i > 0; i--, j--) {
			s[j] = s[j-1];
		    }
		    s[j] = '.';
		}
	    } else {
		smiAsprintf(&s, INT64_FORMAT, smiValuePtr->value.integer64);
	    }
	} else if (smiTypePtr->format[0] == 'x') {
	    if (smiValuePtr->value.integer64 >= 0) {
		strcpy(f, UINT64_FORMAT);
		f[strlen(f)-1] = 'x';
		smiAsprintf(&s, f, smiValuePtr->value.integer64);
	    } else {
		sprintf(f, "-%s", UINT64_FORMAT);
		f[strlen(f)-1] = 'x';
		smiAsprintf(&s, f, - smiValuePtr->value.integer64);
	    }
	} else if (smiTypePtr->format[0] == 'o') {
	    if (smiValuePtr->value.integer64 >= 0) {
		strcpy(f, UINT64_FORMAT);
		sprintf(f, "-%s", UINT64_FORMAT);
		f[strlen(f)-1] = 'o';
		smiAsprintf(&s, f, smiValuePtr->value.integer64);
	    } else {
		smiAsprintf(&s, f, - smiValuePtr->value.integer64);
	    }
	} else if (smiTypePtr->format[0] == 'b') {
	    if (smiValuePtr->value.integer64 >= 0) {
		v64 = smiValuePtr->value.integer64;
		j = 0;
	    } else {
		v64 = - smiValuePtr->value.integer64;
		j = 1;
	    }
	    for (i = 64 - 1;
		 i > 0 && !(v64 & (1 << i)); i--);
	    s = smiMalloc(i + j + 1 + 1);
	    if (s) {
		s[0] = '-';
		for (; i >= 0; i--, j++) {
		    s[j] = v64 & (1<<i) ? '1' : '0';
		}
		s[j] = 0;
	    }
	}
	break;
    case SMI_BASETYPE_OBJECTIDENTIFIER:
	s = smiRenderOID(smiValuePtr->len, smiValuePtr->value.oid, flags);
	break;
    case SMI_BASETYPE_OCTETSTRING:
	if (!(flags & SMI_RENDER_FORMAT) ||
	    (!smiTypePtr->format &&
	     (smiTypePtr->name && strcmp( smiTypePtr->name, "IpAddress")) ) ) {
	    for (i = 0; i < smiValuePtr->len; i++) {
                int val = smiValuePtr->value.ptr[i];
		if (!(isprint(val) || ((val >= 0x9) && (val <= 0xd)))) break; /* HTAB,VTAB,LF,FF,CR */
	    }
	    if ((i < smiValuePtr->len) ||
		!(flags & SMI_RENDER_PRINTABLE)) {
		smiAsprintf(&s, "");
		for (i=0; i < smiValuePtr->len; i++) {
		    ss = s;
		    smiAsprintf(&s, "%s%02X ", ss, (smiValuePtr->value.ptr[i]&0xFF));
		    smiFree(ss);
		}
	    } else {
		smiAsprintf(&s, "%s", smiValuePtr->value.ptr);
	    }
	} else {
	    i = 0;
	    smiAsprintf(&s, "");
	    /* SNMPv2-SMI:IpAddress does not have a display hint.
	       ==> let's use this one: "1d." if we have an IpAddress here */
	    fmt = (smiTypePtr->name &&
		   strcmp( smiTypePtr->name, "IpAddress" ) ) ?
		smiTypePtr->format : "1d.";
	    while (*fmt && i < smiValuePtr->len) {
		last_fmt = fmt;
		have_pfx = pfx = 0; /* scan prefix: */
		while (*fmt && isdigit((int)*fmt)) {
		    pfx = pfx * 10 + *fmt - '0', have_pfx = 1, fmt++;
		}
		if (! have_pfx) {
		    pfx = 1;
		}
		switch (*fmt) {
		case 't':
		    /* XXX UTF-8 not implemented, fall through to ASCII (a) */
		case 'a':
		    n = (pfx < (smiValuePtr->len - i)) ?
			pfx : smiValuePtr->len - i;
		    for (k = 0; k < n; k++) {
			if (! isascii((int) smiValuePtr->value.ptr[i+k])) {
			    smiFree(s);
			    if (flags & SMI_RENDER_UNKNOWN) {
				smiAsprintf(&s, SMI_UNKNOWN_LABEL);
			    } else {
				s = NULL;
			    }
			    return s;
			}
			ss = s;
			smiAsprintf(&s, "%s%c", ss, smiValuePtr->value.ptr[i+k]);
			smiFree(ss);
		    }
		    i += n;
		    break;
		case 'b':
		case 'd':
		case 'o':
		case 'x':
		    /* XXX: limited to no more than
		       sizeof(SmiUnsigned64) octets */
		    vv = 0;
		    xlen = pfx * 2;
		    while (pfx > 0 && i < smiValuePtr->len) {
			vv = vv * 256 +
			     ((unsigned char)smiValuePtr->value.ptr[i]);
			i++;
			pfx--;
		    }
		    switch (*fmt) {
		    case 'd':
			ss = s;
			sprintf(f, "%%s%s", UINT64_FORMAT);
			smiAsprintf(&s, f, ss, vv);
			smiFree(ss);
			break;
		    case 'o':
			ss = s;
			sprintf(f, "%%s%s", UINT64_FORMAT);
			f[strlen(f)-1] = 'o';
			smiAsprintf(&s, f, ss, vv);
			smiFree(ss);
			break;
		    case 'x':
			ss = s;
			sprintf(f, "%%s%%0%s", UINT64_FORMAT);
			f[4] = '*';
			f[strlen(f)-1] = 'x';
			smiAsprintf(&s, f, ss, xlen, vv);
			smiFree(ss);
			break;
		    case 'b':
			k = pfx * 8 - 1;
			if (k > sizeof(SmiUnsigned64) * 8 - 1)
			    k = sizeof(SmiUnsigned64) * 8 - 1;
			for (j = 0; k >= 0; k--, j++) {
			    ss = s;
			    smiAsprintf(&s, "%s%c",
					ss, vv & (1 << k) ? '1' : '0');
			    smiFree(ss);
			}
			break;
		    }
		    break;
		default:
		    smiFree(s);
		    if (flags & SMI_RENDER_UNKNOWN) {
			smiAsprintf(&s, SMI_UNKNOWN_LABEL);
		    } else {
			s = NULL;
		    }
		    return s;
		}
		fmt++;

		/*
		 * Check for a separator and repeat with last format if
		 * data is still available.
		 */
		if (*fmt && ! isdigit((int) *fmt) && *fmt != '*') {
		    if (i < smiValuePtr->len) {
			ss = s;
			smiAsprintf(&s, "%s%c", ss, fmt[0]);
			smiFree(ss);
		    }
		    fmt++;
		}

		if (! *fmt && (i < smiValuePtr->len)) {
		    fmt = last_fmt;
		}
	    }
	}
	break;
    case SMI_BASETYPE_ENUM:
	if ((flags & SMI_RENDER_NAME) && (smiTypePtr)) {
	    for (nn = smiGetFirstNamedNumber(smiTypePtr); nn;
		 nn = smiGetNextNamedNumber(nn)) {
		if (nn->value.value.integer32 == smiValuePtr->value.integer32)
		    break;
	    }
	    if (nn) {
		if (flags & SMI_RENDER_NUMERIC) {
		    smiAsprintf(&s, "%s(%ld)",
				nn->name, nn->value.value.integer32);
		} else {
		    smiAsprintf(&s, "%s", nn->name);
		}
	    } else {
		smiAsprintf(&s, "%ld", smiValuePtr->value.integer32);
	    }
	} else {
	    smiAsprintf(&s, "%ld", smiValuePtr->value.integer32);
	}
	break;
    case SMI_BASETYPE_BITS:
	smiAsprintf(&s, "");
	for (i = 0, nn = NULL; i < smiValuePtr->len * 8; i++) {
	    if (smiValuePtr->value.ptr[i/8] & (1 << (7-(i%8)))) {
		if ((flags & SMI_RENDER_NAME) && (smiTypePtr)) {
		    for (nn = smiGetFirstNamedNumber(smiTypePtr); nn;
			 nn = smiGetNextNamedNumber(nn)) {
			if (nn->value.value.unsigned32 == i)
			    break;
		    }
		}
		ss = s;
		if ((flags & SMI_RENDER_NAME) &&
		    (flags & SMI_RENDER_NUMERIC) && nn) {
		    smiAsprintf(&s, "%s%s%s(%d)",
				ss, strlen(ss) ? " " : "", nn->name, i);
		} else if (nn) {
		    smiAsprintf(&s, "%s%s%s",
				ss, strlen(ss) ? " " : "", nn->name);
		} else {
		    smiAsprintf(&s, "%s%s%d",
				ss, strlen(ss) ? " " : "", i);
		}
		smiFree(ss);
	    }
	}
	break;
    case SMI_BASETYPE_FLOAT32:
    case SMI_BASETYPE_FLOAT64:
    case SMI_BASETYPE_FLOAT128:
    case SMI_BASETYPE_UNKNOWN:
    default:
	if (flags & SMI_RENDER_UNKNOWN) {
	    smiAsprintf(&s, SMI_UNKNOWN_LABEL);
	} else {
	    s = NULL;
	}
	break;
    }

    return s;
}

char *smiRenderNode(SmiNode *smiNodePtr, int flags)
{
    char *s;
    SmiModule *modulePtr;
    
    if ((!smiNodePtr) || (smiNodePtr->name == NULL)) {
	if (flags & SMI_RENDER_UNKNOWN) {
	    smiAsprintf(&s, SMI_UNKNOWN_LABEL);
	} else {
	    s = NULL;
	}
    } else {
	modulePtr = smiGetNodeModule(smiNodePtr);
	if ((!(flags & SMI_RENDER_QUALIFIED)) ||
	    (!modulePtr) ||
	    (!strlen(modulePtr->name))) {
	    smiAsprintf(&s, "%s", smiNodePtr->name);
	} else {
	    smiAsprintf(&s, "%s::%s", modulePtr->name, smiNodePtr->name);
	}
    }
    return s;
}



char *smiRenderType(SmiType *smiTypePtr, int flags)
{
    char *s;
    SmiModule *modulePtr;
    
    if ((!smiTypePtr) || (smiTypePtr->name == NULL)) {
	if (flags & SMI_RENDER_UNKNOWN) {
	    smiAsprintf(&s, SMI_UNKNOWN_LABEL);
	} else {
	    s = NULL;
	}
    } else {
	modulePtr = smiGetTypeModule(smiTypePtr);
	if ((!(flags & SMI_RENDER_QUALIFIED)) ||
	    (!modulePtr) ||
	    (!strlen(modulePtr->name))) {
	    smiAsprintf(&s, "%s", smiTypePtr->name);
	} else {
	    smiAsprintf(&s, "%s::%s", modulePtr->name, smiTypePtr->name);
	}
    }
    return s;
}



unsigned int smiGetMinSize(SmiType *smiType)
{
    SmiRange *smiRange;
    SmiType  *parentType;
    unsigned int min = 65535, size;
    
    switch (smiType->basetype) {
    case SMI_BASETYPE_BITS:
	return 0;
    case SMI_BASETYPE_OCTETSTRING:
    case SMI_BASETYPE_OBJECTIDENTIFIER:
	size = 0;
	break;
    default:
	return 0;
    }

    for (smiRange = smiGetFirstRange(smiType);
	 smiRange ; smiRange = smiGetNextRange(smiRange)) {
	if (smiRange->minValue.value.unsigned32 < min) {
	    min = smiRange->minValue.value.unsigned32;
	}
    }
    if (min < 65535 && min > size) {
	size = min;
    }

    parentType = smiGetParentType(smiType);
    if (parentType) {
	unsigned int psize = smiGetMinSize(parentType);
	if (psize > size) {
	    size = psize;
	}
    }

    return size;
}



unsigned int smiGetMaxSize(SmiType *smiType)
{
    SmiRange *smiRange;
    SmiType  *parentType;
    SmiNamedNumber *nn;
    unsigned int max = 0, size;
    
    switch (smiType->basetype) {
    case SMI_BASETYPE_BITS:
    case SMI_BASETYPE_OCTETSTRING:
	size = 65535;
	break;
    case SMI_BASETYPE_OBJECTIDENTIFIER:
	size = 128;
	break;
    default:
	return 0xffffffff;
    }

    if (smiType->basetype == SMI_BASETYPE_BITS) {
	for (nn = smiGetFirstNamedNumber(smiType);
	     nn;
	     nn = smiGetNextNamedNumber(nn)) {
	    if (nn->value.value.unsigned32 > max) {
		max = nn->value.value.unsigned32;
	    }
	}
	size = (max / 8) + 1;
	return size;
    }

    for (smiRange = smiGetFirstRange(smiType);
	 smiRange ; smiRange = smiGetNextRange(smiRange)) {
	if (smiRange->maxValue.value.unsigned32 > max) {
	    max = smiRange->maxValue.value.unsigned32;
	}
    }
    if (max > 0 && max < size) {
	size = max;
    }

    parentType = smiGetParentType(smiType);
    if (parentType) {
	unsigned int psize = smiGetMaxSize(parentType);
	if (psize < size) {
	    size = psize;
	}
    }

    return size;
}



int smiUnpack(SmiNode *row, SmiSubid *oid, unsigned int oidlen,
	      SmiValue **vals, int *valslen)
{
    SmiNode *indexNode = NULL;
    SmiElement *smiElement;
    SmiNode *iNode;
    SmiType *iType; 
    int i, j, last = 0;
   
    if (!vals || !valslen || !row || !oid) {
	return 0;
    }

    switch (row->indexkind) {
    case SMI_INDEX_INDEX:
    case SMI_INDEX_REORDER:
	indexNode = row;
	break;
    case SMI_INDEX_EXPAND:	/* TODO: we have to do more work here! */
	indexNode = NULL;
	break;
    case SMI_INDEX_AUGMENT:
    case SMI_INDEX_SPARSE:
	indexNode = smiGetRelatedNode(row);
	break;
    case SMI_INDEX_UNKNOWN:
	indexNode = NULL;
	break;
    }

    *valslen = 0;
    for (smiElement = smiGetFirstElement(indexNode);
	 smiElement; smiElement = smiGetNextElement(smiElement)) {
	iNode = smiGetElementNode(smiElement);
	if (iNode) {
	    iType = smiGetNodeType(iNode);
	    if (! iType) break;
	    (*valslen)++;
	}
    }
    if (smiElement) {
	return 0;
    }

    *vals = smiMalloc(*valslen * sizeof(SmiValue));

    for (smiElement = smiGetFirstElement(indexNode), i = 0, j = 0;
	 smiElement; smiElement = smiGetNextElement(smiElement), i++) {
	iNode = smiGetElementNode(smiElement);
	last = (smiGetNextElement(smiElement) == NULL);
	iType = smiGetNodeType(iNode);
	fprintf(stderr, "** %s (%s)\n", iNode->name, iType->name);
	(*vals)[i].basetype = iType->basetype;
	switch (iType->basetype) {
	case SMI_BASETYPE_ENUM:
	case SMI_BASETYPE_INTEGER32:
	    (*vals)[i].value.integer32 = oid[j]; j++;
	    break;
	case SMI_BASETYPE_UNSIGNED32:
	    (*vals)[i].value.unsigned32 = oid[j]; j++;
	    break;
	case SMI_BASETYPE_OCTETSTRING:
	    /* need to know whether implied/fixed length or not */
	    break;
	case SMI_BASETYPE_OBJECTIDENTIFIER:
	    /* need to know whether implied/fixed length or not */
	    break;
	default:
	    return 0;
	}
    }

    return *valslen;
}



int smiAsprintf(char **strp, const char *format, ...)
{
    int rc;
    va_list ap;

    va_start(ap, format);
    rc = vasprintf(strp, format, ap);
    va_end(ap);
    if (! strp) {
	smiPrintError(NULL, ERR_OUT_OF_MEMORY);
    }
    return rc;
}



int smiVasprintf(char **strp, const char *format, va_list ap)
{
    int rc;

    rc = vasprintf(strp, format, ap);
    if (! strp) {
	smiPrintError(NULL, ERR_OUT_OF_MEMORY);
    }
    return rc;
}


int smiGetMinMaxRange(SmiType *smiType, SmiValue *min, SmiValue *max)
{
    SmiBasetype    basetype = SMI_BASETYPE_UNKNOWN;
    SmiRange       *range;

    min->basetype = max->basetype = SMI_BASETYPE_UNKNOWN;
    min->len = max->len = 0;

    range = smiGetFirstRange(smiType);
    if (!range) {
	return 0;
    }

    basetype = range->minValue.basetype;
    min->basetype = max->basetype = basetype;

    switch (basetype) {
    case SMI_BASETYPE_INTEGER32:
	min->value.integer32 = SMI_BASETYPE_INTEGER32_MAX;
	max->value.integer32 = SMI_BASETYPE_INTEGER32_MIN;
	break;
    case SMI_BASETYPE_INTEGER64:
	min->value.integer64 = SMI_BASETYPE_INTEGER64_MAX;
	max->value.integer64 = SMI_BASETYPE_INTEGER64_MIN;
	break;
    case SMI_BASETYPE_UNSIGNED32:
	min->value.unsigned32 = SMI_BASETYPE_UNSIGNED32_MAX;
	max->value.unsigned32 = SMI_BASETYPE_UNSIGNED32_MIN;
	break;
    case SMI_BASETYPE_UNSIGNED64:
	min->value.unsigned64 = SMI_BASETYPE_UNSIGNED64_MAX;
	max->value.unsigned64 = SMI_BASETYPE_UNSIGNED32_MIN;
	break;
    default:
	fprintf(stderr, "smidump: unexpected basetype %d\n", basetype);
	return -1;
    }

    for (range = smiGetFirstRange(smiType);
	 range;
	 range = smiGetNextRange(range)) {
	switch (basetype) {
	case SMI_BASETYPE_INTEGER32:
	    if (range->minValue.value.integer32 < min->value.integer32) {
		min->value.integer32 = range->minValue.value.integer32;
	    }
	    if (range->maxValue.value.integer32 > max->value.integer32) {
		max->value.integer32 = range->maxValue.value.integer32;
	    }
	    break;
	case SMI_BASETYPE_INTEGER64:
	    if (range->minValue.value.integer64 < min->value.integer64) {
		min->value.integer64 = range->minValue.value.integer64;
	    }
	    if (range->maxValue.value.integer64 > max->value.integer64) {
		max->value.integer64 = range->maxValue.value.integer64;
	    }
	    break;
	case SMI_BASETYPE_UNSIGNED32:
	    if (range->minValue.value.unsigned32 < min->value.unsigned32) {
		min->value.unsigned32 = range->minValue.value.unsigned32;
	    }
	    if (range->maxValue.value.unsigned32 > max->value.unsigned32) {
		max->value.unsigned32 = range->maxValue.value.unsigned32;
	    }
	    break;
	case SMI_BASETYPE_UNSIGNED64:
	    if (range->minValue.value.unsigned64 < min->value.unsigned64) {
		min->value.unsigned64 = range->minValue.value.unsigned64;
	    }
	    if (range->maxValue.value.unsigned64 > max->value.unsigned64) {
		max->value.unsigned64 = range->maxValue.value.unsigned64;
	    }
	    break;
	default:
	    fprintf(stderr, "smidump: unexpected basetype %d\n", basetype);
	    return -1;
	}
    }

    return 0;
}
