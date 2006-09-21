/*
 * check.h --
 *
 *      Definitions for the semantic check functions.
 *
 * Copyright (c) 2000 Frank Strauss, Technical University of Braunschweig.
 *
 * See the file "COPYING" for information on usage and redistribution
 * of this file, and for a DISCLAIMER OF ALL WARRANTIES.
 *
 * @(#) $Id: check.h 2032 2005-01-06 23:58:00Z schoenw $
 */

#ifndef _CHECK_H
#define _CHECK_H


#include "data.h"
#include "smi.h"


extern void smiCheckObjectName(Parser *parser, Module *module, char *name);

extern void smiCheckTypeName(Parser *parser,
			     Module *module, char *name, int line);

extern int  smiCheckFormat(Parser *parser,
			   SmiBasetype basetype, char *format, int line);

extern void smiCheckTypeRanges(Parser *parser, Type *type);

extern void smiCheckTypeFormat(Parser *parser, Type *type);

extern void smiCheckTypeUsage(Parser *parserPtr, Module *modulePtr);

extern void smiCheckValueType(Parser *parser, SmiValue *value, Type *type,
			      int line);

extern void smiCheckDefault(Parser *parser, Object *object);

extern void smiCheckComplianceStatus(Parser *parser, Object *compliance);

extern void smiCheckNamedNumberRedefinition(Parser *parser, Type *type);

extern void smiCheckNamedNumberSubtyping(Parser *parser, Type *type);

extern void smiCheckNamedNumbersOrder(Parser *parser, Type *type);

extern void smiCheckIndex(Parser *parser, Object *object);

extern void smiCheckAugment(Parser *parser, Object *object);

extern void smiCheckGroupMembership(Parser *parser, Object *object);

extern void smiCheckGroupMembers(Parser *parser, Object *group);

extern void smiCheckObjectReuse(Parser *parser,
				char *name, Object **objectPtr);

extern void smiCheckNotificationOid(Parser *parser, Module *module,
				    Object *object);

extern void smiCheckNotificationMembers(Parser *parser, Object *object);

extern void smiCheckUniqueness(Parser *parser, Object *object);

extern void smiCheckModuleIdentityRegistration(Parser *parser, Object *object);

extern void smiyyerror(char *msg, Parser *parserPtr);

#endif /* _CHECK_H */
