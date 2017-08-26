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
 * @(#) $Id: smiquery.c 1797 2013-06-06 09:13:00Z schoenw $
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

static const int indent = 12;


static void
print_key(const char *s)
{
    if (s) {
	printf("%*s: ", indent, s);
    } else {
	printf("%*s  ", indent, "");
    }
}


static void
print_val(const char *s)
{
    const char *p;
    
    if (!s) {
	printf("-");
    } else {
	for (p = s; *p; p++) {
	    putc(*p, stdout);
	    if (*p == '\n') {
		print_key(NULL);
	    }
	}
    }
    putc('\n', stdout);
}


static void
print_key_val(const char *key, const char *val)
{
    print_key(key);
    print_val(val);
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
    int i;
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
		print_key_val("Module", module->name);
		if (module->path)
		    print_key_val("Pathname", module->path);
		if (node)
		    print_key_val("Object",
				  smiRenderNode(node, SMI_RENDER_ALL));
		if (module->organization)
		    print_key_val("Organization", module->organization);
		if (module->contactinfo)
		    print_key_val("ContactInfo", module->contactinfo);
		if (module->description)
		    print_key_val("Description", module->description);
		if (module->reference)
		    print_key_val("Reference", module->reference);
		print_key_val("Language",
			      smiLanguageAsString(module->language));
		print_key("Conformance"); printf("%d\n", module->conformance);
		print_key_val("Loaded", smiIsLoaded(name) ? "yes" : "no");

		for(revision = smiGetFirstRevision(module);
		    revision ; revision = smiGetNextRevision(revision)) {
		    print_key_val("Revision", ctime(&revision->date));
		    if (revision->description)
			print_key_val("Description", revision->description);
		}
	    }
	} else if (!strcmp(command, "imports")) {
	    module = smiGetModule(name);
	    if (module && smiGetFirstImport(module)) {
		print_key("Imports");
		for(import = smiGetFirstImport(module); import; ) {
		    printf("%s::%s\n", import->module, import->name);
		    import = smiGetNextImport(import);
		    if (import) {
			print_key(NULL);
		    }
		}
	    }
	} else if (!strcmp(command, "node")) {
	    node = smiGetNode(NULL, name);
	    if (node) {
		parent = smiGetParentNode(node);
	    }
	    if (node) {
		type = smiGetNodeType(node);
		print_key_val("MibNode", smiRenderNode(node, SMI_RENDER_ALL));
		print_key_val("OID", smiRenderOID(node->oidlen, node->oid, 0));
		for (node2 = smiGetFirstAlias(node);
		     node2; node2 = smiGetNextAlias(node2)) {
		    if (node2 == node) continue;
		    print_key_val("Alias",
				  smiRenderNode(node2, SMI_RENDER_ALL));
		}
		
		if (parent)
		    print_key_val("ParentNode",
				  smiRenderNode(parent, SMI_RENDER_ALL));
		if (type)
		    print_key_val("Type", smiRenderType(type, SMI_RENDER_ALL));
		if (node->value.basetype != SMI_BASETYPE_UNKNOWN)
		    print_key_val("Default",
			  smiRenderValue(&node->value, type, SMI_RENDER_ALL));
		if (node->decl != SMI_DECL_UNKNOWN)
		    print_key_val("Declaration", smiDeclAsString(node->decl));
		print_key_val("NodeKind", smiNodekindAsString(node->nodekind));
		if (node->nodekind == SMI_NODEKIND_ROW) {
		    print_key_val("Creatable", node->create ? "yes" : "no");
		    print_key_val("Implied", node->implied ? "yes" : "no");
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
		    print_key(p);
		    for(element = smiGetFirstElement(node);
			element; ) {
			node2 = smiGetElementNode(element);
			print_val(smiRenderNode(node2, SMI_RENDER_ALL));
			element = smiGetNextElement(element);
			if (element) {
			    print_key(NULL);
			}
		    }
		}
		if (node->access != SMI_ACCESS_UNKNOWN)
		    print_key_val("Access", smiAccessAsString(node->access));
		if (node->status != SMI_STATUS_UNKNOWN)
		    print_key_val("Status", smiStatusAsString(node->status));
		if (node->format)
		    print_key_val("Format", node->format);
		if (node->units)
		    print_key_val("Units", node->units);
		if (node->description)
		    print_key_val("Description", node->description);
		if (node->reference)
		    print_key_val("Reference", node->reference);
	    }
	} else if (!strcmp(command, "compliance")) {
	    node = smiGetNode(NULL, name);
	    if (node) {
		if (smiGetFirstElement(node)) {
		    print_key("Mandatory");
		    for(element = smiGetFirstElement(node);
			element ; ) {
			node2 = smiGetElementNode(element);
			print_val(smiRenderNode(node2, SMI_RENDER_ALL));
			element = smiGetNextElement(element);
			if (element) {
			    print_key(NULL);
			}
		    }
		}
		if (smiGetFirstOption(node)) {
		    for(option = smiGetFirstOption(node); option ;) {
			node2 = smiGetOptionNode(option);
			print_key_val("Option",
				      smiRenderNode(node2, SMI_RENDER_ALL));
			if (option->description)
			    print_key_val("Description",
					  option->description);
			option = smiGetNextOption(option);
		    }
		}
		if (smiGetFirstRefinement(node)) {
		    for(refinement = smiGetFirstRefinement(node);
			refinement ;
			refinement = smiGetNextRefinement(refinement)) {
			node2 = smiGetRefinementNode(refinement);
			print_key_val("Refinement",
				      smiRenderNode(node2, SMI_RENDER_ALL));
			type = smiGetRefinementType(refinement);
			if (type) {
			    print_key_val("Type",
				  smiRenderType(type, SMI_RENDER_ALL));
			}
			type = smiGetRefinementWriteType(refinement);
			if (type) {
			    print_key_val("Write-Type",
				  smiRenderType(type, SMI_RENDER_ALL));
			}
			if (refinement->access != SMI_ACCESS_UNKNOWN) {
			    print_key_val("Access",
				  smiAccessAsString(refinement->access));
			}
			if (refinement->description)
			    print_key_val("Description",
					  refinement->description);
		    }
		}
	    }
	} else if (!strcmp(command, "children")) {
	    node = smiGetNode(NULL, name);
	    if (node && smiGetFirstChildNode(node)) {
		print_key("Children");
		for(child = smiGetFirstChildNode(node); child ; ) {
		    print_val(smiRenderNode(child, SMI_RENDER_ALL));
		    child = smiGetNextChildNode(child);
		    if (child) {
			print_key(NULL);
		    }
		}
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
		print_key_val("Type", smiRenderType(type, SMI_RENDER_ALL));
		print_key_val("Basetype", smiBasetypeAsString(type->basetype));
		if (parenttype)
		    print_key_val("Parent Type",
				  smiRenderType(parenttype, SMI_RENDER_ALL));
		if (type->value.basetype != SMI_BASETYPE_UNKNOWN)
		    print_key_val("Default",
			  smiRenderValue(&type->value, type, SMI_RENDER_ALL));
		if ((type->basetype == SMI_BASETYPE_ENUM) ||
		    (type->basetype == SMI_BASETYPE_BITS)) {
		    if (smiGetFirstNamedNumber(type)) {
			print_key("Numbers");
			for(nn = smiGetFirstNamedNumber(type); nn; ) {
			    printf("%s(%ld)\n",
				   nn->name, nn->value.value.integer32);
			    nn = smiGetNextNamedNumber(nn);
			    if (nn) {
				print_key(NULL);
			    }
			}
		    }
		} else {
		    if (smiGetFirstRange(type)) {
			print_key("Ranges");
			for(range = smiGetFirstRange(type); range; ) {
			    strcpy(s1, smiRenderValue(&range->minValue, type, 0));
			    strcpy(s2, smiRenderValue(&range->maxValue, type, 0));
			    printf("%s", s1);
			    if (strcmp(s1, s2)) printf("..%s", s2);
			    printf("\n");
			    range = smiGetNextRange(range);
			    if (range) {
				print_key(NULL);
			    }
			}
		    }
		}
		print_key_val("Declaration", smiDeclAsString(type->decl));
		if (type->status != SMI_STATUS_UNKNOWN)
		    print_key_val("Status", smiStatusAsString(type->status));
		if (type->format)
		    print_key_val("Format", type->format);
		if (type->units)
		    print_key_val("Units", type->units);
		if (type->description)
		    print_key_val("Description", type->description);
		if (type->reference)
		    print_key_val("Reference", type->reference);
	    }
	} else if (!strcmp(command, "macro")) {
	    macro = smiGetMacro(NULL, name);
	    if (macro) {
		print_key_val("Macro", macro->name);
		if (macro->status != SMI_STATUS_UNKNOWN)
		    print_key_val("Status", smiStatusAsString(macro->status));
		if (macro->description)
		    print_key_val("Description", macro->description);
		if (macro->reference)
		    print_key_val("Reference", macro->reference);
	    }
	} else {
	    usage();
	    return 1;
	}
    }
    
    smiExit();
    
    return 0;
}
