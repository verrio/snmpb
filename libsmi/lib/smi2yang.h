/*
 * smi2yang.h --
 *
 *      Conversion of SMIv2 modules to YANG modules
 *
 * Copyright (c) 2010-2011 J. Schoenwaelder, Jacobs University Bremen
 *
 * See the file "COPYING" for information on usage and redistribution
 * of this file, and for a DISCLAIMER OF ALL WARRANTIES.
 *
 * Author: Juergen Schoenwaelder
 * @(#) $Id: data.c 12198 2009-01-05 08:37:07Z schoenw $
 */

#ifndef _SMI2YANG_H
#define _SMI2YANG_H

#include <smi.h>
#include <yang.h>

#ifdef __cplusplus
extern "C" {
#endif

#define SMI_TO_YANG_FLAG_SMI_EXTENSIONS	0x01

extern YangNode *yangGetModuleFromSmiModule(SmiModule *smiModule, int flags);

#ifdef __cplusplus
}
#endif

#endif /* _SMI2YANG_H */


