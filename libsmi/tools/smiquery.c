/*
 * smiquery.c --
 *
 *      A simple SMI query program, using libsmi.
 *
 * Copyright (c) 1999 Frank Strauss, Technical University of Braunschweig.
 *
 * See the file "COPYING" for information on usage and redistribution
 * of this file, and for a DISCLAIMER OF ALL WARRANTIES.
 *
 * @(#) $Id: smiquery.c 2445 2005-06-13 10:44:30Z schoenw $
 */

#include <config.h>

#include <stdio.h>
#include <stdlib.h>
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
#include "shhopt.h"



static char *smiStringStatus(SmiStatus status)
{
    return
	(status == SMI_STATUS_CURRENT)     ? "current" :
	(status == SMI_STATUS_DEPRECATED)  ? "deprecated" :
	(status == SMI_STATUS_OBSOLETE)    ? "obsolete" :
	(status == SMI_STATUS_MANDATORY)   ? "mandatory" :
	(status == SMI_STATUS_OPTIONAL)    ? "optional" :
					     "<UNDEFINED>";
}

static char *smiStringAccess(SmiAccess access)
{
    return
	(access == SMI_ACCESS_NOT_ACCESSIBLE) ? "not-accessible" :
	(access == SMI_ACCESS_NOTIFY)	      ? "accessible-for-notify" :
	(access == SMI_ACCESS_READ_ONLY)      ? "read-only" :
	(access == SMI_ACCESS_READ_WRITE)     ? "read-write" :
						"<UNDEFINED>";
}

static char *smiStringLanguage(SmiLanguage language)
{
    return
	(language == SMI_LANGUAGE_UNKNOWN)    ? "<unknown>" :
	(language == SMI_LANGUAGE_SMIV1)      ? "SMIv1" :
	(language == SMI_LANGUAGE_SMIV2)      ? "SMIv2" :
	(language == SMI_LANGUAGE_SMING)      ? "SMIng" :
						"<UNDEFINED>";
}

static char *smiStringDecl(SmiDecl macro)
{
    return
        (macro == SMI_DECL_UNKNOWN)           ? "<unknown>" :
        (macro == SMI_DECL_IMPLICIT_TYPE)     ? "<implicit>" :
        (macro == SMI_DECL_TYPEASSIGNMENT)    ? "<type-assignment>" :
        (macro == SMI_DECL_IMPL_SEQUENCEOF)   ? "<implicit-sequence-of>" :
        (macro == SMI_DECL_VALUEASSIGNMENT)   ? "<value-assignment>" :
        (macro == SMI_DECL_OBJECTTYPE)        ? "OBJECT-TYPE" :
        (macro == SMI_DECL_OBJECTIDENTITY)    ? "OBJECT-IDENTITY" :
        (macro == SMI_DECL_MODULEIDENTITY)    ? "MODULE-IDENTITY" :
        (macro == SMI_DECL_NOTIFICATIONTYPE)  ? "NOTIFICATIONTYPE" :
        (macro == SMI_DECL_TRAPTYPE)          ? "TRAP-TYPE" :
        (macro == SMI_DECL_OBJECTGROUP)       ? "OBJECT-GROUP" :
        (macro == SMI_DECL_NOTIFICATIONGROUP) ? "NOTIFICATION-GROUP" :
        (macro == SMI_DECL_MODULECOMPLIANCE)  ? "MODULE-COMPLIANCE" :
        (macro == SMI_DECL_AGENTCAPABILITIES) ? "AGENT-CAPABILITIES" :
        (macro == SMI_DECL_TEXTUALCONVENTION) ? "TEXTUAL-CONVENTION" :
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
        (macro == SMI_DECL_IMPL_OBJECT)	      ? "<implicit object>" :
                                                "<UNDEFINED>";
}

static char *smiStringNodekind(SmiNodekind nodekind)
{
    return
        (nodekind == SMI_NODEKIND_UNKNOWN)      ? "<unknown>" :
        (nodekind == SMI_NODEKIND_NODE)         ? "node" :
        (nodekind == SMI_NODEKIND_SCALAR)       ? "scalar" :
        (nodekind == SMI_NODEKIND_TABLE)        ? "table" :
        (nodekind == SMI_NODEKIND_ROW)          ? "row" :
        (nodekind == SMI_NODEKIND_COLUMN)       ? "column" :
        (nodekind == SMI_NODEKIND_NOTIFICATION) ? "notification" :
        (nodekind == SMI_NODEKIND_GROUP)        ? "group" :
        (nodekind == SMI_NODEKIND_COMPLIANCE)   ? "compliance" :
        (nodekind == SMI_NODEKIND_CAPABILITIES) ? "capabilities" :
                                                  "<UNDEFINED>";
}

static char *smiStringBasetype(SmiBasetype basetype)
{
    return
        (basetype == SMI_BASETYPE_UNKNOWN)           ? "<unknown>" :
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
                                                   "<UNDEFINED>";
}



static char *format(const char *s)
{
    static char ss[20000];
    int i, j;

    if (!s) {
	sprintf(ss, "-");
    } else {
	for(i = 0, j = 0; s[i]; i++) {
	    ss[j++] = s[i];
	    if (s[i] == '\n') {
		sprintf(&ss[j], "              ");
		j += 14;
	    }
	}
	ss[j] = 0;
    }
    return ss;
}


static void usage()
{
    fprintf(stderr,
	    "Usage: smiquery [options] command name [command name ...]\n"
	    "  -V, --version        show version and license information\n"
	    "  -h, --help           show usage information\n"
	    "  -c, --config=file    load a specific configuration file\n"
	    "  -p, --preload=module preload <module>\n"
	    "\nSupported commands are:\n"
	    "  module <module>           show information on module <module>\n"
	    "  imports <module>          show import list of module <module>\n"
	    "  node <module::name>       show information on node <module::name>\n"
	    "  compliance <module::name> show information on compliance node <module::name>\n"
	    "  children <module::name>   show children list of node <module::name>\n"
	    "  type <module::name>       show information on type <module::name>\n"
	    "  macro <module::name>      show information on macro <module::name>\n");
}



static void help() { usage(); exit(0); }
static void version() { printf("smiquery " SMI_VERSION_STRING "\n"); exit(0); }
static void config(char *filename) { smiReadConfig(filename, "smiquery"); }
static void preload(char *module) { smiLoadModule(module); }



int main(int argc, char *argv[])
{
    SmiModule *module;
    SmiNode *node, *node2, *child, *parent;
    SmiType *type, *parenttype;
    SmiMacro *macro;
    SmiNamedNumber *nn;
    SmiRange *range;
    SmiImport *import;
    SmiRevision *revision;
    SmiOption *option;
    SmiRefinement *refinement;
    SmiElement *element;
    char *command, *name, *p;
    int flags, i;
    char s1[40], s2[40];

    static optStruct opt[] = {
	/* short long              type        var/func       special       */
	{ 'h', "help",           OPT_FLAG,   help,          OPT_CALLFUNC },
	{ 'V', "version",        OPT_FLAG,   version,       OPT_CALLFUNC },
	{ 'c', "config",         OPT_STRING, config,        OPT_CALLFUNC },
	{ 'p', "preload",        OPT_STRING, preload,       OPT_CALLFUNC },
	{ 0, 0, OPT_END, 0, 0 }  /* no more options */
    };
    
    for (i = 1; i < argc; i++)
	if ((strstr(argv[i], "-c") == argv[i]) ||
	    (strstr(argv[i], "--config") == argv[i])) break;
    if (i == argc) 
	smiInit("smiquery");
    else
	smiInit(NULL);

    flags = smiGetFlags();

    optParseOptions(&argc, argv, opt, 0);

    if (!(argc % 2)) {
	usage();
	return 1;
    }

    for (i = 1; i < argc; i +=2) {

	command = argv[i];
	name = argv[i+1];

	if (!strcmp(command, "module")) {
	    module = smiGetModule(name);
	    if (module) {
		node = smiGetModuleIdentityNode(module);
		printf("      Module: %s\n", format(module->name));
		if (module->path)
		    printf("    Pathname: %s\n", module->path);
		if (node)
		    printf("      Object: %s\n",
			   smiRenderNode(node, SMI_RENDER_ALL));
		if (module->organization)
		    printf("Organization: %s\n", format(module->organization));
		if (module->contactinfo)
		    printf(" ContactInfo: %s\n", format(module->contactinfo));
		if (module->description)
		    printf(" Description: %s\n", format(module->description));
		if (module->reference)
		    printf("   Reference: %s\n", format(module->reference));
		printf("    Language: %s\n", smiStringLanguage(module->language));
		printf(" Conformance: %d\n", module->conformance);
		printf("      Loaded: %s\n", smiIsLoaded(name) ? "yes" : "no");

		for(revision = smiGetFirstRevision(module);
		    revision ; revision = smiGetNextRevision(revision)) {
		    printf("    Revision: %s", ctime(&revision->date));
		    if (revision->description)
			printf(" Description: %s\n", format(revision->description));
		}
	    }
	} else if (!strcmp(command, "imports")) {
	    module = smiGetModule(name);
	    if (module && smiGetFirstImport(module)) {
		printf("     Imports:");
		for(import = smiGetFirstImport(module); import ; ) {
		    printf(" %s::%s", import->module, import->name);
		    import = smiGetNextImport(import);
		    if (import) {
			printf("\n             ");
		    }
		}
		printf("\n");
	    }
	} else if (!strcmp(command, "node")) {
	    node = smiGetNode(NULL, name);
	    if (node) {
		parent = smiGetParentNode(node);
	    }
	    if (node) {
		type = smiGetNodeType(node);
		printf("     MibNode: %s\n", smiRenderNode(node, SMI_RENDER_ALL));
		printf("         OID: %s\n", smiRenderOID(node->oidlen, node->oid,
							  0));
		if (parent)
		    printf("  ParentNode: %s\n",
			   smiRenderNode(parent, SMI_RENDER_ALL));
		if (type)
		    printf("        Type: %s\n",
			   smiRenderType(type, SMI_RENDER_ALL));
		if (node->value.basetype != SMI_BASETYPE_UNKNOWN)
		    printf("     Default: %s\n", smiRenderValue(&node->value, type,
								SMI_RENDER_ALL));
		if (node->decl != SMI_DECL_UNKNOWN)
		    printf(" Declaration: %s\n", smiStringDecl(node->decl));
		printf("    NodeKind: %s\n", smiStringNodekind(node->nodekind));
		if (node->nodekind == SMI_NODEKIND_ROW) {
		    printf ("   Creatable: %s\n", node->create ? "yes" : "no");
		    printf ("     Implied: %s\n", node->implied ? "yes" : "no");
		}
		switch (node->nodekind) {
		case SMI_NODEKIND_ROW: p = "Index"; break;
		case SMI_NODEKIND_COMPLIANCE: p = "Mandatory"; break;
		case SMI_NODEKIND_CAPABILITIES: p = "Includes"; break;
		case SMI_NODEKIND_GROUP: p = "Members"; break;
		case SMI_NODEKIND_NOTIFICATION: p = "Objects"; break;
		default: p = "Elements";
		}
		if (smiGetFirstElement(node)) {
		    printf("%12s:", p);
		    for(element = smiGetFirstElement(node);
			element ; ) {
			node2 = smiGetElementNode(element);
			printf(" %s", smiRenderNode(node2, SMI_RENDER_ALL));
			element = smiGetNextElement(element);
			if (element) {
			    printf("\n             ");
			}
		    }
		    printf("\n");
		}
		if (node->access != SMI_ACCESS_UNKNOWN)
		    printf("      Access: %s\n", smiStringAccess(node->access));
		if (node->status != SMI_STATUS_UNKNOWN)
		    printf("      Status: %s\n", smiStringStatus(node->status));
		if (node->format)
		    printf("      Format: %s\n", format(node->format));
		if (node->units)
		    printf("       Units: %s\n", format(node->units));
		if (node->description)
		    printf(" Description: %s\n", format(node->description));
		if (node->reference)
		    printf("   Reference: %s\n", format(node->reference));
	    }
	} else if (!strcmp(command, "compliance")) {
	    node = smiGetNode(NULL, name);
	    if (node) {
		if (smiGetFirstElement(node)) {
		    printf("   Mandatory:");
		    for(element = smiGetFirstElement(node);
			element ; ) {
			node2 = smiGetElementNode(element);
			printf(" %s", smiRenderNode(node2, SMI_RENDER_ALL));
			element = smiGetNextElement(element);
			if (element) {
			    printf("\n             ");
			}
		    }
		    printf("\n");
		}
		if (smiGetFirstOption(node)) {
		    for(option = smiGetFirstOption(node); option ;) {
			node2 = smiGetOptionNode(option);
			printf("      Option: %s\n",
			       smiRenderNode(node2, SMI_RENDER_ALL));
			if (option->description)
			    printf(" Description: %s\n",
				   format(option->description));
			option = smiGetNextOption(option);
		    }
		}
		if (smiGetFirstRefinement(node)) {
		    for(refinement = smiGetFirstRefinement(node);
			refinement ;
			refinement = smiGetNextRefinement(refinement)) {
			node2 = smiGetRefinementNode(refinement);
			printf("  Refinement: %s\n",
			       smiRenderNode(node2, SMI_RENDER_ALL));
			type = smiGetRefinementType(refinement);
			if (type) {
			    printf("        Type: %s\n",
				   smiRenderType(type, SMI_RENDER_ALL));
			}
			type = smiGetRefinementWriteType(refinement);
			if (type) {
			    module = smiGetTypeModule(type);
			    printf("  Write-Type: %s\n",
				   smiRenderType(type, SMI_RENDER_ALL));
			}
			if (refinement->access != SMI_ACCESS_UNKNOWN) {
			    printf("      Access: %s\n",
				   smiStringAccess(refinement->access));
			}
			if (refinement->description)
			    printf(" Description: %s\n",
				   format(refinement->description));
		    }
		}
	    }
	} else if (!strcmp(command, "children")) {
	    node = smiGetNode(NULL, name);
	    if (node && smiGetFirstChildNode(node)) {
		printf("    Children:");
		for(child = smiGetFirstChildNode(node);
		    child ; ) {
		    printf(" %s", smiRenderNode(child, SMI_RENDER_ALL));
		    child = smiGetNextChildNode(child);
		    if (child) {
			printf("\n             ");
		    }
		}
		printf("\n");
	    }
	} else if (!strcmp(command, "type")) {
	    p = strrchr(name, ':');
	    if (!p) p = strrchr(name, '.');
	    if (!p) p = strrchr(name, '!');
	    if (p) {
		p++;
	    } else {
		p = name;
	    }
	    if (islower((int)name[0]) || isdigit((int)name[0]) ||
		!isupper((int)p[0])) {
		node = smiGetNode(NULL, name);
		if (node) {
		    type = smiGetNodeType(node);
		} else {
		    type = smiGetType(NULL, name);
		}
	    } else {
		type = smiGetType(NULL, name);
	    }
	    if (type) {
		parenttype = smiGetParentType(type);
		printf("        Type: %s\n", smiRenderType(type, SMI_RENDER_ALL));
		printf("    Basetype: %s\n", smiStringBasetype(type->basetype));
		if (parenttype)
		    printf(" Parent Type: %s\n",
			   smiRenderType(parenttype, SMI_RENDER_ALL));
		if (type->value.basetype != SMI_BASETYPE_UNKNOWN)
		    printf("     Default: %s\n", smiRenderValue(&type->value, type,
								SMI_RENDER_ALL));
		if ((type->basetype == SMI_BASETYPE_ENUM) ||
		    (type->basetype == SMI_BASETYPE_BITS)) {
		    if (smiGetFirstNamedNumber(type)) {
			printf("     Numbers:");
			for(nn = smiGetFirstNamedNumber(type);
			    nn ; nn = smiGetNextNamedNumber(nn)) {
			    printf(" %s(%ld)",
				   nn->name, nn->value.value.integer32);
			}
			printf("\n");
		    }
		} else {
		    if (smiGetFirstRange(type)) {
			printf("      Ranges:");
			for(range = smiGetFirstRange(type);
			    range ; range = smiGetNextRange(range)) {
			    strcpy(s1, smiRenderValue(&range->minValue, type, 0));
			    strcpy(s2, smiRenderValue(&range->maxValue, type, 0));
			    printf(" %s", s1);
			    if (strcmp(s1, s2)) printf("..%s", s2);
			}
			printf("\n");
		    }
		}
		printf(" Declaration: %s\n", smiStringDecl(type->decl));
		if (type->status != SMI_STATUS_UNKNOWN)
		    printf("      Status: %s\n", smiStringStatus(type->status));
		if (type->format)
		    printf("      Format: %s\n", format(type->format));
		if (type->units)
		    printf("       Units: %s\n", format(type->units));
		if (type->description)
		    printf(" Description: %s\n", format(type->description));
		if (type->reference)
		    printf("   Reference: %s\n", format(type->reference));
	    }
	} else if (!strcmp(command, "macro")) {
	    macro = smiGetMacro(NULL, name);
	    if (macro) {
		printf("       Macro: %s\n", format(macro->name));
		if (macro->status != SMI_STATUS_UNKNOWN)
		    printf("      Status: %s\n", smiStringStatus(macro->status));
		if (macro->description)
		    printf(" Description: %s\n", format(macro->description));
		if (macro->reference)
		    printf("   Reference: %s\n", format(macro->reference));
	    }
	} else {
	    usage();
	    return 1;
	}
    }
    
    smiExit();
    
    return 0;
}
