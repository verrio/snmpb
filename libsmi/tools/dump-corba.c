/*
 * dump-corba.c --
 *
 *      Operations to dump CORBA IDL and OID definitions. This is based
 *	on the JIDM Specification Translation developed by the JIDM task
 *	force, published as Open Group <URL:http://www.opengroup.org/>
 *	document C802 (ISBN 1-85912-256-6, January 2000).
 *
 * Copyright (c) 1999 Frank Strauss, Technical University of Braunschweig.
 * Copyright (c) 1999 J. Schoenwaelder, Technical University of Braunschweig.
 *
 * See the file "COPYING" for information on usage and redistribution
 * of this file, and for a DISCLAIMER OF ALL WARRANTIES.
 *
 * @(#) $Id: dump-corba.c 8090 2008-04-18 12:56:29Z strauss $
 */

#include <config.h>

#include <stdlib.h>
#include <stdio.h>
#include <stdarg.h>
#include <string.h>
#include <ctype.h>
#include <time.h>
#ifdef HAVE_UNISTD_H
#include <unistd.h>
#endif
#ifdef HAVE_WIN_H
#include "win.h"
#endif

#include "smi.h"
#include "smidump.h"



#define  INDENT		4    /* indent factor */
#define  INDENTVALUE	20   /* column to start values, except multiline */
#define  INDENTTEXTS	13   /* column to start multiline texts */
#define  INDENTMAX	72   /* max column to fill, break lines otherwise */


static int current_column = 0;

static int silent = 0;


/*
 * The following list of IDL keywords is taken from the CORBA
 * 2.3.1 IDL specification section 3.2.4 (October 1999).
 */


static char *idlKeywords[] = {
    "abstract",
    "any",		"attribute",	"boolean",	"case",
    "char",		"const",	"context",	"custom",
    "default",		"double",	"enum",		"exception",
    "factory",		"FALSE",	"fixed",	"float",
    "in",		"inout",	"interface",	"long",
    "module",		"native",	"object",	"octet",
    "oneway",		"out",		"private",	"public",
    "raises",		"readonly",	"sequence",	"short",
    "string",		"struct",	"supports",	"switch",
    "TRUE",		"truncatable",	"typedef",	"unsigned",
    "union",		"valuebase",	"valuetype",	"void",
    "wchar",		"wstring",	NULL
};


/*
 * Structure used to build a list of imported types.
 */


typedef struct Import {
    char          *module;
    char          *name;
    struct Import *nextPtr;
} Import;

static Import *importList = NULL;


/*
 * Structure used to build dictionaries that translate names
 * into IDL names following the generic JIDM rules.
 */


typedef struct IdlEntry {
    char            *module;
    char            *name;
    char            *idlname;
    struct IdlEntry *nextPtr;
} IdlEntry;

static IdlEntry *idlModuleNameList = NULL;
static IdlEntry *idlNodeNameList = NULL;
static IdlEntry *idlTypeNameList = NULL;
static IdlEntry *idlVBTypeNameList = NULL;

static IdlEntry **idlNameLists[] = {
    &idlModuleNameList, &idlNodeNameList,
    &idlTypeNameList, &idlVBTypeNameList,
    NULL
};



static FILE * createFile(char *name, char *suffix)
{
    char *fullname;
    FILE *f;

    fullname = xmalloc(strlen(name) + (suffix ? strlen(suffix) : 0) + 2);
    strcpy(fullname, name);
    if (suffix) {
        strcat(fullname, suffix);
    }
    if (!access(fullname, R_OK)) {
        fprintf(stderr, "smidump: %s already exists\n", fullname);
        xfree(fullname);
        return NULL;
    }
    f = fopen(fullname, "w");
    if (!f) {
        fprintf(stderr, "smidump: cannot open %s for writing: ", fullname);
        perror(NULL);
        xfree(fullname);
        exit(1);
    }
    xfree(fullname);
    return f;
}



static char* dictFindName(IdlEntry *list, char *module, char* name)
{
    IdlEntry *p;

    for (p = list; p; p = p->nextPtr) {
	if (! strcasecmp(p->module, module)
	    && ((! p->name && ! name) || ! strcasecmp(p->name, name))) {
	    return p->idlname;
	}
    }

    return NULL;
}



static char* dictAddName(IdlEntry **listPtr, char *module, char *name)
{
    IdlEntry *p;
    char *s, *idl;
    int i;

    /*
     * Create a new IDL identifier by translating hyphens
     * to underscores.
     */

    s = name ? name : module;
    idl = xmalloc(strlen(s) + 1);
    for (i = 0; s[i]; i++) {
	idl[i] = (s[i] == '-') ? '_' : s[i];
    }
    idl[i] = 0;

    /*
     * Check for any collisions with IDL keywords or previously
     * created IDL identifiers.
     */

    for (i = 0; idlKeywords[i]; i++) {
	if (! strcasecmp(idlKeywords[i], idl)) {
	    fprintf(stderr, "smidump: "
		    "`%s' (%s%s%s) collides with IDL keyword `%s'\n",
		    idl, module, name ? "::" : "", name ? name : "",
		    idlKeywords[i]);
	}
    }

    for (i = 0; idlNameLists[i]; i++) {
	IdlEntry *list = *(idlNameLists[i]);
	for (p = list; p; p = p->nextPtr) {
	    if (! strcasecmp(p->idlname, idl)) {
		fprintf(stderr, "smidump: "
			"`%s' (%s%s%s) collides with `%s' (%s%s%s)\n",
			idl, module,
			name ? "::" : "", name ? name : "",
			p->idlname, p->module,
			p->name ? "::" : "", p->name ? p->name : "");
	    }
	}
    }
    
    /*
     * Safe the translated identifier in the dictionary.
     */
    
    p = xmalloc(sizeof(IdlEntry));
    p->module = xstrdup(module);
    p->name = name ? xstrdup(name) : NULL;
    p->idlname = idl;
    p->nextPtr = *listPtr;
    *listPtr = p;

    return idl;
}



static void dictFree(IdlEntry **list)
{
    IdlEntry *p, *q;

    for (p = *list; p; ) {
	q = p;
	p = p->nextPtr;
	xfree(q->module);
	if (q->name) xfree(q->name);
	xfree(q->idlname);
	xfree(q);
    }

    *list = NULL;
}



static char* getIdlModuleName(char *module)
{
    char *s;

    s = dictFindName(idlModuleNameList, module, NULL);
    if (! s) {
	s = dictAddName(&idlModuleNameList, module, NULL);
    }
    return s;
}



static char *getIdlNodeName(char *module, char *name)
{
    char *s;
    s = dictFindName(idlNodeNameList, module, name);
    if (! s) {
	s = dictAddName(&idlNodeNameList, module, name);
    }
    return s;
}



static char* getIdlTypeName(char *module, char *name)
{
    char *s, *type_name;

    type_name = xmalloc(strlen(name) + 10);
    sprintf(type_name, "%sType", name);
    type_name[0] = toupper((int) type_name[0]);

    s = dictFindName(idlTypeNameList, module, type_name);
    if (! s) {
	s = dictAddName(&idlTypeNameList, module, type_name);
    }

    xfree(type_name);
    return s;
}



static char* getIdlVBTypeName(char *module, char *name, int *isnew)
{
    char *s, *vbTypeName;

    vbTypeName = xmalloc(strlen(name) + 10);
    sprintf(vbTypeName, "%sVBType", name);
    vbTypeName[0] = toupper((int) vbTypeName[0]);

    if (isnew) {
	*isnew = 0;
    }
    s = dictFindName(idlVBTypeNameList, module, vbTypeName);
    if (! s) {
	if (isnew) {
	    *isnew = 1;
	}
	s = dictAddName(&idlVBTypeNameList, module, vbTypeName);
    }

    xfree(vbTypeName);
    return s;
}



static int current(SmiStatus status)
{
    switch (status) {
    case SMI_STATUS_CURRENT:
    case SMI_STATUS_UNKNOWN:
    case SMI_STATUS_MANDATORY:
    case SMI_STATUS_OPTIONAL:
	return 1;
    default:
	return 0;
    }
}



static char *getTimeString(time_t t)
{
    static char   *s = NULL;
    struct tm	  *tm;

    if (s) xfree(s);

    tm = gmtime(&t);
    smiAsprintf(&s, "%04d%02d%02d%02d%02dZ",
		tm->tm_year + 1900, tm->tm_mon + 1, tm->tm_mday,
		tm->tm_hour, tm->tm_min);
    return s;
}



static char *getAccessString(SmiAccess access, int create)
{
    if (create && (access == SMI_ACCESS_READ_WRITE)) {
	return "read-create";
    } else {
	return
	    (access == SMI_ACCESS_NOT_ACCESSIBLE) ? "not-accessible" :
	    (access == SMI_ACCESS_NOTIFY)         ? "accessible-for-notify" :
	    (access == SMI_ACCESS_READ_ONLY)      ? "read-only" :
	    (access == SMI_ACCESS_READ_WRITE)     ? "read-write" :
					            "<unknown>";
    }
}



static char *getBaseTypeString(SmiBasetype basetype)
{
    switch(basetype) {
    case SMI_BASETYPE_UNKNOWN:
	return "ASN1_Null";
    case SMI_BASETYPE_POINTER:
	return "ASN1_Null";
    case SMI_BASETYPE_INTEGER32:
    case SMI_BASETYPE_ENUM:
	return "ASN1_Integer";
    case SMI_BASETYPE_OCTETSTRING:
    case SMI_BASETYPE_BITS:
	return "ASN1_OctetString";
    case SMI_BASETYPE_OBJECTIDENTIFIER:
	return "ASN1_ObjectIdentifier";
    case SMI_BASETYPE_UNSIGNED32:
	return "ASN1_Unsigned";
    case SMI_BASETYPE_INTEGER64:
	return "ASN1_Integer64";
    case SMI_BASETYPE_UNSIGNED64:
	return "ASN1_Unsigned64";
    case SMI_BASETYPE_FLOAT32:
    case SMI_BASETYPE_FLOAT64:
    case SMI_BASETYPE_FLOAT128:
	return "ASN1_Real";
    }

    return NULL;
}


static char *getIdlAnyTypeName(SmiNode *smiNode, SmiType *smiType)
{
    SmiModule *smiModule;
    char *typeName;

    if (! smiType->name) {
	smiModule = smiGetNodeModule(smiNode);
	if (smiModule && strlen(smiModule->name)) {
	    typeName = getIdlTypeName(smiModule->name, smiNode->name);
	} else {
	    typeName = getBaseTypeString(smiType->basetype);
	}
    } else {
	smiModule = smiGetTypeModule(smiType);
	if (smiModule && strlen(smiModule->name)) {
	    typeName = getIdlTypeName(smiModule->name, smiType->name);
	} else {
	    typeName = getBaseTypeString(smiType->basetype);
	}
    }

    return typeName;
}


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
            sprintf(s, "'%*s'H", 2 * valuePtr->len, "");
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
	break;
    case SMI_BASETYPE_POINTER:
	break;
    case SMI_BASETYPE_OBJECTIDENTIFIER:
	/* TODO */
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



static Import* addImport(char *module, char *name)
{
    Import **import, *newImport;
    
    for (import = &importList; *import; import = &(*import)->nextPtr) {
	int c = strcmp((*import)->module, module);
	if (c < 0) continue;
	if (c == 0) {
	    int d = strcmp((*import)->name, name);
	    if (d < 0) continue;
	    if (d == 0) return *import;
	    if (d > 0) break;
	}
	if (c > 0) break;
    }

    newImport = xmalloc(sizeof(Import));
    newImport->module = module;
    newImport->name = name;
    newImport->nextPtr = *import;
    *import = newImport;
	
    return *import;
}



static void createImportList(SmiModule *smiModule)
{
    SmiNode     *smiNode;
    SmiType     *smiType;
    SmiModule   *smiTypeModule;
    SmiElement  *smiElement;
    SmiNode     *smiNodeIndex;
    SmiNodekind kind = SMI_NODEKIND_SCALAR
	 | SMI_NODEKIND_COLUMN | SMI_NODEKIND_ROW;
    
    for (smiNode = smiGetFirstNode(smiModule, kind);
	 smiNode;
	 smiNode = smiGetNextNode(smiNode, kind)) {

	 switch (smiNode->nodekind) {
	 case SMI_NODEKIND_ROW:
	      for (smiElement = smiGetFirstElement(smiNode); smiElement;
		   smiElement = smiGetNextElement(smiElement)) {
		   smiNodeIndex = smiGetElementNode(smiElement);
		  smiType = smiGetNodeType(smiNodeIndex);
		  if (smiType) {
		      smiTypeModule = smiGetTypeModule(smiType);
		      if (smiTypeModule &&
			  strcmp(smiTypeModule->name, smiModule->name)) {
			  if (strlen(smiTypeModule->name)) {
			      addImport(smiTypeModule->name, smiType->name);
			  }
		      }
		      if (smiType->basetype == SMI_BASETYPE_INTEGER32) {
			   addImport("SNMPv2-SMI", "Integer32");
		      }
		  }
	      }
	      break;
	 case SMI_NODEKIND_SCALAR:
	 case SMI_NODEKIND_COLUMN:
	      smiType = smiGetNodeType(smiNode);
	      if (smiType) {
		   smiTypeModule = smiGetTypeModule(smiType);
		   if (smiTypeModule &&
		       strcmp(smiTypeModule->name, smiModule->name)) {
			if (strlen(smiTypeModule->name)) {
			     addImport(smiTypeModule->name, smiType->name);
			}
		   }
		   if (smiType->basetype == SMI_BASETYPE_INTEGER32) {
			addImport("SNMPv2-SMI", "Integer32");
		   }
	      }
	      break;
	}
    }
}



static void freeImportList(void)
{
    Import *import, *freeme;

    for (import = importList; import; ) {
	freeme = import;
	import = import->nextPtr;
	xfree(freeme);
    }
    importList = NULL;
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



static void fprintMultilineString(FILE *f, const char *s)
{
    int i, len;
    
    fprintSegment(f, INDENTTEXTS - 1, "\"", 0);
    if (s) {
	len = strlen(s);
	for (i=0; i < len; i++) {
	    putc(s[i], f);
	    current_column++;
	    if (s[i] == '\n') {
		current_column = 0;
		fprintSegment(f, INDENTTEXTS, "", 0);
	    }
	}
    }
    putc('\"', f);
    current_column++;
}



static void fprintMultiline(FILE *f, const char *s)
{
    int i, len;
    
    fprintSegment(f, INDENTTEXTS, "", 0);
    if (s) {
	len = strlen(s);
	for (i=0; i < len; i++) {
	    putc(s[i], f);
	    current_column++;
	    if (s[i] == '\n') {
		current_column = 0;
		fprintSegment(f, INDENTTEXTS, "", 0);
	    }
	}
    }
    putc('\n', f);
    current_column++;
}



static char *
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



static void
fprintCommentString(FILE *f, char *s)
{
    int i, len;

    if (s) {
	fprintf(f, " *   ");
	len = strlen(s);
	for (i = 0; i < len; i++) {
	    fputc(s[i], f);
	    if (s[i] == '\n') {
		fprintf(f, " *   ");
	    }
	}
	fputc('\n', f);
    }
}



static char* translate(char *s)
{
    int i;
    
    s = xstrdup(s);
    for (i = 0; s[i]; i++) {
	if (s[i] == '-') s[i] = '_';
    }

    return s;
}



static int isGroup(SmiNode *smiNode)
{
    SmiNode *childNode;
    
    for (childNode = smiGetFirstChildNode(smiNode);
	 childNode;
	 childNode = smiGetNextChildNode(childNode)) {
	if ((childNode->nodekind == SMI_NODEKIND_SCALAR
	     || childNode->nodekind == SMI_NODEKIND_TABLE)
	    && current(childNode->status)) {
	    return 1;
	}
    }

    return 0;
}



static int isCreatable(SmiNode *smiNode)
{
    SmiNode *childNode;
    
    if ((isGroup(smiNode) || smiNode->nodekind == SMI_NODEKIND_ROW)
	&& current(smiNode->status)) {

	for (childNode = smiGetFirstChildNode(smiNode);
	     childNode;
	     childNode = smiGetNextChildNode(childNode)) {
	    
	    if ((childNode->nodekind == SMI_NODEKIND_SCALAR
		 || childNode->nodekind == SMI_NODEKIND_COLUMN)
		&& current(childNode->status)
		&& childNode->access == SMI_ACCESS_READ_WRITE) {
		return 1;
	    }
	}
    }

    return 0;
}



static void fprintDescription(FILE *f, SmiNode *smiNode, int indent)
{
    fprint(f, "\n");
    fprintSegment(f, indent, "/*\n", 0);
    if (smiNode->description) {
	fprintMultiline(f, smiNode->description);
    }
    if (smiNode->reference) {
	fprintSegment(f, indent, "REFERENCE:", 0);
	fprint(f, "\n");
	fprintMultilineString(f, smiNode->reference);
	fprint(f, "\n\n");
    }
    if (smiNode->units) {
	fprintSegment(f, indent, "UNITS:", 0);
	fprint(f, "\n");
	fprintMultilineString(f, smiNode->units);
	fprint(f, "\n\n");
    }
    fprintSegment(f, indent, "*/\n", 0);
}



static void fprintIndex(FILE *f, SmiNode *indexNode)
{
    SmiElement *smiElement;
    int        j;

    for (j = 0, smiElement = smiGetFirstElement(indexNode);
	 smiElement;
	 j++, smiElement = smiGetNextElement(smiElement)) {
	if (j) {
	    fprint(f, " ");
	}
	fprint(f, smiGetElementNode(smiElement)->name);
	/* TODO: non-local name if non-local */
    } /* TODO: empty? -> print error */
}



static void fprintIncludes(FILE *f, SmiModule *smiModule)
{
    Import    *import;
    char      *lastModulename = NULL;
    
    fprint(f, "#include <ASN1Types.idl>\n");
    fprint(f, "#include <SNMPMgmt.idl>\n");

    for (import = importList; import; import = import->nextPtr) {
	if (!lastModulename
	    || strcmp(lastModulename, import->module)) {
	    fprint(f, "#include <%s.idl>\n",
		  getIdlModuleName(import->module));
	    lastModulename = import->module;
	}
    }

    fprint(f, "\n");
}



static void fprintImportedTypedefs(FILE *f, SmiModule *smiModule)
{
    Import    *import;
    int	      cnt = 0;
    char      *idlTypeName;

    for (import = importList; import; import = import->nextPtr) {
	cnt++;
	idlTypeName = getIdlTypeName(import->module, import->name);
	fprintSegment(f, INDENT, "typedef ", 0);
	fprint(f, "%s::%s %s;\n",
	      getIdlModuleName(import->module), idlTypeName, idlTypeName);
    }

    if (cnt) {
	fprint(f, "\n");
    }
}



static void fprintModule(FILE *f, SmiModule *smiModule)
{
    SmiRevision  *smiRevision;
    SmiNode      *smiNode;
    char         *idlModuleName;

    smiNode = smiGetModuleIdentityNode(smiModule);

    if (smiNode) {

	idlModuleName = getIdlModuleName(smiModule->name);
	fprintSegment(f, INDENT, "const ", 0);
	fprint(f, "string moduleIdentity = \"%s\";\n", smiNode->name);
	fprintSegment(f, INDENT, "const ", 0);
	fprint(f, "ASN1_ObjectIdentifier %s = \"::%s::%s\";\n\n",
	      getIdlModuleName(smiNode->name),
	      idlModuleName, smiNode->name);
	if (! silent) {
	    fprintSegment(f, INDENT, "/*\n", 0);
	    if (smiModule->description) {
		fprintMultiline(f, smiModule->description);
		fprint(f, "\n");
	    }
	    smiRevision = smiGetFirstRevision(smiModule);
	    fprintSegment(f, INDENT, "LAST-UPDATED:", INDENTVALUE);
	    fprint(f, smiRevision
		  ? getTimeString(smiRevision->date) : "197001010000Z");
	    fprint(f, "\n\n");
	    fprintSegment(f, INDENT, "ORGANIZATION:", 0);
	    fprint(f, "\n");
	    fprintMultilineString(f, smiModule->organization);
	    fprint(f, "\n\n");
	    fprintSegment(f, INDENT, "CONTACT-INFO:", 0);
	    fprint(f, "\n");
	    fprintMultilineString(f, smiModule->contactinfo);
	    fprint(f, "\n\n");
	    for (; smiRevision;
		 smiRevision = smiGetNextRevision(smiRevision)) {
		if (! smiRevision->description ||
		    strcmp(smiRevision->description,
			   "[Revision added by libsmi due to a LAST-UPDATED clause.]")) {
		    fprintSegment(f, INDENT, "REVISION:", INDENTVALUE);
		    fprint(f, "\"%s\"\n", getTimeString(smiRevision->date));
		    fprintSegment(f, INDENT, "REVISION-DESCRIPTION:", 0);
		    fprint(f, "\n");
		    if (smiRevision->description) {
			fprintMultilineString(f, smiRevision->description);
		    } else {
			fprintMultilineString(f, "...");
		    }
		    fprint(f, "\n\n");
		}
	    }
	    fprintSegment(f, INDENT, "*/", 0);
	    fprint(f, "\n\n");
	}
    }

}



static void fprintType(FILE *f, SmiNode *smiNode, SmiType *smiType)
{
    SmiNamedNumber *nn;
    char           *idlTypeName;
    char           *nnName;
    int            i;

    if (! silent) {
	if (smiType->name) {
	    fprintSegment(f, INDENT, "/*\n", 0);
	    if (smiType->description) {
		fprintMultiline(f, smiType->description);
	    }
	    if (smiType->reference) {
		fprintSegment(f, INDENT, "REFERENCE:", 0);
		fprint(f, "\n");
		fprintMultilineString(f, smiType->reference);
		fprint(f, "\n\n");
	    }
	    if (smiType->format) {
		fprintSegment(f, INDENT, "DISPLAY-HINT:", 0);
		fprint(f, " %s\n", smiType->format);
		fprint(f, "\n\n");
	    }
	    fprintSegment(f, INDENT, "*/\n", 0);
	}
    }
    if (! smiType->name) {
	idlTypeName = getIdlTypeName(smiGetNodeModule(smiNode)->name,
				     smiNode->name);
    } else {
	idlTypeName = getIdlTypeName(smiGetTypeModule(smiType)->name,
				     smiType->name);
    }
    fprintSegment(f, INDENT, "typedef ", 0);
    fprint(f, "%s %s; \n",
	  getBaseTypeString(smiType->basetype), idlTypeName);
    
    if (smiType->basetype == SMI_BASETYPE_ENUM) {
	for (nn = smiGetFirstNamedNumber(smiType);
	     nn;
	     nn = smiGetNextNamedNumber(nn)) {
	    fprintSegment(f, INDENT, "const ", 0);
	    nnName = translate(nn->name);
	    fprint(f, "%s %s_%s = %s;\n", idlTypeName, idlTypeName, nnName,
		  getValueString(&nn->value, smiType));
	    xfree(nnName);
	}
	fprintSegment(f, INDENT, "const string ", 0);
	fprint(f, "%s_NameNumberList = \"", idlTypeName);
	for (i = 0, nn = smiGetFirstNamedNumber(smiType);
	     nn;
	     i++, nn = smiGetNextNamedNumber(nn)) {
	    nnName = translate(nn->name);
	    if (i) {
		fprint(f, " , ");
	    }
	    fprint(f, "%s (%s)", nnName, getValueString(&nn->value, smiType));
	    xfree(nnName);
	}
	fprint(f, "\";\n");
    }
    fprint(f, "\n");
}



static void fprintTypedefs(FILE *f, SmiModule *smiModule)
{
    SmiNode        *smiNode;
    SmiType        *smiType;
    SmiNodekind    kind = SMI_NODEKIND_SCALAR | SMI_NODEKIND_COLUMN;
    
    for (smiType = smiGetFirstType(smiModule);
	 smiType;
	 smiType = smiGetNextType(smiType)) {
	if (current(smiType->status)) {
	    fprintType(f, NULL, smiType);
	}
    }

    for (smiNode = smiGetFirstNode(smiModule, kind);
	 smiNode;
	 smiNode = smiGetNextNode(smiNode, kind)) {
	if (current(smiNode->status)) {
	    smiType = smiGetNodeType(smiNode);
	    if (smiType && ! smiType->name) {
		fprintType(f, smiNode, smiType);
	    }
	}
    }
}



static void fprintAttribute(FILE *f, SmiNode *smiNode)
{
    char *idlTypeName = NULL, *idlNodeName;
    SmiType *smiType;
    SmiModule *smiModule;
    
    if (smiNode->access < SMI_ACCESS_READ_ONLY) {
	return;
    }

    smiType = smiGetNodeType(smiNode);
    smiModule = smiGetNodeModule(smiNode);

    if (! smiType) {
	return;
    }
    
    idlNodeName = getIdlNodeName(smiModule->name, smiNode->name);
    idlTypeName = getIdlAnyTypeName(smiNode, smiType);
    if (! silent) {
	fprintDescription(f, smiNode, 2*INDENT);
    }
    fprintSegment(f, 2*INDENT,
		 smiNode->access == SMI_ACCESS_READ_ONLY
		 ? "readonly attribute" : "attribute", 0);

    fprint(f, " %s %s;\n", idlTypeName, idlNodeName);
}



static void fprintGroupInterface(FILE *f, SmiNode *smiNode)
{
    SmiNode *childNode;
    char *idlNodeName;
    SmiModule *smiModule, *childModule;

    smiModule = smiGetNodeModule(smiNode);
    idlNodeName = getIdlNodeName(smiModule->name, smiNode->name);
    fprintSegment(f, INDENT, "interface", 0);
    fprint(f, " %s : SNMPMgmt::SmiEntry {\n", idlNodeName);

    for (childNode = smiGetFirstChildNode(smiNode);
	 childNode;
	 childNode = smiGetNextChildNode(childNode)) {
	if (childNode->nodekind == SMI_NODEKIND_TABLE
	    && current(childNode->status)) {
	    if (! silent) {
		fprintDescription(f, childNode, 2*INDENT);
	    }
	    fprintSegment(f, 2*INDENT, "SNMPMgmt::SmiTableIterator", 0);
	    childModule = smiGetNodeModule(childNode);
	    fprint(f, " get_%s();\n", getIdlNodeName(childModule->name,
						 childNode->name));
	}
	if (childNode->nodekind == SMI_NODEKIND_SCALAR
	    && current(childNode->status)) {
	    fprintAttribute(f, childNode);
	}
    }

    fprintSegment(f, INDENT, "};\n\n", 0);
}



static void fprintRowInterface(FILE *f, SmiNode *smiNode)
{
    SmiNode *childNode, *relatedNode;
    char *idlModuleName, *idlNodeName;

    idlNodeName = getIdlNodeName(smiGetNodeModule(smiNode)->name,
				 smiNode->name);
    if (! silent) {
	fprintDescription(f, smiNode, INDENT);
    }
    fprintSegment(f, INDENT, "interface", 0);
    if (smiNode->indexkind == SMI_INDEX_AUGMENT
	|| smiNode->indexkind == SMI_INDEX_SPARSE) {
	relatedNode = smiGetRelatedNode(smiNode);
	idlModuleName = getIdlModuleName(smiGetNodeModule(relatedNode)->name);
	fprint(f, " %s : %s::%s {\n", idlNodeName,
	      idlModuleName, relatedNode->name);
    } else {
	fprint(f, " %s : SNMPMgmt::SmiEntry {\n", idlNodeName);	
    }

    if (smiNode->indexkind == SMI_INDEX_INDEX
	|| smiNode->indexkind == SMI_INDEX_REORDER) {
	fprint(f, "\n");
	fprintSegment(f, 2*INDENT, "const ", 0);
	fprint(f, "string IndexVarList = \"");
	fprintIndex(f, smiNode);
	fprint(f, "\";\n");
    }

    /* SMI_INDEX_EXPAND ? */

    for (childNode = smiGetFirstChildNode(smiNode);
	 childNode;
	 childNode = smiGetNextChildNode(childNode)) {
	if (childNode->nodekind == SMI_NODEKIND_COLUMN
	    && current(childNode->status)) {
	    fprintAttribute(f, childNode);
	}
    }
    
    fprintSegment(f, INDENT, "};\n\n", 0);
}



static void fprintInterfaces(FILE *f, SmiModule *smiModule)
{
    SmiNode *smiNode;

    for (smiNode = smiGetFirstNode(smiModule, SMI_NODEKIND_ANY);
	 smiNode;
	 smiNode = smiGetNextNode(smiNode, SMI_NODEKIND_ANY)) {
	if (isGroup(smiNode)) {
	    fprintGroupInterface(f, smiNode);
	}
	if (smiNode->nodekind == SMI_NODEKIND_ROW
	    && current(smiNode->status)) {
	    fprintRowInterface(f, smiNode);
	}
    }
}



static void fprintConstructor(FILE *f, SmiNode *smiNode)
{
    SmiNode *childNode;
    SmiNode *indexNode;
    SmiType *smiType;
    SmiModule *smiModule;
    SmiElement *smiElement = NULL;
    char    *idlNodeName;
    char    *idlChildNodeName, *idlChildTypeName;
    int	    cnt = 0;

    smiModule = smiGetNodeModule(smiNode);
    idlNodeName = getIdlNodeName(smiModule->name, smiNode->name);

    fprint(f, "\n");
    fprintSegment(f, 2*INDENT, "", 0);
    fprint(f, "%s create_%s (\n", idlNodeName, idlNodeName);

    /* First include the INDEXes as parameters to allow row creation
       for rows with not-accesible index objects. */

    if (smiNode->indexkind == SMI_INDEX_INDEX
	|| smiNode->indexkind == SMI_INDEX_REORDER) {
	 for (smiElement = smiGetFirstElement(smiNode);
	      smiElement; smiElement = smiGetNextElement(smiElement)) {
	      cnt++;
	      indexNode = smiGetElementNode(smiElement);
	      idlChildNodeName =
		   getIdlNodeName(smiGetNodeModule(indexNode)->name,
				  indexNode->name);
	      smiType = smiGetNodeType(indexNode);
	      idlChildTypeName = getIdlAnyTypeName(indexNode, smiType);
	      if (cnt > 1) {
		   fprint(f, ",\n");
	      }
	      fprintSegment(f, 3*INDENT, "in ", 0);
	      fprint(f, "%s %s", idlChildTypeName, idlChildNodeName);
	 }
    }
    
    for (childNode = smiGetFirstChildNode(smiNode);
	 childNode;
	 childNode = smiGetNextChildNode(childNode)) {
	
	if ((childNode->nodekind == SMI_NODEKIND_SCALAR
	     || childNode->nodekind == SMI_NODEKIND_COLUMN)
	    && current(childNode->status)
	    && childNode->access == SMI_ACCESS_READ_WRITE) {

	    /* Test if this column is already used as parameter
	       because it is an INDEX of the row. */

	    if (childNode->nodekind == SMI_NODEKIND_SCALAR
		|| childNode->nodekind == SMI_NODEKIND_COLUMN) {
		for (smiElement = smiGetFirstElement(smiNode);
		    smiElement; smiElement = smiGetNextElement(smiElement)) {
		    indexNode = smiGetElementNode(smiElement);
		    if (indexNode == childNode) {
			break;
		    }
		}
	    }

	    if (! smiElement) {
		cnt++;
		idlChildNodeName =
		    getIdlNodeName(smiGetNodeModule(childNode)->name,
				   childNode->name);
		smiType = smiGetNodeType(childNode);
		idlChildTypeName = getIdlAnyTypeName(childNode, smiType);
		if (cnt > 1) {
		    fprint(f, ",\n");
		}
		fprintSegment(f, 3*INDENT, "in ", 0);
		fprint(f, "%s %s", idlChildTypeName, idlChildNodeName);
	    }
	}
    }
    fprint(f, "\n");
    
    fprintSegment(f, 2*INDENT, ") raises (\n", 0);
    fprintSegment(f, 3*INDENT, "SNMPMgmt::AlreadyExists,\n", 0);
    fprintSegment(f, 3*INDENT, "CosLifeCycle::InvalidCriteria,\n", 0);
    fprintSegment(f, 3*INDENT, "CosLifeCycle::CannotMeetCriteria\n", 0);
    fprintSegment(f, 2*INDENT, ");\n", 0);
}



static void fprintFactory(FILE *f, SmiModule *smiModule)
{
    SmiNode *smiNode;
    int	    cnt = 0;

    for (smiNode = smiGetFirstNode(smiModule, SMI_NODEKIND_ANY);
	 smiNode;
	 smiNode = smiGetNextNode(smiNode, SMI_NODEKIND_ANY)) {
	
	if (isCreatable(smiNode)) {
	    cnt++;
	    if (cnt == 1) {
		fprintSegment(f, INDENT, "interface SmiEntryFactory : "
			     "SNMPMgmt::GenericFactory {\n", 0);
	    }
	    fprintConstructor(f, smiNode);
	}
    }

    if (cnt) {
	fprintSegment(f, INDENT, "};\n\n", 0);
    }
}



static void fprintNotificationVBTypes(FILE *f, SmiModule *smiModule)
{
    SmiNode     *smiNode, *listSmiNode;
    SmiElement  *smiElement;
    SmiType	*smiType;
    char	*idlTypeName;
    char	*idlVBTypeName;
    int		isnew;
    
    for (smiNode = smiGetFirstNode(smiModule, SMI_NODEKIND_NOTIFICATION);
	 smiNode;
	 smiNode = smiGetNextNode(smiNode, SMI_NODEKIND_NOTIFICATION)) {

	for (smiElement = smiGetFirstElement(smiNode);
	     smiElement; smiElement = smiGetNextElement(smiElement)) {
	    listSmiNode = smiGetElementNode(smiElement);
	    idlVBTypeName = getIdlVBTypeName(
		                           smiGetNodeModule(listSmiNode)->name,
					   listSmiNode->name, &isnew);
	    if (isnew && listSmiNode) {
		smiType = smiGetNodeType(listSmiNode);
		if (smiType) {
		    idlTypeName = getIdlAnyTypeName(listSmiNode, smiType);
		    fprintSegment(f, INDENT, "struct ", 0);
		    fprint(f, "%s {\n", idlVBTypeName);
		    fprintSegment(f, 2*INDENT, "string var_name;\n", 0);
		    fprintSegment(f, 2*INDENT, "string var_index;\n", 0);
		    fprintSegment(f, 2*INDENT, "", 0);
		    fprint(f, "%s %s;\n", idlTypeName,
			   smiGetElementNode(smiElement)->name);
		    fprintSegment(f, INDENT, "};\n\n", 0);
		}
	    }
	}
    }
}



static void fprintNotificationTypes(FILE *f, SmiModule *smiModule)
{
    SmiNode     *smiNode;
    SmiElement  *smiElement;
    char	*idlTypeName;
    char	*idlVBTypeName;
    
    for (smiNode = smiGetFirstNode(smiModule, SMI_NODEKIND_NOTIFICATION);
	 smiNode;
	 smiNode = smiGetNextNode(smiNode, SMI_NODEKIND_NOTIFICATION)) {

	if ((smiElement = smiGetFirstElement(smiNode))) {
	    idlTypeName = getIdlTypeName(smiGetNodeModule(smiNode)->name,
					 smiNode->name);
	    fprintSegment(f, INDENT, "struct ", 0);
	    fprint(f, "%s {\n", idlTypeName);
	    for (; smiElement; smiElement = smiGetNextElement(smiElement)) {
		idlVBTypeName = getIdlVBTypeName(smiGetNodeModule(
		                          smiGetElementNode(smiElement))->name,
				    smiGetElementNode(smiElement)->name, NULL);
		fprintSegment(f, 2*INDENT, "", 0);
		fprint(f, "%s %s;\n", idlVBTypeName,
		      smiGetElementNode(smiElement)->name);
	    }
	    fprintSegment(f, INDENT, "};\n\n", 0);
	}
    }
}



static void fprintPushNotifications(FILE *f, SmiModule *smiModule)
{
    SmiNode     *smiNode;
    SmiElement  *smiElement;
    char        *idlNodeName;
    char	*idlTypeName;
    int         cnt = 0;

    for (smiNode = smiGetFirstNode(smiModule, SMI_NODEKIND_NOTIFICATION);
	 smiNode;
	 smiNode = smiGetNextNode(smiNode, SMI_NODEKIND_NOTIFICATION)) {

	cnt++;
	if (cnt == 1) {
	    if (! silent) {
		fprintSegment(f, INDENT,
			     "/* typed push event communication */\n", 0);
	    }
	    fprintSegment(f, INDENT, "interface Notifications : ", 0);
	    fprint(f, "SNMPMgmt::Notifications {\n");
	}
	idlNodeName = getIdlNodeName(smiModule->name,
				     smiNode->name);
	if (! silent) {
	    fprintDescription(f, smiNode, 2*INDENT);
	}
	fprintSegment(f, 2*INDENT, "void ", 0);
	fprint(f, "%s (\n", idlNodeName);
	fprintSegment(f, 3*INDENT, "in CosNaming::Name src_entry_name,\n", 0);
	fprintSegment(f, 3*INDENT, "in CORBA::ScopedName event_type,\n", 0);
	fprintSegment(f, 3*INDENT, "in ASN1_GeneralizedTime event_time", 0);
	if ((smiElement = smiGetFirstElement(smiNode))) {
	    idlTypeName = getIdlTypeName(smiModule->name, smiNode->name);
	    fprint(f, ",\n");
	    fprintSegment(f, 3*INDENT, "in ", 0);
	    fprint(f, "%s notification_info", idlTypeName);
	}
	fprint(f, "\n");
	fprintSegment(f, 2*INDENT, ");\n", 0);
    }    

    if (cnt) {
	fprintSegment(f, INDENT, "};\n\n", 0);
    }
}



static void fprintPullNotifications(FILE *f, SmiModule *smiModule)
{
    SmiNode     *smiNode;
    SmiElement  *smiElement;
    int         cnt = 0;
    char        *idlNodeName;
    char	*idlTypeName;

    for (smiNode = smiGetFirstNode(smiModule, SMI_NODEKIND_NOTIFICATION);
	 smiNode;
	 smiNode = smiGetNextNode(smiNode, SMI_NODEKIND_NOTIFICATION)) {

	cnt++;
	if (cnt == 1) {
	    if (! silent) {
		fprintSegment(f, INDENT,
			     "/* typed pull event communication */\n", 0);
	    }
	    fprintSegment(f, INDENT, "interface PullNotifications : ", 0);
	    fprint(f, "SNMPMgmt::PullNotifications {\n");
	}
	idlNodeName = getIdlNodeName(smiModule->name, smiNode->name);
	
	if (! silent) {
	    fprintDescription(f, smiNode, 2*INDENT);
	}
	fprintSegment(f, 2*INDENT, "void ", 0);
	fprint(f, "pull_%s (\n", idlNodeName);
	fprintSegment(f, 3*INDENT, "out CosNaming::Name src_entry_name,\n", 0);
	fprintSegment(f, 3*INDENT, "out CORBA::ScopedName event_type,\n", 0);
	fprintSegment(f, 3*INDENT, "out ASN1_GeneralizedTime event_time", 0);
	if ((smiElement = smiGetFirstElement(smiNode))) {
	    idlTypeName = getIdlTypeName(smiModule->name, smiNode->name);
	    fprint(f, ",\n");
	    fprintSegment(f, 3*INDENT, "out ", 0);
	    fprint(f, "%s notification_info", idlTypeName);
	}
	fprint(f, "\n");
	fprintSegment(f, 2*INDENT, ");\n", 0);
	fprintSegment(f, 2*INDENT, "boolean ", 0);
	fprint(f, "try_%s (\n", idlNodeName);
	fprintSegment(f, 3*INDENT, "out CosNaming::Name src_entry_name,\n", 0);
	fprintSegment(f, 3*INDENT, "out CORBA::ScopedName event_type,\n", 0);
	fprintSegment(f, 3*INDENT, "out ASN1_GeneralizedTime event_time", 0);
	if ((smiElement = smiGetFirstElement(smiNode))) {
	    char *idlTypeName;
	    idlTypeName = getIdlTypeName(smiModule->name, smiNode->name);
	    fprint(f, ",\n");
	    fprintSegment(f, 3*INDENT, "out ", 0);
	    fprint(f, "%s notification_info", idlTypeName);
	}
	fprint(f, "\n");
	fprintSegment(f, 2*INDENT, ");\n", 0);
    }    

    if (cnt) {
	fprintSegment(f, INDENT, "};\n\n", 0);
    }
}



static void fprintDefVals(FILE *f, SmiModule *smiModule)
{
    SmiNode *smiNode;
    SmiType *smiType;
    int     cnt = 0;
    char    *idlTypeName;
    
    
    for (smiNode = smiGetFirstNode(smiModule, SMI_NODEKIND_ANY);
	 smiNode;
	 smiNode = smiGetNextNode(smiNode, SMI_NODEKIND_ANY)) {
	
	if (smiNode->value.basetype != SMI_BASETYPE_UNKNOWN) {
	    smiType = smiGetNodeType(smiNode);
	    if (smiType) {
		cnt++;
		if (cnt == 1) {
		    fprintSegment(f, INDENT, "/* pseudo */\n", 0);
		    fprintSegment(f, INDENT, "interface DefaultValues {\n", 0);
		}
		if (! silent) {
		    fprintSegment(f, 2*INDENT, "/* DEFVAL: ", 0);
		    fprint(f, " %s */\n",
			   getValueString(&smiNode->value, smiType));
		}
		fprintSegment(f, 2*INDENT, "", 0);
		idlTypeName = getIdlAnyTypeName(smiNode, smiType);
		fprint(f, "%s %s();\n\n", idlTypeName, smiNode->name);
	    }
	}
    }

    if (cnt) {
	fprintSegment(f, INDENT, "};\n\n", 0);
    }
}



static void fprintDisplayHints(FILE *f, SmiModule *smiModule)
{
    SmiType *smiType;
    int     cnt = 0;

    for (smiType = smiGetFirstType(smiModule);
	 smiType;
	 smiType = smiGetNextType(smiType)) {
	if (current(smiType->status) && smiType->format) {
	    cnt++;
	    if (cnt == 1) {
		fprintSegment(f, INDENT, "/* pseudo */\n", 0);
		fprintSegment(f, INDENT, "interface TextualConventions {\n", 0);
	    }
	    fprint(f, "\n");
	    if (! silent) {
		fprintSegment(f, 2*INDENT, "/*\n", 0);
		if (smiType->description) {
		    fprintMultiline(f, smiType->description);
		}
		if (smiType->reference) {
		    fprintSegment(f, 2*INDENT, "REFERENCE:", 0);
		    fprint(f, "\n");
		    fprintMultilineString(f, smiType->reference);
		}
		if (smiType->format) {
		    fprintSegment(f, 2*INDENT, "DISPLAY-HINT:", 0);
		    fprint(f, " %s\n", smiType->format);
		}
		fprintSegment(f, 2*INDENT, "*/\n", 0);
	    }
	    fprintSegment(f, 2*INDENT, "", 0);
	    fprint(f, "string %sToString (in %s Value);\n", smiType->name,
		  getIdlTypeName(smiGetTypeModule(smiType)->name,
				 smiType->name));
	    fprintSegment(f, 2*INDENT, "", 0);
	    fprint(f, "%s %sFromString (in string str);\n",
		  getIdlTypeName(smiGetTypeModule(smiType)->name,
				 smiType->name),
		  smiType->name);
	}
    }

    if (cnt) {
	fprintSegment(f, INDENT, "};\n\n", 0);
    }
}



static void dumpIdl(SmiModule *smiModule)
{
    char        *idlModuleName;
    FILE        *f;
    SmiRevision *smiRevision;
    char        *date;

    f = createFile(getIdlModuleName(smiModule->name), ".idl");
    if (! f) {
        return;
    }

    fprintf(f,
	    "/*	\t\t\t\t\t\t-- DO NOT EDIT --\n"
	    " * Generated by smidump version " SMI_VERSION_STRING ":\n");

    fprintf(f, " *   smidump -f corba %s\n *\n", smiModule->name);

    fprintf(f,
	    " * Derived from %s:\n", smiModule->name);
    fprintCommentString(f, smiModule->description);

    for (smiRevision = smiGetFirstRevision(smiModule);
	 smiRevision;
	 smiRevision = smiGetNextRevision(smiRevision)) {
	date = getStringTime(smiRevision->date);
	fprintf(f,
		" *\n"
		" * Revision %s:\n", date);
	fprintCommentString(f, smiRevision->description);
    }
    
    fprintf(f,
	    " *\n * $I" "d$\n"
	    " */\n"
	    "\n");

    idlModuleName = getIdlModuleName(smiModule->name);
    createImportList(smiModule);

    fprint(f, "#ifndef _%s_IDL_\n", idlModuleName);
    fprint(f, "#define _%s_IDL_\n\n", idlModuleName);

    fprintIncludes(f, smiModule);

    fprint(f, "module %s {\n\n", idlModuleName);

    fprintImportedTypedefs(f, smiModule);
    fprintModule(f, smiModule);
    fprintTypedefs(f, smiModule);
    fprintInterfaces(f, smiModule);
    fprintNotificationVBTypes(f, smiModule);
    fprintNotificationTypes(f, smiModule);
    fprintPushNotifications(f, smiModule);
    fprintPullNotifications(f, smiModule);
    fprintFactory(f, smiModule);
    fprintDefVals(f, smiModule);
    fprintDisplayHints(f, smiModule);
    
    fprint(f, "};\n\n");
    fprint(f, "#endif /* !_%s_IDL_ */\n", idlModuleName);

    freeImportList();
    dictFree(&idlModuleNameList);
    dictFree(&idlNodeNameList);
    dictFree(&idlTypeNameList);
    dictFree(&idlVBTypeNameList);

    if (fflush(f) || ferror(f)) {
	perror("smidump: write error");
	exit(1);
    }

    fclose(f);
}



static void fprintNameAndOid(FILE *f, SmiNode *smiNode, SmiNode *smiParentNode)
{
    unsigned int i;
    char         *idlModuleName;

    idlModuleName = getIdlModuleName(smiGetNodeModule(smiNode)->name);

    if (smiParentNode) {
	fprint(f, "::%s::%s::%s ",
	       idlModuleName, smiParentNode->name, smiNode->name);
    } else {
	fprint(f, "::%s::%s ", idlModuleName, smiNode->name);
    }
    for (i = 0; i < smiNode->oidlen; i++) {
	fprint(f, "%s%u", i ? "." : "", smiNode->oid[i]);
    }
    fprint(f, " ");
}



static void dumpOid(SmiModule *smiModule)
{
    SmiNode   *smiNode;
    SmiType   *smiType;
    FILE      *f;

    f = createFile(getIdlModuleName(smiModule->name), ".oid");
    if (! f) {
        return;
    }

    for (smiNode = smiGetFirstNode(smiModule, SMI_NODEKIND_ANY);
	 smiNode;
	 smiNode = smiGetNextNode(smiNode, SMI_NODEKIND_ANY)) {

	if (isGroup(smiNode)) {
            fprintNameAndOid(f, smiNode, NULL);
	    fprint(f, "Group not-accessible\n");
	    /* XXX what if the node is also of SMI_NODEKIND_MODULE ?? */
	    continue;
	}

	smiType = smiGetNodeType(smiNode);
	
	switch (smiNode->nodekind) {
	case SMI_NODEKIND_NODE:
	    if (current(smiNode->status)) {
		fprintNameAndOid(f, smiNode, NULL);
		fprint(f, "ASN1_ObjectIdentifier not-accessible\n");
	    }
	    break;
	case SMI_NODEKIND_SCALAR:
	    if (smiType && current(smiNode->status)) {
		SmiNode *smiParentNode = smiGetParentNode(smiNode);
		fprintNameAndOid(f, smiNode, smiParentNode);
		fprint(f, "%s %s\n",
		       getBaseTypeString(smiType->basetype),
		       getAccessString(smiNode->access, 0));
	    }
	    break;
	case SMI_NODEKIND_TABLE:
	    if (current(smiNode->status)) {
		fprintNameAndOid(f, smiNode, NULL);
		fprint(f, "Table not-accessible\n");
	    }
	    break;
	case SMI_NODEKIND_ROW:
	    if (current(smiNode->status)) {
		fprintNameAndOid(f, smiNode, NULL);
		fprint(f, "TableEntry not-accessible\n");
	    }
	    break;
	case SMI_NODEKIND_COLUMN:
	    if (smiType && current(smiNode->status)) {
		SmiNode *smiParentNode = smiGetParentNode(smiNode);
		int create = smiParentNode ? smiParentNode->create : 0;
		fprintNameAndOid(f, smiNode, smiParentNode);
		fprint(f, "%s %s\n",
		       getBaseTypeString(smiType->basetype),
		       getAccessString(smiNode->access, create));
	    }
	    break;
	case SMI_NODEKIND_NOTIFICATION:
	    if (current(smiNode->status)) {
		SmiNode *smiParentNode = smiGetParentNode(smiNode);
		fprintNameAndOid(f, smiNode, smiParentNode);
		fprint(f, "Notification not-accessible\n");
	    }
	    break;
	case SMI_NODEKIND_GROUP:
	    break;
	case SMI_NODEKIND_COMPLIANCE:
	    break;
	}
    }

    dictFree(&idlModuleNameList);

    if (fflush(f) || ferror(f)) {
	perror("smidump: write error");
	exit(1);
    }

    fclose(f);
}


static void dumpCorba(int modc, SmiModule **modv, int flags, char *output)
{
    int       i;

    silent = (flags & SMIDUMP_FLAG_SILENT);

    for (i = 0; i < modc; i++) {
	dumpIdl(modv[i]);
	dumpOid(modv[i]);
    }
}



void initCorba()
{
    
    static SmidumpDriver driver = {
	"corba",
	dumpCorba,
	0,
	SMIDUMP_DRIVER_CANT_UNITE | SMIDUMP_DRIVER_CANT_OUTPUT,
	"corba IDL interface and OID definitions (JIDM)",
	NULL,
	NULL
    };
    
    smidumpRegisterDriver(&driver);
}
