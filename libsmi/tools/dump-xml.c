/*
 * dump-xml.c --
 *
 *      Operations to dump SMIng module information in XML format.
 *
 * Copyright (c) 2000 Frank Strauss, Technical University of Braunschweig.
 * Copyright (c) 2000 J. Schoenwaelder, Technical University of Braunschweig.
 *
 * See the file "COPYING" for information on usage and redistribution
 * of this file, and for a DISCLAIMER OF ALL WARRANTIES.
 *
 * @(#) $Id: dump-xml.c 8090 2008-04-18 12:56:29Z strauss $
 */

/*
 * TODO:
 *
 * - value representations (getValueString())
 * - finish DTD and check against it
 * - shall we nest tables like in SMIng?
 */

#include <config.h>

#include <stdlib.h>
#include <stdio.h>
#include <stdarg.h>
#include <string.h>
#include <ctype.h>
#include <time.h>

#include "smi.h"
#include "smidump.h"



#define  INDENT		2    /* indent factor */
#define  INDENTVALUE	20   /* column to start values, except multiline */
#define  INDENTTEXTS	4    /* column to start multiline texts */
#define  INDENTMAX	64   /* max column to fill, break lines otherwise */



static int disableschema  = 0;
static int disabledoctype = 0;



typedef struct XmlEscape {
    char character;
    char *escape;
} XmlEscape;

static XmlEscape xmlEscapes [] = {
    { '<',	"&lt;" },
    { '>',	"&gt;" },
    { '&',	"&amp;" },
    { 0,	NULL }
};


static int current_column = 0;



static char *getStringLanguage(SmiLanguage lang)
{
    return
	(lang == SMI_LANGUAGE_SMIV1) ? "SMIv1" :
	(lang == SMI_LANGUAGE_SMIV2) ? "SMIv2" :
	(lang == SMI_LANGUAGE_SMING) ? "SMIng" :
                                     	NULL;
}



static char *getStringStatus(SmiStatus status)
{
    return
	(status == SMI_STATUS_CURRENT)     ? "current" :
	(status == SMI_STATUS_DEPRECATED)  ? "deprecated" :
	(status == SMI_STATUS_OBSOLETE)    ? "obsolete" :
	(status == SMI_STATUS_MANDATORY)   ? "mandatory" :
	(status == SMI_STATUS_OPTIONAL)    ? "optional" :
					     "<unknown>";
}



static char *getAccessString(SmiAccess access)
{
    return
	(access == SMI_ACCESS_NOT_ACCESSIBLE) ? "noaccess" :
	(access == SMI_ACCESS_NOTIFY)	      ? "notifyonly" :
	(access == SMI_ACCESS_READ_ONLY)      ? "readonly" :
	(access == SMI_ACCESS_READ_WRITE)     ? "readwrite" :
						"<unknown>";
}



static char *getStringBasetype(SmiBasetype basetype)
{
    return
        (basetype == SMI_BASETYPE_UNKNOWN)           ? "<UNKNOWN>" :
        (basetype == SMI_BASETYPE_OCTETSTRING)       ? "OctetString" :
        (basetype == SMI_BASETYPE_OBJECTIDENTIFIER)  ? "ObjectIdentifier" :
        (basetype == SMI_BASETYPE_UNSIGNED32)        ? "Unsigned32" :
        (basetype == SMI_BASETYPE_INTEGER32)         ? "Integer32" :
        (basetype == SMI_BASETYPE_UNSIGNED64)        ? "Unsigned64" :
        (basetype == SMI_BASETYPE_INTEGER64)         ? "Integer64" :
        (basetype == SMI_BASETYPE_FLOAT32)           ? "Float32" :
        (basetype == SMI_BASETYPE_FLOAT64)           ? "Float64" :
        (basetype == SMI_BASETYPE_FLOAT128)          ? "Float128" :
        (basetype == SMI_BASETYPE_ENUM)              ? "Enumeration" :
        (basetype == SMI_BASETYPE_BITS)              ? "Bits" :
                                                   "<unknown>";
}



static char *getTimeString(time_t t)
{
    static char   *s = NULL;
    struct tm	  *tm;

    if (s) xfree(s);
    
    tm = gmtime(&t);
    smiAsprintf(&s, "%04d-%02d-%02d %02d:%02d",
		tm->tm_year + 1900, tm->tm_mon + 1, tm->tm_mday,
		tm->tm_hour, tm->tm_min);
    return s;
}



static void fprint(FILE *f, char *fmt, ...)
{
    va_list ap;
    char    *s;
    char    *p;

    va_start(ap, fmt);
    current_column += smiVasprintf(&s, fmt, ap);
    va_end(ap);
    fputs(s, f);
    if ((p = strrchr(s, '\n'))) {
        current_column = strlen(p) - 1;
    }
    free(s);
}



static void fprintSegment(FILE *f, int column, char *string, int length)
{
    fprint(f, "%*c%s", column, ' ', string);
    if (length) {
	fprint(f, "%*c", length - strlen(string) - column, ' ');
    }
}



static void fprintMultilineString(FILE *f, int column, const char *s)
{
    int i, j, len;

#ifdef INDENTTEXTS
    fprintSegment(f, column + INDENTTEXTS, "", 0);
#endif
    if (s) {
	len = strlen(s);
	for (i=0; i < len; i++) {
	    for (j = 0; xmlEscapes[j].character; j++) {
		if (xmlEscapes[j].character == s[i]) break;
	    }
	    if (xmlEscapes[j].character) {
		fputs(xmlEscapes[j].escape, f);
		current_column += strlen(xmlEscapes[j].escape);
	    } else {
		putc(s[i], f);
		current_column++;
	    }
	    if (s[i] == '\n') {
		current_column = 0;
#ifdef INDENTTEXTS
		fprintSegment(f, column + INDENTTEXTS, "", 0);
#endif
	    }
	}
    }
    current_column++;
}



static char *getValueString(SmiValue *valuePtr, SmiType *typePtr)
{
    static char    s[1024];
    char           ss[9];
    int		   n;
    unsigned int   i;
    SmiNamedNumber *nn;
    
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
            sprintf(s, "0x%*s", 2 * valuePtr->len, "");
            for (i=0; i < valuePtr->len; i++) {
                sprintf(ss, "%02x", valuePtr->value.ptr[i]);
                strncpy(&s[2+2*i], ss, 2);
            }
	}
	break;
    case SMI_BASETYPE_BITS:
	sprintf(s, "(");
	for (i = 0, n = 0; i < valuePtr->len * 8; i++) {
	    if (valuePtr->value.ptr[i/8] & (1 << (7-(i%8)))) {
		if (n)
		    sprintf(&s[strlen(s)], ", ");
		n++;
		for (nn = smiGetFirstNamedNumber(typePtr); nn;
		     nn = smiGetNextNamedNumber(nn)) {
		    if (nn->value.value.unsigned32 == i)
			break;
		}
		if (nn) {
		    sprintf(&s[strlen(s)], "%s", nn->name);
		} else {
		    sprintf(s, "%d", i);
		}
	    }
	}
	sprintf(&s[strlen(s)], ")");
	break;
    case SMI_BASETYPE_UNKNOWN:
	break;
    case SMI_BASETYPE_POINTER:
	break;
    case SMI_BASETYPE_OBJECTIDENTIFIER:
	for (i = 0; i < valuePtr->len; i++) {
	    sprintf(&s[strlen(s)], i ? ".%u" : "%u", valuePtr->value.oid[i]);
	}
	break;
    }

    return s;
}



static void fprintNodeStartTag(FILE *f, int indent,
			       const char *tag, SmiNode *smiNode)
{
    unsigned int i;
    
    fprintSegment(f, indent, "", 0);
    fprint(f, "<%s name=\"%s\"", tag, smiNode->name);
    fprint(f, " oid=\"");
    for (i = 0; i < smiNode->oidlen; i++) {
	fprint(f, i ? ".%u" : "%u", smiNode->oid[i]);
    }
    fprint(f, "\"");
    if (smiNode->create) {
	fprint(f, " create=\"true\"");
    }
    if (smiNode->status != SMI_STATUS_UNKNOWN) {
	fprint(f, " status=\"%s\"", getStringStatus(smiNode->status));
    }
    fprint(f, ">\n");
}



static void fprintNodeEndTag(FILE *f, int indent, const char *tag)
{
    fprintSegment(f, indent, "", 0);
    fprint(f, "</%s>\n", tag);
}



static void fprintRanges(FILE *f, int indent, SmiType *smiType)
{
    SmiRange       *range;

    for(range = smiGetFirstRange(smiType);
	range;
	range = smiGetNextRange(range)) {
	fprintSegment(f, indent, "<range", 0);
	fprint(f, " min=\"%s\"", getValueString(&range->minValue, smiType));
	fprint(f, " max=\"%s\"", getValueString(&range->maxValue, smiType));
	fprint(f, "/>\n");
    }
}



static void fprintNamedNumbers(FILE *f, int indent, SmiType *smiType)
{
    SmiNamedNumber *nn;

    if ((smiType->basetype != SMI_BASETYPE_ENUM) &&
	(smiType->basetype != SMI_BASETYPE_BITS)) {
	return;
    }

    for (nn = smiGetFirstNamedNumber(smiType);
	 nn;
	 nn = smiGetNextNamedNumber(nn)) {
	fprintSegment(f, indent, "<namednumber", 0);
	fprint(f, " name=\"%s\"", nn->name);
	fprint(f, " number=\"%s\"", getValueString(&nn->value, smiType));
	fprint(f, "/>\n");
    }
}



static void fprintValue(FILE *f, int indent, SmiValue *smiValue,
			SmiType *smiType)
{
    if (smiType && smiValue && smiValue->basetype != SMI_BASETYPE_UNKNOWN) {
	fprintSegment(f, indent, "<default>", 0);
	fprint(f, "%s", getValueString(smiValue, smiType));
	fprint(f, "</default>\n");
    }
}



static void fprintDescription(FILE *f, int indent, const char *description)
{
    if (description) {
	fprintSegment(f, indent, "<description>\n", 0);
	fprintMultilineString(f, indent, description);
	fprint(f, "\n");
	fprintSegment(f, indent, "</description>\n", 0);
    }
}



static void fprintReference(FILE *f, int indent, const char *reference)
{
    if (reference) {
	fprintSegment(f, indent, "<reference>\n", 0);
	fprintMultilineString(f, indent, reference);
	fprint(f, "\n");
	fprintSegment(f, indent, "</reference>\n", 0);
    }
}



static void fprintFormat(FILE *f, int indent, const char *format)
{
    if (format) {
	fprintSegment(f, indent, "", 0);
	fprint(f, "<format>%s</format>\n", format);
    }
}



static void fprintUnits(FILE *f, int indent, const char *units)
{
    if (units) {
	fprintSegment(f, indent, "", 0);
	fprint(f, "<units>%s</units>\n", units);
    }
}



static void fprintAccess(FILE *f, int indent, SmiAccess smiAccess)
{
    if (smiAccess != SMI_ACCESS_UNKNOWN) {
	fprintSegment(f, indent, "", 0);
	fprint(f, "<access>%s</access>\n", getAccessString(smiAccess));
    }
}



static void fprintElementList(FILE *f, int indent, const char *tag,
			      SmiElement *smiElement)
{
    SmiModule *smiModule;
    SmiNode   *smiNode;

    for (; smiElement; smiElement = smiGetNextElement(smiElement)) {
	smiNode = smiGetElementNode(smiElement);
	smiModule = smiGetNodeModule(smiNode);
	fprintSegment(f, indent, "", 0);
	fprint(f, "<%s module=\"%s\" name=\"%s\"/>\n",
	       tag, smiModule->name, smiNode->name);
    }
}



static void fprintIndex(FILE *f, int indent, SmiNode *smiNode)
{
    SmiNode   *relatedNode;
    SmiModule *relatedModule = NULL;

    fprintSegment(f, indent, "<linkage", 0);
    if (smiNode->implied) {
	fprint(f, " implied=\"true\"");
    }
    fprint(f, ">\n");

    relatedNode = smiGetRelatedNode(smiNode);
    if (relatedNode) {
	relatedModule = smiGetNodeModule(relatedNode);
    }
    switch (smiNode->indexkind) {
    case SMI_INDEX_INDEX:
	fprintElementList(f, indent + INDENT, "index",
			  smiGetFirstElement(smiNode));
	break;
    case SMI_INDEX_AUGMENT:
	if (relatedNode && relatedModule) {
	    fprintSegment(f, indent + INDENT, "", 0);
	    fprint(f, "<augments module=\"%s\" name=\"%s\"/>\n",
		   relatedModule->name, relatedNode->name);
	} /* TODO: else print error */
	break;
    case SMI_INDEX_REORDER:
	if (relatedNode && relatedModule) {
	    fprintSegment(f, indent + INDENT, "", 0);
	    fprint(f, "<reorders module=\"%s\" name=\"%s\"/>\n",
		   relatedModule->name, relatedNode->name);
	    fprintElementList(f, indent + INDENT, "index",
			      smiGetFirstElement(smiNode));
	} /* TODO: else print error */
	break;
    case SMI_INDEX_SPARSE:
	if (relatedNode && relatedModule) {
	    fprintSegment(f, indent + INDENT, "", 0);
	    fprint(f, "<sparse module=\"%s\" name=\"%s\"/>\n",
		   relatedModule->name, relatedNode->name);
	} /* TODO: else print error */
	break;
    case SMI_INDEX_EXPAND:
	if (relatedNode && relatedModule) {
	    fprintSegment(f, indent + INDENT, "", 0);
	    fprint(f, "<expands module=\"%s\" name=\"%s\"/>\n",
		   relatedModule->name, relatedNode->name);
	    fprintElementList(f, indent + INDENT, "index",
			      smiGetFirstElement(smiNode));
	} /* TODO: else print error */
	break;
    case SMI_INDEX_UNKNOWN:
	break;
    }
    fprintSegment(f, indent, "</linkage>\n", 0);
}



static void fprintModule(FILE *f, SmiModule *smiModule)
{
    SmiRevision *smiRevision;
    SmiNode     *smiNode;
    int         i;
    char	*lang;

    lang = getStringLanguage(smiModule->language);

    fprintSegment(f, INDENT, "", 0);
    if (lang) {
	fprint(f, "<module name=\"%s\" language=\"%s\">\n",
	       smiModule->name, lang);
    } else {
	fprint(f, "<module name=\"%s\">\n", smiModule->name);
    }

    if (smiModule->organization) {
	fprintSegment(f, 2 * INDENT, "<organization>", INDENTVALUE);
	fprint(f, "\n");
	fprintMultilineString(f, 2 * INDENT, smiModule->organization);
	fprint(f, "\n");
	fprintSegment(f, 2 * INDENT, "</organization>\n", 0);
    }

    if (smiModule->contactinfo) {
	fprintSegment(f, 2 * INDENT, "<contact>", INDENTVALUE);
	fprint(f, "\n");
	fprintMultilineString(f, 2 * INDENT, smiModule->contactinfo);
	fprint(f, "\n");
	fprintSegment(f, 2 * INDENT, "</contact>\n", 0);
    }
    fprintDescription(f, 2 * INDENT, smiModule->description);
    fprintReference(f, 2 * INDENT, smiModule->reference);

    for(i = 0, smiRevision = smiGetFirstRevision(smiModule);
	smiRevision; smiRevision = smiGetNextRevision(smiRevision)) {
	fprintSegment(f, 2 * INDENT, "", 0);
	fprint(f, "<revision date=\"%s\">\n",
	       getTimeString(smiRevision->date));
	fprintDescription(f, 3 * INDENT, smiRevision->description);
        fprintSegment(f, 2 * INDENT, "</revision>\n", 0);
	i++;
    }

    smiNode = smiGetModuleIdentityNode(smiModule);
    if (smiNode) {
	fprintSegment(f, 2 * INDENT, "", 0);
	fprint(f, "<identity node=\"%s\"/>\n", smiNode->name);
    }

    fprintSegment(f, INDENT, "</module>\n\n", 0);
}



static void fprintImport(FILE *f, int indent, SmiImport *smiImport)
{
    fprintSegment(f, indent, "", 0);
    fprint(f, "<import module=\"%s\" name=\"%s\"/>\n",
	   smiImport->module, smiImport->name);
}


    
static void fprintImports(FILE *f, SmiModule *smiModule)
{
    SmiImport *smiImport;
    int        i;

    for (i = 0, smiImport = smiGetFirstImport(smiModule);
	 smiImport;
	 i++, smiImport = smiGetNextImport(smiImport)) {
	if (i == 0) {
	    fprintSegment(f, INDENT, "<imports>\n", 0);
	}
	fprintImport(f, 2 * INDENT, smiImport);
    }

    if (i) {
	fprintSegment(f, INDENT, "</imports>\n\n", 0);
    }
}



static void fprintTypedef(FILE *f, int indent, SmiType *smiType)
{
    SmiModule *parentModule;
    SmiType *parentType;
    
    fprintSegment(f, indent, "<typedef", 0);
    if (smiType->name) {
	fprint(f, " name=\"%s\"", smiType->name);
    }
    fprint(f, " basetype=\"%s\"", getStringBasetype(smiType->basetype));
    if (smiType->name && smiType->status != SMI_STATUS_UNKNOWN) {
	fprint(f, " status=\"%s\"", getStringStatus(smiType->status));
    }
    fprint(f, ">\n");
    
    parentType = smiGetParentType(smiType);
    parentModule = smiGetTypeModule(parentType);
    if (parentType && parentType->name &&
	parentModule && strlen(parentModule->name)) {
	fprintSegment(f, indent + INDENT, "<parent ", 0);
	fprintf(f, "module=\"%s\" name=\"%s\"/>\n",
		parentModule->name, parentType->name);
    }
    fprintRanges(f, indent + INDENT, smiType);
    fprintNamedNumbers(f, indent + INDENT, smiType);
    fprintValue(f, indent + INDENT, &smiType->value, smiType);
    fprintFormat(f, indent + INDENT, smiType->format);
    fprintUnits(f, indent + INDENT, smiType->units);
    fprintDescription(f, indent + INDENT, smiType->description);
    fprintReference(f, indent + INDENT, smiType->reference);
    
    fprintSegment(f, indent, "</typedef>\n", 0);
}



static void fprintTypedefs(FILE *f, SmiModule *smiModule)
{
    int		 i;
    SmiType	 *smiType;
    
    for(i = 0, smiType = smiGetFirstType(smiModule);
	smiType;
	i++, smiType = smiGetNextType(smiType)) {

	if (i == 0) {
	    fprintSegment(f, INDENT, "<typedefs>\n", 0);
	}
	fprintTypedef(f, 2 * INDENT, smiType);
    }

    if (i) {
	fprintSegment(f, INDENT, "</typedefs>\n\n", 0);
    }
}



static void fprintNode(FILE *f, int indent, SmiNode *smiNode,
		       SmiNode *lastSmiNode)
{
    SmiModule   *smiModule;
    SmiType     *smiType;
    char        *tag = NULL;
    
    if (smiNode->nodekind == SMI_NODEKIND_NODE) {
	tag = "node";
    } else if (smiNode->nodekind == SMI_NODEKIND_CAPABILITIES) {
	tag = "node";
    } else if (smiNode->nodekind == SMI_NODEKIND_TABLE) {
	tag = "table";
    } else if (smiNode->nodekind == SMI_NODEKIND_ROW) {
	indent += INDENT;
	tag = "row";
    } else if (smiNode->nodekind == SMI_NODEKIND_COLUMN) {
	indent += 2 * INDENT;
	tag = "column";
    } else if (smiNode->nodekind == SMI_NODEKIND_SCALAR) {
	tag = "scalar";
    }

    if (lastSmiNode
	&& lastSmiNode->nodekind == SMI_NODEKIND_COLUMN
	&& smiNode->nodekind != SMI_NODEKIND_COLUMN) {
	fprintNodeEndTag(f, indent + INDENT, "row");
        fprintNodeEndTag(f, indent, "table");
    }

    smiType = smiGetNodeType(smiNode);
    
    fprintNodeStartTag(f, indent, tag, smiNode);
    if (smiType && (smiType->basetype != SMI_BASETYPE_UNKNOWN)) {
	fprintSegment(f, indent + INDENT, "<syntax>\n", 0);
	smiModule = smiGetTypeModule(smiType);
	if (smiType->name && smiModule) {
	    fprintSegment(f, indent + 2 *INDENT, "", 0);
	    fprint(f, "<type ");
	    fprintf(f, "module=\"%s\" name=\"%s\"/>\n",
		    smiModule->name, smiType->name);
	} else {
	    fprintTypedef(f, indent + 2 * INDENT, smiType);
	}
	fprintSegment(f, indent + INDENT, "</syntax>\n", 0);
    }
    if ((smiNode->nodekind != SMI_NODEKIND_TABLE) &&
	(smiNode->nodekind != SMI_NODEKIND_ROW) &&
	(smiNode->nodekind != SMI_NODEKIND_CAPABILITIES) &&
	(smiNode->nodekind != SMI_NODEKIND_NODE)) {
	fprintAccess(f, indent + INDENT, smiNode->access);
    }
    if (smiType) {
	fprintValue(f, indent + INDENT, &smiNode->value, smiType);
    }
    fprintFormat(f, indent + INDENT, smiNode->format);
    fprintUnits(f, indent + INDENT, smiNode->units);
    if (smiNode->nodekind == SMI_NODEKIND_ROW) {
	fprintIndex(f, indent + INDENT, smiNode);
    }
    fprintDescription(f, indent + INDENT, smiNode->description);
    fprintReference(f, indent + INDENT, smiNode->reference);

    if (smiNode->nodekind != SMI_NODEKIND_ROW
	&& smiNode->nodekind != SMI_NODEKIND_TABLE) {
	fprintNodeEndTag(f, indent, tag);
    }
}



static void fprintNodes(FILE *f, SmiModule *smiModule)
{
    int		 i;
    SmiNode	 *smiNode, *lastSmiNode;
    SmiNodekind  nodekinds;

    nodekinds =  SMI_NODEKIND_NODE | SMI_NODEKIND_TABLE |
	SMI_NODEKIND_ROW | SMI_NODEKIND_COLUMN | SMI_NODEKIND_SCALAR |
	SMI_NODEKIND_CAPABILITIES;
    
    for (i = 0, lastSmiNode = NULL,
	     smiNode = smiGetFirstNode(smiModule, nodekinds);
	 smiNode;
	 i++, lastSmiNode = smiNode,
	     smiNode = smiGetNextNode(smiNode, nodekinds)) {

	if (i == 0) {
	    fprintSegment(f, INDENT, "<nodes>\n", 0);
	}

	fprintNode(f, 2 * INDENT, smiNode, lastSmiNode);
    }
    
    if (lastSmiNode
	&& lastSmiNode->nodekind == SMI_NODEKIND_COLUMN) {
	fprintNodeEndTag(f, 3 * INDENT, "row");
        fprintNodeEndTag(f, 2 * INDENT, "table");
    }

    if (i) {
	fprintSegment(f, INDENT, "</nodes>\n\n", 0);
    }
}



static void fprintNotification(FILE *f, int indent, SmiNode *smiNode)
{
    fprintNodeStartTag(f, indent, "notification", smiNode);

    fprintSegment(f, indent + INDENT, "<objects>\n", 0);
    fprintElementList(f, indent + 2 * INDENT, "object",
		      smiGetFirstElement(smiNode));
    fprintSegment(f, indent + INDENT, "</objects>\n", 0);
    fprintDescription(f, indent + INDENT, smiNode->description);
    fprintReference(f, indent + INDENT, smiNode->reference);
    
    fprintNodeEndTag(f, indent, "notification");
}



static void fprintNotifications(FILE *f, SmiModule *smiModule)
{
    SmiNode *smiNode;
    int	     i;
    
    for(i = 0, smiNode = smiGetFirstNode(smiModule, SMI_NODEKIND_NOTIFICATION);
	smiNode;
	i++, smiNode = smiGetNextNode(smiNode, SMI_NODEKIND_NOTIFICATION)) {

	if (i == 0) {
	    fprintSegment(f, INDENT, "<notifications>\n", 0);
	}
	fprintNotification(f, 2 * INDENT, smiNode);
    }

    if (i) {
	fprintSegment(f, INDENT, "</notifications>\n\n", 0);
    }
}



static void fprintGroup(FILE *f, int indent, SmiNode *smiNode)
{
    fprintNodeStartTag(f, indent, "group", smiNode);
    
    fprintSegment(f, indent + INDENT, "<members>\n", 0);
    fprintElementList(f, indent + 2 * INDENT, "member",
		      smiGetFirstElement(smiNode));
    fprintSegment(f, indent + INDENT, "</members>\n", 0);
    fprintDescription(f, indent + INDENT, smiNode->description);
    fprintReference(f, indent + INDENT, smiNode->reference);

    fprintNodeEndTag(f, indent, "group");
}



static void fprintGroups(FILE *f, SmiModule *smiModule)
{
    SmiNode *smiNode;
    int	     i;
    
    for(i = 0, smiNode = smiGetFirstNode(smiModule, SMI_NODEKIND_GROUP);
	smiNode;
	i++, smiNode = smiGetNextNode(smiNode, SMI_NODEKIND_GROUP)) {

	if (i == 0) {
	    fprintSegment(f, INDENT, "<groups>\n", 0);
	}
	fprintGroup(f, 2 * INDENT, smiNode);
    }

    if (i) {
	fprintSegment(f, INDENT, "</groups>\n\n", 0);
    }
}



static void fprintComplGroups(FILE *f, int indent, SmiNode *smiNode)
{
    SmiNode   *optSmiNode;
    SmiModule *optSmiModule;
    SmiOption *smiOption;

    if (! smiGetFirstElement(smiNode) && !smiGetFirstOption(smiNode)) {
	return;
    }
    
    fprintSegment(f, indent, "<requires>\n", 0);
    fprintElementList(f, indent + INDENT, "mandatory",
		      smiGetFirstElement(smiNode));

    for(smiOption = smiGetFirstOption(smiNode);
	smiOption;
	smiOption = smiGetNextOption(smiOption)) {
	optSmiNode = smiGetOptionNode(smiOption);
	optSmiModule = smiGetNodeModule(optSmiNode);
	fprintSegment(f, indent + INDENT, "", 0);
	fprint(f, "<option module=\"%s\" name=\"%s\">\n",
	       optSmiModule->name, optSmiNode->name);
	fprintDescription(f, indent + 2 * INDENT, smiOption->description);
	fprintSegment(f, indent + INDENT, "</option>\n", 0);
    }
    
    fprintSegment(f, indent, "</requires>\n", 0);
}



static void fprintRefinement(FILE *f, int indent, SmiRefinement *smiRefinement)
{
    SmiModule *smiModule;
    SmiNode   *smiNode;
    SmiType   *smiType;

    smiNode = smiGetRefinementNode(smiRefinement);
    smiModule = smiGetNodeModule(smiNode);

    fprintSegment(f, indent, "<refinement ", 0);
    fprintf(f, "module=\"%s\" name=\"%s\">\n", smiModule->name, smiNode->name);

    smiType = smiGetRefinementType(smiRefinement);
    if (smiType) {
	fprintSegment(f, indent + INDENT, "<syntax>\n", 0);
	fprintTypedef(f, indent + 2 * INDENT, smiType);
	fprintSegment(f, indent + INDENT, "</syntax>\n", 0);
    }
    
    smiType = smiGetRefinementWriteType(smiRefinement);
    if (smiType) {
	fprintSegment(f, indent + INDENT, "<writesyntax>\n", 0);
	fprintTypedef(f, indent + 2 * INDENT, smiType);
	fprintSegment(f, indent + INDENT, "</writesyntax>\n", 0);
    }

    if (smiRefinement->access != SMI_ACCESS_UNKNOWN) {
	fprintAccess(f, indent + INDENT, smiRefinement->access);
    }
    fprintDescription(f, indent + INDENT, smiRefinement->description);
    fprintSegment(f, indent, "</refinement>\n", 0);
}



static void fprintRefinements(FILE *f, int indent, SmiNode *smiNode)
{
    SmiRefinement *smiRefinement;
    int            i;

    for(i = 0, smiRefinement = smiGetFirstRefinement(smiNode);
	smiRefinement;
	i++, smiRefinement = smiGetNextRefinement(smiRefinement)) {

    	if (!i) {
	    fprintSegment(f, indent, "<refinements>\n", 0);
	}

	fprintRefinement(f, indent + INDENT, smiRefinement);
    }
    
    if (i) {
	fprintSegment(f, indent, "</refinements>\n\n", 0);
    }
}



static void fprintCompliance(FILE *f, int indent, SmiNode *smiNode)
{
    fprintNodeStartTag(f, indent, "compliance", smiNode);

    fprintDescription(f, indent + INDENT, smiNode->description);
    fprintReference(f, indent + INDENT, smiNode->reference);
    fprintComplGroups(f, indent + INDENT, smiNode);
    fprintRefinements(f, indent + INDENT, smiNode);

    fprintNodeEndTag(f, indent, "compliance");
}



static void fprintCompliances(FILE *f, SmiModule *smiModule)
{
    SmiNode *smiNode;
    int      i;

    for(i = 0, smiNode = smiGetFirstNode(smiModule, SMI_NODEKIND_COMPLIANCE);
	smiNode;
	i++, smiNode = smiGetNextNode(smiNode, SMI_NODEKIND_COMPLIANCE)) {
	
	if (!i) {
	    fprintSegment(f, INDENT, "<compliances>\n", 0);
	}

	fprintCompliance(f, 2 * INDENT, smiNode);
    }

    if (i) {
	fprintSegment(f, INDENT, "</compliances>\n\n", 0);
    }
}



static void dumpXml(int modc, SmiModule **modv, int flags, char *output)
{
    int  i;
    FILE *f = stdout;

    if (output) {
	f = fopen(output, "w");
	if (!f) {
	    fprintf(stderr, "smidump: cannot open %s for writing: ", output);
	    perror(NULL);
	    exit(1);
	}
    }

    for (i = 0; i < modc; i++) {
	
	fprint(f, "<?xml version=\"1.0\"?>\n");
	if (!disabledoctype) {
	    fprint(f, "<!DOCTYPE smi SYSTEM \"http://www.ibr.cs.tu-bs.de/projects/nmrg/smi.dtd\">\n");
	}
	fprint(f, "\n");
	fprint(f, "<!-- This module has been generated by smidump "
	       SMI_VERSION_STRING ". Do not edit. -->\n");
	fprint(f, "\n");

	if (!disableschema) {
	    fprint(f, "<smi xmlns:xsi=\"http://www.w3.org/2001/XMLSchema-instance\"\n");
	    fprint(f, "     xsi:noNamespaceSchemaLocation=\"http://www.ibr.cs.tu-bs.de/projects/nmrg/smi.xsd\">\n");
	} else {
	    fprint(f, "<smi>\n");
	}
	
	fprintModule(f, modv[i]);
	fprintImports(f, modv[i]);
	fprintTypedefs(f, modv[i]);
	fprintNodes(f, modv[i]);
	fprintNotifications(f, modv[i]);
	fprintGroups(f, modv[i]);
	fprintCompliances(f, modv[i]);
	
	fprint(f, "</smi>\n");
    }
    
    if (fflush(f) || ferror(f)) {
	perror("smidump: write error");
	exit(1);
    }

    if (output) {
	fclose(f);
    }
}



void initXml()
{
    
    static SmidumpDriverOption opt[] = {
	{ "no-schema", OPT_FLAG, &disableschema, 0,
	  "disable XML Schema spec in the toplevel element"},
	{ "no-doctype", OPT_FLAG, &disabledoctype, 0,
	  "disable DOCTYPE spec in the XML prolog"},
        { 0, OPT_END, 0, 0 }
    };

    static SmidumpDriver driver = {
	"xml",
	dumpXml,
	0,
	SMIDUMP_DRIVER_CANT_UNITE,
	"intermediate SMI XML exchange format",
	opt,
	NULL
    };
    
    smidumpRegisterDriver(&driver);
}
