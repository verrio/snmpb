#include "mibnode.h"

MibNode::MibNode(enum MibType mibtype, SmiNode *node, MibNode * parent, MibNode * sibling) : QListViewItem(parent, sibling)
{    
    Type = mibtype;
    Node = node;
    setText(0, node->name); 
    SetPixmap(FALSE);
}

MibNode::MibNode(QString label, QListView* parent) : QListViewItem(parent, label)
{
    Type = MIBNODE_NODE;
    Node = NULL;
    SetPixmap(FALSE);    
}

void MibNode::SetPixmap(bool isOpened)
{
    switch(Type)
    {
    case MIBNODE_SCALAR:
        setPixmap( 0,  QPixmap::fromMimeSource( "scalar.png" ));
        break;
    case MIBNODE_COLUMN:
        setPixmap( 0,  QPixmap::fromMimeSource( "column_item.png" ));
        break;
    case MIBNODE_ROW:
       if (isOpened)		
            setPixmap( 0,  QPixmap::fromMimeSource( "folder_red_open.png" ));
        else
            setPixmap( 0,  QPixmap::fromMimeSource( "folder_red.png" ));
        break;
    case MIBNODE_TABLE:	    
       if (isOpened)		
            setPixmap( 0,  QPixmap::fromMimeSource( "folder_blue_open.png" ));
        else
            setPixmap( 0,  QPixmap::fromMimeSource( "folder_blue.png" ));
        break;
    case MIBNODE_NOTIFICATION:
        setPixmap( 0,  QPixmap::fromMimeSource( "notification.png" ));
        break;
    case MIBNODE_GROUP:
        setPixmap( 0,  QPixmap::fromMimeSource( "group.png" ));
        break;
    case MIBNODE_COMPLIANCE:
        setPixmap( 0,  QPixmap::fromMimeSource( "compliance.png" ));
        break;
    case MIBNODE_CAPABILITIES:
        setPixmap( 0,  QPixmap::fromMimeSource( "agentcap.png" ));
        break;
    case MIBNODE_NODE:
    default:
        if (isOpened)		
            setPixmap( 0,  QPixmap::fromMimeSource( "folder_yellow_open.png" ));
        else
            setPixmap( 0,  QPixmap::fromMimeSource( "folder_yellow.png" ));
    }
}

char *MibNode::GetAccess(void)
{
    switch (Node->access)
    {
    case SMI_ACCESS_NOT_ACCESSIBLE:
        return "not-accessible";
    case SMI_ACCESS_NOTIFY:
        return "notify";
    case SMI_ACCESS_READ_ONLY:
        return "read-only";
    case SMI_ACCESS_READ_WRITE:
        return "read-write";
    case SMI_ACCESS_INSTALL:
        return "install";
    case SMI_ACCESS_INSTALL_NOTIFY:
        return "install-notify";
    case SMI_ACCESS_REPORT_ONLY:
        return "report-only";
    case SMI_ACCESS_UNKNOWN:
    case SMI_ACCESS_NOT_IMPLEMENTED:
        break;
    }
	
    return "";
}

char *MibNode::GetStatus(void)
{
    switch (Node->status)
    {
    case SMI_STATUS_CURRENT:
        return "current";
    case SMI_STATUS_DEPRECATED:
        return "<font color=red>deprecated</font>";
    case SMI_STATUS_MANDATORY:
        return "mandatory";
    case SMI_STATUS_OPTIONAL:
        return "optional";
    case SMI_STATUS_OBSOLETE:
        return "<font color=red>obsolete</font>";
    case SMI_STATUS_UNKNOWN:
        break;
    }

    return "";
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
void MibNode::PrintProperties(QString& text)
{
       if (!Node)
	   return;
       
       // Create a table and add elements ...
       text = QString("<table border=\"1\" cellpadding=\"0\" cellspacing=\"0\" align=\"left\">");  
       
       // Add the name
       text += QString("<tr><td><b>Name:</b></td><td><font color=#009000><b>%1</b></font></td>").arg(Node->name);
	   
       // Add the full Oid
       text += QString("<tr><td><b>Oid:</b></td><td>");
       for (unsigned int i = 0; i < Node->oidlen; i++)
       {
	   text += QString("%1").arg(Node->oid[i]);
	   if (i != (Node->oidlen-1))
	       text += ".";
       }
       text += QString("</td></tr>");
       
       // Add misc attributes
       text += QString("<tr><td><b>Type:</b></td><td>%1</td></tr>").arg(GetTypeName());
       text += QString("<tr><td><b>Status:</b></td><td>%1</td></tr>").arg(GetStatus());
       text += QString("<tr><td><b>Access:</b></td><td>%1</td></tr>").arg(GetAccess());
	   
       // Add the description
       text += QString("<tr><td><b>Description:</b></td><td><font face=fixed size=-1 color=blue>");
       text += QStyleSheet::convertFromPlainText (Node->description);
       text += QString("</font></td>>/tr>");
	   
       text += QString("</table>");
}


       /* Table Indexes print
            switch (smiNode->indexkind)
            {
            case SMI_INDEX_INDEX:
            case SMI_INDEX_REORDER:
                fprintIndex(smiNode);
                break;
            case SMI_INDEX_EXPAND: //TODO: we have to do more work here!
                break;
            case SMI_INDEX_AUGMENT:
            case SMI_INDEX_SPARSE:
                indexNode = smiGetRelatedNode(smiNode);
                if (indexNode)
                {
                    fprintIndex(indexNode);
                }
                break;
            case SMI_INDEX_UNKNOWN:
                break;
            }
            
       SmiModule *smiGetNodeModule(SmiNode *smiNodePtr);
       SmiType *smiGetNodeType(SmiNode *smiNodePtr);
       int smiGetNodeLine(SmiNode *smiNodePtr);
       SmiElement *smiGetFirstElement(SmiNode *smiNodePtr);
       SmiElement *smiGetNextElement(SmiElement *smiElementPtr);
       SmiNode *smiGetElementNode(SmiElement *smiElementPtr);
       SmiOption *smiGetFirstOption(SmiNode *smiComplianceNodePtr);
       SmiOption *smiGetNextOption(SmiOption *smiOptionPtr);
       SmiNode *smiGetOptionNode(SmiOption *smiOptionPtr);
       SmiRefinement *smiGetFirstRefinement(SmiNode *smiComplianceNodePtr);
       SmiRefinement *smiGetNextRefinement(SmiRefinement *smiRefinementPtr);
       SmiNode *smiGetRefinementNode(SmiRefinement *smiRefinementPtr);
       SmiType *smiGetRefinementType(SmiRefinement *smiRefinementPtr);
       SmiType *smiGetRefinementWriteType(SmiRefinement *smiRefinementPtr);

       typedef struct SmiNode {
           SmiIdentifier       name;
           int                 oidlen;
           SmiSubid            *oid; 
           SmiDecl             decl;
           SmiAccess           access;
           SmiStatus           status;
           char                *format;
           SmiValue            value;
           char                *units;
           char                *description;
           char                *reference;
           SmiIndexkind        indexkind;
           int                 implied;
           int                 create;
           SmiNodekind         nodekind;
       } SmiNode;

       typedef struct SmiElement {
           // no visible attributes
       } SmiElement;

       typedef struct SmiOption {
           char                *description;
       } SmiOption;

       typedef struct SmiRefinement {
           SmiAccess           access;
           char                *description;
       } SmiRefinement;  
*/
