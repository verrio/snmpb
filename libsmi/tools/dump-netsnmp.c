/*
 * dump-netsnmp.c --
 *
 *      Operations to generate NET-SNMP mib module implementation code.
 *
 * Copyright (c) 1999 Frank Strauss, Technical University of Braunschweig.
 * Copyright (c) 1999 J. Schoenwaelder, Technical University of Braunschweig.
 *
 * See the file "COPYING" for information on usage and redistribution
 * of this file, and for a DISCLAIMER OF ALL WARRANTIES.
 *
 * @(#) $Id: dump-netsnmp.c 8090 2008-04-18 12:56:29Z strauss $
 */

/*
 * TODO:
 *	  - assume that we build a dynamic loadable module
 *	  - update to 4.X version of the UCD API
 *	  - generate #defines for deprecated and obsolete objects
 *	  - generate stub codes for the various functions
 *	  - generate type and range checking code
 */

#include <config.h>

#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <ctype.h>
#ifdef HAVE_UNISTD_H
#include <unistd.h>
#endif
#ifdef HAVE_WIN_H
#include "win.h"
#endif

#include "smi.h"
#include "smidump.h"


static int noMgrStubs = 0;
static int noAgtStubs = 0;


static char *getAccessString(SmiAccess access)
{
    if (access == SMI_ACCESS_READ_WRITE) {
	return "RWRITE";
    } else if (access == SMI_ACCESS_READ_ONLY) {
	return "RONLY";
    } else {
	return "";
    }
}


static char *getBaseTypeString(SmiBasetype basetype)
{
    switch(basetype) {
    case SMI_BASETYPE_UNKNOWN:
	return "ASN_NULL";
    case SMI_BASETYPE_POINTER:
	return "ASN_NULL";
    case SMI_BASETYPE_INTEGER32:
    case SMI_BASETYPE_ENUM:
	return "ASN_INTEGER";
    case SMI_BASETYPE_OCTETSTRING:
    case SMI_BASETYPE_BITS:
	return "ASN_OCTET_STR";
    case SMI_BASETYPE_OBJECTIDENTIFIER:
	return "ASN_OBJECT_ID";
    case SMI_BASETYPE_UNSIGNED32:
	return "ASN_INTEGER";
    case SMI_BASETYPE_INTEGER64:
	return "ASN_INTEGER";
    case SMI_BASETYPE_UNSIGNED64:
	return "ASN_INTEGER";
    case SMI_BASETYPE_FLOAT32:
    case SMI_BASETYPE_FLOAT64:
    case SMI_BASETYPE_FLOAT128:
	return "ASN_Real";
    }

    return NULL;
}



static char* translate(char *m)
{
    char *s;
    int i;

    s = xstrdup(m);
    for (i = 0; s[i]; i++) {
	if (s[i] == '-') s[i] = '_';
    }
  
    return s;
}



static char* translateUpper(char *m)
{
    char *s;
    int i;

    s = xstrdup(m);
    for (i = 0; s[i]; i++) {
	if (s[i] == '-') s[i] = '_';
	if (islower((int) s[i])) {
	    s[i] = toupper(s[i]);
	}
    }
  
    return s;
}



static char* translateLower(char *m)
{
    char *s;
    int i;

    s = xstrdup(m);
    for (i = 0; s[i]; i++) {
	if (s[i] == '-') s[i] = '_';
	if (isupper((int) s[i])) {
	    s[i] = tolower(s[i]);
	}
    }
  
    return s;
}



static char* translateFileName(char *m)
{
    char *s;
    int i;

    s = xstrdup(m);
    for (i = 0; s[i]; i++) {
	if (s[i] == '_') s[i] = '-';
	if (isupper((int) s[i])) {
	    s[i] = tolower(s[i]);
	}
    }
  
    return s;
}



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



static int isGroup(SmiNode *smiNode)
{
    SmiNode *childNode;

    if (smiNode->nodekind == SMI_NODEKIND_ROW) {
	return 1;
    }
    
    for (childNode = smiGetFirstChildNode(smiNode);
	 childNode;
	 childNode = smiGetNextChildNode(childNode)) {
	if (childNode->nodekind == SMI_NODEKIND_SCALAR) {
	    return 1;
	}
    }

    return 0;
}



static int isAccessible(SmiNode *groupNode)
{
    SmiNode *smiNode;
    int num = 0;
    
    for (smiNode = smiGetFirstChildNode(groupNode);
	 smiNode;
	 smiNode = smiGetNextChildNode(smiNode)) {
	if ((smiNode->nodekind == SMI_NODEKIND_SCALAR
	     || smiNode->nodekind == SMI_NODEKIND_COLUMN)
	    && (smiNode->access == SMI_ACCESS_READ_ONLY
		|| smiNode->access == SMI_ACCESS_READ_WRITE)) {
	    num++;
	}
    }

    return num;
}



static void printHeaderTypedef(FILE *f, SmiModule *smiModule,
			       SmiNode *groupNode)
{
    SmiNode *smiNode;
    SmiType *smiType;
    char    *cModuleName, *cGroupName, *cName;
    unsigned minSize, maxSize;

    cModuleName = translateLower(smiModule->name);
    cGroupName = translate(groupNode->name);

    fprintf(f,
	    "/*\n"
	    " * C type definitions for %s::%s.\n"
	    " */\n\n",
	    smiModule->name, groupNode->name);
    
    fprintf(f, "typedef struct %s {\n", cGroupName);
	    
    for (smiNode = smiGetFirstChildNode(groupNode);
	 smiNode;
	 smiNode = smiGetNextChildNode(smiNode)) {
	if (smiNode->nodekind & (SMI_NODEKIND_COLUMN | SMI_NODEKIND_SCALAR)
#if 0
	    && (smiNode->access == SMI_ACCESS_READ_ONLY
		|| smiNode->access == SMI_ACCESS_READ_WRITE)
#endif
	    ) {
	    smiType = smiGetNodeType(smiNode);
	    if (!smiType) {
		continue;
	    }
	    
	    cName = translate(smiNode->name);
	    switch (smiType->basetype) {
	    case SMI_BASETYPE_OBJECTIDENTIFIER:
		maxSize = smiGetMaxSize(smiType);
		minSize = smiGetMinSize(smiType);
		fprintf(f,
			"    uint32_t  *%s;\n", cName);
		if (maxSize != minSize) {
		    fprintf(f,
			    "    size_t    _%sLength;\n", cName);
		}
		break;
	    case SMI_BASETYPE_OCTETSTRING:
	    case SMI_BASETYPE_BITS:
		maxSize = smiGetMaxSize(smiType);
		minSize = smiGetMinSize(smiType);
		fprintf(f,
			"    u_char    *%s;\n", cName);
		if (maxSize != minSize) {
		    fprintf(f,
			    "    size_t    _%sLength;\n", cName);
		}
		break;
	    case SMI_BASETYPE_ENUM:
	    case SMI_BASETYPE_INTEGER32:
		fprintf(f,
			"    int32_t   *%s;\n", cName);
		break;
	    case SMI_BASETYPE_UNSIGNED32:
		fprintf(f,
			"    uint32_t  *%s;\n", cName);
		break;
	    case SMI_BASETYPE_INTEGER64:
		fprintf(f,
			"    int64_t   *%s; \n", cName);
		break;
	    case SMI_BASETYPE_UNSIGNED64:
		fprintf(f,
			"    uint64_t  *%s; \n", cName);
		break;
	    default:
		fprintf(f,
			"    /* ?? */  __%s; \n", cName);
		break;
	    }
	    xfree(cName);
	}
    }
    
    fprintf(f,
	    "    void      *_clientData;\t\t"
	    "/* pointer to client data structure */\n");
    if (groupNode->nodekind == SMI_NODEKIND_ROW) {
	fprintf(f, "    struct %s *_nextPtr;\t"
		"/* pointer to next table entry */\n", cGroupName);
    }
    fprintf(f,
	    "\n    /* private space to hold actual values */\n\n");

    for (smiNode = smiGetFirstChildNode(groupNode);
	 smiNode;
	 smiNode = smiGetNextChildNode(smiNode)) {
	if (smiNode->nodekind & (SMI_NODEKIND_COLUMN | SMI_NODEKIND_SCALAR)
#if 0
	    && (smiNode->access == SMI_ACCESS_READ_ONLY
		|| smiNode->access == SMI_ACCESS_READ_WRITE)
#endif
	    ) {
	    smiType = smiGetNodeType(smiNode);
	    if (!smiType) {
		continue;
	    }
	    
	    cName = translate(smiNode->name);
	    switch (smiType->basetype) {
	    case SMI_BASETYPE_OBJECTIDENTIFIER:
		maxSize = smiGetMaxSize(smiType);
		fprintf(f,
			"    uint32_t  __%s[%u];\n", cName, maxSize);
		break;
	    case SMI_BASETYPE_OCTETSTRING:
	    case SMI_BASETYPE_BITS:
		maxSize = smiGetMaxSize(smiType);
		fprintf(f,
			"    u_char    __%s[%u];\n", cName, maxSize);
		break;
	    case SMI_BASETYPE_ENUM:
	    case SMI_BASETYPE_INTEGER32:
		fprintf(f,
			"    int32_t   __%s;\n", cName);
		break;
	    case SMI_BASETYPE_UNSIGNED32:
		fprintf(f,
			"    uint32_t  __%s;\n", cName);
		break;
	    case SMI_BASETYPE_INTEGER64:
		fprintf(f,
			"    int64_t   __%s; \n", cName);
		break;
	    case SMI_BASETYPE_UNSIGNED64:
		fprintf(f,
			"    uint64_t  __%s; \n", cName);
		break;
	    default:
		fprintf(f,
			"    /* ?? */  __%s; \n", cName);
		break;
	    }
	    xfree(cName);
	}
    }

    fprintf(f, "} %s_t;\n\n", cGroupName);

    fprintf(f,
	    "/*\n"
	    " * C manager interface stubs for %s::%s.\n"
	    " */\n\n",
	    smiModule->name, groupNode->name);
	    
    fprintf(f, "extern int\n"
	    "%s_mgr_get_%s(struct snmp_session *s, %s_t **%s);\n",
	    cModuleName, cGroupName, cGroupName, cGroupName);
    fprintf(f, "\n");

    fprintf(f,
	    "/*\n"
	    " * C agent interface stubs for %s::%s.\n"
	    " */\n\n",
	    smiModule->name, groupNode->name);
    
    fprintf(f, "extern int\n"
	    "%s_agt_read_%s(%s_t *%s);\n",
	    cModuleName, cGroupName, cGroupName, cGroupName);
    fprintf(f, "extern int\n"
	    "%s_agt_register_%s();\n\n",
	    cModuleName, cGroupName);
    xfree(cGroupName);
    xfree(cModuleName);
}



static void printHeaderTypedefs(FILE *f, SmiModule *smiModule)
{
    SmiNode   *smiNode;
    int       cnt = 0;
    char      *cModuleName;
    char      *cSmiNodeName;
    
    for (smiNode = smiGetFirstNode(smiModule, SMI_NODEKIND_ANY);
	 smiNode;
	 smiNode = smiGetNextNode(smiNode, SMI_NODEKIND_ANY)) {
	if (isGroup(smiNode) && isAccessible(smiNode)) {
	    cnt++;
	    printHeaderTypedef(f, smiModule, smiNode);
	}
    }
    
    if (cnt) {
	fprintf(f, "\n");
    }

    if (cnt) {
	/*
	 * Should this go into the agent implementation module?
	 */
	cModuleName = translateLower(smiModule->name);
	fprintf(f, "typedef struct %s {\n", cModuleName);
	for (smiNode = smiGetFirstNode(smiModule, SMI_NODEKIND_ANY);
	     smiNode;
	     smiNode = smiGetNextNode(smiNode, SMI_NODEKIND_ANY)) {
	    if (isGroup(smiNode) && isAccessible(smiNode)) {
		cSmiNodeName = translate(smiNode->name);
		if (smiNode->nodekind == SMI_NODEKIND_ROW) {
		    fprintf(f, "    %s_t\t*%s;\n", cSmiNodeName, cSmiNodeName);
		} else {
		    fprintf(f, "    %s_t\t%s;\n", cSmiNodeName, cSmiNodeName);
		}
		xfree(cSmiNodeName);
	    }
	}
	fprintf(f, "} %s_t;\n\n", cModuleName);
	xfree(cModuleName);
    }
}



static void dumpHeader(SmiModule *smiModule, char *baseName)
{
    char	*pModuleName;
    char	*cModuleName;
    FILE	*f;

    pModuleName = translateUpper(smiModule->name);

    f = createFile(baseName, ".h");
    if (! f) {
	return;
    }
    
    fprintf(f,
	    "/*\n"
	    " * This C header file has been generated by smidump "
	    SMI_VERSION_STRING ".\n"
	    " * It is intended to be used with the NET-SNMP package.\n"
	    " *\n"
	    " * This header is derived from the %s module.\n"
	    " *\n * $I" "d$\n"
	    " */\n\n", smiModule->name);

    fprintf(f, "#ifndef _%s_H_\n", pModuleName);
    fprintf(f, "#define _%s_H_\n\n", pModuleName);

    fprintf(f, "#include <stdlib.h>\n\n");

    fprintf(f,
	    "#ifdef HAVE_STDINT_H\n"
	    "#include <stdint.h>\n"
	    "#endif\n\n");

    printHeaderTypedefs(f, smiModule);

    fprintf(f,
	    "/*\n"
	    " * Initialization function:\n"
	    " */\n\n");
    cModuleName = translateLower(smiModule->name);
    fprintf(f, "void %s_agt_init(void);\n\n", cModuleName);
    xfree(cModuleName);

    fprintf(f, "#endif /* _%s_H_ */\n", pModuleName);

    fclose(f);
    xfree(pModuleName);
}



static void printAgtReadMethodDecls(FILE *f, SmiModule *smiModule)
{
    SmiNode   *smiNode;
    int       cnt = 0;

    for (smiNode = smiGetFirstNode(smiModule, SMI_NODEKIND_ANY);
	 smiNode;
	 smiNode = smiGetNextNode(smiNode, SMI_NODEKIND_ANY)) {
	if (isGroup(smiNode) && isAccessible(smiNode)) {
	    cnt++;
	    if (cnt == 1) {
		fprintf(f,
			"/*\n"
			" * Forward declaration of read methods for groups of scalars and tables:\n"
			" */\n\n");
	    }
	    fprintf(f,
		    "static unsigned char *\nread_%s_stub(struct variable *,"
		    " oid *, size_t *, int, size_t *, WriteMethod **);\n",
		    smiNode->name);
	}
    }
    
    if (cnt) {
	fprintf(f, "\n");
    }
}



static void printAgtWriteMethodDecls(FILE *f, SmiModule *smiModule)
{
    SmiNode     *smiNode;
    int         cnt = 0;
    
    for (smiNode = smiGetFirstNode(smiModule, SMI_NODEKIND_ANY);
	 smiNode;
	 smiNode = smiGetNextNode(smiNode, SMI_NODEKIND_ANY)) {
	if (smiNode->access == SMI_ACCESS_READ_WRITE) {
	    cnt++;
	    if (cnt == 1) {
		fprintf(f,
			"/*\n"
			" * Forward declaration of write methods for writable objects:\n"
			" */\n\n");
	    }
	    fprintf(f,
		    "static int\nwrite_%s_stub(int,"
		    " u_char *, u_char, int, u_char *, oid *, int);\n",
		    smiNode->name);
	}
    }
    
    if (cnt) {
	fprintf(f, "\n");
    }
}



static void printAgtDefinesGroup(FILE *f, SmiNode *groupNode, int cnt)
{
    char         *cName, *cGroupName;
    SmiNode   	 *smiNode;
    SmiType   	 *smiType;
    int	      	 num = 0;
    unsigned int i;
    
    if (cnt == 1) {
	fprintf(f,
	"/*\n"
	" * Definitions of tags that are used internally to read/write\n"
	" * the selected object type. These tags should be unique.\n"
	" */\n\n");
    }

    cGroupName = translate(groupNode->name);

    for (smiNode = smiGetFirstChildNode(groupNode);
	 smiNode;
	 smiNode = smiGetNextChildNode(smiNode)) {
	if (smiNode->nodekind & (SMI_NODEKIND_COLUMN | SMI_NODEKIND_SCALAR)
	    && (smiNode->access == SMI_ACCESS_READ_ONLY
		|| smiNode->access == SMI_ACCESS_READ_WRITE)) {
	    num++;
	    cName = translateUpper(smiNode->name);
	    fprintf(f, "#define %-32s %d\n", cName,
		    smiNode->oid[smiNode->oidlen-1]);
	    xfree(cName);
	}
    }
    fprintf(f, "\n");

    if (num) {
	fprintf(f, "static oid %s_base[] = {", cGroupName);
	for (i = 0; i < groupNode->oidlen; i++) {
	    fprintf(f, "%s%d", i ? ", " : "", groupNode->oid[i]);
	}
	fprintf(f, "};\n\n");
	fprintf(f, "struct variable %s_variables[] = {\n", cGroupName);
	for (smiNode = smiGetFirstChildNode(groupNode);
	     smiNode;
	     smiNode = smiGetNextChildNode(smiNode)) {
	    if (smiNode->nodekind & (SMI_NODEKIND_COLUMN | SMI_NODEKIND_SCALAR)
		&& (smiNode->access == SMI_ACCESS_READ_ONLY
		    || smiNode->access == SMI_ACCESS_READ_WRITE)) {
		smiType = smiGetNodeType(smiNode);
		if (!smiType) {
		    continue;
		}
		cName = translateUpper(smiNode->name);
		fprintf(f, "    { %s, %s, %s, read_%s_stub, %d, {%d} },\n",
			cName, getBaseTypeString(smiType->basetype),
			getAccessString(smiNode->access),
			cGroupName, 1, smiNode->oid[smiNode->oidlen-1]);
		xfree(cName);
	    }
	}
	fprintf(f, "};\n\n");
    }

    xfree(cGroupName);
}



static void printAgtDefines(FILE *f, SmiModule *smiModule)
{
    SmiNode   *smiNode;
    int       cnt = 0;
    
    for (smiNode = smiGetFirstNode(smiModule, SMI_NODEKIND_ANY);
	 smiNode;
	 smiNode = smiGetNextNode(smiNode, SMI_NODEKIND_ANY)) {
	if (isGroup(smiNode)) {
	    printAgtDefinesGroup(f, smiNode, ++cnt);
	}
    }
    
    if (cnt) {
	fprintf(f, "\n");
    }
}



static void printAgtRegister(FILE *f, SmiNode *groupNode, int cnt)
{
    SmiNode *smiNode;
    char    *cGroupName;
    int     num = 0;

    for (smiNode = smiGetFirstChildNode(groupNode);
	 smiNode;
	 smiNode = smiGetNextChildNode(smiNode)) {
	if (smiNode->nodekind & (SMI_NODEKIND_COLUMN | SMI_NODEKIND_SCALAR)
	    && (smiNode->access == SMI_ACCESS_READ_ONLY
		|| smiNode->access == SMI_ACCESS_READ_WRITE)) {
	    num++;
	}
    }
    fprintf(f, "\n");

    if (cnt == 1) {
	fprintf(f,
		"/*\n"
		" * Registration functions for the various MIB groups.\n"
		" */\n\n");
    }
    
    cGroupName = translate(groupNode->name);

    fprintf(f, "int register_%s()\n{\n", cGroupName);
    fprintf(f,
	    "    return register_mib(\"%s\",\n"
	    "         %s_variables,\n"
	    "         sizeof(struct variable),\n"
	    "         sizeof(%s_variables)/sizeof(struct variable),\n"
	    "         %s_base,\n"
	    "         sizeof(%s_base)/sizeof(oid));\n",
	    cGroupName, cGroupName, cGroupName, cGroupName, cGroupName);
    fprintf(f, "};\n\n");

    xfree(cGroupName);
}



static void printAgtInit(FILE *f, SmiModule *smiModule)
{
    SmiNode   *smiNode;
    int       cnt = 0;

    for (smiNode = smiGetFirstNode(smiModule, SMI_NODEKIND_ANY);
	 smiNode;
	 smiNode = smiGetNextNode(smiNode, SMI_NODEKIND_ANY)) {
	if (isGroup(smiNode)) {
	    printAgtRegister(f, smiNode, ++cnt);
	}
    }

    if (cnt) {
	fprintf(f, "\n");
    }
}



static void printAgtReadMethod(FILE *f, SmiNode *groupNode)
{
    SmiNode   *smiNode;
    SmiType   *smiType;
    char      *cName, *sName, *lName;

    sName = translate(groupNode->name);

    fprintf(f,
	    "static unsigned char *\nread_%s_stub(struct variable *vp,\n"
	    "    oid     *name,\n"
	    "    size_t  *length,\n"
	    "    int     exact,\n"
	    "    size_t  *var_len,\n"
	    "    WriteMethod **write_method)\n"
	    "{\n", sName);

    fprintf(f, "    static %s_t %s;\n\n", sName, sName);
    
    smiNode = smiGetFirstChildNode(groupNode);
    if (smiNode && smiNode->nodekind == SMI_NODEKIND_SCALAR) {
	fprintf(f,
		"    /* check whether the instance identifier is valid */\n"
		"\n"
		"    if (header_generic(vp, name, length, exact, var_len,\n"
		"                       write_method) == MATCH_FAILED) {\n"
		"        return NULL;\n"
		"    }\n"
		"\n");
    }

    fprintf(f,
	    "    /* call the user supplied function to retrieve values */\n"
	    "\n"
	    "    read_%s(&%s);\n"
	    "\n", sName, sName);

    fprintf(f,
	    "    /* return the current value of the variable */\n"
	    "\n"
	    "    switch (vp->magic) {\n"
	    "\n");

    for (smiNode = smiGetFirstChildNode(groupNode);
	 smiNode;
	 smiNode = smiGetNextChildNode(smiNode)) {
	if (smiNode->nodekind & (SMI_NODEKIND_COLUMN | SMI_NODEKIND_SCALAR)
	    && (smiNode->access == SMI_ACCESS_READ_ONLY
		|| smiNode->access == SMI_ACCESS_READ_WRITE)) {
	    cName = translateUpper(smiNode->name);
	    lName = translate(smiNode->name);
	    smiType = smiGetNodeType(smiNode);
	    if (! smiType) {
		continue;
	    }
	    fprintf(f, "    case %s:\n", cName);
	    switch (smiType->basetype) {
	    case SMI_BASETYPE_OBJECTIDENTIFIER:
		fprintf(f,
			"        *var_len = %s._%sLength;\n"
			"        return (unsigned char *) %s.%s;\n",
			sName, lName, sName, lName);
		break;
	    case SMI_BASETYPE_OCTETSTRING:
	    case SMI_BASETYPE_BITS:
		fprintf(f,
			"        *var_len = %s._%sLength;\n"
			"        return (unsigned char *) %s.%s;\n",
			sName, lName, sName, lName);
		break;
	    case SMI_BASETYPE_ENUM:
	    case SMI_BASETYPE_INTEGER32:
	    case SMI_BASETYPE_UNSIGNED32:
		fprintf(f,
			"        return (unsigned char *) &%s.%s;\n",
			sName, lName);
		break;
	    default:
		fprintf(f,
			"        /* add code to return the value here */\n");
	    }
	    fprintf(f, "\n");
	    xfree(cName);
	    xfree(lName);
	}
    }

    fprintf(f,
	    "    default:\n"
	    "         ERROR_MSG(\"\");\n"
	    "    }\n"
	    "\n"
	    "    return NULL;\n"
	    "}\n"
	    "\n");

    xfree(sName);
}



static void printAgtReadMethods(FILE *f, SmiModule *smiModule)
{
    SmiNode   *smiNode;
    int       cnt = 0;
    
    for (smiNode = smiGetFirstNode(smiModule, SMI_NODEKIND_ANY);
	 smiNode;
	 smiNode = smiGetNextNode(smiNode, SMI_NODEKIND_ANY)) {
	if (isGroup(smiNode) && isAccessible(smiNode)) {
	    cnt++;
	    if (cnt == 1) {
		fprintf(f,
			"/*\n"
			" * Read methods for groups of scalars and tables:\n"
			" */\n\n");
	    }
	    printAgtReadMethod(f, smiNode);
	}
    }
    
    if (cnt) {
	fprintf(f, "\n");
    }
}



static void printAgtWriteMethods(FILE *f, SmiModule *smiModule)
{
    SmiNode     *smiNode;
    int         cnt = 0;
    
    for (smiNode = smiGetFirstNode(smiModule, SMI_NODEKIND_ANY);
	 smiNode;
	 smiNode = smiGetNextNode(smiNode, SMI_NODEKIND_ANY)) {
	if (smiNode->access == SMI_ACCESS_READ_WRITE) {
	    cnt++;
	    if (cnt == 1) {
		fprintf(f,
			"/*\n"
			" * Forward declaration of write methods for writable objects:\n"
			" */\n\n");
	    }
	    fprintf(f,
		    "static int\nwrite_%s_stub(int action,\n"
		    "    u_char   *var_val,\n"
		    "    u_char   var_val_type,\n"
		    "    int      var_val_len,\n"
		    "    u_char   *statP,\n"
		    "    oid      *name,\n"
		    "    int      name_len)\n"
		    "{\n", smiNode->name);
	    fprintf(f,
		    "    return SNMP_ERR_NOERROR;\n"
		    "}\n\n");
	}
    }
    
    if (cnt) {
	fprintf(f, "\n");
    }
}



static void dumpAgtStub(SmiModule *smiModule, char *baseName)
{
    char	*stubModuleName;
    FILE	*f;

    stubModuleName = xmalloc(strlen(baseName) + 10);
    strcpy(stubModuleName, baseName);
    strcat(stubModuleName, "-agt-stub");
    
    f = createFile(stubModuleName, ".c");
    if (! f) {
	xfree(stubModuleName);
        return;
    }

    fprintf(f,
	    "/*\n"
	    " * This C file has been generated by smidump "
	    SMI_VERSION_STRING ".\n"
	    " * It is intended to be used with the NET-SNMP agent library.\n"
	    " *\n"
	    " * This C file is derived from the %s module.\n"
	    " *\n * $I" "d$\n"
	    " */\n\n", smiModule->name );
	
    fprintf(f,
	    "#include <stdio.h>\n"
	    "#include <string.h>\n"
	    "#include <malloc.h>\n"
	    "\n"
	    "#include \"%s.h\"\n"
	    "\n"
	    "#include <ucd-snmp/asn1.h>\n"
	    "#include <ucd-snmp/snmp.h>\n"
	    "#include <ucd-snmp/snmp_api.h>\n"
	    "#include <ucd-snmp/snmp_impl.h>\n"
	    "#include <ucd-snmp/snmp_vars.h>\n"
	    "\n",
	    baseName);

    printAgtReadMethodDecls(f, smiModule);
    printAgtWriteMethodDecls(f, smiModule);
    printAgtDefines(f, smiModule);
    printAgtInit(f, smiModule);

    printAgtReadMethods(f, smiModule);
    printAgtWriteMethods(f, smiModule);

    fclose(f);
    xfree(stubModuleName);
}



static void printMgrOidDefinitions(FILE *f, SmiModule *smiModule)
{
    SmiNode      *smiNode;
    char         *cName;
    unsigned int i;
    
    for (smiNode = smiGetFirstNode(smiModule, SMI_NODEKIND_ANY);
	 smiNode;
	 smiNode = smiGetNextNode(smiNode, SMI_NODEKIND_ANY)) {
	if (smiNode->nodekind & (SMI_NODEKIND_COLUMN | SMI_NODEKIND_SCALAR)
	    && smiNode->access != SMI_ACCESS_NOTIFY) {
	    cName = translate(smiNode->name);
  	    fprintf(f, "static oid %s[] = {", cName);
	    for (i = 0; i < smiNode->oidlen; i++) {
		fprintf(f, "%s%u", i ? ", " : "", smiNode->oid[i]);
	    }
	    fprintf(f, "};\n");
	    xfree(cName);
	}
    }
    fprintf(f, "\n");
}



static void printMgrGetScalarAssignement(FILE *f, SmiNode *groupNode)
{
    SmiNode *smiNode;
    SmiType *smiType;
    char    *cGroupName, *cName;
    unsigned maxSize, minSize;

    cGroupName = translate(groupNode->name);

    for (smiNode = smiGetFirstChildNode(groupNode);
	 smiNode;
	 smiNode = smiGetNextChildNode(smiNode)) {
	if (smiNode->nodekind & (SMI_NODEKIND_COLUMN | SMI_NODEKIND_SCALAR)
	    && (smiNode->access == SMI_ACCESS_READ_ONLY
		|| smiNode->access == SMI_ACCESS_READ_WRITE)) {

	    smiType = smiGetNodeType(smiNode);
	    if (!smiType) {
		continue;
	    }
	    
	    cName = translate(smiNode->name);
	    fprintf(f,
		    "        if (vars->name_length > sizeof(%s)/sizeof(oid)\n"
		    "            && memcmp(vars->name, %s, sizeof(%s)) == 0) {\n",
		    cName, cName, cName);
	    switch (smiType->basetype) {
	    case SMI_BASETYPE_INTEGER32:
	    case SMI_BASETYPE_UNSIGNED32:
	    case SMI_BASETYPE_ENUM:
		fprintf(f,
			"            (*%s)->__%s = *vars->val.integer;\n"
			"            (*%s)->%s = &((*%s)->__%s);\n",
			cGroupName, cName,
			cGroupName, cName, cGroupName, cName);
		break;
	    case SMI_BASETYPE_OCTETSTRING:
	    case SMI_BASETYPE_BITS:
		maxSize = smiGetMaxSize(smiType);
		minSize = smiGetMinSize(smiType);
		fprintf(f,
			"            memcpy((*%s)->__%s, vars->val.string, vars->val_len);\n",
			cGroupName, cName);
		if (minSize != maxSize) {
		    fprintf(f,
			    "            (*%s)->_%sLength = vars->val_len;\n",
			    cGroupName, cName);
		}
		fprintf(f,
			"            (*%s)->%s = (*%s)->__%s;\n",
			cGroupName, cName, cGroupName, cName);
		break;
	    case SMI_BASETYPE_OBJECTIDENTIFIER:
		break;
	    default:
		break;
	    }
	    fprintf(f,
		    "        }\n");
	    xfree(cName);
	}
    }

    xfree(cGroupName);
}



static void printMgrGetMethod(FILE *f, SmiModule *smiModule,
			      SmiNode *groupNode)
{
    SmiNode *smiNode;
    char    *cModuleName, *cGroupName;

    cModuleName = translateLower(smiModule->name);
    cGroupName = translate(groupNode->name);

    fprintf(f,
	    "int %s_mgr_get_%s(struct snmp_session *s, %s_t **%s)\n"
	    "{\n"
	    "    struct snmp_session *peer;\n"
	    "    struct snmp_pdu *request, *response;\n"
	    "    struct variable_list *vars;\n"
	    "    int status;\n"
	    "\n",
	    cModuleName, cGroupName, cGroupName, cGroupName);

    fprintf(f,
	    "    request = snmp_pdu_create(SNMP_MSG_GETNEXT);\n");
	    
    for (smiNode = smiGetFirstChildNode(groupNode);
	 smiNode;
	 smiNode = smiGetNextChildNode(smiNode)) {
	if (smiNode->nodekind & (SMI_NODEKIND_COLUMN | SMI_NODEKIND_SCALAR)
	    && (smiNode->access == SMI_ACCESS_READ_ONLY
		|| smiNode->access == SMI_ACCESS_READ_WRITE)) {
	    fprintf(f,
	    "    snmp_add_null_var(request, %s, sizeof(%s)/sizeof(oid));\n",
		    smiNode->name, smiNode->name);
	}
    }

    fprintf(f,
	    "\n"
	    "    peer = snmp_open(s);\n"
	    "    if (!peer) {\n"
	    "        snmp_free_pdu(request);\n"
	    "        return -1;\n"
	    "    }\n"
	    "\n"
	    "    status = snmp_synch_response(peer, request, &response);\n"
	    "    if (status != STAT_SUCCESS) {\n"
	    "        if (response) snmp_free_pdu(response);\n"
	    "        snmp_close(peer);\n"
	    "        return -2;\n"
	    "    }\n"
	    "\n");

    /* generate code for error checking and handling */

    fprintf(f,
	    "    *%s = (%s_t *) malloc(sizeof(%s_t));\n"
	    "    if (! *%s) {\n"
	    "        if (response) snmp_free_pdu(response);\n"
	    "        snmp_close(peer);\n"
	    "        return -4;\n"
	    "    }\n"
	    "\n",
	    cGroupName, cGroupName, cGroupName, cGroupName);

    fprintf(f,
	    "    for (vars = response->variables; vars; vars = vars->next_variable) {\n");
    printMgrGetScalarAssignement(f, groupNode);
    fprintf(f,
	    "    }\n"
	    "\n");


#if 0
    if (response->errstat != SNMP_ERR_NOERROR) {
	return -3;
    }

    /* copy to data structures */

    /* cleanup */

#endif

    fprintf(f,
	    "    if (response) snmp_free_pdu(response);\n"
	    "\n"
	    "    if (snmp_close(peer) == 0) {\n"
	    "        return -5;\n"
	    "    }\n"
	    "\n"
	    "    return 0;\n"
	    "}\n\n");

    xfree(cGroupName);
    xfree(cModuleName);
}
 



static void printMgrGetMethods(FILE *f, SmiModule *smiModule)
{
    SmiNode   *smiNode;
    int       cnt = 0;
    
    for (smiNode = smiGetFirstNode(smiModule, SMI_NODEKIND_ANY);
	 smiNode;
	 smiNode = smiGetNextNode(smiNode, SMI_NODEKIND_ANY)) {
	if (isGroup(smiNode) && isAccessible(smiNode)) {
	    cnt++;
	    printMgrGetMethod(f, smiModule, smiNode);
	}
    }
    
    if (cnt) {
	fprintf(f, "\n");
    }
}



static void dumpMgrStub(SmiModule *smiModule, char *baseName)
{
    char	*stubModuleName;
    FILE	*f;

    stubModuleName = xmalloc(strlen(baseName) + 10);
    strcpy(stubModuleName, baseName);
    strcat(stubModuleName, "-mgr-stub");
    
    f = createFile(stubModuleName, ".c");
    if (! f) {
	xfree(stubModuleName);
        return;
    }

    fprintf(f,
	    "/*\n"
	    " * This C file has been generated by smidump "
	    SMI_VERSION_STRING ".\n"
	    " * It is intended to be used with the NET-SNMP library.\n"
	    " *\n"
	    " * This C file is derived from the %s module.\n"
	    " *\n * $I" "d$\n"
	    " */\n\n", smiModule->name );
	
    fprintf(f,
	    "#include <stdlib.h>\n"
	    "\n"
	    "#include <ucd-snmp/asn1.h>\n"
	    "#include <ucd-snmp/snmp.h>\n"
	    "#include <ucd-snmp/snmp_api.h>\n"
	    "#include <ucd-snmp/snmp_client.h>\n"
	    "\n"
	    "#include \"%s.h\"\n"
	    "\n",
	    baseName);

    printMgrOidDefinitions(f, smiModule);
    
    printMgrGetMethods(f, smiModule);
    
    if (fflush(f) || ferror(f)) {
	perror("smidump: write error");
	exit(1);
    }

    fclose(f);
    xfree(stubModuleName);
}



static void dumpAgtImpl(SmiModule *smiModule, char *baseName)
{
    char	*stubModuleName, *cModuleName;
    FILE	*f;

    stubModuleName = xmalloc(strlen(baseName) + 10);
    strcpy(stubModuleName, baseName);
    strcat(stubModuleName, "-agt");
    

    f = createFile(stubModuleName, ".c");
    if (! f) {
	xfree(stubModuleName);
        return;
    }

    cModuleName = translateLower(smiModule->name);

    fprintf(f,
	    "/*\n"
	    " * This C file has been generated by smidump "
	    SMI_VERSION_STRING ".\n"
	    " * It is intended to be used with the NET-SNMP agent library.\n"
	    " *\n"
	    " * This C file is derived from the %s module.\n"
	    " *\n * $I" "d$\n"
	    " */\n\n", smiModule->name );
	
    fprintf(f,
	    "#include <stdio.h>\n"
	    "#include <string.h>\n"
	    "#include <malloc.h>\n"
	    "\n"
	    "#include \"%s.h\"\n"
	    "\n"
	    "#include <ucd-snmp/asn1.h>\n"
	    "#include <ucd-snmp/snmp.h>\n"
	    "#include <ucd-snmp/snmp_api.h>\n"
	    "#include <ucd-snmp/snmp_impl.h>\n"
	    "#include <ucd-snmp/snmp_vars.h>\n"
	    "\n",
	    baseName);

    fprintf(f,
	    "static oid %s_caps[] = {0,0};\n"
	    "\n",
	    cModuleName);

    fprintf(f,
	    "void init_%s(void)\n"
	    "{\n"
#if 0
	    /* create an entry in the sysORTable */
	    
	    register_sysORTable(if_mib_caps, sizeof(if_mib_caps),
				"IF-MIB implementation version 0.0.");
	    
	    /* register the various parts of the MIB */
	    
	    register_interfaces();
	    register_ifEntry();
	    
	    /* register essential callbacks */
	    
	    snmp_register_callback(SNMP_CALLBACK_LIBRARY,
				   SNMP_CALLBACK_SHUTDOWN,
				   term_if_mib, NULL);
#endif
	    "}\n"
	    "\n",
	    cModuleName);


    fprintf(f,
	    "void deinit_%s()\n"
	    "{\n"
	    "    unregister_sysORTable(%s_caps, sizeof(%s_caps));\n"
	    "}\n"
	    "\n",
	    cModuleName, cModuleName, cModuleName);

    fprintf(f,
	    "int term_%s()\n"
	    "{\n"
	    "    deinit_%s();\n"
	    "    return 0;\n"
	    "}\n"
	    "\n",
	    cModuleName, cModuleName);

    xfree(cModuleName);
    
    if (fflush(f) || ferror(f)) {
	perror("smidump: write error");
	exit(1);
    }

    fclose(f);
    xfree(stubModuleName);
}



static void dumpNetSnmp(int modc, SmiModule **modv, int flags, char *output)
{
    char	*baseName;
    int		i;

    if (flags & SMIDUMP_FLAG_UNITE) {
	/* not implemented yet */
    } else {
	for (i = 0; i < modc; i++) {
	    baseName = output ? output : translateFileName(modv[i]->name);
	    dumpHeader(modv[i], baseName);
	    if (! noAgtStubs) {
		dumpAgtStub(modv[i], baseName);
		dumpAgtImpl(modv[i], baseName);
	    }
	    if (! noMgrStubs) {
		dumpMgrStub(modv[i], baseName);
	    }
	    if (! output) xfree(baseName);
	}
    }

}



void initNetsnmp()
{
    static SmidumpDriverOption opt[] = {
	{ "no-mgr-stubs", OPT_FLAG, &noMgrStubs, 0,
	  "do not generate manager stub code"},
	{ "no-agt-stubs", OPT_FLAG, &noAgtStubs, 0,
	  "do not generate agent stub code"},
        { 0, OPT_END, 0, 0 }
    };

    static SmidumpDriver driver = {
	"netsnmp",
	dumpNetSnmp,
	SMI_FLAG_NODESCR,
	SMIDUMP_DRIVER_CANT_UNITE,
	"ANSI C code for the NET-SNMP package",
	opt,
	NULL
    };

    smidumpRegisterDriver(&driver);
}
