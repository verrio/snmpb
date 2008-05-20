/*
    Copyright (C) 2004-2008  Martin Jolicoeur (snmpb1@gmail.com) 

    This file is part of the SnmpB project 
    (http://sourceforge.net/projects/snmpb)

    This program is free software: you can redistribute it and/or modify
    it under the terms of the GNU General Public License as published by
    the Free Software Foundation, either version 2 of the License, or
    (at your option) any later version.

    This program is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
    GNU General Public License for more details.

    You should have received a copy of the GNU General Public License
    along with this program.  If not, see <http://www.gnu.org/licenses/>.
*/
#include "mibnode.h"
#include <qtextedit.h>

MibNode::MibNode(enum MibType mibtype, SmiNode *node, MibNode * parent, MibNode * sibling) : QTreeWidgetItem(parent, sibling)
{    
    Type = mibtype;
    Node = node;
    setText(0, node->name); 
    SetPixmap(FALSE);
}

MibNode::MibNode(QString label, QTreeWidget* parent) : QTreeWidgetItem(parent)
{
    setText (0, label);
    Type = MIBNODE_NODE;
    Node = NULL;
    SetPixmap(FALSE);    
}

void MibNode::SetPixmap(bool isOpened)
{
    switch(Type)
    {
    case MIBNODE_SCALAR:
        setIcon( 0,  QIcon( ":/images/scalar.png" ));
        break;
    case MIBNODE_COLUMN:
        setIcon( 0,  QIcon( ":/images/column_item.png" ));
        break;
    case MIBNODE_ROW:
       if (isOpened)		
            setIcon( 0,  QIcon( ":/images/folder_red_open.png" ));
        else
            setIcon( 0,  QIcon( ":/images/folder_red.png" ));
        break;
    case MIBNODE_TABLE:	    
       if (isOpened)		
            setIcon( 0,  QIcon( ":/images/folder_blue_open.png" ));
        else
            setIcon( 0,  QIcon( ":/images/folder_blue.png" ));
        break;
    case MIBNODE_NOTIFICATION:
        setIcon( 0,  QIcon( ":/images/notification.png" ));
        break;
    case MIBNODE_GROUP:
        setIcon( 0,  QIcon( ":/images/group.png" ));
        break;
    case MIBNODE_COMPLIANCE:
        setIcon( 0,  QIcon( ":/images/compliance.png" ));
        break;
    case MIBNODE_CAPABILITIES:
        setIcon( 0,  QIcon( ":/images/agentcap.png" ));
        break;
    case MIBNODE_NODE:
    default:
        if (isOpened)		
            setIcon( 0,  QIcon( ":/images/folder_yellow_open.png" ));
        else
            setIcon( 0,  QIcon( ":/images/folder_yellow.png" ));
    }
}

char *MibNode::GetAccess(void)
{
    switch (Node->access)
    {
    case SMI_ACCESS_NOT_ACCESSIBLE:
        return (char*)"not-accessible";
    case SMI_ACCESS_NOTIFY:
        return (char*)"notify";
    case SMI_ACCESS_READ_ONLY:
        return (char*)"read-only";
    case SMI_ACCESS_READ_WRITE:
        return (char*)"read-write";
    case SMI_ACCESS_INSTALL:
        return (char*)"install";
    case SMI_ACCESS_INSTALL_NOTIFY:
        return (char*)"install-notify";
    case SMI_ACCESS_REPORT_ONLY:
        return (char*)"report-only";
    case SMI_ACCESS_UNKNOWN:
    case SMI_ACCESS_NOT_IMPLEMENTED:
        break;
    }
	
    return (char*)"";
}

char *MibNode::GetStatus(void)
{
    switch (Node->status)
    {
    case SMI_STATUS_CURRENT:
        return (char*)"current";
    case SMI_STATUS_DEPRECATED:
        return (char*)"<font color=red>deprecated</font>";
    case SMI_STATUS_MANDATORY:
        return (char*)"mandatory";
    case SMI_STATUS_OPTIONAL:
        return (char*)"optional";
    case SMI_STATUS_OBSOLETE:
        return (char*)"<font color=red>obsolete</font>";
    case SMI_STATUS_UNKNOWN:
        break;
    }

    return (char*)"";
}

char *MibNode::GetKindName(void)
{
    switch(Node->nodekind)
    {
    case SMI_NODEKIND_NODE:
        return (char*)"Node";
    case SMI_NODEKIND_SCALAR:
        return (char*)"Scalar";
    case SMI_NODEKIND_TABLE:
        return (char*)"Table";
    case SMI_NODEKIND_ROW:
        return (char*)"Row";
    case SMI_NODEKIND_COLUMN:
        return (char*)"Column";
    case SMI_NODEKIND_NOTIFICATION:
        return (char*)"Notification";
    case SMI_NODEKIND_GROUP:
        return (char*)"Group";
    case SMI_NODEKIND_COMPLIANCE:
        return (char*)"Compliance";
    case SMI_NODEKIND_CAPABILITIES:
        return (char*)"Capabilities";
    case SMI_NODEKIND_UNKNOWN:
    case SMI_NODEKIND_ANY:
    default:
        break;
    }
    
    return (char*)"";
}

char *MibNode::GetSmiTypeName(void)
{
    switch(Node->decl)
    {    
    /* SMIv1/v2 ASN.1 statements and macros */    
    case SMI_DECL_OBJECTTYPE:
        return (char*)"OBJECT-TYPE";
    case SMI_DECL_OBJECTIDENTITY:
        return (char*)"OBJECT-IDENTITY";
    case SMI_DECL_MODULEIDENTITY:
        return (char*)"MODULE-IDENTITY";
    case SMI_DECL_NOTIFICATIONTYPE:
        return (char*)"NOTIFICATION-TYPE";
    case SMI_DECL_TRAPTYPE:
        return (char*)"TRAP-TYPE";
    case SMI_DECL_OBJECTGROUP:
        return (char*)"OBJECT-GROUP";
    case SMI_DECL_NOTIFICATIONGROUP:
        return (char*)"NOTIFICATION-GROUP";
    case SMI_DECL_MODULECOMPLIANCE:
        return (char*)"MODULE-COMPLIANCE";
    case SMI_DECL_AGENTCAPABILITIES:
        return (char*)"AGENT-CAPABILITIES";
    case SMI_DECL_VALUEASSIGNMENT:
        return (char*)"OBJECT-IDENTIFIER";
    /* SMIng statements */
    case SMI_DECL_MODULE:
        return (char*)"module";
    case SMI_DECL_NODE:
        return (char*)"node";
    case SMI_DECL_SCALAR:
        return (char*)"scalar";
    case SMI_DECL_TABLE:
        return (char*)"table";
    case SMI_DECL_ROW:
        return (char*)"row";
    case SMI_DECL_COLUMN:
        return (char*)"column";
    case SMI_DECL_NOTIFICATION:
        return (char*)"notification";
    case SMI_DECL_GROUP:
        return (char*)"group";
    case SMI_DECL_COMPLIANCE:
        return (char*)"compliance";
        
    case SMI_DECL_IMPLICIT_TYPE:
    case SMI_DECL_TYPEASSIGNMENT:
    case SMI_DECL_IMPL_SEQUENCEOF:
    case SMI_DECL_TEXTUALCONVENTION:
    case SMI_DECL_MACRO:
    case SMI_DECL_COMPL_GROUP:
    case SMI_DECL_COMPL_OBJECT:
    case SMI_DECL_EXTENSION:
    case SMI_DECL_TYPEDEF:
    case SMI_DECL_UNKNOWN:
    default:
        break;
    }
    
    return (char*)"";
}
                                  
char *MibNode::GetTypeName(void)
{
    SmiType *smiType, *parentType;
    
    smiType = smiGetNodeType(Node);

    if (!smiType || Node->nodekind == SMI_NODEKIND_TABLE)
        return NULL;

    if (smiType->decl == SMI_DECL_IMPLICIT_TYPE)
    {
        parentType = smiGetParentType(smiType);

        if (!parentType)
            return NULL;
        smiType = parentType;
    }

    return (smiType->name);
}

char *MibNode::GetOid(void)
{
    // If the Node is NULL, this is the MIBTree node, return the iso OID ...
    return (Node?smiRenderOID(Node->oidlen, Node->oid, 
                             SMI_RENDER_NUMERIC):(char*)"1");
}

void MibNode::PrintProperties(QString& text)
{
       if (!Node)
	   return;

       // Create a table and add elements ...
       text = QString("<table border=\"1\" cellpadding=\"0\" cellspacing=\"0\" align=\"left\">");  
       
       // Add the name
       text += QString("<tr><td><b>Name:</b></td><td><font color=#009000><b>%1</b></font></td>").arg(Node->name);
	   
       // Add the full Oid
       text += QString("<tr><td><b>Oid:</b></td><td>%1</td></tr>").arg(smiRenderOID(Node->oidlen, Node->oid, SMI_RENDER_NUMERIC));
       
       // Add misc attributes
       text += QString("<tr><td><b>Type:</b></td><td>%1</td></tr>").arg(GetTypeName());
       text += QString("<tr><td><b>Status:</b></td><td>%1</td></tr>").arg(GetStatus());
       text += QString("<tr><td><b>Access:</b></td><td>%1</td></tr>").arg(GetAccess());
       text += QString("<tr><td><b>Kind:</b></td><td>%1</td></tr>").arg(GetKindName());
       text += QString("<tr><td><b>SMI Type:</b></td><td>%1</td></tr>").arg(GetSmiTypeName());
       
       // Add units (seconds, bits, ....)
       text += QString("<tr><td><b>Units:</b></td><td>%1</td></tr>").arg(Node->units);
       
       // Add module
       text += QString("<tr><td><b>Module:</b></td><td>%1</td></tr>").arg(smiGetNodeModule(Node)->name);

       // Add the reference
       text += QString("<tr><td><b>Reference:</b></td><td><font face=fixed size=-1 color=blue>");
       text += Qt::convertFromPlainText (Node->reference);
       text += QString("</font></td></tr>");
       
       // Add the description
       text += QString("<tr><td><b>Description:</b></td><td><font face=fixed size=-1 color=blue>");
       text += Qt::convertFromPlainText (Node->description);
       text += QString("</font></td></tr>");
	   
       text += QString("</table>");
}

