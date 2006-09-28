#include "mibnode.h"
//Added by qt3to4:
#include <q3mimefactory.h>

MibNode::MibNode(enum MibType mibtype, SmiNode *node, MibNode * parent, MibNode * sibling) : Q3ListViewItem(parent, sibling)
{    
    Type = mibtype;
    Node = node;
    setText(0, node->name); 
    SetPixmap(FALSE);
}

MibNode::MibNode(QString label, Q3ListView* parent) : Q3ListViewItem(parent, label)
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
        setPixmap( 0,  qPixmapFromMimeSource( "images/scalar.png" ));
        break;
    case MIBNODE_COLUMN:
        setPixmap( 0,  qPixmapFromMimeSource( "images/column_item.png" ));
        break;
    case MIBNODE_ROW:
       if (isOpened)		
            setPixmap( 0,  qPixmapFromMimeSource( "images/folder_red_open.png" ));
        else
            setPixmap( 0,  qPixmapFromMimeSource( "images/folder_red.png" ));
        break;
    case MIBNODE_TABLE:	    
       if (isOpened)		
            setPixmap( 0,  qPixmapFromMimeSource( "images/folder_blue_open.png" ));
        else
            setPixmap( 0,  qPixmapFromMimeSource( "images/folder_blue.png" ));
        break;
    case MIBNODE_NOTIFICATION:
        setPixmap( 0,  qPixmapFromMimeSource( "images/notification.png" ));
        break;
    case MIBNODE_GROUP:
        setPixmap( 0,  qPixmapFromMimeSource( "images/group.png" ));
        break;
    case MIBNODE_COMPLIANCE:
        setPixmap( 0,  qPixmapFromMimeSource( "images/compliance.png" ));
        break;
    case MIBNODE_CAPABILITIES:
        setPixmap( 0,  qPixmapFromMimeSource( "images/agentcap.png" ));
        break;
    case MIBNODE_NODE:
    default:
        if (isOpened)		
            setPixmap( 0,  qPixmapFromMimeSource( "images/folder_yellow_open.png" ));
        else
            setPixmap( 0,  qPixmapFromMimeSource( "images/folder_yellow.png" ));
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

char *MibNode::GetKindName(void)
{
    switch(Node->nodekind)
    {
    case SMI_NODEKIND_NODE:
        return "Node";
    case SMI_NODEKIND_SCALAR:
        return "Scalar";
    case SMI_NODEKIND_TABLE:
        return "Table";
    case SMI_NODEKIND_ROW:
        return "Row";
    case SMI_NODEKIND_COLUMN:
        return "Column";
    case SMI_NODEKIND_NOTIFICATION:
        return "Notification";
    case SMI_NODEKIND_GROUP:
        return "Group";
    case SMI_NODEKIND_COMPLIANCE:
        return "Compliance";
    case SMI_NODEKIND_CAPABILITIES:
        return "Capabilities";
    case SMI_NODEKIND_UNKNOWN:
    case SMI_NODEKIND_ANY:
    default:
        break;
    }
    
    return "";
}

char *MibNode::GetSmiTypeName(void)
{
    switch(Node->decl)
    {    
    /* SMIv1/v2 ASN.1 statements and macros */    
    case SMI_DECL_OBJECTTYPE:
        return "OBJECT-TYPE";
    case SMI_DECL_OBJECTIDENTITY:
        return "OBJECT-IDENTITY";
    case SMI_DECL_MODULEIDENTITY:
        return "MODULE-IDENTITY";
    case SMI_DECL_NOTIFICATIONTYPE:
        return "NOTIFICATION-TYPE";
    case SMI_DECL_TRAPTYPE:
        return "TRAP-TYPE";
    case SMI_DECL_OBJECTGROUP:
        return "OBJECT-GROUP";
    case SMI_DECL_NOTIFICATIONGROUP:
        return "NOTIFICATION-GROUP";
    case SMI_DECL_MODULECOMPLIANCE:
        return "MODULE-COMPLIANCE";
    case SMI_DECL_AGENTCAPABILITIES:
        return "AGENT-CAPABILITIES";
    case SMI_DECL_VALUEASSIGNMENT:
        return "OBJECT-IDENTIFIER";
    /* SMIng statements */
    case SMI_DECL_MODULE:
        return "module";
    case SMI_DECL_NODE:
        return "node";
    case SMI_DECL_SCALAR:
        return "scalar";
    case SMI_DECL_TABLE:
        return "table";
    case SMI_DECL_ROW:
        return "row";
    case SMI_DECL_COLUMN:
        return "column";
    case SMI_DECL_NOTIFICATION:
        return "notification";
    case SMI_DECL_GROUP:
        return "group";
    case SMI_DECL_COMPLIANCE:
        return "compliance";
        
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
       text += Q3StyleSheet::convertFromPlainText (Node->reference);
       text += QString("</font></td>>/tr>");
       
       // Add the description
       text += QString("<tr><td><b>Description:</b></td><td><font face=fixed size=-1 color=blue>");
       text += Q3StyleSheet::convertFromPlainText (Node->description);
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
            
       SmiType *smiGetNodeType(SmiNode *smiNodePtr);
       
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
       
       SmiType *smiGetType(SmiModule *smiModulePtr, char *type);
       SmiType *smiGetFirstType(SmiModule *smiModulePtr);
       SmiType *smiGetNextType(SmiType *smiTypePtr);
       SmiType *smiGetParentType(SmiType *smiTypePtr);
       
       SmiRange *smiGetFirstRange(SmiType *smiTypePtr);
       SmiRange *smiGetNextRange(SmiRange *smiRangePtr);
       
       SmiNamedNumber *smiGetFirstNamedNumber(SmiType *smiTypePtr);
       SmiNamedNumber *smiGetNextNamedNumber(SmiNamedNumber *smiNamedNumberPtr);

       typedef struct SmiNode {
           
           SmiValue            value;
           
           SmiIndexkind        indexkind;
           int                 implied;
           int                 create;
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
