#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

#include <qpopupmenu.h>
#include <qlabel.h>
#include <qcursor.h>

#include "mibview.h"

void MibView::contextMenuEvent ( QContextMenuEvent *)
{    
    QPopupMenu* contextMenu = new QPopupMenu( this );
    QLabel *caption = new QLabel( "<font color=darkblue><b>Operations</b></font>", this );
    caption->setAlignment( Qt::AlignCenter );
    contextMenu->insertItem( caption );
    contextMenu->insertItem( QPixmap::fromMimeSource( "expand.png" ), "&Expand", this, SLOT(ExpandFromNode()));
    contextMenu->insertItem( QPixmap::fromMimeSource( "collapse.png" ), "&Collapse", this, SLOT(CollapseFromNode()));
    contextMenu->exec( QCursor::pos() );
    delete contextMenu;
}

void MibView::ExpandFromNode(void)
{
    QListViewItem *start = NULL, *end = NULL;
    
    // Could it be null ?
    if ((start = currentItem()) == NULL)
        return;
    
    // Go back in the tree till we find a sibling to mark the end
    // If end is NULL, we expanded from the root
    QListViewItem *ptr = start;
    while (ptr && !(end = ptr->nextSibling()))
        ptr = ptr->parent();
    
    // Now go thru all nodes till the end marker
    QListViewItemIterator it( start );
    while ( it.current() && (it.current() != end)) {
        QListViewItem *item = it.current();
        item->setOpen(TRUE);
        ++it;
    }
}

void MibView::CollapseFromNode(void)
{
    QListViewItem *start = NULL, *end = NULL;
    
    // Could it be null ?
    if ((start = currentItem()) == NULL)
        return;
    
    // Go back in the tree till we find a sibling to mark the end
    // If end is NULL, we collapsed from the root
    QListViewItem *ptr = start;
    while (ptr && !(end = ptr->nextSibling()))
        ptr = ptr->parent();
    
    // Now go thru all nodes till the end marker
    QListViewItemIterator it( start );
    while ( it.current() && (it.current() != end)) {
        QListViewItem *item = it.current();
        item->setOpen(FALSE);
        ++it;
    }
}

MibView::MibView (QWidget * parent, const char * name, WFlags f) : QListView(parent, name, f)
{
     // Set some properties for the ListView
    header()->hide();
    setSorting(-1, false);
    setLineWidth( 2 );
    addColumn( tr( "MibName" ) );
    setHScrollBarMode(QScrollView::AlwaysOn);
    setFrameShadow( MibView::Plain );
    setResizePolicy( QScrollView::Manual );
    setAllColumnsShowFocus( FALSE );
    setShowSortIndicator( FALSE );
    setRootIsDecorated( TRUE );
    
    pmodc = 0;
    pmodv = NULL;
    ignoreconformance = 0;
    ignoreleafs = 0;
    
    // Connect some signals
    connect( this, SIGNAL( expanded( QListViewItem * ) ),
                 this, SLOT( ExpandNode( QListViewItem * ) ) );
    connect( this, SIGNAL( collapsed( QListViewItem * ) ),
                 this, SLOT( CollapseNode( QListViewItem * ) ) );
    connect( this, SIGNAL( currentChanged( QListViewItem * ) ),
                 this, SLOT( SelectedNode( QListViewItem * ) ) );
}

void MibView::ExpandNode( QListViewItem * item)
{
    MibNode *node = (MibNode*)item;
    node->SetPixmap(TRUE);
}

void MibView::CollapseNode( QListViewItem * item)
{
    MibNode *node = (MibNode*)item;
    node->SetPixmap(FALSE);
}

void MibView::SelectedNode( QListViewItem * item)
{
    MibNode *node = (MibNode*)item;
    QString text;
    
    node->PrintProperties(text);
    emit NodeProperties(text);
}

void MibView::Populate(void)
{    
    char *modulename;
    SmiModule *smiModule;
    int smiflags, i;
    SmiModule **modv = NULL;
    int modc = 0;
    SmiNode *smiNode;

     // Create the root folder
    MibNode *root = new MibNode("MIB Tree", this);
    
    printf("SnmpB using libsmi version: " SMI_VERSION_STRING "\n");
        
    smiInit(NULL);

    smiflags = smiGetFlags();
    smiflags |= SMI_FLAG_ERRORS;
    smiSetFlags(smiflags);

    smiSetErrorLevel(0);

    int argc = 19;
    char argv[19][100] = {
	"OSPF-MIB", 
	"IF-MIB", 
	"RFC1213-MIB", 
	"BGP4-MIB", 
	"SONET-MIB", 
	"IP-MIB", 
	"IP-FORWARD-MIB",
	"SNMP-COMMUNITY-MIB",
	"SNMP-FRAMEWORK-MIB",
	"SNMP-MPD-MIB",
	"SNMP-NOTIFICATION-MIB",
	"SNMP-TARGET-MIB",
	"SNMP-USER-BASED-SM-MIB",
	"SNMPv2-CONF",
	"SNMPv2-MIB",
	"SNMPv2-SMI",
	"SNMPv2-TC",
	"SNMP-VIEW-BASED-ACM-MIB",
	"CISCO-CCM-CAPABILITY"
    };
    
    modv = (SmiModule **)malloc(argc * sizeof(SmiModule *));
    modc = 0;

    for (i = 0; i < argc; i++)
    {
        modulename = smiLoadModule(argv[i]);
        smiModule = modulename ? smiGetModule(modulename) : NULL;

        if (smiModule)
        {
            if ((smiModule->conformance) && (smiModule->conformance < 3))
            {
                fprintf(stderr,
                    "snmpb: module `%s' contains errors, expect flawed output\n", argv[i]);
            }

            modv[modc++] = smiModule;
        }
        else
        {
            fprintf(stderr, "snmpb: cannot locate module `%s'\n", argv[i]);
        }
    }

    pmodc = modc;
    pmodv = modv;

    smiNode = smiGetNode(NULL, "iso");    
    if (smiNode)
        PopulateSubTree(smiNode, root, NULL);
    
    if (modv)
        free(modv);
}

int MibView::IsPartOfLoadedModules(SmiNode *smiNode)
{
    SmiModule *smiModule;
    int i;

    smiModule = smiGetNodeModule(smiNode);

    for (i = 0; i < pmodc; i++) {
        if (strcmp(pmodv[i]->name, smiModule->name) == 0) {
            return 1;
        }
    }
    return 0;
}

/*
 * The following function pruneSubTree() is tricky. There are some
 * interactions between the supported options. See the detailed
 * comments below. Good examples to test the implemented behaviour
 * are:
 *
 * smidump -u -f tree --tree-no-leaf IF-MIB ETHER-CHIPSET-MIB
 *
 * (And the example above does _not_ work in combination with
 * --tree-no-conformance so the code below is still broken.)
 */

int MibView::PruneSubTree(SmiNode *smiNode)
{
    SmiNode   *childNode;

    const int confmask = (SMI_NODEKIND_GROUP | SMI_NODEKIND_COMPLIANCE);
    const int leafmask = (SMI_NODEKIND_GROUP | SMI_NODEKIND_COMPLIANCE
                          | SMI_NODEKIND_COLUMN | SMI_NODEKIND_SCALAR
                          | SMI_NODEKIND_ROW | SMI_NODEKIND_NOTIFICATION);

    if (! smiNode) {
        return 1;
    }

    /*
     * First, prune all nodes which the user has told us to ignore.
     * In the case of ignoreleafs, we have to special case nodes with
     * an unknown status (which actually represent OBJECT-IDENTITY
     * definitions). More special case code is needed to exclude
     * module identity nodes.
     */

    if (ignoreconformance && (smiNode->nodekind & confmask)) {
        return 1;
    }

    if (ignoreleafs) {
        if (smiNode->nodekind & leafmask) {
            return 1;
        }
        if (smiNode->nodekind == SMI_NODEKIND_NODE
            && smiNode->status != SMI_STATUS_UNKNOWN) {
            SmiModule *smiModule = smiGetNodeModule(smiNode);
            if (smiModule && smiNode != smiGetModuleIdentityNode(smiModule)) {
                return 1;
            }
        }
    }

    /*
      * Next, generally do not prune nodes that belong to the set of
      * modules we are looking at.
      */

    if (IsPartOfLoadedModules(smiNode)) {
        if (!ignoreconformance || !smiGetFirstChildNode(smiNode)) {
            return 0;
        }
    }

    /*
     * Finally, prune all nodes where all child nodes are pruned.
     */

    for (childNode = smiGetFirstChildNode(smiNode);
         childNode;
         childNode = smiGetNextChildNode(childNode)) {

        /*
         * In the case of ignoreleafs, we have to peek at the child
         * nodes. Otherwise, we would prune too much. we still want to
         * see the path to the leafs we have pruned away. This also
         * interact with the semantics of ignoreconformance since we
         * still want in combination with ignoreleafs to see the path
         * to the pruned conformance leafs.
         */

        if (ignoreleafs && (childNode->nodekind & leafmask)) {
            if (IsPartOfLoadedModules(childNode)) {
                if (ignoreconformance && (childNode->nodekind & confmask)) {
                    return 1;
                }
                return 0;
            }
        }

        if (! PruneSubTree(childNode)) {
            return 0;
        }
    }

    return 1;
}

enum MibNode::MibType MibView::SmiKindToMibNodeType(int smikind)
{
    enum MibNode::MibType val;
    
    switch(smikind)
    {
    case SMI_NODEKIND_NODE:
	val = MibNode::MIBNODE_NODE;
	break;
    case SMI_NODEKIND_SCALAR:
	val = MibNode::MIBNODE_SCALAR;
	break;
    case SMI_NODEKIND_TABLE:
	val = MibNode::MIBNODE_TABLE;
	break;
    case SMI_NODEKIND_ROW:
	val = MibNode::MIBNODE_ROW;
	break;
    case SMI_NODEKIND_COLUMN:
	val = MibNode::MIBNODE_COLUMN;
	break;
    case SMI_NODEKIND_NOTIFICATION:
	val = MibNode::MIBNODE_NOTIFICATION;
	break;
    case SMI_NODEKIND_GROUP:
	val = MibNode::MIBNODE_GROUP;
	break;
    case SMI_NODEKIND_COMPLIANCE:
	val = MibNode::MIBNODE_COMPLIANCE;
	break;
    case SMI_NODEKIND_CAPABILITIES:
	val = MibNode::MIBNODE_CAPABILITIES;
	break;
    case SMI_NODEKIND_UNKNOWN:
    case SMI_NODEKIND_ANY:
    default:
	val = MibNode::MIBNODE_NODE; /* Huh ?*/
    }
    return val;
}

MibNode * MibView::PopulateSubTree (SmiNode *smiNode, MibNode *parent, MibNode *sibling)
{
    SmiNode     *childNode;
    MibNode *current = NULL, *prev = NULL;
    
    if (smiNode)
    {
        current = new MibNode(SmiKindToMibNodeType(smiNode->nodekind), 
			       smiNode, parent, sibling);
	       
       for (childNode = smiGetFirstChildNode(smiNode);
             childNode;
             childNode = smiGetNextChildNode(childNode))
        {
            if (PruneSubTree(childNode)) continue;
	    
            prev = PopulateSubTree(childNode, current, prev);
        }
    }
    
    return current;
}
