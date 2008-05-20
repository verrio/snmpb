/*
 * smidiff.c --
 *
 *      Compute and check differences between MIB modules.
 *
 * Copyright (c) 2001 T. Klie, Technical University of Braunschweig.
 * Copyright (c) 2001 J. Schoenwaelder, Technical University of Braunschweig.
 * Copyright (c) 2001 F. Strauss, Technical University of Braunschweig.
 * Copyright (c) 2006 J. Schoenwaelder, International University Bremen.
 *
 * See the file "COPYING" for information on usage and redistribution
 * of this file, and for a DISCLAIMER OF ALL WARRANTIES.
 *
 * @(#) $Id: smidiff.c 8090 2008-04-18 12:56:29Z strauss $ 
 */

/*
 * TODO:
 *
 * - replacing an implicit type with a named type should cause a real
 *   error if the new type is formally not identical (Mike Heard)
 */

#include <config.h>

#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <stdarg.h>
#include <ctype.h>
#ifdef HAVE_WIN_H
#include "win.h"
#endif

#include "smi.h"
#include "shhopt.h"


static int errorLevel = 6;	/* smidiff/libsmi error level (inclusive) */
static int mFlag = 0;		/* show the name for error messages */
static int sFlag = 0;		/* show the severity for error messages */
static char *oldCompl = NULL;	/* name of old compliance statement */
static char *newCompl = NULL;	/* name of new compliance statement */

/* the `:' separates the view identifier */
static const char *oldTag = "smidiff:old";
static const char *newTag = "smidiff:new";


#define CODE_SHOW_PREVIOUS		0x01
#define CODE_SHOW_PREVIOUS_IMPLICIT	0x02


typedef struct Error {
    int level;		/* error level - roughly the same as smilint */
    int id;		/* error id used in the error() invocation */
    char *tag;		/* tag for error identification on cmd line */
    char *fmt;		/* the complete error format string */
    char *description;	/* description of the error message */
} Error;


#define ERR_INTERNAL				0
#define ERR_TYPE_REMOVED			1
#define ERR_TYPE_ADDED				2
#define ERR_NODE_REMOVED			3
#define ERR_NODE_ADDED				4
#define ERR_BASETYPE_CHANGED			5
#define ERR_DECL_CHANGED			6
#define ERR_LEGAL_STATUS_CHANGED		8
#define ERR_PREVIOUS_DEFINITION			9
#define ERR_STATUS_CHANGED			10
#define ERR_DESCR_ADDED				11
#define ERR_DESCR_REMOVED			12
#define ERR_DESCR_CHANGED			13
#define ERR_REF_ADDED				14
#define ERR_REF_REMOVED				15
#define ERR_REF_CHANGED				16
#define ERR_FORMAT_ADDED			17
#define ERR_FORMAT_REMOVED			18
#define ERR_FORMAT_CHANGED			19
#define ERR_UNITS_ADDED				20
#define ERR_UNITS_REMOVED			21
#define ERR_UNITS_CHANGED			22
#define ERR_ACCESS_ADDED			23
#define ERR_ACCESS_REMOVED			24
#define ERR_ACCESS_CHANGED			25
#define ERR_NAME_ADDED				26
#define ERR_NAME_REMOVED			27
#define ERR_NAME_CHANGED			28
#define ERR_TO_IMPLICIT				29
#define ERR_FROM_IMPLICIT			30
#define ERR_RANGE_ADDED				31
#define ERR_RANGE_REMOVED			32
#define ERR_RANGE_CHANGED			33
#define ERR_DEFVAL_ADDED			34
#define ERR_DEFVAL_REMOVED			35
#define ERR_DEFVAL_CHANGED			36
#define ERR_ORGA_ADDED				37
#define ERR_ORGA_REMOVED			38
#define ERR_ORGA_CHANGED			39
#define ERR_CONTACT_ADDED			40
#define ERR_CONTACT_REMOVED			41
#define ERR_CONTACT_CHANGED			42
#define ERR_SMIVERSION_CHANGED			43
#define ERR_REVISION_ADDED			44
#define ERR_REVISION_REMOVED			45
#define ERR_REVISION_CHANGED			46
#define ERR_LENGTH_CHANGED			47
#define ERR_LENGTH_OF_TYPE_CHANGED		48
#define ERR_LENGTH_ADDED			49
#define ERR_MEMBER_ADDED			50
#define ERR_MEMBER_REMOVED			51
#define ERR_MEMBER_CHANGED			52
#define ERR_OBJECT_ADDED			53
#define ERR_OBJECT_REMOVED			54
#define ERR_OBJECT_CHANGED			55
#define ERR_NAMED_NUMBER_ADDED			56
#define ERR_NAMED_NUMBER_REMOVED		57
#define ERR_NAMED_NUMBER_CHANGED		58
#define ERR_NAMED_BIT_ADDED_OLD_BYTE		59
#define ERR_NODEKIND_CHANGED			60
#define ERR_INDEXKIND_CHANGED			61
#define ERR_INDEX_CHANGED			62
#define ERR_TYPE_IS_AND_WAS			63
#define ERR_RANGE_OF_TYPE_CHANGED		64
#define ERR_RANGE_OF_TYPE_ADDED			65
#define ERR_RANGE_OF_TYPE_REMOVED		66
#define ERR_TYPE_BASED_ON			67
#define ERR_INDEX_AUGMENT_CHANGED		68
#define ERR_NAMED_NUMBER_OF_TYPE_REMOVED	69
#define ERR_NAMED_NUMBER_TO_TYPE_ADDED		70
#define ERR_NAMED_NUMBER_OF_TYPE_CHANGED	71
#define ERR_NAMED_BIT_OF_TYPE_ADDED_OLD_BYTE	72
#define ERR_LENGTH_REMOVED			73
#define ERR_PREVIOUS_IMPLICIT_DEFINITION	74
#define ERR_STATUS_CHANGED_IMPLICIT		75
#define ERR_LEGAL_STATUS_CHANGED_IMPLICIT	76
#define ERR_LENGTH_OF_TYPE_ADDED		77
#define ERR_LENGTH_OF_TYPE_REMOVED		78
#define ERR_STATUS_ADDED			79
#define ERR_STATUS_REMOVED			80
#define ERR_MANDATORY_GROUP_ADDED		81
#define ERR_MANDATORY_GROUP_REMOVED		82
#define ERR_MANDATORY_EXT_GROUP_ADDED		83
#define ERR_MANDATORY_EXT_GROUP_REMOVED		84
#define ERR_OPTION_ADDED			85
#define ERR_OPTION_REMOVED			86
#define ERR_EXT_OPTION_ADDED			87
#define ERR_EXT_OPTION_REMOVED			88
#define ERR_REFINEMENT_ADDED			89
#define ERR_REFINEMENT_REMOVED			90
#define ERR_EXT_REFINEMENT_ADDED		91
#define ERR_EXT_REFINEMENT_REMOVED		92
#define ERR_MANDATORY_REMOVED			93
#define ERR_MANDATORY_ADDED			94
#define ERR_OPTIONAL_REMOVED			95
#define ERR_OPTIONAL_ADDED			96
#define ERR_MANDATORY_EXT_REMOVED		97
#define ERR_MANDATORY_EXT_ADDED			98
#define ERR_OPTIONAL_EXT_REMOVED		99
#define ERR_OPTIONAL_EXT_ADDED			100

static Error errors[] = {
    { 0, ERR_INTERNAL, "internal", 
      "internal error!!!", NULL },
    { 1, ERR_TYPE_REMOVED, "type-removed",
      "type `%s' has been deleted", NULL },
    { 5, ERR_TYPE_ADDED, "type-added",
      "type `%s' has been added", NULL },
    { 1, ERR_NODE_REMOVED, "node-removed",
      "%s `%s' has been deleted", NULL },
    { 5, ERR_NODE_ADDED, "node-added",
      "%s `%s' has been added", NULL },
    { 1, ERR_BASETYPE_CHANGED, "basetype-changed",
      "base type of `%s' changed", NULL },
    { 5, ERR_DECL_CHANGED, "decl-changed",
      "declaration changed for `%s'", NULL },
    { 5, ERR_LEGAL_STATUS_CHANGED, "status-change",
      "legal status change from `%s' to `%s' for `%s'", NULL },
    { 6, ERR_PREVIOUS_DEFINITION, "previous-definition",
      "previous definition of `%s'", NULL },
    { 2, ERR_STATUS_CHANGED, "status-change",
      "status change from `%s' to `%s' for `%s'", NULL },
    { 5, ERR_DESCR_ADDED, "description-added",
      "description added to `%s'", NULL },
    { 2, ERR_DESCR_REMOVED, "description-removed",
      "description removed from `%s'", NULL },
    { 5, ERR_DESCR_CHANGED, "description-changed",
      "description of %s `%s' changed", NULL },
    { 5, ERR_REF_ADDED, "ref-added",
      "reference added to `%s'", NULL },
    { 3, ERR_REF_REMOVED, "ref-removed",
      "reference removed from `%s'", NULL },
    { 5, ERR_REF_CHANGED, "ref-changed",
      "reference of `%s' changed", NULL },
    { 5, ERR_FORMAT_ADDED, "format-added",
      "format added to `%s'", NULL },
    { 3, ERR_FORMAT_REMOVED, "format-removed",
      "format removed from `%s'", NULL },
    { 5, ERR_FORMAT_CHANGED, "format-changed",
      "format of `%s' changed", NULL },
    { 5, ERR_UNITS_ADDED, "units-added",
      "units added to `%s'", NULL },
    { 3, ERR_UNITS_REMOVED, "units-removed",
      "units removed from `%s'", NULL },
    { 5, ERR_UNITS_CHANGED, "units-changed",
      "units of `%s' changed", NULL },
    { 5, ERR_ACCESS_ADDED, "access-added",
      "access added to `%s'", NULL },
    { 3, ERR_ACCESS_REMOVED, "access-removed",
      "access removed from `%s'", NULL },
    { 5, ERR_ACCESS_CHANGED, "access-changed",
      "access of `%s' changed from `%s' to `%s'", NULL },
    { 5, ERR_NAME_ADDED, "name-added",
      "name added to `%s'", NULL },
    { 3, ERR_NAME_REMOVED, "name-removed",
      "name removed from `%s'", NULL },
    { 5, ERR_NAME_CHANGED, "name-changed",
      "name changed from `%s' to `%s'", NULL },
    { 3, ERR_TO_IMPLICIT, "to-implicit",
      "implicit type for `%s' replaces type `%s'", NULL },
    { 5, ERR_FROM_IMPLICIT, "from-implicit",
      "type `%s' replaces implicit type for `%s'", NULL },
    { 3, ERR_RANGE_ADDED, "range-added",
      "range `%s' added to type used in `%s'", NULL },
    { 3, ERR_RANGE_REMOVED, "range-removed",
      "range `%s' removed from type used in `%s'", NULL },
    { 3, ERR_RANGE_CHANGED, "range-changed",
      "range of type used in `%s' changed from `%s' to `%s'", NULL }, 
    { 3, ERR_DEFVAL_ADDED, "defval-added",
      "default value added to `%s'", NULL },
    { 3, ERR_DEFVAL_REMOVED, "defval-removed",
      "default value removed from `%s'", NULL },
    { 3, ERR_DEFVAL_CHANGED, "defval-changed",
      "default value of `%s' changed", NULL },
    { 5, ERR_ORGA_ADDED, "organization-added",
      "organization added to `%s'", NULL },
    { 3, ERR_ORGA_REMOVED, "organization-removed",
      "organization removed from `%s'", NULL },
    { 5, ERR_ORGA_CHANGED, "organization-changed",
      "organization of `%s' changed", NULL },
    { 5, ERR_CONTACT_ADDED, "contact-added",
      "contact added to `%s'", NULL },
    { 3, ERR_CONTACT_REMOVED, "contact-removed",
      "contact removed from `%s'", NULL },
    { 5, ERR_CONTACT_CHANGED, "contact-changed",
      "contact of `%s' changed", NULL },
    { 3, ERR_SMIVERSION_CHANGED, "smi-version-changed",
      "SMI version changed", NULL },
    { 5, ERR_REVISION_ADDED, "revision-added",
      "revision `%s' added", NULL },
    { 3, ERR_REVISION_REMOVED, "revision-removed",
      "revision `%s' removed", NULL },
    { 5, ERR_REVISION_CHANGED, "revision-changed",
      "revision `%s' changed", NULL },
    { 3, ERR_LENGTH_CHANGED, "range-changed",
      "size of type used in `%s' changed from `%s' to `%s'", NULL },
    { 3, ERR_LENGTH_OF_TYPE_CHANGED, "range-changed",
      "size of type `%s' changed from `%s' to `%s'", NULL },
    { 3, ERR_LENGTH_ADDED, "range-added",
      "size `%s' added to type used in `%s'", NULL },
    { 2, ERR_MEMBER_ADDED, "member-added",
      "member `%s' added to group `%s'", NULL },
    { 2, ERR_MEMBER_REMOVED, "member-removed",
      "member `%s' removed from group `%s'", NULL },
    { 3, ERR_MEMBER_CHANGED, "member-changed",
      "member `%s' changed in group `%s'", NULL },
    { 3, ERR_OBJECT_ADDED, "object-added",
      "object `%s' added", NULL },
    { 2, ERR_OBJECT_REMOVED, "object-removed",
      "object `%s' removed", NULL },
    { 3, ERR_OBJECT_CHANGED, "object-changed",
      "object `%s' changed", NULL },
    { 5, ERR_NAMED_NUMBER_ADDED, "named-number-added",
      "named number `%s' added to type used in `%s'", NULL },
    { 2, ERR_NAMED_NUMBER_REMOVED, "named-number-removed",
      "named number `%s' removed from type used in `%s'", NULL },
    { 5, ERR_NAMED_NUMBER_CHANGED, "named-number-changed",
      "named number `%s' changed to `%s' at type used in `%s'", NULL },
    { 3, ERR_NAMED_BIT_ADDED_OLD_BYTE, "named-bit-added-old-byte",
      "named bit `%s' added without starting in a new byte in type used in `%s'", NULL },
     { 3, ERR_LENGTH_REMOVED, "range-removed",
      "size `%s' removed from type used in `%s'", NULL },
    { 2, ERR_NODEKIND_CHANGED, "nodekind-changed",
      "node kind of `%s' changed", NULL },
    { 2, ERR_INDEXKIND_CHANGED, "indexkind-changed",
      "changed kind of index from `%s' to `%s' in node `%s'", NULL },
    { 2, ERR_INDEX_CHANGED, "index-changed",
      "index of `%s' changed from %s to %s", NULL },
    { 5, ERR_TYPE_IS_AND_WAS, "type-is-and-was",
      "type changed from %s to %s", NULL },
    { 3, ERR_RANGE_OF_TYPE_CHANGED, "range-changed",
      "range of type `%s' changed from `%s' to `%s'", NULL },
    { 3, ERR_RANGE_OF_TYPE_ADDED, "range-added",
      "range `%s' added to type `%s'", NULL },
    { 3, ERR_RANGE_OF_TYPE_REMOVED, "range-removed",
      "range `%s' removed from type `%s'", NULL },
    { 6, ERR_TYPE_BASED_ON, "type-based-on",
      "type `%s' based on `%s'", NULL },
    { 2, ERR_INDEX_AUGMENT_CHANGED, "index-changed",
      "index of `%s' changed from augmenting `%s' to augmenting `%s'", NULL },
    { 2, ERR_NAMED_NUMBER_OF_TYPE_REMOVED, "named-number-removed",
      "named number `%s' removed from type `%s'", NULL },
    { 5, ERR_NAMED_NUMBER_TO_TYPE_ADDED, "named-number-added",
      "named number `%s' added to type `%s'", NULL },
    { 5, ERR_NAMED_NUMBER_OF_TYPE_CHANGED, "named-number-changed",
      "named number `%s' changed to `%s' in type `%s'", NULL },
    { 3, ERR_NAMED_BIT_OF_TYPE_ADDED_OLD_BYTE, "named-bit-added-old-byte",
      "named bit `%s' added without starting in a new byte in type `%s'", NULL },
    { 6, ERR_PREVIOUS_IMPLICIT_DEFINITION, "previous-definition",
      "previous implicit definition", NULL },
    { 2, ERR_STATUS_CHANGED_IMPLICIT, "status-change",
      "status change from `%s' to `%s' for implicit type", NULL },
    { 5, ERR_LEGAL_STATUS_CHANGED_IMPLICIT, "status-change",
      "legal status change from `%s' to `%s' for implicit type", NULL },
    { 3, ERR_LENGTH_OF_TYPE_ADDED, "range-added",
      "size `%s' added to type `%s'", NULL },
    { 3, ERR_LENGTH_OF_TYPE_REMOVED, "range-removed",
      "size `%s' removed from type `%s'", NULL },
    { 5, ERR_STATUS_ADDED, "status-added",
      "status added to `%s'", NULL },
    { 3, ERR_STATUS_REMOVED, "status-removed",
      "status removed from `%s'", NULL },
    { 2, ERR_MANDATORY_GROUP_ADDED, "mandatory-added",
      "mandatory group `%s' added to `%s'", NULL },
    { 2, ERR_MANDATORY_GROUP_REMOVED, "mandatory-removed",
      "mandatory group `%s' removed from `%s'", NULL },
    { 2, ERR_MANDATORY_EXT_GROUP_ADDED, "mandatory-added",
      "mandatory group `%s::%s' added to `%s'", NULL },
    { 2, ERR_MANDATORY_EXT_GROUP_REMOVED, "mandatory-removed",
      "mandatory group `%s::%s' removed from `%s'", NULL },
    { 2, ERR_OPTION_ADDED, "option-added",
      "optional group `%s' added to `%s'", NULL },
    { 2, ERR_OPTION_REMOVED, "option-removed",
      "optional group `%s' removed from `%s'", NULL },
    { 2, ERR_EXT_OPTION_ADDED, "option-added",
      "optional group `%s::%s' added to `%s'", NULL },
    { 2, ERR_EXT_OPTION_REMOVED, "option-removed",
      "optional group `%s::%s' removed from `%s'", NULL },
    { 5, ERR_REFINEMENT_ADDED, "refinement-added",
      "object refinement for `%s' added to `%s'", NULL },
    { 2, ERR_REFINEMENT_REMOVED, "refinement-removed",
      "object refinement for `%s' removed from `%s'", NULL },
    { 5, ERR_EXT_REFINEMENT_ADDED, "refinement-added",
      "object refinement for `%s::%s' added to `%s'", NULL },
    { 2, ERR_EXT_REFINEMENT_REMOVED, "refinement-removed",
      "object refinement for `%s::%s' removed from `%s'", NULL },
    { 3, ERR_MANDATORY_REMOVED, "mandatory-removed",
      "%s `%s' is mandatory under `%s' but not mandatory under `%s'", NULL },
    { 3, ERR_MANDATORY_ADDED, "mandatory-added",
      "%s `%s' is not mandatory under `%s' but mandatory under `%s'", NULL },
    { 3, ERR_OPTIONAL_REMOVED, "optional-removed",
      "%s `%s' is conditionally optional under `%s' but not under `%s'", NULL },
    { 3, ERR_OPTIONAL_ADDED, "optional-added",
      "%s `%s' is not conditionally optional under `%s' but under `%s'", NULL },
    { 3, ERR_MANDATORY_EXT_REMOVED, "mandatory-removed",
      "%s `%s::%s' is mandatory under `%s' but not mandatory under `%s'", NULL },
    { 3, ERR_MANDATORY_EXT_ADDED, "mandatory-added",
      "%s `%s::%s' is not mandatory under `%s' but mandatory under `%s'", NULL },
    { 3, ERR_OPTIONAL_EXT_REMOVED, "optional-removed",
      "%s `%s::%s' is conditionally optional under `%s' but not under `%s'", NULL },
    { 3, ERR_OPTIONAL_EXT_ADDED, "optional-added",
      "%s `%s::%s' is not conditionally optional under `%s' but under `%s'", NULL },
    { 0, 0, NULL, NULL }
};



static char *smiStringDecl(SmiDecl macro)
{
    return
        (macro == SMI_DECL_UNKNOWN)           ? "unknown construct" :
        (macro == SMI_DECL_IMPLICIT_TYPE)     ? "implicit construct" :
        (macro == SMI_DECL_TYPEASSIGNMENT)    ? "type assignment" :
        (macro == SMI_DECL_IMPL_SEQUENCEOF)   ? "implicit sequence-of construct" :
        (macro == SMI_DECL_VALUEASSIGNMENT)   ? "value assignment" :
        (macro == SMI_DECL_OBJECTTYPE)        ? "object definition" :
        (macro == SMI_DECL_OBJECTIDENTITY)    ? "object identity definition" :
        (macro == SMI_DECL_MODULEIDENTITY)    ? "module identity definition" :
        (macro == SMI_DECL_NOTIFICATIONTYPE)  ? "notification definition" :
        (macro == SMI_DECL_TRAPTYPE)          ? "trap definition" :
        (macro == SMI_DECL_OBJECTGROUP)       ? "object group definition" :
        (macro == SMI_DECL_NOTIFICATIONGROUP) ? "notification group definition" :
        (macro == SMI_DECL_MODULECOMPLIANCE)  ? "module compliance definition" :
        (macro == SMI_DECL_AGENTCAPABILITIES) ? "agent capabilities definition" :
        (macro == SMI_DECL_TEXTUALCONVENTION) ? "textual convention definition" :
        (macro == SMI_DECL_MACRO)             ? "macro definition" :
        (macro == SMI_DECL_COMPL_GROUP)       ? "optional group" :
        (macro == SMI_DECL_COMPL_OBJECT)      ? "object refinement" :
        (macro == SMI_DECL_MODULE)	      ? "module" :
        (macro == SMI_DECL_TYPEDEF)	      ? "typedef" :
        (macro == SMI_DECL_NODE)	      ? "node" :
        (macro == SMI_DECL_SCALAR)	      ? "scalar" :
        (macro == SMI_DECL_TABLE)	      ? "table" :
        (macro == SMI_DECL_ROW)		      ? "row" :
        (macro == SMI_DECL_COLUMN)	      ? "column" :
        (macro == SMI_DECL_NOTIFICATION)      ? "notification" :
        (macro == SMI_DECL_GROUP)	      ? "group" :
        (macro == SMI_DECL_COMPLIANCE)	      ? "compliance" :
                                                "<UNDEFINED>";
}



static void
setErrorSeverity(char *pattern, int severity)
{
    int i;
    
    for (i = 0; errors[i].fmt; i++) {
	if (strstr(errors[i].tag, pattern) == errors[i].tag) {
	    errors[i].level = severity;
	}
    }
}



static void
printError(SmiModule *smiModule, int id, int line, va_list ap)
{
    int i;

    /*
     * Search for the tag instead of just using the id as an index so
     * that we do not run into trouble if the id is bogus.
     */

    for (i = 0; errors[i].fmt; i++) {
	if (errors[i].id == id) break;
    }
    if (! errors[i].fmt) {
	i = 0;		/* assumes that 0 is the internal error */
    }
    
    if (errors[i].level <= errorLevel) {
	fprintf(stdout, "%s", smiModule->path);

    	if (line >= 0) {
	    fprintf(stdout, ":%d", line);
	}
	fprintf(stdout, " ");
	if (sFlag) {
	    fprintf(stdout, "[%d] ", errors[i].level);
	}
	if (mFlag) {
	    fprintf(stdout, "{%s} ", errors[i].tag);
	}
	switch (errors[i].level) {
	case 4:
	case 5:
	    fprintf(stdout, "warning: ");
	    break;
	case 6:	
	    fprintf(stdout, "info: ");
	    break;
	}
	vfprintf(stdout, errors[i].fmt, ap);
	fprintf(stdout, "\n");
    }
}



static void
printErrorAtLine(SmiModule *smiModule, int id, int line, ...)
{
    va_list ap;

    va_start(ap, line);
    printError(smiModule, id, line, ap);
    va_end(ap);
}



static char*
getStringTime(time_t t)
{
    static char   s[27];
    struct tm	  *tm;

    tm = gmtime(&t);
    sprintf(s, "%04d-%02d-%02d %02d:%02d",
	    tm->tm_year + 1900, tm->tm_mon + 1, tm->tm_mday,
	    tm->tm_hour, tm->tm_min);
    return s;
}



static char*
getStringNodekind(SmiNodekind nodekind)
{
    return
	(nodekind == SMI_NODEKIND_UNKNOWN)      ? "unknown" :
	(nodekind == SMI_NODEKIND_NODE)         ? "node" :
	(nodekind == SMI_NODEKIND_SCALAR)       ? "scalar" :
	(nodekind == SMI_NODEKIND_TABLE)        ? "table" :
	(nodekind == SMI_NODEKIND_ROW)          ? "row" :
	(nodekind == SMI_NODEKIND_COLUMN)       ? "column" :
	(nodekind == SMI_NODEKIND_NOTIFICATION) ? "notification" :
	(nodekind == SMI_NODEKIND_GROUP)        ? "group" :
	(nodekind == SMI_NODEKIND_COMPLIANCE)   ? "compliance" :
	(nodekind == SMI_NODEKIND_CAPABILITIES) ? "capabilities" :
                                                  "<unknown>";
}



static int
diffStrings(const char *s1, const char *s2)
{
    int i, j;

    for (i = 0, j = 0; s1[i] && s2[j]; i++, j++) {
	while (s1[i] && isspace((int) s1[i])) i++;
	while (s2[j] && isspace((int) s2[j])) j++;
	if (! s1[i] || ! s2[j]) break;
	if (s1[i] != s2[j]) {
	    return 1;
	}
    }
    return (s1[i] != s2[j]);
}



static int
checkName(SmiModule *oldModule, int oldLine,
	    SmiModule *newModule, int newLine,
	    char *oldName, char *newName)
{
    int code = 0;
    
    if (!oldName && newName) {
	printErrorAtLine(newModule, ERR_NAME_ADDED,
			 newLine, newName);
    }

    if (oldName && !newName) {
	printErrorAtLine(oldModule, ERR_NAME_REMOVED,
			 oldLine, oldName);
    }
    
    if (oldName && newName && strcmp(oldName, newName) != 0) {
	printErrorAtLine(newModule, ERR_NAME_CHANGED,
			 newLine, oldName, newName);
	code |= CODE_SHOW_PREVIOUS;
    }

    return code;
}



static int
checkDecl(SmiModule *oldModule, int oldLine,
	  SmiModule *newModule, int newLine,
	  char *name, SmiDecl oldDecl, SmiDecl newDecl)
{
    int code = 0;

    if (oldDecl != newDecl) {
	printErrorAtLine(newModule, ERR_DECL_CHANGED,
			 newLine, name);
        code |= CODE_SHOW_PREVIOUS;
    }
    return code;
}



static char*
getStringStatus(SmiStatus status)
{
    char *statStr;
    
    switch( status ) {
    case SMI_STATUS_CURRENT:
	statStr = "current";
	break;
    case SMI_STATUS_DEPRECATED:
	statStr = "deprecated";
	break;
    case SMI_STATUS_OBSOLETE:
	statStr = "obsolete";
	break;
    case SMI_STATUS_MANDATORY:
	statStr = "mandatory";
	break;
    case SMI_STATUS_OPTIONAL:
	statStr = "optional";
	break;
    case SMI_STATUS_UNKNOWN:
    default:
	statStr = "unknown";
	break;
    }
    return statStr;
}



static int
checkStatus(SmiModule *oldModule, int oldLine,
	    SmiModule *newModule, int newLine,
	    char *name, SmiStatus oldStatus, SmiStatus newStatus)
{
    int code = 0;

    if (oldStatus == newStatus) {
	return code;
    }
    
    if (oldStatus == SMI_STATUS_UNKNOWN) {
	printErrorAtLine(newModule, ERR_STATUS_ADDED,
			 newLine, name);
    } else if (newStatus == SMI_STATUS_UNKNOWN) {
	printErrorAtLine(newModule, ERR_STATUS_REMOVED,
			 newLine, name);

    } else if (((oldStatus == SMI_STATUS_CURRENT
	  && (newStatus == SMI_STATUS_DEPRECATED
	      || newStatus == SMI_STATUS_OBSOLETE)))
	|| ((oldStatus == SMI_STATUS_DEPRECATED
	     && newStatus == SMI_STATUS_OBSOLETE))) {
	if (name) {
	    printErrorAtLine(newModule, ERR_LEGAL_STATUS_CHANGED, newLine,
			     getStringStatus(oldStatus),
			     getStringStatus(newStatus),
			     name);
	    code |= CODE_SHOW_PREVIOUS;
	} else {
	    printErrorAtLine(newModule, ERR_LEGAL_STATUS_CHANGED_IMPLICIT, newLine,
			     getStringStatus(oldStatus),
			     getStringStatus(newStatus));
	    code |= CODE_SHOW_PREVIOUS_IMPLICIT;
	}
    } else {
	if (name) {
	    printErrorAtLine(newModule, ERR_STATUS_CHANGED, newLine,
			     getStringStatus(oldStatus),
			     getStringStatus(newStatus),
			     name);
	    code |= CODE_SHOW_PREVIOUS;
	} else {
	    printErrorAtLine(newModule, ERR_STATUS_CHANGED_IMPLICIT,
			     newLine,
			     getStringStatus(oldStatus),
			     getStringStatus(newStatus));
	    code |= CODE_SHOW_PREVIOUS_IMPLICIT;
	}
    }

    return code;
}



static char*
getStringAccess( SmiAccess smiAccess )
{
    switch( smiAccess ) {
    case SMI_ACCESS_NOT_IMPLEMENTED: return "not-implemented";
    case SMI_ACCESS_NOT_ACCESSIBLE : return "not-accessible";
    case SMI_ACCESS_NOTIFY         : return "notify";
    case SMI_ACCESS_READ_ONLY      : return "read-only";
    case SMI_ACCESS_READ_WRITE     : return "read-write";
    case SMI_ACCESS_UNKNOWN:
    default: return "unknown";
    }
}



static int
checkAccess(SmiModule *oldModule, int oldLine,
	    SmiModule *newModule, int newLine,
	    char *name, SmiAccess oldAccess, SmiAccess newAccess)
{
    int code = 0;
    
    if (oldAccess == newAccess) {
	return code;
    }

    if (oldAccess == SMI_ACCESS_UNKNOWN) {
	printErrorAtLine(newModule, ERR_ACCESS_ADDED,
			 newLine, name);
    } else if (newAccess == SMI_ACCESS_UNKNOWN) {
	printErrorAtLine(newModule, ERR_ACCESS_REMOVED,
			 newLine, name);
    } else {
	printErrorAtLine(newModule, ERR_ACCESS_CHANGED,
			 newLine, name,
			 getStringAccess( oldAccess ),
			 getStringAccess( newAccess ));
	code |= CODE_SHOW_PREVIOUS;
    }

    return code;
}



static int
checkDescription(SmiModule *oldModule, int oldLine,
		 SmiModule *newModule, int newLine,
		 char *name, SmiDecl decl, char *oldDescr, char *newDescr)
{
    int code = 0;
    
    if (!oldDescr && newDescr) {
	printErrorAtLine(newModule, ERR_DESCR_ADDED,
			 newLine, name);
    }

    if (oldDescr && !newDescr) {
	printErrorAtLine(newModule, ERR_DESCR_REMOVED,
			 newLine, name);
	code |= CODE_SHOW_PREVIOUS;
    }

    if (oldDescr && newDescr && diffStrings(oldDescr, newDescr)) {
	printErrorAtLine(newModule, ERR_DESCR_CHANGED,
			 newLine, smiStringDecl(decl), name);
	code |= CODE_SHOW_PREVIOUS;
    }

    return code;
}



static int
checkReference(SmiModule *oldModule, int oldLine,
	       SmiModule *newModule, int newLine,
	       char *name, char *oldRef, char *newRef)
{
    int code = 0;
    
    if (!oldRef && newRef) {
	printErrorAtLine(newModule, ERR_REF_ADDED,
			 newLine, name);
    }

    if (oldRef && !newRef) {
	printErrorAtLine(oldModule, ERR_REF_REMOVED,
			 oldLine, name);
    }
    
    if (oldRef && newRef && diffStrings(oldRef, newRef) != 0) {
	printErrorAtLine(newModule, ERR_REF_CHANGED,
			 newLine, name);
	code |= CODE_SHOW_PREVIOUS;
    }

    return code;
}



static int
checkFormat(SmiModule *oldModule, int oldLine,
	    SmiModule *newModule, int newLine,
	    char *name, char *oldFormat, char *newFormat)
{
    int code = 0;

    if (!oldFormat && newFormat) {
	printErrorAtLine(newModule, ERR_FORMAT_ADDED,
			 newLine, name);
    }

    if (oldFormat && !newFormat) {
	printErrorAtLine(oldModule, ERR_FORMAT_REMOVED,
			 oldLine, name);
    }
    
    if (oldFormat && newFormat && strcmp(oldFormat, newFormat) != 0) {
	printErrorAtLine(newModule, ERR_FORMAT_CHANGED,
			 newLine, name);
	code |= CODE_SHOW_PREVIOUS;
    }

    return code;
}



static int
checkUnits(SmiModule *oldModule, int oldLine,
	   SmiModule *newModule, int newLine,
	   char *name, char *oldUnits, char *newUnits)
{
    int code = 0;
    
    if (!oldUnits && newUnits) {
	printErrorAtLine(newModule, ERR_UNITS_ADDED,
			 newLine, name);
    }

    if (oldUnits && !newUnits) {
	printErrorAtLine(oldModule, ERR_UNITS_REMOVED,
			 oldLine, name);
    }
    
    if (oldUnits && newUnits && strcmp(oldUnits, newUnits) != 0) {
	printErrorAtLine(newModule, ERR_UNITS_CHANGED,
			 newLine, name);
	code |= CODE_SHOW_PREVIOUS;
    }

    return code;
}



static SmiType*
findTypeWithRange(SmiType *smiType)
{
    SmiType *iterType;

    for (iterType = smiType; iterType; iterType = smiGetParentType(iterType)) {
	if (smiGetFirstRange(iterType)) {
	    return iterType;
	}
    }
    return NULL;
}


/* This function assumes that the compared values have the same basetype.
 * If the basetype is different, no comparison is done
 * and '0' will be returned. Same for SMI_BASETYPE_UNKNOWN.
 */
static int 
cmpSmiValues( SmiValue a, SmiValue b )
{
    unsigned int i;
    int changed = 0;

    switch (a.basetype) {
    case SMI_BASETYPE_INTEGER32:
    case SMI_BASETYPE_ENUM :
	changed = (a.value.integer32 != b.value.integer32);
	break;
    case SMI_BASETYPE_UNSIGNED32:
	changed = (a.value.unsigned32 != b.value.unsigned32);
	break;
    case SMI_BASETYPE_INTEGER64:
	changed = (a.value.integer64 != b.value.integer64);
	break;
    case SMI_BASETYPE_UNSIGNED64:
	changed = (a.value.unsigned64 != b.value.unsigned64);
	break;
    case SMI_BASETYPE_FLOAT32:
	changed = (a.value.float32 != b.value.float32);
	break;
    case SMI_BASETYPE_FLOAT64:
	changed = (a.value.float64 != b.value.float64);
	break;
    case SMI_BASETYPE_FLOAT128:
	changed = (a.value.float128 != b.value.float128);
	break;
    case SMI_BASETYPE_OCTETSTRING:
    case SMI_BASETYPE_BITS:
	changed = (a.len != b.len)
	    || (memcmp(a.value.ptr, b.value.ptr, a.len) != 0);
	break;
    case SMI_BASETYPE_OBJECTIDENTIFIER:
	changed = (a.len != b.len);
	for (i = 0; !changed && i < a.len; i++) {
	    changed = (a.value.oid[i] - b.value.oid[i]);
	}
	break;
    case SMI_BASETYPE_UNKNOWN:
    case SMI_BASETYPE_POINTER:
	/* this should not occur */
	break;
    }
    
    return changed;
}

#if 0
static char*
getTypeName(SmiType *smiType, SmiModule *smiModule)
{
    char* name;
    SmiModule * tm;
    
    if( ! smiType ) {
	return 0;
    }

    if( smiType->name ) {
	tm = smiGetTypeModule( smiType );
	if( smiModule != tm ) {
	    if( smiModule->name ) {
		name = (char *)malloc( strlen( smiType->name ) +
				       strlen( tm->name ) + 5 );
		sprintf( name, "%s::%s",
			 tm->name, smiType->name );
	    }
	    else {
		name = strdup( smiType->name );
	    }
	}
	else {
	    name = strdup( smiType->name );
	}
    }
    else {
	name = NULL;
    }
    return name;
}

static void
iterateTypeImports(char *typeName,
		   SmiType *smiType, SmiType *smiTwR,
		   int line,
		   SmiModule *smiModule)
{
    SmiType *iterType, *oldIterType;
    char *iterTypeName, *oldIterTypeName = strdup( typeName );

    iterType =  smiType;
    while( 1 ) {
	iterType = smiGetParentType( iterType );
	iterTypeName = getTypeName( iterType, smiModule );	
	if( (!iterType) || !iterTypeName ) {
	    return;
	}
	printErrorAtLine( smiGetTypeModule( smiType ),
			  ERR_TYPE_BASED_ON,
			  line,
			  oldIterTypeName,
			  iterTypeName );
	free( oldIterTypeName );
	oldIterTypeName = iterTypeName;
	oldIterType = iterType;
    }
}
#endif

static char *getValueString(SmiValue *valuePtr, SmiType *typePtr)
{
    static char    s[1024];
    char           ss[9];
    int		   n;
    unsigned int   i;
    SmiNamedNumber *nn;
    SmiNode        *nodePtr;
    
    s[0] = 0;
    
    switch (valuePtr->basetype) {
    case SMI_BASETYPE_UNSIGNED32:
	sprintf(s, "%lu", valuePtr->value.unsigned32);
	break;
    case SMI_BASETYPE_INTEGER32:
	sprintf(s, "%ld", valuePtr->value.integer32);
	break;
    case SMI_BASETYPE_UNSIGNED64:
	sprintf(s, UINT64_FORMAT, valuePtr->value.unsigned64);
	break;
    case SMI_BASETYPE_INTEGER64:
	sprintf(s, INT64_FORMAT, valuePtr->value.integer64);
	break;
    case SMI_BASETYPE_FLOAT32:
    case SMI_BASETYPE_FLOAT64:
    case SMI_BASETYPE_FLOAT128:
	break;
    case SMI_BASETYPE_ENUM:
	for (nn = smiGetFirstNamedNumber(typePtr); nn;
	     nn = smiGetNextNamedNumber(nn)) {
	    if (nn->value.value.unsigned32 == valuePtr->value.unsigned32)
		break;
	}
	if (nn) {
	    sprintf(s, "%s", nn->name);
	} else {
	    sprintf(s, "%ld", valuePtr->value.integer32);
	}
	break;
    case SMI_BASETYPE_OCTETSTRING:
	for (i = 0; i < valuePtr->len; i++) {
	    if (!isprint((int)valuePtr->value.ptr[i])) break;
	}
	if (i == valuePtr->len) {
	    sprintf(s, "\"%s\"", valuePtr->value.ptr);
	} else {
            sprintf(s, "'%*s'H", 2 * valuePtr->len, " ");
            for (i=0; i < valuePtr->len; i++) {
                sprintf(ss, "%02x", valuePtr->value.ptr[i]);
                strncpy(&s[1+2*i], ss, 2);
            }
	}
	break;
    case SMI_BASETYPE_BITS:
	
	sprintf(s, "{");
	for (i = 0, n = 0; i < valuePtr->len * 8; i++) {
	    if (valuePtr->value.ptr[i/8] & (1 << (7-(i%8)))) {
		for (nn = smiGetFirstNamedNumber(typePtr); nn;
		     nn = smiGetNextNamedNumber(nn)) {
		    if (nn->value.value.unsigned32 == i)
			break;
		}
		if (nn) {
		    if (n)
			sprintf(&s[strlen(s)], ", ");
		    n++;
		    sprintf(&s[strlen(s)], "%s", nn->name);
		}
	    }
	}
	sprintf(&s[strlen(s)], "}");
	break;
    case SMI_BASETYPE_UNKNOWN:
    case SMI_BASETYPE_POINTER:
	break;
    case SMI_BASETYPE_OBJECTIDENTIFIER:
	nodePtr = smiGetNodeByOID(valuePtr->len, valuePtr->value.oid);
	if (nodePtr) {
	    sprintf(s, "%s", nodePtr->name);
	} else {
	    strcpy(s, "{");
	    for (i=0; i < valuePtr->len; i++) {
		if (i) strcat(s, " ");
		sprintf(&s[strlen(s)], "%u", valuePtr->value.oid[i]);
	    }
	    strcat(s, "}");
	}
	break;
    }

    return s;
}

static char*
getStringSubrange(SmiRange *range, SmiType *smiType)
{
    char *minStr, *maxStr, *str;
    minStr = strdup( getValueString(&range->minValue, smiType) );
    maxStr = strdup( getValueString(&range->maxValue, smiType) );
    if (memcmp(&range->minValue, &range->maxValue,
	       sizeof(SmiValue))) {
	str = malloc( strlen( minStr ) + strlen( maxStr ) + 3 );
	if( !str ) {
	    return NULL;
	}
	sprintf(str, "%s..%s", minStr, maxStr);
    } else {
	str = strdup( minStr );
    }
    
    return str;
}


static char*
getStringRange(SmiType *smiType)
{
    SmiRange *range;
    int i;
    char *str, *subRange;

    str = NULL;
    for(i = 0, range = smiGetFirstRange(smiType);
	range; i++, range = smiGetNextRange(range)) {
	
	if (i) {
	    str = realloc( str, strlen( str ) +2 );
	    if( str ) {
		sprintf(str, "%s|", str);
	    }
	    
	}
	else {
	    str = strdup("(");
	}
	
	subRange = getStringSubrange( range, smiType );
	if( !subRange ) {
	    return NULL;
	}
	str = realloc( str, strlen( str ) + strlen( subRange ) + 1 );
	if( !str ) {
	    return NULL;
	}
	sprintf( str, "%s%s", str, subRange );
	
    }
    str = realloc( str, strlen( str ) + 2 );
    if( str ) {
	sprintf(str, "%s)", str);
    }
    return str;
}

static void
printRangeChangeError( SmiType *oldTwR, SmiType *newTwR,
		       SmiModule *newModule, char *name )
{
    char *strOldRange, *strNewRange;
    int error, errorOT;
    if( newTwR->basetype == SMI_BASETYPE_OCTETSTRING ) {
	error =  ERR_LENGTH_CHANGED;
	errorOT = ERR_LENGTH_OF_TYPE_CHANGED;
    }
    else {
	error = ERR_RANGE_CHANGED;
	errorOT = ERR_RANGE_OF_TYPE_CHANGED;
    }
    strOldRange = getStringRange( oldTwR );
    strNewRange = getStringRange( newTwR );
    if( name ) {
	printErrorAtLine(newModule,
			 error,
			 smiGetTypeLine( newTwR ),
			 name, strOldRange, strNewRange );
    }
    else {
	printErrorAtLine(newModule,
			 errorOT,
			 smiGetTypeLine( newTwR ),
			 oldTwR->name, strOldRange, strNewRange );
    }
    free( strOldRange );
    free( strNewRange );
}

static void
checkRanges(SmiModule *oldModule, int oldLine, 
	    SmiModule *newModule, int newLine,
	    char *name,
	    SmiType *oldType, SmiType *newType)
{
    SmiType *oldTwR, *newTwR; /* parent types with ranges */
   
    oldTwR = findTypeWithRange(oldType);
    newTwR = findTypeWithRange(newType);
    
    if (!oldTwR && newTwR) {
	char *strRange;
	int error, errorOT;

	strRange = getStringRange( newTwR );
	if( newTwR->basetype == SMI_BASETYPE_OCTETSTRING ) {
	    error = ERR_LENGTH_ADDED;
	    errorOT = ERR_LENGTH_OF_TYPE_ADDED;
	}
	else {
	    error = ERR_RANGE_ADDED;
	    errorOT = ERR_RANGE_OF_TYPE_ADDED;
	}
	if( name ) {
	    printErrorAtLine(newModule, error,
			     newLine, strRange, name);
	}
	else {
	    printErrorAtLine( newModule, errorOT,
			      newLine, strRange, newTwR->name );
	}
	
	free( strRange );
	return;
    }
    
    if (oldTwR && !newTwR) {
	char *strRange;
	int error, errorOT;
	
	strRange = getStringRange( oldTwR );
	if( oldTwR->basetype == SMI_BASETYPE_OCTETSTRING ) {
	    error = ERR_LENGTH_REMOVED;
	    errorOT = ERR_LENGTH_OF_TYPE_REMOVED;
	}
	else {
	    error = ERR_RANGE_REMOVED;
	    errorOT = ERR_RANGE_OF_TYPE_REMOVED;
	}
	if( name ) {
	    printErrorAtLine( newModule, error,
			      newLine, strRange, name);
	}
	else {
	    printErrorAtLine( newModule, errorOT,
			      newLine, strRange, oldTwR->name );
	}
	free( strRange );

	if( oldTwR == oldType ) {
	    
	    printErrorAtLine(oldModule, ERR_PREVIOUS_DEFINITION,
			     oldLine, name);
	}
	else {
	    SmiModule *modTwR;
	    int line;
	    
	    modTwR = smiGetTypeModule( oldTwR );
	    line = smiGetTypeLine( oldTwR );
	    
	    printErrorAtLine( modTwR, ERR_PREVIOUS_DEFINITION,
			      line, name );

	}
	return;
    }
    
    if (oldTwR && newTwR) {
	
	SmiRange *oldRange, *newRange;
	oldRange = smiGetFirstRange(oldTwR);
	newRange = smiGetFirstRange(newTwR);

	while( oldRange || newRange ) {

	    if( oldRange && newRange ) {
		
		if(cmpSmiValues(oldRange->minValue, newRange->minValue) ||
		   cmpSmiValues(oldRange->maxValue, newRange->maxValue)) {
		    printRangeChangeError( oldTwR, newTwR, newModule, name );
		    return;
		}
	    }
	    
	    else if (oldRange){
		printRangeChangeError( oldTwR, newTwR, newModule, name );
		return;
	    }
	    
	    else if( newRange ) {
		printRangeChangeError( oldTwR, newTwR, newModule, name );
		return;
	    }
	    
	    oldRange = smiGetNextRange( oldRange );
	    newRange = smiGetNextRange( newRange );
	}
    }
}



static void
checkDefVal(SmiModule *oldModule, int oldLine,
	    SmiModule *newModule, int newLine,
	    char *name,
	    SmiValue oldVal, SmiValue newVal)
{
    if ((oldVal.basetype != SMI_BASETYPE_UNKNOWN) && 
	(newVal.basetype == SMI_BASETYPE_UNKNOWN)) {
	printErrorAtLine(newModule, ERR_DEFVAL_REMOVED, newLine, name);
	printErrorAtLine(oldModule, ERR_PREVIOUS_DEFINITION, oldLine, name);
	return;
    }

    if ((oldVal.basetype == SMI_BASETYPE_UNKNOWN) && 
	(newVal.basetype != SMI_BASETYPE_UNKNOWN)) {
	printErrorAtLine(newModule, ERR_DEFVAL_ADDED, newLine, name);
	return;
    }

#if 0 /* changed base type is reported, anyway. */
    if (oldVal.basetype != newVal.basetype) {
	printErrorAtLine(newModule, ERR_DEFVAL_CHANGED, newLine, name);
	printErrorAtLine(oldModule, ERR_PREVIOUS_DEFINITION, oldLine, name);
	return;
    }
#endif
    
    if (cmpSmiValues(oldVal, newVal)) {
	printErrorAtLine(newModule, ERR_DEFVAL_CHANGED, newLine,name);
	printErrorAtLine(oldModule, ERR_PREVIOUS_DEFINITION, oldLine, name);
    }
}



static void
checkNamedNumbers(SmiModule *oldModule, int oldLine,
		  SmiModule *newModule, int newLine,
		  char *name, SmiNode *smiNode,
		  SmiType *oldType, SmiType *newType)
{
    SmiNamedNumber *oldNN, *newNN;

    oldNN = smiGetFirstNamedNumber( oldType );
    newNN = smiGetFirstNamedNumber( newType );

    while( oldNN || newNN ) {
	if( oldNN && !newNN ) {
	    if( smiNode ) {
		printErrorAtLine(newModule, ERR_NAMED_NUMBER_REMOVED, newLine,
				 oldNN->name, smiNode->name);
	    }
	    else {
		printErrorAtLine(newModule, ERR_NAMED_NUMBER_OF_TYPE_REMOVED,
				 newLine, oldNN->name, name);
	    }
	    oldNN = smiGetNextNamedNumber( oldNN );
	}
	else if( !oldNN && newNN ) {
	    /* check if new byte has been started (bits only) */
	    if( newType->basetype == SMI_BASETYPE_BITS ) {
		SmiNamedNumber *veryOldNN = NULL, *iterNN;

		/* find largest old named number */
		for( iterNN = smiGetFirstNamedNumber( oldType );
		     iterNN; iterNN = smiGetNextNamedNumber( iterNN ) ) {
		    veryOldNN = iterNN;
		}
		
		if( veryOldNN ) {
		    /* we assume that we have bits, and the named numbers
		       of bits are stored in NN->value.value.unsigned32 */
		    if( newNN->value.value.unsigned32 / 8 <=
			veryOldNN->value.value.unsigned32 / 8 ) {
			if( smiNode ) {
			    printErrorAtLine( newModule,
					      ERR_NAMED_BIT_ADDED_OLD_BYTE,
					      newLine, newNN->name,
					      smiNode->name );
			}
			else {
			    printErrorAtLine( newModule,
					      ERR_NAMED_BIT_OF_TYPE_ADDED_OLD_BYTE,
					      newLine, newNN->name, name );
			}
		    }
		    else {
			if( smiNode ){
			    printErrorAtLine(newModule, ERR_NAMED_NUMBER_ADDED,
					     newLine, newNN->name,
					     smiNode->name);
			}
			else {
			    printErrorAtLine(newModule,
					     ERR_NAMED_NUMBER_TO_TYPE_ADDED,
					     newLine, newNN->name, name);
			}
		    }
		}
		else {
		    if( smiNode ) {
			printErrorAtLine(newModule, ERR_NAMED_NUMBER_ADDED,
					 newLine, newNN->name, smiNode->name);
		    }
		    else {
			printErrorAtLine(newModule,
					 ERR_NAMED_NUMBER_TO_TYPE_ADDED,
					 newLine, newNN->name, name);
		    }
		}
	    }
	    else {
		if( smiNode ) {
		    printErrorAtLine(newModule, ERR_NAMED_NUMBER_ADDED,
				     newLine, newNN->name, smiNode->name);
		}
		else {
		    printErrorAtLine(newModule, ERR_NAMED_NUMBER_TO_TYPE_ADDED,
				     newLine, newNN->name, name);
		}
	    }
	    newNN = smiGetNextNamedNumber( newNN );
	}
	else if( oldNN && newNN ) {
	    switch( oldType->basetype ) {
	    case SMI_BASETYPE_BITS:
		/* we assume that we have bits, and the named numbers
		   of bits are stored in NN->value.value.unsigned32 */
		if( oldNN->value.value.unsigned32 <
		    newNN->value.value.unsigned32 ) {
		    if( smiNode ) {
			printErrorAtLine( newModule, ERR_NAMED_NUMBER_REMOVED,
					  newLine,
					  oldNN->name,
					  smiNode->name );
		    }
		    else {
			printErrorAtLine( newModule,
					  ERR_NAMED_NUMBER_OF_TYPE_REMOVED,
					  newLine,
					  oldNN->name, name );
		    }
		    oldNN = smiGetNextNamedNumber( oldNN );
		}
		else if( oldNN->value.value.unsigned32 >
			 newNN->value.value.unsigned32 ) {
		    if( smiNode ) {
			printErrorAtLine( newModule, ERR_NAMED_NUMBER_ADDED,
					  newLine, newNN->name,
					  smiNode->name );
		    }
		    else {
			printErrorAtLine( newModule,
					  ERR_NAMED_NUMBER_TO_TYPE_ADDED,
					  newLine, newNN->name, name );
		    }
		    newNN = smiGetNextNamedNumber( newNN );
		}
		else {
		    if( strcmp( oldNN->name, newNN->name ) ) {
			if( smiNode ) {
			    printErrorAtLine( newModule,
					      ERR_NAMED_NUMBER_CHANGED,
					      newLine,
					      oldNN->name, newNN->name,
					      smiNode->name );
			}
			else {
			    printErrorAtLine( newModule,
					      ERR_NAMED_NUMBER_OF_TYPE_CHANGED,
					      newLine,
					      oldNN->name, newNN->name, name );
			}
		    }
		    oldNN = smiGetNextNamedNumber( oldNN );
		    newNN = smiGetNextNamedNumber( newNN );
		}
		break;
	    case SMI_BASETYPE_ENUM:
		/* we assume that we have an enumeration, and the named numbers
		   of an enumeration are stored in NN->value.value.integer32 */
		if( oldNN->value.value.integer32 <
		    newNN->value.value.integer32 ) {
		    if( smiNode ) {
			printErrorAtLine( newModule, ERR_NAMED_NUMBER_REMOVED,
					  newLine,oldNN->name,
					  smiNode->name );
		    }
		    else {
			printErrorAtLine( newModule,
					  ERR_NAMED_NUMBER_OF_TYPE_REMOVED,
					  newLine, oldNN->name, name );
		    }
		    oldNN = smiGetNextNamedNumber( oldNN );
		}
		else if( oldNN->value.value.integer32 >
			 newNN->value.value.integer32 ) {
		    if( smiNode ) {
			printErrorAtLine( newModule, ERR_NAMED_NUMBER_ADDED,
					  newLine ,newNN->name,
					  smiNode->name );
		    }
		    else {
			printErrorAtLine( newModule,
					  ERR_NAMED_NUMBER_TO_TYPE_ADDED,
					  newLine, newNN->name, name );
		    }
		    newNN = smiGetNextNamedNumber( newNN );
		}
		else {
		    if( strcmp( oldNN->name, newNN->name ) ) {
			if( smiNode ) {
			    printErrorAtLine( newModule,
					      ERR_NAMED_NUMBER_CHANGED,
					      newLine, oldNN->name,
					      newNN->name, smiNode->name );
			}
			else {
			    printErrorAtLine( newModule,
					      ERR_NAMED_NUMBER_OF_TYPE_CHANGED,
					      newLine, oldNN->name,
					      newNN->name, name );
			}
		    }
		    oldNN = smiGetNextNamedNumber( oldNN );
		    newNN = smiGetNextNamedNumber( newNN );
		}
		break;
	    default:
		break;
	    }
	}
    }
}


static void
checkTypeCompatibility(SmiModule *oldModule, SmiNode *oldNode,
		       SmiType *oldType,
		       SmiModule *newModule, int newLine,
		       SmiType *newType)
{
    int oldLine;
    char *oldName;

    if ((!oldType) && (!newType)) return;

    if (!oldType) {
	if (newType->name) {
	    printErrorAtLine(newModule, ERR_TYPE_ADDED,
			     smiGetTypeLine(newType));
	}
	return;
    }
    
    if (!newType) {
	if (oldType->name) {
	    printErrorAtLine(oldModule, ERR_TYPE_REMOVED,
			     smiGetTypeLine(oldType));
	}
	return;
    }
    
    if (oldType->basetype != newType->basetype) {
	if( newType->name ) {
	    printErrorAtLine(newModule, ERR_BASETYPE_CHANGED,
			     smiGetTypeLine(newType), newType->name);
	}
	else {
	    printErrorAtLine(newModule, ERR_BASETYPE_CHANGED,
			     smiGetTypeLine(newType), "implicit type");
	}
	if( oldType->name ) {
	    printErrorAtLine(oldModule, ERR_PREVIOUS_DEFINITION,
			     smiGetTypeLine(oldType), oldType->name);
	}
	else {
	    printErrorAtLine(oldModule, ERR_PREVIOUS_DEFINITION,
			     smiGetTypeLine(oldType), "implicit type" );
	}
    }

    oldLine = oldNode ? smiGetNodeLine( oldNode ) : smiGetTypeLine( oldType );
    checkNamedNumbers(oldModule,
		      oldLine,
		      newModule,
		      newLine > 0 ? newLine : smiGetTypeLine( newType ),
		      oldType->name,
		      oldNode,
		      oldType,
		      newType);

    oldName = oldNode ? oldNode->name : NULL;
    checkRanges(oldModule,
		oldLine,
		newModule,
		newLine > 0 ? newLine : smiGetTypeLine( newType ),
		oldName,
		oldType,
		newType);
}


static void
checkNodeTypeCompatibility(SmiModule *oldModule, SmiNode *oldNode,
			   SmiModule *newModule, SmiNode *newNode)
{
    SmiType *oldType, *newType;
    
    const int oldLine = smiGetNodeLine(oldNode);
    const int newLine = smiGetNodeLine(newNode);

    oldType = smiGetNodeType(oldNode);
    newType = smiGetNodeType(newNode);

    if ((!oldType) && (!newType)) return;

    if (oldType && newType && oldType->basetype != newType->basetype) {
	printErrorAtLine(newModule, ERR_BASETYPE_CHANGED,
			 newLine, newNode->name);
	printErrorAtLine(oldModule, ERR_PREVIOUS_DEFINITION,
			 oldLine, oldNode->name);
	return;
    }

    checkNamedNumbers(oldModule,
		      oldLine,
		      newModule,
		      newLine > 0 ? newLine : smiGetTypeLine(newType),
		      oldType->name,
		      oldNode,
		      oldType,
		      newType);
    checkRanges(oldModule,
		oldLine,
		newModule,
		newLine > 0 ? newLine : smiGetTypeLine(newType),
		oldNode->name,
		oldType,
		newType);
}


static void
checkTypes(SmiModule *oldModule, SmiNode *oldNode, SmiType *oldType,
	   SmiModule *newModule, SmiNode *newNode, SmiType *newType)
{
    int code = 0;
    
    code |= checkName(oldModule, smiGetTypeLine(oldType),
		      newModule, smiGetTypeLine(newType),
		      oldType->name, newType->name);

    checkTypeCompatibility(oldModule, oldNode, oldType,
			   newModule,
			   newNode ? smiGetNodeLine(newNode) : smiGetTypeLine(newType),
			   newType);
    
    checkDefVal(oldModule, smiGetTypeLine(oldType),
		newModule, smiGetTypeLine(newType),
		oldType->name, 
		oldType->value, newType->value);

    code |= checkDecl(oldModule, smiGetTypeLine(oldType),
		      newModule, smiGetTypeLine(newType),
		      newType->name,
		      oldType->decl, newType->decl);

    if (newType->name) {
	code |= checkStatus(oldModule, smiGetTypeLine(oldType),
			    newModule, smiGetTypeLine(newType),
			    newType->name, oldType->status, newType->status);
    }

    code |= checkFormat(oldModule, smiGetTypeLine(oldType),
			newModule, smiGetTypeLine(newType),
			newType->name,
			oldType->format, newType->format);

    code |= checkUnits(oldModule, smiGetTypeLine(oldType),
		       newModule, smiGetTypeLine(newType),
		       newType->name,
		       oldType->units, newType->units);

    code |= checkDescription(oldModule, smiGetTypeLine(oldType),
			     newModule, smiGetTypeLine(newType),
			     newType->name, oldType->decl,
			     oldType->description, newType->description);

    code |= checkReference(oldModule, smiGetTypeLine(oldType),
			   newModule, smiGetTypeLine(newType),
			   newType->name,
			   oldType->reference, newType->reference);

    if (code & CODE_SHOW_PREVIOUS) {
	printErrorAtLine(oldModule, ERR_PREVIOUS_DEFINITION,
			 smiGetTypeLine(oldType), oldType->name);
    }
    if (code & CODE_SHOW_PREVIOUS_IMPLICIT) {
	printErrorAtLine(oldModule, ERR_PREVIOUS_IMPLICIT_DEFINITION,
			 smiGetTypeLine(oldType));
    }
}



static void
diffTypes(SmiModule *oldModule, const char *oldTag,
	  SmiModule *newModule, const char *newTag)
{
    SmiType *oldType, *newType;

    /*
     * First check whether the old type definitions still exist and
     * whether the updates (if any) are consistent with the SMI rules.
     */
    
    smiInit(oldTag);
    for (oldType = smiGetFirstType(oldModule);
	 oldType;
	 oldType = smiGetNextType(oldType)) {
	
	smiInit(newTag);
	newType = smiGetType(newModule, oldType->name);
	if (newType) {
	    checkTypes(oldModule, NULL, oldType,
		       newModule, NULL, newType);
	} else {
	    printErrorAtLine(oldModule, ERR_TYPE_REMOVED,
			     smiGetTypeLine(oldType), oldType->name);
	}
	smiInit(oldTag);
    }

    /*
     * Let's see if there are any new definitions.
     */

    smiInit(newTag);
    for (newType = smiGetFirstType(newModule);
	 newType;
	 newType = smiGetNextType(newType)) {
	
	smiInit(oldTag);
	oldType = smiGetType(oldModule, newType->name);
	if (! oldType) {
	    printErrorAtLine(newModule, ERR_TYPE_ADDED,
			     smiGetTypeLine(newType), newType->name);
	}
	smiInit(newTag);
    }
}



static int
checkNodekind(SmiModule *oldModule, SmiNode *oldNode,
	      SmiModule *newModule, SmiNode *newNode)
{
    int code = 0;
    
    if (oldNode->nodekind != newNode->nodekind) {
	printErrorAtLine(newModule, ERR_NODEKIND_CHANGED,
			 smiGetNodeLine(newNode), newNode->name);
	code |= CODE_SHOW_PREVIOUS;
    }

    return code;
}



static char*
getStringIndexkind( SmiIndexkind indexkind )
{
    switch( indexkind ) {
    case SMI_INDEX_INDEX  : return "index";
    case SMI_INDEX_AUGMENT: return "augment";
    case SMI_INDEX_REORDER: return "reorder";
    case SMI_INDEX_SPARSE : return "sparse";
    case SMI_INDEX_EXPAND : return "expand";
    case SMI_INDEX_UNKNOWN:
    default: return "unknown";
    }
}

static char*
getStringIndexList( SmiNode *smiNode )
{
    SmiNode *indexNode;
    SmiElement *smiElement;
    char *strIdxLst;

    smiElement = smiGetFirstElement( smiNode );
    indexNode = smiGetElementNode( smiElement );
    strIdxLst = (char *)malloc( strlen( indexNode->name ) + 3);
    if( strIdxLst ) {
	sprintf( strIdxLst, "`%s'", indexNode->name );
    }
    smiElement = smiGetNextElement( smiElement );
    while ( smiElement ) {
	indexNode = smiGetElementNode( smiElement );
	strIdxLst = (char *)realloc( strIdxLst,
				     strlen( strIdxLst ) +
				     strlen( indexNode->name ) + 4 );
	sprintf( strIdxLst, "%s, `%s'", strIdxLst, indexNode->name );
	smiElement = smiGetNextElement( smiElement );
    }
    return strIdxLst;
}

static void
checkIndex(SmiModule *oldModule, SmiNode *oldNode,
	   SmiModule *newModule, SmiNode *newNode)
{
    if (newNode->indexkind == SMI_INDEX_UNKNOWN
	&& oldNode->indexkind == SMI_INDEX_UNKNOWN) {
	return;
    }

    if( newNode->indexkind != oldNode->indexkind) {
	printf( "%s\n", newModule->name );
	printErrorAtLine( newModule, ERR_INDEXKIND_CHANGED, 
			  smiGetNodeLine( oldNode ),
			  getStringIndexkind( oldNode->indexkind ),
			  getStringIndexkind( newNode->indexkind ),
			  newNode->name );
	printErrorAtLine( oldModule, ERR_PREVIOUS_DEFINITION,
			  smiGetNodeLine( newNode ), oldNode->name );
	
    }

    switch (newNode->indexkind) {
	SmiElement *oldElement, *newElement;
	SmiNode *oldRelNode, *newRelNode;
	int i;
	
    case SMI_INDEX_INDEX:
	/* compare OIDs of all index elements */
	oldElement = smiGetFirstElement( oldNode );
	newElement = smiGetFirstElement( newNode );
	while( oldElement && newElement ) {
	    SmiNode *oldIndexNode, *newIndexNode;
	    
	    oldIndexNode = smiGetElementNode( oldElement );
	    newIndexNode = smiGetElementNode( newElement );

	    if( oldIndexNode->oidlen != newIndexNode->oidlen ) {
		char *oldIdxLst, *newIdxLst;
		oldIdxLst = getStringIndexList( oldNode );
		newIdxLst = getStringIndexList( newNode );
		printErrorAtLine( newModule, ERR_INDEX_CHANGED,
				  smiGetNodeLine( newNode ), oldNode->name,
				  oldIdxLst, newIdxLst);
		free( oldIdxLst );
		free( newIdxLst );
		printErrorAtLine( oldModule, ERR_PREVIOUS_DEFINITION,
				  smiGetNodeLine( oldNode ), oldNode->name );
		return;
	    }

	    for( i = 0; i < oldIndexNode->oidlen; i++ ) {
		if( oldIndexNode->oid[i] != newIndexNode->oid[i] ) {
		    char *oldIdxLst, *newIdxLst;
		    oldIdxLst = getStringIndexList( oldNode );
		    newIdxLst = getStringIndexList( newNode );
		    printErrorAtLine( newModule, ERR_INDEX_CHANGED,
				      smiGetNodeLine( newNode ),
				      oldNode->name,
				      oldIdxLst, newIdxLst);
		    free( oldIdxLst );
		    free( newIdxLst );
		    printErrorAtLine( oldModule, ERR_PREVIOUS_DEFINITION,
				      smiGetNodeLine( oldNode ),
				      oldNode->name );
		    return;
		}
	    }
	    oldElement = smiGetNextElement( oldElement );
	    newElement = smiGetNextElement( newElement );
	}
	break;
	
    case SMI_INDEX_AUGMENT:
	/* compare OIDs of related nodes */
	oldRelNode = smiGetRelatedNode( oldNode );
	newRelNode = smiGetRelatedNode( newNode );

	if( !oldRelNode || !newRelNode ) {
	    /* should not occur */
	    return;
	}
	if( oldRelNode->oidlen != newRelNode->oidlen ) {
	    printErrorAtLine( newModule, ERR_INDEX_AUGMENT_CHANGED,
			      smiGetNodeLine( newNode ), oldNode->name,
			      oldRelNode->name, newRelNode->name);
	    printErrorAtLine( oldModule, ERR_PREVIOUS_DEFINITION,
			      smiGetNodeLine( oldNode ), oldNode->name );
	    return;
	}
	for( i = 0; i < oldRelNode->oidlen; i++ ) {
	    if( oldRelNode->oid[i] != newRelNode->oid[i] ) {
		printErrorAtLine( newModule, ERR_INDEX_AUGMENT_CHANGED,
				  smiGetNodeLine( newNode ),
				  oldNode->name,
				  oldRelNode->name, newRelNode->name);
		printErrorAtLine( oldModule, ERR_PREVIOUS_DEFINITION,
				  smiGetNodeLine( oldNode ), oldNode->name );
	    }
	}
	break;

    case SMI_INDEX_UNKNOWN:
	return;
	
    case SMI_INDEX_REORDER:
    case SMI_INDEX_SPARSE:
    case SMI_INDEX_EXPAND:
	/* xxx do things for SMI-NG */
	break;
    }
}


static void
checkObject(SmiModule *oldModule, SmiNode *oldNode,
	    SmiModule *newModule, SmiNode *newNode)
{
    int code = 0;
    SmiType *oldType, *newType;

    const int oldLine = smiGetNodeLine(oldNode);
    const int newLine = smiGetNodeLine(newNode);

    oldType = smiGetNodeType(oldNode);
    newType = smiGetNodeType(newNode);
    
    code |= checkName(oldModule, oldLine, newModule, newLine,
		      oldNode->name, newNode->name);
    
    if (oldType && newType) {
	if (oldType->name && !newType->name) {
	    printErrorAtLine(newModule, ERR_TO_IMPLICIT,
			     smiGetNodeLine(newNode),
			     newNode->name, oldType->name);
	    printErrorAtLine(oldModule, ERR_PREVIOUS_DEFINITION,
			     smiGetNodeLine(oldNode), oldNode->name);
	} else if (!oldType->name && newType->name) {
	    printErrorAtLine(newModule, ERR_FROM_IMPLICIT,
			     smiGetNodeLine(newNode),
			     newType->name, oldNode->name);
	    printErrorAtLine(oldModule, ERR_PREVIOUS_DEFINITION,
			     smiGetNodeLine(oldNode), oldNode->name);
	}
    }

    checkNodeTypeCompatibility(oldModule, oldNode,
			       newModule, newNode);

    code |= checkDecl(oldModule, oldLine, newModule, newLine,
		      newNode->name, oldNode->decl, newNode->decl);

    code |= checkStatus(oldModule, oldLine, newModule, newLine,
			newNode->name, oldNode->status, newNode->status);

    code |= checkAccess(oldModule, oldLine, newModule, newLine,
			newNode->name, oldNode->access, newNode->access);

    code |= checkNodekind(oldModule, oldNode, newModule, newNode);

    checkIndex(oldModule, oldNode, newModule, newNode);

   
    checkDefVal(oldModule, oldLine, newModule, newLine,
		newNode->name, oldNode->value, newNode->value);

    code |= checkFormat(oldModule, oldLine, newModule, newLine,
			newNode->name, oldNode->format, newNode->format);
    
    code |= checkUnits(oldModule, oldLine, newModule, newLine,
		       newNode->name, oldNode->units, newNode->units);

    code |= checkDescription(oldModule, oldLine, newModule, newLine,
			     newNode->name, oldNode->decl,
			     oldNode->description, newNode->description);

    code |= checkReference(oldModule, oldLine, newModule, newLine,
			   newNode->name,
			   oldNode->reference, newNode->reference);

    if (code & CODE_SHOW_PREVIOUS) {
	printErrorAtLine(oldModule, ERR_PREVIOUS_DEFINITION,
			 smiGetNodeLine(oldNode), oldNode->name);
    }
    if (code & CODE_SHOW_PREVIOUS_IMPLICIT) {
	printErrorAtLine(oldModule, ERR_PREVIOUS_IMPLICIT_DEFINITION,
			 smiGetNodeLine(oldNode));
    }
}



static void
diffObjects(SmiModule *oldModule, const char *oldTag,
	    SmiModule *newModule, const char *newTag)
{
    SmiNode *oldNode, *newNode;
    SmiNodekind nodekinds;

    nodekinds =  SMI_NODEKIND_NODE | SMI_NODEKIND_TABLE |
	SMI_NODEKIND_ROW | SMI_NODEKIND_COLUMN | SMI_NODEKIND_SCALAR;
    
    /*
     * First check whether the old node definitions still exist and
     * whether the updates (if any) are consistent with the SMI rules.
     */
    
    smiInit(oldTag);
    for(oldNode = smiGetFirstNode(oldModule, nodekinds);
	oldNode;
        oldNode = smiGetNextNode(oldNode, nodekinds)) {
	
	smiInit(newTag);
	newNode = smiGetNodeByOID(oldNode->oidlen, oldNode->oid);
	if (newNode
	    && newNode->oidlen == oldNode->oidlen
	    && smiGetNodeModule(newNode) == newModule) {
	    checkObject(oldModule, oldNode, newModule, newNode);
	} else {
	    switch (oldNode->nodekind) {
	    case SMI_NODEKIND_NODE:
	    case SMI_NODEKIND_TABLE:
	    case SMI_NODEKIND_ROW:
	    case SMI_NODEKIND_COLUMN:
	    case SMI_NODEKIND_SCALAR:
		printErrorAtLine(oldModule, ERR_NODE_REMOVED,
				 smiGetNodeLine(oldNode),
				 getStringNodekind(oldNode->nodekind),
				 oldNode->name);
	    }
	}
	smiInit(oldTag);
    }

    /*
     * Let's see if there are any new definitions.
     */

    smiInit(newTag);
    for (newNode = smiGetFirstNode(newModule, nodekinds);
	 newNode;
	 newNode = smiGetNextNode(newNode, nodekinds)) {
	
	smiInit(oldTag);
	oldNode = smiGetNodeByOID(newNode->oidlen, newNode->oid);
	if (! oldNode
	    || newNode->oidlen != oldNode->oidlen
	    || smiGetNodeModule(oldNode) != oldModule) {
	    printErrorAtLine(newModule, ERR_NODE_ADDED,
			     smiGetNodeLine(newNode),
			     getStringNodekind(newNode->nodekind),
			     newNode->name);
	}
	smiInit(newTag);
    }
}



static int
checkObjects(SmiModule *oldModule, const char *oldTag,
	     SmiModule *newModule, const char *newTag,
	     SmiNode *oldNode, SmiNode *newNode)
{
    SmiElement *oldElem, *newElem;
    SmiNode *oldElemNode, *newElemNode;
    int code = 0;

    smiInit(oldTag);
    for (oldElem = smiGetFirstElement(oldNode);
	 oldElem; oldElem = smiGetNextElement(oldElem)) {
	oldElemNode = smiGetElementNode(oldElem);
	smiInit(newTag);
	for (newElem = smiGetFirstElement(newNode);
	     newElem; newElem = smiGetNextElement(newElem)) {
	    newElemNode = smiGetElementNode(newElem);
	    if (strcmp(oldElemNode->name, newElemNode->name) == 0) {
		break;
	    }
	}
	if (! newElem) {
	    printErrorAtLine(oldModule, ERR_OBJECT_REMOVED,
			     smiGetNodeLine(oldNode), oldNode->name);
	    code |= CODE_SHOW_PREVIOUS;
	}
	smiInit(oldTag);
    }

    smiInit(newTag);
    for (newElem = smiGetFirstElement(newNode);
	 newElem; newElem = smiGetNextElement(newElem)) {
	newElemNode = smiGetElementNode(newElem);
	smiInit(oldTag);
	for (oldElem = smiGetFirstElement(oldNode);
	     oldElem; oldElem = smiGetNextElement(oldElem)) {
	    oldElemNode = smiGetElementNode(oldElem);
	    if (strcmp(oldElemNode->name, newElemNode->name) == 0) {
		break;
	    }
	}
	if (! oldElem) {
	    printErrorAtLine(newModule, ERR_OBJECT_ADDED,
			     smiGetNodeLine(newNode), newNode->name);
	}
	smiInit(newTag);
    }

    return code;
}



static void
checkNotification(SmiModule *oldModule, const char *oldTag,
		  SmiModule *newModule, const char *newTag,
		  SmiNode *oldNode, SmiNode *newNode)
{
    int code = 0;
    
    code |= checkDecl(oldModule, smiGetNodeLine(oldNode),
		      newModule, smiGetNodeLine(newNode),
		      newNode->name, oldNode->decl, newNode->decl);

    code |= checkStatus(oldModule, smiGetNodeLine(oldNode),
			newModule, smiGetNodeLine(newNode),
			newNode->name, oldNode->status, newNode->status);

    code |= checkObjects(oldModule, oldTag, newModule, newTag,
			 oldNode, newNode);

    code |= checkDescription(oldModule, smiGetNodeLine(oldNode),
			     newModule, smiGetNodeLine(newNode),
			     newNode->name, oldNode->decl,
			     oldNode->description, newNode->description);

    code |= checkReference(oldModule, smiGetNodeLine(oldNode),
			   newModule, smiGetNodeLine(newNode),
			   newNode->name,
			   oldNode->reference, newNode->reference);

    if (code & CODE_SHOW_PREVIOUS) {
	printErrorAtLine(oldModule, ERR_PREVIOUS_DEFINITION,
			 smiGetNodeLine(oldNode), oldNode->name);
    }
    if (code & CODE_SHOW_PREVIOUS_IMPLICIT) {
	printErrorAtLine(oldModule, ERR_PREVIOUS_IMPLICIT_DEFINITION,
			 smiGetNodeLine(oldNode));
    }
}



static void
diffNotifications(SmiModule *oldModule, const char *oldTag,
		  SmiModule *newModule, const char *newTag)
{
    SmiNode *oldNode, *newNode;

    /*
     * First check whether the old node definitions still exist and
     * whether the updates (if any) are consistent with the SMI rules.
     */
    
    smiInit(oldTag);
    for(oldNode = smiGetFirstNode(oldModule, SMI_NODEKIND_NOTIFICATION);
	oldNode;
        oldNode = smiGetNextNode(oldNode, SMI_NODEKIND_NOTIFICATION)) {
	
	smiInit(newTag);
	newNode = smiGetNodeByOID(oldNode->oidlen, oldNode->oid);
	if (newNode
	    && newNode->oidlen == oldNode->oidlen
	    && smiGetNodeModule(newNode) == newModule) {
	    checkNotification(oldModule, oldTag, newModule, newTag,
			      oldNode, newNode);
	} else {
	    printErrorAtLine(oldModule, ERR_NODE_REMOVED,
			     smiGetNodeLine(oldNode),
			     getStringNodekind(oldNode->nodekind),
			     oldNode->name);
	}
	smiInit(oldTag);
    }

    /*
     * Let's see if there are any new definitions.
     */

    smiInit(newTag);
    for (newNode = smiGetFirstNode(newModule, SMI_NODEKIND_NOTIFICATION);
	 newNode;
	 newNode = smiGetNextNode(newNode, SMI_NODEKIND_NOTIFICATION)) {
	
	smiInit(oldTag);
	oldNode = smiGetNodeByOID(newNode->oidlen, newNode->oid);
	if (! oldNode
	    || newNode->oidlen != oldNode->oidlen
	    || smiGetNodeModule(oldNode) != oldModule) {
	    printErrorAtLine(newModule, ERR_NODE_ADDED,
			     smiGetNodeLine(newNode),
			     getStringNodekind(newNode->nodekind),
			     newNode->name);
	}
	smiInit(newTag);
    }
}



static int
checkOrganization(SmiModule *oldModule, int oldLine,
		  SmiModule *newModule, int newLine,
		  char *name, char *oldOrga, char *newOrga)
{
    int code = 0;
    
    if (! oldOrga && newOrga) {
	printErrorAtLine(newModule, ERR_ORGA_ADDED,
			 newLine, name);
    }

    if (oldOrga && !newOrga) {
	printErrorAtLine(oldModule, ERR_ORGA_REMOVED,
			 oldLine, name);
	code |= CODE_SHOW_PREVIOUS;
    }

    if (oldOrga && newOrga && diffStrings(oldOrga, newOrga)) {
	printErrorAtLine(newModule, ERR_ORGA_CHANGED,
			 newLine, name);
	code |= CODE_SHOW_PREVIOUS;
    }

    return code;
}



static int
checkContact(SmiModule *oldModule, int oldLine,
	     SmiModule *newModule, int newLine,
	     char *name, char *oldContact, char *newContact)
{
    int code = 0;
    
    if (! oldContact && newContact) {
	printErrorAtLine(newModule, ERR_CONTACT_ADDED,
			 newLine, name);
    }

    if (oldContact && !newContact) {
	printErrorAtLine(oldModule, ERR_CONTACT_REMOVED,
			 oldLine, name);
	code |= CODE_SHOW_PREVIOUS;
    }

    if (oldContact && newContact && diffStrings(oldContact, newContact)) {
	printErrorAtLine(newModule, ERR_CONTACT_CHANGED,
			 newLine, name);
	code |= CODE_SHOW_PREVIOUS;
    }

    return code;
}



static void
diffModules(SmiModule *oldModule, const char *oldTag,
	    SmiModule *newModule, const char *newTag)
{
    SmiNode *oldIdentityNode, *newIdentityNode;
    SmiRevision *oldRev, *newRev;
    int oldLine = -1, newLine = -1;
    int code = 0;
    
    if (oldModule->language != newModule->language) {
	printErrorAtLine(newModule, ERR_SMIVERSION_CHANGED, -1);
    }

    oldIdentityNode = smiGetModuleIdentityNode(oldModule);
    if (oldIdentityNode) {
	oldLine = smiGetNodeLine(oldIdentityNode);
    }
    newIdentityNode = smiGetModuleIdentityNode(newModule);
    if (newIdentityNode) {
	newLine = smiGetNodeLine(newIdentityNode);
    }

    code |= checkOrganization(oldModule, oldLine,
			      newModule, newLine,
			      newModule->name,
			      oldModule->organization, newModule->organization);

    code |= checkContact(oldModule, oldLine, newModule, newLine,
			 newModule->name,
			 oldModule->contactinfo, newModule->contactinfo);

    code |= checkDescription(oldModule, oldLine,
			     newModule, newLine,
			     newModule->name, SMI_DECL_MODULEIDENTITY,
			     oldModule->description, newModule->description);

    code |= checkReference(oldModule, oldLine, newModule, newLine, newModule->name,
			   oldModule->reference, newModule->reference);

    /*
     * First check whether the old revisions still exist and
     * whether there are any updates.
     */

    smiInit(oldTag);
    for (oldRev = smiGetFirstRevision(oldModule);
	 oldRev; oldRev = smiGetNextRevision(oldRev)) {
	smiInit(newTag);
	for (newRev = smiGetFirstRevision(newModule);
	     newRev; newRev = smiGetNextRevision(newRev)) {
	    if (oldRev->date == newRev->date) {
		break;
	    }
	}
	if (newRev) {
	    if ((diffStrings(oldRev->description, newRev->description)) &&
		diffStrings(
		    "[Revision added by libsmi due to a LAST-UPDATED clause.]",
		    oldRev->description)) {
		printErrorAtLine(newModule, ERR_REVISION_CHANGED,
				 smiGetRevisionLine(newRev),
				 getStringTime(newRev->date));
		printErrorAtLine(oldModule, ERR_PREVIOUS_DEFINITION,
				 smiGetRevisionLine(oldRev),
				 getStringTime(oldRev->date));
	    }
	} else {
	    printErrorAtLine(oldModule, ERR_REVISION_REMOVED,
			     smiGetRevisionLine(oldRev),
			     getStringTime(oldRev->date));
	}
	smiInit(oldTag);
    }

    /*
     * Let's see if there are any new revisions.
     */

    smiInit(newTag);
    for (newRev = smiGetFirstRevision(newModule);
	 newRev; newRev = smiGetNextRevision(newRev)) {
	smiInit(oldTag);
	for (oldRev = smiGetFirstRevision(oldModule);
	     oldRev; oldRev = smiGetNextRevision(oldRev)) {
	    if (oldRev->date == newRev->date) {
		break;
	    }
	}
	if (!oldRev) {
	    printErrorAtLine(newModule, ERR_REVISION_ADDED,
			     smiGetRevisionLine(newRev),
			     getStringTime(newRev->date));
	}
	smiInit(newTag);
    }

    if (code & CODE_SHOW_PREVIOUS && oldLine >= 0) {
	printErrorAtLine(oldModule, ERR_PREVIOUS_DEFINITION,
			 oldLine, oldModule->name);
    }
    if (code & CODE_SHOW_PREVIOUS_IMPLICIT) {
	printErrorAtLine(oldModule, ERR_PREVIOUS_IMPLICIT_DEFINITION,
			 oldLine);
    }
}



static void
checkMember(SmiModule *oldModule, const char *oldTag,
	    SmiModule *newModule, const char *newTag,
	    SmiNode *oldNode, SmiNode *newNode)
{
    SmiElement *oldElem, *newElem;
    SmiNode *oldElemNode, *newElemNode;

    smiInit(oldTag);
    for (oldElem = smiGetFirstElement(oldNode);
	 oldElem; oldElem = smiGetNextElement(oldElem)) {
	oldElemNode = smiGetElementNode(oldElem);
	smiInit(newTag);
	for (newElem = smiGetFirstElement(newNode);
	     newElem; newElem = smiGetNextElement(newElem)) {
	    newElemNode = smiGetElementNode(newElem);
	    if (strcmp(oldElemNode->name, newElemNode->name) == 0) {
		break;
	    }
	}
	if (! newElem) {
	    printErrorAtLine(oldModule, ERR_MEMBER_REMOVED,
			     smiGetNodeLine(oldNode),
			     oldElemNode->name, oldNode->name);
	}
	smiInit(oldTag);
    }

    smiInit(newTag);
    for (newElem = smiGetFirstElement(newNode);
	 newElem; newElem = smiGetNextElement(newElem)) {
	newElemNode = smiGetElementNode(newElem);
	smiInit(oldTag);
	for (oldElem = smiGetFirstElement(oldNode);
	     oldElem; oldElem = smiGetNextElement(oldElem)) {
	    oldElemNode = smiGetElementNode(oldElem);
	    if (strcmp(oldElemNode->name, newElemNode->name) == 0) {
		break;
	    }
	}
	if (! oldElem) {
	    printErrorAtLine(newModule, ERR_MEMBER_ADDED,
			     smiGetNodeLine(newNode),
			     newElemNode->name, newNode->name);
	}
	smiInit(newTag);
    }
}



static void
checkGroup(SmiModule *oldModule, const char *oldTag,
	   SmiModule *newModule, const char *newTag,
	   SmiNode *oldNode, SmiNode *newNode)
{
    int code = 0;
    
    code = checkName(oldModule, smiGetNodeLine(oldNode),
		     newModule, smiGetNodeLine(newNode),
		     oldNode->name, newNode->name);
    
    code |= checkDecl(oldModule, smiGetNodeLine(oldNode),
		      newModule, smiGetNodeLine(newNode),
		      newNode->name, oldNode->decl, newNode->decl);
    
    code |= checkStatus(oldModule, smiGetNodeLine(oldNode),
			newModule, smiGetNodeLine(newNode),
			newNode->name, oldNode->status, newNode->status);

    code |= checkDescription(oldModule, smiGetNodeLine(oldNode),
			     newModule, smiGetNodeLine(newNode),
			     newNode->name, oldNode->decl,
			     oldNode->description, newNode->description);

    code |= checkReference(oldModule, smiGetNodeLine(oldNode),
			   newModule, smiGetNodeLine(newNode),
			   newNode->name,
			   oldNode->reference, newNode->reference);

    checkMember(oldModule, oldTag, newModule, newTag, oldNode, newNode);

    if (code & CODE_SHOW_PREVIOUS) {
	printErrorAtLine(oldModule, ERR_PREVIOUS_DEFINITION,
			 smiGetNodeLine(oldNode), oldNode->name);
    }
    if (code & CODE_SHOW_PREVIOUS_IMPLICIT) {
	printErrorAtLine(oldModule, ERR_PREVIOUS_IMPLICIT_DEFINITION,
			 smiGetNodeLine(oldNode));
    }
}



static void
diffGroups(SmiModule *oldModule, const char *oldTag,
	   SmiModule *newModule, const char *newTag)
{
    SmiNode *oldNode, *newNode;
    
    /*
     * First check whether the old node definitions still exist and
     * whether the updates (if any) are consistent with the SMI rules.
     */
    
    smiInit(oldTag);
    for(oldNode = smiGetFirstNode(oldModule, SMI_NODEKIND_GROUP);
	oldNode;
        oldNode = smiGetNextNode(oldNode, SMI_NODEKIND_GROUP)) {
	smiInit(newTag);
	newNode = smiGetNodeByOID(oldNode->oidlen, oldNode->oid);
	if (newNode
	    && newNode->oidlen == oldNode->oidlen
	    && smiGetNodeModule(newNode) == newModule) {
	    checkGroup(oldModule, oldTag, newModule, newTag, oldNode, newNode);
	} else {
	    printErrorAtLine(oldModule, ERR_NODE_REMOVED,
			     smiGetNodeLine(oldNode),
			     getStringNodekind(oldNode->nodekind),
			     oldNode->name);
	}
	smiInit(oldTag);
    }

    /*
     * Let's see if there are any new definitions.
     */

    smiInit(newTag);
    for (newNode = smiGetFirstNode(newModule, SMI_NODEKIND_GROUP);
	 newNode;
	 newNode = smiGetNextNode(newNode, SMI_NODEKIND_GROUP)) {
	
	smiInit(oldTag);
	oldNode = smiGetNodeByOID(newNode->oidlen, newNode->oid);
	if (! oldNode
	    || newNode->oidlen != oldNode->oidlen
	    || smiGetNodeModule(oldNode) != oldModule) {
	    printErrorAtLine(newModule, ERR_NODE_ADDED,
			     smiGetNodeLine(newNode),
			     getStringNodekind(newNode->nodekind),
			     newNode->name);
	}
	smiInit(newTag);
    }
}



static void
checkComplMandatory(SmiModule *oldModule, const char *oldTag,
		    SmiModule *newModule, const char *newTag,
		    SmiNode *oldNode, SmiNode *newNode)
{
    SmiElement *oldElem, *newElem;
    SmiNode *oldElemNode, *newElemNode;

    smiInit(oldTag);
    for (oldElem = smiGetFirstElement(oldNode);
	 oldElem; oldElem = smiGetNextElement(oldElem)) {
	oldElemNode = smiGetElementNode(oldElem);
	smiInit(newTag);
	for (newElem = smiGetFirstElement(newNode);
	     newElem; newElem = smiGetNextElement(newElem)) {
	    newElemNode = smiGetElementNode(newElem);
	    if (strcmp(oldElemNode->name, newElemNode->name) == 0) {
		break;
	    }
	}
	if (! newElem) {
	    if (strcmp(smiGetNodeModule(oldElemNode)->name, oldModule->name)) {
		printErrorAtLine(oldModule, ERR_MANDATORY_EXT_GROUP_REMOVED,
				 smiGetNodeLine(oldNode),
				 oldModule->name, oldElemNode->name,
				 oldNode->name);
	    } else {
		printErrorAtLine(oldModule, ERR_MANDATORY_GROUP_REMOVED,
				 smiGetNodeLine(oldNode),
				 oldElemNode->name,
				 oldNode->name);
	    }
	}
	smiInit(oldTag);
    }

    smiInit(newTag);
    for (newElem = smiGetFirstElement(newNode);
	 newElem; newElem = smiGetNextElement(newElem)) {
	newElemNode = smiGetElementNode(newElem);
	smiInit(oldTag);
	for (oldElem = smiGetFirstElement(oldNode);
	     oldElem; oldElem = smiGetNextElement(oldElem)) {
	    oldElemNode = smiGetElementNode(oldElem);
	    if (strcmp(oldElemNode->name, newElemNode->name) == 0) {
		break;
	    }
	}
	if (! oldElem) {
	    if (strcmp(smiGetNodeModule(newElemNode)->name, newModule->name)) {
		printErrorAtLine(newModule, ERR_MANDATORY_EXT_GROUP_ADDED,
				 smiGetNodeLine(newNode),
				 newModule->name, newElemNode->name,
				 newNode->name);
	    } else {
		printErrorAtLine(newModule, ERR_MANDATORY_GROUP_ADDED,
				 smiGetNodeLine(newNode),
				 newElemNode->name, newNode->name);
	    }
	}
	smiInit(newTag);
    }
}



static void
checkComplOptions(SmiModule *oldModule, const char *oldTag,
		  SmiModule *newModule, const char *newTag,
		  SmiNode *oldNode, SmiNode *newNode)
{
    int code;
    SmiOption *oldOption, *newOption;
    SmiNode *oldOptionNode, *newOptionNode;

    smiInit(oldTag);
    for (oldOption = smiGetFirstOption(oldNode);
	 oldOption; oldOption = smiGetNextOption(oldOption)) {
	oldOptionNode = smiGetOptionNode(oldOption);
	smiInit(newTag);
	for (newOption = smiGetFirstOption(newNode);
	     newOption; newOption = smiGetNextOption(newOption)) {
	    newOptionNode = smiGetOptionNode(newOption);
	    if (strcmp(oldOptionNode->name, newOptionNode->name) == 0) {
		break;
	    }
	}
	if (! newOption) {
	    if (strcmp(smiGetNodeModule(oldOptionNode)->name,
		       oldModule->name)) {
		printErrorAtLine(oldModule, ERR_EXT_OPTION_REMOVED,
				 smiGetOptionLine(oldOption),
				 oldModule->name, oldOptionNode->name,
				 oldNode->name);
	    } else {
		printErrorAtLine(oldModule, ERR_OPTION_REMOVED,
				 smiGetOptionLine(oldOption),
				 oldOptionNode->name,
				 oldNode->name);
	    }
	} else {
	    code = 0;
	    code |= checkDescription(oldModule, smiGetOptionLine(oldOption),
				     newModule, smiGetOptionLine(newOption),
				     newOptionNode->name, SMI_DECL_COMPL_GROUP,
				     oldOption->description,
				     newOption->description);
	    if (code & CODE_SHOW_PREVIOUS) {
		printErrorAtLine(oldModule, ERR_PREVIOUS_DEFINITION,
				 smiGetOptionLine(oldOption),
				 oldOptionNode->name);
	    }
	}
	smiInit(oldTag);
    }

    smiInit(newTag);
    for (newOption = smiGetFirstOption(newNode);
	 newOption; newOption = smiGetNextOption(newOption)) {
	newOptionNode = smiGetOptionNode(newOption);
	smiInit(oldTag);
	for (oldOption = smiGetFirstOption(oldNode);
	     oldOption; oldOption = smiGetNextOption(oldOption)) {
	    oldOptionNode = smiGetOptionNode(oldOption);
	    if (strcmp(oldOptionNode->name, newOptionNode->name) == 0) {
		break;
	    }
	}
	if (! oldOption) {
	    if (strcmp(smiGetNodeModule(newOptionNode)->name,
		       newModule->name)) {
		printErrorAtLine(newModule, ERR_EXT_OPTION_ADDED,
				 smiGetOptionLine(newOption),
				 newModule->name, newOptionNode->name,
				 newNode->name);
	    } else {
		printErrorAtLine(newModule, ERR_OPTION_ADDED,
				 smiGetOptionLine(newOption),
				 newOptionNode->name,
				 newNode->name);
	    }
	}
	smiInit(newTag);
    }
}



static void
checkComplRefinements(SmiModule *oldModule, const char *oldTag,
		  SmiModule *newModule, const char *newTag,
		  SmiNode *oldNode, SmiNode *newNode)
{
    int code;
    SmiRefinement *oldRefinement, *newRefinement;
    SmiNode *oldRefinementNode, *newRefinementNode;

    smiInit(oldTag);
    for (oldRefinement = smiGetFirstRefinement(oldNode);
	 oldRefinement; oldRefinement = smiGetNextRefinement(oldRefinement)) {
	oldRefinementNode = smiGetRefinementNode(oldRefinement);
	smiInit(newTag);
	for (newRefinement = smiGetFirstRefinement(newNode);
	     newRefinement; newRefinement = smiGetNextRefinement(newRefinement)) {
	    newRefinementNode = smiGetRefinementNode(newRefinement);
	    if (strcmp(oldRefinementNode->name, newRefinementNode->name) == 0) {
		break;
	    }
	}
	if (! newRefinement) {
	    if (strcmp(smiGetNodeModule(oldRefinementNode)->name,
		       oldModule->name)) {
		printErrorAtLine(oldModule, ERR_EXT_REFINEMENT_REMOVED,
				 smiGetRefinementLine(oldRefinement),
				 oldModule->name, oldRefinementNode->name,
				 oldNode->name);
	    } else {
		printErrorAtLine(oldModule, ERR_REFINEMENT_REMOVED,
				 smiGetRefinementLine(oldRefinement),
				 oldRefinementNode->name,
				 oldNode->name);
	    }
	} else {
	    code = 0;
	    code |= checkDescription(oldModule, smiGetRefinementLine(oldRefinement),
				     newModule, smiGetRefinementLine(newRefinement),
				     newRefinementNode->name,
				     SMI_DECL_COMPL_OBJECT,
				     oldRefinement->description,
				     newRefinement->description);

	    code |= checkAccess(oldModule,
				smiGetRefinementLine(oldRefinement),
				newModule,
				smiGetRefinementLine(newRefinement),
				newRefinementNode->name,
				oldRefinement->access, newRefinement->access);

	    checkTypeCompatibility(oldModule, oldRefinementNode,
				   smiGetRefinementType(oldRefinement),
				   newModule, smiGetRefinementLine(newRefinement),
				   smiGetRefinementType(newRefinement));
	    
	    checkTypeCompatibility(oldModule, oldRefinementNode,
				   smiGetRefinementWriteType(oldRefinement),
				   newModule, smiGetRefinementLine(newRefinement),
				   smiGetRefinementWriteType(newRefinement));
	    
	    if (code & CODE_SHOW_PREVIOUS) {
		printErrorAtLine(oldModule, ERR_PREVIOUS_DEFINITION,
				 smiGetRefinementLine(oldRefinement),
				 oldRefinementNode->name);
	    }
	}
	smiInit(oldTag);
    }

    smiInit(newTag);
    for (newRefinement = smiGetFirstRefinement(newNode);
	 newRefinement; newRefinement = smiGetNextRefinement(newRefinement)) {
	newRefinementNode = smiGetRefinementNode(newRefinement);
	smiInit(oldTag);
	for (oldRefinement = smiGetFirstRefinement(oldNode);
	     oldRefinement; oldRefinement = smiGetNextRefinement(oldRefinement)) {
	    oldRefinementNode = smiGetRefinementNode(oldRefinement);
	    if (strcmp(oldRefinementNode->name, newRefinementNode->name) == 0) {
		break;
	    }
	}
	if (! oldRefinement) {
	    if (strcmp(smiGetNodeModule(newRefinementNode)->name,
		       newModule->name)) {
		printErrorAtLine(newModule, ERR_EXT_REFINEMENT_ADDED,
				 smiGetRefinementLine(newRefinement),
				 newModule->name, newRefinementNode->name,
				 newNode->name);
	    } else {
		printErrorAtLine(newModule, ERR_REFINEMENT_ADDED,
				 smiGetRefinementLine(newRefinement),
				 newRefinementNode->name,
				 newNode->name);
	    }
	}
	smiInit(newTag);
    }
}



static void
checkCompliance(SmiModule *oldModule, const char *oldTag,
		SmiModule *newModule, const char *newTag,
		SmiNode *oldNode, SmiNode *newNode)
{
    int code = 0;
    
    code = checkName(oldModule, smiGetNodeLine(oldNode),
		     newModule, smiGetNodeLine(newNode),
		     oldNode->name, newNode->name);
    
    code |= checkDecl(oldModule, smiGetNodeLine(oldNode),
		      newModule, smiGetNodeLine(newNode),
		      newNode->name, oldNode->decl, newNode->decl);
    
    code |= checkStatus(oldModule, smiGetNodeLine(oldNode),
			newModule, smiGetNodeLine(newNode),
			newNode->name, oldNode->status, newNode->status);

    code |= checkDescription(oldModule, smiGetNodeLine(oldNode),
			     newModule, smiGetNodeLine(newNode),
			     newNode->name, oldNode->decl,
			     oldNode->description, newNode->description);

    code |= checkReference(oldModule, smiGetNodeLine(oldNode),
			   newModule, smiGetNodeLine(newNode),
			   newNode->name,
			   oldNode->reference, newNode->reference);

    if (code & CODE_SHOW_PREVIOUS) {
	printErrorAtLine(oldModule, ERR_PREVIOUS_DEFINITION,
			 smiGetNodeLine(oldNode), oldNode->name);
    }
    if (code & CODE_SHOW_PREVIOUS_IMPLICIT) {
	printErrorAtLine(oldModule, ERR_PREVIOUS_IMPLICIT_DEFINITION,
			 smiGetNodeLine(oldNode));
    }

    checkComplMandatory(oldModule, oldTag, newModule, newTag,
			oldNode, newNode);

    checkComplOptions(oldModule, oldTag, newModule, newTag,
		      oldNode, newNode);

    checkComplRefinements(oldModule, oldTag, newModule, newTag,
			  oldNode, newNode);
}



static void
diffCompliances(SmiModule *oldModule, const char *oldTag,
		SmiModule *newModule, const char *newTag)
{
    SmiNode *oldNode, *newNode;
    
    /*
     * First check whether the old node definitions still exist and
     * whether the updates (if any) are consistent with the SMI rules.
     */
    
    smiInit(oldTag);
    for(oldNode = smiGetFirstNode(oldModule, SMI_NODEKIND_COMPLIANCE);
	oldNode;
        oldNode = smiGetNextNode(oldNode, SMI_NODEKIND_COMPLIANCE)) {
	smiInit(newTag);
	newNode = smiGetNodeByOID(oldNode->oidlen, oldNode->oid);
	if (newNode
	    && newNode->oidlen == oldNode->oidlen
	    && smiGetNodeModule(newNode) == newModule) {
	    checkCompliance(oldModule, oldTag, newModule, newTag,
			    oldNode, newNode);
	} else {
	    printErrorAtLine(oldModule, ERR_NODE_REMOVED,
			     smiGetNodeLine(oldNode),
			     getStringNodekind(oldNode->nodekind),
			     oldNode->name);
	}
	smiInit(oldTag);
    }

    /*
     * Let's see if there are any new definitions.
     */

    smiInit(newTag);
    for (newNode = smiGetFirstNode(newModule, SMI_NODEKIND_COMPLIANCE);
	 newNode;
	 newNode = smiGetNextNode(newNode, SMI_NODEKIND_COMPLIANCE)) {
	
	smiInit(oldTag);
	oldNode = smiGetNodeByOID(newNode->oidlen, newNode->oid);
	if (! oldNode
	    || newNode->oidlen != oldNode->oidlen
	    || smiGetNodeModule(oldNode) != oldModule) {
	    printErrorAtLine(newModule, ERR_NODE_ADDED,
			     smiGetNodeLine(newNode),
			     getStringNodekind(newNode->nodekind),
			     newNode->name);
	}
	smiInit(newTag);
    }
}



static SmiNode*
findGroupElement(SmiNode *groupNode, const char *name)
{
    SmiElement *smiElement = NULL;
    SmiNode *smiNode = NULL;
    
    for (smiElement = smiGetFirstElement(groupNode);
	 smiElement;
	 smiElement = smiGetNextElement(smiElement)) {
	smiNode = smiGetElementNode(smiElement);
	if (strcmp(smiNode->name, name) == 0) {
	    return smiNode;
	}
    }
    return NULL;
}


static SmiNode*
findGroupsElement(SmiNode *groupNode, const char *name)
{
    SmiElement *smiElement = NULL;
    SmiNode *smiNode = NULL, *foundNode;
    
    for (smiElement = smiGetFirstElement(groupNode);
	 smiElement;
	 smiElement = smiGetNextElement(smiElement)) {
	smiNode = smiGetElementNode(smiElement);
	foundNode = findGroupElement(smiNode, name);
	if (foundNode) {
	    return foundNode;
	}
    }
    return NULL;
}



static void
diffOldNewComplianceMandatory(SmiModule *oldModule, const char *oldTag,
			      SmiModule *newModule, const char *newTag,
			      SmiNode *oldComplNode, SmiNode *newComplNode)
{
    SmiElement *oldGroupElement, *newGroupElement, *oldElement, *newElement;
    SmiNode *oldGroupNode, *newGroupNode, *oldNode, *newNode;

    smiInit(oldTag);
    for (oldGroupElement = smiGetFirstElement(oldComplNode);
	 oldGroupElement;
	 oldGroupElement = smiGetNextElement(oldGroupElement)) {
	oldGroupNode = smiGetElementNode(oldGroupElement);
	for (oldElement = smiGetFirstElement(oldGroupNode);
	     oldElement;
	     oldElement = smiGetNextElement(oldElement)) {
	    oldNode = smiGetElementNode(oldElement);
	    smiInit(newTag);
	    newNode = findGroupsElement(newComplNode, oldNode->name);
	    if (! newNode) {
		if (strcmp(smiGetNodeModule(oldNode)->name, oldModule->name)) {
		    printErrorAtLine(oldModule, ERR_MANDATORY_EXT_REMOVED,
				     smiGetNodeLine(oldNode),
				     getStringNodekind(oldNode->nodekind),
				     oldModule->name, oldNode->name,
				     oldComplNode->name,
				     newComplNode->name);
		} else {
		    printErrorAtLine(oldModule, ERR_MANDATORY_REMOVED,
				     smiGetNodeLine(oldNode),
				     getStringNodekind(oldNode->nodekind),
				     oldNode->name,
				     oldComplNode->name,
				     newComplNode->name);
		}
	    }
	    smiInit(oldTag);
	}
    }

    smiInit(newTag);
    for (newGroupElement = smiGetFirstElement(newComplNode);
	 newGroupElement;
	 newGroupElement = smiGetNextElement(newGroupElement)) {
	newGroupNode = smiGetElementNode(newGroupElement);
	for (newElement = smiGetFirstElement(newGroupNode);
	     newElement;
	     newElement = smiGetNextElement(newElement)) {
	    newNode = smiGetElementNode(newElement);
	    smiInit(oldTag);
	    oldNode = findGroupsElement(oldComplNode, newNode->name);
	    if (! oldNode) {
		if (strcmp(smiGetNodeModule(newNode)->name, newModule->name)) {
		    printErrorAtLine(newModule, ERR_MANDATORY_EXT_ADDED,
				     smiGetNodeLine(newNode),
				     getStringNodekind(newNode->nodekind),
				     newModule->name, newNode->name,
				     oldComplNode->name,
				     newComplNode->name);
		} else {
		    printErrorAtLine(newModule, ERR_MANDATORY_ADDED,
				     smiGetNodeLine(newNode),
				     getStringNodekind(newNode->nodekind),
				     newNode->name,
				     oldComplNode->name,
				     newComplNode->name);
		}
	    }
	    smiInit(newTag);
	}
    }
}



static void
diffOldNewComplianceOptional(SmiModule *oldModule, const char *oldTag,
			     SmiModule *newModule, const char *newTag,
			     SmiNode *oldComplNode, SmiNode *newComplNode)
{
    SmiElement *oldElement, *newElement;
    SmiOption *oldOption, *newOption;
    SmiNode *oldGroupNode, *newGroupNode, *oldNode, *newNode;

    smiInit(oldTag);
    for (oldOption = smiGetFirstOption(oldComplNode);
	 oldOption;
	 oldOption = smiGetNextOption(oldOption)) {
	oldGroupNode = smiGetOptionNode(oldOption);
	for (oldElement = smiGetFirstElement(oldGroupNode);
	     oldElement;
	     oldElement = smiGetNextElement(oldElement)) {
	    oldNode = smiGetElementNode(oldElement);
	    smiInit(newTag);
	    newNode = findGroupsElement(newComplNode, oldNode->name);
	    if (! newNode) {
		if (strcmp(smiGetNodeModule(oldNode)->name, oldModule->name)) {
		    printErrorAtLine(oldModule, ERR_OPTIONAL_EXT_REMOVED,
				     smiGetNodeLine(oldNode),
				     getStringNodekind(oldNode->nodekind),
				     oldModule->name, oldNode->name,
				     oldComplNode->name,
				     newComplNode->name);
		} else {
		    printErrorAtLine(oldModule, ERR_OPTIONAL_REMOVED,
				     smiGetNodeLine(oldNode),
				     getStringNodekind(oldNode->nodekind),
				     oldNode->name,
				     oldComplNode->name,
				     newComplNode->name);
		}
	    } else {
		/* xxx compare group condition description here? xxx */
	    }
	    smiInit(oldTag);
	}
    }

    smiInit(newTag);
    for (newOption = smiGetFirstOption(newComplNode);
	 newOption;
	 newOption = smiGetNextOption(newOption)) {
	newGroupNode = smiGetOptionNode(newOption);
	for (newElement = smiGetFirstElement(newGroupNode);
	     newElement;
	     newElement = smiGetNextElement(newElement)) {
	    newNode = smiGetElementNode(newElement);
	    smiInit(oldTag);
	    oldNode = findGroupsElement(oldComplNode, newNode->name);
	    if (! oldNode) {
		if (strcmp(smiGetNodeModule(newNode)->name, newModule->name)) {
		    printErrorAtLine(newModule, ERR_OPTIONAL_EXT_ADDED,
				     smiGetNodeLine(newNode),
				     getStringNodekind(newNode->nodekind),
				     newModule->name, newNode->name,
				     oldComplNode->name,
				     newComplNode->name);
		} else {
		    printErrorAtLine(newModule, ERR_OPTIONAL_ADDED,
				     smiGetNodeLine(newNode),
				     getStringNodekind(newNode->nodekind),
				     newNode->name,
				     oldComplNode->name,
				     newComplNode->name);
		}
	    }
	    smiInit(newTag);
	}
    }
}



static void
diffOldNewCompliance(SmiModule *oldModule, const char *oldTag,
		     SmiModule *newModule, const char *newTag,
		     const char *oldCompl, const char *newCompl)
{
    SmiNode *smiNode;
    SmiNode *oldComplNode = NULL, *newComplNode = NULL;

    for (smiNode = smiGetFirstNode(oldModule, SMI_NODEKIND_COMPLIANCE);
	 smiNode;
	 smiNode = smiGetNextNode(smiNode, SMI_NODEKIND_COMPLIANCE)) {
	if (strcmp(smiNode->name, oldCompl) == 0) {
	    oldComplNode = smiNode;
	}
    }

    if (! oldComplNode) {
	    fprintf(stderr, "smidiff: unable to find old compliance `%s'\n",
		oldCompl);
    }

    for (smiNode = smiGetFirstNode(newModule, SMI_NODEKIND_COMPLIANCE);
	 smiNode;
	 smiNode = smiGetNextNode(smiNode, SMI_NODEKIND_COMPLIANCE)) {
	if (strcmp(smiNode->name, newCompl) == 0) {
	    newComplNode = smiNode;
	}
    }

    if (! newComplNode) {
	fprintf(stderr, "smidiff: unable to find new compliance `%s'\n",
		newCompl);
    }

    if (!oldComplNode || !newComplNode) {
	return;
    }

    diffOldNewComplianceMandatory(oldModule, oldTag,
				  newModule, newTag,
				  oldComplNode, newComplNode);
    diffOldNewComplianceOptional(oldModule, oldTag,
				 newModule, newTag,
				 oldComplNode, newComplNode);
}



static void
usage()
{
    fprintf(stderr,
	    "Usage: smidiff [options] oldmodule newmodule\n"
	    "  -V, --version             show version and license information\n"
	    "  -c, --config=file         load a specific configuration file\n"
	    "  -h, --help                show usage information\n"
	    "  -i, --ignore=prefix       ignore errors matching prefix pattern\n"
	    "  -l, --level=level         set maximum level of errors and warnings\n"
	    "  -m, --error-names         print the name of errors in braces\n"
	    "  -p, --preload=module      preload <module>\n"
	    "  -s, --severity            print the severity of errors in brackets\n"
	    "      --old-compliance=name name of the old compliance statement\n"
	    "      --new-compliance=name name of the new compliance statement\n");
}



static void help() { usage(); exit(0); }
static void version() { printf("smidiff " SMI_VERSION_STRING "\n"); exit(0); }
static void config(char *filename) { smiReadConfig(filename, "smidiff"); }
static void level(int lev) { errorLevel = lev; }
static void ignore(char *ign)
{
    smiSetSeverity(ign, 9999);		/* libsmi  error messages */
    setErrorSeverity(ign, 9999);	/* smidiff error messages */
}

static void preload(char *module) {
    smiInit(oldTag);
    smiLoadModule(module);
    smiInit(newTag);
    smiLoadModule(module);
}


int
main(int argc, char *argv[])
{
    SmiModule *oldModule, *newModule;
    int flags;

    static optStruct opt[] = {
	/* short long              type        var/func       special       */
	{ 'h', "help",           OPT_FLAG,   help,          OPT_CALLFUNC },
	{ 'V', "version",        OPT_FLAG,   version,       OPT_CALLFUNC },
	{ 'c', "config",         OPT_STRING, config,        OPT_CALLFUNC },
	{ 'l', "level",          OPT_INT,    level,         OPT_CALLFUNC },
	{ 'p', "preload",        OPT_STRING, preload,       OPT_CALLFUNC },
	{ 'm', "error-names",    OPT_FLAG,   &mFlag,        0 },
	{ 's', "severity",       OPT_FLAG,   &sFlag,        0 },
	{ 'i', "ignore",	 OPT_STRING, ignore,	    OPT_CALLFUNC },
	{   0, "old-compliance", OPT_STRING, &oldCompl,	    0 },
	{   0, "new-compliance", OPT_STRING, &newCompl,	    0 },
	{ 0, 0, OPT_END, 0, 0 }  /* no more options */
    };
    
    smiInit(oldTag);
    flags = smiGetFlags();
    flags |= SMI_FLAG_ERRORS;
    smiSetFlags(flags);
    smiSetErrorLevel(errorLevel);

    smiInit(newTag);
    flags = smiGetFlags();
    flags |= SMI_FLAG_ERRORS;
    smiSetFlags(flags);
    smiSetErrorLevel(errorLevel);

    optParseOptions(&argc, argv, opt, 0);

    if (argc != 3) {
	usage();
	return 1;
    }

    if (oldCompl && !newCompl) {
	fprintf(stderr, "smidiff: missing new compliance statement name\n");
	return 1;
    }

    if (!oldCompl && newCompl) {
	fprintf(stderr, "smidiff: missing old compliance statement name\n");
	return 1;
    }

    smiInit(oldTag);
    smiSetErrorLevel(errorLevel);
    oldModule = smiGetModule(smiLoadModule(argv[1]));
    if (! oldModule) {
        fprintf(stderr, "smidiff: cannot locate module `%s'\n", argv[1]);
        smiExit();
        exit(1);
    }

    smiInit(newTag);
    smiSetErrorLevel(errorLevel);
    newModule = smiGetModule(smiLoadModule(argv[2]));
    if (! newModule) {
        fprintf(stderr, "smidiff: cannot locate module `%s'\n", argv[2]);
        smiExit();
        smiInit(oldTag);
        smiExit();
        exit(2);
    }

    if (oldCompl && newCompl) {
	diffOldNewCompliance(oldModule, oldTag, newModule, newTag,
			     oldCompl, newCompl);
    } else {
	diffModules(oldModule, oldTag, newModule, newTag);
	diffTypes(oldModule, oldTag, newModule, newTag);
	diffObjects(oldModule, oldTag, newModule, newTag);
	diffNotifications(oldModule, oldTag, newModule, newTag);
	diffGroups(oldModule, oldTag, newModule, newTag);
	diffCompliances(oldModule, oldTag, newModule, newTag);
    }

    smiInit(oldTag);
    smiExit();

    smiInit(newTag);
    smiExit();

    if (fflush(stdout) || ferror(stdout)) {
	perror("smidiff: write error");
	exit(1);
    }

    return 0;
}
