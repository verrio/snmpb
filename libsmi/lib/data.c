/*
 * data.c --
 *
 *      Operations on the main data structures.
 *
 * Copyright (c) 1999-2002 Frank Strauss, Technical University of Braunschweig.
 *
 * See the file "COPYING" for information on usage and redistribution
 * of this file, and for a DISCLAIMER OF ALL WARRANTIES.
 *
 * @(#) $Id: data.c 7822 2008-03-01 13:22:42Z schoenw $
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

#include "error.h"
#include "util.h"
#include "data.h"
#include "smi.h"

#ifdef HAVE_DMALLOC_H
#include <dmalloc.h>
#endif



#ifdef BACKEND_SMI
#include "scanner-smi.h"
extern int smiparse();
#endif

#ifdef BACKEND_SMING
#include "scanner-sming.h"
extern int smingparse();
#endif

#define stringKind(kind) ( \
	(kind == KIND_ANY)                 ? "ANY" : \
	(kind == KIND_MODULE)              ? "MODULE" : \
	(kind == KIND_MACRO)               ? "MACRO" : \
	(kind == KIND_TYPE)                ? "TYPE" : \
	(kind == KIND_OBJECT)              ? "OBJECT" : \
	(kind == KIND_IMPORT)              ? "IMPORT" : \
					     "unknown" )


int		smiDepth = 0;
static Handle   *firstHandlePtr = NULL;
static Handle   *lastHandlePtr  = NULL;


/*
 *----------------------------------------------------------------------
 *
 * addHandle --
 *
 *      Adds a libsmi handle with a given name.
 *
 * Results:
 *      0 on success or -1 on an error.
 *
 * Side effects:
 *      None.
 *
 *----------------------------------------------------------------------
 */

Handle *addHandle(const char *name)
{
    Handle *handlePtr;

    handlePtr = (Handle *) smiMalloc(sizeof(Handle));

    handlePtr->name    = smiStrdup(name);

    handlePtr->nextPtr = NULL;
    handlePtr->prevPtr = lastHandlePtr;
    if (!firstHandlePtr) firstHandlePtr = handlePtr;
    if (lastHandlePtr) lastHandlePtr->nextPtr = handlePtr;
    lastHandlePtr = handlePtr;

    return (handlePtr);
}



/*
 *----------------------------------------------------------------------
 *
 * removeHandle --
 *
 *      Removes a given libsmi handle.
 *
 * Results:
 *      None.
 *
 * Side effects:
 *      None.
 *
 *----------------------------------------------------------------------
 */

void removeHandle(Handle *handlePtr)
{
    if (handlePtr->prevPtr) {
	handlePtr->prevPtr->nextPtr = handlePtr->nextPtr;
    } else {
	firstHandlePtr = handlePtr->nextPtr;
    }
    if (handlePtr->nextPtr) {
	handlePtr->nextPtr->prevPtr = handlePtr->prevPtr;
    } else {
	lastHandlePtr = handlePtr->prevPtr;
    }

    smiFree(handlePtr->name);
    smiFree(handlePtr);
}



/*
 *----------------------------------------------------------------------
 *
 * findHandleByName --
 *
 *      Lookup an libsmi handle by its name.
 *
 * Results:
 *      A pointer to the Handle structure or
 *	NULL if it is not found.
 *
 * Side effects:
 *      None.
 *
 *----------------------------------------------------------------------
 */

Handle *findHandleByName(const char *name)
{
    Handle *handlePtr;

    if (!name)
	return NULL;
    
    for (handlePtr = firstHandlePtr; handlePtr;
	 handlePtr = handlePtr->nextPtr) {
	if (!strcmp(handlePtr->name, name)) {
		return (handlePtr);
	}
    }

    return NULL;
    
}



/*
 *----------------------------------------------------------------------
 *
 * addView --
 *
 *      Add a module to the `view' (the list of modules, seen by the user).
 *
 * Results:
 *      A pointer to the new View structure or
 *	NULL if terminated due to an error.
 *
 * Side effects:
 *      None.
 *
 *----------------------------------------------------------------------
 */

View *addView(const char *modulename)
{
    View	      *viewPtr;

    viewPtr = (View *) smiMalloc(sizeof(View));

    viewPtr->name				= smiStrdup(modulename);
    viewPtr->nextPtr				= NULL;
    viewPtr->prevPtr				= smiHandle->lastViewPtr;
    if (!smiHandle->firstViewPtr) smiHandle->firstViewPtr		= viewPtr;
    if (smiHandle->lastViewPtr) smiHandle->lastViewPtr->nextPtr	= viewPtr;
    smiHandle->lastViewPtr	     				= viewPtr;
    
    return (viewPtr);
}



/*
 *----------------------------------------------------------------------
 *
 * isInView --
 *
 *      Check, whether a given module is in the current view.
 *
 * Results:
 *      != 0 if in view, 0 otherwise.
 *
 * Side effects:
 *      None.
 *
 *----------------------------------------------------------------------
 */

int isInView(const char *modulename)
{
    View	      *viewPtr;

#if 0
    if (smiHandle->flags & SMI_FLAG_VIEWALL) {
	return 1;
    }
#endif
    
    for (viewPtr = smiHandle->firstViewPtr; viewPtr; viewPtr = viewPtr->nextPtr) {
	if (!strcmp(modulename, viewPtr->name)) {
	    return 1;
	}
    }
    return 0;
}


/*
 *----------------------------------------------------------------------
 *
 * addModule --
 *
 *      Create a new MIB module.
 *
 * Results:
 *      A pointer to the new Module structure or
 *	NULL if terminated due to an error.
 *
 * Side effects:
 *      None.
 *
 *----------------------------------------------------------------------
 */

Module *addModule(char *modulename, char *path, ModuleFlags flags,
		  Parser *parserPtr)
{
    Module	      *modulePtr;

    modulePtr = (Module *) smiMalloc(sizeof(Module));

    modulePtr->export.name			= modulename;
    modulePtr->export.path			= path;
    modulePtr->export.language			= SMI_LANGUAGE_UNKNOWN;
    modulePtr->export.organization		= NULL;
    modulePtr->export.contactinfo		= NULL;
    modulePtr->export.description		= NULL;
    modulePtr->export.reference			= NULL;
    modulePtr->export.conformance               = 0;
    
    modulePtr->lastUpdated			= 0;
    modulePtr->flags				= flags;
    modulePtr->objectPtr			= NULL;
    
    modulePtr->prefixNodePtr                    = NULL;
    modulePtr->firstObjectPtr			= NULL;
    modulePtr->lastObjectPtr			= NULL;
    modulePtr->firstTypePtr			= NULL;
    modulePtr->lastTypePtr			= NULL;
    modulePtr->firstMacroPtr			= NULL;
    modulePtr->lastMacroPtr			= NULL;
    modulePtr->firstImportPtr			= NULL;
    modulePtr->lastImportPtr			= NULL;
    modulePtr->firstRevisionPtr			= NULL;
    modulePtr->lastRevisionPtr			= NULL;
    
    modulePtr->numImportedIdentifiers		= 0;
    modulePtr->numStatements			= 0;
    modulePtr->numModuleIdentities		= 0;

    modulePtr->nextPtr				= NULL;
    modulePtr->prevPtr				= smiHandle->lastModulePtr;
    if (!smiHandle->firstModulePtr) smiHandle->firstModulePtr		= modulePtr;
    if (smiHandle->lastModulePtr) smiHandle->lastModulePtr->nextPtr	= modulePtr;
    smiHandle->lastModulePtr				= modulePtr;
    
    return (modulePtr);
}



/*
 *----------------------------------------------------------------------
 *
 * setModuleIdentityObject --
 *
 *      Set the objectPtr of a given Module to the OBJECT-IDENTITY object.
 *
 * Results:
 *	None.
 *
 * Side effects:
 *      None.
 *
 *----------------------------------------------------------------------
 */

void setModuleIdentityObject(Module *modulePtr, Object *objectPtr)
{
    modulePtr->objectPtr = objectPtr;
}



/*
 *----------------------------------------------------------------------
 *
 * setModuleLastUpdated --
 *
 *      Set the lastUpdated time_t value of a given Module.
 *
 * Results:
 *	None.
 *
 * Side effects:
 *      None.
 *
 *----------------------------------------------------------------------
 */

void setModuleLastUpdated(Module *modulePtr, time_t lastUpdated)
{
    modulePtr->lastUpdated = lastUpdated;
}



/*
 *----------------------------------------------------------------------
 *
 * setModuleOrganization --
 *
 *      Set the organization string of a given Module.
 *
 * Results:
 *	None.
 *
 * Side effects:
 *      None.
 *
 *----------------------------------------------------------------------
 */

void setModuleOrganization(Module *modulePtr, char *organization)
{
    modulePtr->export.organization = organization;
    while (strlen(organization) && organization[strlen(organization)-1] == '\n') {
	    organization[strlen(organization) - 1] = 0;
    }
}



/*
 *----------------------------------------------------------------------
 *
 * setModuleContactInfo --
 *
 *      Set the contactInfo string of a given Module.
 *
 * Results:
 *	None.
 *
 * Side effects:
 *      None.
 *
 *----------------------------------------------------------------------
 */

void setModuleContactInfo(Module *modulePtr, char *contactinfo)
{
    modulePtr->export.contactinfo = contactinfo;
}



/*
 *----------------------------------------------------------------------
 *
 * setModuleDescription --
 *
 *      Set the description string of a given Module.
 *
 * Results:
 *	None.
 *
 * Side effects:
 *      None.
 *
 *----------------------------------------------------------------------
 */

void setModuleDescription(Module *modulePtr, char *description,
			  Parser *parserPtr)
{
    if (modulePtr->export.description)
	smiFree(modulePtr->export.description);
    if (parserPtr->flags & SMI_FLAG_NODESCR) {
	smiFree(description);
	modulePtr->export.description = NULL;
    } else {
	modulePtr->export.description = description;
    }
}



/*
 *----------------------------------------------------------------------
 *
 * setModuleReference --
 *
 *      Set the reference string of a given Module.
 *
 * Results:
 *	None.
 *
 * Side effects:
 *      None.
 *
 *----------------------------------------------------------------------
 */

void setModuleReference(Module *modulePtr, char *reference, Parser *parserPtr)
{
    if (modulePtr->export.reference)
	smiFree(modulePtr->export.reference);
    if (parserPtr->flags & SMI_FLAG_NODESCR) {
	smiFree(reference);
	modulePtr->export.reference = NULL;
    } else {
	modulePtr->export.reference = reference;
    }
}



/*
 *----------------------------------------------------------------------
 *
 * findModuleByName --
 *
 *      Lookup a Module by a given name.
 *
 * Results:
 *      A pointer to the Module structure or
 *	NULL if it is not found.
 *
 * Side effects:
 *      None.
 *
 *----------------------------------------------------------------------
 */

Module *findModuleByName(const char *modulename)
{
    Module	*modulePtr;

    for (modulePtr = smiHandle->firstModulePtr; modulePtr;
	 modulePtr = modulePtr->nextPtr) {
	if ((modulePtr->export.name) &&
	    !strcmp(modulePtr->export.name, modulename)) {
	    return (modulePtr);
	}
    }

    return (NULL);
}



/*
 *----------------------------------------------------------------------
 *
 * addRevision --
 *
 *      Adds a revision entry for the given module.
 *
 * Results:
 *      0 on success or -1 on an error.
 *
 * Side effects:
 *      None.
 *
 *----------------------------------------------------------------------
 */

Revision *addRevision(time_t date, char *description, Parser *parserPtr)
{
    Revision	  *revisionPtr, *r;
    Module	  *modulePtr;

    revisionPtr = (Revision *) smiMalloc(sizeof(Revision));

    modulePtr = parserPtr->modulePtr;

    revisionPtr->modulePtr		 = modulePtr;
    revisionPtr->export.date	       	 = date;
    if (parserPtr->flags & SMI_FLAG_NODESCR) {
	smiFree(description);
	revisionPtr->export.description	 = NULL;
    } else {
	revisionPtr->export.description	 = description;
    }
    revisionPtr->line			 = parserPtr ? parserPtr->line : -1;

    for (r = modulePtr->lastRevisionPtr; r; r = r->prevPtr) {
	if (r->export.date > date) break;
    }
    if (r) {
	revisionPtr->nextPtr = r->nextPtr;
	revisionPtr->prevPtr = r;
	if (r->nextPtr) {
	    r->nextPtr->prevPtr = revisionPtr;
	} else {
	    modulePtr->lastRevisionPtr = revisionPtr;
	}
	r->nextPtr = revisionPtr;
    } else {
	revisionPtr->prevPtr = NULL;
	if (modulePtr->firstRevisionPtr) {
	    modulePtr->firstRevisionPtr->prevPtr = revisionPtr;
	    revisionPtr->nextPtr = modulePtr->firstRevisionPtr;
	} else {
	    modulePtr->lastRevisionPtr = revisionPtr;
	    revisionPtr->nextPtr = NULL;
	}
	modulePtr->firstRevisionPtr = revisionPtr;
    }

    return (revisionPtr);
}



/*
 *----------------------------------------------------------------------
 *
 * setRevisionLine --
 *
 *      Set the line of definition of a given Revision.
 *
 * Results:
 *	None.
 *
 * Side effects:
 *      None.
 *
 *----------------------------------------------------------------------
 */

void setRevisionLine(Revision *revisionPtr, int line, Parser *parserPtr)
{
    if (line) {
	revisionPtr->line = line;
    } else {
	revisionPtr->line = parserPtr ? parserPtr->line : -1;
    }
}



/*
 *----------------------------------------------------------------------
 *
 * addImport --
 *
 *      Adds a descriptor to the actual module's list of imported
 *      descriptors. This list may be checked by checkImports()
 *	afterwards.
 *
 * Results:
 *      0 on success or -1 on an error.
 *
 * Side effects:
 *      None.
 *
 *----------------------------------------------------------------------
 */

Import *addImport(char *name, Parser *parserPtr)
{
    Import        *importPtr;
    Module	  *modulePtr;

    importPtr = (Import *) smiMalloc(sizeof(Import));

    modulePtr = parserPtr->modulePtr;

    importPtr->modulePtr		 = modulePtr;
    importPtr->export.name       	 = name;
    importPtr->export.module		 = NULL; /* not yet known */
    importPtr->kind			 = KIND_UNKNOWN; /* not yet known */
    importPtr->use			 = 0;
    importPtr->flags			 = 0;
    importPtr->line			 = parserPtr ? parserPtr->line : -1;
    
    importPtr->nextPtr			 = NULL;
    importPtr->prevPtr			 = modulePtr->lastImportPtr;
    if (!modulePtr->firstImportPtr)
	modulePtr->firstImportPtr	 = importPtr;
    if (modulePtr->lastImportPtr)
	modulePtr->lastImportPtr->nextPtr = importPtr;
    modulePtr->lastImportPtr		 = importPtr;
    
    return (importPtr);
}



/*
 *----------------------------------------------------------------------
 *
 * addImportFlags --
 *
 *      Add flags to the flags of a given Import struct.
 *
 * Results:
 *	None.
 *
 * Side effects:
 *      None.
 *
 *----------------------------------------------------------------------
 */

void addImportFlags(Import *importPtr, ImportFlags flags)
{
    importPtr->flags |= flags;
}



/*
 *----------------------------------------------------------------------
 *
 * setImportModulename --
 *
 *      Set the modulename part of a given Import struct.
 *
 * Results:
 *	None.
 *
 * Side effects:
 *      None.
 *
 *----------------------------------------------------------------------
 */

void setImportModulename(Import *importPtr, char *modulename)
{
    if (importPtr->export.module) {
	smiFree(importPtr->export.module);
    }
    importPtr->export.module = modulename;
}



/*
 *----------------------------------------------------------------------
 *
 * checkImports --
 *
 *      Check wheather all descriptors in the actual module's list
 *	are imported by a given Module. Implicitly set all Imports'
 *	module names.
 *
 * Results:
 *      0 on success or -1 on an error or number of descriptors not found.
 *
 * Side effects:
 *      None.
 *
 *----------------------------------------------------------------------
 */

int checkImports(Module *modulePtr, Parser *parserPtr)
{
    int         n = 0;
    Import      *importPtr;
    SmiNode	*smiNode;
    SmiType	*smiType;
    SmiMacro	*smiMacro;
    
    for (importPtr = parserPtr->modulePtr->firstImportPtr;
	 importPtr; importPtr = importPtr->nextPtr) {

	if (importPtr->kind == KIND_UNKNOWN) {
	    if (modulePtr) {
		if ((smiNode = smiGetNode(&modulePtr->export,
					  importPtr->export.name))) {
		    importPtr->export.module =
			smiStrdup(modulePtr->export.name);
		    importPtr->kind	= KIND_OBJECT;
		} else if ((smiType = smiGetType(&modulePtr->export,
						 importPtr->export.name))) {
		    importPtr->export.module =
			smiStrdup(modulePtr->export.name);
		    importPtr->kind	= KIND_TYPE;
		} else if ((smiMacro = smiGetMacro(&modulePtr->export,
						   importPtr->export.name))) {
		    importPtr->export.module =
			smiStrdup(modulePtr->export.name);
		    importPtr->kind = KIND_MACRO;
		} else {
		    n++;
		    importPtr->export.module =
			smiStrdup(modulePtr->export.name);
		    smiPrintError(parserPtr, ERR_IDENTIFIER_NOT_IN_MODULE,
				  importPtr->export.name,
				  modulePtr->export.name);
		    importPtr->kind   = KIND_NOTFOUND;
		}
	    } else {
		    n++;
		    importPtr->export.module = smiStrdup("");
		    importPtr->kind   = KIND_NOTFOUND;
	    }
	}
    }

    return (n);
}



/*
 *----------------------------------------------------------------------
 *
 * findImportByName --
 *
 *      Lookup an import descriptor by its name and the module to look in.
 *
 * Results:
 *      A pointer to the Import structure or
 *	NULL if it is not found.
 *
 * Side effects:
 *      None.
 *
 *----------------------------------------------------------------------
 */

Import *findImportByName(const char *name, Module *modulePtr)
{
    Import           *importPtr;

    if (!name)
	return NULL;
    
    for (importPtr = modulePtr->firstImportPtr; importPtr;
	 importPtr = importPtr->nextPtr) {
	if ((!strcmp(importPtr->export.name, name)) &&
	    (!(importPtr->flags & FLAG_INCOMPLIANCE))) {
		return (importPtr);
	}
    }

    return NULL;
    
}



/*
 *----------------------------------------------------------------------
 *
 * findImportByModulenameAndName --
 *
 *      Lookup an import descriptor by its name and the modulename
 *	it is imported from and the module to look in.
 *
 * Results:
 *      A pointer to the Import structure or
 *	NULL if it is not found.
 *
 * Side effects:
 *      None.
 *
 *----------------------------------------------------------------------
 */

Import *findImportByModulenameAndName(const char *modulename,
				      const char *name, Module *modulePtr)
{
    Import           *importPtr;

    for (importPtr = modulePtr->firstImportPtr; importPtr;
	 importPtr = importPtr->nextPtr) {
	if ((!strcmp(importPtr->export.name, name)) &&
	    (!strcmp(importPtr->export.module, modulename))) {
	    return (importPtr);
	}
    }

    return (NULL);
}



/*
 *----------------------------------------------------------------------
 *
 * addObject --
 *
 *      Create a new Object and Node or update an existing one.
 *	Also updates other Objects and Nodes according
 *	to the PendingNode information.
 *
 * Results:
 *      A pointer to the new Object structure or
 *	NULL if terminated due to an error.
 *
 * Side effects:
 *      None.
 *
 *----------------------------------------------------------------------
 */

Object *addObject(char *objectname, Node *parentNodePtr, SmiSubid subid,
		  ObjectFlags flags, Parser *parserPtr)
{
    Object	     *objectPtr;
    Node	     *nodePtr;
    Module	     *modulePtr;


    objectPtr = (Object *) smiMalloc(sizeof(Object));

    modulePtr = parserPtr ? parserPtr->modulePtr : NULL;

    objectPtr->export.name		= objectname;
    objectPtr->export.decl		= SMI_DECL_UNKNOWN;
    objectPtr->export.access		= SMI_ACCESS_UNKNOWN;
    objectPtr->export.status		= SMI_STATUS_UNKNOWN;
    objectPtr->export.format		= NULL;
    objectPtr->export.value.basetype	= SMI_BASETYPE_UNKNOWN;
    objectPtr->export.units		= NULL;
    objectPtr->export.description	= NULL;
    objectPtr->export.reference		= NULL;
    objectPtr->export.indexkind		= SMI_INDEX_UNKNOWN;
    objectPtr->export.implied		= 0;
    objectPtr->export.create		= 0;
    objectPtr->export.nodekind		= SMI_NODEKIND_UNKNOWN;
					
    objectPtr->modulePtr		= modulePtr;
    objectPtr->nodePtr			= NULL;
    objectPtr->prevSameNodePtr		= NULL;
    objectPtr->nextSameNodePtr		= NULL;
    objectPtr->typePtr			= NULL;
    objectPtr->listPtr			= NULL;
    objectPtr->flags			= flags;
    objectPtr->line			= parserPtr ? parserPtr->line : -1;
    
    objectPtr->uniquenessPtr            = NULL;
    					
    objectPtr->export.oidlen            = 0;     /* filled in by  */
    objectPtr->export.oid               = NULL;  /* second pass.  */
    
    objectPtr->nextPtr				= NULL;
    if (modulePtr) {
        objectPtr->prevPtr			= modulePtr->lastObjectPtr;
	if (!modulePtr->firstObjectPtr)
	    modulePtr->firstObjectPtr		= objectPtr;
	if (modulePtr->lastObjectPtr)
	    modulePtr->lastObjectPtr->nextPtr	= objectPtr;
	modulePtr->lastObjectPtr		= objectPtr;
    } else {
	objectPtr->prevPtr			= NULL;
    }

    /*
     * Link it into the tree.
     */
    nodePtr = findNodeByParentAndSubid(parentNodePtr, subid);
    if ((parentNodePtr == parserPtr->pendingNodePtr) || (!nodePtr)) {

	/* a new Node has to be created for this Object */
	nodePtr = addNode(parentNodePtr, subid, flags, parserPtr);
	nodePtr->firstObjectPtr			      = objectPtr;
	nodePtr->lastObjectPtr			      = objectPtr;
    } else {

        objectPtr->prevSameNodePtr		      = nodePtr->lastObjectPtr;
	if (!nodePtr->firstObjectPtr)
	    nodePtr->firstObjectPtr	              = objectPtr;
	if (nodePtr->lastObjectPtr)
	    nodePtr->lastObjectPtr->nextSameNodePtr   = objectPtr;
	nodePtr->lastObjectPtr			      = objectPtr;
    }
    objectPtr->nodePtr				      = nodePtr;
    
    return (objectPtr);
}



/*
 *----------------------------------------------------------------------
 *
 * duplicateObject --
 *
 *      Create a new Object as a duplicate of a given one but with
 *      an affiliation to another module with new flags and with
 *	uninitialzied values.
 *
 * Results:
 *      A pointer to the new Object structure or
 *	NULL if terminated due to an error.
 *
 * Side effects:
 *      None.
 *
 *----------------------------------------------------------------------
 */

Object *duplicateObject(Object *templatePtr, ObjectFlags flags,
			Parser *parserPtr)
{
    Object		  *objectPtr;
    Node		  *nodePtr;
    Module		  *modulePtr;
    
    objectPtr = (Object *) smiMalloc(sizeof(Object));

    modulePtr = parserPtr->modulePtr;
    nodePtr   = templatePtr->nodePtr;
    
    objectPtr->export.name			= NULL;
    objectPtr->export.decl			= SMI_DECL_UNKNOWN;
    objectPtr->export.access			= SMI_ACCESS_UNKNOWN;
    objectPtr->export.status			= SMI_STATUS_UNKNOWN;
    objectPtr->export.format			= NULL;
    objectPtr->export.value.basetype		= SMI_BASETYPE_UNKNOWN;
    objectPtr->export.units			= NULL;
    objectPtr->export.description		= NULL;
    objectPtr->export.reference			= NULL;
    objectPtr->export.indexkind			= SMI_INDEX_UNKNOWN;
    objectPtr->export.implied			= 0;
    objectPtr->export.create			= 0;
    objectPtr->export.nodekind			= SMI_NODEKIND_UNKNOWN;
						
    objectPtr->modulePtr		        = modulePtr;
    objectPtr->nodePtr				= nodePtr;
    objectPtr->prevSameNodePtr			= NULL;
    objectPtr->nextSameNodePtr			= NULL;
    objectPtr->typePtr				= NULL;
    objectPtr->listPtr				= NULL;
    objectPtr->flags				= flags;
    objectPtr->line				= parserPtr ? parserPtr->line : -1;

    objectPtr->export.oidlen                    = 0;     /* filled in by  */
    objectPtr->export.oid                       = NULL;  /* second pass.  */
    
    objectPtr->nextPtr				= NULL;
    if (modulePtr) {
        objectPtr->prevPtr			= modulePtr->lastObjectPtr;
	if (!modulePtr->firstObjectPtr)
	    modulePtr->firstObjectPtr		= objectPtr;
	if (modulePtr->lastObjectPtr)
	    modulePtr->lastObjectPtr->nextPtr	= objectPtr;
	modulePtr->lastObjectPtr		= objectPtr;
    } else {
	objectPtr->prevPtr			= NULL;
    }
    
    objectPtr->prevSameNodePtr			      = nodePtr->lastObjectPtr;
    if (!nodePtr->firstObjectPtr)
	nodePtr->firstObjectPtr			      = objectPtr;
    if (nodePtr->lastObjectPtr)
	nodePtr->lastObjectPtr->nextSameNodePtr       = objectPtr;
    nodePtr->lastObjectPtr			      = objectPtr;
    objectPtr->nodePtr				      = nodePtr;
    
    return (objectPtr);
}



/*
 *----------------------------------------------------------------------
 *
 * addNode --
 *
 *      Create a new Node by a given parent Node and subid.
 *
 * Results:
 *      A pointer to the new Node structure or
 *	NULL if terminated due to an error.
 *
 * Side effects:
 *      None.
 *
 *----------------------------------------------------------------------
 */

Node *addNode (Node *parentNodePtr, SmiSubid subid, NodeFlags flags,
	       Parser *parserPtr)
{
    Node	    *nodePtr;
    Node	    *c;

    nodePtr = (Node *) smiMalloc(sizeof(Node));
    
    nodePtr->flags				= flags;
    nodePtr->subid				= subid;
    nodePtr->parentPtr				= parentNodePtr;
    nodePtr->firstChildPtr			= NULL;
    nodePtr->lastChildPtr			= NULL;
    nodePtr->firstObjectPtr			= NULL;
    nodePtr->lastObjectPtr			= NULL;

    /*
     * this cannot be set in all situations (pending sub trees).
     * we delay it to the second pass.
     */
    nodePtr->oidlen                             = 0;
    nodePtr->oid				= NULL;

    if (parentNodePtr) {
	if (parentNodePtr->firstChildPtr) {
	    for (c = parentNodePtr->firstChildPtr;
		 c && (c->subid < subid);
		 c = c->nextPtr);
	    if (c) {
		if (c != parentNodePtr->firstChildPtr) {
		    c->prevPtr->nextPtr = nodePtr;
		    nodePtr->prevPtr = c->prevPtr;
		    c->prevPtr = nodePtr;
		    nodePtr->nextPtr = c;
		} else {
		    c->prevPtr = nodePtr;
		    nodePtr->nextPtr = c;
		    nodePtr->prevPtr = NULL;
		    parentNodePtr->firstChildPtr = nodePtr;
		}
	    } else {
		nodePtr->nextPtr = NULL;
		nodePtr->prevPtr = parentNodePtr->lastChildPtr;
		parentNodePtr->lastChildPtr->nextPtr = nodePtr;
		parentNodePtr->lastChildPtr = nodePtr;
	    }
	} else {
	    parentNodePtr->firstChildPtr = nodePtr;
	    parentNodePtr->lastChildPtr = nodePtr;
	    nodePtr->nextPtr = NULL;
	    nodePtr->prevPtr = NULL;
	}
    }

    return nodePtr;
}



/*
 *----------------------------------------------------------------------
 *
 * createNodes --
 *
 *      Create all missing Nodes down the tree along all subids of
 *	a given Oid.
 *
 * Results:
 *      A pointer to the leaf Node structure or
 *	NULL if terminated due to an error.
 *
 * Side effects:
 *      None.
 *
 *----------------------------------------------------------------------
 */

Node *createNodes(unsigned int oidlen, SmiSubid *oid)
{
    Node	 *parentNodePtr, *nodePtr;
    unsigned int i;

    parentNodePtr = smiHandle->rootNodePtr;

    for(i = 0; i < oidlen; i++) {
	if (!(nodePtr = findNodeByParentAndSubid(parentNodePtr, oid[i]))) {
	    nodePtr = addNode(parentNodePtr, oid[i], 0, NULL);
	}
	parentNodePtr = nodePtr;
    }

    return parentNodePtr;    
}



/*
 *----------------------------------------------------------------------
 *
 * createNodesByOidString --
 *
 *      Create all missing Nodes down the tree along all subids of
 *	a given Oid.
 *
 * Results:
 *      A pointer to the leaf Node structure or
 *	NULL if terminated due to an error.
 *
 * Side effects:
 *      None.
 *
 *----------------------------------------------------------------------
 */

Node *createNodesByOidString(const char *oid)
{
    char		*p, *elements;
    Node		*parentNodePtr, *nodePtr;
    SmiSubid		subid;

    parentNodePtr = smiHandle->rootNodePtr;
    elements = smiStrdup(oid);

    p = strtok(elements, ".");
    do {
	subid = (unsigned int)strtoul(p, NULL, 0);
	if (!(nodePtr = findNodeByParentAndSubid(parentNodePtr,
						 subid))) {
	    nodePtr = addNode(parentNodePtr, subid, 0, NULL);
	}
	parentNodePtr = nodePtr;
    } while ((p = strtok(NULL, ".")));

    smiFree(elements);

    return parentNodePtr;
}



/*
 *----------------------------------------------------------------------
 *
 * getParentNode --
 *
 *      Return the parent of a given Node.
 *
 * Results:
 *      A pointer to the parent Node structure.
 *
 * Side effects:
 *      None.
 *
 *----------------------------------------------------------------------
 */

Node *getParentNode(Node *nodePtr)
{
    return nodePtr->parentPtr;
}



/*
 *----------------------------------------------------------------------
 *
 * mergeNodeTrees --
 *
 *      Merge the subtree rooted at `from' into the `to' tree recursively
 *      and release the `from' tree.
 *
 * Results:
 *	None.
 *
 * Side effects:
 *      None.
 *
 *----------------------------------------------------------------------
 */

static void mergeNodeTrees(Node *toNodePtr, Node *fromNodePtr,
			   Parser *parserPtr)
{
    Node	      *nodePtr, *toChildPtr, *nextPtr;
    Object	      *objectPtr;

    /* (1) merge lists of Objects for this node */
    if (fromNodePtr->firstObjectPtr) {
	if (!toNodePtr->firstObjectPtr) {
	    toNodePtr->firstObjectPtr = fromNodePtr->firstObjectPtr;
	    toNodePtr->lastObjectPtr = fromNodePtr->lastObjectPtr;
	} else {
	    fromNodePtr->firstObjectPtr->prevSameNodePtr =
		toNodePtr->lastObjectPtr;
	    toNodePtr->lastObjectPtr->nextSameNodePtr =
		fromNodePtr->firstObjectPtr;
	    toNodePtr->lastObjectPtr = fromNodePtr->lastObjectPtr;
	}
    }
    for (objectPtr = fromNodePtr->firstObjectPtr;
	 objectPtr; objectPtr = objectPtr->nextSameNodePtr) {
	objectPtr->nodePtr = toNodePtr;

    }
    
    /* (2) loop: merge all first-level `from' sub-trees to `to' */
    /* adjust all `from' sub-nodes' parentPtrs */
    for (nodePtr = fromNodePtr->firstChildPtr; nodePtr;
	 nodePtr = nodePtr->nextPtr) {
	nodePtr->parentPtr = toNodePtr;
    }
    if (!toNodePtr->firstChildPtr) {
	/*
	 * if `to' has no sub-nodes, just move the `from' sub-nodes.
	 */
	toNodePtr->firstChildPtr = fromNodePtr->firstChildPtr;
	toNodePtr->lastChildPtr = fromNodePtr->lastChildPtr;
    } else {
	/*
	 * otherwise, we really have to merge both trees...
	 */
	for (nodePtr = fromNodePtr->firstChildPtr; nodePtr; ) {
	    nextPtr = nodePtr->nextPtr;
	    if ((toChildPtr = findNodeByParentAndSubid(toNodePtr,
						       nodePtr->subid))) {
		/*
		 * if a sub-node with the same subid is already present
		 * in `to', merge them recursively.
		 */
		mergeNodeTrees(toChildPtr, nodePtr, parserPtr);
	    } else {
		/*
		 * otherwise, move the sub-tree from `from' to `to'.
		 */
		if (nodePtr->subid < toNodePtr->firstChildPtr->subid) {
		    /* move to the head. */
		    nodePtr->nextPtr = toNodePtr->firstChildPtr;
		    toNodePtr->firstChildPtr = nodePtr;
		} else if (nodePtr->subid > toNodePtr->lastChildPtr->subid) {
		    /* move to the end. */
		    nodePtr->prevPtr = toNodePtr->lastChildPtr;
		    toNodePtr->lastChildPtr->nextPtr = nodePtr;
		    toNodePtr->lastChildPtr = nodePtr;
		} else {
		    /* move to the appropriate place in the `to' list. */
		    for (toChildPtr = toNodePtr->firstChildPtr;
			 toChildPtr->nextPtr->subid < nodePtr->subid;
			 toChildPtr = toChildPtr->nextPtr);
		    toChildPtr->nextPtr->prevPtr = nodePtr;
		    nodePtr->nextPtr = toChildPtr->nextPtr;
		    nodePtr->prevPtr = toChildPtr;
		    toChildPtr->nextPtr = nodePtr;
		}
	    }
	    nodePtr = nextPtr;
	}
    }

    smiFree(fromNodePtr);
}



/*
 *----------------------------------------------------------------------
 *
 * setObjectName --
 *
 *      Set the name of a given Object. Combine two Objects if the name
 *	already exists.
 *
 * Results:
 *	(Object *) of the potentially combined object.
 *
 * Side effects:
 *      None.
 *
 *----------------------------------------------------------------------
 */

Object *setObjectName(Object *objectPtr, char *name, Parser *parserPtr)
{
    Node	      *nodePtr, *nextPtr;
    Module	      *modulePtr;
    Object	      *newObjectPtr;

    if (objectPtr->export.name) {
	smiFree(objectPtr->export.name);
    }
    objectPtr->export.name = name;
    /*
     * If this name is found on the pending list (at depth==1 in
     * pendingRootNode), we have to move the corresponding subtree to
     * the main tree.
     */
    for (nodePtr = parserPtr->pendingNodePtr->firstChildPtr; nodePtr;
	 nodePtr = nextPtr) {

	/*
	 * probably we change the contents of `pending', so remember
	 * the next pointer.
	 */
	nextPtr = nodePtr->nextPtr;

	if (!strcmp(nodePtr->firstObjectPtr->export.name, name)) {

	    /*
	     * remove nodePtr from the pendingRootNode tree.
	     */
	    if (nodePtr->prevPtr) {
		nodePtr->prevPtr->nextPtr = nodePtr->nextPtr;
	    } else {
		parserPtr->pendingNodePtr->firstChildPtr = nodePtr->nextPtr;
	    }
	    if (nodePtr->nextPtr) {
		nodePtr->nextPtr->prevPtr = nodePtr->prevPtr;
	    } else {
		parserPtr->pendingNodePtr->lastChildPtr = nodePtr->prevPtr;
	    }

#if 0
	    objectPtr->nodePtr->firstObjectPtr = NULL;
	    objectPtr->nodePtr->lastObjectPtr = NULL;
#else
	    if (objectPtr->nodePtr->lastObjectPtr != NULL) {
		if (objectPtr->nodePtr->lastObjectPtr->export.oid == NULL) {
		    objectPtr->nodePtr->lastObjectPtr = 
			objectPtr->nodePtr->lastObjectPtr->prevSameNodePtr;
		    if (objectPtr->nodePtr->lastObjectPtr == NULL) {
			objectPtr->nodePtr->firstObjectPtr = NULL;
		    }
		}
	    }
#endif
	    
	    newObjectPtr = nodePtr->firstObjectPtr;
	    if (newObjectPtr) {
		modulePtr = newObjectPtr->modulePtr;
		if (modulePtr->objectPtr == objectPtr) {
		    modulePtr->objectPtr = newObjectPtr;
		}
		if (modulePtr->firstObjectPtr == objectPtr) {
		    modulePtr->firstObjectPtr = objectPtr->nextPtr;
		    modulePtr->firstObjectPtr->prevPtr = NULL;
		}
		if (modulePtr->lastObjectPtr == objectPtr) {
		    modulePtr->lastObjectPtr = objectPtr->prevPtr;
		    modulePtr->lastObjectPtr->nextPtr = NULL;
		}

		mergeNodeTrees(objectPtr->nodePtr, nodePtr, parserPtr);
		smiFree(objectPtr->export.name);
		smiFree(objectPtr);
		return newObjectPtr;
	    } else {
		return objectPtr;
	    }
	}
    }
    return objectPtr;
}



/*
 *----------------------------------------------------------------------
 *
 * setObjectType --
 *
 *      Set the type (pointer to a Type struct) of a given Object.
 *
 * Results:
 *	None.
 *
 * Side effects:
 *      None.
 *
 *----------------------------------------------------------------------
 */

void setObjectType(Object *objectPtr, Type *typePtr)
{
    objectPtr->typePtr = typePtr;
}



/*
 *----------------------------------------------------------------------
 *
 * setObjectAccess --
 *
 *      Set the access of a given Object.
 *
 * Results:
 *	None.
 *
 * Side effects:
 *      None.
 *
 *----------------------------------------------------------------------
 */

void setObjectAccess(Object *objectPtr, SmiAccess access)
{
    objectPtr->export.access = access;
}



/*
 *----------------------------------------------------------------------
 *
 * setObjectStatus --
 *
 *      Set the status of a given Object.
 *
 * Results:
 *	None.
 *
 * Side effects:
 *      None.
 *
 *----------------------------------------------------------------------
 */

void setObjectStatus(Object *objectPtr, SmiStatus status)
{
    objectPtr->export.status = status;
}



/*
 *----------------------------------------------------------------------
 *
 * setObjectDescription --
 *
 *      Set the description of a given Object.
 *
 * Results:
 *	None.
 *
 * Side effects:
 *      None.
 *
 *----------------------------------------------------------------------
 */

void setObjectDescription(Object *objectPtr, char *description,
			  Parser *parserPtr)
{
    if (objectPtr->export.description)
	smiFree(objectPtr->export.description);
    if (parserPtr->flags & SMI_FLAG_NODESCR) {
	smiFree(description);
	objectPtr->export.description = NULL;
    } else {
	objectPtr->export.description = description;
    }
}



/*
 *----------------------------------------------------------------------
 *
 * setObjectReference --
 *
 *      Set the reference of a given Object.
 *
 * Results:
 *	None.
 *
 * Side effects:
 *      None.
 *
 *----------------------------------------------------------------------
 */

void setObjectReference(Object *objectPtr, char *reference, Parser *parserPtr)
{
    if (objectPtr->export.reference)
	smiFree(objectPtr->export.reference);
    if (parserPtr->flags & SMI_FLAG_NODESCR) {
	smiFree(reference);
	objectPtr->export.reference = NULL;
    } else {
	objectPtr->export.reference = reference;
    }
}


/*
 *----------------------------------------------------------------------
 *
 * setObjectFormat --
 *
 *      Set the format of a given Object.
 *
 * Results:
 *	None.
 *
 * Side effects:
 *      None.
 *
 *----------------------------------------------------------------------
 */

void setObjectFormat(Object *objectPtr, char *format)
{
    if (objectPtr->export.format) smiFree(objectPtr->export.format);
    objectPtr->export.format = format;
}



/*
 *----------------------------------------------------------------------
 *
 * setObjectUnits --
 *
 *      Set the units of a given Object.
 *
 * Results:
 *	None.
 *
 * Side effects:
 *      None.
 *
 *----------------------------------------------------------------------
 */

void setObjectUnits(Object *objectPtr, char *units)
{
    if (objectPtr->export.units) smiFree(objectPtr->export.units);
    objectPtr->export.units = units;
}



/*
 *----------------------------------------------------------------------
 *
 * setObjectDecl --
 *
 *      Set the declaring macro of a given Object.
 *
 * Results:
 *	None.
 *
 * Side effects:
 *      None.
 *
 *----------------------------------------------------------------------
 */

void setObjectDecl(Object *objectPtr, SmiDecl decl)
{
    objectPtr->export.decl = decl;
}



/*
 *----------------------------------------------------------------------
 *
 * setObjectLine --
 *
 *      Set the line of definition of a given Object.
 *
 * Results:
 *	None.
 *
 * Side effects:
 *      None.
 *
 *----------------------------------------------------------------------
 */

void setObjectLine(Object *objectPtr, int line, Parser *parserPtr)
{
    if (line) {
	objectPtr->line = line;
    } else {
	objectPtr->line = parserPtr ? parserPtr->line : -1;
    }
}



/*
 *----------------------------------------------------------------------
 *
 * setObjectNodekind --
 *
 *      Set the language independant SmiNodekind of a given Object.
 *
 * Results:
 *	None.
 *
 * Side effects:
 *      None.
 *
 *----------------------------------------------------------------------
 */

void setObjectNodekind(Object *objectPtr, SmiNodekind nodekind)
{
    objectPtr->export.nodekind = nodekind;
}



/*
 *----------------------------------------------------------------------
 *
 * addObjectFlags --
 *
 *      Add flags to the flags of a given Object.
 *
 * Results:
 *	None.
 *
 * Side effects:
 *      None.
 *
 *----------------------------------------------------------------------
 */

void addObjectFlags(Object *objectPtr, ObjectFlags flags)
{
    objectPtr->flags |= flags;
}



/*
 *----------------------------------------------------------------------
 *
 * deleteObjectFlags --
 *
 *      Delete flags from the flags of a given Object.
 *
 * Results:
 *	None.
 *
 * Side effects:
 *      None.
 *
 *----------------------------------------------------------------------
 */

void deleteObjectFlags(Object *objectPtr, ObjectFlags flags)
{
    objectPtr->flags &= ~flags;
}



/*
 *----------------------------------------------------------------------
 *
 * checkObjectFlag --
 *
 *      Check whether a given set of flags of a given Object are all set.
 *
 * Results:
 *	true if all named flags are set.
 *
 * Side effects:
 *      None.
 *
 *----------------------------------------------------------------------
 */

int checkObjectFlags(Object *objectPtr, ObjectFlags flags)
{
    return ((objectPtr->flags & flags) == flags);
}



/*
 *----------------------------------------------------------------------
 *
 * setObjectIndex --
 *
 *      Set the list of INDEX elements of a given Object.
 *
 * Results:
 *	None.
 *
 * Side effects:
 *      None.
 *
 *----------------------------------------------------------------------
 */

/* TODO remove me
void setObjectIndex(Object *objectPtr, Index *indexPtr)
    Object	 *objectPtr;
    Index	 *indexPtr;
{
    objectPtr->indexPtr = indexPtr;
}
*/


/*
 *----------------------------------------------------------------------
 *
 * setObjectList --
 *
 *      Set the list of objects of a notification type or object group
 *	or the list of notifications of a notification group.
 *
 * Results:
 *	None.
 *
 * Side effects:
 *      None.
 *
 *----------------------------------------------------------------------
 */

void setObjectList(Object *objectPtr, List *listPtr)
{
    objectPtr->listPtr = listPtr;
}



/*
 *----------------------------------------------------------------------
 *
 * setObjectRelated --
 *
 *      Set the related object of a given object (e.g. SMIv2 AUGMENTS)
 *
 * Results:
 *	None.
 *
 * Side effects:
 *      None.
 *
 *----------------------------------------------------------------------
 */

void setObjectRelated(Object *objectPtr, Object *relatedPtr)
{
    objectPtr->relatedPtr = relatedPtr;
}



/*
 *----------------------------------------------------------------------
 *
 * setObjectImplied --
 *
 *      Set the implied flag of a given object
 *
 * Results:
 *	None.
 *
 * Side effects:
 *      None.
 *
 *----------------------------------------------------------------------
 */

void setObjectImplied(Object *objectPtr, int implied)
{
    objectPtr->export.implied = implied;
}



/*
 *----------------------------------------------------------------------
 *
 * setObjectCreate --
 *
 *      Set the create flag of a given (table entry) object
 *
 * Results:
 *	None.
 *
 * Side effects:
 *      None.
 *
 *----------------------------------------------------------------------
 */

void setObjectCreate(Object *objectPtr, int create)
{
    objectPtr->export.create = create;
}



/*
 *----------------------------------------------------------------------
 *
 * setObjectIndexkind --
 *
 *      Set the indexkind of a given (table entry) object
 *
 * Results:
 *	None.
 *
 * Side effects:
 *      None.
 *
 *----------------------------------------------------------------------
 */

void setObjectIndexkind(Object *objectPtr, SmiIndexkind indexkind)
{
    objectPtr->export.indexkind = indexkind;
}



/*
 *----------------------------------------------------------------------
 *
 * setObjectValue --
 *
 *      Set the default value pointer of a given Object.
 *
 * Results:
 *	None.
 *
 * Side effects:
 *      None.
 *
 *----------------------------------------------------------------------
 */

void setObjectValue(Object *objectPtr, SmiValue *valuePtr)
{
    objectPtr->export.value = *valuePtr;
    smiFree(valuePtr);
}



/*
 *----------------------------------------------------------------------
 *
 * setObjectUniqueness --
 *
 *      Set the uniqueness entry of an object
 *
 * Results:
 *	None.
 *
 * Side effects:
 *      None.
 *
 *----------------------------------------------------------------------
 */

void setObjectUniqueness(Object *objectPtr, List *listPtr)
{
    objectPtr->uniquenessPtr = listPtr;
}



/*
 *----------------------------------------------------------------------
 *
 * setObjectInstallErrors --
 *
 *      Set the install errors entry of an object
 *
 * Results:
 *	None.
 *
 * Side effects:
 *      None.
 *
 *----------------------------------------------------------------------
 */

/*void setObjectInstallErrors(Object *objectPtr, List *listPtr)
{
    objectPtr->installErrorsPtr = listPtr;
}*/



/*
 *----------------------------------------------------------------------
 *
 * findNodeByParentAndSubid --
 *
 *      Lookup a Node by a given parent and subid value.
 *
 * Results:
 *      A pointer to the Node structure or
 *	NULL if it is not found.
 *
 * Side effects:
 *      None.
 *
 *----------------------------------------------------------------------
 */

Node *findNodeByParentAndSubid(Node *parentNodePtr, SmiSubid subid)
{
    Node *nodePtr;
    
    if (parentNodePtr &&
	(parentNodePtr != smiHandle->parserPtr->pendingNodePtr)) {
	for (nodePtr = parentNodePtr->firstChildPtr; nodePtr;
	     nodePtr = nodePtr->nextPtr) {
	    if (nodePtr->subid == subid) {
		return (nodePtr);
	    }
	}
    }
    
    return (NULL);
}



/*
 *----------------------------------------------------------------------
 *
 * findNodeByOid --
 *
 *      Lookup a Node by a given array of numerical subids.
 *
 * Results:
 *      A pointer to the Node structure or
 *	NULL if it is not found.
 *
 * Side effects:
 *      None.
 *
 *----------------------------------------------------------------------
 */

Node *findNodeByOid(unsigned int oidlen, SmiSubid *oid)
{
    Node          *nodePtr;
    unsigned int  i;
    
    nodePtr = smiHandle->rootNodePtr;
    for(i = 0; i < oidlen && nodePtr; i++) {
	nodePtr = findNodeByParentAndSubid(nodePtr, oid[i]);
    }
    
    return (nodePtr);
}



/*
 *----------------------------------------------------------------------
 *
 * findNodeByOidString --
 *
 *      Lookup a Node by a given string of concatinated numerical subids.
 *
 * Results:
 *      A pointer to the Node structure or
 *	NULL if it is not found.
 *
 * Side effects:
 *      None.
 *
 *----------------------------------------------------------------------
 */

Node *findNodeByOidString(char *oid)
{
    Node *nodePtr;
    char *s;
    char *p;
    
    s = smiStrdup(oid);
    nodePtr = smiHandle->rootNodePtr;
    for(p = strtok(s, ". "); p && nodePtr; p = strtok(NULL, ". ")) {
	nodePtr = findNodeByParentAndSubid(nodePtr, atoi(p));
    }
    
    smiFree(s);
    return (nodePtr);
}



/*
 *----------------------------------------------------------------------
 *
 * findObjectByNode --
 *
 *      Lookup an Object by a given Node. Note, that there might be
 *	multiple definitions for one node.
 *
 * Results:
 *      A pointer to the first Object structure in the current View or
 *	a pointer to the first Object if none is in the current View or
 *	NULL if it is not found.
 *
 * Side effects:
 *      None.
 *
 *----------------------------------------------------------------------
 */

Object *findObjectByNode(Node *nodePtr)
{
    Object    *objectPtr;
    Object    *goodObjectPtr = NULL;

    /* first, try to find an object in the current view. */
    for (objectPtr = nodePtr->firstObjectPtr; objectPtr;
	 objectPtr = objectPtr->nextSameNodePtr) {
	if (isInView(objectPtr->modulePtr->export.name)) {
	    if (! goodObjectPtr) {
		goodObjectPtr = objectPtr;
	    } else if (objectPtr->modulePtr->export.language
		       > goodObjectPtr->modulePtr->export.language) {
		goodObjectPtr = objectPtr;
	    }
	}
    }

    return goodObjectPtr ? goodObjectPtr : nodePtr->firstObjectPtr;
}



/*
 *----------------------------------------------------------------------
 *
 * findObjectByModuleAndNode --
 *
 *      Lookup an Object by a given Node and Module. This is necessary
 *	since there might be different declarations in different modules
 *	for the same OID.
 *
 * Results:
 *      A pointer to the Object structure or
 *	NULL if it is not found.
 *
 * Side effects:
 *      None.
 *
 *----------------------------------------------------------------------
 */

Object *findObjectByModuleAndNode(Module *modulePtr, Node *nodePtr)
{
    Object    *objectPtr;

    for (objectPtr = nodePtr->firstObjectPtr; objectPtr;
	 objectPtr = objectPtr->nextSameNodePtr) {
	if (objectPtr->modulePtr == modulePtr) {
	    return (objectPtr);
	}
    }

    return (NULL);
}



/*
 *----------------------------------------------------------------------
 *
 * findObjectByModulenameAndNode --
 *
 *      Lookup an Object by a given Node and Modulename. This is necessary
 *	since there might be different declarations in different modules
 *	for the same OID.
 *
 * Results:
 *      A pointer to the Object structure or
 *	NULL if it is not found.
 *
 * Side effects:
 *      None.
 *
 *----------------------------------------------------------------------
 */

Object *findObjectByModulenameAndNode(const char *modulename, Node *nodePtr)
{
    Object     *objectPtr;

    for (objectPtr = nodePtr->firstObjectPtr; objectPtr;
	 objectPtr = objectPtr->nextSameNodePtr) {
	if (!strcmp(objectPtr->modulePtr->export.name, modulename)) {
	    return (objectPtr);
	}
    }

    return (NULL);
}



/*
 *----------------------------------------------------------------------
 *
 * findObjectByName --
 *
 *      Lookup an Object by a given name. Note, that
 *	there might be more than one Object with the same name.
 *	In this case, it is undefined which Object is returned.
 *
 * Results:
 *      A pointer to the Object structure or
 *	NULL if it is not found.
 *
 * Side effects:
 *      None.
 *
 *----------------------------------------------------------------------
 */

Object *findObjectByName(const char *objectname)
{
    Module	     *modulePtr;
    Object           *objectPtr;

    for (modulePtr = smiHandle->firstModulePtr; modulePtr;
	 modulePtr = modulePtr->nextPtr) {
	for (objectPtr = modulePtr->firstObjectPtr; objectPtr;
	     objectPtr = objectPtr->nextPtr) {
	    if ((objectPtr->export.name) &&
		!strcmp(objectPtr->export.name, objectname)) {
		/*
		 * We return the first matching object.
		 * TODO: probably we should check if there are more matching
		 *       objects, and give a warning if there's another one.
		 */
		return (objectPtr);
	    }
	}
    }

    return (NULL);
}



/*
 *----------------------------------------------------------------------
 *
 * findNextObjectByName --
 *
 *      Lookup the next Object by a given name. Note, that
 *	there might be more than one Object with the same name.
 *
 * Results:
 *      A pointer to the Object structure or
 *	NULL if it is not found.
 *
 * Side effects:
 *      None.
 *
 *----------------------------------------------------------------------
 */

Object *findNextObjectByName(const char *objectname, Object *prevObjectPtr)
{
    Module	     *modulePtr;
    Object           *objectPtr;

    for (modulePtr = prevObjectPtr->modulePtr->nextPtr; modulePtr;
	 modulePtr = modulePtr->nextPtr) {
	for (objectPtr = modulePtr->firstObjectPtr; objectPtr;
	     objectPtr = objectPtr->nextPtr) {
	    if ((objectPtr->export.name)
		&& !strcmp(objectPtr->export.name, objectname)) {
		/*
		 * We return the first matching object.
		 * TODO: probably we should check if there are more matching
		 *       objects, and give a warning if there's another one.
		 */
		return (objectPtr);
	    }
	}
    }

    return (NULL);
}



/*
 *----------------------------------------------------------------------
 *
 * findObjectByModulenameAndName --
 *
 *      Lookup a Object by a given Module and name.
 *
 * Results:
 *      A pointer to the Object structure or
 *	NULL if it is not found.
 *
 * Side effects:
 *      None.
 *
 *----------------------------------------------------------------------
 */

Object *findObjectByModulenameAndName(const char *modulename,
				      const char *objectname)
{
    Module	     *modulePtr;
    Object	     *objectPtr;

    modulePtr = findModuleByName(modulename);

    if (modulePtr) {
	for (objectPtr = modulePtr->firstObjectPtr; objectPtr;
	     objectPtr = objectPtr->nextPtr) {
	    if ((objectPtr->export.name) &&
		!strcmp(objectPtr->export.name, objectname)) {
		return (objectPtr);
	    }
	}
    }

    /*
     * Some toplevel Objects seem to be always known.
     */
    if ((!strcmp(objectname, "iso")) ||
	(!strcmp(objectname, "ccitt")) ||
	(!strcmp(objectname, "joint-iso-ccitt"))) {
	return findObjectByName(objectname);
    }

    return (NULL);
}



/*
 *----------------------------------------------------------------------
 *
 * findObjectByModuleAndName --
 *
 *      Lookup a Object by a given Module and name.
 *
 * Results:
 *      A pointer to the Object structure or
 *	NULL if it is not found.
 *
 * Side effects:
 *      None.
 *
 *----------------------------------------------------------------------
 */

Object *findObjectByModuleAndName(Module *modulePtr, const char *objectname)
{
    Object	  *objectPtr;

    if (! objectname) {
	return NULL;
    }
    
    if (modulePtr) {
	for (objectPtr = modulePtr->firstObjectPtr; objectPtr;
	     objectPtr = objectPtr->nextPtr) {
	    if ((objectPtr->export.name) &&
		!strcmp(objectPtr->export.name, objectname)) {
		return (objectPtr);
	    }
	}
    }
    
    /*
     * Some toplevel Objects seem to be always known.
     */
    if ((!strcmp(objectname, "iso")) ||
	(!strcmp(objectname, "ccitt")) ||
	(!strcmp(objectname, "joint-iso-ccitt"))) {
	return findObjectByName(objectname);
    }
    
    return (NULL);
}



/*
 *----------------------------------------------------------------------
 *
 * addType --
 *
 *      Create a new Type structure.
 *
 * Results:
 *      A pointer to the new Type structure or
 *	NULL if terminated due to an error.
 *
 * Side effects:
 *      None.
 *
 *----------------------------------------------------------------------
 */

Type *addType(char *type_name, SmiBasetype basetype, TypeFlags flags,
	      Parser *parserPtr)
{
    Type	   *typePtr;
    Module	   *modulePtr;
    
    modulePtr = parserPtr ? parserPtr->modulePtr : NULL;
    
    typePtr = smiMalloc(sizeof(Type));

    typePtr->export.name	        = type_name;
    typePtr->export.basetype		= basetype;
    typePtr->export.decl		= SMI_DECL_UNKNOWN;
    typePtr->export.format		= NULL;
    typePtr->export.value.basetype	= SMI_BASETYPE_UNKNOWN;
    typePtr->export.units		= NULL;
    typePtr->export.status		= SMI_STATUS_UNKNOWN;
    typePtr->export.description		= NULL;
    typePtr->export.reference		= NULL;

    typePtr->modulePtr			= modulePtr;
    typePtr->listPtr			= NULL;
    typePtr->flags			= flags;
    typePtr->parentPtr                  = NULL;
    typePtr->line			= parserPtr ? parserPtr->line : -1;
    
    typePtr->nextPtr			= NULL;
    if (modulePtr) {
	typePtr->prevPtr		= modulePtr->lastTypePtr;
	if (!modulePtr->firstTypePtr)
	    modulePtr->firstTypePtr	= typePtr;
	if (modulePtr->lastTypePtr)
	    modulePtr->lastTypePtr->nextPtr = typePtr;
	modulePtr->lastTypePtr		= typePtr;
    } else {
	typePtr->prevPtr		= NULL;
    }
	
    return (typePtr);
}



/*
 *----------------------------------------------------------------------
 *
 * duplicateType --
 *
 *      Create a new Type as a duplicate of a given one but with
 *      an affiliation to the current module.
 *
 * Results:
 *      A pointer to the new Type structure or
 *	NULL if terminated due to an error.
 *
 * Side effects:
 *      None.
 *
 *----------------------------------------------------------------------
 */

Type *duplicateType(Type *templatePtr, TypeFlags flags, Parser *parserPtr)
{
    Type		  *typePtr;
    Module		  *modulePtr;
    
    typePtr = (Type *) smiMalloc(sizeof(Type));

    modulePtr = parserPtr->modulePtr;
    
    typePtr->export.name	        = NULL;
    typePtr->export.basetype		= templatePtr->export.basetype;
    typePtr->export.decl		= SMI_DECL_IMPLICIT_TYPE;
    typePtr->export.format		= NULL;
    typePtr->export.value.basetype	= SMI_BASETYPE_UNKNOWN;
    typePtr->export.units		= NULL;
    typePtr->export.status		= templatePtr->export.status;
    typePtr->export.description		= NULL;
    typePtr->export.reference		= NULL;

    typePtr->modulePtr			= modulePtr;
    typePtr->listPtr			= NULL;
    typePtr->flags			= templatePtr->flags;
    typePtr->line			= parserPtr ? parserPtr->line : -1;

    typePtr->nextPtr			= NULL;
    typePtr->prevPtr			= modulePtr->lastTypePtr;
    if (!modulePtr->firstTypePtr)
	modulePtr->firstTypePtr		= typePtr;
    if (modulePtr->lastTypePtr)
	modulePtr->lastTypePtr->nextPtr	= typePtr;
    modulePtr->lastTypePtr		= typePtr;

    setTypeParent(typePtr, templatePtr);
    
    return (typePtr);
}



/*
 *----------------------------------------------------------------------
 *
 * setTypeName --
 *
 *      Set the name of a given Type. If it already exists, merge the
 *	two types.
 *
 * Results:
 *	None.
 *
 * Side effects:
 *      None.
 *
 *----------------------------------------------------------------------
 */

Type *setTypeName(Type *typePtr, char *name)
{
    Type              *type2Ptr;
    List	      *listPtr;
    
    if (typePtr->export.name) {
	smiFree(typePtr->export.name);
    }
    typePtr->export.name = smiStrdup(name);

    if (! typePtr->export.name) {
	return typePtr;
    }
	
    /*
     * If a type with this name already exists, it must be a forward
     * reference and both types have to be merged.
     */
    for (type2Ptr = typePtr->modulePtr->firstTypePtr; type2Ptr;
	 type2Ptr = type2Ptr->nextPtr) {

	if (type2Ptr->export.name &&
	    (!strcmp(type2Ptr->export.name, name)) &&
	    (type2Ptr != typePtr)) {

	    /*
	     * remove typePtr from the type list.
	     */
	    if (typePtr->prevPtr) {
		typePtr->prevPtr->nextPtr = typePtr->nextPtr;
	    } else {
		typePtr->modulePtr->firstTypePtr = typePtr->nextPtr;
	    }
	    if (typePtr->nextPtr) {
		typePtr->nextPtr->prevPtr = typePtr->prevPtr;
	    } else {
		typePtr->modulePtr->lastTypePtr = typePtr->prevPtr;
	    }

	    type2Ptr->export.basetype     = typePtr->export.basetype;
	    type2Ptr->export.decl         = typePtr->export.decl;
	    type2Ptr->export.format       = typePtr->export.format;
	    type2Ptr->export.value        = typePtr->export.value;
	    type2Ptr->export.units        = typePtr->export.units;
	    type2Ptr->export.status       = typePtr->export.status;
	    type2Ptr->export.description  = typePtr->export.description;
	    type2Ptr->export.reference    = typePtr->export.reference;
	    
	    type2Ptr->parentPtr    = typePtr->parentPtr;
	    type2Ptr->listPtr      = typePtr->listPtr;
	    type2Ptr->flags        = typePtr->flags;
	    type2Ptr->line         = typePtr->line;

	    /*
	     * if it's an enum or bits type, we also have to adjust
	     * the references from the named numbers back to the type.
	     */
	    if ((type2Ptr->export.basetype == SMI_BASETYPE_ENUM) ||
		(type2Ptr->export.basetype == SMI_BASETYPE_BITS)) {
		for (listPtr = type2Ptr->listPtr; listPtr;
		     listPtr = listPtr->nextPtr) {
		    ((NamedNumber *)(listPtr->ptr))->typePtr = type2Ptr;
		}
	    }
	    
	    smiFree(typePtr->export.name);
	    smiFree(typePtr);

	    return type2Ptr;
	}
    }
    return typePtr;
}



/*
 *----------------------------------------------------------------------
 *
 * setTypeParent --
 *
 *      Set the parent of a given Type.
 *
 * Results:
 *	None.
 *
 * Side effects:
 *      None.
 *
 *----------------------------------------------------------------------
 */

void setTypeParent(Type *typePtr, Type *parentPtr)
{
    typePtr->parentPtr = parentPtr;
}



/*
 *----------------------------------------------------------------------
 *
 * setTypeStatus --
 *
 *      Set the status of a given Type.
 *
 * Results:
 *	None.
 *
 * Side effects:
 *      None.
 *
 *----------------------------------------------------------------------
 */

void setTypeStatus(Type *typePtr, SmiStatus status)
{
    typePtr->export.status = status;
}



/*
 *----------------------------------------------------------------------
 *
 * setTypeBasetype --
 *
 *      Set the basetype of a given Type.
 *
 * Results:
 *	None.
 *
 * Side effects:
 *      None.
 *
 *----------------------------------------------------------------------
 */

void setTypeBasetype(Type *typePtr, SmiBasetype basetype)
{
    typePtr->export.basetype = basetype;
}



/*
 *----------------------------------------------------------------------
 *
 * setTypeDescription --
 *
 *      Set the description of a given Type.
 *
 * Results:
 *	None.
 *
 * Side effects:
 *      None.
 *
 *----------------------------------------------------------------------
 */

void setTypeDescription(Type *typePtr, char *description, Parser *parserPtr)
{
    if (typePtr->export.description)
	smiFree(typePtr->export.description);
    if (parserPtr->flags & SMI_FLAG_NODESCR) {
	smiFree(description);
	typePtr->export.description = NULL;
    } else {
	typePtr->export.description = description;
    }
}



/*
 *----------------------------------------------------------------------
 *
 * setTypeReference --
 *
 *      Set the reference of a given Type.
 *
 * Results:
 *	None.
 *
 * Side effects:
 *      None.
 *
 *----------------------------------------------------------------------
 */

void setTypeReference(Type *typePtr, char *reference, Parser *parserPtr)
{
    if (typePtr->export.reference)
	smiFree(typePtr->export.reference);
    if (parserPtr->flags & SMI_FLAG_NODESCR) {
	smiFree(reference);
	typePtr->export.reference = NULL;
    } else {
	typePtr->export.reference = reference;
    }
}



/*
 *----------------------------------------------------------------------
 *
 * setTypeList --
 *
 *      Set the pointer to a struct list. This used for
 *	- columns of a SEQUENCE type,
 *	- enumeration items of an enumeration integer type,
 *	- min-max pair items of a range restricted type,
 *	- min-max pars items of a size restricted type.
 *
 * Results:
 *	None.
 *
 * Side effects:
 *      None.
 *
 *----------------------------------------------------------------------
 */

void setTypeList(Type *typePtr, List *listPtr)
{
    if (!typePtr->listPtr) {
	typePtr->listPtr  = listPtr;
    }
}



/*
 *----------------------------------------------------------------------
 *
 * setTypeFormat --
 *
 *      Set the format (displayHint) of a given Type.
 *
 * Results:
 *	None.
 *
 * Side effects:
 *      None.
 *
 *----------------------------------------------------------------------
 */

void setTypeFormat(Type *typePtr, char *format)
{
    if (typePtr->export.format) smiFree(typePtr->export.format);
    typePtr->export.format = format;
}



/*
 *----------------------------------------------------------------------
 *
 * setTypeUnits --
 *
 *      Set the units of a given Type. Note: units of types are only
 *	present in SMIng, not in SMIv2.
 *
 * Results:
 *	None.
 *
 * Side effects:
 *      None.
 *
 *----------------------------------------------------------------------
 */

void setTypeUnits(Type *typePtr, char *units)
{
    if (typePtr->export.units) smiFree(typePtr->export.units);
    typePtr->export.units = units;
}



/*
 *----------------------------------------------------------------------
 *
 * setTypeDecl --
 *
 *      Set the declaring macro of a given Type.
 *
 * Results:
 *	None.
 *
 * Side effects:
 *      None.
 *
 *----------------------------------------------------------------------
 */

void setTypeDecl(Type *typePtr, SmiDecl decl)
{
    typePtr->export.decl = decl;
}



/*
 *----------------------------------------------------------------------
 *
 * setTypeLine --
 *
 *      Set the line of definition of a given Type.
 *
 * Results:
 *	None.
 *
 * Side effects:
 *      None.
 *
 *----------------------------------------------------------------------
 */

void setTypeLine(Type *typePtr, int line, Parser *parserPtr)
{
    if (line) {
	typePtr->line = line;
    } else {
	typePtr->line = parserPtr ? parserPtr->line : -1;
    }
}



/*
 *----------------------------------------------------------------------
 *
 * setTypeValue --
 *
 *      Set the default value pointer of a given Type.
 *
 * Results:
 *	None.
 *
 * Side effects:
 *      None.
 *
 *----------------------------------------------------------------------
 */

void setTypeValue(Type *typePtr, SmiValue *valuePtr)
{
    typePtr->export.value = *valuePtr;
}



/*
 *----------------------------------------------------------------------
 *
 * addTypeFlags --
 *
 *      Add flags to the flags of a given Type.
 *
 * Results:
 *	None.
 *
 * Side effects:
 *      None.
 *
 *----------------------------------------------------------------------
 */

void addTypeFlags(Type *typePtr, TypeFlags flags)
{
    typePtr->flags |= flags;
}



/*
 *----------------------------------------------------------------------
 *
 * deleteTypeFlags --
 *
 *      Delete flags from the flags of a given Type.
 *
 * Results:
 *	None.
 *
 * Side effects:
 *      None.
 *
 *----------------------------------------------------------------------
 */

void deleteTypeFlags(Type *typePtr, TypeFlags flags)
{
    typePtr->flags &= ~flags;
}



/*
 *----------------------------------------------------------------------
 *
 * findTypeByName --
 *
 *      Lookup a Type by a given name.
 *
 * Results:
 *      A pointer to the Type structure or
 *	NULL if it is not found.
 *
 * Side effects:
 *      None.
 *
 *----------------------------------------------------------------------
 */

Type * findTypeByName(const char *type_name)
{
    Module *modulePtr;
    Type   *typePtr;
    
    for (modulePtr = smiHandle->firstModulePtr; modulePtr;
	 modulePtr = modulePtr->nextPtr) {
	for (typePtr = modulePtr->firstTypePtr; typePtr;
	     typePtr = typePtr->nextPtr) {
	    if ((typePtr->export.name) &&
		!strcmp(typePtr->export.name, type_name)) {
		return (typePtr);
	    }
	}
    }

    return (NULL);
}



/*
 *----------------------------------------------------------------------
 *
 * findNextTypeByName --
 *
 *      Lookup the next Type by a given name.
 *
 * Results:
 *      A pointer to the Type structure or
 *	NULL if it is not found.
 *
 * Side effects:
 *      None.
 *
 *----------------------------------------------------------------------
 */

Type *findNextTypeByName(const char *type_name, Type *prevTypePtr)
{
    Module *modulePtr;
    Type   *typePtr;
    
    for (modulePtr = prevTypePtr->modulePtr->nextPtr; modulePtr;
	 modulePtr = modulePtr->nextPtr) {
	for (typePtr = modulePtr->firstTypePtr; typePtr;
	     typePtr = typePtr->nextPtr) {
	    if ((typePtr->export.name) &&
		!strcmp(typePtr->export.name, type_name)) {
		return (typePtr);
	    }
	}
    }

    return (NULL);
}



/*
 *----------------------------------------------------------------------
 *
 * findTypeByModulenameAndName --
 *
 *      Lookup a Type by a given Module and name.
 *
 * Results:
 *      A pointer to the Type structure or
 *	NULL if it is not found.
 *
 * Side effects:
 *      None.
 *
 *----------------------------------------------------------------------
 */

Type *findTypeByModulenameAndName(const char *modulename,
				  const char *type_name)
{
    Type       *typePtr;
    Module     *modulePtr;

    modulePtr = findModuleByName(modulename);

    if (modulePtr) {
	for (typePtr = modulePtr->firstTypePtr; typePtr;
	     typePtr = typePtr->nextPtr) {
	    if ((typePtr->export.name) && !strcmp(typePtr->export.name, type_name)) {
		return (typePtr);
	    }
	}
    }
	
    return (NULL);
}



/*
 *----------------------------------------------------------------------
 *
 * findTypeByModuleAndName --
 *
 *      Lookup a Type by a given Module and name.
 *
 * Results:
 *      A pointer to the Type structure or
 *	NULL if it is not found.
 *
 * Side effects:
 *      None.
 *
 *----------------------------------------------------------------------
 */

Type *findTypeByModuleAndName(Module *modulePtr, const char *type_name)
{
    Type        *typePtr;

    if (modulePtr) {
	for (typePtr = modulePtr->firstTypePtr; typePtr;
	     typePtr = typePtr->nextPtr) {
	    if ((typePtr->export.name) &&
		!strcmp(typePtr->export.name, type_name)) {
		return (typePtr);
	    }
	}
    }

    return (NULL);
}



/*
 *----------------------------------------------------------------------
 *
 * findTypeNamedNumber --
 *
 *      
 *
 * Results:
 *
 * Side effects:
 *
 *---------------------------------------------------------------------- */

NamedNumber *findTypeNamedNumber(Type *typePtr,
				 SmiInteger32 number)
{
    List *listPtr;
    
    for (listPtr = typePtr->listPtr;
	 listPtr; listPtr = listPtr->nextPtr) {
	if (((NamedNumber *)(listPtr->ptr))->export.value.value.integer32 ==
	    number)
	    break;
    }

    return (NamedNumber *)(listPtr->ptr);
}



/*
 *----------------------------------------------------------------------
 *
 * addIdentity --
 *
 *      Create a new Identity structure.
 *
 * Results:
 *      A pointer to the new Identity structure or
 *	NULL if terminated due to an error.
 *
 * Side effects:
 *      None.
 *
 *---------------------------------------------------------------------- */

Identity *addIdentity(char *identityname, Parser *parserPtr)
{
	Identity  *identityPtr;
    Module	  *modulePtr;
    
    modulePtr = parserPtr->modulePtr;
    
    identityPtr = (Identity*) smiMalloc(sizeof(Identity));
    
    identityPtr->export.name = identityname;
    identityPtr->export.status      = SMI_STATUS_UNKNOWN;
    identityPtr->export.description = NULL;
    identityPtr->export.reference   = NULL;
	identityPtr->parentPtr   = NULL;
	
    identityPtr->modulePtr   	 	= parserPtr->modulePtr;
    identityPtr->line	  	 		= parserPtr ? parserPtr->line : -1;
    
    identityPtr->nextPtr			= NULL;
    identityPtr->prevPtr			= modulePtr->lastIdentityPtr;
    if (!modulePtr->firstIdentityPtr)
		modulePtr->firstIdentityPtr	= identityPtr;
    if (modulePtr->lastIdentityPtr)
		modulePtr->lastIdentityPtr->nextPtr	= identityPtr;
    modulePtr->lastIdentityPtr		= identityPtr;
    
    return (identityPtr);
    
}

/*
 *----------------------------------------------------------------------
 *
 * setIdentityDecl --
 *
 *      Set the declaring macro of a given Identity.
 *
 * Results:
 *	None.
 *
 * Side effects:
 *      None.
 *
 *----------------------------------------------------------------------
 */

void setIdentityDecl(Identity *identityPtr, SmiDecl decl)
{
    identityPtr->export.decl = decl;
}

/*
 *----------------------------------------------------------------------
 *
 * setIdentityStatus --
 *
 *      Set the status of a given Identity.
 *
 * Results:
 *	None.
 *
 * Side effects:
 *      None.
 *
 *----------------------------------------------------------------------
 */

void setIdentityStatus(Identity *identityPtr, SmiStatus status)
{
    identityPtr->export.status = status;
}



/*
 *----------------------------------------------------------------------
 *
 * setIdentityDescription --
 *
 *      Set the description of a given Identity.
 *
 * Results:
 *	None.
 *
 * Side effects:
 *      None.
 *
 *----------------------------------------------------------------------
 */

void setIdentityDescription(Identity *identityPtr, char *description, Parser *parserPtr)
{
    if (identityPtr->export.description) smiFree(identityPtr->export.description);
    if (parserPtr->flags & SMI_FLAG_NODESCR) {
	smiFree(description);
	identityPtr->export.description = NULL;
    } else {
	identityPtr->export.description = description;
    }
}



/*
 *----------------------------------------------------------------------
 *
 * setIdentityReference --
 *
 *      Set the reference of a given Identity.
 *
 * Results:
 *	None.
 *
 * Side effects:
 *      None.
 *
 *----------------------------------------------------------------------
 */

void setIdentityReference(Identity *identityPtr, char *reference, Parser *parserPtr)
{
    if (identityPtr->export.reference)
	smiFree(identityPtr->export.reference);
    if (parserPtr->flags & SMI_FLAG_NODESCR) {
	smiFree(reference);
	identityPtr->export.reference = NULL;
    } else {
	identityPtr->export.reference = reference;
    }
}

/*
 *----------------------------------------------------------------------
 *
 * setIdentityParent --
 *
 *      Set the parent of a given Identity to given Identity pointer.
 *
 * Results:
 *	None.
 *
 * Side effects:
 *      None.
 *
 *----------------------------------------------------------------------
 */

void setIdentityParent(Identity *identityPtr, Identity *parentPtr)
{
    if(identityPtr) identityPtr->parentPtr = parentPtr;
}


/*
 *----------------------------------------------------------------------
 *
 * findIdentityByName --
 *
 *      Lookup a Identity by a given name.
 *
 * Results:
 *      A pointer to the Identity structure or
 *	NULL if it is not found.
 *
 * Side effects:
 *      None.
 *
 *----------------------------------------------------------------------
 */

Identity *findIdentityByName(const char *identityname)
{
    Module *modulePtr;
    Identity   *identityPtr;
    
    for (modulePtr = smiHandle->firstModulePtr; modulePtr;
	 modulePtr = modulePtr->nextPtr) {
	for (identityPtr = modulePtr->firstIdentityPtr; identityPtr;
	     identityPtr = identityPtr->nextPtr) {
	    if ((identityPtr->export.name) &&
		!strcmp(identityPtr->export.name, identityname)) {
		return (identityPtr);
	    }
	}
    }

    return (NULL);
}

/*
 *----------------------------------------------------------------------
 *
 * findIdentityByModuleAndName --
 *
 *      Lookup a Identity by a given name and Module.
 *
 * Results:
 *      A pointer to the Identity structure or
 *	NULL if it is not found.
 *
 * Side effects:
 *      None.
 *
 *----------------------------------------------------------------------
 */

Identity *findIdentityByModuleAndName(Module *modulePtr,
									const char *identityname)
{
    Identity   *identityPtr;
    
    
	for (identityPtr = modulePtr->firstIdentityPtr; identityPtr;
	     identityPtr = identityPtr->nextPtr) {
	    if ((identityPtr->export.name) &&
		!strcmp(identityPtr->export.name, identityname)) {
		return (identityPtr);
	    }
	}


    return (NULL);
}

/*
 *----------------------------------------------------------------------
 *
 * findIdentityByModulenameAndName --
 *
 *      Lookup a Identity by a given Module and name.
 *
 * Results:
 *      A pointer to the Identity structure or
 *	NULL if it is not found.
 *
 * Side effects:
 *      None.
 *
 *----------------------------------------------------------------------
 */

Identity *findIdentityByModulenameAndName(const char *modulename,
				  const char *identity_name)
{
    Identity       *identityPtr;
    Module     *modulePtr;

    modulePtr = findModuleByName(modulename);

    if (modulePtr) {
	for (identityPtr = modulePtr->firstIdentityPtr; identityPtr;
	     identityPtr = identityPtr->nextPtr) {
	    if ((identityPtr->export.name) && !strcmp(identityPtr->export.name, identity_name)) {
		return (identityPtr);
	    }
	}
    }
	
    return (NULL);
}



/*
 *----------------------------------------------------------------------
 *
 * addClass --
 *
 *      Create a new Class structure.
 *
 * Results:
 *      A pointer to the new Class structure or
 *	NULL if terminated due to an error.
 *
 * Side effects:
 *      None.
 *
 *---------------------------------------------------------------------- */

Class *addClass(char *classname, Parser *parserPtr)
{
	Class  *classPtr;
    Module	  *modulePtr;
    
    modulePtr = parserPtr->modulePtr;
    
    classPtr = (Class*) smiMalloc(sizeof(Class));
    
    classPtr->export.name = classname;
    classPtr->export.status      = SMI_STATUS_UNKNOWN;
    classPtr->export.description = NULL;
    classPtr->export.reference   = NULL;
   
    classPtr->modulePtr   	 	= parserPtr->modulePtr;
    classPtr->line	  	 		= parserPtr ? parserPtr->line : -1;
    
    classPtr->parentPtr			= NULL;
    classPtr->firstAttributePtr = NULL;
    classPtr->lastAttributePtr  = NULL;
    classPtr->firstEventPtr 	= NULL;
    classPtr->lastEventPtr 		= NULL;
    
    classPtr->nextPtr			= NULL;
    classPtr->prevPtr			= modulePtr->lastClassPtr;
    if (!modulePtr->firstClassPtr)
		modulePtr->firstClassPtr	= classPtr;
    if (modulePtr->lastClassPtr)
		modulePtr->lastClassPtr->nextPtr	= classPtr;
    modulePtr->lastClassPtr		= classPtr;
    
    return (classPtr);
    
}

/*
 *----------------------------------------------------------------------
 *
 * setClassDecl --
 *
 *      Set the declaring macro of a given Class.
 *
 * Results:
 *	None.
 *
 * Side effects:
 *      None.
 *
 *----------------------------------------------------------------------
 */

void setClassDecl(Class *classPtr, SmiDecl decl)
{
    classPtr->export.decl = decl;
}

/*
 *----------------------------------------------------------------------
 *
 * setClassStatus --
 *
 *      Set the status of a given Class.
 *
 * Results:
 *	None.
 *
 * Side effects:
 *      None.
 *
 *----------------------------------------------------------------------
 */

void setClassStatus(Class *classPtr, SmiStatus status)
{
    classPtr->export.status = status;
}



/*
 *----------------------------------------------------------------------
 *
 * setClassDescription --
 *
 *      Set the description of a given Class.
 *
 * Results:
 *	None.
 *
 * Side effects:
 *      None.
 *
 *----------------------------------------------------------------------
 */

void setClassDescription(Class *classPtr, char *description, Parser *parserPtr)
{
    if (classPtr->export.description) smiFree(classPtr->export.description);
    if (parserPtr->flags & SMI_FLAG_NODESCR) {
	smiFree(description);
	classPtr->export.description = NULL;
    } else {
	classPtr->export.description = description;
    }
}



/*
 *----------------------------------------------------------------------
 *
 * setClassReference --
 *
 *      Set the reference of a given Class.
 *
 * Results:
 *	None.
 *
 * Side effects:
 *      None.
 *
 *----------------------------------------------------------------------
 */

void setClassReference(Class *classPtr, char *reference, Parser *parserPtr)
{
    if (classPtr->export.reference)
	smiFree(classPtr->export.reference);
    if (parserPtr->flags & SMI_FLAG_NODESCR) {
	smiFree(reference);
	classPtr->export.reference = NULL;
    } else {
	classPtr->export.reference = reference;
    }
}

/*
 *----------------------------------------------------------------------
 *
 * setClassParent --
 *
 *      Set the parent of a given Class to given Class pointer.
 *
 * Results:
 *	None.
 *
 * Side effects:
 *      None.
 *
 *----------------------------------------------------------------------
 */

void setClassParent(Class *classPtr, Class *parentPtr)
{
    if(classPtr) classPtr->parentPtr = parentPtr;
}

/*
 *----------------------------------------------------------------------
 *
 * findClassByModuleAndName --
 *
 *      Lookup a Class by a given name.
 *
 * Results:
 *      A pointer to the Class structure or
 *	NULL if it is not found.
 *
 * Side effects:
 *      None.
 *
 *----------------------------------------------------------------------
 */
Class *findClassByModuleAndName(Module *modulePtr,char *name)
{
    Class   *classPtr;
    
	for (classPtr = modulePtr->firstClassPtr; classPtr;
	     classPtr = classPtr->nextPtr) {
	    if ((classPtr->export.name) &&
		!strcmp(classPtr->export.name, name)) {
		return (classPtr);
	    }
	}

    return NULL;
}

/*
 *----------------------------------------------------------------------
 *
 * findClassByModulenameAndName --
 *
 *      Lookup a Class by a given Module and name.
 *
 * Results:
 *      A pointer to the Class structure or
 *	NULL if it is not found.
 *
 * Side effects:
 *      None.
 *
 *----------------------------------------------------------------------
 */

Class *findClassByModulenameAndName(const char *modulename,
				  const char *class_name)
{
    Class       *classPtr;
    Module     *modulePtr;

    modulePtr = findModuleByName(modulename);

    if (modulePtr) {
	for (classPtr = modulePtr->firstClassPtr; classPtr;
	     classPtr = classPtr->nextPtr) {
	    if ((classPtr->export.name) && !strcmp(classPtr->export.name, class_name)) {
		return (classPtr);
	    }
	}
    }
	
    return (NULL);
}

/*
 *----------------------------------------------------------------------
 *
 * duplicateTypeToAttribute --
 *
 *      Create a new Attribute as a duplicate of a given type but with
 *      an affiliation to the given Class.
 *
 * Results:
 *      A pointer to the new Attribute structure or
 *	NULL if terminated due to an error.
 *
 * Side effects:
 *      None.
 *
 *----------------------------------------------------------------------
 */

Attribute *duplicateTypeToAttribute(Type *templatePtr, Class *classPtr, Parser *parserPtr)
{
    Attribute		  *attributePtr;
    
    if(!classPtr) return NULL;
    
    attributePtr = (Attribute *) smiMalloc(sizeof(Attribute));

    
    attributePtr->export.name	        = NULL;
    attributePtr->export.basetype		= templatePtr->export.basetype;
    attributePtr->export.decl		= SMI_DECL_ATTRIBUTE;
    attributePtr->export.format		= NULL;
    attributePtr->export.value.basetype	= templatePtr->export.basetype;
    attributePtr->export.units		= NULL;
    attributePtr->export.status		= templatePtr->export.status;
    attributePtr->export.description		= NULL;
    attributePtr->export.reference		= NULL;

    attributePtr->classPtr			= classPtr;
    attributePtr->listPtr			= NULL;
    attributePtr->line			= parserPtr ? parserPtr->line : -1;

    attributePtr->nextPtr			= NULL;
    attributePtr->prevPtr			= classPtr->lastAttributePtr;
    if (!classPtr->firstAttributePtr)
	classPtr->firstAttributePtr		= attributePtr;
    if (classPtr->lastAttributePtr)
	classPtr->lastAttributePtr->nextPtr	= attributePtr;
    classPtr->lastAttributePtr		= attributePtr;

    setAttributeParentType(attributePtr, templatePtr);
    
    return (attributePtr);
}


/*
 *----------------------------------------------------------------------
 *
 * addAttribute --
 *
 *      Create a new Attribute structure and tie it to the given Class.
 *
 * Results:
 *      A pointer to the new Attribute structure or
 *	NULL if terminated due to an error.
 *
 * Side effects:
 *      None.
 *
 *----------------------------------------------------------------------
 */

Attribute *addAttribute(char *attribute_name,
	      Class *classPtr, Parser *parserPtr)
{
    Attribute	   *attributePtr;
    
    attributePtr = smiMalloc(sizeof(Attribute));

    attributePtr->export.name	        = attribute_name;
   	attributePtr->export.basetype		= SMI_BASETYPE_UNKNOWN;
    attributePtr->export.decl		= SMI_DECL_UNKNOWN;
    attributePtr->export.format		= NULL;
    attributePtr->export.value.basetype	= SMI_BASETYPE_UNKNOWN;
    attributePtr->export.units		= NULL;
    attributePtr->export.status		= SMI_STATUS_UNKNOWN;
    attributePtr->export.description		= NULL;
    attributePtr->export.reference		= NULL;

    attributePtr->classPtr			= classPtr;
    attributePtr->listPtr			= NULL;
    attributePtr->parentTypePtr     = NULL;
    attributePtr->parentClassPtr    = NULL;
    attributePtr->line			= parserPtr ? parserPtr->line : -1;
    
    attributePtr->nextPtr			= NULL;
    if (classPtr) {
	attributePtr->prevPtr		= classPtr->lastAttributePtr;
	if (!classPtr->firstAttributePtr)
	    classPtr->firstAttributePtr	= attributePtr;
	if (classPtr->lastAttributePtr)
	    classPtr->lastAttributePtr->nextPtr = attributePtr;
	classPtr->lastAttributePtr		= attributePtr;
    } else {
	attributePtr->prevPtr		= NULL;
    }
	
    return (attributePtr);
}

/*
 *----------------------------------------------------------------------
 *
 * setAttributeDecl --
 *
 *      Set the declaring macro of a given Attribute.
 *
 * Results:
 *	None.
 *
 * Side effects:
 *      None.
 *
 *----------------------------------------------------------------------
 */

void setAttributeDecl(Attribute *attributePtr, SmiDecl decl)
{
    attributePtr->export.decl = decl;
}

/*
 *----------------------------------------------------------------------
 *
 * setAttributeParentType --
 *
 *      Set the parent of a given attribute.
 *
 * Results:
 *	None.
 *
 * Side effects:
 *      None.
 *
 *----------------------------------------------------------------------
 */

void setAttributeParentType(Attribute *attributePtr, Type *parentPtr)
{
    attributePtr->parentTypePtr = parentPtr;
}

/*
 *----------------------------------------------------------------------
 *
 * setAttributeParentClass --
 *
 *      Set the parent Class of a given attribute.
 *
 * Results:
 *	None.
 *
 * Side effects:
 *      None.
 *
 *----------------------------------------------------------------------
 */

void setAttributeParentClass(Attribute *attributePtr, Class *parentPtr)
{
    attributePtr->parentClassPtr = parentPtr;
}

/*
 *----------------------------------------------------------------------
 *
 * setAttributeList --
 *
 *      Set the pointer to a struct list. This used for
 *	- enumeration items of an enumeration integer type,
 *	- min-max pair items of a range restricted type,
 *	- min-max pars items of a size restricted type.
 *
 * Results:
 *	None.
 *
 * Side effects:
 *      None.
 *
 *----------------------------------------------------------------------
 */

void setAttributeList(Attribute *attributePtr, List *listPtr)
{
    if (!attributePtr->listPtr) {
	attributePtr->listPtr  = listPtr;
    }
}

/*
 *----------------------------------------------------------------------
 *
 * setAttributeName --
 *
 *      Set the name of a given Attribute. 
 *
 * Results:
 *	None.
 *
 * Side effects:
 *      None.
 *
 *----------------------------------------------------------------------
 */

void setAttributeName(Attribute *attributePtr, char *name)
{
    
    attributePtr->export.name = smiStrdup(name);
}

/*
 *----------------------------------------------------------------------
 *
 * setAttributeAccess --
 *
 *      Set the access of a given Attribute. 
 *
 * Results:
 *	None.
 *
 * Side effects:
 *      None.
 *
 *----------------------------------------------------------------------
 */
void setAttributeAccess(Attribute *attributePtr,SmiAccess access)
{
	attributePtr->export.access = access;
}


/*
 *----------------------------------------------------------------------
 *
 * addEvent --
 *
 *      Create a new Event structure and tie it to the given Class.
 *
 * Results:
 *      A pointer to the new Event structure or
 *	NULL if terminated due to an error.
 *
 * Side effects:
 *      None.
 *
 *----------------------------------------------------------------------
 */
Event *addEvent(char *eventname, Class *classPtr, 
		             Parser *parserPtr)
{
    Event	   *eventPtr;
    
   eventPtr = smiMalloc(sizeof(Event));

   	eventPtr->export.name	    = eventname;
    eventPtr->export.decl		= SMI_DECL_EVENT;
    eventPtr->export.status	= SMI_STATUS_UNKNOWN;
    eventPtr->export.description		= NULL;
    eventPtr->export.reference		= NULL;

    eventPtr->classPtr			= classPtr;
    eventPtr->line			= parserPtr ? parserPtr->line : -1;
    
    eventPtr->nextPtr			= NULL;
    if (classPtr) {
	eventPtr->prevPtr		= classPtr->lastEventPtr;
	if (!classPtr->firstEventPtr)
	    classPtr->firstEventPtr	= eventPtr;
	if (classPtr->lastEventPtr)
	    classPtr->lastEventPtr->nextPtr = eventPtr;
	classPtr->lastEventPtr		= eventPtr;
    } else {
	eventPtr->prevPtr		= NULL;
    }
	
    return (eventPtr);
}



/*
 *----------------------------------------------------------------------
 *
 * addMacro --
 *
 *      Create a new Macro structure.
 *
 * Results:
 *      A pointer to the new Macro structure or
 *	NULL if terminated due to an error.
 *
 * Side effects:
 *      None.
 *
 *---------------------------------------------------------------------- */

Macro *addMacro(char *macroname, MacroFlags flags, Parser *parserPtr)
{
    Macro	  *macroPtr;
    Module	  *modulePtr;
    
    modulePtr = parserPtr->modulePtr;
    
    /* TODO: Check wheather this macro already exists?? */

    macroPtr = (Macro *) smiMalloc(sizeof(Macro));
	    
    macroPtr->export.name 	 = macroname;
    macroPtr->export.status      = SMI_STATUS_UNKNOWN;
    macroPtr->export.description = NULL;
    macroPtr->export.reference   = NULL;

    macroPtr->modulePtr   	 = parserPtr->modulePtr;
    macroPtr->flags       	 = flags;
    macroPtr->line	  	 = parserPtr ? parserPtr->line : -1;
    
    macroPtr->nextPtr				= NULL;
    macroPtr->prevPtr				= modulePtr->lastMacroPtr;
    if (!modulePtr->firstMacroPtr)
	modulePtr->firstMacroPtr		= macroPtr;
    if (modulePtr->lastMacroPtr)
	modulePtr->lastMacroPtr->nextPtr	= macroPtr;
    modulePtr->lastMacroPtr			= macroPtr;
    
    return (macroPtr);
}



/*
 *----------------------------------------------------------------------
 *
 * setMacroStatus --
 *
 *      Set the status of a given Macro.
 *
 * Results:
 *	None.
 *
 * Side effects:
 *      None.
 *
 *----------------------------------------------------------------------
 */

void setMacroStatus(Macro *macroPtr, SmiStatus status)
{
    macroPtr->export.status = status;
}



/*
 *----------------------------------------------------------------------
 *
 * setMacroDescription --
 *
 *      Set the description of a given Macro.
 *
 * Results:
 *	None.
 *
 * Side effects:
 *      None.
 *
 *----------------------------------------------------------------------
 */

void setMacroDescription(Macro *macroPtr, char *description, Parser *parserPtr)
{
    if (macroPtr->export.description) smiFree(macroPtr->export.description);
    if (parserPtr->flags & SMI_FLAG_NODESCR) {
	smiFree(description);
	macroPtr->export.description = NULL;
    } else {
	macroPtr->export.description = description;
    }
}



/*
 *----------------------------------------------------------------------
 *
 * setMacroReference --
 *
 *      Set the reference of a given Macro.
 *
 * Results:
 *	None.
 *
 * Side effects:
 *      None.
 *
 *----------------------------------------------------------------------
 */

void setMacroReference(Macro *macroPtr, char *reference, Parser *parserPtr)
{
    if (macroPtr->export.reference)
	smiFree(macroPtr->export.reference);
    if (parserPtr->flags & SMI_FLAG_NODESCR) {
	smiFree(reference);
	macroPtr->export.reference = NULL;
    } else {
	macroPtr->export.reference = reference;
    }
}

/*
 *----------------------------------------------------------------------
 *
 * setMacroAbnf --
 *
 *      Set the abnf string of a given extension(SMIng only).
 *
 * Results:
 *	None.
 *
 * Side effects:
 *      None.
 *
 *----------------------------------------------------------------------
 */

void setMacroAbnf(Macro *macroPtr, char *abnf, Parser *parserPtr)
{
    if (macroPtr->export.abnf)
	smiFree(macroPtr->export.abnf);
    if (parserPtr->flags & SMI_FLAG_NODESCR) {
	smiFree(abnf);
	macroPtr->export.abnf = NULL;
    } else {
	macroPtr->export.abnf = abnf;
    }
}



/*
 *----------------------------------------------------------------------
 *
 * setMacroDecl --
 *
 *      Set the declaring macro of a given Macro.
 *
 * Results:
 *	None.
 *
 * Side effects:
 *      None.
 *
 *----------------------------------------------------------------------
 */

void setMacroDecl(Macro *macroPtr, SmiDecl decl)
{
    macroPtr->export.decl = decl;
}



/*
 *----------------------------------------------------------------------
 *
 * setMacroLine --
 *
 *      Set the line of definition of a given Macro.
 *
 * Results:
 *	None.
 *
 * Side effects:
 *      None.
 *
 *----------------------------------------------------------------------
 */

void setMacroLine(Macro *macroPtr, int line, Parser *parserPtr)
{
    if (line) {
	macroPtr->line = line;
    } else {
	macroPtr->line = parserPtr ? parserPtr->line : -1;
    }
}



/*
 *----------------------------------------------------------------------
 *
 * findMacroByName --
 *
 *      Lookup a Macro by a given name.
 *
 * Results:
 *      A pointer to the Macro structure or
 *	NULL if it is not found.
 *
 * Side effects:
 *      None.
 *
 *----------------------------------------------------------------------
 */

Macro *findMacroByName(const char *macroname)
{
    Module *modulePtr;
    Macro   *macroPtr;
    
    for (modulePtr = smiHandle->firstModulePtr; modulePtr;
	 modulePtr = modulePtr->nextPtr) {
	for (macroPtr = modulePtr->firstMacroPtr; macroPtr;
	     macroPtr = macroPtr->nextPtr) {
	    if ((macroPtr->export.name) &&
		!strcmp(macroPtr->export.name, macroname)) {
		return (macroPtr);
	    }
	}
    }

    return (NULL);
}



/*
 *----------------------------------------------------------------------
 *
 * findMacroByModuleAndName --
 *
 *      Lookup a Macro by a given Module and name.
 *
 * Results:
 *      A pointer to the Macro structure or
 *	NULL if it is not found.
 *
 * Side effects:
 *      None.
 *
 *----------------------------------------------------------------------
 */

Macro *findMacroByModuleAndName(Module *modulePtr, const char *macroname)
{
    Macro      *macroPtr;
    
    if (modulePtr) {
	for (macroPtr = modulePtr->firstMacroPtr; macroPtr;
	     macroPtr = macroPtr->nextPtr) {
	    if (!strcmp(macroPtr->export.name, macroname)) {
		return (macroPtr);
	    }
	}
    }

    return (NULL);
}



/*
 *----------------------------------------------------------------------
 *
 * findNamedNumberByName --
 *
 *      Lookup the value of a namev nuber in a given typ.
 *
 * Results:
 *      A pointer to the NamedNumber structure or
 *	NULL if it is not found.
 *
 * Side effects:
 *      None.
 *
 *----------------------------------------------------------------------
 */

NamedNumber *findNamedNumberByName(Type *typePtr,const char *name)
{
	List *listPtr;
    
    if(typePtr->export.basetype != SMI_BASETYPE_ENUM && 
    	typePtr->export.basetype != SMI_BASETYPE_BITS) return NULL;
    	
    for (listPtr = typePtr->listPtr;
	 listPtr; listPtr = listPtr->nextPtr) {
	if (!strcmp(((NamedNumber *)(listPtr->ptr))->export.name 
	    , name))
	    return (NamedNumber *)(listPtr->ptr);
    }

    return NULL;
}





/*
 *----------------------------------------------------------------------
 *
 * smiInitData --
 *
 *      Initialize all need data structures at program start.
 *
 * Results:
 *      0 on success or -1 on an error.
 *
 * Side effects:
 *      None.
 *
 *----------------------------------------------------------------------
 */

int smiInitData()
{
    Object	    *objectPtr;
    Parser	    parser;
    
    smiHandle->flags = 0;
    
    smiHandle->firstModulePtr = NULL;
    smiHandle->lastModulePtr = NULL;
    smiHandle->firstViewPtr = NULL;
    smiHandle->lastViewPtr = NULL;
    
    /*
     * Initialize a root Node for the main MIB tree.
     */
    smiHandle->rootNodePtr = addNode(NULL, 0, NODE_FLAG_ROOT, NULL);

    /*
     * Initialize a root Node for pending (forward referenced) nodes.
     */
    smiHandle->parserPtr = &parser;
    parser.pendingNodePtr = addNode(NULL, 0, NODE_FLAG_ROOT, NULL);

    /*
     * Initialize the top level well-known nodes, ccitt, iso, joint-iso-ccitt
     * belonging to a dummy module "". This is needed for SMIv1/v2. SMIng
     * defines it in a special SMIng module.
     */
    parser.path			= NULL;
    parser.flags		= smiHandle->flags;
    parser.file			= NULL;
    parser.line			= -1;
    parser.modulePtr = addModule(smiStrdup(""), smiStrdup(""), 0, NULL);

    addView("");

    objectPtr = addObject(smiStrdup("ccitt"), smiHandle->rootNodePtr, 0, 0, &parser);
    objectPtr->export.oid = objectPtr->nodePtr->oid =
	smiMalloc(sizeof(int));
    objectPtr->export.oidlen = objectPtr->nodePtr->oidlen = 1;
    objectPtr->nodePtr->oid[0] = 0;
    objectPtr->export.nodekind = SMI_NODEKIND_NODE;
    objectPtr = addObject(smiStrdup("iso"), smiHandle->rootNodePtr, 1, 0, &parser);
    objectPtr->export.oid = objectPtr->nodePtr->oid =
	smiMalloc(sizeof(int));
    objectPtr->export.oidlen = objectPtr->nodePtr->oidlen = 1;
    objectPtr->nodePtr->oid[0] = 1;
    objectPtr->export.nodekind = SMI_NODEKIND_NODE;
    objectPtr = addObject(smiStrdup("joint-iso-ccitt"), smiHandle->rootNodePtr, 2, 0, &parser);
    objectPtr->export.oid = objectPtr->nodePtr->oid =
	smiMalloc(sizeof(int));
    objectPtr->export.oidlen = objectPtr->nodePtr->oidlen = 1;
    objectPtr->nodePtr->oid[0] = 2;
    objectPtr->export.nodekind = SMI_NODEKIND_NODE;
    
    
    smiHandle->typeOctetStringPtr =
	addType(smiStrdup("OctetString"),
		SMI_BASETYPE_OCTETSTRING, 0, &parser);
    smiHandle->typeObjectIdentifierPtr =
	addType(smiStrdup("ObjectIdentifier"),
		SMI_BASETYPE_OBJECTIDENTIFIER, 0, &parser);
    smiHandle->typeInteger32Ptr =
	addType(smiStrdup("Integer32"),
		SMI_BASETYPE_INTEGER32, 0, &parser);
    smiHandle->typeUnsigned32Ptr =
	addType(smiStrdup("Unsigned32"),
		SMI_BASETYPE_UNSIGNED32, 0, &parser);
    smiHandle->typeInteger64Ptr =
	addType(smiStrdup("Integer64"),
		SMI_BASETYPE_INTEGER64, 0, &parser);
    smiHandle->typeUnsigned64Ptr =
	addType(smiStrdup("Unsigned64"),
		SMI_BASETYPE_UNSIGNED64, 0, &parser);
    smiHandle->typeFloat32Ptr =
	addType(smiStrdup("Float32"),
		SMI_BASETYPE_FLOAT32, 0, &parser);
    smiHandle->typeFloat64Ptr =
	addType(smiStrdup("Float64"),
		SMI_BASETYPE_FLOAT64, 0, &parser);
    smiHandle->typeFloat128Ptr =
	addType(smiStrdup("Float128"),
		SMI_BASETYPE_FLOAT128, 0, &parser);
    smiHandle->typeEnumPtr =
	addType(smiStrdup("Enumeration"),
		SMI_BASETYPE_ENUM, 0, &parser);
    smiHandle->typeBitsPtr =
	addType(smiStrdup("Bits"),
		SMI_BASETYPE_BITS, 0, &parser);
	smiHandle->typePointerPtr =
	addType(smiStrdup("Pointer"),
		SMI_BASETYPE_POINTER, 0, &parser);

    return (0);
}



/*
 *----------------------------------------------------------------------
 *
 * freeNodeTree --
 *
 *      Free all node of a node (sub)tree.
 *
 * Results:
 *      0 on success or -1 on an error.
 *
 * Side effects:
 *      None.
 *
 *----------------------------------------------------------------------
 */

static void freeNodeTree(Node *rootPtr)
{
    Node       *nodePtr, *nextPtr;
    
    for (nodePtr = rootPtr->firstChildPtr; nodePtr; nodePtr = nextPtr) {
	nextPtr = nodePtr->nextPtr;
	freeNodeTree(nodePtr);
	smiFree(nodePtr->oid);
	smiFree(nodePtr);
    }
    rootPtr->firstChildPtr = NULL;
    rootPtr->lastChildPtr = NULL;
    rootPtr->firstObjectPtr = NULL;
    rootPtr->lastObjectPtr = NULL;
    rootPtr->nextPtr = NULL;
    rootPtr->prevPtr = NULL;
    rootPtr->parentPtr = NULL;
}



/*
 *----------------------------------------------------------------------
 *
 * smiFreeData --
 *
 *      Free all data structures.
 *
 * Results:
 *      0 on success or -1 on an error.
 *
 * Side effects:
 *      None.
 *
 *----------------------------------------------------------------------
 */

void smiFreeData()
{
    View       *viewPtr, *nextViewPtr;
    Macro      *macroPtr, *nextMacroPtr;
    Module     *modulePtr, *nextModulePtr;
    Import     *importPtr, *nextImportPtr;
    Identity     *identityPtr, *nextIdentityPtr;
    Revision   *revisionPtr, *nextRevisionPtr;
    List       *listPtr, *nextListPtr;
    Type       *typePtr, *nextTypePtr;
    Class	   *classPtr, *nextClassPtr;
    Attribute  *attributePtr, *nextAttributePtr;
    Event 	   *eventPtr,	*nextEventPtr;
    Object     *objectPtr, *nextObjectPtr;

    for (viewPtr = smiHandle->firstViewPtr; viewPtr; viewPtr = nextViewPtr) {
	nextViewPtr = viewPtr->nextPtr;
	smiFree(viewPtr->name);
	smiFree(viewPtr);
    }

    /*
     * In this first module loop we remove each module's imports,
     * revisions, macros, and objects.
     */
    for (modulePtr = smiHandle->firstModulePtr; modulePtr;
	 modulePtr = nextModulePtr) {
	nextModulePtr = modulePtr->nextPtr;

	for (importPtr = modulePtr->firstImportPtr; importPtr;
	     importPtr = nextImportPtr) {
	    nextImportPtr = importPtr->nextPtr;
	    smiFree(importPtr->export.module);
	    smiFree(importPtr->export.name);
	    smiFree(importPtr);
	}

	for (revisionPtr = modulePtr->firstRevisionPtr; revisionPtr;
	     revisionPtr = nextRevisionPtr) {
	    nextRevisionPtr = revisionPtr->nextPtr;
	    smiFree(revisionPtr->export.description);
	    smiFree(revisionPtr);
	}
	
	for (macroPtr = modulePtr->firstMacroPtr; macroPtr;
	     macroPtr = nextMacroPtr) {
	    nextMacroPtr = macroPtr->nextPtr;
	    smiFree(macroPtr->export.name);
	    smiFree(macroPtr->export.abnf);
	    smiFree(macroPtr->export.reference);
	    smiFree(macroPtr->export.description);
	    smiFree(macroPtr);
	    
	}
	for (identityPtr = modulePtr->firstIdentityPtr; identityPtr;
	     identityPtr = nextIdentityPtr) {
	    nextIdentityPtr = identityPtr->nextPtr;
	    smiFree(identityPtr->export.name);
	    smiFree(identityPtr->export.reference);
	    smiFree(identityPtr->export.description);
	    smiFree(identityPtr);
	}

	for (objectPtr = modulePtr->firstObjectPtr; objectPtr;
	     objectPtr = nextObjectPtr) {

	    nextObjectPtr = objectPtr->nextPtr;
	    smiFree(objectPtr->export.name);
	    smiFree(objectPtr->export.description);
	    smiFree(objectPtr->export.reference);
	    smiFree(objectPtr->export.format);
	    smiFree(objectPtr->export.units);
	    for (listPtr = objectPtr->listPtr; listPtr;
		 listPtr = nextListPtr) {
		nextListPtr = listPtr->nextPtr;
		smiFree(listPtr);
	    }
	    for (listPtr = objectPtr->optionlistPtr; listPtr;
		 listPtr = nextListPtr) {
		nextListPtr = listPtr->nextPtr;
		smiFree(((Option *)(listPtr->ptr))->export.description);
		smiFree((Option *)(listPtr->ptr));
		smiFree(listPtr);
	    }
	    for (listPtr = objectPtr->refinementlistPtr; listPtr;
		 listPtr = nextListPtr) {
		nextListPtr = listPtr->nextPtr;
		smiFree(((Refinement *)(listPtr->ptr))->export.description);
		smiFree((Refinement *)(listPtr->ptr));
		smiFree(listPtr);
	    }
	    if (objectPtr->typePtr) {
		if ((objectPtr->typePtr->export.basetype ==
		     SMI_BASETYPE_OCTETSTRING ||
		     objectPtr->typePtr->export.basetype ==
		     SMI_BASETYPE_BITS)) {
		    smiFree(objectPtr->export.value.value.ptr);
		} else if ((objectPtr->typePtr->export.basetype ==
			    SMI_BASETYPE_OBJECTIDENTIFIER) &&
			   (objectPtr->export.value.basetype ==
			    objectPtr->typePtr->export.basetype)) {
		    smiFree(objectPtr->export.value.value.oid);
		} 
		
	    }
	    smiFree(objectPtr);
	    
	    
	    
	    
	}
	

	for (classPtr = modulePtr->firstClassPtr; classPtr;
	     classPtr = nextClassPtr) {

	    nextClassPtr = classPtr->nextPtr;
	   	for (attributePtr = classPtr->firstAttributePtr; attributePtr;
	     attributePtr = nextAttributePtr) {

	    nextAttributePtr = attributePtr->nextPtr;
	    
	    for (listPtr = attributePtr->listPtr; listPtr;
		 listPtr = nextListPtr) {
		nextListPtr = listPtr->nextPtr;
		if ((attributePtr->export.basetype == SMI_BASETYPE_BITS) ||
		    (attributePtr->export.basetype == SMI_BASETYPE_ENUM)) {
		    smiFree(((NamedNumber *)(listPtr->ptr))->export.name);
		    smiFree((NamedNumber *)(listPtr->ptr));
		} else if ((attributePtr->export.basetype == SMI_BASETYPE_INTEGER32) ||
			   (attributePtr->export.basetype == SMI_BASETYPE_INTEGER64) ||
			   (attributePtr->export.basetype == SMI_BASETYPE_UNSIGNED32) ||
			   (attributePtr->export.basetype == SMI_BASETYPE_UNSIGNED64) ||
			   (attributePtr->export.basetype == SMI_BASETYPE_FLOAT32) ||
			   (attributePtr->export.basetype == SMI_BASETYPE_FLOAT64) ||
			   (attributePtr->export.basetype == SMI_BASETYPE_FLOAT128) ||
			   (attributePtr->export.basetype == SMI_BASETYPE_OCTETSTRING)) {
		    smiFree((Range *)(listPtr->ptr));
		}
		smiFree(listPtr);
	    }
	    smiFree(attributePtr->export.name);
	    smiFree(attributePtr->export.format);
	    smiFree(attributePtr->export.units);
	    smiFree(attributePtr->export.description);
	    smiFree(attributePtr->export.reference);
	    smiFree(attributePtr);
	    
	    }
	    
	    for (eventPtr = classPtr->firstEventPtr; eventPtr;
	     eventPtr = nextEventPtr) {
	     
		nextEventPtr = eventPtr->nextPtr;
		smiFree(eventPtr->export.name);
	    smiFree(eventPtr->export.reference);
	    smiFree(eventPtr->export.description);
	    }
	   
	   	
	    for (listPtr = classPtr->uniqueList; listPtr;
		 listPtr = nextListPtr) {
		nextListPtr = listPtr->nextPtr;
			smiFree(listPtr);
	    }
	    
	    smiFree(classPtr->export.name);
	    smiFree(classPtr->export.description);
	    smiFree(classPtr->export.reference);
	    smiFree(classPtr);
	
	}
    }

    /*
     * In this second module loop we remove each module's types
     * and the modules themselves. This separation is required, because
     * we reference some types of foreign modules in the first loop.
     */
    for (modulePtr = smiHandle->firstModulePtr; modulePtr;
	 modulePtr = nextModulePtr) {
	nextModulePtr = modulePtr->nextPtr;

	for (typePtr = modulePtr->firstTypePtr; typePtr;
	     typePtr = nextTypePtr) {
	    nextTypePtr = typePtr->nextPtr;
	    for (listPtr = typePtr->listPtr; listPtr;
		 listPtr = nextListPtr) {
		nextListPtr = listPtr->nextPtr;
		if ((typePtr->export.basetype == SMI_BASETYPE_BITS) ||
		    (typePtr->export.basetype == SMI_BASETYPE_ENUM)) {
		    smiFree(((NamedNumber *)(listPtr->ptr))->export.name);
		    smiFree((NamedNumber *)(listPtr->ptr));
		} else if ((typePtr->export.basetype == SMI_BASETYPE_INTEGER32) ||
			   (typePtr->export.basetype == SMI_BASETYPE_INTEGER64) ||
			   (typePtr->export.basetype == SMI_BASETYPE_UNSIGNED32) ||
			   (typePtr->export.basetype == SMI_BASETYPE_UNSIGNED64) ||
			   (typePtr->export.basetype == SMI_BASETYPE_FLOAT32) ||
			   (typePtr->export.basetype == SMI_BASETYPE_FLOAT64) ||
			   (typePtr->export.basetype == SMI_BASETYPE_FLOAT128) ||
			   (typePtr->export.basetype == SMI_BASETYPE_OCTETSTRING)) {
		    smiFree((Range *)(listPtr->ptr));
		}
		smiFree(listPtr);
	    }
	    smiFree(typePtr->export.name);
	    smiFree(typePtr->export.format);
	    smiFree(typePtr->export.units);
	    smiFree(typePtr->export.description);
	    smiFree(typePtr->export.reference);
	    smiFree(typePtr);
	}

	smiFree(modulePtr->export.name);
	smiFree(modulePtr->export.path);
	smiFree(modulePtr->export.organization);
	smiFree(modulePtr->export.contactinfo);
	smiFree(modulePtr->export.description);
	smiFree(modulePtr->export.reference);
	smiFree(modulePtr);
    }

    freeNodeTree(smiHandle->rootNodePtr);
    smiFree(smiHandle->rootNodePtr);
    
    return;
}



/*
 *----------------------------------------------------------------------
 *
 * loadModule --
 *
 *      Load a MIB module. If modulename is a plain name, the file is
 *	search along the SMIPATH environment variable. If modulename
 *	contains a `.' or DIR_SEPARATOR it is assumed to be the path.
 *
 * Results:
 *      0 on success or -1 on an error.
 *
 * Side effects:
 *      None.
 *
 *----------------------------------------------------------------------
 */

Module *loadModule(const char *modulename, Parser *parserPtr)
{
    Parser	    parser;
    Parser          *parentParserPtr;
    char	    *path = NULL, *dir, *smipath;
    int		    sming = 0;
    int             c, i;
    FILE	    *file;
    char	    sep[2];

    static const char *ext[] = {
	"", ".my", ".smiv1", ".smiv2", ".sming", ".mib", ".txt", NULL
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
		smiAsprintf(&path, "%s%c%s%s", dir, DIR_SEPARATOR,
			    modulename, ext[i]);
		if (! access(path, R_OK)) {
		    break;
		}
		smiFree(path);
	    }
	    if (ext[i]) break;
	    {
		char *newmodulename = smiStrdup(modulename);
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
	    }
	    
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
	/* Not found in the path; now try to fetch & cache the module. */
	int  pid;
	char *argv[4];
	char *cmd;
	int  status;
	smiAsprintf(&path, "%s%c%s",
		    smiHandle->cache, DIR_SEPARATOR, modulename);
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
    
    if (!path) {
	smiPrintError(parserPtr, ERR_MODULE_NOT_FOUND, modulename);
	return NULL;
    }

    parser.path			= path;

    /*
     * Look into the file to determine whether it contains
     * SMIv1/SMIv2 or SMIng definitions.
     */

    file = fopen(path, "r");
    if (! file) {
	smiPrintError(parserPtr, ERR_OPENING_INPUTFILE, path, strerror(errno));
	smiFree(path);
	return NULL;
    }
    while ((c = fgetc(file))) {
	if (c == '-' || isupper(c)) {
	    sming = 0;
	    break;
	} else if (c == '/' || c == 'm') {
	    sming = 1;
	    break;
	} else if (c == EOF || ! isspace(c)) {
	    smiPrintError(parserPtr, ERR_ILLEGAL_INPUTFILE, path);
	    smiFree(path);
	    fclose(file);
	    return NULL;
	}
    }
    rewind(file);

    
    if (sming == 0) {
#ifdef BACKEND_SMI
	parentParserPtr = smiHandle->parserPtr;
	smiHandle->parserPtr = &parser;
	parser.path			= path;
	parser.flags			= smiHandle->flags;
	parser.modulePtr		= NULL;
	parser.complianceModulePtr	= NULL;
	parser.capabilitiesModulePtr	= NULL;
	parser.currentDecl              = SMI_DECL_UNKNOWN;
	parser.firstStatementLine       = 0;
	parser.firstNestedStatementLine = 0;
	parser.firstRevisionLine        = 0;
	parser.file			= file;

	/*
	 * Initialize a root Node for pending (forward referenced) nodes.
	 */
	parser.pendingNodePtr = addNode(NULL, 0, NODE_FLAG_ROOT, NULL);
    
	if (smiEnterLexRecursion(parser.file) < 0) {
	    smiPrintError(&parser, ERR_MAX_LEX_DEPTH);
	    fclose(parser.file);
	}
	smiDepth++;
	parser.line			= 1;
	smiparse((void *)&parser);
	freeNodeTree(parser.pendingNodePtr);
	smiFree(parser.pendingNodePtr);
	smiLeaveLexRecursion();
	smiDepth--;
	fclose(parser.file);
	smiFree(path);
	smiHandle->parserPtr = parentParserPtr;
	return parser.modulePtr;
#else
	smiPrintError(parserPtr, ERR_SMI_NOT_SUPPORTED, path);
	smiFree(path);
        fclose(file);
	return NULL;
#endif
    }
    
    if (sming == 1) {
#ifdef BACKEND_SMING
	parentParserPtr = smiHandle->parserPtr;
	smiHandle->parserPtr = &parser;
	parser.path			= path;
	parser.flags			= smiHandle->flags;
	parser.modulePtr		= NULL;
	parser.complianceModulePtr	= NULL;
	parser.capabilitiesModulePtr	= NULL;
	parser.currentDecl              = SMI_DECL_UNKNOWN;
	parser.firstStatementLine       = 0;
	parser.firstNestedStatementLine = 0;
	parser.firstRevisionLine        = 0;
	parser.file			= file;

	/*
	 * Initialize a root Node for pending (forward referenced) nodes.
	 */
	parser.pendingNodePtr = addNode(NULL, 0, NODE_FLAG_ROOT, NULL);
    
	if (smingEnterLexRecursion(parser.file) < 0) {
	    smiPrintError(&parser, ERR_MAX_LEX_DEPTH);
	    fclose(parser.file);
	}
	smiDepth++;
	parser.line			= 1;
	smingparse((void *)&parser);
	freeNodeTree(parser.pendingNodePtr);
	smiFree(parser.pendingNodePtr);
	smingLeaveLexRecursion();
	smiDepth--;
	fclose(parser.file);
	smiFree(path);
	smiHandle->parserPtr = parentParserPtr;
	return parser.modulePtr;
#else
	smiPrintError(parserPtr, ERR_SMING_NOT_SUPPORTED, path);
	smiFree(path);
        fclose(file);
	return NULL;
#endif
    }

    smiFree(path);
    fclose(file);
    return NULL;
}
