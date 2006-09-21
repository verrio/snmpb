/*
 * dump-jax.c --
 *
 *      Operations to generate JAX AgentX class files.
 *
 * Copyright (c) 2000 Frank Strauss, Technical University of Braunschweig.
 *
 * See the file "COPYING" for information on usage and redistribution
 * of this file, and for a DISCLAIMER OF ALL WARRANTIES.
 *
 * @(#) $Id: dump-jax.c 1455 2002-10-30 09:17:37Z schoenw $
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



static char *package = NULL;



static struct {
    SmiBasetype basetype;
    char        *smitype;
    char        *javatype;
    char        *agentxtype;
} convertType[] = {
    { SMI_BASETYPE_OCTETSTRING,      "Opaque",    "byte[]",    "OPAQUE" },
    { SMI_BASETYPE_UNSIGNED32,       "TimeTicks", "long",      "TIMETICKS" },
    { SMI_BASETYPE_UNSIGNED32,       "Counter",   "long",      "COUNTER32" },
    { SMI_BASETYPE_UNSIGNED32,       "Counter32", "long",      "COUNTER32" },
    { SMI_BASETYPE_OCTETSTRING,      "IpAddress", "byte[]",    "IPADDRESS" },
    { SMI_BASETYPE_INTEGER32,        NULL,        "int",       "INTEGER" },
    { SMI_BASETYPE_OCTETSTRING,      NULL,        "byte[]",    "OCTETSTRING" },
    { SMI_BASETYPE_OBJECTIDENTIFIER, NULL,        "AgentXOID", "OBJECTIDENTIFIER" },
    { SMI_BASETYPE_UNSIGNED32,       NULL,        "long",      "GAUGE32" },
    { SMI_BASETYPE_INTEGER64,        NULL,        "long",      "INTEGER" },
    { SMI_BASETYPE_UNSIGNED64,       NULL,        "long",      "COUNTER64" },
    { SMI_BASETYPE_ENUM,             NULL,        "int",       "INTEGER" },
    { SMI_BASETYPE_BITS,             NULL,        "byte[]",    "OCTETSTRING" },
    { SMI_BASETYPE_UNKNOWN,          NULL,        NULL,        NULL }
};



static char* translate1Upper(char *m)
{
    static char *sarray[5];
    static int spos = 0;
    char *s;
    int i;

    s = sarray[spos];
    spos++;
    if (spos == 5) spos = 0;
    if (s) xfree(s);
    s = xstrdup(m);
    for (i = 0; s[i]; i++) {
        if (s[i] == '-') s[i] = '_';
    }
    if (islower((int) s[0])) {
        s[0] = toupper(s[0]);
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



static char *getJavaType(SmiType *smiType)
{
    int i;

    if (smiType) {
    	for(i=0; convertType[i].basetype != SMI_BASETYPE_UNKNOWN; i++) {
	    if (smiType->basetype == convertType[i].basetype)
		return convertType[i].javatype;
	}
    }

    return "<UNKNOWN>";
}



static char *getAgentXType(SmiType *smiType)
{
    int i;
    SmiType *parentType;
    SmiModule *smiModule;

    if (smiType) {
	
	parentType = smiGetParentType(smiType);
	if (parentType) {
	    smiModule = smiGetTypeModule(parentType);
	    if (smiModule && strlen(smiModule->name)) {
		smiType = parentType;
	    }
	}
	
	for(i=0; convertType[i].basetype != SMI_BASETYPE_UNKNOWN; i++) {
	    if (smiType->basetype == convertType[i].basetype) {
		if (!convertType[i].smitype) {
		    return convertType[i].agentxtype;
		}
		if ((smiType->name) &&
		    (!strcmp(convertType[i].smitype, smiType->name))) {
		    return convertType[i].agentxtype;
		}
	    }
	}
    }

    return "<UNKNOWN>";
}



static int isGroup(SmiNode *smiNode)
{
    SmiNode *childNode;

    if (smiNode->nodekind == SMI_NODEKIND_ROW) {
        return 1;
    }
    
    for(childNode = smiGetFirstChildNode(smiNode);
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



static unsigned int getMaxSize(SmiType *smiType)
{
    SmiRange *smiRange;
    SmiType  *parentType;
    unsigned int max = 0, size;
    
    switch (smiType->basetype) {
    case SMI_BASETYPE_OCTETSTRING:
        size = 65535;
        break;
    case SMI_BASETYPE_OBJECTIDENTIFIER:
        size = 128;
        break;
    default:
        return 0xffffffff;
    }

    for(smiRange = smiGetFirstRange(smiType);
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
        unsigned int psize = getMaxSize(parentType);
        if (psize < size) {
            size = psize;
        }
    }

    return size;
}



static void dumpTable(SmiNode *smiNode)
{
    FILE *f;
    SmiNode *parentNode, *columnNode;
    unsigned int i;
    char *vb_type;

    parentNode = smiGetParentNode(smiNode);

    f = createFile(translate1Upper(parentNode->name), ".java");
    if (! f) {
        return;
    }

    fprintf(f,
            "/*\n"
            " * This Java file has been generated by smidump "
                SMI_VERSION_STRING "." " Do not edit!\n"
            " * It is intended to be used within a Java AgentX sub-agent"
                " environment.\n"
            " *\n"
            " * $I" "d$\n"
            " */\n\n");

    fprintf(f,
            "/**\n"
            "    This class represents a Java AgentX (JAX) implementation of\n"
            "    the table %s defined in %s.\n"
            "\n"
            "    @version 1\n"
            "    @author  smidump " SMI_VERSION_STRING "\n"
            "    @see     AgentXTable\n"
            " */\n\n", parentNode->name, smiGetNodeModule(smiNode)->name);

    if (package) {
	fprintf(f,
		"package %s;\n\n", package);
    }
    
    fprintf(f,
            "import java.util.Vector;\n"
            "\n"
            "import jax.AgentXOID;\n"
            "import jax.AgentXVarBind;\n"
            "import jax.AgentXResponsePDU;\n"
            "import jax.AgentXSetPhase;\n"
            "import jax.AgentXTable;\n"
            "import jax.AgentXEntry;\n"
            "\n");

    fprintf(f, "public class %s extends AgentXTable\n{\n\n",
            translate1Upper(parentNode->name));

    fprintf(f,
            "    // entry OID\n"
            "    private final static long[] OID = {");
    for (i = 0; i < smiNode->oidlen; i++) {
        fprintf(f, "%s%d", i ? ", " : "", smiNode->oid[i]);
    }
    fprintf(f, "};\n\n");

    fprintf(f,
            "    // constructors\n"
            "    public %s()\n", translate1Upper(parentNode->name));
    fprintf(f,
            "    {\n"
            "        oid = new AgentXOID(OID);\n"
            "\n"
            "        // register implemented columns\n");
    for (columnNode = smiGetFirstChildNode(smiNode);
         columnNode;
         columnNode = smiGetNextChildNode(columnNode)) {
        if (columnNode->access >= SMI_ACCESS_READ_ONLY) {
            fprintf(f,
                    "        columns.addElement(new Long(%d));\n",
                    columnNode->oid[columnNode->oidlen-1]);
        }
    }
    fprintf(f,
            "    }\n\n");
    
    fprintf(f,
            "    public %s(boolean shared)\n",
	    translate1Upper(parentNode->name));
    fprintf(f,
            "    {\n"
            "        super(shared);\n"
	    "\n"
            "        oid = new AgentXOID(OID);\n"
            "\n"
            "        // register implemented columns\n");
    for (columnNode = smiGetFirstChildNode(smiNode);
         columnNode;
         columnNode = smiGetNextChildNode(columnNode)) {
        if (columnNode->access >= SMI_ACCESS_READ_ONLY) {
            fprintf(f,
                    "        columns.addElement(new Long(%d));\n",
                    columnNode->oid[columnNode->oidlen-1]);
        }
    }
    fprintf(f,
            "    }\n\n");
    
    fprintf(f,
            "    public AgentXVarBind getVarBind(AgentXEntry entry,"
                 " long column)\n");
    fprintf(f,
            "    {\n"
            "        AgentXOID oid = new AgentXOID(getOID(), column, entry.getInstance());\n"
            "\n"
            "        switch ((int)column) {\n");

    for (columnNode = smiGetFirstChildNode(smiNode);
         columnNode;
         columnNode = smiGetNextChildNode(columnNode)) {
        if (columnNode->access >= SMI_ACCESS_NOTIFY) {
            fprintf(f,
                    "        case %d: // %s\n",
                    columnNode->oid[columnNode->oidlen-1],
                    columnNode->name);
            fprintf(f,
                    "        {\n");
            fprintf(f,
                    "            %s value = ((%s)entry).get_%s();\n",
                    getJavaType(smiGetNodeType(columnNode)),
                    translate1Upper(smiNode->name),
                    columnNode->name);
            fprintf(f,
                    "            return new AgentXVarBind(oid, "
                    "AgentXVarBind.%s, value);\n",
                    getAgentXType(smiGetNodeType(columnNode)));
            
            fprintf(f,
                    "        }\n");
        }
    }

    fprintf(f,
            "        }\n"
            "\n"
            "        return null;\n"
            "    }\n\n");

    fprintf(f,
            "    public int setEntry(AgentXSetPhase phase,\n"
            "                        AgentXEntry entry,\n"
            "                        long column,\n"
            "                        AgentXVarBind vb)\n");
    fprintf(f,
            "    {\n"
            "\n"
            "        switch ((int)column) {\n");
    
    for (columnNode = smiGetFirstChildNode(smiNode);
         columnNode;
         columnNode = smiGetNextChildNode(columnNode)) {
        if (columnNode->access >= SMI_ACCESS_READ_WRITE) {
            fprintf(f,
                    "        case %d: // %s\n",
                    columnNode->oid[columnNode->oidlen-1],
                    columnNode->name);
            fprintf(f,
                    "        {\n");
            fprintf(f,
                    "            if (vb.getType() != AgentXVarBind.%s)\n"
                    "                return AgentXResponsePDU.WRONG_TYPE;\n",
                    getAgentXType(smiGetNodeType(columnNode)));      
            vb_type = getJavaType(smiGetNodeType(columnNode));
            vb_type = strcmp("byte[]", vb_type) ? vb_type : "bytes";
            fprintf(f,
                    "            else\n"
                    "                return ((%s)entry).set_%s(phase, vb.%sValue());\n",
                    translate1Upper(smiNode->name),
                    columnNode->name,
                    vb_type);
            
            fprintf(f,
                    "        }\n");
        }
    }

    fprintf(f,
            "        }\n"
            "\n"
            "        return AgentXResponsePDU.NOT_WRITABLE;\n"
            "    }\n\n");

    fprintf(f,
            "}\n\n");

    if (fflush(f) || ferror(f)) {
	perror("smidump: write error");
	exit(1);
    }

    fclose(f);
}



static void dumpEntry(SmiNode *smiNode)
{
    FILE *f;
    SmiNode *columnNode, *indexNode;
    SmiType *smiType;
    SmiRange *smiRange;
    SmiElement *element;
    int cnt;
    char *p;
    char init[20];

    f = createFile(translate1Upper(smiNode->name), ".java");
    if (! f) {
        return;
    }

    fprintf(f,
            "/*\n"
            " * This Java file has been generated by smidump "
                SMI_VERSION_STRING "." " Do not edit!\n"
            " * It is intended to be used within a Java AgentX sub-agent"
                " environment.\n"
            " *\n"
            " * $I" "d$\n"
            " */\n\n");

    fprintf(f,
            "/**\n"
            "    This class represents a Java AgentX (JAX) implementation of\n"
            "    the table row %s defined in %s.\n"
            "\n"
            "    @version 1\n"
            "    @author  smidump " SMI_VERSION_STRING "\n"
            "    @see     AgentXTable, AgentXEntry\n"
            " */\n\n", smiNode->name, smiGetNodeModule(smiNode)->name);
    
    if (package) {
	fprintf(f,
		"package %s;\n\n", package);
    }
    
    fprintf(f,
            "import jax.AgentXOID;\n"
            "import jax.AgentXSetPhase;\n"
            "import jax.AgentXResponsePDU;\n"
            "import jax.AgentXEntry;\n"
            "\n");

    fprintf(f, "public class %s extends AgentXEntry\n{\n\n",
            translate1Upper(smiNode->name));

    for (columnNode = smiGetFirstChildNode(smiNode);
         columnNode;
         columnNode = smiGetNextChildNode(columnNode)) {
        
        smiType = smiGetNodeType(columnNode);
	if (!smiType) continue;
        p = getJavaType(smiType);
        if (!strcmp(p, "long")) {
            strcpy(init, "0");
        } else if (!strcmp(p, "int")) {
            strcpy(init, "0");
        } else if (!strcmp(p, "byte[]")) {
            smiRange = smiGetFirstRange(smiType);
            if ((smiRange && (!smiGetNextRange(smiRange)) &&
                 (!memcmp(&smiRange->minValue, &smiRange->maxValue,
                          sizeof(SmiValue))))) {
                sprintf(init, "new byte[%ld]",
                        smiRange->maxValue.value.integer32);
            } else {
                sprintf(init, "new byte[0]");
            }
        } else if (!strcmp(p, "AgentXOID")) {
            strcpy(init, "new AgentXOID()");
        } else {
            strcpy(init, "null");
        }
        fprintf(f,
                "    protected %s %s = %s;\n",
                getJavaType(smiGetNodeType(columnNode)),
                columnNode->name,
                init);

        if (columnNode->access == SMI_ACCESS_READ_WRITE) {
            fprintf(f,
                    "    protected %s undo_%s = %s;\n",
                    getJavaType(smiGetNodeType(columnNode)),
                    columnNode->name,
                    init);
        }
    }
    for (element = smiGetFirstElement(smiNode), cnt = 0;
         element;
         element = smiGetNextElement(element)) {
        indexNode = smiGetElementNode(element);
        for (columnNode = smiGetFirstChildNode(smiNode);
             columnNode;
             columnNode = smiGetNextChildNode(columnNode)) {
            if (!strcmp(columnNode->name, indexNode->name))
                break;
        }
        if (!columnNode) {
            if (!cnt) {
                fprintf(f, "    // foreign indices\n");
            }
            cnt++;
            fprintf(f, "    protected %s %s;\n",
                    getJavaType(smiGetNodeType(indexNode)),
                    indexNode->name);
        }
    }
    fprintf(f, "\n");
    
    fprintf(f,
            "    public %s(", translate1Upper(smiNode->name));
    for (element = smiGetFirstElement(smiNode), cnt = 0;
         element;
         element = smiGetNextElement(element)) {
        if (cnt) {
            fprintf(f, ",\n%*s", 4 + 7 + 1 + strlen(smiNode->name), " ");
        }
        cnt++;
        indexNode = smiGetElementNode(element);
        fprintf(f, "%s %s",
                getJavaType(smiGetNodeType(indexNode)),
                indexNode->name);
    }
    fprintf(f, ")\n"
            "    {\n");
    for (element = smiGetFirstElement(smiNode);
         element;
         element = smiGetNextElement(element)) {
        indexNode = smiGetElementNode(element);
        fprintf(f, "        this.%s = %s;\n",
                indexNode->name, indexNode->name);
    }
    fprintf(f, "\n");
    for (element = smiGetFirstElement(smiNode);
         element;
         element = smiGetNextElement(element)) {
        indexNode = smiGetElementNode(element);

        p = getJavaType(smiGetNodeType(indexNode));
        if (!strcmp(p, "long")) {
            fprintf(f, "        instance.append(%s);\n",
                    indexNode->name);
        } else if (!strcmp(p, "int")) {
            fprintf(f, "        instance.append(%s);\n",
                    indexNode->name);
        } else if (!strcmp(p, "byte[]")) {
            smiType = smiGetNodeType(indexNode);
            smiRange = smiGetFirstRange(smiType);
            if ((smiRange && (!smiGetNextRange(smiRange)) &&
                 (!memcmp(&smiRange->minValue, &smiRange->maxValue,
                          sizeof(SmiValue)))) ||
                (smiNode->implied && (!smiGetNextElement(element)))) {
                fprintf(f, "        instance.appendImplied(%s);\n",
                        indexNode->name);
            } else {
                fprintf(f, "        instance.append(%s);\n",
                        indexNode->name);
            }
        } else if (!strcmp(p, "AgentXOID")) {
            if (smiNode->implied && (!smiGetNextElement(element))) {
                fprintf(f, "        instance.appendImplied(%s);\n",
                        indexNode->name);
            } else {
                fprintf(f, "        instance.append(%s);\n",
                        indexNode->name);
            }
        } else {
            fprintf(f, "        // [smidump: type of %s not supported]\n",
                    indexNode->name);
        }
    }
    
    fprintf(f,
            "    }\n"
            "\n");

    for (element = smiGetFirstElement(smiNode);
         element;
         element = smiGetNextElement(element)) {
        indexNode = smiGetElementNode(element);
        
        fprintf(f,
                "    public %s get_%s()\n"
                "    {\n"
                "        return %s;\n"
                "    }\n"
                "\n",
                getJavaType(smiGetNodeType(indexNode)),
                indexNode->name, indexNode->name);
    }

    for (columnNode = smiGetFirstChildNode(smiNode);
         columnNode;
         columnNode = smiGetNextChildNode(columnNode)) {
        smiType = smiGetNodeType(columnNode);
        if (columnNode->access >= SMI_ACCESS_NOTIFY) {
	    for (element = smiGetFirstElement(smiNode);
		 element;
		 element = smiGetNextElement(element)) {
		indexNode = smiGetElementNode(element);
		if (indexNode == columnNode) break;
	    }
	    if (!element) {
		fprintf(f,
			"    public %s get_%s()\n"
			"    {\n"
			"        return %s;\n"
			"    }\n"
			"\n",
			getJavaType(smiType),
			columnNode->name, columnNode->name);
	    }
        }
        if (columnNode->access == SMI_ACCESS_READ_WRITE) {
            fprintf(f,
                    "    public int set_%s(AgentXSetPhase phase, %s value)\n"
                    "    {\n",
                    columnNode->name,
                    getJavaType(smiGetNodeType(columnNode)));
            fprintf(f,
                    "        switch (phase.getPhase()) {\n"
                    "        case AgentXSetPhase.TEST_SET:\n"
                    "            break;\n"                     
                    "        case AgentXSetPhase.COMMIT:\n"
                    "            undo_%s = %s;\n",
                    columnNode->name,
                    columnNode->name);
            if (!strcmp("byte[]", getJavaType(smiGetNodeType(columnNode)))) {
                fprintf(f,
                        "            %s = new byte[value.length];\n"
                        "            for(int i = 0; i < value.length; i++)\n"
                        "                %s[i] = value[i];\n",
                        columnNode->name,
                        columnNode->name);
            } else {
                fprintf(f,
                        "            %s = value;\n",
                        columnNode->name);
            }

            fprintf(f,
                    "            break;\n"                     
                    "        case AgentXSetPhase.UNDO:\n"
                    "            %s = undo_%s;\n"
                    "            break;\n",
                    columnNode->name,
                    columnNode->name);
            fprintf(f,
                    "        case AgentXSetPhase.CLEANUP:\n");
            if (!strcmp("byte[]",getJavaType(smiGetNodeType(columnNode)))) {
                fprintf(f,
                        "            undo_%s = null;\n",
                        columnNode->name);
            }
            fprintf(f,
                    "            break;\n"
                    "        default:\n"
                    "            return AgentXResponsePDU.PROCESSING_ERROR;\n"
                    "        }\n"
                    "        return AgentXResponsePDU.NO_ERROR;\n"
                    "    }\n");
        }
    }
    
    fprintf(f,
            "}\n\n");

    if (fflush(f) || ferror(f)) {
	perror("smidump: write error");
	exit(1);
    }

    fclose(f);
}

static void dumpEntryImpl(SmiNode *smiNode)
{
    FILE *f;
    SmiNode *columnNode, *indexNode;
    SmiType *smiType;
    SmiElement *element;
    int cnt;

    f = createFile(translate1Upper(smiNode->name), "Impl.java");
    if (! f) {
        return;
    }

    fprintf(f,
            "/*\n"
            " * This Java file has been generated by smidump "
                SMI_VERSION_STRING ". It\n"
            " * is intended to be edited by the application programmer and\n"
            " * to be used within a Java AgentX sub-agent environment.\n"
            " *\n"
            " * $I" "d$\n"
            " */\n\n");
    fprintf(f,
            "/**\n"
            "    This class extends the Java AgentX (JAX) implementation of\n"
            "    the table row %s defined in %s.\n"
            " */\n\n", smiNode->name, smiGetNodeModule(smiNode)->name);
    
    if (package) {
	fprintf(f,
		"package %s;\n\n", package);
    }
    
    fprintf(f,
            "import jax.AgentXOID;\n"
            "import jax.AgentXSetPhase;\n"
            "import jax.AgentXResponsePDU;\n"
            "import jax.AgentXEntry;\n"
            "\n");

    fprintf(f, "public class %sImpl extends %s\n{\n\n",
            translate1Upper(smiNode->name),
            translate1Upper(smiNode->name));

    fprintf(f,
            "    // constructor\n"
            "    public %sImpl(", translate1Upper(smiNode->name));
    for (element = smiGetFirstElement(smiNode), cnt = 0;
         element;
         element = smiGetNextElement(element)) {
        if (cnt) {
            fprintf(f, ",\n%*s", 4 + 7 + 1 + strlen(smiNode->name), " ");
        }
        cnt++;
        indexNode = smiGetElementNode(element);
        fprintf(f, "%s %s",
                getJavaType(smiGetNodeType(indexNode)),
                indexNode->name);
    }
    fprintf(f, ")\n"
            "    {\n"
            "        super(");
    for (element = smiGetFirstElement(smiNode), cnt = 0;
         element;
         element = smiGetNextElement(element)) {
        if (cnt) {
            fprintf(f, ",\n%*s", 2 + strlen(smiNode->name), " ");
        }
        cnt++;
        indexNode = smiGetElementNode(element);
        fprintf(f, "%s", indexNode->name);
    }
    fprintf(f,");\n");
    
    fprintf(f,
            "    }\n"
            "\n");

    for (columnNode = smiGetFirstChildNode(smiNode);
         columnNode;
         columnNode = smiGetNextChildNode(columnNode)) {
        smiType = smiGetNodeType(columnNode);
	if (!smiType) continue;
        if (columnNode->access >= SMI_ACCESS_NOTIFY) {
            fprintf(f,
                    "    public %s get_%s()\n"
                    "    {\n"
                    "        return %s;\n"
                    "    }\n"
                    "\n",
                    getJavaType(smiType),
                    columnNode->name, columnNode->name);
        }
        if (columnNode->access == SMI_ACCESS_READ_WRITE) {
            fprintf(f,
                    "    public int set_%s(AgentXSetPhase phase, %s value)\n"
                    "    {\n",
                    columnNode->name,
                    getJavaType(smiGetNodeType(columnNode)));
            fprintf(f,
                    "        switch (phase.getPhase()) {\n"
                    "        case AgentXSetPhase.TEST_SET:\n"
                    "            break;\n"                     
                    "        case AgentXSetPhase.COMMIT:\n"
                    "            undo_%s = %s;\n",
                    columnNode->name,
                    columnNode->name);
            if (!strcmp("byte[]", getJavaType(smiGetNodeType(columnNode)))) {
                fprintf(f,
                        "            %s = new byte[value.length];\n"
                        "            for(int i = 0; i < value.length; i++)\n"
                        "                %s[i] = value[i];\n",
                        columnNode->name,
                        columnNode->name);
            } else {
                fprintf(f,
                        "            %s = value;\n",
                        columnNode->name);
            }

            fprintf(f,
                    "            break;\n"                     
                    "        case AgentXSetPhase.UNDO:\n"
                    "            %s = undo_%s;\n"
                    "            break;\n",
                    columnNode->name,
                    columnNode->name);
            fprintf(f,
                    "        case AgentXSetPhase.CLEANUP:\n");
            if (!strcmp("byte[]",getJavaType(smiGetNodeType(columnNode)))) {
                fprintf(f,
                        "            undo_%s = null;\n",
                        columnNode->name);
            }
            fprintf(f,
                    "            break;\n"
                    "        default:\n"
                    "            return AgentXResponsePDU.PROCESSING_ERROR;\n"
                    "        }\n"
                    "        return AgentXResponsePDU.NO_ERROR;\n"
                    "    }\n");
        }
    }
    
    fprintf(f,
            "}\n\n");

    if (fflush(f) || ferror(f)) {
	perror("smidump: write error");
	exit(1);
    }

    fclose(f);
}



static SmiNode *dumpScalars(SmiNode *smiNode)
{
    FILE *f;
    char *vb_type;
    SmiNode *parentNode, *currNode;
    SmiType *smiType;
    SmiRange *smiRange;
    unsigned int i;
    char *p;
    char init[20];

    parentNode = smiGetParentNode(smiNode);

    f = createFile(translate1Upper(parentNode->name), ".java");
    if (! f) {
        return NULL;
    }

    fprintf(f,
            "/*\n"
            " * This Java file has been generated by smidump "
                SMI_VERSION_STRING "." " Do not edit!\n"
            " * It is intended to be used within a Java AgentX sub-agent"
            " environment.\n"
            " *\n"
            " * $I" "d$\n"
            " */\n\n");
    fprintf(f,
            "/**\n"
            "    This class represents a Java AgentX (JAX) implementation of\n"
            "    the scalar group %s defined in %s.\n"
            "\n"
            "    @version 1\n"
            "    @author  smidump " SMI_VERSION_STRING "\n"
            "    @see     AgentXGroup, AgentXScalars\n"
            " */\n\n", parentNode->name, smiGetNodeModule(smiNode)->name);

    if (package) {
	fprintf(f,
		"package %s;\n\n", package);
    }
    
    fprintf(f,
            "import java.util.Vector;\n"
            "import java.util.Enumeration;\n"
            "import jax.AgentXOID;\n"
            "import jax.AgentXVarBind;\n"
            "import jax.AgentXSetPhase;\n"
            "import jax.AgentXResponsePDU;\n"
            "import jax.AgentXScalars;\n"
            "\n");

    fprintf(f, "public class %s extends AgentXScalars\n{\n\n",
            translate1Upper(parentNode->name));

    fprintf(f,
            "    private final static long[] %sOID = {",
            translate1Upper(parentNode->name));
    for (i = 0; i < parentNode->oidlen; i++) {
        fprintf(f, "%s%d", i ? ", " : "", parentNode->oid[i]);
    }
    fprintf(f, "};\n\n");

    /* -------       variable definitions */

    for (currNode = smiNode;
         currNode && (currNode->nodekind == SMI_NODEKIND_SCALAR) ;
         currNode = smiGetNextNode(currNode,SMI_NODEKIND_ANY)) {
        fprintf(f,
                "    protected AgentXOID %sOID;\n"
                "    protected final static long[] %sName = {",
                translate1Upper(currNode->name),
                translate1Upper(currNode->name)
                );
        for (i = 0; i < currNode->oidlen; i++) {
            fprintf(f, "%s%d", i ? ", " : "", currNode->oid[i]);
        }
        fprintf(f, ", 0};\n");

        smiType = smiGetNodeType(currNode);
        p = getJavaType(smiType);
        if (!strcmp(p, "long")) {
            strcpy(init, "0");
        } else if (!strcmp(p, "int")) {
            strcpy(init, "0");
        } else if (!strcmp(p, "byte[]")) {
            smiRange = smiGetFirstRange(smiType);
            if ((smiRange && (!smiGetNextRange(smiRange)) &&
                 (!memcmp(&smiRange->minValue, &smiRange->maxValue,
                          sizeof(SmiValue))))) {
                sprintf(init, "new byte[%ld]",
                        smiRange->maxValue.value.integer32);
            } else {
                sprintf(init, "new byte[0]");
            }
        } else if (!strcmp(p, "AgentXOID")) {
            strcpy(init, "new AgentXOID()");
        } else {
            strcpy(init, "null");
        }
        fprintf(f,
                "    protected %s %s = %s;\n",
                getJavaType(smiGetNodeType(currNode)),
                currNode->name,
                init);
        
        if (currNode->access == SMI_ACCESS_READ_WRITE) {
            fprintf(f,
                    "    protected %s undo_%s = %s;\n",
                    getJavaType(smiGetNodeType(currNode)),
                    currNode->name,
                    init);
        }
    }
    /* -------       constructor */
    fprintf(f,
            "    public %s()\n    {\n"
            "        oid = new AgentXOID(%sOID);\n"
            "        data = new Vector();\n",
            translate1Upper(parentNode->name),
            translate1Upper(parentNode->name));

    for (currNode = smiNode;
         currNode && (currNode->nodekind == SMI_NODEKIND_SCALAR) ;
         currNode = smiGetNextNode(currNode,SMI_NODEKIND_ANY)) {
        fprintf(f,
                "        %sOID = new AgentXOID(%sName);\n"
                "        data.addElement(%sOID);\n",
                translate1Upper(currNode->name),
                translate1Upper(currNode->name),
                translate1Upper(currNode->name));
    }
    fprintf(f,"    }\n\n");
    /* -------         get & set methods */ 
    for (currNode = smiNode;
         currNode && (currNode->nodekind == SMI_NODEKIND_SCALAR) ;
         currNode = smiGetNextNode(currNode,SMI_NODEKIND_ANY)) {
        smiType = smiGetNodeType(currNode);
        if (currNode->access >= SMI_ACCESS_NOTIFY) {
            fprintf(f,
                    "    public %s get_%s()\n"
                    "    {\n"
                    "        return %s;\n"
                    "    }\n"
                    "\n",
                    getJavaType(smiType),
                    currNode->name, currNode->name);
        }
        if (currNode->access == SMI_ACCESS_READ_WRITE) {
            fprintf(f,
                    "    public int set_%s(AgentXSetPhase phase, %s value)\n"
                    "    {\n",
                    currNode->name,
                    getJavaType(smiGetNodeType(currNode)));
            fprintf(f,
                    "        switch (phase.getPhase()) {\n"
                    "        case AgentXSetPhase.TEST_SET:\n"
                    "            break;\n"
                    "        case AgentXSetPhase.COMMIT:\n"
                    "            undo_%s = %s;\n",
                    currNode->name,
                    currNode->name);
            if (!strcmp("byte[]", getJavaType(smiGetNodeType(currNode)))) {
                fprintf(f,
                        "            %s = new byte[value.length];\n"
                        "            for(int i = 0; i < value.length; i++)\n"
                        "                %s[i] = value[i];\n",
                        currNode->name,
                        currNode->name);
            } else {
                fprintf(f,
                        "            %s = value;\n",
                        currNode->name);
            }

            fprintf(f,
                    "            break;\n"                      
                    "        case AgentXSetPhase.UNDO:\n"
                    "            %s = undo_%s;\n"
                    "            break;\n",
                    currNode->name,
                    currNode->name);
            fprintf(f,
                    "        case AgentXSetPhase.CLEANUP:\n");
            if (!strcmp("byte[]",getJavaType(smiGetNodeType(currNode)))) {
                fprintf(f,
                        "            undo_%s = null;\n",
                        currNode->name);
            }
            fprintf(f,
                    "            break;\n"
                    "        default:\n"
                    "            return AgentXResponsePDU.PROCESSING_ERROR;\n"
                    "        }\n"
                    "        return AgentXResponsePDU.NO_ERROR;\n"
                    "    }\n");
        }
    }
    /* ------    handle get get-next and set requests */
    fprintf(f,
            "    public AgentXVarBind getScalar(AgentXOID pos,"
            " AgentXOID oid)\n"
            "    {\n"
            "        if ((pos == null) || (pos.compareTo(oid) != 0))\n"
            "            return new AgentXVarBind(oid,"
            " AgentXVarBind.NOSUCHOBJECT);\n"
            "        else {\n"
            );
    for (currNode = smiNode;
         currNode && (currNode->nodekind == SMI_NODEKIND_SCALAR) ;
         currNode = smiGetNextNode(currNode,SMI_NODEKIND_ANY)) {    
        fprintf(f,
                "            if (pos == %sOID)\n"
                "                return new AgentXVarBind(oid,"
                " AgentXVarBind.%s, \n"
                "                                         get_%s());\n",
                translate1Upper(currNode->name),
                getAgentXType(smiGetNodeType(currNode)),
                currNode->name
                );
    }
    fprintf(f,
            "        }\n"
            "        return new AgentXVarBind(oid,"
            " AgentXVarBind.NOSUCHOBJECT);\n    }\n\n");
        
    fprintf(f,
            "    public int setScalar(AgentXSetPhase phase,"
            " AgentXOID pos,\n"
            "                         AgentXVarBind inVb)\n    {\n"
            "        if ((pos == null) || (pos.compareTo(inVb.getOID()) != 0))\n"
            "            return AgentXResponsePDU.INCONSISTENT_NAME;\n"
            "        else {\n"
            );

    for (currNode = smiNode;
         currNode && (currNode->nodekind == SMI_NODEKIND_SCALAR) ;
         currNode = smiGetNextNode(currNode,SMI_NODEKIND_ANY)) {
        vb_type = getJavaType(smiGetNodeType(currNode));
        vb_type = strcmp("byte[]", vb_type) ? vb_type : "bytes";
        if (currNode->access == SMI_ACCESS_READ_WRITE)
            fprintf(f,
                    "            if (pos == %sOID)\n"
                    "                return "
                    "set_%s(phase, inVb.%sValue());\n",
                    translate1Upper(currNode->name),
                    currNode->name,
                    vb_type
                    );
    }
    fprintf(f,
            "        }\n"
            "        return AgentXResponsePDU.NOT_WRITABLE;\n"
            "    }\n\n");
    fprintf(f,
            "    public AgentXVarBind getNextScalar(AgentXOID pos,"
            " AgentXOID oid)\n    {\n"
            "        if ((pos == null) || (pos.compareTo(oid) <= 0))\n"
            "            return new AgentXVarBind(oid,"
            " AgentXVarBind.ENDOFMIBVIEW);\n"
            "        else {\n"
            );
    for (currNode = smiNode;
         currNode && (currNode->nodekind == SMI_NODEKIND_SCALAR) ;
         currNode = smiGetNextNode(currNode,SMI_NODEKIND_ANY)) {    
        fprintf(f,
                "            if (pos == %sOID)\n"
                "                return new AgentXVarBind(pos,"
                " AgentXVarBind.%s, \n"
                "                                         get_%s());\n",
                translate1Upper(currNode->name),
                getAgentXType(smiGetNodeType(currNode)),
                currNode->name
                );
    }
    fprintf(f,
            "        }\n"
            "        return new AgentXVarBind(pos,"
            " AgentXVarBind.ENDOFMIBVIEW);\n    }\n\n");
    fprintf(f,
            "}\n\n");

    if (fflush(f) || ferror(f)) {
	perror("smidump: write error");
	exit(1);
    }

    fclose(f);
    /* skip all already processed nodes */
    for(;
        smiNode && (smiNode->nodekind == SMI_NODEKIND_SCALAR);
        smiNode = smiGetNextNode(smiNode, SMI_NODEKIND_ANY)) {
    }
    return smiNode;
}

static void dumpNotifications(SmiNode *smiNode)
{
    FILE *f;
    int cnt;
    unsigned int i;
    SmiElement *element;
    SmiNode *elementNode;

    SmiType *snt;
    snt = smiGetNodeType(smiNode);

    f = createFile(translate1Upper(smiNode->name), ".java");
    if (! f) {
        return;
    }

    fprintf(f,
            "/*\n"
            " * This Java file has been generated by smidump "
                SMI_VERSION_STRING "." " Do not edit!\n"
            " * It is intended to be used within a Java AgentX sub-agent"
            " environment.\n"
            " *\n"
            " * $I" "d$\n"
            " */\n\n");

    if (package) {
	fprintf(f,
		"package %s;\n\n", package);
    }
    
    fprintf(f,
            "import jax.AgentXOID;\n"
            "import jax.AgentXVarBind;\n"
            "import jax.AgentXNotification;\n"
            "import java.util.Vector;\n"
            "\n");

    fprintf(f, "public class %s extends AgentXNotification\n{\n\n",
            translate1Upper(smiNode->name));

    fprintf(f,
            "    private final static long[] %s_OID = {",
            smiNode->name);
    for (i = 0; i < smiNode->oidlen; i++) {
        fprintf(f, "%s%d", i ? ", " : "", smiNode->oid[i]);
    }
    fprintf(f, "};\n"); 
    fprintf(f,
            "    private static AgentXVarBind snmpTrapOID_VarBind =\n"
            "        new AgentXVarBind(snmpTrapOID_OID,\n"
            "                          AgentXVarBind.OBJECTIDENTIFIER,\n"
            "                          new AgentXOID(%s_OID));\n\n",
            smiNode->name);

    for (element = smiGetFirstElement(smiNode), cnt = 1;
         element;
         element = smiGetNextElement(element), cnt++) {
        elementNode = smiGetElementNode(element);
        fprintf(f, "    private final static long[] OID%d = {", cnt);
        for (i = 0; i < elementNode->oidlen; i++) {
            fprintf(f, "%s%d", i ? ", " : "", elementNode->oid[i]);
        }
        fprintf(f, 
                "};\n"
                "    private final AgentXOID %s_OID = "
                "new AgentXOID(OID%d",
                elementNode->name, cnt);
        fprintf(f,");\n");
#if 0
        if (elementNode->nodekind != SMI_NODEKIND_COLUMN || 
            (smiGetNodeModule(elementNode) != smiGetNodeModule(smiNode))) {
            fprintf(f,
                    "    private static AgentXVarBind varBind_%s = new AgentXVarBind(\n"
                    "            %sOID, AgentXVarBind.%s\n);\n",
                    elementNode->name,
                    elementNode->name,
                    getAgentXType(smiGetNodeType(elementNode)));        
        }
#endif
    }    
    fprintf(f, 
            "\n\n    public %s(",
            translate1Upper(smiNode->name));
    for (element = smiGetFirstElement(smiNode), cnt = 0;
         element;
         element = smiGetNextElement(element)) {
        elementNode = smiGetElementNode(element); 
        if (smiGetNodeModule(elementNode) == smiGetNodeModule(smiNode)) {
            if (cnt) fprintf(f,", ");
            cnt++;
            fprintf(f, "%s %s_%d",
                    translate1Upper(smiGetParentNode(elementNode)->name),
                    smiGetParentNode(elementNode)->name, cnt);
        }
    }
    fprintf(f, ") {\n"
            "        AgentXOID oid;\n"
            "        AgentXVarBind varBind;\n"
            "\n"
	    "        // add the snmpTrapOID object\n"
            "        varBindList.addElement(snmpTrapOID_VarBind);\n");
    for (element = smiGetFirstElement(smiNode), cnt = 1;
         element;
         element = smiGetNextElement(element), cnt++) {
        elementNode = smiGetElementNode(element);

	if (smiGetNodeModule(elementNode) == smiGetNodeModule(smiNode)) {

	    fprintf(f,
		    "\n        // add the %s %s object of %s_%d\n",
		    elementNode->name,
		    elementNode->nodekind == SMI_NODEKIND_COLUMN ?
		    "columnar" : "scalar",
		    smiGetParentNode(elementNode)->name, cnt);

	    if (elementNode->nodekind == SMI_NODEKIND_COLUMN) {
		fprintf(f,
			"        oid = %s_OID;\n"
			"        oid.appendImplied(%s_%d.getInstance());\n",
			elementNode->name,
			smiGetParentNode(elementNode)->name, cnt);
		fprintf(f,
			"        varBind = new AgentXVarBind(oid,\n"
			"                                    AgentXVarBind.%s,\n"
			"                                    %s_%d.get_%s());\n",
			getAgentXType(smiGetNodeType(elementNode)),
			smiGetParentNode(elementNode)->name, cnt,
			elementNode->name);
		fprintf(f,
			"        varBindList.addElement(varBind);\n");
	    } else {
		fprintf(f,
			"        oid = %s_OID;\n",
			elementNode->name);
		fprintf(f,
			"        oid.append(0);\n");
		fprintf(f,
			"        varBind = new AgentXVarBind(oid,\n"
			"                                    AgentXVarBind.%s,\n"
			"                                    %s_%d.get_%s());\n",
			getAgentXType(smiGetNodeType(elementNode)),
			smiGetParentNode(elementNode)->name, cnt,
			elementNode->name);
		fprintf(f,
			"        varBindList.addElement(varBind);\n");
	    }

        } else {
	    fprintf(f,
		    "        // the notification's object %s "
		    "is not contained in this module.\n",
		    elementNode->name);
	}
    }    

    fprintf(f,
            "    }\n\n");

    fprintf(f,
            "    public Vector getVarBindList() {\n"
            "        return varBindList;\n    }\n\n");

    fprintf(f,
            "}\n\n");

    if (fflush(f) || ferror(f)) {
	perror("smidump: write error");
	exit(1);
    }

    fclose(f);
}

static void dumpScalarImpl(SmiNode *smiNode)
{
    FILE *f;
    SmiNode *parentNode, *currNode;
    SmiType *smiType;

    parentNode = smiGetParentNode(smiNode);

    f = createFile(translate1Upper(parentNode->name), "Impl.java");
    if (! f) {
        return;
    }

    fprintf(f,
            "/*\n"
            " * This Java file has been generated by smidump "
                SMI_VERSION_STRING ". It\n"
            " * is intended to be edited by the application programmer and\n"
            " * to be used within a Java AgentX sub-agent environment.\n"
            " *\n"
            " * $I" "d$\n"
            " */\n\n");
    fprintf(f,
            "/**\n"
            "    This class extends the Java AgentX (JAX) implementation of\n"
            "    the scalar group %s defined in %s.\n"
            " */\n\n", parentNode->name, smiGetNodeModule(smiNode)->name);

    if (package) {
	fprintf(f,
		"package %s;\n\n", package);
    }
    
    fprintf(f,
            "import java.util.Vector;\n"
            "import java.util.Enumeration;\n"
            "import jax.AgentXOID;\n"
            "import jax.AgentXSetPhase;\n"
            "import jax.AgentXResponsePDU;\n"
            "\n");

    fprintf(f, "public class %sImpl extends %s\n{\n\n",
            translate1Upper(parentNode->name),
            translate1Upper(parentNode->name));

    for (currNode = smiNode;
         currNode && (currNode->nodekind == SMI_NODEKIND_SCALAR) ;
         currNode = smiGetNextNode(currNode,SMI_NODEKIND_ANY)) {
        smiType = smiGetNodeType(currNode);
        if (currNode->access >= SMI_ACCESS_NOTIFY) {
            fprintf(f,
                    "    public %s get_%s()\n"
                    "    {\n"
                    "        return %s;\n"
                    "    }\n"
                    "\n",
                    getJavaType(smiType),
                    currNode->name, currNode->name);
        }
        if (currNode->access == SMI_ACCESS_READ_WRITE) {
            fprintf(f,
                    "    public int set_%s(AgentXSetPhase phase, %s value)\n"
                    "    {\n",
                    currNode->name,
                    getJavaType(smiGetNodeType(currNode)));
            fprintf(f,
                    "        switch (phase.getPhase()) {\n"
                    "        case AgentXSetPhase.TEST_SET:\n"
                    "            break;\n"
                    "        case AgentXSetPhase.COMMIT:\n"
                    "            undo_%s = %s;\n",
                    currNode->name,
                    currNode->name);
            if (!strcmp("byte[]", getJavaType(smiGetNodeType(currNode)))) {
                fprintf(f,
                        "            %s = new byte[value.length];\n"
                        "            for(int i = 0; i < value.length; i++)\n"
                        "                %s[i] = value[i];\n",
                        currNode->name,
                        currNode->name);
            } else {
                fprintf(f,
                        "            %s = value;\n",
                        currNode->name);
            }
            fprintf(f,
                    "            break;\n"                      
                    "        case AgentXSetPhase.UNDO:\n"
                    "            %s = undo_%s;\n"
                    "            break;\n",
                    currNode->name,
                    currNode->name);
            fprintf(f,
                    "        case AgentXSetPhase.CLEANUP:\n");
            if (!strcmp("byte[]",getJavaType(smiGetNodeType(currNode)))) {
                fprintf(f,
                        "            undo_%s = null;\n",
                        currNode->name);
            } else {
                fprintf(f,
                        "            undo_%s = -1; // TODO: better check!\n",
                        currNode->name);
            }
            fprintf(f,
                    "            break;\n"
                    "        default:\n"
                    "            return AgentXResponsePDU.PROCESSING_ERROR;\n"
                    "        }\n"
                    "        return AgentXResponsePDU.NO_ERROR;\n"
                    "    }\n"
                    "\n");
        }
    }
    fprintf(f,
            "}\n\n");

    if (fflush(f) || ferror(f)) {
	perror("smidump: write error");
	exit(1);
    }

    fclose(f);
}


static void dumpJax(int modc, SmiModule **modv, int flags, char *output)
{
    SmiNode     *smiNode;
    int		i;

    for (i = 0; i < modc; i++) {

	for(smiNode = smiGetFirstNode(modv[i], SMI_NODEKIND_ROW);
	    smiNode;
	    smiNode = smiGetNextNode(smiNode, SMI_NODEKIND_ROW)) {
	    if (isGroup(smiNode) && isAccessible(smiNode)) {
		dumpTable(smiNode);
		dumpEntry(smiNode);
		dumpEntryImpl(smiNode);
	    }
	}
    
	for(smiNode = smiGetFirstNode(modv[i], SMI_NODEKIND_SCALAR);
	    smiNode;
	    smiNode = smiGetNextNode(smiNode, SMI_NODEKIND_SCALAR)) {
	    dumpScalarImpl(smiNode);
	    smiNode = dumpScalars(smiNode); 
	}
	
	for(smiNode = smiGetFirstNode(modv[i], SMI_NODEKIND_NOTIFICATION);
	    smiNode;
	    smiNode = smiGetNextNode(smiNode, SMI_NODEKIND_NOTIFICATION)) {
	    dumpNotifications(smiNode);
	}
    }
}



void initJax()
{
    static SmidumpDriverOption opt[] = {
	{ "package", OPT_STRING, &package, 0,
	  "make classes part of a given package"},
        { 0, OPT_END, 0, 0 }
    };

    static SmidumpDriver driver = {
	"jax",
	dumpJax,
	SMI_FLAG_NODESCR,
	SMIDUMP_DRIVER_CANT_UNITE | SMIDUMP_DRIVER_CANT_OUTPUT,
	"Java AgentX sub-agent classes in separate files",
	opt,
	NULL
    };

    smidumpRegisterDriver(&driver);
}
