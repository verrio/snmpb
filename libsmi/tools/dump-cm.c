/*
 * dump-cm.c --
 *
 *      Operations to dump conceptual models for MIB modules.
 *
 * Copyright (c) 2000 A. Mueller, Technical University of Braunschweig.
 *
 * See the file "COPYING" for information on usage and redistribution
 * of this file, and for a DISCLAIMER OF ALL WARRANTIES.
 *
 * @(#) $Id: dump-cm.c 2718 2005-08-24 06:59:15Z sperner $
 */


/* 
 * -- TO DO --
 *
 * Berechnungen der UML Diagramme debuggen
 *
 */





#include <config.h>

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>
#ifdef HAVE_WIN_H
#include "win.h"
#endif

#include "smi.h"
#include "smidump.h"
#include "rea.h"



/*
 * Definitions used by the dia output driver (node layout).
 */

static const float HEADFONTSIZETABLE   = (float)0.51;
static const float HEADSPACESIZETABLE  = (float)0.6;
static const float ATTRFONTSIZE        = (float)0.48;
static const float ATTRSPACESIZE       = (float)2.4;
static const float RECTCORRECTION      = (float)0.85;
static const float EDGEYSPACING        = (float)2.0;
static const float TABLEHEIGHT         = (float)2.6;   /* headline of the table */
static const float TABLEELEMHEIGHT     = (float)0.675; /* height of one attribute */

/*
 * global dia graph layout
 */
static const float YSPACING            = (float)3.0;  /* y space between nodes */
static const float XSPACING            = (float)4.0;  /* x space between nodes */ 
static const float NEWLINEDISTANCE     = (float)40.0; /* length of one line */
static const float XOFFSET             = (float)2.0;  /* left upper start of graph */
static const float YOFFSET             = (float)5.0;  /* left upper start of graph */

/*
 * position of the dia info note 
 */
static const float XNOTE               = (float)1.0;  /* left upper corner of note */
static const float YNOTE               = (float)1.0;  /* left upper corner of note */

/*
 * Stereotype Name
 */
static const char* STEREOTYPE          = "smi mib class";

/*
 * Property String for index objects
 */
static const char* INDEXPROPERTY       = " {index}";


/* -------------- main functions ------------------------------------------- */


/*
 * Creates the graph nodes of the given module
 */
static void algCreateNodes(SmiModule *module)
{
    SmiNode *node;
    
    /* get tables and scalars from the MIB module */
    for (node = smiGetFirstNode(module, SMI_NODEKIND_TABLE);
	 node;
	 node = smiGetNextNode(node, SMI_NODEKIND_TABLE)) {
	if (node->status != SMI_STATUS_OBSOLETE) {
	    if (!SUPPRESS_DEPRECATED || node->status != SMI_STATUS_DEPRECATED)
		graphInsertNode(graph, node);
	}
    }
    for (node = smiGetFirstNode(module, SMI_NODEKIND_SCALAR);
	 node;
	 node = smiGetNextNode(node, SMI_NODEKIND_SCALAR)) {
	if (node->status != SMI_STATUS_OBSOLETE) {
	    if (!SUPPRESS_DEPRECATED || node->status != SMI_STATUS_DEPRECATED)
		graphInsertNode(graph, node);
	}
    }
}



/* ------ XML primitives ------                                              */




static void diaPrintXMLHeader()
{
    printf("<?xml version=\"1.0\"?>\n");
    printf("<diagram xmlns:dia=\"http://www.lysator.liu.se/~alla/dia/\">\n");
    printf("  <diagramdata>\n");
    printf("    <attribute name=\"background\">\n");
    printf("      <color val=\"#ffffff\"/>\n");
    printf("    </attribute>\n");
    printf("    <attribute name=\"paper\">\n");
    printf("      <composite type=\"paper\">\n");
    printf("        <attribute name=\"name\">\n");
    printf("          <string>#A4#</string>\n");
    printf("        </attribute>\n");
    printf("        <attribute name=\"tmargin\">\n");
    printf("         <real val=\"2.82\"/>\n");
    printf("       </attribute>\n");
    printf("       <attribute name=\"bmargin\">\n");
    printf("         <real val=\"2.82\"/>\n");
    printf("        </attribute>\n");
    printf("       <attribute name=\"lmargin\">\n");
    printf("         <real val=\"2.82\"/>\n");
    printf("       </attribute>\n");
    printf("       <attribute name=\"rmargin\">\n");
    printf("         <real val=\"2.82\"/>\n");
    printf("       </attribute>\n");
    printf("       <attribute name=\"is_portrait\">\n");
    printf("         <boolean val=\"true\"/>\n");
    printf("       </attribute>\n");
    printf("      <attribute name=\"scaling\">\n");
    printf("         <real val=\"1\"/>\n");
    printf("      </attribute>\n");
    printf("      <attribute name=\"fitto\">\n");
    printf("        <boolean val=\"false\"/>\n");
    printf("      </attribute>\n");
    printf("    </composite>\n");
    printf("   </attribute>\n");
    printf("  </diagramdata>\n");
    printf("  <layer name=\"Background\" visible=\"true\">\n");   
}

static void diaPrintXMLClose()
{
    printf("  </layer>\n");
    printf("</diagram>\n");
}

/*
 * prints the type of a given node
 */  
static void diaPrintXMLType(SmiNode *smiNode, int index)
{
    printf("          <attribute name=\"type\">\n");
    if (index) {
	printf("            <string>#%s%s#</string>\n",
	       algGetTypeName(smiNode), INDEXPROPERTY);
    } else {
	printf("            <string>#%s#</string>\n", algGetTypeName(smiNode));
    }
    printf("          </attribute>\n");    
}

/*
 * index = 0 -> no index element
 * index = 1 -> index element -> printed with "+"
 */
static void diaPrintXMLAttribute(SmiNode *node, int index)
{
    printf("        <composite type=\"umlattribute\">\n");
    printf("          <attribute name=\"name\">\n");
    printf("            <string>#%s#</string>\n", node->name);
    printf("          </attribute>\n");
    
    diaPrintXMLType(node,index);
    
    printf("          <attribute name=\"value\">\n");
    printf("            <string/>\n");
    printf("          </attribute>\n");
    printf("          <attribute name=\"visibility\">\n");

    if (node->access == SMI_ACCESS_NOT_ACCESSIBLE) {
	printf("            <enum val=\"1\"/>\n");
    } else {
	printf("            <enum val=\"0\"/>\n");
    }
    
    printf("          </attribute>\n");
    printf("          <attribute name=\"abstract\">\n");
    printf("            <boolean val=\"false\"/>\n");
    printf("          </attribute>\n");
    printf("          <attribute name=\"class_scope\">\n");
    
    if (node->nodekind == SMI_NODEKIND_SCALAR) {
	printf("            <boolean val=\"true\"/>\n");
    } else {
	printf("            <boolean val=\"false\"/>\n");	
    }

    printf("          </attribute>\n");
    printf("        </composite>\n");	   
}

/*
 * prints the related scalars for a given table
 */
static void diaPrintXMLRelatedScalars(GraphNode *node)
{
    GraphEdge *tEdge;
    
    for (tEdge = graphGetFirstEdgeByNode(graph, node);
	 tEdge;
	 tEdge = graphGetNextEdgeByNode(graph, tEdge, node)) {
	if (tEdge->startNode == node  &&
	    tEdge->endNode->smiNode->nodekind == SMI_NODEKIND_SCALAR) {
	    tEdge->dia.flags |= DIA_PRINT_FLAG;
	    tEdge->endNode->dia.flags |= DIA_PRINT_FLAG;

	    diaPrintXMLAttribute(tEdge->endNode->smiNode,0);
	}
    }
}

/*
 * prints all columns objects of the given node
 */
static void diaPrintXMLAllColumns(GraphNode *node)
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
	    diaPrintXMLAttribute(smiNode, 0);
    }
}

/*
 * adds the index to an augmenting table (row-element)
 */
static void diaPrintAugmentIndex(GraphNode *tNode)
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
		    diaPrintXMLAttribute(smiGetElementNode(smiElement),1);
		}
	    }
	}
    }
}

static void diaPrintXMLObject(GraphNode *node, float x, float y)
{
    SmiElement *smiElement;
    
    if (!node) return;
    if (node->dia.flags & DIA_PRINT_FLAG) return;

    node->dia.x = x;
    node->dia.y = y;
    node->dia.flags |= DIA_PRINT_FLAG; /* object is now printed */
    
    printf("    <object type=\"UML - Class\" version=\"0\" id=\"%s\">\n",
	   node->smiNode->name);
    printf("      <attribute name=\"obj_pos\">\n");
    printf("       <point val=\"%.2f,%.2f\"/>\n",x,y);
    printf("      </attribute>\n");
    printf("     <attribute name=\"obj_bb\">\n");
    printf("       <rectangle val=\"0.0,0.0;0.0,0.0\"/>\n");
    printf("     </attribute>\n");
    printf("     <attribute name=\"elem_corner\">\n");
    printf("       <point val=\"%.2f,%.2f\"/>\n",x,y);
    printf("     </attribute>\n");
    printf("     <attribute name=\"elem_width\">\n");
    printf("       <real val=\"%.2f\"/>\n",node->dia.w);
    printf("     </attribute>\n");
    printf("     <attribute name=\"elem_height\">\n");
    printf("       <real val=\"%.2f\"/>\n",node->dia.h);
    printf("     </attribute>\n");
    printf("     <attribute name=\"name\">\n");
    printf("       <string>#%s#</string>\n",
	   smiGetFirstChildNode(node->smiNode)->name);
    printf("     </attribute>\n");
    printf("     <attribute name=\"stereotype\">\n");
    printf("        <string>#%s#</string>\n", STEREOTYPE);
    printf("     </attribute>\n");
    printf("     <attribute name=\"abstract\">\n");
    printf("       <boolean val=\"false\"/>\n");
    printf("     </attribute>\n");
    printf("     <attribute name=\"suppress_attributes\">\n");
    printf("        <boolean val=\"false\"/>\n");    
    printf("      </attribute>\n");   
    printf("      <attribute name=\"suppress_operations\">\n");
    printf("        <boolean val=\"true\"/>\n");
    printf("      </attribute>\n");
    printf("     <attribute name=\"visible_attributes\">\n");
    printf("       <boolean val=\"true\"/>\n");
    printf("     </attribute>\n");
    printf("     <attribute name=\"visible_operations\">\n");
    printf("        <boolean val=\"false\"/>\n");
    printf("      </attribute>\n");

    printf("     <attribute name=\"attributes\">\n");

    if (node->smiNode->nodekind == SMI_NODEKIND_TABLE) {

	diaPrintXMLRelatedScalars(node);

	diaPrintAugmentIndex(node);
	
	for (smiElement = smiGetFirstElement(
	    smiGetFirstChildNode(node->smiNode));
	     smiElement;
	     smiElement = smiGetNextElement(smiElement)) {
	    diaPrintXMLAttribute(smiGetElementNode(smiElement),1);
	}

	if (PRINT_DETAILED_ATTR) {
	    diaPrintXMLAllColumns(node);
	}
    }
    
    printf("      </attribute>\n");
    
    printf("     <attribute name=\"operations\"/>\n");
    printf("    <attribute name=\"template\">\n");
    printf("      <boolean val=\"false\"/>\n");
    printf("    </attribute>\n");
    printf("     <attribute name=\"templates\"/>\n");
    printf("   </object>\n");
}

/*
 * prints a group of scalars denoted by group
 */
static void diaPrintXMLGroup(int group, float x, float y)
{
    GraphNode *tNode;

    for (tNode = graph->nodes; tNode; tNode = tNode->nextPtr) {
	if (tNode->group == group) break;
    }

    if (!tNode) return;
    
    printf("    <object type=\"UML - Class\" version=\"0\" id=\"%s\">\n",
	   smiGetParentNode(tNode->smiNode)->name);
    printf("      <attribute name=\"obj_pos\">\n");
    printf("       <point val=\"%.2f,%.2f\"/>\n",x,y);
    printf("      </attribute>\n");
    printf("     <attribute name=\"obj_bb\">\n");
    printf("       <rectangle val=\"0.0,0.0;0.0,0.0\"/>\n");
    printf("     </attribute>\n");
    printf("     <attribute name=\"elem_corner\">\n");
    printf("       <point val=\"%.2f,%.2f\"/>\n",x,y);
    printf("     </attribute>\n");
    printf("     <attribute name=\"elem_width\">\n");
    printf("       <real val=\"%.2f\"/>\n",0.0);
    printf("     </attribute>\n");
    printf("     <attribute name=\"elem_height\">\n");
    printf("       <real val=\"%.2f\"/>\n",0.0);
    printf("     </attribute>\n");
    printf("     <attribute name=\"name\">\n");
    printf("       <string>#%s#</string>\n",
	   smiGetParentNode(tNode->smiNode)->name);
    printf("     </attribute>\n");
    printf("     <attribute name=\"stereotype\">\n");
    printf("         <string>#%s#</string>\n", STEREOTYPE);
    printf("     </attribute>\n");
    printf("     <attribute name=\"abstract\">\n");
    printf("       <boolean val=\"false\"/>\n");
    printf("     </attribute>\n");
    printf("     <attribute name=\"suppress_attributes\">\n");
    printf("        <boolean val=\"false\"/>\n");    
    printf("      </attribute>\n");   
    printf("      <attribute name=\"suppress_operations\">\n");
    printf("        <boolean val=\"true\"/>\n");
    printf("      </attribute>\n");
    printf("     <attribute name=\"visible_attributes\">\n");
    printf("       <boolean val=\"true\"/>\n");
    printf("     </attribute>\n");
    printf("     <attribute name=\"visible_operations\">\n");
    printf("        <boolean val=\"false\"/>\n");
    printf("      </attribute>\n");

    printf("     <attribute name=\"attributes\">\n");

    for (tNode = graph->nodes; tNode; tNode = tNode->nextPtr) {
	if (tNode->group == group) {
	    diaPrintXMLAttribute(tNode->smiNode,0);
	}
    }
    
    printf("      </attribute>\n");
    
    printf("     <attribute name=\"operations\"/>\n");
    printf("    <attribute name=\"template\">\n");
    printf("      <boolean val=\"false\"/>\n");
    printf("    </attribute>\n");
    printf("     <attribute name=\"templates\"/>\n");
    printf("   </object>\n");
}

static float getRectSX(GraphNode *tNode)
{
    return (float) (tNode->dia.w / 2.0 + tNode->dia.x - RECTCORRECTION);
}

static float getRectEX(GraphNode *tNode)
{
    return (float) (tNode->dia.w / 2.0 + tNode->dia.x + RECTCORRECTION);
}

static float getRectSY(GraphNode *tNode)
{
    return (float) (tNode->dia.y - 2.0 - RECTCORRECTION);
}

static float getRectEY(GraphNode *tNode)
{
    return (float) (tNode->dia.y - 2.0 + RECTCORRECTION);
}


static int getConPoint(GraphNode *snode, GraphNode *enode)
{
    float x1,y1,x2,y2;
    int   con = 1;
    
    x1 = snode->dia.x;
    y1 = snode->dia.y;
    x2 = enode->dia.x;
    y2 = enode->dia.y;    

    if (x1 == x2 && y1 < y2) con = 6;
    if (x1 == x2 && y1 > y1) con = 1;
    if (x1 > x2 && y1 == y2) con = 3;
    if (x1 < x2 && y1 == y2) con = 4;
    if (x1 > x2 && y1 > y2)  con = 0;
    if (x1 > x2 && y1 < y2)  con = 5;
    if (x1 < x2 && y1 > y2)  con = 2;
    if (x1 < x2 && y1 < y2)  con = 7;

    return con;
}

static float getObjX(GraphNode *node, int con)
{
    switch (con) {
    case 0 :
	return node->dia.x;
	break;
    case 1 :
	return (float) (node->dia.w / 2.0 + node->dia.x);
	break;
    case 2 :
	return node->dia.x + node->dia.w;
	break;
    case 3 :
	return node->dia.x;
	break;	
    case 4 :
	return node->dia.x + node->dia.w;
	break;
    case 5 :
	return node->dia.x;
	break;	
    case 6 :
	return (float) (node->dia.w / 2.0 + node->dia.x);
	break;
    case 7 :
	return node->dia.x + node->dia.w;
	break;
    }
    return 0.0;
}

static float getObjY(GraphNode *node, int con)
{
    switch (con) {
    case 0 :
	return node->dia.y;
	break;
    case 1 :
	return node->dia.y;
	break;
    case 2 :
	return node->dia.y;
	break;
    case 3 :
	return (float) (node->dia.y + TABLEHEIGHT / 2.0);
	break;	
    case 4 :
	return (float) (node->dia.y + TABLEHEIGHT / 2.0);
	break;
    case 5 :
	return node->dia.y + node->dia.h;
	break;	
    case 6 :
	return node->dia.y + node->dia.h;
	break;
    case 7 :
	return node->dia.y + node->dia.h;
	break;
    }
    return 0.0;
}

static float getObjYRel(GraphEdge *edge, int con)
{
    GraphNode *node, *node2;
    float     dist;
    
    node = edge->startNode;
    node2 = edge->endNode;
    if (node->dia.y < node2->dia.y) {
	dist = ABS(((node->dia.y + node->dia.h) - node2->dia.y ) / 2.0);
    } else {
	dist = ABS((node->dia.y - (node2->dia.y + node2->dia.h)) / 2.0);
    }
    
    switch (con) {
    case 0 :
	return node->dia.y - dist;
	break;
    case 1 :
	return node->dia.y - dist;
	break;
    case 2 :
	return node->dia.y - dist;
	break;
    case 3 :
	return (float) (node->dia.y + TABLEHEIGHT / 2.0);
	break;	
    case 4 :
	return (float) (node->dia.y + TABLEHEIGHT / 2.0);
	break;
    case 5 :
	return node->dia.y + node->dia.h + dist;
	break;	
    case 6 :
	return node->dia.y + node->dia.h + dist;
	break;
    case 7 :
	return node->dia.y + node->dia.h + dist;
	break;
    }
    return 0.0;
}

/*
 * diaPrintXMLCoordinates
 *
 * prints and calculates the coordinates of a given edge
 */
static void diaPrintXMLCoordinates(GraphEdge *tEdge)
{
    int scon, econ;

    scon = getConPoint(tEdge->startNode, tEdge->endNode);
    econ = getConPoint(tEdge->endNode, tEdge->startNode);
    
    printf("      <attribute name=\"obj_pos\">\n");
    printf("        <point val=\"%.2f,%.2f\"/>\n"
	   ,getObjX(tEdge->startNode,scon)
	   ,getObjY(tEdge->startNode,scon));	   
    printf("     </attribute>\n");
    printf("      <attribute name=\"obj_bb\">\n");
    printf("       <rectangle val=\"%.2f,%.2f;%.2f,%.2f\"/>\n"
	   ,getRectSX(tEdge->startNode)
	   ,getRectSY(tEdge->startNode)
	   ,getRectEX(tEdge->startNode)
	   ,getRectEY(tEdge->startNode));
    printf("     </attribute>\n");
    printf("     <attribute name=\"orth_points\">\n");
    printf("       <point val=\"%.2f,%.2f\"/>\n"
	   ,getObjX(tEdge->startNode,scon)
	   ,getObjY(tEdge->startNode,scon));	
    printf("       <point val=\"%.2f,%.2f\"/>\n"
	   ,getObjX(tEdge->startNode,scon)
	   ,getObjYRel(tEdge,scon));
    printf("       <point val=\"%.2f,%.2f\"/>\n"
	   ,getObjX(tEdge->endNode,econ)
	   ,getObjYRel(tEdge,scon));
    printf("       <point val=\"%.2f,%.2f\"/>\n"
	   ,getObjX(tEdge->endNode,econ)
	   ,getObjY(tEdge->endNode,econ));	
    printf("     </attribute>\n");
}

/*
 * diaPrintXMLConPoints
 *
 * prints the connection points of an edge
 */
static void diaPrintXMLConPoints(GraphEdge *tEdge)
{
    int scon, econ;

    scon = getConPoint(tEdge->startNode, tEdge->endNode);
    econ = getConPoint(tEdge->endNode, tEdge->startNode);
    
    printf("    <connections>\n");
    printf("      <connection handle=\"0\" to=\"%s\" connection=\"%d\"/>\n",
	   tEdge->startNode->smiNode->name,scon);
    printf("      <connection handle=\"1\" to=\"%s\" connection=\"%d\"/>\n",
	   tEdge->endNode->smiNode->name, econ);
    printf("    </connections>\n");    
}

static void diaPrintXMLDependency(GraphEdge *tEdge)
{
    if (tEdge->dia.flags & DIA_PRINT_FLAG) return;
    tEdge->dia.flags |= DIA_PRINT_FLAG;

    printf("    <object type=\"UML - Dependency\" "
	   "version=\"0\" id=\"Depend:%s:%s\">\n",
	   tEdge->startNode->smiNode->name,
	   tEdge->endNode->smiNode->name);    

    diaPrintXMLCoordinates(tEdge);
    
    printf("     <attribute name=\"orth_orient\">\n");
    printf("       <enum val=\"1\"/>\n");
    printf("       <enum val=\"0\"/>\n");
    printf("       <enum val=\"1\"/>\n");
    printf("     </attribute>\n");
    printf("    <attribute name=\"draw_arrow\">\n");
    printf("       <boolean val=\"true\"/>\n");
    printf("      </attribute>\n");
    printf("     <attribute name=\"name\">\n");
    printf("        <string/>\n");
    printf("   </attribute>\n");
    printf("     <attribute name=\"stereotype\">\n");
    printf("      <string/>\n");
    printf("    </attribute>\n");

    diaPrintXMLConPoints(tEdge);
    
    printf("    </object>\n");
}

/*
 * Aggregation is a special case of the association.
 * If aggregate = 1 it is an aggregation if 0 it is an association.
 */
static void diaPrintXMLAssociation(GraphEdge *tEdge, int aggregate)
{
    if (tEdge->dia.flags & DIA_PRINT_FLAG) return;
    tEdge->dia.flags |= DIA_PRINT_FLAG;
    if (aggregate > 1) aggregate = 1;
    if (aggregate < 0) aggregate = 0;
    
    printf("    <object type=\"UML - Association\" "
	   "version=\"0\" id=\"Assoc:%s:%s\">\n",
	   tEdge->startNode->smiNode->name,
	   tEdge->endNode->smiNode->name);
    
    diaPrintXMLCoordinates(tEdge);
    
    printf("      <attribute name=\"orth_orient\">\n");
    printf("        <enum val=\"1\"/>\n");
    printf("        <enum val=\"0\"/>\n");
    printf("        <enum val=\"1\"/>\n");   
    printf("      </attribute>\n");
    printf("      <attribute name=\"name\">\n");

    switch(tEdge->indexkind) {
    case SMI_INDEX_UNKNOWN :
	switch (tEdge->enhancedindex) {
	case GRAPH_ENHINDEX_UNKNOWN :
	    break;
	case GRAPH_ENHINDEX_TYPES :
	    printf("       <string>#%s#</string>\n","");
	    break;
	case GRAPH_ENHINDEX_NAMES :
	    printf("       <string>#%s#</string>\n","");
	    break;
	case GRAPH_ENHINDEX_NOTIFICATION :
	    printf("       <string>#%s#</string>\n","");
	    break;
	case GRAPH_ENHINDEX_INDEX :
	    /* should not occur - is handled below */
	    printf("       <string>#%s#</string>\n","");
	    break;
	case GRAPH_ENHINDEX_REROUTE :
	    printf("       <string>#%s#</string>\n","");
	    break;
	case GRAPH_ENHINDEX_POINTER :
	    printf("       <string>#%s#</string>\n","");
	    break;	    
	}
	break;
    case SMI_INDEX_INDEX :
	printf("       <string>#%s#</string>\n","");
	break;
    case SMI_INDEX_AUGMENT :
	printf("       <string>#%s#</string>\n","augments");
	break;
    case SMI_INDEX_SPARSE :
	printf("       <string>#%s#</string>\n","sparses");
	break;
    case SMI_INDEX_REORDER :
	printf("       <string>#%s#</string>\n","reorders");
	break;
    case SMI_INDEX_EXPAND :
	printf("       <string>#%s#</string>\n","expands");
	break;
    }
    
    printf("      </attribute>\n");
    printf("      <attribute name=\"direction\">\n");
    printf("        <enum val=\"0\"/>\n");
    printf("      </attribute>\n");
    printf("      <attribute name=\"ends\">\n");
    printf("        <composite>\n");
    printf("          <attribute name=\"role\">\n");
    printf("            <string/>\n");
    printf("          </attribute>\n");
    printf("          <attribute name=\"multiplicity\">\n");
    
    switch (tEdge->cardinality) {
    case GRAPH_CARD_UNKNOWN :
	printf("       <string># #</string>\n");
	break;
    case GRAPH_CARD_ONE_TO_ONE :
	printf("       <string>#1#</string>\n");
	break;
    case GRAPH_CARD_ONE_TO_MANY :
	printf("       <string>#1#</string>\n");
	break;
    case GRAPH_CARD_ZERO_TO_ONE :
	printf("       <string>#0#</string>\n");
	break;
    case GRAPH_CARD_ZERO_TO_MANY :
	printf("       <string>#0#</string>\n");
	break;
    case GRAPH_CARD_ONE_TO_ZERO_OR_ONE :
	printf("       <string>#1#</string>\n");
	break;		
    }
   
    printf("          </attribute>\n");
    printf("          <attribute name=\"arrow\">\n");
    printf("            <boolean val=\"false\"/>\n");
    printf("          </attribute>\n");
    printf("          <attribute name=\"aggregate\">\n");
    printf("            <enum val=\"0\"/>\n");
    printf("          </attribute>\n");
    printf("        </composite>\n");
    printf("        <composite>\n");
    printf("          <attribute name=\"role\">\n");
    printf("            <string/>\n");
    printf("          </attribute>\n");

    printf("          <attribute name=\"multiplicity\">\n");

    switch (tEdge->cardinality) {
    case GRAPH_CARD_UNKNOWN :
	printf("       <string># #</string>\n");
	break;
    case GRAPH_CARD_ONE_TO_ONE :
	printf("       <string>#1#</string>\n");
	break;
    case GRAPH_CARD_ONE_TO_MANY :
	printf("       <string>#*#</string>\n");
	break;
    case GRAPH_CARD_ZERO_TO_ONE :
	printf("       <string>#1#</string>\n");
	break;
    case GRAPH_CARD_ZERO_TO_MANY :
	printf("       <string>#*#</string>\n");
	break;
    case GRAPH_CARD_ONE_TO_ZERO_OR_ONE :
	printf("       <string>#0..1#</string>\n");
	break;	
    }
    
    printf("          </attribute>\n");
    printf("          <attribute name=\"arrow\">\n");
    printf("            <boolean val=\"false\"/>\n");
    printf("          </attribute>\n");
    printf("          <attribute name=\"aggregate\">\n");
    printf("            <enum val=\"%d\"/>\n",aggregate);
    printf("          </attribute>\n");
    printf("        </composite>\n");
    printf("      </attribute>\n");

    diaPrintXMLConPoints(tEdge);
    
    printf("    </object>\n");
}

static void diaPrintXMLConnection(GraphEdge *tEdge)
{
    switch (tEdge->connection) {
    case GRAPH_CON_UNKNOWN:
	break;
    case GRAPH_CON_AGGREGATION :
	diaPrintXMLAssociation(tEdge,1);
	break;
    case GRAPH_CON_DEPENDENCY :
	diaPrintXMLDependency(tEdge);
	break;
    case GRAPH_CON_ASSOCIATION :
	diaPrintXMLAssociation(tEdge,0);
	break;	    
    }
}

/*
 * diaPrintXMLInfoNote
 *
 * Prints an UML note with a short information on it (Modulename and
 * smidump version).
 */
static void diaPrintXMLInfoNote(int modc, SmiModule **modv)
{
    size_t  length;
    float   width;
    char   *note;
    int	    i;

    const char *s1 = "Conceptual model of ";
    const char *s2 = "- generated by smidump " SMI_VERSION_STRING;

    /*
     * Calculate the length of the string...
     */
    
    length = strlen(s1) + strlen(s2) + 1;

    for (i = 0; i < modc; i++) {
	length += strlen(modv[i]->name) + 1;
    }

    /*
     * ... before allocating a buffer and putting the string together.
     */

    note = xmalloc(length);
    strcpy(note, s1);
    for (i = 0; i < modc; i++) {
	strcat(note, modv[i]->name);
	strcat(note, " ");
    }
    strcat(note, s2);

    width = (float)strlen(note) * (float)0.76;	/* don't ask */

    printf("<object type=\"UML - Note\" version=\"0\" id=\"O0\">\n");
    printf("  <attribute name=\"obj_pos\">\n");
    printf("    <point val=\"%.2f,%.2f\"/>\n",XNOTE, YNOTE);
    printf("  </attribute>\n");
    printf("  <attribute name=\"obj_bb\">\n");
    printf("    <rectangle val=\"%.2f,%.2f;%.2f,%.2f\"/>\n",
	   XNOTE-0.5, YNOTE-0.5, XNOTE-0.5 + width, YNOTE - 0.5 + 1.7);
    printf("  </attribute>\n");
    printf("  <attribute name=\"elem_corner\">\n");
    printf("    <point val=\"%.2f,%.2f\"/>\n",XNOTE, YNOTE);
    printf("  </attribute>\n");
    printf("  <attribute name=\"elem_width\">\n");
    printf("    <real val=\"%.2f\"/>\n", width);
    printf("  </attribute>\n");
    printf("  <attribute name=\"elem_height\">\n");
    printf("    <real val=\"1.7\"/>\n");
    printf("  </attribute>\n");
    printf("  <attribute name=\"text\">\n");
    printf("    <composite type=\"text\">\n");
    printf("      <attribute name=\"string\">\n");
    printf("        <string>#%s#</string>\n", note);
    printf("      </attribute>\n");
    printf("      <attribute name=\"font\">\n");
    printf("        <font name=\"Courier\"/>\n");
    printf("      </attribute>\n");
    printf("      <attribute name=\"height\">\n");
    printf("        <real val=\"0.8\"/>\n");
    printf("      </attribute>\n");
    printf("      <attribute name=\"pos\">\n");
    printf("        <point val=\"%.2f,%.2f\"/>\n", XNOTE + 0.35, YNOTE + 1.28);
    printf("      </attribute>\n");
    printf("      <attribute name=\"color\">\n");
    printf("        <color val=\"#000000\"/>\n");
    printf("      </attribute>\n");
    printf("      <attribute name=\"alignment\">\n");
    printf("        <enum val=\"0\"/>\n");
    printf("      </attribute>\n");
    printf("    </composite>\n");
    printf("  </attribute>\n");
    printf("</object>\n");

    xfree(note);
}

/*
 * diaCalcSize
 *
 * Calculates the size of a given node for the UML representation.
 */
static GraphNode *diaCalcSize(GraphNode *node)
{
    GraphEdge  *tEdge;
    SmiNode    *tNode,*ppNode;
    SmiElement *smiElement;
    SmiModule  *module;

    if (node->smiNode->nodekind == SMI_NODEKIND_SCALAR) return node;
    
    node->dia.w = (strlen(node->smiNode->name)+4) * HEADFONTSIZETABLE
	+ HEADSPACESIZETABLE;
    
    node->dia.h = TABLEHEIGHT;
    for (smiElement = smiGetFirstElement(
	smiGetFirstChildNode(node->smiNode));
	 smiElement;
	 smiElement = smiGetNextElement(smiElement)) {
	
	tNode = smiGetElementNode(smiElement);
	
	node->dia.w = max(node->dia.w, (strlen(tNode->name) +
					strlen(algGetTypeName(tNode)) +
					strlen(INDEXPROPERTY))
		      * ATTRFONTSIZE
		      + ATTRSPACESIZE);
	node->dia.h += TABLEELEMHEIGHT;
    }
    
    for (tEdge = graphGetFirstEdgeByNode(graph,node);
	 tEdge;
	 tEdge = graphGetNextEdgeByNode(graph, tEdge, node)) {
	if (tEdge->startNode == node &&
	    tEdge->endNode->smiNode->nodekind == SMI_NODEKIND_SCALAR) {
	    node->dia.h += TABLEELEMHEIGHT;
	    tNode = tEdge->endNode->smiNode;
	    
	    node->dia.w = max(node->dia.w, (strlen(tNode->name) +
				    strlen(algGetTypeName(tNode)))
			  * ATTRFONTSIZE
			  + ATTRSPACESIZE);		
	}
    }

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

		typeName = algGetTypeName(tNode);
		len = strlen(tNode->name) + (typeName ? strlen(typeName) : 0);
		node->dia.h += TABLEELEMHEIGHT;
		node->dia.w = max(node->dia.w, len)
		    * ATTRFONTSIZE
		    + ATTRSPACESIZE;
	    }
	}
    }
    
    return node;
}

static float diaPrintNode(GraphNode *node, float x, float y)
{
    GraphEdge *tEdge;

    for (tEdge = graphGetFirstEdgeByNode(graph, node);
	 tEdge;
	 tEdge = graphGetNextEdgeByNode(graph, tEdge, node)) {
	if (! (tEdge->dia.flags & DIA_PRINT_FLAG)) {
	    if (node == tEdge->startNode) {
		y += tEdge->endNode->dia.h + YSPACING;    
		diaPrintXMLObject(tEdge->endNode, x, y);
		diaPrintXMLConnection(tEdge);
		y = diaPrintNode(tEdge->startNode, x, y);
			      /* (x+tEdge->startNode->dia.w+XSPACING),y); */
		
		y = diaPrintNode(tEdge->endNode,
		  (x+tEdge->startNode->dia.w+XSPACING), y);
	    }
	}
    }

    return y;
}

static void diaPrintXML(int modc, SmiModule **modv)
{
    GraphNode *tNode;
    GraphEdge *tEdge;
    float     x,y,ydiff;
    int       group;
    
    diaPrintXMLHeader();

    for (tNode = graph->nodes; tNode; tNode = tNode->nextPtr) {	
	tNode = diaCalcSize(tNode);
    }

    diaPrintXMLInfoNote(modc, modv);
    
    x = XOFFSET;
    y = YOFFSET;
    ydiff = 0;

    for (tEdge = graph->edges; tEdge; tEdge = tEdge->nextPtr) {
	if (! (tEdge->dia.flags & DIA_PRINT_FLAG)) {
	    diaPrintXMLObject(tEdge->startNode, x, y);
	    x = x + tEdge->startNode->dia.w + XSPACING;

	    diaPrintXMLObject(tEdge->endNode, x, y);
	    diaPrintXMLConnection(tEdge);
	    
	    ydiff = tEdge->startNode->dia.h;

      	    y = diaPrintNode(tEdge->startNode,x,y);
	    y = diaPrintNode(tEdge->endNode,x,y);    

	    y = y + ydiff + YSPACING;
	    x = XOFFSET;
	}
    }
    
    x = XOFFSET;
    y += ydiff;
    ydiff = 0;
    
    /* printing singular tables */
    for (tNode = graph->nodes; tNode; tNode = tNode->nextPtr) {
	if (!graphGetFirstEdgeByNode(graph,tNode) &&
	    tNode->smiNode->nodekind != SMI_NODEKIND_SCALAR) {
	    diaPrintXMLObject(tNode,x,y);
	    
	    x += tNode->dia.w + XSPACING;
	    ydiff = max(ydiff, tNode->dia.h);
	    if (x >= NEWLINEDISTANCE) {
		x = XOFFSET;
		y += ydiff + YSPACING;
	    }
	}
    }

    /* printing scalar groups */
    x = XOFFSET;
    y += ydiff + YSPACING;
    for (group = 1;
	 group <= algGetNumberOfGroups();
	 group++) {
	diaPrintXMLGroup(group,x,y);
	x += 2.0;
	y += 2.0;
    }
    
    diaPrintXMLClose();
}



/* ------------------------------------------------------------------------- */

static void printModuleNames(int modc, SmiModule **modv)
{
    int i;
    
    printf("Conceptual model of: ");

    for (i = 0; i < modc; i++) {
	printf("%s ", modv[i]->name);
    }

    printf("(generated by smidump " SMI_VERSION_STRING ")\n\n");
}



static void dumpCm(int modc, SmiModule **modv, int flags, char *output)
{
    int       i;

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
	
	if (XPLAIN) {
	    printModuleNames(modc, modv);
	    printf("\n--- First Phase - loading tables and scalars\n\n");
	    graphShowNodes(graph);
	    printf("\n");
	}
	
	algLinkTables();
	algCheckLinksByName();
	algConnectLonelyNodes();
	algCheckForDependency();
	algCheckForPointerRels();
	
	if (!XPLAIN) {
	    diaPrintXML(modc, modv);
	}
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
	    
	    if (XPLAIN) {
		printModuleNames(1, &(modv[i]));
		printf("\n--- First Phase - loading tables and scalars\n\n");
		graphShowNodes(graph);
		printf("\n");
	    }
	
	    algLinkTables();
	    algCheckLinksByName();
	    algConnectLonelyNodes();
	    algCheckForDependency();
	    algCheckForPointerRels();
	    
	    if (!XPLAIN) {
		diaPrintXML(1, &(modv[i]));
	    }
	
	    graphExit(graph);
	    graph = NULL;
	}
    }

    if (fflush(stdout) || ferror(stdout)) {
	perror("smidump: write error");
	exit(1);
    }
}



void initCm()
{
    static SmidumpDriverOption opt[] = {
	{ "explain", OPT_FLAG, &XPLAIN, 0,
	  "explain what the algorithm does"},
        { 0, OPT_END, 0, 0 }
    };

    static SmidumpDriver driver = {
	"cm",
	dumpCm,
	SMI_FLAG_NODESCR,
	SMIDUMP_DRIVER_CANT_OUTPUT,
	"reverse engineered conceptual model",
	opt,
	NULL
    };

    smidumpRegisterDriver(&driver);
}
