/*
 * dump-svg.c --
 *
 *      Operations to extract a SVG diagram from MIB modules.
 *      This driver is based on the cm-driver by A. Mueller.
 *      Mail comments and suggestions to sperner@ibr.cs.tu-bs.de
 *
 * Copyright (c) 2004-2005 K. Sperner, Technical University of Braunschweig.
 *
 * See the file "COPYING" for information on usage and redistribution
 * of this file, and for a DISCLAIMER OF ALL WARRANTIES.
 *
 * @(#) $Id: dump-svg.c 8090 2008-04-18 12:56:29Z strauss $
 */



#include <config.h>

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>
#include <math.h>
#ifdef HAVE_WIN_H
#include "win.h"
#endif

#include "smi.h"
#include "smidump.h"
#include "rea.h"
#include "dump-svg-script.h"

#define URL "http://www.ibr.cs.tu-bs.de/projects/libsmi/svg/mib2svg.cgi?"



extern int smiAsprintf(char **strp, const char *format, ...);



/*
 * Definitions used by the svg output driver (node layout).
 */

/* FIXME int or float? */
static const float HEADFONTSIZETABLE   = (float)7;
static const float HEADSPACESIZETABLE  = (float)4;
static const float ATTRFONTSIZE        = (float)7;
static const float ATTRSPACESIZE       = (float)2;
static const float TABLEHEIGHT         = (float)20; /*headline of the table*/
static const float TABLEELEMHEIGHT     = (float)15; /*height of one attribute*/
static const float TABLEBOTTOMHEIGHT   = (float)5;  /*bottom of the table*/

static const int MODULE_INFO_WIDTH     =150;
/* The description of RowStatus is quite long... :-/ */
static const int DYN_TEXT              =470;

/* used by the springembedder */
static const int ITERATIONS            =100;

static char *link;
static const char *linkcolor = "blue";



/* ------ Misc. -----------------                                            */



static char *getTimeString(time_t t)
{
    static char   *s = NULL;
    struct tm	  *tm;

    if (s) xfree(s);
    
    tm = gmtime(&t);
    if (tm->tm_hour == 0 && tm->tm_min == 0) {
	smiAsprintf(&s, "%04d-%02d-%02d",
			tm->tm_year + 1900, tm->tm_mon + 1, tm->tm_mday);
    } else {
	smiAsprintf(&s, "%04d-%02d-%02d %02d:%02d",
			tm->tm_year + 1900, tm->tm_mon + 1, tm->tm_mday,
			tm->tm_hour, tm->tm_min);
    }
    return s;
}



/* -------------- main functions ------------------------------------------- */



/*
 * Creates the graph nodes of the given module
 */
static void algCreateNodes(SmiModule *module)
{
    SmiNode   *node;
    GraphNode *newNode;
    
    /* get tables and scalars from the MIB module */
    for (node = smiGetFirstNode(module, SMI_NODEKIND_TABLE);
	 node;
	 node = smiGetNextNode(node, SMI_NODEKIND_TABLE)) {
	if ((node->status == SMI_STATUS_DEPRECATED
	    && !SHOW_DEPRECATED && !SHOW_DEPR_OBSOLETE)
	    || (node->status == SMI_STATUS_OBSOLETE
	    && !SHOW_DEPR_OBSOLETE))
	    continue;
	newNode = graphInsertNode(graph, node);
	newNode->smiModule = module;
    }
    for (node = smiGetFirstNode(module, SMI_NODEKIND_SCALAR);
	 node;
	 node = smiGetNextNode(node, SMI_NODEKIND_SCALAR)) {
	if ((node->status == SMI_STATUS_DEPRECATED
	    && !SHOW_DEPRECATED && !SHOW_DEPR_OBSOLETE)
	    || (node->status == SMI_STATUS_OBSOLETE
	    && !SHOW_DEPR_OBSOLETE))
	    continue;
	newNode = graphInsertNode(graph, node);
	newNode->smiModule = module;
    }
}



/* ------ XML primitives ------                                              */



/*
 * parseTooltip: Parse any input to output to make the text safe for the
 * ShowTooltipMZ-functin in the ecma-script.
 */
static void parseTooltip(char *input, char *output)
{
    int i, j;

    for (i = j = 0; input[i]; i++) {
	switch (input[i]) {
	case '\n':
	    output[j++] = '\\';
	    output[j++] = 'n';
	    break;
	case '\\':
	    output[j++] = '\\';
	    output[j++] = '\\';
	    break;
	case '\"':
	    /* quotes are not allowed in strings. */
	    /* See chapter 3.4.5 in "Understanding SNMP MIBs" */
	    break;
	case '&':
	    output[j++] = '&';
	    output[j++] = 'a';
	    output[j++] = 'm';
	    output[j++] = 'p';
	    output[j++] = ';';
	    break;
	case '<':
	    output[j++] = '&';
	    output[j++] = 'l';
	    output[j++] = 't';
	    output[j++] = ';';
	    break;
	case '>':
	    output[j++] = '&';
	    output[j++] = 'g';
	    output[j++] = 't';
	    output[j++] = ';';
	    break;
	case '\'':
	    /* It seems, &apos; doesn't work... */
	    output[j++] = '\\';
	    output[j++] = '\'';
	    break;
	default:
	    output[j++] = input[i];
	}
    }
    output[j] = '\0';
}

static int isObjectGroup(SmiNode *groupNode)
{
    SmiNode     *smiNode;
    SmiElement  *smiElement;
    
    for (smiElement = smiGetFirstElement(groupNode); smiElement;
	 smiElement = smiGetNextElement(smiElement)) {

	smiNode = smiGetElementNode(smiElement);
	
	if (smiNode->nodekind != SMI_NODEKIND_SCALAR
	    && smiNode->nodekind != SMI_NODEKIND_COLUMN) {
	    return 0;
	}
    }

    return 1;
}

static int isNotificationGroup(SmiNode *groupNode)
{
    SmiNode     *smiNode;
    SmiElement  *smiElement;
    
    for (smiElement = smiGetFirstElement(groupNode); smiElement;
	 smiElement = smiGetNextElement(smiElement)) {

	smiNode = smiGetElementNode(smiElement);
	
	if (smiNode->nodekind != SMI_NODEKIND_NOTIFICATION) {
	    return 0;
	}
    }

    return 1;
}

static char *printFillColor(SmiStatus status)
{
    return
	(status == SMI_STATUS_CURRENT)     ? "rgb(0%,0%,0%)" :
	(status == SMI_STATUS_DEPRECATED)  ? "rgb(40%,40%,40%)" :
	(status == SMI_STATUS_OBSOLETE)    ? "rgb(60%,60%,60%)" :
	(status == SMI_STATUS_MANDATORY)   ? "rgb(0%,0%,0%)" :
	(status == SMI_STATUS_OPTIONAL)    ? "rgb(20%,20%,20%)" :
					     "";
}

static char *getStatusString(SmiStatus status)
{
    return
	(status == SMI_STATUS_CURRENT)     ? "current" :
	(status == SMI_STATUS_DEPRECATED)  ? "deprecated" :
	(status == SMI_STATUS_OBSOLETE)    ? "obsolete" :
	(status == SMI_STATUS_MANDATORY)   ? "mandatory" :
	(status == SMI_STATUS_OPTIONAL)    ? "optional" :
					     "<unknown>";
}

/*
 * Prints the footer of the SVG output file.
 */
static void printSVGClose(float xMin, float yMin, float xMax, float yMax)
{
    float scale;

    scale = max((xMax-xMin)/CANVASWIDTH,(yMax-yMin)/CANVASHEIGHT);
    /* enclose whole canvas in its bounding box */
    /*
    printf(" <rect x=\"%.2f\" y=\"%.2f\" width=\"%.2f\" height=\"%.2f\"\n",
           xMin, yMin, xMax-xMin-1, yMax-yMin-1);
    printf("       fill=\"none\" stroke=\"blue\" stroke-width=\"1\"/>\n");
    */
    if (!STATIC_OUTPUT) {
	printf(" <g transform=\"translate(%.2f,%.2f) scale(%.2f)\">\n",
							xMin, yMin, scale);
	printf(" <g id=\"tooltip\" style=\"visibility: hidden\">\n");
	printf("   <rect id=\"ttr\" x=\"0\" y=\"0\" rx=\"5\" ry=\"5\"");
	printf(" width=\"100\" height=\"16\"/>\n");
	printf("   <line id=\"ttl\" x1=\"0\" y1=\"0\" x2=\"0\" y2=\"0\"/>\n");
	printf("   <text class=\"tooltip\" xml:space=\"preserve\"");
	printf(" id=\"ttt\" x=\"0\" y=\"0\" style=\"visibility: hidden\">");
	printf("dyn. Text</text>\n");
	printf("   <text class=\"tooltip\" xml:space=\"preserve\"");
	printf(" x=\"-10\" y=\"-10\">dyn. Text</text>\n");
	printf(" </g>\n");
	printf(" </g>\n");
    }
    printf("</svg>\n");
}

/*
 * FIXME stimmt das?
 * index = 0 -> no index element
 * index = 1 -> index element -> printed with "+"
 */
static void printSVGAttribute(SmiNode *node, SmiNode *tableNode, int index,
			      int modc, SmiModule **modv,
			      float *textYOffset, float *textXOffset)
{
    int         i, target_exists = 0;
    size_t      length;
    char        *tooltip, *tooltipDescription, *typeDescription;
    const char  *baseTypeTooltipText = "This is a basetype.";
    const char  *isDefined = " is defined in module ";

    if ((node->status == SMI_STATUS_DEPRECATED
	&& !SHOW_DEPRECATED && !SHOW_DEPR_OBSOLETE)
	|| (node->status == SMI_STATUS_OBSOLETE
	&& !SHOW_DEPR_OBSOLETE))
	return;

    printf("  <text");
    if (!index) {
	printf(" id=\"%s\"", node->name);
    }
    printf(" fill=\"%s\"", printFillColor(node->status));
    printf(" x=\"%.2f\" y=\"%.2f\">\n",
				*textXOffset + ATTRSPACESIZE + 4, *textYOffset);

    *textYOffset += TABLEELEMHEIGHT;

    /* FIXME
       rintf(" textLength=\"100\" lengthAdjust=\"spacingAndGlyphs\""); */

    if (!index) {
	if (node->access == SMI_ACCESS_NOT_ACCESSIBLE) {
	    printf("    <tspan style=\"text-anchor:middle\">-</tspan>");
	} else {
	    printf("    <tspan style=\"text-anchor:middle\">+</tspan>");
	}
    }

    printf("<tspan x=\"%.2f\"", *textXOffset + ATTRSPACESIZE + 8);
    if (!STATIC_OUTPUT) {
	if (node->description) {
	    tooltip = (char *)xmalloc(2*strlen(node->description));
	    parseTooltip(node->description, tooltip);
	    printf(" onmousemove=\"ShowTooltipMZ(evt,'%s')\"", tooltip);
	    printf(" onmouseout=\"HideTooltip(evt)\"");
	    xfree(tooltip);
	}
	if (index) {
	    for (i=0; i<modc; i++) {
		if (modv[i] == smiGetNodeModule(node)) {
		    target_exists = 1;
		}
	    }
	    if (!target_exists) {
		printf(" fill=\"%s\">\n", linkcolor);
		printf("      <a xlink:href=\"%s", link);
		for (i=0; i<modc; i++) {
		    printf("&amp;mibs=%s", modv[i]->name);
		}
		printf("&amp;mibs=%s\">\n", smiGetNodeModule(node)->name);
		printf("        %s:\n", node->name);
		printf("      </a></tspan>\n");
	    } else {
		printf(">%s:</tspan>\n", node->name);
	    }
	} else {
	    printf(">%s:</tspan>\n", node->name);
	}
    } else {
	printf(">%s:</tspan>\n", node->name);
    }

    printf("    <tspan");
    if (!STATIC_OUTPUT) {
	if ((typeDescription = algGetTypeDescription(node))) {
	    tooltipDescription = (char *)xmalloc(2*strlen(typeDescription));
	    parseTooltip(typeDescription, tooltipDescription);
	    if (algGetTypeModule(node)) {
		if ((smiGetNodeModule(node) != smiGetNodeModule(tableNode)) ||
		    (smiGetNodeModule(node) != algGetTypeModule(node))) {
		    length = strlen(tooltipDescription) + 150;
		    tooltip = (char *)xmalloc(length);
		    strcpy(tooltip, algGetTypeName(node));
		    strcat(tooltip, isDefined);
		    strcat(tooltip, algGetTypeModule(node)->name);
		    strcat(tooltip, ":\\n\\n");
		    strcat(tooltip, tooltipDescription);
		} else {
		    length = strlen(tooltipDescription) + 150;
		    tooltip = (char *)xmalloc(length);
		    strcpy(tooltip, tooltipDescription);
		}
	    } else {
		length = strlen(tooltipDescription);
		tooltip = (char *)xmalloc(length);
		strcpy(tooltip, tooltipDescription);
	    }
	    xfree(tooltipDescription);
	    printf(" onmousemove=\"ShowTooltipMZ(evt,'%s')\"", tooltip);
	    printf(" onmouseout=\"HideTooltip(evt)\"");
	    xfree(tooltip);
	} else if (isBaseType(node)) {
	    length = strlen(baseTypeTooltipText) + 1;
	    tooltip = (char *)xmalloc(length);
	    strcpy(tooltip, baseTypeTooltipText);
	    printf(" onmousemove=\"ShowTooltipMZ(evt,'%s')\"", tooltip);
	    printf(" onmouseout=\"HideTooltip(evt)\"");
	    xfree(tooltip);
	}
    }
    printf(">%s</tspan>", algGetTypeName(node));
    switch (node->status) {
    case SMI_STATUS_DEPRECATED:
    case SMI_STATUS_OBSOLETE:
	printf(" (%s)", getStatusString(node->status));
    case SMI_STATUS_MANDATORY:
    case SMI_STATUS_OPTIONAL:
    case SMI_STATUS_CURRENT:
    case SMI_STATUS_UNKNOWN:
	;
    }
    printf("</text>\n");
}

/*
 * prints the related scalars for a given table
 */
static void printSVGRelatedScalars(GraphNode *node, SmiNode *tableNode,
				   int modc, SmiModule **modv,
				   float *textYOffset, float *textXOffset)
{
    GraphEdge *tEdge;
    
    for (tEdge = graphGetFirstEdgeByNode(graph, node);
	 tEdge;
	 tEdge = graphGetNextEdgeByNode(graph, tEdge, node)) {
	if (tEdge->startNode == node  &&
	    tEdge->endNode->smiNode->nodekind == SMI_NODEKIND_SCALAR) {

	    printSVGAttribute(tEdge->endNode->smiNode, tableNode, 0,
			      modc, modv,
			      textYOffset, textXOffset);
	}
    }
}

/*
 * prints all columns objects of the given node
 */
static void printSVGAllColumns(GraphNode *node, SmiNode *tableNode,
			       int modc, SmiModule **modv,
			       float *textYOffset, float *textXOffset)
{
    SmiModule *module  = NULL;
    SmiNode   *smiNode = NULL;
    SmiNode   *ppNode;

    module  = smiGetNodeModule(node->smiNode);

    for (smiNode = smiGetFirstNode(module, SMI_NODEKIND_COLUMN);
	 smiNode;
	 smiNode = smiGetNextNode(smiNode, SMI_NODEKIND_COLUMN)) {
	ppNode = smiGetParentNode(smiNode);
	ppNode = smiGetParentNode(ppNode);
	
	if (!algIsIndexElement(node->smiNode, smiNode) &&
	    cmpSmiNodes(node->smiNode, ppNode))
	    printSVGAttribute(smiNode, tableNode, 0,
			      modc, modv,
			      textYOffset, textXOffset);
    }
}

/*
 * adds the index to an augmenting table (row-element)
 */
static void printSVGAugmentIndex(GraphNode *tNode, SmiNode *tableNode,
				 int modc, SmiModule **modv,
				 float *textYOffset, float *textXOffset)
{
    GraphEdge  *tEdge;
    SmiElement *smiElement;

    for (tEdge = graphGetFirstEdgeByNode(graph, tNode);
	 tEdge;
	 tEdge = graphGetNextEdgeByNode(graph, tEdge, tNode)) {
	if (tEdge->indexkind == SMI_INDEX_AUGMENT) {
	    for (smiElement = smiGetFirstElement(
		smiGetFirstChildNode(tEdge->startNode->smiNode));
		 smiElement;
		 smiElement = smiGetNextElement(smiElement)) {
		if (!cmpSmiNodes(tNode->smiNode, tEdge->startNode->smiNode)) {
		    printSVGAttribute(smiGetElementNode(smiElement), tableNode,
				      1, modc, modv,
				      textYOffset, textXOffset);
		}
	    }
	}
    }
}

/*
 * print "This module doesn't contain any objects"
 */
static void printNoObjects()
{
    printf(" <rect x=\"10\" y=\"10\" width=\"120\" height=\"40\"");
    printf(" fill=\"white\" stroke=\"black\"/>\n");
    printf("  <text x=\"15\" y=\"25\" fill=\"black\">\n");
    printf("   This module doesn't\n");
    printf("  </text>\n");
    printf("  <text x=\"15\" y=\"40\" fill=\"black\">\n");
    printf("   contain any objects.\n");
    printf("  </text>\n");
}

/*
 * print "This module only contains textual conventions"
 */
static void printOnlyTCs()
{
    printf(" <rect x=\"10\" y=\"10\" width=\"150\" height=\"40\"");
    printf(" fill=\"white\" stroke=\"black\"/>\n");
    printf("  <text x=\"15\" y=\"25\" fill=\"black\">\n");
    printf("   This module only contains\n");
    printf("  </text>\n");
    printf("  <text x=\"15\" y=\"40\" fill=\"black\">\n");
    printf("   textual conventions.\n");
    printf("  </text>\n");
}

/*
 * create svg-output for the given node
 */
static void printSVGObject(GraphNode *node, int *classNr,
			   int modc, SmiModule **modv)
{
    SmiElement *smiElement;
    float textXOffset, textYOffset, xOrigin, yOrigin;
    size_t length = 1;
    char *tooltip, *tooltipTable, *tooltipEntry;
    const char *blankLine = "\\n-- -- --\\n";
    
    if (!node) return;

    xOrigin = node->dia.w/-2;
    yOrigin = node->dia.h/-2;
    textYOffset = yOrigin + TABLEHEIGHT + TABLEELEMHEIGHT;
    textXOffset = xOrigin;

    printf("  <g transform=\"translate(%.2f,%.2f)\">\n",
           node->dia.x + node->component->xOffset,
           node->dia.y + node->component->yOffset);
    printf("    <rect id=\"%s\"", node->smiNode->name);
    printf(" x=\"%.2f\" y=\"%.2f\" width=\"%.2f\" height=\"%.2f\"\n",
           xOrigin, yOrigin, node->dia.w, node->dia.h);
    printf("          fill=\"white\" stroke=\"black\"/>\n");
    if (!STATIC_OUTPUT) {
	printf("    <rect x=\"%.2f\" y=\"%.2f\" width=\"16\" height=\"16\"",
	       xOrigin + 2, yOrigin + 2);
	printf(" rx=\"4\" ry=\"4\"");
	printf(" stroke-width=\"3\" stroke=\"gray\" fill=\"white\"\n");
	printf("          onmousedown=\"ClickObj(evt)\"");
	printf(" onclick=\"ClickObj(evt)\"");
	printf(" onmousemove=\"MoveObj(evt)\"");
	printf(" onmouseup=\"OutOfObj(evt)\"");
	printf(" onmouseout=\"OutOfObj(evt)\"/>\n");
    }
    printf("    <polygon points=\"%.2f %.2f %.2f %.2f\"\n",
           xOrigin, yOrigin + TABLEHEIGHT,
           xOrigin + node->dia.w, yOrigin + TABLEHEIGHT);
    printf("          fill=\"none\" stroke=\"black\"/>\n");
    printf("    <text x=\"0\" y=\"%.2f\"", yOrigin + 15);
    printf(" fill=\"%s\"", printFillColor(node->smiNode->status));
    printf(" style=\"text-anchor:middle; font-weight:bold\"");

    /* descriptions for the table and the entries */
    if (!STATIC_OUTPUT) {
	if (node->smiNode->description) {
	    tooltipTable=(char *)xmalloc(2*strlen(node->smiNode->description));
	    parseTooltip(node->smiNode->description, tooltipTable);
	}
	if (smiGetFirstChildNode(node->smiNode)->description) {
	    tooltipEntry=(char *)xmalloc(2*strlen(smiGetFirstChildNode(
						node->smiNode)->description));
	    parseTooltip(smiGetFirstChildNode(node->smiNode)->description,
								tooltipEntry);
	}

	if (node->smiNode->description) {
	    length += strlen(tooltipTable);
	}
	if (node->smiNode->description
		&& smiGetFirstChildNode(node->smiNode)->description) {
	    length += strlen(blankLine);
	}
	if (smiGetFirstChildNode(node->smiNode)->description) {
	    length += strlen(tooltipEntry);
	}
	tooltip = (char *)xmalloc(length);

	strcpy(tooltip, "\0");
	if (node->smiNode->description) {
	    strcat(tooltip, tooltipTable);
	}
	if (node->smiNode->description
		&& smiGetFirstChildNode(node->smiNode)->description) {
	    strcat(tooltip, blankLine);
	}
	if (smiGetFirstChildNode(node->smiNode)->description) {
	    strcat(tooltip, tooltipEntry);
	}

	printf(" onmousemove=\"ShowTooltipMZ(evt,'%s')\"", tooltip);
	printf(" onmouseout=\"HideTooltip(evt)\"");

	if (node->smiNode->description) {
	    xfree(tooltipTable);
	}
	if (smiGetFirstChildNode(node->smiNode)->description) {
	    xfree(tooltipEntry);
	}
	xfree(tooltip);
    }

    printf(">\n");
    printf("         %s",smiGetFirstChildNode(node->smiNode)->name);
    switch (node->smiNode->status) {
    case SMI_STATUS_DEPRECATED:
    case SMI_STATUS_OBSOLETE:
	printf(" (%s)", getStatusString(node->smiNode->status));
    case SMI_STATUS_MANDATORY:
    case SMI_STATUS_OPTIONAL:
    case SMI_STATUS_CURRENT:
    case SMI_STATUS_UNKNOWN:
	;
    }
    printf("</text>\n");

    (*classNr)++;

    if (node->smiNode->nodekind == SMI_NODEKIND_TABLE) {

	if (node->dia.relatedScalars) {
	    /* A */
	    printSVGRelatedScalars(node, node->smiNode,
				   modc, modv,
				   &textYOffset, &textXOffset);

	    printf("    <polygon points=\"%.2f %.2f %.2f %.2f\"\n",
			    xOrigin,
			    textYOffset - TABLEELEMHEIGHT + TABLEBOTTOMHEIGHT,
			    xOrigin + node->dia.w,
			    textYOffset - TABLEELEMHEIGHT + TABLEBOTTOMHEIGHT);
	    printf("          fill=\"none\" stroke=\"black\"/>\n");
	    textYOffset += TABLEBOTTOMHEIGHT;
	}

	if (node->dia.indexObjects) {
	    /* B */
	    printSVGAugmentIndex(node, node->smiNode,
				 modc, modv,
				 &textYOffset, &textXOffset);
	    /* C */
	    for (smiElement = smiGetFirstElement(
		smiGetFirstChildNode(node->smiNode));
		 smiElement;
		 smiElement = smiGetNextElement(smiElement)) {
		printSVGAttribute(smiGetElementNode(smiElement), node->smiNode,
				  1, modc, modv,
				  &textYOffset, &textXOffset);
	    }

	    printf("    <polygon points=\"%.2f %.2f %.2f %.2f\"\n",
			    xOrigin,
			    textYOffset - TABLEELEMHEIGHT + TABLEBOTTOMHEIGHT,
			    xOrigin + node->dia.w,
			    textYOffset - TABLEELEMHEIGHT + TABLEBOTTOMHEIGHT);
	    printf("          fill=\"none\" stroke=\"black\"/>\n");
	    textYOffset += TABLEBOTTOMHEIGHT;
	}

	/* D */
	if (PRINT_DETAILED_ATTR) {
	    printSVGAllColumns(node, node->smiNode,
			       modc, modv,
			       &textYOffset, &textXOffset);
	}
    }

    printf("  </g>\n");
}

/*
 * prints a group of scalars denoted by group
 */
static void printSVGGroup(int group, int *classNr,
			  int modc, SmiModule **modv)
{
    GraphNode *tNode;
    float textXOffset, textYOffset, xOrigin, yOrigin;

    for (tNode = graph->nodes; tNode; tNode = tNode->nextPtr) {
	if (tNode->group == group) break;
    }

    if (!tNode) return;

    xOrigin = tNode->dia.w/-2;
    yOrigin = tNode->dia.h/-2;
    textYOffset = yOrigin + TABLEHEIGHT + TABLEELEMHEIGHT;
    textXOffset = xOrigin;

    printf("  <g transform=\"translate(%.2f,%.2f)\">\n",
           tNode->dia.x + tNode->component->xOffset,
           tNode->dia.y + tNode->component->yOffset);
    printf("    <rect id=\"%s\"",
           smiGetParentNode(tNode->smiNode)->name);
    printf(" x=\"%.2f\" y=\"%.2f\" width=\"%.2f\" height=\"%.2f\"\n",
           xOrigin, yOrigin, tNode->dia.w, tNode->dia.h);
    printf("          fill=\"white\" stroke=\"black\"/>\n");
    if (!STATIC_OUTPUT) {
	printf("    <rect x=\"%.2f\" y=\"%.2f\" width=\"16\" height=\"16\"",
	       xOrigin + 2, yOrigin + 2);
	printf(" rx=\"4\" ry=\"4\"");
	printf(" stroke-width=\"3\" stroke=\"gray\" fill=\"white\"\n");
	printf("          onmousedown=\"ClickObj(evt)\"");
	printf(" onclick=\"ClickObj(evt)\"");
	printf(" onmousemove=\"MoveObj(evt)\"");
	printf(" onmouseup=\"OutOfObj(evt)\"");
	printf(" onmouseout=\"OutOfObj(evt)\"/>\n");
    }
    printf("    <polygon points=\"%.2f %.2f %.2f %.2f\"\n",
           xOrigin, yOrigin + TABLEHEIGHT,
           xOrigin + tNode->dia.w, yOrigin + TABLEHEIGHT);
    printf("          fill=\"none\" stroke=\"black\"/>\n");
    printf("    <text x=\"0\" y=\"%.2f\"", yOrigin + 15);
    printf(" fill=\"%s\"",
		    printFillColor(smiGetParentNode(tNode->smiNode)->status));
    printf(" style=\"text-anchor:middle; font-weight:bold\">\n");
    /* groups don't seem to have a description. */
    printf("         %s", smiGetParentNode(tNode->smiNode)->name);
    switch (smiGetParentNode(tNode->smiNode)->status) {
    case SMI_STATUS_DEPRECATED:
    case SMI_STATUS_OBSOLETE:
	printf(" (%s)",
		    getStatusString(smiGetParentNode(tNode->smiNode)->status));
    case SMI_STATUS_MANDATORY:
    case SMI_STATUS_OPTIONAL:
    case SMI_STATUS_CURRENT:
    case SMI_STATUS_UNKNOWN:
	;
    }
    printf("</text>\n");

    (*classNr)++;

    for (tNode = graph->nodes; tNode; tNode = tNode->nextPtr) {
	if (tNode->group == group) {
	    printSVGAttribute(tNode->smiNode, tNode->smiNode, 0,
			      modc, modv,
			      &textYOffset, &textXOffset);
	}
    }
    
    printf("  </g>\n");
}

static void calculateIntersectionPoints(GraphEdge *tEdge)
{
    float alpha, beta;
    const float PI = acos(-1);

    /* calculate intersection of edge and startNode */
    alpha = atan2(tEdge->startNode->dia.y-tEdge->endNode->dia.y,
		  tEdge->startNode->dia.x-tEdge->endNode->dia.x);
    beta = atan2(tEdge->startNode->dia.h, tEdge->startNode->dia.w);
    if (alpha < 0)
	alpha += PI;
    if (alpha < beta
	|| (alpha > PI-beta && alpha < PI+beta)
	|| alpha > 2*PI-beta) {
	/* intersection at left or right border */
	if (tEdge->startNode->dia.x < tEdge->endNode->dia.x) {
	    tEdge->dia.startX = tEdge->startNode->dia.x +
						    tEdge->startNode->dia.w/2;
	} else {
	    tEdge->dia.startX = tEdge->startNode->dia.x -
						    tEdge->startNode->dia.w/2;
	}
	if (tEdge->startNode->dia.y < tEdge->endNode->dia.y) {
	    tEdge->dia.startY = tEdge->startNode->dia.y +
				fabsf(tEdge->startNode->dia.w*tan(alpha)/2);
	} else {
	    tEdge->dia.startY = tEdge->startNode->dia.y -
				fabsf(tEdge->startNode->dia.w*tan(alpha)/2);
	}
    } else {
	/* intersection at top or bottom border */
	if (tEdge->startNode->dia.y < tEdge->endNode->dia.y) {
	    tEdge->dia.startY = tEdge->startNode->dia.y +
						    tEdge->startNode->dia.h/2;
	} else {
	    tEdge->dia.startY = tEdge->startNode->dia.y -
						    tEdge->startNode->dia.h/2;
	}
	if (tEdge->startNode->dia.x < tEdge->endNode->dia.x) {
	    tEdge->dia.startX = tEdge->startNode->dia.x +
				fabsf(tEdge->startNode->dia.h/(2*tan(alpha)));
	} else {
	    tEdge->dia.startX = tEdge->startNode->dia.x -
				fabsf(tEdge->startNode->dia.h/(2*tan(alpha)));
	}
    }

    /* calculate intersection of edge and endNode */
    alpha = atan2(tEdge->startNode->dia.y-tEdge->endNode->dia.y,
		  tEdge->startNode->dia.x-tEdge->endNode->dia.x);
    beta = atan2(tEdge->endNode->dia.h, tEdge->endNode->dia.w);
    if (alpha < 0)
	alpha += PI;
    if (alpha < beta
	|| (alpha > PI-beta && alpha < PI+beta)
	|| alpha > 2*PI-beta) {
	/* intersection at left or right border */
	if (tEdge->startNode->dia.x > tEdge->endNode->dia.x) {
	    tEdge->dia.endX = tEdge->endNode->dia.x + tEdge->endNode->dia.w/2;
	} else {
	    tEdge->dia.endX = tEdge->endNode->dia.x - tEdge->endNode->dia.w/2;
	}
	if (tEdge->startNode->dia.y > tEdge->endNode->dia.y) {
	    tEdge->dia.endY = tEdge->endNode->dia.y +
				    fabsf(tEdge->endNode->dia.w*tan(alpha)/2);
	} else {
	    tEdge->dia.endY = tEdge->endNode->dia.y -
				    fabsf(tEdge->endNode->dia.w*tan(alpha)/2);
	}
    } else {
	/* intersection at top or bottom border */
	if (tEdge->startNode->dia.y > tEdge->endNode->dia.y) {
	    tEdge->dia.endY = tEdge->endNode->dia.y + tEdge->endNode->dia.h/2;
	} else {
	    tEdge->dia.endY = tEdge->endNode->dia.y - tEdge->endNode->dia.h/2;
	}
	if (tEdge->startNode->dia.x > tEdge->endNode->dia.x) {
	    tEdge->dia.endX = tEdge->endNode->dia.x +
				fabsf(tEdge->endNode->dia.h/(2*tan(alpha)));
	} else {
	    tEdge->dia.endX = tEdge->endNode->dia.x -
				fabsf(tEdge->endNode->dia.h/(2*tan(alpha)));
	}
    }
}

static void printSVGDependency(GraphEdge *tEdge)
{
    int revert = 0;

    calculateIntersectionPoints(tEdge);

    /* print text upside down, if angle is between 180° and 360° */
    if (tEdge->startNode->dia.x > tEdge->endNode->dia.x)
	revert = 1;

    printf(" <path id=\"%s-%s\"\n",
	tEdge->startNode->smiNode->name,
	tEdge->endNode->smiNode->name);
    if (!revert) {
	printf("       d=\"M %.2f %.2f %.2f %.2f\"\n",
	    tEdge->dia.startX + tEdge->startNode->component->xOffset,
	    tEdge->dia.startY + tEdge->startNode->component->yOffset,
	    tEdge->dia.endX + tEdge->endNode->component->xOffset,
	    tEdge->dia.endY + tEdge->endNode->component->yOffset);
    } else {
	printf("       d=\"M %.2f %.2f %.2f %.2f\"\n",
	    tEdge->dia.endX + tEdge->endNode->component->xOffset,
	    tEdge->dia.endY + tEdge->endNode->component->yOffset,
	    tEdge->dia.startX + tEdge->startNode->component->xOffset,
	    tEdge->dia.startY + tEdge->startNode->component->yOffset);
    }
    printf("       stroke-dasharray=\"10, 10\" stroke=\"black\"");
    if (!revert) {
	printf(" marker-end=\"url(#arrowend)\"/>\n");
    } else {
	printf(" marker-start=\"url(#arrowstart)\"/>\n");
    }
}

/*
 * Aggregation is a special case of the association.
 * If aggregate = 1 it is an aggregation if 0 it is an association.
 */
static void printSVGAssociation(GraphEdge *tEdge, int aggregate)
{
    int revert = 0;

    if (aggregate > 1) aggregate = 1;
    if (aggregate < 0) aggregate = 0;

    calculateIntersectionPoints(tEdge);

    /* expands should have cardinalities 1 * */
    if (tEdge->indexkind==SMI_INDEX_EXPAND)
	tEdge->cardinality = GRAPH_CARD_ONE_TO_MANY;

    /* print text upside down, if angle is between 180° and 360° */
    if (tEdge->startNode->dia.x > tEdge->endNode->dia.x)
	revert = 1;

    /* print edge */
    printf(" <path id=\"%s-%s\"\n",
	tEdge->startNode->smiNode->name,
	tEdge->endNode->smiNode->name);
    if (!revert) {
	printf("       d=\"M %.2f %.2f %.2f %.2f\"\n",
	    tEdge->dia.startX + tEdge->startNode->component->xOffset,
	    tEdge->dia.startY + tEdge->startNode->component->yOffset,
	    tEdge->dia.endX + tEdge->endNode->component->xOffset,
	    tEdge->dia.endY + tEdge->endNode->component->yOffset);
    } else {
	printf("       d=\"M %.2f %.2f %.2f %.2f\"\n",
	    tEdge->dia.endX + tEdge->endNode->component->xOffset,
	    tEdge->dia.endY + tEdge->endNode->component->yOffset,
	    tEdge->dia.startX + tEdge->startNode->component->xOffset,
	    tEdge->dia.startY + tEdge->startNode->component->yOffset);
    }
    printf("       stroke=\"black\"");
    if (tEdge->indexkind==SMI_INDEX_AUGMENT ||
	tEdge->indexkind==SMI_INDEX_SPARSE ||
	tEdge->indexkind==SMI_INDEX_EXPAND) {
	if (!revert) {
	    printf(" marker-start=\"url(#arrowstart)\"");
	} else {
	    printf(" marker-end=\"url(#arrowend)\"");
	}
    } else if (tEdge->indexkind==SMI_INDEX_REORDER) {
	printf(" marker-start=\"url(#arrowstart)\"");
	printf(" marker-end=\"url(#arrowend)\"");
    }
    printf("/>\n");

    /* edges without labels are finished here */
    if (tEdge->cardinality==GRAPH_CARD_UNKNOWN)
	return;

    /* print labels */
    printf(" <text text-anchor=\"middle\">\n");
    printf("    <textPath xlink:href=\"#%s-%s\"",
		tEdge->startNode->smiNode->name, tEdge->endNode->smiNode->name);
    if (!revert) {
	printf(" startOffset=\"10%%\">\n");
    } else {
	printf(" startOffset=\"90%%\">\n");
    }
    switch (tEdge->cardinality) {
    case GRAPH_CARD_ZERO_TO_ONE:
    case GRAPH_CARD_ZERO_TO_MANY:
	printf("       0");
	break;
    case GRAPH_CARD_ONE_TO_ONE:
    case GRAPH_CARD_ONE_TO_MANY:
    case GRAPH_CARD_ONE_TO_ZERO_OR_ONE:
	printf("       1");
	break;
    case GRAPH_CARD_UNKNOWN:
	;
    }
    printf("</textPath>\n");
    printf(" </text>\n");

    if (tEdge->indexkind==SMI_INDEX_AUGMENT ||
	tEdge->indexkind==SMI_INDEX_SPARSE ||
	tEdge->indexkind==SMI_INDEX_REORDER ||
	tEdge->indexkind==SMI_INDEX_EXPAND) {
	printf(" <text text-anchor=\"middle\">\n");
	printf("    <textPath xlink:href=\"#%s-%s\" startOffset=\"50%%\">\n",
		tEdge->startNode->smiNode->name, tEdge->endNode->smiNode->name);
    }
    switch(tEdge->indexkind) {
    case SMI_INDEX_AUGMENT:
	printf("       augments");
	break;
    case SMI_INDEX_SPARSE:
	printf("       sparsly augments");
	break;
    case SMI_INDEX_REORDER:
	printf("       reorders");
	break;
    case SMI_INDEX_EXPAND:
	printf("       expands");
	break;
    case SMI_INDEX_UNKNOWN:
    case SMI_INDEX_INDEX:
	;
    }
    if (tEdge->indexkind==SMI_INDEX_AUGMENT ||
	tEdge->indexkind==SMI_INDEX_SPARSE ||
	tEdge->indexkind==SMI_INDEX_REORDER ||
	tEdge->indexkind==SMI_INDEX_EXPAND) {
	printf("</textPath>\n");
	printf(" </text>\n");
    }

    printf(" <text text-anchor=\"middle\">\n");
    printf("    <textPath xlink:href=\"#%s-%s\"",
		tEdge->startNode->smiNode->name, tEdge->endNode->smiNode->name);
    if (!revert) {
	printf(" startOffset=\"90%%\">\n");
    } else {
	printf(" startOffset=\"10%%\">\n");
    }
    switch (tEdge->cardinality) {
    case GRAPH_CARD_ONE_TO_ONE:
    case GRAPH_CARD_ZERO_TO_ONE:
	printf("       1");
	break;
    case GRAPH_CARD_ONE_TO_MANY:
    case GRAPH_CARD_ZERO_TO_MANY:
	printf("       *");
	break;
    case GRAPH_CARD_ONE_TO_ZERO_OR_ONE:
	printf("       0..1");
	break;
    case GRAPH_CARD_UNKNOWN:
	;
    }
    printf("</textPath>\n");
    printf(" </text>\n");
}

static void printSVGConnection(GraphEdge *tEdge)
{
    switch (tEdge->connection) {
    case GRAPH_CON_UNKNOWN:
	break;
    case GRAPH_CON_AGGREGATION :	/* never used??? */
	printSVGAssociation(tEdge,1);
	break;
    case GRAPH_CON_DEPENDENCY :
	printSVGDependency(tEdge);
	break;
    case GRAPH_CON_ASSOCIATION :
	printSVGAssociation(tEdge,0);
	break;	    
    }
}

/*
 * Prints the title of the SVG output file (Modulename and smidump version).
 * TODO
 * Print title somewhere into the SVG.
 * Make size of SVG configurable.
 */
static void printSVGHeaderAndTitle(int modc, SmiModule **modv,
				   int miCount, int idCount,
				   float xMin, float yMin,
				   float xMax, float yMax)
{
    size_t  length1;
    char   *note1;
    int	    i;
    const char *s11 = "Conceptual model of ";
    const char *s12 = "- generated by smidump " SMI_VERSION_STRING;
    float scale;

    scale = max((xMax-xMin)/CANVASWIDTH,(yMax-yMin)/CANVASHEIGHT);

    /*
     * Calculate the length of the string...
     */

    length1 = strlen(s11) + strlen(s12) + 1;
    for (i = 0; i < modc; i++) {
	length1 += strlen(modv[i]->name) + 1;
    }

    /*
     * ... before allocating a buffer and putting the string together.
     */

    note1 = xmalloc(length1);
    strcpy(note1, s11);
    for (i = 0; i < modc; i++) {
	strcat(note1, modv[i]->name);
	strcat(note1, " ");
    }
    strcat(note1, s12);

    printf("<?xml version=\"1.0\"?>\n");
    printf("<!DOCTYPE svg PUBLIC \"-//W3C//DTD SVG 1.1//EN\"\n");
    printf("  \"http://www.w3.org/Graphics/SVG/1.1/DTD/svg11.dtd\">\n");
    printf("<svg preserveAspectRatio=\"xMinYMin meet\"\n");
    printf("     width=\"%i\" height=\"%i\" viewBox=\"%.2f %.2f %.2f %.2f\"\n",
           CANVASWIDTH, CANVASHEIGHT, xMin, yMin, xMax-xMin, yMax-yMin);
    printf("     version=\"1.1\"\n");
    printf("     xmlns=\"http://www.w3.org/2000/svg\"\n");
    printf("     xmlns:xlink=\"http://www.w3.org/1999/xlink\"");
    if (!STATIC_OUTPUT)
	printf("\n     onload=\"init(evt)\" onzoom=\"ZoomControl()\"");
    printf(">\n\n");

    if (!STATIC_OUTPUT) {
	/* css-stylesheet for the tooltip-text */
	printf("<style type=\"text/css\">\n<![CDATA[\ntext.tooltip {\n");
	printf("    font-family: \"Courier New\", Courier, monospace;\n}\n");
	printf("]]>\n</style>\n\n");

	/* the ecma-script for the tooltip */
	/* and the folding of the module information */
	/* and the colorizing of the text */
	printf("<script type=\"text/ecmascript\">\n<![CDATA[\n");
	/* print the script from the included file */
	/* FIXME calculate things dynamically: */
	/*       * maximal number of lines for the tooltip. */
	printf(code, idCount, idCount, idCount, idCount,
			scale, xMin, scale, yMin, DYN_TEXT, DYN_TEXT,
			miCount, miCount, miCount,
			idCount, idCount, idCount, idCount, idCount, idCount);
	printf("// ]]>\n</script>\n\n");
    }

    printf(" <title>%s</title>\n", note1);

    /* definitions for the arrowheads */
    printf(" <defs>\n");
    printf("   <marker id=\"arrowstart\" markerWidth=\"12\"");
    printf(" markerHeight=\"8\" refX=\"0\" refY=\"4\" orient=\"auto\">\n");
    printf("     <line x1=\"12\" y1=\"0\" x2=\"0\" y2=\"4\"");
    printf(" fill=\"none\" stroke=\"black\"/>\n");
    printf("     <line x1=\"0\" y1=\"4\" x2=\"12\" y2=\"8\"");
    printf(" fill=\"none\" stroke=\"black\"/>\n");
    printf("   </marker>\n");
    printf("   <marker id=\"arrowend\" markerWidth=\"12\"");
    printf(" markerHeight=\"8\" refX=\"12\" refY=\"4\" orient=\"auto\">\n");
    printf("     <line x1=\"0\" y1=\"0\" x2=\"12\" y2=\"4\"");
    printf(" fill=\"none\" stroke=\"black\"/>\n");
    printf("     <line x1=\"12\" y1=\"4\" x2=\"0\" y2=\"8\"");
    printf(" fill=\"none\" stroke=\"black\"/>\n");
    printf("   </marker>\n");
    printf(" </defs>\n\n");

    xfree(note1);
}

/*
 * Calculates the size of a given node for the UML representation.
 *
 * FIXME this algorithm may work good for a monospace-font. we have some
 * problems with the proportional-font. :-(
 */
static GraphNode *calcNodeSize(GraphNode *node, int *idCount)
{
    GraphEdge  *tEdge;
    SmiNode    *tNode,*ppNode;
    SmiElement *smiElement;
    SmiModule  *module;
    float      lastHeight;
    int        stringlen;

    if (node->smiNode->nodekind == SMI_NODEKIND_SCALAR) return node;

    node->use = 1;
    node->dia.x = (float) rand();
    node->dia.y = (float) rand();
    node->dia.x /= (float) RAND_MAX;
    node->dia.y /= (float) RAND_MAX;
    node->dia.w = strlen(node->smiNode->name) * HEADFONTSIZETABLE
	+ HEADSPACESIZETABLE;
    node->dia.h = TABLEHEIGHT + TABLEBOTTOMHEIGHT;

    lastHeight = node->dia.h;
    /* A */
    for (tEdge = graphGetFirstEdgeByNode(graph,node);
	 tEdge;
	 tEdge = graphGetNextEdgeByNode(graph, tEdge, node)) {
	if (tEdge->startNode == node &&
	    tEdge->endNode->smiNode->nodekind == SMI_NODEKIND_SCALAR) {

	    tNode = tEdge->endNode->smiNode;

	    if ((tNode->status == SMI_STATUS_DEPRECATED
		&& !SHOW_DEPRECATED && !SHOW_DEPR_OBSOLETE)
		|| (tNode->status == SMI_STATUS_OBSOLETE
		&& !SHOW_DEPR_OBSOLETE))
		continue;

	    stringlen = strlen(tNode->name) + strlen(algGetTypeName(tNode)) +2;
	    switch (tNode->status) {
	    case SMI_STATUS_DEPRECATED:
	    case SMI_STATUS_OBSOLETE:
		stringlen += strlen(getStatusString(tNode->status)) +3;
	    case SMI_STATUS_MANDATORY:
	    case SMI_STATUS_OPTIONAL:
	    case SMI_STATUS_CURRENT:
	    case SMI_STATUS_UNKNOWN:
		;
	    }
	    node->dia.w = max(node->dia.w, stringlen
			  * ATTRFONTSIZE
			  + ATTRSPACESIZE + 5);		
	    node->dia.h += TABLEELEMHEIGHT;
	    (*idCount)++;
	}
    }
    if (node->dia.h > lastHeight) {
	node->dia.relatedScalars = 1;
	node->dia.h += TABLEBOTTOMHEIGHT;
    }

    lastHeight = node->dia.h;
    /* B */
    for (tEdge = graphGetFirstEdgeByNode(graph,node);
	 tEdge;
	 tEdge = graphGetNextEdgeByNode(graph, tEdge, node)) {
	if (tEdge->indexkind == SMI_INDEX_AUGMENT) {
	    for (smiElement = smiGetFirstElement(
		smiGetFirstChildNode(tEdge->startNode->smiNode));
		 smiElement;
		 smiElement = smiGetNextElement(smiElement)) {
		if (!cmpSmiNodes(node->smiNode, tEdge->startNode->smiNode)) {

		    tNode = smiGetElementNode(smiElement);

		    if ((tNode->status == SMI_STATUS_DEPRECATED
			&& !SHOW_DEPRECATED && !SHOW_DEPR_OBSOLETE)
			|| (tNode->status == SMI_STATUS_OBSOLETE
			&& !SHOW_DEPR_OBSOLETE))
			continue;

		    stringlen = strlen(tNode->name)
					    + strlen(algGetTypeName(tNode)) +3;
		    switch (tNode->status) {
		    case SMI_STATUS_DEPRECATED:
		    case SMI_STATUS_OBSOLETE:
			stringlen += strlen(getStatusString(tNode->status)) +3;
		    case SMI_STATUS_MANDATORY:
		    case SMI_STATUS_OPTIONAL:
		    case SMI_STATUS_CURRENT:
		    case SMI_STATUS_UNKNOWN:
			;
		    }
		    node->dia.w = max(node->dia.w, stringlen
						    * ATTRFONTSIZE
						    + ATTRSPACESIZE + 5);
		    node->dia.h += TABLEELEMHEIGHT;
		}
	    }
	}
    }

    /* C */
    for (smiElement = smiGetFirstElement(
	smiGetFirstChildNode(node->smiNode));
	 smiElement;
	 smiElement = smiGetNextElement(smiElement)) {
	
	tNode = smiGetElementNode(smiElement);

	if ((tNode->status == SMI_STATUS_DEPRECATED
	    && !SHOW_DEPRECATED && !SHOW_DEPR_OBSOLETE)
	    || (tNode->status == SMI_STATUS_OBSOLETE
	    && !SHOW_DEPR_OBSOLETE))
	    continue;

	stringlen = strlen(tNode->name) + strlen(algGetTypeName(tNode)) +3;
	switch (tNode->status) {
	case SMI_STATUS_DEPRECATED:
	case SMI_STATUS_OBSOLETE:
	    stringlen += strlen(getStatusString(tNode->status)) +3;
	case SMI_STATUS_MANDATORY:
	case SMI_STATUS_OPTIONAL:
	case SMI_STATUS_CURRENT:
	case SMI_STATUS_UNKNOWN:
	    ;
	}
	node->dia.w = max(node->dia.w, stringlen
		      * ATTRFONTSIZE
		      + ATTRSPACESIZE + 5);
	node->dia.h += TABLEELEMHEIGHT;
    }
    if (node->dia.h > lastHeight) {
	node->dia.indexObjects = 1;
	node->dia.h += TABLEBOTTOMHEIGHT;
    }

    /* D */
    if (PRINT_DETAILED_ATTR && node->smiNode->nodekind == SMI_NODEKIND_TABLE) {
	module  = smiGetNodeModule(node->smiNode);

	for (tNode = smiGetFirstNode(module, SMI_NODEKIND_COLUMN);
	     tNode;
	     tNode = smiGetNextNode(tNode, SMI_NODEKIND_COLUMN)) {
	    ppNode = smiGetParentNode(tNode);
	    ppNode = smiGetParentNode(ppNode);

	    if (cmpSmiNodes(node->smiNode, ppNode)) {
		int len;
		char *typeName;

		if ((tNode->status == SMI_STATUS_DEPRECATED
		    && !SHOW_DEPRECATED && !SHOW_DEPR_OBSOLETE)
		    || (tNode->status == SMI_STATUS_OBSOLETE
		    && !SHOW_DEPR_OBSOLETE))
		    continue;

		typeName = algGetTypeName(tNode);
		len = strlen(tNode->name) + (typeName ? strlen(typeName)+2 : 1);
		switch (tNode->status) {
		case SMI_STATUS_DEPRECATED:
		case SMI_STATUS_OBSOLETE:
		    len += strlen(getStatusString(tNode->status)) +3;
		case SMI_STATUS_MANDATORY:
		case SMI_STATUS_OPTIONAL:
		case SMI_STATUS_CURRENT:
		case SMI_STATUS_UNKNOWN:
		    ;
		}
		node->dia.w = max(node->dia.w, len
		    * ATTRFONTSIZE
		    + ATTRSPACESIZE + 5);
		node->dia.h += TABLEELEMHEIGHT;
		(*idCount)++;
	    }
	}
    }

    return node;
}

/*
 * Calculates the size of a group-node for the UML representation.
 */
static GraphNode *calcGroupSize(int group, int *idCount)
{
    GraphNode *calcNode, *node;
    SmiNode   *tNode;
    int       stringlen;

    for (calcNode = graph->nodes; calcNode; calcNode = calcNode->nextPtr) {
	if (calcNode->group == group) break;
    }

    if (!calcNode) return NULL;

    calcNode->use = 1;
    calcNode->dia.w = strlen(calcNode->smiNode->name) * HEADFONTSIZETABLE
	+ HEADSPACESIZETABLE;
    calcNode->dia.h = TABLEHEIGHT + TABLEBOTTOMHEIGHT;

    for (node = graph->nodes; node; node = node->nextPtr) {
	if (node->group == group) {
	    tNode = node->smiNode;

	    if ((tNode->status == SMI_STATUS_DEPRECATED
		&& !SHOW_DEPRECATED && !SHOW_DEPR_OBSOLETE)
		|| (tNode->status == SMI_STATUS_OBSOLETE
		&& !SHOW_DEPR_OBSOLETE))
		continue;

	    stringlen = strlen(tNode->name) + strlen(algGetTypeName(tNode)) +2;
	    switch (tNode->status) {
	    case SMI_STATUS_DEPRECATED:
	    case SMI_STATUS_OBSOLETE:
		stringlen += strlen(getStatusString(tNode->status)) +3;
	    case SMI_STATUS_MANDATORY:
	    case SMI_STATUS_OPTIONAL:
	    case SMI_STATUS_CURRENT:
	    case SMI_STATUS_UNKNOWN:
		;
	    }
	    calcNode->dia.w = max(calcNode->dia.w, stringlen
			    * ATTRFONTSIZE
			    + ATTRSPACESIZE + 5);
	    calcNode->dia.h += TABLEELEMHEIGHT;
	    (*idCount)++;
	}
    }

    return calcNode;
}


/* ------------------------------------------------------------------------- */



static int invalidType(SmiBasetype basetype)
{
    return (basetype == SMI_BASETYPE_FLOAT32)
	|| (basetype == SMI_BASETYPE_FLOAT64)
	|| (basetype == SMI_BASETYPE_FLOAT128);
}

static int countTCs(int modc, SmiModule **modv)
{
    SmiType *smiType;
    int     i, invalid, j = 0;

    for (i=0; i<modc; i++) {
	for(smiType = smiGetFirstType(modv[i]);
	    smiType; smiType = smiGetNextType(smiType)) {
	    if (smiType->status != SMI_STATUS_UNKNOWN) {
		invalid = invalidType(smiType->basetype);
		if (!invalid) {
		    j++;
		}
	    }
	}
    }

    return j;
}


static void calcModuleIdentityCount(int modc, SmiModule **modv,
				    int *miCount, int modId[])
{
    int         i;
    SmiNode     *smiNode;
    SmiRevision *smiRevision;

    /* MODULE-IDENTITY */
    (*miCount)++;
    for (i = 0; i < modc; i++) {
	modId[i] = 0;
	smiNode = smiGetModuleIdentityNode(modv[i]);
	if (smiNode) {
	    /* name of the module */
	    (*miCount)++;
	    modId[i] = 1;
	    /* revision history of the module */
	    smiRevision = smiGetFirstRevision(modv[i]);
	    if (!smiRevision) {
		(*miCount)++;
	    } else {
		for(; smiRevision;
				smiRevision = smiGetNextRevision(smiRevision)) {
		    (*miCount)++;
		}
	    }
	}
    }
}

static void calcNotificationTypeCount(int modc, SmiModule **modv,
				      int *miCount, int nType[])
{
    int     i;
    SmiNode *smiNode;

    /* NOTIFICATION-TYPE */
    (*miCount)++;
    for (i = 0; i < modc; i++) {
	nType[i] = 0;
	smiNode = smiGetModuleIdentityNode(modv[i]);
	if (smiNode) {
	    /* name of the module */
	    (*miCount)++;
	    /* name of the notification */
	    for (smiNode = smiGetFirstNode(modv[i], SMI_NODEKIND_NOTIFICATION);
		smiNode;
		smiNode = smiGetNextNode(smiNode, SMI_NODEKIND_NOTIFICATION)) {
		if ((smiNode->status == SMI_STATUS_DEPRECATED
		    && !SHOW_DEPRECATED && !SHOW_DEPR_OBSOLETE)
		    || (smiNode->status == SMI_STATUS_OBSOLETE
		    && !SHOW_DEPR_OBSOLETE))
		    continue;
		(*miCount)++;
		nType[i] = 1;
	    }
	}
    }
}

static void calcObjectGroupCount(int modc, SmiModule **modv,
				 int *miCount, int oGroup[])
{
    int     i;
    SmiNode *smiNode;

    /* OBJECT-GROUP */
    (*miCount)++;
    for (i = 0; i < modc; i++) {
	oGroup[i] = 0;
	smiNode = smiGetModuleIdentityNode(modv[i]);
	if (smiNode) {
	    /* name of the module */
	    (*miCount)++;
	    /* name of the group */
	    for (smiNode = smiGetFirstNode(modv[i], SMI_NODEKIND_GROUP);
		smiNode;
		smiNode = smiGetNextNode(smiNode, SMI_NODEKIND_GROUP)) {
		if (!isObjectGroup(smiNode))
		    continue;
		if ((smiNode->status == SMI_STATUS_DEPRECATED
		    && !SHOW_DEPRECATED && !SHOW_DEPR_OBSOLETE)
		    || (smiNode->status == SMI_STATUS_OBSOLETE
		    && !SHOW_DEPR_OBSOLETE))
		    continue;
		(*miCount)++;
		oGroup[i] = 1;
	    }
	}
    }
}

static void calcNotificationGroupCount(int modc, SmiModule **modv,
				       int *miCount, int nGroup[])
{
    int     i;
    SmiNode *smiNode;

    /* NOTIFICATION-GROUP */
    (*miCount)++;
    for (i = 0; i < modc; i++) {
	nGroup[i] = 0;
	smiNode = smiGetModuleIdentityNode(modv[i]);
	if (smiNode) {
	    /* name of the module */
	    (*miCount)++;
	    /* name of the group */
	    for (smiNode = smiGetFirstNode(modv[i], SMI_NODEKIND_GROUP);
		smiNode;
		smiNode = smiGetNextNode(smiNode, SMI_NODEKIND_GROUP)) {
		if (!isNotificationGroup(smiNode))
		    continue;
		if ((smiNode->status == SMI_STATUS_DEPRECATED
		    && !SHOW_DEPRECATED && !SHOW_DEPR_OBSOLETE)
		    || (smiNode->status == SMI_STATUS_OBSOLETE
		    && !SHOW_DEPR_OBSOLETE))
		    continue;
		(*miCount)++;
		nGroup[i] = 1;
	    }
	}
    }
}

static void calcModuleComplianceCount(int modc, SmiModule **modv,
				      int *miCount, int mCompl[])
{
    int           i;
    char          *done = NULL;
    char          s[1024];
    char          *module;
    SmiNode       *smiNode, *smiNode2;
    SmiModule     *smiModule2;
    SmiElement    *smiElement;
    SmiOption     *smiOption;
    SmiRefinement *smiRefinement;

    /* MODULE-COMPLIANCE */
    (*miCount)++;
    for (i = 0; i < modc; i++) {
	mCompl[i] = 0;
	smiNode = smiGetModuleIdentityNode(modv[i]);
	if (smiNode) {
	    /* name of the module */
	    (*miCount)++;
	    /* name of the compliance */
	    for (smiNode = smiGetFirstNode(modv[i], SMI_NODEKIND_COMPLIANCE);
		smiNode;
		smiNode = smiGetNextNode(smiNode, SMI_NODEKIND_COMPLIANCE)) {
		if ((smiNode->status == SMI_STATUS_DEPRECATED
		    && !SHOW_DEPRECATED && !SHOW_DEPR_OBSOLETE)
		    || (smiNode->status == SMI_STATUS_OBSOLETE
		    && !SHOW_DEPR_OBSOLETE))
		    continue;
		(*miCount)++;
		mCompl[i] = 1;
		/* modules for the compliance */
		done = xstrdup("+");
		for (module = modv[i]->name; module; ) {
		    /* name of the module */
		    (*miCount)++;
		    /* mandatory groups */
		    (*miCount)++;
		    /* groups */
		    for (smiOption = smiGetFirstOption(smiNode); smiOption;
				    smiOption = smiGetNextOption(smiOption)) {
			smiNode2 = smiGetOptionNode(smiOption);
			smiModule2 = smiGetNodeModule(smiNode2);
			if (!strcmp(smiModule2->name, module)) {
			    (*miCount)++;
			}
		    }
		    /* objects */
		    for (smiRefinement = smiGetFirstRefinement(smiNode);
			smiRefinement;
			smiRefinement = smiGetNextRefinement(smiRefinement)) {
			smiNode2 = smiGetRefinementNode(smiRefinement);
			smiModule2 = smiGetNodeModule(smiNode2);
			if (!strcmp(smiModule2->name, module)) {
			    (*miCount)++;
			}
		    }
		    /* find next module */
		    done = xrealloc(done,
				strlen(done)+strlen(module)+2*sizeof(char));
		    strcat(done, module);
		    strcat(done, "+");
		    module = NULL;
		    for (smiElement = smiGetFirstElement(smiNode);
			 smiElement;
			 smiElement = smiGetNextElement(smiElement)) {
			sprintf(s, "+%s+", smiGetNodeModule(smiGetElementNode(
							smiElement))->name);
			if ((!strstr(done, s))) {
			    module = smiGetNodeModule(smiGetElementNode(
							smiElement))->name;
			    break;
			}
		    }
		}
		xfree(done);
	    }
	}
    }
}

/*
 * calculate the number of entries in the module-information-section.
 * headings for empty sections are counted here, but they are omitted
 * in the svg, so the calculated number is an upper bound. the maximal
 * size of this gap is 4*(modc+1). this may be considered as a bug.
 */
static void prepareModInfo(int modc, SmiModule **modv, int *miCount,
	    int modId[], int nType[], int oGroup[], int nGroup[], int mCompl[])
{
    calcModuleIdentityCount(modc, modv, miCount, modId);
    calcNotificationTypeCount(modc, modv, miCount, nType);
    calcObjectGroupCount(modc, modv, miCount, oGroup);
    calcNotificationGroupCount(modc, modv, miCount, nGroup);
    calcModuleComplianceCount(modc, modv, miCount, mCompl);
}


/* ------------------------------------------------------------------------- */

static void populateMarkupList(SmiNode *smiNode, int *miNr,
			       StringListElem markupList[], int miCount)
{
    int            i;
    SmiElement     *smiElement;
    StringListElem *lastElem;
    StringListElem *tElem;
    StringListElem *newElem;
    StringListElem *cloneElem;

    markupList[*miNr].miElem = smiNode->name;
    markupList[*miNr].status = smiNode->status;
    markupList[*miNr].nextPtr = NULL;

    for (smiElement = smiGetFirstElement(smiNode); smiElement;
	smiElement = smiGetNextElement(smiElement)) {

	newElem = xmalloc(sizeof(StringListElem));
	memset(newElem, 0, sizeof(StringListElem));
	newElem->miElem = smiGetElementNode(smiElement)->name;
	newElem->status = smiGetElementNode(smiElement)->status;

	if (markupList[*miNr].nextPtr == NULL) {
	    markupList[*miNr].nextPtr = newElem;
	} else {
	    for (tElem = markupList[*miNr].nextPtr;
		tElem; tElem = tElem->nextPtr) {
		lastElem = tElem;
	    }
	    lastElem->nextPtr = newElem;
	}
	if (isNotificationGroup(smiNode)) {
	    for (i=0; i<miCount; i++) {
		if (markupList[i].miElem == NULL)
		    continue;
		if (markupList[i].miElem != newElem->miElem)
		    continue;
		for (tElem = markupList[i].nextPtr;
		    tElem; tElem = tElem->nextPtr) {
		    cloneElem = xmalloc(sizeof(StringListElem));
		    memcpy(cloneElem, tElem, sizeof(StringListElem));
		    newElem->nextPtr = cloneElem;
		    newElem = newElem->nextPtr;
		}
	    }
	}
    }
}

static void printInformationNode(SmiNode *smiNode,
				 float *x, float *y, int *miNr,
				 StringListElem markupList[], int miCount)
{
    int            j, k;
    char           *tooltip;
    SmiElement     *smiElement;
    StringListElem *tElem;

    printf(" <g id=\"MI%i\" transform=\"translate", *miNr);
    printf("(%.2f,%.2f)\">\n", *x, *y);
    printf("  <text id=\"%s\"", smiNode->name);
    printf(" fill=\"%s\"", printFillColor(smiNode->status));

    if (!STATIC_OUTPUT) {
	smiElement = smiGetFirstElement(smiNode);
	if (smiElement || smiNode->description) {
	    printf(" onmousemove=\"");
	}
	if (smiNode->description) {
	    tooltip = (char *)xmalloc(2*strlen(smiNode->description));
	    parseTooltip(smiNode->description, tooltip);
	    printf("ShowTooltipMZ(evt,'%s')", tooltip);
	    xfree(tooltip);
	}
	if (smiElement && smiNode->description) {
	    printf(";");
	}
	for (j = 0; smiElement;
	    j++, smiElement = smiGetNextElement(smiElement)) {
	    if (j) {
		printf(";");
	    }
	    printf("colorText('%s','red')",
					smiGetElementNode(smiElement)->name);
	    if (isNotificationGroup(smiNode)) {
		/* parse markupList */
		for (k=0; k<miCount; k++) {
		    if (markupList[k].miElem == NULL)
			continue;
		    if (markupList[k].miElem !=
					smiGetElementNode(smiElement)->name)
			continue;
		    for (tElem = markupList[k].nextPtr;
			tElem; tElem = tElem->nextPtr) {
			printf(";colorText('%s','red')", tElem->miElem);
		    }
		}
	    }
	}
	if (j || smiNode->description) {
	    printf("\"");
	}

	smiElement = smiGetFirstElement(smiNode);
	if (smiElement) {
	    printf(" onclick=\"setStatus(evt,'red','%s')",
					    printFillColor(smiNode->status));
	}
	for (j = 0; smiElement;
	    j++, smiElement = smiGetNextElement(smiElement)) {
	    printf(";changeColor(evt,'%s','red','%s')",
			smiGetElementNode(smiElement)->name,
			printFillColor(smiGetElementNode(smiElement)->status));
	    if (isNotificationGroup(smiNode)) {
		/* parse markupList */
		for (k=0; k<miCount; k++) {
		    if (markupList[k].miElem == NULL)
			continue;
		    if (markupList[k].miElem !=
					smiGetElementNode(smiElement)->name)
			continue;
		    for (tElem = markupList[k].nextPtr;
			tElem; tElem = tElem->nextPtr) {
			printf(";changeColor(evt,'%s','red','%s')",
				tElem->miElem, printFillColor(tElem->status));
		    }
		}
	    }
	}
	if (j) {
	    printf("\"");
	}

	smiElement = smiGetFirstElement(smiNode);
	if (smiElement || smiNode->description) {
	    printf(" onmouseout=\"");
	}
	if (smiNode->description) {
	    printf("HideTooltip(evt)");
	}
	if (smiElement && smiNode->description) {
	    printf(";");
	}
	for (j = 0; smiElement;
	    j++, smiElement = smiGetNextElement(smiElement)) {
	    if (j) {
		printf(";");
	    }
	    printf("colorText('%s',", smiGetElementNode(smiElement)->name);
	    printf("'%s')",
			printFillColor(smiGetElementNode(smiElement)->status));
	    if (isNotificationGroup(smiNode)) {
		/* parse markupList */
		for (k=0; k<miCount; k++) {
		    if (markupList[k].miElem == NULL)
			continue;
		    if (markupList[k].miElem !=
				    smiGetElementNode(smiElement)->name)
			continue;
		    for (tElem = markupList[k].nextPtr;
			tElem; tElem = tElem->nextPtr) {
			printf(";colorText('%s',", tElem->miElem);
			printf("'%s')", printFillColor(tElem->status));
		    }
		}
	    }
	}
	if (j || smiNode->description) {
	    printf("\"");
	}
    }

    printf(">%s", smiNode->name);
    switch (smiNode->status) {
    case SMI_STATUS_DEPRECATED:
    case SMI_STATUS_OBSOLETE:
	printf(" (%s)", getStatusString(smiNode->status));
    case SMI_STATUS_MANDATORY:
    case SMI_STATUS_OPTIONAL:
    case SMI_STATUS_CURRENT:
    case SMI_STATUS_UNKNOWN:
	;
    }
    printf("</text>\n");
    printf(" </g>\n");
    *y += TABLEELEMHEIGHT;
    (*miNr)++;
}

static void printComplianceNode(SmiNode *smiNode, int modc, SmiModule **modv,
				float *x, float *y, int *miNr, int i,
				StringListElem markupList[], int miCount)
{
    int            j, k, foreign_exists, textColor = 0;
    char           *tooltip;
    char           *done = NULL;
    char           s[1024];
    char           *module;
    SmiNode        *smiNode2;
    SmiModule      *smiModule2;
    SmiElement     *smiElement;
    /* SmiRevision    *smiRevision; */
    SmiOption      *smiOption;
    SmiRefinement  *smiRefinement;
    StringListElem *tElem;

    printf(" <g id=\"MI%i\" transform=\"translate", *miNr);
    printf("(%.2f,%.2f)\">\n", *x, *y);
    printf("  <text");
    switch (smiNode->status) {
    case SMI_STATUS_DEPRECATED:
	printf(" fill=\"rgb(40%%,40%%,40%%)\"");
	textColor = 40;
	break;
    case SMI_STATUS_OBSOLETE:
	printf(" fill=\"rgb(60%%,60%%,60%%)\"");
	textColor = 60;
	break;
    case SMI_STATUS_CURRENT:
    case SMI_STATUS_MANDATORY:
	printf(" fill=\"rgb(0%%,0%%,0%%)\"");
	textColor = 0;
	break;
    case SMI_STATUS_OPTIONAL:
	printf(" fill=\"rgb(20%%,20%%,20%%)\"");
	textColor = 20;
	break;
    case SMI_STATUS_UNKNOWN:
	;
    }
    printf(">\n");

    if (!STATIC_OUTPUT) {
	printf("   <tspan style=\"text-anchor:middle\"");
	printf(" onclick=\"collapse(evt)\">--</tspan>\n");
    }
    printf("   <tspan x=\"5\"");

    if (!STATIC_OUTPUT && smiNode->description) {
	tooltip = (char *)xmalloc(2*strlen(smiNode->description));
	parseTooltip(smiNode->description, tooltip);
	printf(" onmousemove=\"ShowTooltipMZ(evt,'%s')", tooltip);
	xfree(tooltip);
	printf("\" onmouseout=\"HideTooltip(evt)\"");
    }
    printf(">%s", smiNode->name);
    switch (smiNode->status) {
    case SMI_STATUS_DEPRECATED:
    case SMI_STATUS_OBSOLETE:
	printf(" (%s)", getStatusString(smiNode->status));
    case SMI_STATUS_MANDATORY:
    case SMI_STATUS_OPTIONAL:
    case SMI_STATUS_CURRENT:
    case SMI_STATUS_UNKNOWN:
	;
    }
    printf("</tspan>\n");
    printf("  </text>\n");
    printf(" </g>\n");
    (*miNr)++;
    *y += TABLEELEMHEIGHT;

    /* modules for the compliance */
    *x += TABLEELEMHEIGHT;
    done = xstrdup("+");
    for (module = modv[i]->name; module; ) {
	foreign_exists = 0;
	if (module == modv[i]->name) {
	    foreign_exists = 1;
	} else {
	    for (j = 0; j < modc; j++) {
		if (module == modv[j]->name) {
		    foreign_exists = 1;
		    break;
		}
	    }
	}
	printf(" <g id=\"MI%i\" transform=\"translate", *miNr);
	printf("(%.2f,%.2f)\">\n", *x, *y);
	printf("  <text fill=\"rgb(%i%%,%i%%,%i%%)\">\n",
					    textColor, textColor, textColor);
	if (!STATIC_OUTPUT) {
	    printf("   <tspan style=\"text-anchor:middle\"");
	    printf(" onclick=\"collapse(evt)\">--</tspan>\n");
	}
	if (!foreign_exists && !STATIC_OUTPUT) {
	    printf("   <tspan fill=\"%s\" x=\"5\">\n", linkcolor);
	    printf("    <a xlink:href=\"%s", link);
	    for (k=0; k<modc; k++) {
		printf("&amp;mibs=%s", modv[k]->name);
	    }
	    printf("&amp;mibs=%s\">", module);
	    printf("%s", module);
	    printf("</a>\n");
	    printf("   </tspan>\n");
	} else {
	    printf("    <tspan x=\"5\">%s</tspan>\n", module);
	}
	printf("  </text>\n");
	printf(" </g>\n");
	(*miNr)++;
	*y += TABLEELEMHEIGHT;

	/* mandatory groups */
	*x += TABLEELEMHEIGHT;
	*x += TABLEBOTTOMHEIGHT;
	printf(" <g id=\"MI%i\" transform=\"translate", *miNr);
	printf("(%.2f,%.2f)\">\n", *x, *y);
	printf("  <text id=\"mandatorygroups%s%s\"", smiNode->name, module);
	printf(" fill=\"rgb(%i%%,%i%%,%i%%)\"",
					    textColor, textColor, textColor);
	if (!STATIC_OUTPUT && foreign_exists) {
	    smiElement = smiGetFirstElement(smiNode);
	    if (smiElement) {
		printf(" onmousemove=\"");
	    }
	    for (j = 0; smiElement;
		j++, smiElement = smiGetNextElement(smiElement)) {
		if (!strcmp(smiGetNodeModule(smiGetElementNode(
						smiElement))->name, module)) {
		    if (j) {
			printf(";");
		    }
		    printf("colorText('%s','red')",
					smiGetElementNode(smiElement)->name);
		    /* parse markupList */
		    for (k=0; k<miCount; k++) {
			if (markupList[k].miElem == NULL)
			    continue;
			if (markupList[k].miElem !=
					smiGetElementNode(smiElement)->name)
			    continue;
			for (tElem = markupList[k].nextPtr;
			    tElem; tElem = tElem->nextPtr) {
			    printf(";colorText('%s','red')", tElem->miElem);
			}
		    }
		}
	    }
	    if (j) {
		printf("\"");
	    }

	    smiElement = smiGetFirstElement(smiNode);
	    if (smiElement) {
		printf(" onclick=\"setStatus(evt,'red','%s')",
					    printFillColor(smiNode->status));
	    }
	    for (j = 0; smiElement;
		j++, smiElement = smiGetNextElement(smiElement)) {
		if (!strcmp(smiGetNodeModule(smiGetElementNode(
						smiElement))->name, module)) {
		    printf(";changeColor(evt,'%s','red','%s')",
			smiGetElementNode(smiElement)->name,
			printFillColor(smiGetElementNode(smiElement)->status));
		    /* parse markupList */
		    for (k=0; k<miCount; k++) {
			if (markupList[k].miElem == NULL)
			    continue;
			if (markupList[k].miElem !=
					    smiGetElementNode(smiElement)->name)
			    continue;
			for (tElem = markupList[k].nextPtr;
			    tElem; tElem = tElem->nextPtr) {
			    printf(";changeColor(evt,'%s','red','%s')",
				tElem->miElem, printFillColor(tElem->status));
			}
		    }
		}
	    }
	    if (j) {
		printf("\"");
	    }

	    smiElement = smiGetFirstElement(smiNode);
	    if (smiElement) {
		printf(" onmouseout=\"");
	    }
	    for (j = 0; smiElement;
		j++, smiElement = smiGetNextElement(smiElement)) {
		if (!strcmp(smiGetNodeModule(smiGetElementNode(
						smiElement))->name, module)) {
		    if (j) {
			printf(";");
		    }
		    printf("colorText('%s',",
					smiGetElementNode(smiElement)->name);
		    printf("'%s')",
			printFillColor(smiGetElementNode(smiElement)->status));
		    /* parse markupList */
		    for (k=0; k<miCount; k++) {
			if (markupList[k].miElem == NULL)
			    continue;
			if (markupList[k].miElem !=
					smiGetElementNode(smiElement)->name)
			    continue;
			for (tElem = markupList[k].nextPtr;
			    tElem; tElem = tElem->nextPtr) {
			    printf(";colorText('%s',", tElem->miElem);
			    printf("'%s')", printFillColor(tElem->status));
			}
		    }
		}
	    }
	    if (j) {
		printf("\"");
	    }
	}
	printf(">Mandatory Groups</text>\n");
	printf(" </g>\n");
	*y += TABLEELEMHEIGHT;
	(*miNr)++;

	/* groups */
	for (smiOption = smiGetFirstOption(smiNode); smiOption;
				    smiOption = smiGetNextOption(smiOption)) {
	    smiNode2 = smiGetOptionNode(smiOption);
	    smiModule2 = smiGetNodeModule(smiNode2);
	    if (!strcmp(smiModule2->name, module)) {
		printf(" <g id=\"MI%i\" transform=", *miNr);
		printf("\"translate(%.2f,%.2f)\">\n", *x, *y);
		printf("  <text id=\"group%s%s%s\"",
					smiNode->name, smiNode2->name, module);
		printf(" fill=\"rgb(%i%%,%i%%,%i%%)\"",
					    textColor, textColor, textColor);
		if (!STATIC_OUTPUT) {
		    printf(" onmousemove=\"");
		    if (smiOption->description) {
			tooltip = (char *)xmalloc(2*strlen(
						    smiOption->description));
			parseTooltip(smiOption->description, tooltip);
			printf("ShowTooltipMZ(evt,'%s')", tooltip);
			xfree(tooltip);
		    }
		    if (smiOption->description && foreign_exists)
			printf(";");
		    if (foreign_exists) {
			printf("colorText('%s','salmon')", smiNode2->name);
			/* parse markupList */
			for (j=0; j<miCount; j++) {
			    if (markupList[j].miElem == NULL)
				continue;
			    if (markupList[j].miElem != smiNode2->name)
				continue;
			    for (tElem = markupList[j].nextPtr;
				tElem; tElem = tElem->nextPtr) {
				printf(";colorText('%s','salmon')",
								tElem->miElem);
			    }
			}
		    }

		    if (foreign_exists) {
			printf("\" onclick=\"setStatus(evt,'salmon','%s')",
					    printFillColor(smiNode2->status));
			printf(";changeColor(evt,'%s','salmon','%s')",
			    smiNode2->name, printFillColor(smiNode2->status));
			/* parse markupList */
			for (j=0; j<miCount; j++) {
			    if (markupList[j].miElem == NULL)
				continue;
			    if (markupList[j].miElem != smiNode2->name)
				continue;
			    for (tElem = markupList[j].nextPtr;
				tElem; tElem = tElem->nextPtr) {
				printf(";changeColor(evt,'%s','salmon','%s')",
						tElem->miElem,
						printFillColor(tElem->status));
			    }
			}
		    }

		    printf("\" onmouseout=\"");
		    if (smiOption->description) {
			printf("HideTooltip(evt)");
		    }
		    if (smiOption->description && foreign_exists)
			printf(";");
		    if (foreign_exists) {
			printf("colorText('%s',", smiNode2->name);
			printf("'%s')", printFillColor(smiNode2->status));
			/* parse markupList */
			for (j=0; j<miCount; j++) {
			    if (markupList[j].miElem == NULL)
				continue;
			    if (markupList[j].miElem != smiNode2->name)
				continue;
			    for (tElem = markupList[j].nextPtr;
				tElem; tElem = tElem->nextPtr) {
				printf(";colorText('%s',", tElem->miElem);
				printf("'%s')", printFillColor(tElem->status));
			    }
			}
		    }
		    printf("\"");
		}
		printf(">Group %s</text>\n", smiNode2->name);
		printf(" </g>\n");
		*y += TABLEELEMHEIGHT;
		(*miNr)++;
	    }
	}

	/* objects */
	for (smiRefinement = smiGetFirstRefinement(smiNode); smiRefinement;
			smiRefinement = smiGetNextRefinement(smiRefinement)) {
	    smiNode2 = smiGetRefinementNode(smiRefinement);
	    smiModule2 = smiGetNodeModule(smiNode2);
	    if (!strcmp(smiModule2->name, module)) {
		printf(" <g id=\"MI%i\" transform=", *miNr);
		printf("\"translate(%.2f,%.2f)\">\n", *x, *y);
		printf("  <text id=\"object%s%s%s\"",
					smiNode->name, smiNode2->name, module);
		printf(" fill=\"rgb(%i%%,%i%%,%i%%)\"",
					    textColor, textColor, textColor);
		if (!STATIC_OUTPUT) {
		    printf(" onmousemove=\"");
		    if (smiRefinement->description) {
			tooltip = (char *)xmalloc(2*strlen(
						smiRefinement->description));
			parseTooltip(smiRefinement->description, tooltip);
			printf("ShowTooltipMZ(evt,'%s')", tooltip);
			xfree(tooltip);
		    }
		    if (smiRefinement->description && foreign_exists)
			printf(";");
		    if (foreign_exists)
			printf("colorText('%s','salmon')", smiNode2->name);

		    if (foreign_exists) {
			printf("\" onclick=\"setStatus(evt,'salmon','%s')",
					    printFillColor(smiNode2->status));
			printf(";changeColor(evt,'%s','salmon','%s')",
			    smiNode2->name, printFillColor(smiNode2->status));
		    }

		    printf("\" onmouseout=\"");
		    if (smiRefinement->description) {
			printf("HideTooltip(evt)");
		    }
		    if (smiRefinement->description && foreign_exists)
			printf(";");
		    if (foreign_exists) {
			printf("colorText('%s',", smiNode2->name);
			printf("'%s')", printFillColor(smiNode2->status));
		    }
		    printf("\"");
		}
		printf(">Object %s</text>\n", smiNode2->name);
		printf(" </g>\n");
		*y += TABLEELEMHEIGHT;
		(*miNr)++;
	    }
	}
	*x -= TABLEELEMHEIGHT;
	*x -= TABLEBOTTOMHEIGHT;

	/* find next module */
	done = xrealloc(done, strlen(done)+strlen(module)+2*sizeof(char));
	strcat(done, module);
	strcat(done, "+");
	module = NULL;
	for (smiElement = smiGetFirstElement(smiNode); smiElement;
				smiElement = smiGetNextElement(smiElement)) {
	    sprintf(s, "+%s+", smiGetNodeModule(smiGetElementNode(
							smiElement))->name);
	    if ((!strstr(done, s))) {
		module = smiGetNodeModule(smiGetElementNode(smiElement))->name;
		break;
	    }
	}
    }
    xfree(done);
    *x -= TABLEELEMHEIGHT;
}

static void printModuleIdentity(int modc, SmiModule **modv,
				float *x, float *y, int *miNr)
{
    int         i, j;
    char        *tooltip;
    SmiNode     *smiNode;
    /* SmiElement  *smiElement; */
    SmiRevision *smiRevision;
    GraphNode   *tNode;

    printf(" <g id=\"MI%i\" transform=\"translate(%.2f,%.2f)\">\n",
								*miNr, *x, *y);
    printf("  <text>\n");
    if (!STATIC_OUTPUT) {
	printf("   <tspan style=\"text-anchor:middle\"");
	printf(" onclick=\"collapse(evt)\">--</tspan>\n");
    }
    printf("   <tspan x=\"5\">Modules</tspan>\n");
    printf("  </text>\n");
    printf(" </g>\n");
    (*miNr)++;
    *y += TABLEELEMHEIGHT;
    for (i = 0; i < modc; i++) {
	smiNode = smiGetModuleIdentityNode(modv[i]);
	if (smiNode) {

	    /* name and description of the module. */
	    *x += TABLEELEMHEIGHT;
	    printf(" <g id=\"MI%i\" transform=\"translate(%.2f,%.2f)\">\n",
								*miNr, *x, *y);
	    printf("  <text>\n");
	    if (!STATIC_OUTPUT) {
		printf("   <tspan style=\"text-anchor:middle\"");
		printf(" onclick=\"collapse(evt)\">--</tspan>\n");
	    }
	    printf("   <tspan x=\"5\"");
	    if (!STATIC_OUTPUT) {
		if (modv[i]->description || modc > 1) {
		    printf(" onmousemove=\"");
		}
		if (modv[i]->description) {
		    tooltip = (char *)xmalloc(2*strlen(modv[i]->description));
		    parseTooltip(modv[i]->description, tooltip);
		    printf("ShowTooltipMZ(evt,'%s')", tooltip);
		    xfree(tooltip);
		}
		if (modv[i]->description && modc > 1) {
		    printf(";");
		}
		if (modc > 1) {
		    j = 0;
		    for (tNode = graph->nodes; tNode; tNode = tNode->nextPtr) {
			if (tNode->smiModule == modv[i] && tNode->use) {
			    if (j) {
				printf(";");
			    }
			    if (tNode->group == 0) {
				printf("colorText('%s','blanchedalmond')",
				    tNode->smiNode->name);
			    } else {
				printf("colorText('%s','blanchedalmond')",
				    smiGetParentNode(tNode->smiNode)->name);
			    }
			    j++;
			}
		    }
		}
		if (modv[i]->description || modc > 1) {
		    printf("\" onmouseout=\"");
		}
		if (modv[i]->description) {
		    printf("HideTooltip(evt)");
		}
		if (modv[i]->description && modc > 1) {
		    printf(";");
		}
		if (modc > 1) {
		    j = 0;
		    for (tNode = graph->nodes; tNode; tNode = tNode->nextPtr) {
			if (tNode->smiModule == modv[i] && tNode->use) {
			    if (j) {
				printf(";");
			    }
			    if (tNode->group == 0) {
				printf("colorText('%s','white')",
				    tNode->smiNode->name);
			    } else {
				printf("colorText('%s','white')",
				    smiGetParentNode(tNode->smiNode)->name);
			    }
			    j++;
			}
		    }
		}
		if (modv[i]->description || modc > 1) {
		    printf("\"");
		}
	    }
	    printf(">%s</tspan>\n", modv[i]->name);
	    printf("  </text>\n");
	    printf(" </g>\n");
	    (*miNr)++;
	    *y += TABLEELEMHEIGHT;
	    *x -= TABLEELEMHEIGHT;

	    /* revision history of the module. */
	    *x += 2*TABLEELEMHEIGHT;
	    *x += TABLEBOTTOMHEIGHT;
	    smiRevision = smiGetFirstRevision(modv[i]);
	    if (!smiRevision) {
		printf(" <g id=\"MI%i\" transform=\"translate(%.2f,%.2f)\">\n",
								*miNr, *x, *y);
		printf("  <text>1970-01-01</text>\n");
		printf(" </g>\n");
		(*miNr)++;
		*y += TABLEELEMHEIGHT;
	    } else {
		for(; smiRevision;
				smiRevision = smiGetNextRevision(smiRevision)) {
		    printf(" <g id=\"MI%i\" transform=\"translate", *miNr);
		    printf("(%.2f,%.2f)\">\n", *x, *y);
		    printf("  <text");
		    if (!STATIC_OUTPUT && smiRevision->description && strcmp(
		smiRevision->description,
		"[Revision added by libsmi due to a LAST-UPDATED clause.]")) {
			tooltip = (char *)xmalloc(2*
					strlen(smiRevision->description));
			parseTooltip(smiRevision->description, tooltip);
			printf(" onmousemove=\"ShowTooltipMZ(evt,'%s')\"",
								tooltip);
			printf(" onmouseout=\"HideTooltip(evt)\"");
			xfree(tooltip);
		    }
		    printf(">%s</text>\n", getTimeString(smiRevision->date));
		    printf(" </g>\n");
		    *y += TABLEELEMHEIGHT;
		    (*miNr)++;
		}
	    }
	    *x -= 2*TABLEELEMHEIGHT;
	    *x -= TABLEBOTTOMHEIGHT;
	}
    }
    *y += TABLEELEMHEIGHT;
}

static void printNotificationType(int modc, SmiModule **modv,
				  float *x, float *y, int *miNr, int nType[],
				  StringListElem markupList[], int miCount)
{
    int         i, j;
    SmiNode     *smiNode;
    int         statusOrder[5] = {
		    SMI_STATUS_CURRENT,
		    SMI_STATUS_MANDATORY,
		    SMI_STATUS_OPTIONAL,
		    SMI_STATUS_DEPRECATED,
		    SMI_STATUS_OBSOLETE
		};

    printf(" <g id=\"MI%i\" transform=\"translate(%.2f,%.2f)\">\n",
								*miNr, *x, *y);
    printf("  <text>\n");
    if (!STATIC_OUTPUT) {
	printf("   <tspan style=\"text-anchor:middle\"");
	printf(" onclick=\"collapse(evt)\">--</tspan>\n");
    }
    printf("   <tspan x=\"5\">Notifications</tspan>\n");
    printf("  </text>\n");
    printf(" </g>\n");
    (*miNr)++;
    *y += TABLEELEMHEIGHT;
    for (i = 0; i < modc; i++) {
	if (!nType[i])
	    continue;
	smiNode = smiGetModuleIdentityNode(modv[i]);
	if (smiNode) {

	    /* name of the module */
	    *x += TABLEELEMHEIGHT;
	    printf(" <g id=\"MI%i\" transform=\"translate(%.2f,%.2f)\">\n",
								*miNr, *x, *y);
	    printf("  <text>\n");
	    if (!STATIC_OUTPUT) {
		printf("   <tspan style=\"text-anchor:middle\"");
		printf(" onclick=\"collapse(evt)\">--</tspan>\n");
	    }
	    printf("   <tspan x=\"5\">%s</tspan>\n", modv[i]->name);
	    printf("  </text>\n");
	    printf(" </g>\n");
	    (*miNr)++;
	    *y += TABLEELEMHEIGHT;
	    *x -= TABLEELEMHEIGHT;

	    /* name, status and description of the notification */
	    *x += 2*TABLEELEMHEIGHT;
	    *x += TABLEBOTTOMHEIGHT;
	    for (j=0; j<5; j++) {
		for (smiNode = smiGetFirstNode(modv[i],
						    SMI_NODEKIND_NOTIFICATION);
		    smiNode;
		    smiNode = smiGetNextNode(smiNode,
						SMI_NODEKIND_NOTIFICATION)) {
		    if (smiNode->status != statusOrder[j])
			continue;
		    if ((smiNode->status == SMI_STATUS_DEPRECATED
			&& !SHOW_DEPRECATED && !SHOW_DEPR_OBSOLETE)
			|| (smiNode->status == SMI_STATUS_OBSOLETE
			&& !SHOW_DEPR_OBSOLETE))
			continue;
		    if (!STATIC_OUTPUT)
			populateMarkupList(smiNode, miNr, markupList, miCount);
		    printInformationNode(smiNode, x, y, miNr,
							markupList, miCount);
		}
	    }
	    *x -= 2*TABLEELEMHEIGHT;
	    *x -= TABLEBOTTOMHEIGHT;
	}
    }
    *y += TABLEELEMHEIGHT;
}

static void printObjectGroup(int modc, SmiModule **modv,
			     float *x, float *y, int *miNr, int oGroup[],
			     StringListElem markupList[], int miCount)
{
    int         i, j;
    SmiNode     *smiNode;
    int         statusOrder[5] = {
		    SMI_STATUS_CURRENT,
		    SMI_STATUS_MANDATORY,
		    SMI_STATUS_OPTIONAL,
		    SMI_STATUS_DEPRECATED,
		    SMI_STATUS_OBSOLETE
		};

    printf(" <g id=\"MI%i\" transform=\"translate(%.2f,%.2f)\">\n",
								*miNr, *x, *y);
    printf("  <text>\n");
    if (!STATIC_OUTPUT) {
	printf("   <tspan style=\"text-anchor:middle\"");
	printf(" onclick=\"collapse(evt)\">--</tspan>\n");
    }
    printf("   <tspan x=\"5\">Object Groups</tspan>\n");
    printf("  </text>\n");
    printf(" </g>\n");
    (*miNr)++;
    *y += TABLEELEMHEIGHT;
    for (i = 0; i < modc; i++) {
	if (!oGroup[i])
	    continue;
	smiNode = smiGetModuleIdentityNode(modv[i]);
	if (smiNode) {

	    /* name of the module */
	    *x += TABLEELEMHEIGHT;
	    printf(" <g id=\"MI%i\" transform=\"translate(%.2f,%.2f)\">\n",
								*miNr, *x, *y);
	    printf("  <text>\n");
	    if (!STATIC_OUTPUT) {
		printf("   <tspan style=\"text-anchor:middle\"");
		printf(" onclick=\"collapse(evt)\">--</tspan>\n");
	    }
	    printf("   <tspan x=\"5\">%s</tspan>\n", modv[i]->name);
	    printf("  </text>\n");
	    printf(" </g>\n");
	    (*miNr)++;
	    *y += TABLEELEMHEIGHT;
	    *x -= TABLEELEMHEIGHT;

	    /* name, status and description of the group */
	    *x += 2*TABLEELEMHEIGHT;
	    *x += TABLEBOTTOMHEIGHT;
	    for (j=0; j<5; j++) {
		for (smiNode = smiGetFirstNode(modv[i], SMI_NODEKIND_GROUP);
		    smiNode;
		    smiNode = smiGetNextNode(smiNode, SMI_NODEKIND_GROUP)) {
		    if (!isObjectGroup(smiNode))
			continue;
		    if (smiNode->status != statusOrder[j])
			continue;
		    if ((smiNode->status == SMI_STATUS_DEPRECATED
			&& !SHOW_DEPRECATED && !SHOW_DEPR_OBSOLETE)
			|| (smiNode->status == SMI_STATUS_OBSOLETE
			&& !SHOW_DEPR_OBSOLETE))
			continue;
		    if (!STATIC_OUTPUT)
			populateMarkupList(smiNode, miNr, markupList, miCount);
		    printInformationNode(smiNode, x, y, miNr,
							markupList, miCount);
		}
	    }
	    *x -= 2*TABLEELEMHEIGHT;
	    *x -= TABLEBOTTOMHEIGHT;
	}
    }
    *y += TABLEELEMHEIGHT;
}

static void printNotificationGroup(int modc, SmiModule **modv,
				   float *x, float *y, int *miNr, int nGroup[],
				   StringListElem markupList[], int miCount)
{
    int         i, j;
    SmiNode     *smiNode;
    int         statusOrder[5] = {
		    SMI_STATUS_CURRENT,
		    SMI_STATUS_MANDATORY,
		    SMI_STATUS_OPTIONAL,
		    SMI_STATUS_DEPRECATED,
		    SMI_STATUS_OBSOLETE
		};

    printf(" <g id=\"MI%i\" transform=\"translate(%.2f,%.2f)\">\n",
								*miNr, *x, *y);
    printf("  <text>\n");
    if (!STATIC_OUTPUT) {
	printf("   <tspan style=\"text-anchor:middle\"");
	printf(" onclick=\"collapse(evt)\">--</tspan>\n");
    }
    printf("   <tspan x=\"5\">Notification Groups</tspan>\n");
    printf("  </text>\n");
    printf(" </g>\n");
    (*miNr)++;
    *y += TABLEELEMHEIGHT;
    for (i = 0; i < modc; i++) {
	if (!nGroup[i])
	    continue;
	smiNode = smiGetModuleIdentityNode(modv[i]);
	if (smiNode) {

	    /* name of the module */
	    *x += TABLEELEMHEIGHT;
	    printf(" <g id=\"MI%i\" transform=\"translate(%.2f,%.2f)\">\n",
								*miNr, *x, *y);
	    printf("  <text>\n");
	    if (!STATIC_OUTPUT) {
		printf("   <tspan style=\"text-anchor:middle\"");
		printf(" onclick=\"collapse(evt)\">--</tspan>\n");
	    }
	    printf("   <tspan x=\"5\">%s</tspan>\n", modv[i]->name);
	    printf("  </text>\n");
	    printf(" </g>\n");
	    (*miNr)++;
	    *y += TABLEELEMHEIGHT;
	    *x -= TABLEELEMHEIGHT;

	    /* name, status and description of the group */
	    *x += 2*TABLEELEMHEIGHT;
	    *x += TABLEBOTTOMHEIGHT;
	    for (j=0; j<5; j++) {
		for (smiNode = smiGetFirstNode(modv[i], SMI_NODEKIND_GROUP);
		    smiNode;
		    smiNode = smiGetNextNode(smiNode, SMI_NODEKIND_GROUP)) {
		    if (!isNotificationGroup(smiNode))
			continue;
		    if (smiNode->status != statusOrder[j])
			continue;
		    if ((smiNode->status == SMI_STATUS_DEPRECATED
			&& !SHOW_DEPRECATED && !SHOW_DEPR_OBSOLETE)
			|| (smiNode->status == SMI_STATUS_OBSOLETE
			&& !SHOW_DEPR_OBSOLETE))
			continue;
		    if (!STATIC_OUTPUT)
			populateMarkupList(smiNode, miNr, markupList, miCount);
		    printInformationNode(smiNode, x, y, miNr,
							markupList, miCount);
		}
	    }
	    *x -= 2*TABLEELEMHEIGHT;
	    *x -= TABLEBOTTOMHEIGHT;
	}
    }
    *y += TABLEELEMHEIGHT;
}

static void printModuleCompliance(int modc, SmiModule **modv,
				  float *x, float *y, int *miNr, int mCompl[],
				  StringListElem markupList[], int miCount)
{
    int         i, j;
    SmiNode     *smiNode;
    int         statusOrder[5] = {
		    SMI_STATUS_CURRENT,
		    SMI_STATUS_MANDATORY,
		    SMI_STATUS_OPTIONAL,
		    SMI_STATUS_DEPRECATED,
		    SMI_STATUS_OBSOLETE
		};

    printf(" <g id=\"MI%i\" transform=\"translate(%.2f,%.2f)\">\n",
								*miNr, *x, *y);
    printf("  <text>\n");
    if (!STATIC_OUTPUT) {
	printf("   <tspan style=\"text-anchor:middle\"");
	printf(" onclick=\"collapse(evt)\">--</tspan>\n");
    }
    printf("   <tspan x=\"5\">Compliance Statements</tspan>\n");
    printf("  </text>\n");
    printf(" </g>\n");
    (*miNr)++;
    *y += TABLEELEMHEIGHT;
    for (i = 0; i < modc; i++) {
	if (!mCompl[i])
	    continue;
	smiNode = smiGetModuleIdentityNode(modv[i]);
	if (smiNode) {

	    /* name of the module */
	    *x += TABLEELEMHEIGHT;
	    printf(" <g id=\"MI%i\" transform=\"translate(%.2f,%.2f)\">\n",
								*miNr, *x, *y);
	    printf("  <text>\n");
	    if (!STATIC_OUTPUT) {
		printf("   <tspan style=\"text-anchor:middle\"");
		printf(" onclick=\"collapse(evt)\">--</tspan>\n");
	    }
	    printf("   <tspan x=\"5\">%s</tspan>\n", modv[i]->name);
	    printf("  </text>\n");
	    printf(" </g>\n");
	    (*miNr)++;
	    *y += TABLEELEMHEIGHT;
	    *x -= TABLEELEMHEIGHT;

	    /* name, status and description of the compliance */
	    *x += 2*TABLEELEMHEIGHT;
	    for (j=0; j<5; j++) {
		for (smiNode = smiGetFirstNode(modv[i],
						    SMI_NODEKIND_COMPLIANCE);
		    smiNode;
		    smiNode = smiGetNextNode(smiNode,
						    SMI_NODEKIND_COMPLIANCE)) {
		    if (smiNode->status != statusOrder[j])
			continue;
		    if ((smiNode->status == SMI_STATUS_DEPRECATED
			&& !SHOW_DEPRECATED && !SHOW_DEPR_OBSOLETE)
			|| (smiNode->status == SMI_STATUS_OBSOLETE
			&& !SHOW_DEPR_OBSOLETE))
			continue;
		    printComplianceNode(smiNode, modc, modv, x, y, miNr, i,
							markupList, miCount);
		}
	    }
	    *x -= 2*TABLEELEMHEIGHT;
	}
    }
    *y += TABLEELEMHEIGHT;
}

static void printModuleInformation(int modc, SmiModule **modv,
				   float x, float y, float maxHeight,
				   int modId[], int nType[], int oGroup[],
				   int nGroup[], int mCompl[], int miCount)
{
    int i, j, miNr = 0;
    float scale = 1, miHeight;
    int modIdPrint = 0;
    int nTypePrint = 0, oGroupPrint = 0, nGroupPrint = 0, mComplPrint = 0;

    StringListElem *markupList = xcalloc(miCount,sizeof(StringListElem));
    
    /* only print sections containig information */
    for (i = 0; i < modc; i++) {
	modIdPrint |= modId[i];
	nTypePrint |= nType[i];
	oGroupPrint |= oGroup[i];
	nGroupPrint |= nGroup[i];
	mComplPrint |= mCompl[i];
    }

    /* count blank lines */
    i = 0;
    j = 0;
    if (modIdPrint) {
	i++;
    } else {
	j++;
    }
    if (nTypePrint) {
	i++;
    } else {
	j++;
    }
    if (oGroupPrint) {
	i++;
    } else {
	j++;
    }
    if (nGroupPrint) {
	i++;
    } else {
	j++;
    }
    if (mComplPrint) {
	i++;
    } else {
	j++;
    }
    if (i>1)
	i--;

    /* test if we must shrink moduleInformation to fit it into canvas */
    miHeight = ((miCount + i - (2 * j)) * 15 + 10);
    if (miHeight > maxHeight)
	scale *= maxHeight/miHeight;

    printf(" <g transform=\"translate(%.2f,%.2f) scale(%.2f)\">\n",
								x, y, scale);

    /* now use x and y as relative coordinates. */
    x = 0;
    y = 10;

    if (modIdPrint)
	printModuleIdentity(modc, modv, &x, &y, &miNr);
    if (nTypePrint)
	printNotificationType(modc, modv, &x, &y, &miNr, nType, markupList,
								    miCount);
    if (oGroupPrint)
	printObjectGroup(modc, modv, &x, &y, &miNr, oGroup, markupList,
								    miCount);
    if (nGroupPrint)
	printNotificationGroup(modc, modv, &x, &y, &miNr, nGroup, markupList,
								    miCount);
    if (mComplPrint)
	printModuleCompliance(modc, modv, &x, &y, &miNr, mCompl, markupList,
								    miCount);

    printf(" </g>\n");

    if (markupList) xfree(markupList);
}


/* ------------------------------------------------------------------------- */


static float fa(float d, float k)
{
    return (float) (d*d/k);
}

static float fr(float d, float k)
{
    return (float) (k*k/d);
}

static int overlap(GraphNode *vNode, GraphNode *uNode)
{
    if (vNode->dia.x+vNode->dia.w/2>=uNode->dia.x-uNode->dia.w/2 &&
	vNode->dia.x-vNode->dia.w/2<=uNode->dia.x+uNode->dia.w/2 &&
	vNode->dia.y+vNode->dia.h/2>=uNode->dia.y-uNode->dia.h/2 &&
	vNode->dia.y-vNode->dia.h/2<=uNode->dia.y+uNode->dia.h/2) {
	return 1;
    } else {
	return 0;
    }
}

/*
 * test if node and edge intersect
 */
static float intersect(GraphNode *node, GraphEdge *edge)
{
    float a, b, intersect = 0;

    /* handle case in which edge is parallel to y-axis */
    if (edge->endNode->dia.x == edge->startNode->dia.x) {
	if ((node->dia.x-node->dia.w/2 < edge->endNode->dia.x &&
	    node->dia.x+node->dia.w/2 < edge->endNode->dia.x) ||
	    (node->dia.x-node->dia.w/2 > edge->endNode->dia.x &&
	    node->dia.x+node->dia.w/2 > edge->endNode->dia.x))
	    return intersect;
	intersect = node->dia.x - edge->startNode->dia.x;
    } else {
	/* calculate a and b for y=ax+b */
	a = (edge->endNode->dia.y - edge->startNode->dia.y) /
	    (edge->endNode->dia.x - edge->startNode->dia.x);
	b = edge->startNode->dia.y - (a * edge->startNode->dia.x);
	/* test if entire node is above or under edge */
	if ((node->dia.y-node->dia.h/2 - (a * node->dia.x-node->dia.w/2) > b &&
	    node->dia.y+node->dia.h/2 - (a * node->dia.x-node->dia.w/2) > b &&
	    node->dia.y-node->dia.h/2 - (a * node->dia.x+node->dia.w/2) > b &&
	    node->dia.y+node->dia.h/2 - (a * node->dia.x+node->dia.w/2) > b) ||
	    (node->dia.y-node->dia.h/2 - (a * node->dia.x-node->dia.w/2) < b &&
	    node->dia.y+node->dia.h/2 - (a * node->dia.x-node->dia.w/2) < b &&
	    node->dia.y-node->dia.h/2 - (a * node->dia.x+node->dia.w/2) < b &&
	    node->dia.y+node->dia.h/2 - (a * node->dia.x+node->dia.w/2) < b))
	    return intersect;
	intersect = (a * node->dia.x - node->dia.y + b) /
		    (float)(sqrt(a*a+1));
    }
    /* test if node is over upper end of edge or under lower end of edge */
    if (node->dia.y+node->dia.h/2 <
		min(edge->startNode->dia.y,edge->endNode->dia.y) ||
	node->dia.y-node->dia.h/2 >
		max(edge->startNode->dia.y,edge->endNode->dia.y)) {
	intersect = 0;
	return intersect;
    }
    /* node and edge intersect */
    return intersect;
}

/*
 * Implements the springembedder. Look at LNCS 2025, pp. 71-86.
 * and: http://citeseer.ist.psu.edu/fruchterman91graph.html
 * Input: Graph with known width and height of nodes.
 * Output: Coordinates (x,y) for the nodes.
 * Only the nodes and edges with use==1 are considered.
 */
static void layoutComponent(GraphComponent *component,
			int nodeoverlap, int edgeoverlap)
{
    int i;
    float k, xDelta, yDelta, absDelta, absDisp, t, dist;
    GraphNode *vNode, *uNode;
    GraphEdge *eEdge;

    k = 400;
    t = 200;

    for (i=0; i<ITERATIONS; i++) {
	/* calculate repulsive forces */
	for (vNode = component->firstComponentNode; vNode;
					vNode = vNode->nextComponentNode) {
	    vNode->dia.xDisp = 0;
	    vNode->dia.yDisp = 0;
	    for (uNode = component->firstComponentNode; uNode;
					uNode = uNode->nextComponentNode) {
		if (vNode==uNode)
		    continue;
		xDelta = vNode->dia.x - uNode->dia.x;
		yDelta = vNode->dia.y - uNode->dia.y;
		absDelta = (float) (sqrt(xDelta*xDelta + yDelta*yDelta));
		vNode->dia.xDisp += (xDelta/absDelta)*fr(absDelta, k);
		vNode->dia.yDisp += (yDelta/absDelta)*fr(absDelta, k);
		/* add another repulsive force if the nodes overlap */
		if (nodeoverlap && overlap(vNode, uNode)) {
		    vNode->dia.xDisp += 4*(xDelta/absDelta)*fr(1/absDelta, k);
		    vNode->dia.yDisp += 4*(yDelta/absDelta)*fr(1/absDelta, k);
		}
	    }
	}
	for (eEdge = graph->edges; eEdge; eEdge = eEdge->nextPtr) {
	    if (!eEdge->use || eEdge->startNode->component != component)
		continue;
	    /* add another repulsive force if edge and any node overlap */
	    if (edgeoverlap) {
		for (vNode = component->firstComponentNode; vNode;
					vNode = vNode->nextComponentNode) {
		    if (eEdge->startNode == vNode ||
			eEdge->endNode == vNode ||
			overlap(eEdge->startNode, vNode) ||
			overlap(eEdge->endNode, vNode))
			continue;
		    if ((dist = intersect(vNode, eEdge))) {
			if (eEdge->startNode->dia.x == eEdge->endNode->dia.x) {
			    eEdge->startNode->dia.xDisp -=
				8*(dist/fabsf(dist))*fr(1/dist, k);
			    eEdge->endNode->dia.xDisp -=
				8*(dist/fabsf(dist))*fr(1/dist, k);
			    vNode->dia.xDisp +=
				8*(dist/fabsf(dist))*fr(1/dist, k);
			} else {
			    xDelta = -1*(eEdge->endNode->dia.y
					 -eEdge->startNode->dia.y)
				       /(eEdge->endNode->dia.x
					 -eEdge->startNode->dia.x);
			    yDelta = 1;
			    absDelta = (float) (sqrt(xDelta*xDelta
						     + yDelta*yDelta));
			    eEdge->startNode->dia.xDisp +=
				8*(xDelta/absDelta)*fr(1/dist, k);
			    eEdge->startNode->dia.yDisp +=
				8*(yDelta/absDelta)*fr(1/dist, k);
			    eEdge->endNode->dia.xDisp +=
				8*(xDelta/absDelta)*fr(1/dist, k);
			    eEdge->endNode->dia.yDisp +=
				8*(yDelta/absDelta)*fr(1/dist, k);
			    vNode->dia.xDisp -=
				8*(xDelta/absDelta)*fr(1/dist, k);
			    vNode->dia.yDisp -=
				8*(yDelta/absDelta)*fr(1/dist, k);
			}
		    }
		}
	    }
	    /* calculate attractive forces */
	    xDelta = eEdge->startNode->dia.x - eEdge->endNode->dia.x;
	    yDelta = eEdge->startNode->dia.y - eEdge->endNode->dia.y;
	    absDelta = (float) (sqrt(xDelta*xDelta + yDelta*yDelta));
	    eEdge->startNode->dia.xDisp -= (xDelta/absDelta)*fa(absDelta, k);
	    eEdge->startNode->dia.yDisp -= (yDelta/absDelta)*fa(absDelta, k);
	    eEdge->endNode->dia.xDisp += (xDelta/absDelta)*fa(absDelta, k);
	    eEdge->endNode->dia.yDisp += (yDelta/absDelta)*fa(absDelta, k);
	}
	/* limit the maximum displacement to the temperature t */
	for (vNode = component->firstComponentNode; vNode;
					vNode = vNode->nextComponentNode) {
	    absDisp = (float) (sqrt(vNode->dia.xDisp*vNode->dia.xDisp
					+ vNode->dia.yDisp*vNode->dia.yDisp));
	    vNode->dia.x += (vNode->dia.xDisp/absDisp)*min(absDisp, t);
	    vNode->dia.y += (vNode->dia.yDisp/absDisp)*min(absDisp, t);
	}
	/* reduce the temperature as the layout approaches a better configuration */
	t *= 0.9;
    }
}


/* ------------------------------------------------------------------------- */


static void addNodeToComponent(GraphNode *tNode, GraphComponent *tComponent)
{
    GraphEdge *tEdge;

    tNode->component = tComponent;
    for (tEdge = graph->edges; tEdge; tEdge = tEdge->nextPtr) {
	if (!tEdge->use)
	    continue;
	if (tEdge->startNode == tNode && tEdge->endNode->component == NULL) {
	    tEdge->endNode->nextComponentNode = tNode->nextComponentNode;
	    tNode->nextComponentNode = tEdge->endNode;
	    addNodeToComponent(tEdge->endNode, tComponent);
	}
	if (tEdge->endNode == tNode && tEdge->startNode->component == NULL) {
	    tEdge->startNode->nextComponentNode = tNode->nextComponentNode;
	    tNode->nextComponentNode = tEdge->startNode;
	    addNodeToComponent(tEdge->startNode, tComponent);
	}
    }
}


/* split the graph into components */
static void splitGraphIntoComponents()
{
    GraphNode      *tNode;
    GraphComponent *tComponent;
    for (tNode = graph->nodes; tNode; tNode = tNode->nextPtr) {
	if (!tNode->use)
	    continue;
	if (tNode->component == NULL) {
	    tComponent = graphInsertComponent(graph);
	    tComponent->firstComponentNode = tNode;
	    addNodeToComponent(tNode, tComponent);
	}
    }
}


/* layout components (except first) and calculate bounding boxes and offsets */
static void layoutComponents(float *yMin, float *yMax, float *x)
{
    GraphNode      *tNode;
    GraphComponent *tComponent;

    *x=10;
    for (tComponent = graph->components->nextPtr; tComponent;
					    tComponent = tComponent->nextPtr) {
	layoutComponent(tComponent, 0, 0);
	/* FIXME do we need a stage with nodeoverlap and without edgeoverlap? */
	layoutComponent(tComponent, 1, 0);
	layoutComponent(tComponent, 1, 1);

	for (tNode = tComponent->firstComponentNode; tNode;
					tNode = tNode->nextComponentNode) {
	    if (tNode->dia.x - tNode->dia.w/2 < tComponent->xMin)
		tComponent->xMin = tNode->dia.x - tNode->dia.w/2;
	    if (tNode->dia.x + tNode->dia.w/2 > tComponent->xMax)
		tComponent->xMax = tNode->dia.x + tNode->dia.w/2;
	    if (tNode->dia.y - tNode->dia.h/2 < tComponent->yMin)
		tComponent->yMin = tNode->dia.y - tNode->dia.h/2;
	    if (tNode->dia.y + tNode->dia.h/2 > tComponent->yMax)
		tComponent->yMax = tNode->dia.y + tNode->dia.h/2;
	}

	tComponent->xOffset = *x - tComponent->xMin;
	*x += 10 + tComponent->xMax - tComponent->xMin;
	tComponent->yOffset = -0.5*(tComponent->yMin+tComponent->yMax);
	if (tComponent->yMin + tComponent->yOffset < *yMin)
	    *yMin = tComponent->yMin + tComponent->yOffset;
	if (tComponent->yMax + tComponent->yOffset > *yMax)
	    *yMax = tComponent->yMax + tComponent->yOffset;
    }
}


/* Print SVG to stdout */
static void printSVG(int modc, SmiModule **modv, int miCount, int idCount,
		     float xMin, float yMin, float xMax, float yMax,
		     int nodecount, int TCcount,
		     int modId[], int nType[], int oGroup[],
		     int nGroup[], int mCompl[])
{
    GraphComponent *tComponent;
    GraphNode      *tNode;
    GraphEdge      *tEdge;
    int            classNr=0;

    /* output of svg to stdout begins here */
    printSVGHeaderAndTitle(modc, modv, miCount, idCount,
							xMin, yMin, xMax, yMax);

    /* module doesn't contain any objects. */
    if (nodecount == 0) {
	if (TCcount > 0) {
	    printOnlyTCs();
	} else {
	    printNoObjects();
	}
    }

    /* loop through components (except first) to print edges and nodes */
    for (tComponent = graph->components->nextPtr; tComponent;
					    tComponent = tComponent->nextPtr) {
	for (tEdge = graph->edges; tEdge; tEdge = tEdge->nextPtr) {
	    if (!tEdge->use || tEdge->startNode->component != tComponent)
		continue;
	    printSVGConnection(tEdge);
	}
	for (tNode = tComponent->firstComponentNode; tNode;
					tNode = tNode->nextComponentNode) {
	    printSVGObject(tNode, &classNr, modc, modv);
	}
	/* enclose component in its bounding box */
	/*
	printf(" <rect x=\"%.2f\" y=\"%.2f\" width=\"%.2f\" height=\"%.2f\"\n",
		tComponent->xMin + tComponent->xOffset,
		tComponent->yMin + tComponent->yOffset,
		tComponent->xMax - tComponent->xMin,
		tComponent->yMax - tComponent->yMin);
	printf("       fill=\"none\" stroke=\"green\" stroke-width=\"1\"/>\n");
	*/
    }

    /* print single nodes */
    for (tNode = graph->components->firstComponentNode; tNode;
		    			tNode = tNode->nextComponentNode) {
	if (tNode->group == 0) {
	    printSVGObject(tNode, &classNr, modc, modv);
	} else {
	    printSVGGroup(tNode->group, &classNr, modc, modv);
	}
    }

    /* print Module-Information */
    printModuleInformation(modc, modv, xMax-MODULE_INFO_WIDTH, yMin+10,
				yMax-yMin,
				modId, nType, oGroup, nGroup, mCompl, miCount);

    /* output of svg to stdout ends here */
    printSVGClose(xMin, yMin, xMax, yMax);
}


/* prepare nodes and edges for drawing */
static void prepareNodesAndEdges(int *idCount, float *xMax, int *nodecount,
				 int *singleNodes, float *maxHeight)
{
    GraphNode      *tNode, *lastNode = NULL;
    GraphEdge      *tEdge;
    GraphComponent *tComponent;
    float          x=10;
    int            group;

    /* find edges which are supposed to be drawn */
    for (tEdge = graph->edges; tEdge; tEdge = tEdge->nextPtr) {
	if (tEdge->connection != GRAPH_CON_UNKNOWN
	    && tEdge->startNode->smiNode->nodekind != SMI_NODEKIND_SCALAR
	    && tEdge->endNode->smiNode->nodekind != SMI_NODEKIND_SCALAR
	    && tEdge->startNode != tEdge->endNode) {
	    tEdge->use = 1;
	    tEdge->startNode->degree++;
	    tEdge->endNode->degree++;
	}
    }

    tComponent = graphInsertComponent(graph);

    /* prepare nodes which are supposed to be drawn */
    for (tNode = graph->nodes; tNode; tNode = tNode->nextPtr) {
	tNode = calcNodeSize(tNode, idCount);
	if (tNode->smiNode->nodekind != SMI_NODEKIND_SCALAR) {
	    (*nodecount)++;
	    if (tNode->degree == 0) {
		/* single nodes are members of the first component. */
		if (tComponent->firstComponentNode == NULL) {
		    tComponent->firstComponentNode = tNode;
		} else {
		    lastNode->nextComponentNode = tNode;
		}
		lastNode = tNode;
		tNode->component = tComponent;
		tNode->dia.x = x + tNode->dia.w/2;
		x += 10 + tNode->dia.w;
		tNode->dia.y = 0;
		if (tNode->dia.h > *maxHeight)
		    *maxHeight = tNode->dia.h;
	    }
	}
    }
    for (group = 1; group <= algGetNumberOfGroups(); group++) {
	tNode = calcGroupSize(group, idCount);
	(*nodecount)++;
	/* groupnodes are members of the first component. */
	if (tComponent->firstComponentNode == NULL) {
	    tComponent->firstComponentNode = tNode;
	} else {
	    lastNode->nextComponentNode = tNode;
	}
	lastNode = tNode;
	tNode->component = tComponent;
	tNode->dia.x = x + tNode->dia.w/2;
	x += 10 + tNode->dia.w;
	tNode->dia.y = 0;
	if (tNode->dia.h > *maxHeight)
	    *maxHeight = tNode->dia.h;
    }
    *xMax = x;
    if (tComponent->firstComponentNode == NULL)
	*singleNodes = 0;
}



/*
 * generate SVG diagram and print it to stdout:
 * - identify and prepare nodes and egdes
 * - split graph into its components
 * - layout components
 * - prepare module information
 * - print header
 * - print components
 * - print module information
 * - print footer
 */
static void generateSVG(int modc, SmiModule **modv)
{
    int            nodecount=0, singleNodes=1, miCount=0;
    int            i, idCount=0, TCcount=0, miPrint=0;
    float          x=10, xMin=0, yMin=0, xMax=0, yMax=0, maxHeight=0;

    int            *modId = xcalloc(modc,sizeof(int));
    int            *nType = xcalloc(modc,sizeof(int));
    int            *oGroup = xcalloc(modc,sizeof(int));
    int            *nGroup = xcalloc(modc,sizeof(int));
    int            *mCompl = xcalloc(modc,sizeof(int));

    /* prepare nodes and edges for drawing */
    prepareNodesAndEdges(&idCount, &xMax, &nodecount, &singleNodes, &maxHeight);

    /* split the graph into components */
    splitGraphIntoComponents();

    /* layout components (except first) and calculate bounding boxes and offsets */
    layoutComponents(&yMin, &yMax, &x);

    if (graph->components->nextPtr)
	yMin -= 10;
    yMax += 10;
    if (x > xMax)
	xMax = x;

    /* adjust values for the first component (component of single nodes) */
    graph->components->yOffset = yMax + maxHeight/2;
    if (singleNodes)
	yMax += maxHeight + 10;

    /* module doesn't contain any objects. */
    if (nodecount == 0) {
	TCcount = countTCs(modc, modv);
	if (TCcount > 0) {
	    xMax += 160;
	} else {
	    xMax += 130;
	}
	yMax += 40;
    }

    /* count entries in the ModuleInformation-Section */
    prepareModInfo(modc, modv, &miCount, modId, nType, oGroup, nGroup, mCompl);
    idCount += miCount;

    /* enlarge canvas for ModuleInformation if it is supposed to be printed */
    for (i = 0; i < modc; i++) {
	miPrint |= modId[i];
	miPrint |= nType[i];
	miPrint |= oGroup[i];
	miPrint |= nGroup[i];
	miPrint |= mCompl[i];
    }
    if (miPrint) {
	xMax += MODULE_INFO_WIDTH;
    }

    printSVG(modc, modv, miCount, idCount, xMin, yMin, xMax, yMax, nodecount,
	     TCcount, modId, nType, oGroup, nGroup, mCompl);
    
    xfree(mCompl);
    xfree(nGroup);
    xfree(oGroup);
    xfree(nType);
    xfree(modId);
}



static void buildLink(int modc, SmiModule **modv)
{
    size_t length;
    const char *url = URL;
    /* note: first string, so no &amp; required */
    const char *widthstr = "width=";
    const char *heightstr = "&amp;height=";
    const char *deprstr = "&amp;deprobs=deprecated";
    const char *deprobsstr = "&amp;deprobs=obsolete";
    char width[15];
    char height[15];

    length = strlen(url);
    sprintf(width, "%i", CANVASWIDTH);
    sprintf(height, "%i", CANVASHEIGHT);
    length += strlen(widthstr) + strlen(width);
    length += strlen(heightstr) + strlen(height);
    if (SHOW_DEPRECATED) {
	length += strlen(deprstr);
    }
    if (SHOW_DEPR_OBSOLETE) {
	length += strlen(deprobsstr);
    }
    link = xmalloc(length + 1);
    strcpy(link, url);
    strcat(link, widthstr);
    strcat(link, width);
    strcat(link, heightstr);
    strcat(link, height);
    if (SHOW_DEPRECATED) {
	strcat(link, deprstr);
    }
    if (SHOW_DEPR_OBSOLETE) {
	strcat(link, deprobsstr);
    }
}


static void calcConceptualModel()
{
    algLinkTables();
    algCheckLinksByName();
    algConnectLonelyNodes();
    algCheckForDependency();
    algCheckForPointerRels();
}


/* ------------------------------------------------------------------------- */



static void dumpSvg(int modc, SmiModule **modv, int flags, char *output)
{
    int       i;

    buildLink(modc, modv);

    if (flags & SMIDUMP_FLAG_UNITE) {
	if (! graph) {
	    graph = xmalloc(sizeof(Graph));
	    graph->nodes = NULL;
	    graph->edges = NULL;
	    graph->components = NULL;
	}
	
	for (i = 0; i < modc; i++) {
	    algCreateNodes(modv[i]);
	}
	
	calcConceptualModel();
	
	generateSVG(modc, modv);
	
	graphExit(graph);
	graph = NULL;
    } else {
	for (i = 0; i < modc; i++) {
	    if (! graph) {
		graph = xmalloc(sizeof(Graph));
		graph->nodes = NULL;
		graph->edges = NULL;
		graph->components = NULL;
	    }
	    
	    algCreateNodes(modv[i]);
	
	    calcConceptualModel();
	    
	    generateSVG(1, &(modv[i]));
	
	    graphExit(graph);
	    graph = NULL;
	}
    }

    if (fflush(stdout) || ferror(stdout)) {
	perror("smidump: write error");
	exit(1);
    }

    xfree(link);
}



void initSvg()
{
    static SmidumpDriverOption opt[] = {
	{ "width", OPT_INT, &CANVASWIDTH, 0,
	  "width of the svg output (default=1100)"},
	{ "height", OPT_INT, &CANVASHEIGHT, 0,
	  "height of the svg output (default=700)"},
	{ "show-deprecated", OPT_FLAG, &SHOW_DEPRECATED, 0,
	  "show deprecated items"},
	{ "show-depr-obsolete", OPT_FLAG, &SHOW_DEPR_OBSOLETE, 0,
	  "show deprecated and obsolete items"},
	{ "static-output", OPT_FLAG, &STATIC_OUTPUT, 0,
	  "disable all interactivity (e.g. for printing)"},
        { 0, OPT_END, 0, 0 }
    };

    static SmidumpDriver driver = {
	"svg",
	dumpSvg,
	0,
	SMIDUMP_DRIVER_CANT_OUTPUT,
	"SVG diagram",
	opt,
	NULL
    };

    smidumpRegisterDriver(&driver);
}
