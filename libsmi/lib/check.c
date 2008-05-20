/*
 * check.c --
 *
 *      This module contains semantics checks that are shared between
 *	the SMI parser backends.
 *
 * Copyright (c) 2000 Frank Strauss, Technical University of Braunschweig.
 *
 * See the file "COPYING" for information on usage and redistribution
 * of this file, and for a DISCLAIMER OF ALL WARRANTIES.
 *
 * @(#) $Id: check.c 7640 2008-01-31 15:29:52Z schoenw $
 */

#include <config.h>

#include <string.h>
#include <errno.h>
#include <ctype.h>
#include <sys/types.h>
#ifdef HAVE_UNISTD_H
#include <unistd.h>
#endif
#ifdef HAVE_WIN_H
#include "win.h"
#endif

#include "error.h"
#include "util.h"
#include "data.h"
#include "check.h"
#include "smi.h"

#ifdef HAVE_DMALLOC_H
#include <dmalloc.h>
#endif



/*
 *----------------------------------------------------------------------
 *
 * compareValues --
 *
 *	Compare two SmiValues a and b.
 *
 * Results:
 *      <= -2   if a is less than b-1
 *      -1      if a is b-1
 *      0       if equal
 *      1       if a is b+1
 *      >= 2    if a is greater than b+1
 *
 * Side effects:
 *	None.
 *
 *----------------------------------------------------------------------
 */

static int
compareValues(SmiValue *a, SmiValue *b) {
    if ((a->basetype == SMI_BASETYPE_UNSIGNED32) &&
	(b->basetype == SMI_BASETYPE_UNSIGNED32)) {
	if (a->value.unsigned32 == b->value.unsigned32) {
	    return 0;
	} else {
	    if (a->value.unsigned32 > b->value.unsigned32) {
		if (a->value.unsigned32 == b->value.unsigned32 + 1) {
		    return 1;
		} else {
		    return 2;
		}
	    } else if (a->value.unsigned32 < b->value.unsigned32) {
		if (a->value.unsigned32 + 1 == b->value.unsigned32) {
		    return -1;
		} else {
		    return -2;
		}
	    }
	}
    }
    if ((a->basetype == SMI_BASETYPE_INTEGER32) &&
	(b->basetype == SMI_BASETYPE_INTEGER32)) {
	if (a->value.integer32 == b->value.integer32) {
	    return 0;
	} else {
	    if (a->value.integer32 > b->value.integer32) {
		if (a->value.integer32 == b->value.integer32 + 1) {
		    return 1;
		} else {
		    return 2;
		}
	    } else if (a->value.integer32 < b->value.integer32) {
		if (a->value.integer32 + 1 == b->value.integer32) {
		    return -1;
		} else {
		    return -2;
		}
	    }
	}
    }
    if ((a->basetype == SMI_BASETYPE_UNSIGNED32) &&
	(b->basetype == SMI_BASETYPE_INTEGER32)) {
	if ((b->value.integer32 < -1) ||
	    ((a->value.unsigned32 > 1) &&
	     (a->value.unsigned32-1 > 2147483647))) {
	    return 2;
	}
	return a->value.unsigned32 - b->value.integer32;
    }
    if ((a->basetype == SMI_BASETYPE_INTEGER32) &&
	(b->basetype == SMI_BASETYPE_UNSIGNED32)) {
	if ((a->value.integer32 < -1) ||
	    ((b->value.unsigned32 > 1) &&
	     (b->value.unsigned32-1 > 2147483647))) {
	    return -2;
	}
	return b->value.unsigned32 - a->value.integer32;
    }
    return 0;
}



/*
 *----------------------------------------------------------------------
 *
 * redefinition --
 *
 *	Print out error messages about a (case) redefinition.
 *
 * Results:
 *	None.
 *
 * Side effects:
 *	None.
 *
 *----------------------------------------------------------------------
 */

static void
redefinition(Parser *parser, int line1, char *name1, Module *module,
	     int line2, char *name2)
{
    char *tmp = parser->path;
    int equal = (strcmp(name1, name2) == 0);

    if (!strcmp(name1, "IpAddress") ||
	!strcmp(name1, "TimeTicks") ||
	!strcmp(name1, "Opaque") ||
	!strcmp(name1, "Integer32") ||
	!strcmp(name1, "Unsigned32") ||
	!strcmp(name1, "Counter32") ||
	!strcmp(name1, "Gauge32") ||
	!strcmp(name1, "Counter64") ||
	!strcmp(name1, "Integer64") ||
	!strcmp(name1, "Unsigned64")) {
	if (line1) {
	    smiPrintErrorAtLine(parser, ERR_BASETYPE_REDEFINITION, line1, name1);
	} else {
	    smiPrintError(parser, ERR_BASETYPE_REDEFINITION, name1);
	}
    } else {
	if (! module) {
	    if (equal) {
		if (line1) {
		    smiPrintErrorAtLine(parser, ERR_REDEFINITION, line1, name1);
		} else {
		    smiPrintError(parser, ERR_REDEFINITION, name1);
		}
	    } else {
		if (line1) {
		    smiPrintErrorAtLine(parser, ERR_CASE_REDEFINITION,
					line1, name1, name2);
		} else {
		    smiPrintError(parser, ERR_CASE_REDEFINITION, name1, name2);
		}
	    }
	} else {
	    if (equal) {
		if (line1) {
		    smiPrintErrorAtLine(parser, ERR_EXT_REDEFINITION, line1,
					module->export.name, name1);
		} else {
		    smiPrintError(parser, ERR_EXT_REDEFINITION,
				  module->export.name, name1);
		}
	    } else {
		if (line1) {
		    smiPrintErrorAtLine(parser, ERR_EXT_CASE_REDEFINITION, line1,
					name1, module->export.name, name2);
		} else {
		    smiPrintError(parser, ERR_EXT_CASE_REDEFINITION,
				  name1, module->export.name, name2);
		}
	    }
	    parser->path = module->export.path;
	}
	smiPrintErrorAtLine(parser, ERR_PREVIOUS_DEFINITION, line2, name2);
	if (module) {
	    parser->path = tmp;
	}
    }
}



/*
 *----------------------------------------------------------------------
 *
 * smiCheckObjectName --
 *
 *      Check whether a given object name already exists
 *	in a given module.
 *
 * Results:
 *      None.
 *
 * Side effects:
 *      None.
 *
 *----------------------------------------------------------------------
 */

void
smiCheckObjectName(Parser *parser, Module *module, char *name)
{
    Object	*objectPtr;
    Type        *typePtr;
    Module	*modPtr;

    int errRedef = smiGetErrorSeverity(ERR_REDEFINITION);
    int errExtRedef = smiGetErrorSeverity(ERR_EXT_REDEFINITION);
    int errCaseRedef = smiGetErrorSeverity(ERR_CASE_REDEFINITION);
    int errExtCaseRedef = smiGetErrorSeverity(ERR_EXT_CASE_REDEFINITION);

    if (! (parser->flags & SMI_FLAG_ERRORS)
	|| (errRedef > smiHandle->errorLevel
	    && errExtRedef > smiHandle->errorLevel
	    && errCaseRedef > smiHandle->errorLevel
	    && errExtCaseRedef > smiHandle->errorLevel)) {
	return;
    }

    /*
     * This would really benefit from having a hash table...
     */

    for (modPtr = smiHandle->firstModulePtr;
	 modPtr; modPtr = modPtr->nextPtr) {

	/*
	 * Skip all external modules if we are not interested in
	 * generating warning on extern redefinitions.
	 */

	if (errExtRedef > smiHandle->errorLevel
	    && errExtCaseRedef > smiHandle->errorLevel
	    && modPtr != module) {
	    continue;
	}

        for (objectPtr = modPtr->firstObjectPtr;
	     objectPtr; objectPtr = objectPtr->nextPtr) {
	    if (! (objectPtr->flags & FLAG_INCOMPLETE)
		&& ! strcasecmp(name, objectPtr->export.name)) {
		redefinition(parser, 0, name,
			     modPtr == module ? NULL : objectPtr->modulePtr,
			     objectPtr->line, objectPtr->export.name);
	    }
	}
	for (typePtr = modPtr->firstTypePtr;
	     typePtr; typePtr = typePtr->nextPtr) {
	    /* TODO: must ignore SEQUENCE types here ... */
	    if (! (typePtr->flags & FLAG_INCOMPLETE)
		&& typePtr->export.name
		&& !strcasecmp(name, typePtr->export.name)) {
		redefinition(parser, 0, name,
			     modPtr == module ? NULL : typePtr->modulePtr,
			     typePtr->line, typePtr->export.name);
	    }
	}
    }
}



/*
 *----------------------------------------------------------------------
 *
 * smiCheckTypeName --
 *
 *      Check whether a given type name already exists
 *	in a given module.
 *
 * Results:
 *      None.
 *
 * Side effects:
 *      None.
 *
 *----------------------------------------------------------------------
 */

void
smiCheckTypeName(Parser *parser, Module *module, char *name, int line)
{
    Object	*objectPtr;
    Type        *typePtr;
    Module	*modPtr;

    int errRedef = smiGetErrorSeverity(ERR_REDEFINITION);
    int errExtRedef = smiGetErrorSeverity(ERR_EXT_REDEFINITION);
    int errCaseRedef = smiGetErrorSeverity(ERR_CASE_REDEFINITION);
    int errExtCaseRedef = smiGetErrorSeverity(ERR_EXT_CASE_REDEFINITION);

    if (! (parser->flags & SMI_FLAG_ERRORS)
	|| (errRedef > smiHandle->errorLevel
	    && errExtRedef > smiHandle->errorLevel
	    && errCaseRedef > smiHandle->errorLevel
	    && errExtCaseRedef > smiHandle->errorLevel)) {
	return;
    }

    /*
     * This would really benefit from having a hash table...
     */

    for (modPtr = smiHandle->firstModulePtr;
	 modPtr; modPtr = modPtr->nextPtr) {

	/*
	 * Skip all external modules if we are not interested in
	 * generating warning on extern redefinitions.
	 */

	if (errExtRedef > smiHandle->errorLevel
	    && errExtCaseRedef > smiHandle->errorLevel
	    && modPtr != module) {
	    continue;
	}

	for (typePtr = modPtr->firstTypePtr;
	     typePtr; typePtr = typePtr->nextPtr) {
	    /* TODO: must ignore SEQUENCE types here ... */
	    if (! (typePtr->flags & FLAG_INCOMPLETE)
		&& typePtr->export.name
		&& !strcasecmp(name, typePtr->export.name)) {
		redefinition(parser, line, name,
			     modPtr == module ? NULL : typePtr->modulePtr,
			     typePtr->line, typePtr->export.name);
	    }
	}

        for (objectPtr = modPtr->firstObjectPtr;
	     objectPtr; objectPtr = objectPtr->nextPtr) {
	    if (! (objectPtr->flags & FLAG_INCOMPLETE)
		&& ! strcasecmp(name, objectPtr->export.name)) {
		redefinition(parser, line, name,
			     modPtr == module ? NULL : objectPtr->modulePtr,
			     objectPtr->line, objectPtr->export.name);
	    }
	}
    }
}



/*
 *----------------------------------------------------------------------
 *
 * smiCheckFormat --
 *
 *      Check whether a format specification is valid.
 *
 * Results:
 *      Returns 1 of the format is acceptable and 0 otherwise.
 *
 * Side effects:
 *      None.
 *
 *----------------------------------------------------------------------
 */

int
smiCheckFormat(Parser *parser, SmiBasetype basetype, char *format, int line)
{
    int n, repeat, error = 1;
    char *p = format;

    switch (basetype) {
    case SMI_BASETYPE_INTEGER32:
    case SMI_BASETYPE_INTEGER64:
    case SMI_BASETYPE_UNSIGNED32:
    case SMI_BASETYPE_UNSIGNED64:
	if (*p == 'x' || *p == 'o' || *p == 'b') {
	    p++;
	    error = (*p != 0);
	} else if (*p == 'd') {
	    p++;
	    if (! *p) {
		error = 0;
		break;
	    }
	    if (*p != '-') {
		error = 0;
		break;
	    }
	    for (n = 0, p++; *p && isdigit((int) *p); p++, n++) ;
	    error = (*p != 0 || n <= 0);
	}
	break;
    case SMI_BASETYPE_OCTETSTRING:
	while (*p) {
	    if ((repeat = (*p == '*'))) p++;                /* part 1 */
	    
	    for (n = 0; *p && isdigit((int) *p); p++, n++) ;/* part 2 */
	    if (! *p || n == 0) {
		break;
	    }
	    
	    if (*p != 'x' && *p != 'd' && *p != 'o'         /* part 3 */
		&& *p != 'a' && *p != 't') {
		break;
	    }
	    p++;
	    
	    if (*p                                          /* part 4 */
		&& ! isdigit((int) *p) && *p != '*') p++;
	    
	    if (repeat && *p                                /* part 5 */
		&& ! isdigit((int) *p) && *p != '*') p++;
	}
	error = *p;
	break;
    default:
	break;
    }

    if (error) {
	if (line) {
	    smiPrintErrorAtLine(parser, ERR_INVALID_FORMAT, line, format);
	} else {
	    smiPrintError(parser, ERR_INVALID_FORMAT, format);
	}
    }

    return ! error;
}



/*
 *----------------------------------------------------------------------
 *
 * smiCheckNamedNumberRedefinition --
 *
 *      Check whether named numbers redefine names or numbers.
 *
 * Results:
 *      None.
 *
 * Side effects:
 *      None.
 *
 *----------------------------------------------------------------------
 */

void
smiCheckNamedNumberRedefinition(Parser *parser, Type *type)
{
    List *list1Ptr, *list2Ptr;
    NamedNumber *nn1Ptr, *nn2Ptr;

    if (! type || (type->export.basetype != SMI_BASETYPE_ENUM
		      && type->export.basetype != SMI_BASETYPE_BITS)) {
	return;
    }
	    
    for (list1Ptr = type->listPtr;
	 list1Ptr; list1Ptr = list1Ptr->nextPtr) {
	
	nn1Ptr = (NamedNumber *)(list1Ptr->ptr);

	for (list2Ptr = list1Ptr->nextPtr;
	     list2Ptr; list2Ptr = list2Ptr->nextPtr) {
	    
	    nn2Ptr = (NamedNumber *)(list2Ptr->ptr);

	    if (type->export.basetype == SMI_BASETYPE_ENUM) {
		if (!strcmp(nn1Ptr->export.name, nn2Ptr->export.name)) {
		    smiPrintErrorAtLine(parser, ERR_ENUM_NAME_REDEFINITION,
					type->line,
					nn1Ptr->export.name);
		}
		if (nn1Ptr->export.value.value.integer32
		    == nn2Ptr->export.value.value.integer32) {
		    smiPrintErrorAtLine(parser, ERR_ENUM_NUMBER_REDEFINITION,
					type->line,
					nn1Ptr->export.value.value.integer32);
		}
	    }
	    if (type->export.basetype == SMI_BASETYPE_BITS) {
		if (!strcmp(nn1Ptr->export.name, nn2Ptr->export.name)) {
		    smiPrintErrorAtLine(parser, ERR_BITS_NAME_REDEFINITION,
					type->line,
					nn1Ptr->export.name);
		}
		if (nn1Ptr->export.value.value.unsigned32
		    == nn2Ptr->export.value.value.unsigned32) {
		    smiPrintErrorAtLine(parser, ERR_BITS_NUMBER_REDEFINITION,
					type->line,
					nn1Ptr->export.value.value.unsigned32);
		}
	    }
	}
    }   
}



/*
 *----------------------------------------------------------------------
 *
 * smiCheckNamedNumberSubtyping --
 *
 *      Check whether named numbers in a derived type are compatible
 *	with the named numbers in the parent type.
 *
 * Results:
 *      None.
 *
 * Side effects:
 *      None.
 *
 *----------------------------------------------------------------------
 */

void
smiCheckNamedNumberSubtyping(Parser *parser, Type *type)
{
    List *list1Ptr, *list2Ptr;
    NamedNumber *nn1Ptr, *nn2Ptr;
	
    if (! type || ! type->parentPtr || ! type->parentPtr->parentPtr
	|| (type->export.basetype != SMI_BASETYPE_ENUM
	    && type->export.basetype != SMI_BASETYPE_BITS)) {
	return;
    }

    for (list1Ptr = type->listPtr;
	 list1Ptr; list1Ptr = list1Ptr->nextPtr) {
	
	nn1Ptr = (NamedNumber *)(list1Ptr->ptr);
	
	for (list2Ptr = type->parentPtr->listPtr;
	     list2Ptr; list2Ptr = list2Ptr->nextPtr) {
	    
	    nn2Ptr = (NamedNumber *)(list2Ptr->ptr);
	    
	    if (type->export.basetype == SMI_BASETYPE_ENUM) {
		if (! strcmp(nn1Ptr->export.name, nn2Ptr->export.name)
		    && nn1Ptr->export.value.value.integer32
		    == nn2Ptr->export.value.value.integer32) {
		    break;
		}
	    }
	    
	    if (type->export.basetype == SMI_BASETYPE_BITS) {
		if (! strcmp(nn1Ptr->export.name, nn2Ptr->export.name)
		    && nn1Ptr->export.value.value.unsigned32
		    == nn2Ptr->export.value.value.unsigned32) {
		    break;
		}
	    }
	}
	    
	if (! list2Ptr) {
	    if (type->export.basetype == SMI_BASETYPE_ENUM) {
		if (type->parentPtr->export.name) {
		    smiPrintErrorAtLine(parser, ERR_ENUM_SUBTYPE_OF,
					type->line,
					nn1Ptr->export.name,
					nn1Ptr->export.value.value.integer32,
					type->parentPtr->export.name);
		} else {
		    smiPrintErrorAtLine(parser, ERR_ENUM_SUBTYPE,
					type->line,
					nn1Ptr->export.name,
					nn1Ptr->export.value.value.integer32);
		}
	    }
	    if (type->export.basetype == SMI_BASETYPE_BITS) {
		if (type->parentPtr->export.name) {
		    smiPrintErrorAtLine(parser, ERR_BITS_SUBTYPE_OF,
					type->line,
					nn1Ptr->export.name,
					type->parentPtr->export.name);
		} else {
		    smiPrintErrorAtLine(parser, ERR_BITS_SUBTYPE,
					type->line,
					nn1Ptr->export.name);
		}
	    }
	}
    }
}



/*
 *----------------------------------------------------------------------
 *
 * smiCheckNamedNumbersOrder --
 *
 *      Check and normalize the order of named numbers in a bits
 *	or enumeration type.
 *
 * Results:
 *      None.
 *
 * Side effects:
 *      None.
 *
 *----------------------------------------------------------------------
 */

void
smiCheckNamedNumbersOrder(Parser *parser, Type *type)
{
    List *listPtr, *lastPtr, *nextPtr, *ptr;
    NamedNumber *nnPtr;
    int shutup = 0;
    
    if (! type || ! type->parentPtr
	|| (type->export.basetype != SMI_BASETYPE_ENUM
	    && type->export.basetype != SMI_BASETYPE_BITS)) {
	return;
    }

    /* Check whether the first bit has been given a name. */

    if (type->export.basetype == SMI_BASETYPE_BITS) {
	for (listPtr = type->listPtr; listPtr; listPtr = listPtr->nextPtr) {
	    nnPtr = (NamedNumber *)(listPtr->ptr);
	    if (nnPtr->export.value.value.unsigned32 == 0) break;
	}
	if (! listPtr) {
	    smiPrintErrorAtLine(parser, ERR_BITS_ZERO_NOT_NAMED, type->line);
	}
    }

    lastPtr = NULL;
    for (listPtr = type->listPtr; listPtr; listPtr = nextPtr) {

	nextPtr = listPtr->nextPtr;

	nnPtr = (NamedNumber *)(listPtr->ptr);

	if (lastPtr) {
	    
	    if ((type->export.basetype == SMI_BASETYPE_ENUM) &&
		(((NamedNumber *)(listPtr->ptr))->export.value.value.integer32 <=
		 ((NamedNumber *)(lastPtr->ptr))->export.value.value.integer32)) {
		if (!shutup) {
		    smiPrintErrorAtLine(parser, ERR_NAMED_NUMBERS_NOT_ASCENDING,
					type->line,
					type->export.name);
		    shutup = 1;
		}
		/* remove listPtr from the list */
		lastPtr->nextPtr = listPtr->nextPtr;
		/* re-insert listPtr at the right position */
		if (((NamedNumber *)(type->listPtr->ptr))->export.value.value.integer32 > ((NamedNumber *)(listPtr->ptr))->export.value.value.integer32) {
		    listPtr->nextPtr = type->listPtr;
		    type->listPtr = listPtr;
		} else {
		    for (ptr = type->listPtr; ptr; ptr = ptr->nextPtr) {
			if ((!ptr->nextPtr) ||
			    (((NamedNumber *)(ptr->nextPtr->ptr))->export.value.value.integer32 >= ((NamedNumber *)(listPtr->ptr))->export.value.value.integer32)) {
			    listPtr->nextPtr = ptr->nextPtr;
			    ptr->nextPtr = listPtr;
			    break;
			}
		    }
		}
		/* set lastPtr to the last processed item */
		for (lastPtr = listPtr; lastPtr->nextPtr != nextPtr;
		     lastPtr = lastPtr->nextPtr);
		continue;
	    }
	    if ((type->export.basetype == SMI_BASETYPE_BITS) &&
		(((NamedNumber *)(listPtr->ptr))->export.value.value.unsigned32 <=
		 ((NamedNumber *)(lastPtr->ptr))->export.value.value.unsigned32)) {
		if (!shutup) {
		    smiPrintErrorAtLine(parser, ERR_NAMED_NUMBERS_NOT_ASCENDING,
					type->line,
					type->export.name);
		    shutup = 1;
		}
		/* remove listPtr from the list */
		lastPtr->nextPtr = listPtr->nextPtr;
		/* re-insert listPtr at the right position */
		if (((NamedNumber *)(type->listPtr->ptr))->export.value.value.unsigned32 > ((NamedNumber *)(listPtr->ptr))->export.value.value.unsigned32) {
		    listPtr->nextPtr = type->listPtr;
		    type->listPtr = listPtr;
		} else {
		    for (ptr = type->listPtr; ptr; ptr = ptr->nextPtr) {
			if ((!ptr->nextPtr) ||
			    (((NamedNumber *)(ptr->nextPtr->ptr))->export.value.value.unsigned32 > ((NamedNumber *)(listPtr->ptr))->export.value.value.unsigned32)) {
			    listPtr->nextPtr = ptr->nextPtr;
			    ptr->nextPtr = listPtr;
			    break;
			}
		    }
		}
		/* set lastPtr to the last processed item */
		for (lastPtr = listPtr; lastPtr->nextPtr != nextPtr;
		     lastPtr = lastPtr->nextPtr);
		continue;
	    }
	}
	lastPtr = listPtr;
    }
}



/*
 *----------------------------------------------------------------------
 *
 * smiCheckIndex --
 *
 *      Check whether an index conforms to the SMI restrictions.
 *
 * Results:
 *      None.
 *
 * Side effects:
 *      None.
 *
 *----------------------------------------------------------------------
 */

void
smiCheckIndex(Parser *parser, Object *object)
{
    List *listPtr, *list2Ptr;
    Object *indexPtr;
    Type *typePtr, *rTypePtr;
    Range *rangePtr;
    NamedNumber *nnPtr;
    Node *nodePtr;
    int minSize, maxSize, len = 0;
    int aux = 0, cols = 0, acc = 0;

    for (nodePtr = object->nodePtr->firstChildPtr, cols = 0;
	 nodePtr; nodePtr = nodePtr->nextPtr) {
	cols++;
    }

    for (listPtr = object->listPtr; listPtr; listPtr = listPtr->nextPtr) {
	
	indexPtr = (Object *) listPtr->ptr;
	typePtr = indexPtr->typePtr;

	/* checkObjects() already handles unknown objects */
	if (indexPtr->export.nodekind != SMI_NODEKIND_COLUMN &&
	    ((indexPtr->flags & FLAG_INCOMPLETE) == 0)) {
	    smiPrintErrorAtLine(parser, ERR_INDEX_NOT_COLUMN,
				indexPtr->line,
				indexPtr->export.name,
				object->export.name);
	}

	if (!typePtr)
	    continue;
	
	switch (typePtr->export.basetype) {
	case SMI_BASETYPE_INTEGER32:
	    for (rTypePtr = typePtr; rTypePtr && ! rTypePtr->listPtr;
 		 rTypePtr = rTypePtr->parentPtr) {
	    }
	    if (! rTypePtr) {
		if (object->modulePtr != indexPtr->modulePtr) {
		    smiPrintErrorAtLine(parser, ERR_INDEX_NO_RANGE_MOD,
					object->line,
					indexPtr->modulePtr->export.name,
					indexPtr->export.name,
					object->export.name);

		} else {
		    smiPrintErrorAtLine(parser, ERR_INDEX_NO_RANGE,
					indexPtr->line,
					indexPtr->export.name,
					object->export.name);
		}
	    } else {
		for (list2Ptr = rTypePtr->listPtr;
		     list2Ptr; list2Ptr = list2Ptr->nextPtr) {
		    rangePtr = (Range *) list2Ptr->ptr;
		    if (rangePtr->export.maxValue.value.integer32 < 0) {
			smiPrintErrorAtLine(parser, ERR_INDEX_RANGE_NEGATIVE,
					    indexPtr->line,
					    indexPtr->export.name,
					    object->export.name);
			break;
		    }
		}
	    }
	    len++;
	    break;
	case SMI_BASETYPE_OCTETSTRING:
	    /* TODO: We need to check ranges of parent types as well
	       if this type does not have a range restriction. */
	    for (rTypePtr = typePtr; rTypePtr && ! rTypePtr->listPtr;
		 rTypePtr = rTypePtr->parentPtr) {
	    }
	    minSize = 65535;
	    maxSize = -1;
	    if (! rTypePtr) {
		if (object->modulePtr != indexPtr->modulePtr) {
		    smiPrintErrorAtLine(parser, ERR_INDEX_STRING_NO_SIZE_MOD,
					object->line,
					indexPtr->modulePtr->export.name,
					indexPtr->export.name,
					object->export.name);
		} else {
		    smiPrintErrorAtLine(parser, ERR_INDEX_STRING_NO_SIZE,
					indexPtr->line,
					indexPtr->export.name,
					object->export.name);
		}
		minSize = 0;
		maxSize = 65535;
	    } else {
	        for (list2Ptr = rTypePtr->listPtr;
		     list2Ptr; list2Ptr = list2Ptr->nextPtr) {
		    rangePtr = (Range *) list2Ptr->ptr;
		    if (rangePtr->export.minValue.value.integer32 < minSize) {
			minSize = rangePtr->export.minValue.value.integer32;
		    }
		    if (rangePtr->export.maxValue.value.integer32 > maxSize) {
			maxSize = rangePtr->export.maxValue.value.integer32;
		    }
		}
		if (minSize == 65535) {
		    minSize = 0;
		}
		if (maxSize < 0) {
		    maxSize = 65535;
		}
	    }
	    len += maxSize;
	    if (minSize != maxSize) {
		if (! (object->export.implied && (! listPtr->nextPtr))) {
		    len++;
		}
	    }
	    break;
	case SMI_BASETYPE_OBJECTIDENTIFIER:
	    if (object->modulePtr != indexPtr->modulePtr) {
		smiPrintErrorAtLine(parser, ERR_INDEX_OID_NO_SIZE_MOD,
				    object->line,
				    indexPtr->modulePtr->export.name,
				    indexPtr->export.name,
				    object->export.name);
	    } else {
		smiPrintErrorAtLine(parser, ERR_INDEX_OID_NO_SIZE,
				    indexPtr->line,
				    indexPtr->export.name,
				    object->export.name);
	    }
	    len += 128;
	    if (!indexPtr->export.implied) {
		len++;
	    }
	    break;
	case SMI_BASETYPE_UNSIGNED32:
	    len++;
	    break;
	case SMI_BASETYPE_INTEGER64:
	case SMI_BASETYPE_UNSIGNED64:
	case SMI_BASETYPE_FLOAT32:
	case SMI_BASETYPE_FLOAT64:
	case SMI_BASETYPE_FLOAT128:
	case SMI_BASETYPE_UNKNOWN:
	    smiPrintErrorAtLine(parser, ERR_INDEX_BASETYPE, object->line,
				typePtr->export.name ? typePtr->export.name
				                     : "[unknown]",
				indexPtr->export.name, object->export.name);
	    break;
	case SMI_BASETYPE_BITS:
	    /* TODO: BITS are somehow treated as octet strings - but
	       what is the max len? */
	    break;
	case SMI_BASETYPE_ENUM:
	    for (list2Ptr = typePtr->listPtr;
		 list2Ptr; list2Ptr = list2Ptr->nextPtr) {
		
		nnPtr = (NamedNumber *)(list2Ptr->ptr);

		if (nnPtr->export.value.value.integer32 < 0) {
			smiPrintErrorAtLine(parser, ERR_INDEX_ENUM_NEGATIVE,
					    indexPtr->line,
					    indexPtr->export.name,
					    object->export.name);
			break;
		}
	    }
	    len++;
	    break;
	}

	if (indexPtr->export.value.basetype != SMI_BASETYPE_UNKNOWN) {
	    smiPrintErrorAtLine(parser, ERR_INDEX_DEFVAL,
				indexPtr->line,
				indexPtr->export.name,
				object->export.name);
	}

    	for (nodePtr = object->nodePtr->firstChildPtr;
	     nodePtr; nodePtr = nodePtr->nextPtr) {
	    if (indexPtr == nodePtr->lastObjectPtr) {
		aux++;
		break;
	    }
	}
    }

    if (object->export.oidlen + 1 + len > 128) {
	smiPrintErrorAtLine(parser, ERR_INDEX_TOO_LARGE, object->line,
			    object->export.name,
			    (object->export.oidlen + 1 + len) - 128);
    }

    /* RFC 2578 section 7.7: Auxiliary objects must be not-accessible except
       in some interesting corner cases. */

    for (listPtr = object->listPtr; listPtr; listPtr = listPtr->nextPtr) {
	
	indexPtr = (Object *) listPtr->ptr;
	typePtr = indexPtr->typePtr;

	if (aux < cols) {
	    if ((parser->modulePtr->export.language == SMI_LANGUAGE_SMIV2)
		&& (indexPtr->nodePtr->parentPtr == object->nodePtr)) {
		if (indexPtr->export.access != SMI_ACCESS_NOT_ACCESSIBLE) {
		    smiPrintErrorAtLine(parser, ERR_INDEX_ACCESSIBLE,
					object->line,
					indexPtr->export.name, object->export.name);
		}
	    }
	}
	
	for (nodePtr = object->nodePtr->firstChildPtr, acc = 0;
	     nodePtr; nodePtr = nodePtr->nextPtr) {
	    if (indexPtr == nodePtr->lastObjectPtr
		&& indexPtr->export.access != SMI_ACCESS_NOT_ACCESSIBLE) {
		acc++;
	    }
	}
    }

    if ((parser->modulePtr->export.language == SMI_LANGUAGE_SMIV2)
	&& aux == cols && acc != 1) {
	smiPrintErrorAtLine(parser, ERR_INDEX_NON_ACCESSIBLE,
			    object->line, object->export.name);
    }
}



/*
 *----------------------------------------------------------------------
 *
 * smiCheckAugment --
 *
 *      Check whether a table augmentation conforms to the SMI
 *	restrictions.
 *
 * Results:
 *      None.
 *
 * Side effects:
 *      None.
 *
 *----------------------------------------------------------------------
 */

void
smiCheckAugment(Parser *parser, Object *object)
{
    if (! object->relatedPtr) {
	return;
    }

    if (object->relatedPtr->export.nodekind != SMI_NODEKIND_ROW) {
	smiPrintErrorAtLine(parser, ERR_AUGMENT_NO_ROW, object->line,
			    object->export.name,
			    object->relatedPtr->export.name);
	return;
    }
    
    if (object->relatedPtr->export.indexkind == SMI_INDEX_INDEX)
        return;
    
    if (object->export.indexkind == SMI_INDEX_AUGMENT)
    {
	smiPrintErrorAtLine(parser, ERR_AUGMENT_NESTED, object->line,
			    object->export.name,
			    object->relatedPtr->export.name);
	return;
    }
    
    if (object->relatedPtr->export.indexkind != SMI_INDEX_SPARSE) {
	smiPrintErrorAtLine(parser, ERR_EXTENDS_WRONG_ROW_TYPE, object->line,
			    object->export.name,
			    object->relatedPtr->export.name);
	return;
    }
    
    /*
     * TODO: Check the size of the instance identifier and the OID
     * for this entry node.
     */
}



/*
 *----------------------------------------------------------------------
 *
 * smiCheckTypeRanges --
 *
 *      Check whether all ranges of a given type are valid.
 *
 * Results:
 *      None.
 *
 * Side effects:
 *      None.
 *
 *----------------------------------------------------------------------
 */

void
smiCheckTypeRanges(Parser *parser, Type *type)
{
    List *p, *nextPtr, *pp, *nextPP;
    
    for (p = type->listPtr; p; p = nextPtr) {

	nextPtr = p->nextPtr;
	
	((Range *)p->ptr)->typePtr = type;

	if (type->export.basetype == SMI_BASETYPE_INTEGER32) {
	    if ((((Range *)p->ptr)->export.minValue.basetype ==
		 SMI_BASETYPE_UNSIGNED32) &&
		(((Range *)p->ptr)->export.minValue.value.unsigned32 >
		    2147483647)) {
		smiPrintError(parser, ERR_RANGE_OUT_OF_BASETYPE);
	    }
	    if ((((Range *)p->ptr)->export.maxValue.basetype ==
		 SMI_BASETYPE_UNSIGNED32) &&
		(((Range *)p->ptr)->export.maxValue.value.unsigned32 >
		    2147483647)) {
		smiPrintError(parser, ERR_RANGE_OUT_OF_BASETYPE);
	    }
	    ((Range *)p->ptr)->export.minValue.basetype =
		SMI_BASETYPE_INTEGER32;
	    ((Range *)p->ptr)->export.maxValue.basetype =
		SMI_BASETYPE_INTEGER32;
	}

	if (type->export.basetype == SMI_BASETYPE_UNSIGNED32) {
	    if ((((Range *)p->ptr)->export.minValue.basetype ==
		 SMI_BASETYPE_INTEGER32) &&
		(((Range *)p->ptr)->export.minValue.value.integer32 < 0)) {
		smiPrintError(parser, ERR_RANGE_OUT_OF_BASETYPE);
	    }
	    if ((((Range *)p->ptr)->export.maxValue.basetype ==
		 SMI_BASETYPE_INTEGER32) &&
		(((Range *)p->ptr)->export.maxValue.value.integer32 < 0)) {
		smiPrintError(parser, ERR_RANGE_OUT_OF_BASETYPE);
	    }
	    ((Range *)p->ptr)->export.minValue.basetype =
		SMI_BASETYPE_UNSIGNED32;
	    ((Range *)p->ptr)->export.maxValue.basetype =
		SMI_BASETYPE_UNSIGNED32;
	}

	if (type->export.basetype == SMI_BASETYPE_OCTETSTRING) {
	    if ((((Range *)p->ptr)->export.minValue.basetype ==
		 SMI_BASETYPE_INTEGER32) &&
		(((Range *)p->ptr)->export.minValue.value.integer32 < 0)) {
		smiPrintError(parser, ERR_RANGE_OUT_OF_BASETYPE);
	    }
	    if ((((Range *)p->ptr)->export.maxValue.basetype ==
		 SMI_BASETYPE_INTEGER32) &&
		(((Range *)p->ptr)->export.maxValue.value.integer32 < 0)) {
		smiPrintError(parser, ERR_RANGE_OUT_OF_BASETYPE);
	    }
	    if ((((Range *)p->ptr)->export.minValue.basetype ==
		 SMI_BASETYPE_UNSIGNED32) &&
		(((Range *)p->ptr)->export.minValue.value.unsigned32 > 65535)) {
		smiPrintError(parser, ERR_RANGE_OUT_OF_BASETYPE);
	    }
	    if ((((Range *)p->ptr)->export.maxValue.basetype ==
		 SMI_BASETYPE_UNSIGNED32) &&
		(((Range *)p->ptr)->export.maxValue.value.unsigned32 > 65535)) {
		smiPrintError(parser, ERR_RANGE_OUT_OF_BASETYPE);
	    }
	    ((Range *)p->ptr)->export.minValue.basetype =
		SMI_BASETYPE_UNSIGNED32;
	    ((Range *)p->ptr)->export.maxValue.basetype =
		SMI_BASETYPE_UNSIGNED32;
	}

	if (compareValues(&((Range *)p->ptr)->export.minValue,
			  &((Range *)p->ptr)->export.maxValue) > 0) {
	    SmiValue v;
	    v = ((Range *)p->ptr)->export.minValue;
	    ((Range *)p->ptr)->export.minValue = ((Range *)p->ptr)->export.maxValue;
	    ((Range *)p->ptr)->export.maxValue = v;
	    smiPrintError(parser, ERR_EXCHANGED_RANGE_LIMITS);
	}

	/* sort */
	p->nextPtr = NULL;
	if (p != type->listPtr) {
	    if (compareValues(&((Range *)p->ptr)->export.minValue,
	      	       &((Range *)type->listPtr->ptr)->export.minValue) <= 0) {
		if (compareValues(&((Range *)p->ptr)->export.maxValue,
	      	       &((Range *)type->listPtr->ptr)->export.minValue) >= 0) {
		    smiPrintError(parser, ERR_RANGE_OVERLAP);
		}
		smiPrintError(parser, ERR_RANGES_NOT_ASCENDING);
		p->nextPtr = type->listPtr;
		type->listPtr = p;
	    } else {
		for (pp = type->listPtr; pp; pp = nextPP) {
		    nextPP = pp->nextPtr;
		    if ((!nextPP) ||
			(compareValues(&((Range *)p->ptr)->export.minValue,
			     &((Range *)nextPP->ptr)->export.minValue) <= 0)) {
			if (((nextPP) &&
			     (compareValues(&((Range *)p->ptr)->export.maxValue,
					    &((Range *)nextPP->ptr)->export.minValue) >= 0)) ||
			    (compareValues(&((Range *)p->ptr)->export.minValue,
					   &((Range *)pp->ptr)->export.maxValue) <= 0)) {
			    smiPrintError(parser, ERR_RANGE_OVERLAP);
			}
			p->nextPtr = pp->nextPtr;
			pp->nextPtr = p;
			if (p->nextPtr) {
			    smiPrintError(parser, ERR_RANGES_NOT_ASCENDING);
			    pp->nextPtr = NULL;
			} 
			break;
		    }
		}
	    }
	}
    }

    /* range normalization */
    for (p = type->listPtr, pp = p; p; p = nextPtr) {
	nextPtr = p->nextPtr;
	if (nextPtr &&
	    compareValues(&((Range *)p->ptr)->export.maxValue,
			  &((Range *)nextPtr->ptr)->export.minValue) == -1) {
	    ((Range *)nextPtr->ptr)->export.minValue =
		((Range *)p->ptr)->export.minValue;
	    if (p == type->listPtr) {
		type->listPtr = nextPtr;
		pp = nextPtr;
	    } else {
		pp->nextPtr = nextPtr;
	    }
	    smiFree(p);
	} else {
	    pp = p;
	}
    }

}



/*
 *----------------------------------------------------------------------
 *
 * smiCheckTypeFormat --
 *
 *      Check whether we know a format specification for integer types.
 *
 * Results:
 *      None.
 *
 * Side effects:
 *      None.
 *
 *----------------------------------------------------------------------
 */

void
smiCheckTypeFormat(Parser *parser, Type *type)
{
    Type *t;
    
    if (! type || !type->export.name) {
	return;
    }

    if (type->export.basetype != SMI_BASETYPE_INTEGER32
	&& type->export.basetype != SMI_BASETYPE_INTEGER64
	&& type->export.basetype != SMI_BASETYPE_UNSIGNED32
	&& type->export.basetype != SMI_BASETYPE_UNSIGNED64
	&& type->export.basetype != SMI_BASETYPE_OCTETSTRING) {
	return;
    }

    for (t = type; t; t = t->parentPtr) {
	if (t->export.format) {
	    break;
	}
    }

    if (! t) {
	smiPrintErrorAtLine(parser, ERR_TYPE_WITHOUT_FORMAT, type->line,
			    type->export.name);
    }
}

/*
 *----------------------------------------------------------------------
 *
 * smiCheckValueType --
 *
 *      Check whether a given value matches a given type.
 *
 * Results:
 *      None.
 *
 * Side effects:
 *      None.
 *
 *----------------------------------------------------------------------
 */

void
smiCheckValueType(Parser *parser, SmiValue *value, Type *type, int line)
{
    List *p, *nextPtr;
    
    if (value && (value->basetype != SMI_BASETYPE_UNKNOWN) && type) {

	/*
	 * If defval type and object type don't match, check whether
	 * the defval value is in the allowed range of the object's basetype.
	 */
	if ((type->export.basetype == SMI_BASETYPE_INTEGER32) ||
	    (type->export.basetype == SMI_BASETYPE_ENUM)) {
	    if (((value->basetype == SMI_BASETYPE_INTEGER64) &&
		 ((value->value.integer64 < (SmiInteger64)(-2147483647 - 1)) ||
		  (value->value.integer64 > (SmiInteger64)2147483647))) ||
		((value->basetype == SMI_BASETYPE_UNSIGNED32) &&
		 ((value->value.unsigned32 > 2147483647))) ||
		((value->basetype == SMI_BASETYPE_UNSIGNED64) &&
		 ((value->value.unsigned32 > 2147483647)))) {
		smiPrintErrorAtLine(parser, ERR_DEFVAL_OUT_OF_BASETYPE, line);
	    }
	}
	if (type->export.basetype == SMI_BASETYPE_UNSIGNED32) {
	    if (((value->basetype == SMI_BASETYPE_INTEGER64) &&
		 ((value->value.integer64 < 0) ||
		  (value->value.integer64 > (SmiInteger64)4294967295UL))) ||
		((value->basetype == SMI_BASETYPE_INTEGER32) &&
		 ((value->value.integer32 < 0))) ||
		((value->basetype == SMI_BASETYPE_UNSIGNED64) &&
		 ((value->value.unsigned32 > (SmiUnsigned32)4294967295UL)))) {
		smiPrintErrorAtLine(parser, ERR_DEFVAL_OUT_OF_BASETYPE, line);
	    }
	}

	/*
	 * "cast" the defval to the object's basetype.
	 */
	value->basetype = type->export.basetype;

	/*
	 * check whether the defval matches the object's range restriction.
	 */
	if ((value->basetype == SMI_BASETYPE_UNSIGNED32) ||
	    (value->basetype == SMI_BASETYPE_UNSIGNED64) ||
	    (value->basetype == SMI_BASETYPE_INTEGER32) ||
	    (value->basetype == SMI_BASETYPE_INTEGER64)) {
	    for (p = type->listPtr; p; p = nextPtr) {
		nextPtr = p->nextPtr;
		if ((compareValues(&((Range *)p->ptr)->export.minValue,
				   value) <= 0) &&
		    (compareValues(&((Range *)p->ptr)->export.maxValue,
				   value) >= 0)) {
		    break;
		}
	    }
	    if ((p == NULL) && type->listPtr) {
		smiPrintErrorAtLine(parser, ERR_DEFVAL_OUT_OF_RANGE, line);
	    }
	}

	/*
	 * check whether the defval matches the object's enumeration.
	 */
	if (value->basetype == SMI_BASETYPE_ENUM) {
	    for (p = type->listPtr; p; p = nextPtr) {
		nextPtr = p->nextPtr;

		if (((NamedNumber *)(p->ptr))->export.value.value.integer32 ==
		    value->value.integer32) {
			break;
		}
	    }
	    if (p == NULL) {
		smiPrintErrorAtLine(parser, ERR_DEFVAL_OUT_OF_ENUM, line);
	    }
	}
    }
}



/*
 *----------------------------------------------------------------------
 *
 * smiCheckDefault --
 *
 *      Check whether the default value (if present) matches the
 *	underlying type and restrictions.
 *
 * Results:
 *      None.
 *
 * Side effects:
 *      None.
 *
 *----------------------------------------------------------------------
 */

void
smiCheckDefault(Parser *parser, Object *object)
{
    smiCheckValueType(parser, &object->export.value, object->typePtr,
		      object->line);
}



/*
 *----------------------------------------------------------------------
 *
 * smiCheckTypeUsage --
 *
 *      Check whether the types of all objects are used appropriately.
 *
 * Results:
 *      None.
 *
 * Side effects:
 *      None.
 *
 *----------------------------------------------------------------------
 */

static void
checkInetAddressType(Parser *parserPtr, Module *modulePtr, Object *objectPtr)
{
    Module *inetModulePtr = NULL;		/* RFC 3291 */
    Type *inetAddressTypePtr = NULL;		/* RFC 3291 */
    Type *inetAddressPtr = NULL;		/* RFC 3291 */
    Node *nodePtr;
    List *listPtr = NULL;
    int i;

    const char *protected[] = {
	"InetAddressIPv4", "InetAddressIPv6",
	"InetAddressIPv4z", "InetAddressIPv6z",
	"InetAddressDNS", NULL
    };

    inetModulePtr = findModuleByName("INET-ADDRESS-MIB");
    if (! inetModulePtr) {
	return;
    }

    inetAddressTypePtr = findTypeByModuleAndName(inetModulePtr,
						 "InetAddressType");
    inetAddressPtr = findTypeByModuleAndName(inetModulePtr,
					     "InetAddress");
    
    if (!inetAddressTypePtr || !inetAddressPtr) {
	return;
    }
    
    /* check InetAddressType/InetAddress pair */
    if (smiTypeDerivedFrom(objectPtr->typePtr, inetAddressPtr)) {
	Object *indexObject = NULL;
	Object *entryObject = objectPtr->nodePtr->parentPtr->lastObjectPtr;
	if (entryObject) {
	    switch (entryObject->export.indexkind) {
	    case SMI_INDEX_INDEX:
		indexObject = entryObject;
		break;
	    case SMI_INDEX_AUGMENT:
		indexObject = entryObject->relatedPtr;
		break;
	    default:
		/* xxx need to handle other index constructions */
		indexObject = NULL;
		break;
	    }
	}
	if (indexObject) {
	    for (listPtr = indexObject->listPtr; listPtr;
		 listPtr = listPtr->nextPtr) {
		Object *iObject = (Object *) listPtr->ptr;
		if (iObject
		    && iObject->typePtr == inetAddressTypePtr) {
		    break;
		}
	    }
	    
	}
	if (! indexObject || ! listPtr) {
	    for (nodePtr =
		     objectPtr->nodePtr->parentPtr->firstChildPtr;
		 nodePtr &&
		     nodePtr->subid < objectPtr->nodePtr->subid &&
		     nodePtr->lastObjectPtr->typePtr !=
		     inetAddressTypePtr;
		 nodePtr = nodePtr->nextPtr);
	    if (!nodePtr ||
		nodePtr->subid >= objectPtr->nodePtr->subid) {
		smiPrintErrorAtLine(parserPtr,
				    ERR_INETADDRESS_WITHOUT_TYPE,
				    objectPtr->line);
	    }
	}
    }
    
    /* check InetAddressType subtyping */
    if (objectPtr->typePtr->parentPtr == inetAddressTypePtr) {
	smiPrintErrorAtLine(parserPtr,
			    ERR_INETADDRESSTYPE_SUBTYPED,
			    objectPtr->line);
    }

    /* check for TCs that should not be used directly */
    for (i = 0; protected[i]; i++) {
	if (objectPtr->typePtr == findTypeByModuleAndName(inetModulePtr,
							  protected[i])) {
	    smiPrintErrorAtLine(parserPtr,
				ERR_INETADDRESS_SPECIFIC,
				objectPtr->line,
				objectPtr->typePtr->export.name);
	    break;
	}
    }
}


static void
checkTransportAddressType(Parser *parserPtr, Module *modulePtr, Object *objectPtr)
{
    Module *transportModulePtr = NULL;		/* RFC 3419 */
    Type *transportAddressTypePtr = NULL;	/* RFC 3419 */
    Type *transportAddressPtr = NULL;		/* RFC 3419 */
    Type *transportDomainPtr = NULL;		/* RFC 3419 */
    Node *nodePtr;
    List *listPtr = NULL;
    int i;

    const char *protected[] = {
	"TransportAddressIPv4", "TransportAddressIPv6",
	"TransportAddressIPv4z", "TransportAddressIPv6z",
	"TransportAddressDNS", "TransportAddressLocal",
	NULL
    };

    transportModulePtr = findModuleByName("TRANSPORT-ADDRESS-MIB");
    if (! transportModulePtr) {
	return;
    }

    transportAddressTypePtr = findTypeByModuleAndName(transportModulePtr,
						 "TransportAddressType");
    transportAddressPtr = findTypeByModuleAndName(transportModulePtr,
					     "TransportAddress");
    transportDomainPtr = findTypeByModuleAndName(transportModulePtr,
						 "TransportDomain");

    if (!transportAddressTypePtr || !transportAddressPtr || !transportDomainPtr) {
	return;
    }
    
    /* check TransportAddressType/TransportAddress pair */
    if (smiTypeDerivedFrom(objectPtr->typePtr, transportAddressPtr)) {
	Object *indexObject = NULL;
	Object *entryObject = objectPtr->nodePtr->parentPtr->lastObjectPtr;
	if (entryObject) {
	    switch (entryObject->export.indexkind) {
	    case SMI_INDEX_INDEX:
		indexObject = entryObject;
		break;
	    case SMI_INDEX_AUGMENT:
		indexObject = entryObject->relatedPtr;
		break;
	    default:
		/* xxx need to handle other index constructions */
		indexObject = NULL;
		break;
	    }
	}
	if (indexObject) {
	    for (listPtr = indexObject->listPtr; listPtr;
		 listPtr = listPtr->nextPtr) {
		Object *iObject = (Object *) listPtr->ptr;
		if (iObject
		    && (iObject->typePtr == transportAddressTypePtr
			|| iObject->typePtr == transportDomainPtr)) {
		    break;
		}
	    }
	}
	if (! indexObject || ! listPtr) {
	    for (nodePtr =
		     objectPtr->nodePtr->parentPtr->firstChildPtr;
		 nodePtr &&
		     nodePtr->subid < objectPtr->nodePtr->subid &&
		     nodePtr->lastObjectPtr->typePtr !=
		     transportAddressTypePtr &&
		     nodePtr->lastObjectPtr->typePtr !=
		     transportDomainPtr;
		 nodePtr = nodePtr->nextPtr);
	    if (!nodePtr ||
		nodePtr->subid >= objectPtr->nodePtr->subid) {
		smiPrintErrorAtLine(parserPtr,
				    ERR_TRANSPORTADDRESS_WITHOUT_TYPE,
				    objectPtr->line);
	    }
	}
    }
    
    /* check TransportAddressType subtyping */
    if (objectPtr->typePtr->parentPtr == transportAddressTypePtr) {
	smiPrintErrorAtLine(parserPtr,
			    ERR_TRANSPORTADDRESSTYPE_SUBTYPED,
			    objectPtr->line);
    }

    /* check for TCs that should not be used directly */
    for (i = 0; protected[i]; i++) {
	if (objectPtr->typePtr == findTypeByModuleAndName(transportModulePtr,
							  protected[i])) {
	    smiPrintErrorAtLine(parserPtr,
				ERR_TRANSPORTADDRESS_SPECIFIC,
				objectPtr->line,
				objectPtr->typePtr->export.name);
	    break;
	}
    }
}


void
smiCheckTypeUsage(Parser *parserPtr, Module *modulePtr)
{
    Object *objectPtr;
    Module *tcModulePtr = NULL;
    Type *rowStatusPtr = NULL;
    Type *storageTypePtr = NULL;
    Type *taddressPtr = NULL;
    Type *tdomainPtr = NULL;
    NamedNumber *nnPtr;
    Node *nodePtr;
    
    tcModulePtr = findModuleByName("SNMPv2-TC");
    if (tcModulePtr) {
	rowStatusPtr = findTypeByModuleAndName(tcModulePtr, "RowStatus");
	storageTypePtr = findTypeByModuleAndName(tcModulePtr, "StorageType");
	taddressPtr = findTypeByModuleAndName(tcModulePtr, "TAddress");
	tdomainPtr = findTypeByModuleAndName(tcModulePtr, "TDomain");
    }
    
    for (objectPtr = modulePtr->firstObjectPtr;
	 objectPtr; objectPtr = objectPtr->nextPtr) {

	if (objectPtr->typePtr) {

	    if (tcModulePtr) {

		/* check RowStatus DEFVAL */
		if (objectPtr->typePtr == rowStatusPtr) {
		    if ((objectPtr->export.value.value.integer32 >= 4) &&
			(objectPtr->export.value.value.integer32 <= 6)) {
			nnPtr = findTypeNamedNumber(rowStatusPtr,
				     objectPtr->export.value.value.integer32);
			smiPrintErrorAtLine(parserPtr,
					    ERR_ILLEGAL_ROWSTATUS_DEFAULT,
					    objectPtr->line,
					    nnPtr->export.name);
		    }
		}

		/* check RowStatus read-create status */
		if (objectPtr->typePtr == rowStatusPtr) {
		    Object *entryObject
			= objectPtr->nodePtr->parentPtr->lastObjectPtr;
		    if (objectPtr->export.access != SMI_ACCESS_READ_WRITE
			|| !entryObject->export.create) {
			smiPrintErrorAtLine(parserPtr,
					    ERR_ILLEGAL_ROWSTATUS_ACCESS,
					    objectPtr->line);
		    }
		}
		
		/* check StorageType DEFVAL */
		if (objectPtr->typePtr == storageTypePtr) {
		    if ((objectPtr->export.value.value.integer32 >= 4) &&
			(objectPtr->export.value.value.integer32 <= 5)) {
			nnPtr = findTypeNamedNumber(storageTypePtr,
				     objectPtr->export.value.value.integer32);
			smiPrintErrorAtLine(parserPtr,
					    ERR_ILLEGAL_STORAGETYPE_DEFAULT,
					    objectPtr->line,
					    nnPtr->export.name);
		    }
		}

		/* check TDomain/TAddress pair */
		if (smiTypeDerivedFrom(objectPtr->typePtr, taddressPtr)) {
		    for (nodePtr =
			     objectPtr->nodePtr->parentPtr->firstChildPtr;
			 nodePtr &&
			     nodePtr->lastObjectPtr->typePtr != tdomainPtr;
			 nodePtr = nodePtr->nextPtr);
		    if (!nodePtr ||
			nodePtr->lastObjectPtr->typePtr != tdomainPtr) {
			smiPrintErrorAtLine(parserPtr,
					    ERR_TADDRESS_WITHOUT_TDOMAIN,
					    objectPtr->line);
		    }
		}
		
	    }
	    checkInetAddressType(parserPtr, modulePtr, objectPtr);
	    checkTransportAddressType(parserPtr, modulePtr, objectPtr);
	}
    }
}


static char *status[] = { "Unknown", "current", "deprecated",
			  "mandatory", "optional", "obsolete" };

static int
memberOfGroup(Object *object, Object *group)
{
    List *listPtr;

    for (listPtr = group->listPtr; listPtr; listPtr = listPtr->nextPtr) {
	if (listPtr->ptr == object) {
	    return 1;
	}
    }
    return 0;
}

/*
 *----------------------------------------------------------------------
 *
 * smiCheckComplianceStatus --
 *
 *      Make sure that all groups and objects in a compliance statement
 *      are at least as current as the compliance itself.
 *      XXX I'm not sure I traversed the whole compliance statement,
 *          this at least covers the common case
 *
 * Results:
 *      None.
 *
 * Side effects:
 *      None.
 *
 *----------------------------------------------------------------------
 */

void
smiCheckComplianceStatus(Parser *parser, Object *compliance)
{
    List *listPtr, *groupListPtr;
    Object *memberPtr;
    Object *group;

    for (listPtr = compliance->listPtr;
	 listPtr; listPtr = listPtr->nextPtr) {
	
	memberPtr = (Object *) listPtr->ptr;
	if (!memberPtr)
	    continue;
	addObjectFlags(memberPtr, FLAG_INCOMPLIANCE);
	if (memberPtr->export.status > compliance->export.status) {
	    smiPrintErrorAtLine(parser, ERR_COMPLIANCE_GROUP_STATUS,
				compliance->line,
				status[compliance->export.status],
				compliance->export.name,
				status[memberPtr->export.status],
				memberPtr->export.name);
	}
    }
    for (listPtr = compliance->optionlistPtr;
	 listPtr; listPtr = listPtr->nextPtr) {
	
	memberPtr = ((Option *) listPtr->ptr)->objectPtr;
	addObjectFlags(memberPtr, FLAG_INCOMPLIANCE);
	if (memberPtr->export.status > compliance->export.status) {
	    smiPrintErrorAtLine(parser, ERR_COMPLIANCE_GROUP_STATUS,
				((Option *) listPtr->ptr)->line,
				status[compliance->export.status],
				compliance->export.name,
				status[memberPtr->export.status],
				memberPtr->export.name);
	}
    }
    for (listPtr = compliance->refinementlistPtr;
	 listPtr; listPtr = listPtr->nextPtr) {
	
	memberPtr = ((Refinement *) listPtr->ptr)->objectPtr;

	for (groupListPtr = compliance->listPtr;
	     groupListPtr; groupListPtr = groupListPtr->nextPtr) {

	    group = (Object *) groupListPtr->ptr;
	    if (group && memberOfGroup(memberPtr, group)) {
		break;
	    }
	}

	if (! groupListPtr) {
	    for (groupListPtr = compliance->optionlistPtr;
		 groupListPtr; groupListPtr = groupListPtr->nextPtr) {

		group = ((Option *) groupListPtr->ptr)->objectPtr;
		if (group && memberOfGroup(memberPtr, group)) {
		    break;
		}
	    }
	}

	if (! groupListPtr) {
	    smiPrintErrorAtLine(parser, ERR_REFINEMENT_NOT_LISTED,
				((Refinement *) listPtr->ptr)->line,
				memberPtr->export.name);
	}
		
	addObjectFlags(memberPtr, FLAG_INCOMPLIANCE);
	if (memberPtr->export.status > compliance->export.status) {
	    smiPrintErrorAtLine(parser, ERR_COMPLIANCE_OBJECT_STATUS,
				((Refinement *) listPtr->ptr)->line,
				status[compliance->export.status],
				compliance->export.name,
				status[memberPtr->export.status],
				memberPtr->export.name);
	}
    }
}

/*
 *----------------------------------------------------------------------
 *
 * smiCheckGroupMembers --
 *
 *      Check whether only scalar and column nodes and notifications
 *	are contained in a conformance group.
 *
 *      Also ensure that group members are at least as current
 *      as the group itself.
 *
 * Results:
 *      None.
 *
 * Side effects:
 *      None.
 *
 *----------------------------------------------------------------------
 */

void
smiCheckGroupMembers(Parser *parser, Object *group)
{
    List *listPtr;
    Object *memberPtr;
    int scalarsOrColumns = 0;
    int notifications = 0;
    
    for (listPtr = group->listPtr;
	 listPtr; listPtr = listPtr->nextPtr) {
	
	memberPtr = (Object *) listPtr->ptr;

	if (((memberPtr->export.nodekind == SMI_NODEKIND_COLUMN
	      || memberPtr->export.nodekind == SMI_NODEKIND_SCALAR)
	     && memberPtr->export.access != SMI_ACCESS_NOT_ACCESSIBLE)
	    || memberPtr->export.nodekind == SMI_NODEKIND_NOTIFICATION) {
	    if (memberPtr->export.nodekind == SMI_NODEKIND_NOTIFICATION) {
		notifications++;
		if (group->export.decl == SMI_DECL_OBJECTGROUP) {
		    smiPrintErrorAtLine(parser,
					ERR_NOTIFICATION_IN_OBJECT_GROUP,
					group->line,
					group->export.name,
					memberPtr->export.name);
		}
	    } else {
		scalarsOrColumns++;
		if (group->export.decl == SMI_DECL_NOTIFICATIONGROUP) {
		    smiPrintErrorAtLine(parser,
					ERR_OBJECT_IN_NOTIFICATION_GROUP,
					group->line,
					group->export.name,
					memberPtr->export.name);
		}
	    }
	    addObjectFlags(memberPtr, FLAG_INGROUP);
	} else if (!(memberPtr->flags & FLAG_INCOMPLETE)) {
	    /* unknown OIDs are already flagged */
	    smiPrintErrorAtLine(parser, ERR_INVALID_GROUP_MEMBER,
				group->line,
				memberPtr->export.name,
				group->export.name);
	}
	if (memberPtr->export.status > group->export.status) {
	    smiPrintErrorAtLine(parser, ERR_GROUP_OBJECT_STATUS,
				group->line,
				status[group->export.status],
				group->export.name,
				status[memberPtr->export.status],
				memberPtr->export.name);
	}
    }

    if (scalarsOrColumns && notifications) {
	smiPrintErrorAtLine(parser, ERR_MIXED_GROUP_MEMBERS,
			    group->line,
			    group->export.name);
    }
}

/*
 *----------------------------------------------------------------------
 *
 * smiCheckGroupMembership --
 *
 *      Check whether scalar and column nodes and notifications are
 *	contained in at least one conformance group.
 *
 *	This function assumes that smiCheckGroupMembers() has been
 *	called on all group objects and smiCheckComplianceStatus()
 *      has been called on all compliance objects before.
 *
 * Results:
 *      None.
 *
 * Side effects:
 *      None.
 *
 *----------------------------------------------------------------------
 */

void
smiCheckGroupMembership(Parser *parser, Object *objectPtr)
{
    int found;
    
    if (((objectPtr->export.nodekind == SMI_NODEKIND_COLUMN
	  || objectPtr->export.nodekind == SMI_NODEKIND_SCALAR)
	 && objectPtr->export.access != SMI_ACCESS_NOT_ACCESSIBLE)
	|| objectPtr->export.nodekind == SMI_NODEKIND_NOTIFICATION) {

	found = (objectPtr->flags & FLAG_INGROUP);
	    
	if (! found) {
	    if (objectPtr->export.nodekind == SMI_NODEKIND_NOTIFICATION) {
		smiPrintErrorAtLine(parser, ERR_NOTIFICATION_NOT_IN_GROUP,
				    objectPtr->line,
				    objectPtr->export.name);
	    } else {
		smiPrintErrorAtLine(parser, ERR_NODE_NOT_IN_GROUP,
				    objectPtr->line,
				    objectPtr->export.name);
	    }
	}
    }
    if (objectPtr->export.nodekind == SMI_NODEKIND_GROUP) {

	found = (objectPtr->flags & FLAG_INCOMPLIANCE);

	if (!found && objectPtr->export.status != SMI_STATUS_OBSOLETE) {
	    smiPrintErrorAtLine(parser, ERR_GROUP_UNREF,
				objectPtr->line,
				status[objectPtr->export.status],
				objectPtr->export.name);
	}
    }

}

/*
 *----------------------------------------------------------------------
 *
 * smiCheckObjectReuse --
 *
 *      Check whether a newly defined Object represents a duplicate
 *      or a reused OID.
 *
 * Results:
 *      None.
 *
 * Side effects:
 *      Allocates a new Object and adjusts the objectPtr parameter.
 *
 *----------------------------------------------------------------------
 */

void
smiCheckObjectReuse(Parser *parser, char *name, Object **objectPtr)
{
    if ((((*objectPtr)->flags & FLAG_INCOMPLETE) == 0) &&
	strcmp(name, (*objectPtr)->export.name)) {
	if ((*objectPtr)->export.decl >= SMI_DECL_OBJECTTYPE) {
	    smiPrintError(parser, ERR_OID_REGISTERED, name,
			  (*objectPtr)->export.name);
	} else {
	    smiPrintError(parser, ERR_OID_REUSE, name,
			  (*objectPtr)->export.name);
	}
	smiPrintErrorAtLine(parser, ERR_PREVIOUS_DEFINITION,
			    (*objectPtr)->line, (*objectPtr)->export.name);
	*objectPtr = duplicateObject(*objectPtr, 0, parser);
    }
    
    if ((*objectPtr)->modulePtr != parser->modulePtr) {
	*objectPtr = duplicateObject(*objectPtr, 0, parser);
    }
}

/*
 *----------------------------------------------------------------------
 *
 * smiCheckNotificationOid --
 *
 *      Check whether SMIv2 notifications are reversible and whether
 *	the last sub-identifer fits into a signed 32-bit integer.
 *
 * Results:
 *      None.
 *
 * Side effects:
 *      None.
 *
 *----------------------------------------------------------------------
 */

void
smiCheckNotificationOid(Parser *parser, Module *module, Object *object)
{
    static const char *name[] = {
	"SNMPv2-MIB", "coldStart",
	"SNMPv2-MIB", "warmStart",
	"IF-MIB", "linkDown",
	"IF-MIB", "linkUp",
	"SNMPv2-MIB", "authenticationFailure",
	/* egpNeighborLoss is not really defined in any SMI module */
	NULL, NULL };

    int i;

    if (parser->modulePtr->export.language == SMI_LANGUAGE_SMIV2) {
	for (i = 0; name[i]; i+= 2) {
	    if (strcmp(name[i], module->export.name) == 0
		&& strcmp(name[i+1], object->export.name) == 0) {
		break;
	    }
	}
	if (! name[i]) {
	    Node *parent = object->nodePtr->parentPtr;
	    if (parent && parent->subid != 0) {
		smiPrintErrorAtLine(parser, ERR_NOTIFICATION_NOT_REVERSIBLE,
				    object->line, object->export.name);
	    }
	}
    }

    if (object->nodePtr->subid > 2147483647) {
	smiPrintErrorAtLine(parser, ERR_NOTIFICATION_ID_TOO_LARGE,
			    object->line, object->export.name);
    }
}

/*
 *----------------------------------------------------------------------
 *
 * smiCheckNotificationMembers --
 *
 *      Check whether a newly defined notification contains only members
 *	of a single logical object.
 *
 * Results:
 *      None.
 *
 * Side effects:
 *      None.
 *
 *----------------------------------------------------------------------
 */

void
smiCheckNotificationMembers(Parser *parser, Object *object)
{
    List *listPtr;
    Object *memberPtr;
#if 0
    Node *parent = NULL;
#endif
    Node *node = NULL;

    for (listPtr = object->listPtr;
	 listPtr; listPtr = listPtr->nextPtr) {
	
	memberPtr = (Object *) listPtr->ptr;

	if (memberPtr->export.nodekind == SMI_NODEKIND_SCALAR) {
	    if (memberPtr->nodePtr && memberPtr->nodePtr->parentPtr) {
		node = memberPtr->nodePtr->parentPtr;
	    }
	} else if (memberPtr->export.nodekind == SMI_NODEKIND_COLUMN) {
	    if (memberPtr->nodePtr && memberPtr->nodePtr->parentPtr
		&& memberPtr->nodePtr->parentPtr->parentPtr) {
		node = memberPtr->nodePtr->parentPtr->parentPtr;
	    }
	} else {
	    smiPrintErrorAtLine(parser, ERR_NOTIFICATION_OBJECT_TYPE,
				object->line, memberPtr->export.name,
				object->export.name);
	}

	if (memberPtr->export.access == SMI_ACCESS_NOT_ACCESSIBLE) {
	    smiPrintErrorAtLine(parser, ERR_NOTIFICATION_OBJECT_ACCESS,
				object->line, memberPtr->export.name,
				object->export.name);
	}

	/* xxx check for duplicates */
#if 0
	if (node) {
	    if (! parent) {
		parent = node;
	    } else {
		if (parent != node) {
		    /* xxx do not report multiple times xxx */
		    smiPrintErrorAtLine(parser, ERR_NOTIFICATION_OBJECT_MIX,
					object->line, object->export.name);
		}
	    }
	}
#endif
    }
}

/*
 *----------------------------------------------------------------------
 *
 * smiCheckUniqueness --
 *
 *      Check whether all entries for an UNIQUENESS clause are in fact
 *      columns.
 *
 * Results:
 *      None.
 *
 * Side effects:
 *      None.
 *
 *----------------------------------------------------------------------
 */

void smiCheckUniqueness(Parser *parser, Object *object)
{
    List *p;

    for (p = object->uniquenessPtr; p; p = p->nextPtr) {
        Object *uniq = (Object *)p->ptr;
        int found = 0;
        List *pp;

        if (uniq && object->typePtr) {
            for (pp = object->typePtr->listPtr; pp; pp = pp->nextPtr)
                if (pp->ptr &&
                    !strcmp(uniq->export.name, ((Object *)pp->ptr)->export.name)) {
                    found = 1;
                    break;
                }
            if (!found) {
                if (((object->export.indexkind == SMI_INDEX_AUGMENT) ||
                    (object->export.indexkind == SMI_INDEX_SPARSE)) &&
                    (object->relatedPtr && object->relatedPtr->typePtr)) {
                    for (pp = object->relatedPtr->typePtr->listPtr; pp;
                         pp = pp->nextPtr)
                        if (pp->ptr &&
                            !strcmp(uniq->export.name, ((Object *)pp->ptr)->export.name)) {
                            found = 1;
                            break;
                        }
                }
            }
            if (!found)
                smiPrintErrorAtLine(parser, ERR_NOT_A_COLUMN,
                                    object->line, uniq->export.name);
        }
    }
}

/*
 *----------------------------------------------------------------------
 *
 * smiCheckModuleIdentityRegistration --
 *
 *      Check whether the module identity is registered in a well
 *	known (IANA) controlled location. In particular, warn if
 *	the OID is below iso(1).org(3).dod(6).mgmt(1) and not
 *	below well known registration locations such as mib-2,
 *	transmission, or snmpModules.
 *
 * Results:
 *      None.
 *
 * Side effects:
 *      None.
 *
 *----------------------------------------------------------------------
 */

void
smiCheckModuleIdentityRegistration(Parser *parser, Object *object)
{
    static const SmiSubid mgmt[] = { 1, 3, 6, 1, 2 };
    static const SmiSubid mib2[] = { 1, 3, 6, 1, 2, 1 };
    static const SmiSubid transmission[] = { 1, 3, 6, 1, 2, 1, 10 };
    static const SmiSubid snmpModules[] = { 1, 3, 6, 1, 6, 3 };

    if (object->export.oidlen < sizeof(mgmt)/sizeof(SmiSubid)
	|| memcmp(object->export.oid, mgmt, sizeof(mgmt)) != 0) {
	return;
    }

    if (object->export.oidlen == sizeof(mib2)/sizeof(SmiSubid) + 1
	&& memcmp(object->export.oid, mib2, sizeof(mib2)) == 0) {
	return;
    }

    if (object->export.oidlen == sizeof(transmission)/sizeof(SmiSubid) + 1
	&& memcmp(object->export.oid, transmission, sizeof(transmission)) == 0) {
	return;
    }

    if (object->export.oidlen == sizeof(snmpModules)/sizeof(SmiSubid) + 1
	&& memcmp(object->export.oid, snmpModules, sizeof(snmpModules)) == 0) {
	return;
    }

    smiPrintErrorAtLine(parser, ERR_MODULE_IDENTITY_REGISTRATION,
			object->line);
}

/*
 *----------------------------------------------------------------------
 *
 * smiyyerror --
 *
 *      Prints an error message from the parser.  In SMIv1 and v2,
 *      a common error is to terminate a comment early, so if the
 *	current line contains a comment (parserPtr->lcline) print
 *	the ERR_COMMENT_TERMINATES.
 *
 *----------------------------------------------------------------------
 */
void smiyyerror(char *msg, Parser *parserPtr)
{
	if (parserPtr->line == parserPtr->lcline &&
	    parserPtr->modulePtr &&
	    (parserPtr->modulePtr->export.language == SMI_LANGUAGE_SMIV1 ||
	     parserPtr->modulePtr->export.language == SMI_LANGUAGE_SMIV2))
		smiPrintError(parserPtr, ERR_COMMENT_TERMINATES);
	smiPrintError(parserPtr, ERR_OTHER_ERROR, msg);
}
