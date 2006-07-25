#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include <q3popupmenu.h>
#include <qlabel.h>
#include <qcursor.h>
//Added by qt3to4:
#include <QContextMenuEvent>
#include <q3mimefactory.h>
#include <Q3StrList>

#include "mibview.h"

MibViewLoader MibLoader;

//
// BasicMibView class
//
//

BasicMibView::BasicMibView (QWidget * parent, const char * name, Qt::WFlags f) : Q3ListView(parent, name, f)
{
    // Set some properties for the ListView
    header()->hide();
    setSorting(-1, false);
    setLineWidth( 2 );
    addColumn( tr( "MibName" ) );
    setHScrollBarMode(Q3ScrollView::AlwaysOn);
    setFrameShadow( MibView::Plain );
    setResizePolicy( Q3ScrollView::Manual );
    setAllColumnsShowFocus( FALSE );
    setShowSortIndicator( FALSE );
    setRootIsDecorated( TRUE );
    
    MibLoader.RegisterView(this);
    
    // Connect some signals
    connect( this, SIGNAL( expanded( Q3ListViewItem * ) ),
             this, SLOT( ExpandNode( Q3ListViewItem * ) ) );
    connect( this, SIGNAL( collapsed( Q3ListViewItem * ) ),
             this, SLOT( CollapseNode( Q3ListViewItem * ) ) );
    connect( this, SIGNAL( currentChanged( Q3ListViewItem * ) ),
             this, SLOT( SelectedNode( Q3ListViewItem * ) ) );
}

void BasicMibView::SetDirty(void)
{
    isdirty = 1;
}

void BasicMibView::TreeTabSelected( int index )
{
    if(index == 0) Populate();
    if(index == 3) Populate();
}

void BasicMibView::Populate(void)
{
    SmiNode *smiNode;
    
    if (isdirty)
    {
        isdirty = 0;
        // Create the root folder
        MibNode *root = new MibNode("MIB Tree", this);
        
        smiNode = smiGetNode(NULL, "iso");
        if (smiNode)
            MibLoader.PopulateSubTree(smiNode, root, NULL);
    }
}

void BasicMibView::ExpandFromNode(void)
{
    Q3ListViewItem *start = NULL, *end = NULL;
    
    // Could it be null ?
    if ((start = currentItem()) == NULL)
        return;
    
    // Go back in the tree till we find a sibling to mark the end
    // If end is NULL, we expanded from the root
    Q3ListViewItem *ptr = start;
    while (ptr && !(end = ptr->nextSibling()))
        ptr = ptr->parent();
    
    // Now go thru all nodes till the end marker
    Q3ListViewItemIterator it( start );
    while ( it.current() && (it.current() != end)) {
        Q3ListViewItem *item = it.current();
        item->setOpen(TRUE);
        ++it;
    }
}

void BasicMibView::CollapseFromNode(void)
{
    Q3ListViewItem *start = NULL, *end = NULL;
    
    // Could it be null ?
    if ((start = currentItem()) == NULL)
        return;
    
    // Go back in the tree till we find a sibling to mark the end
    // If end is NULL, we collapsed from the root
    Q3ListViewItem *ptr = start;
    while (ptr && !(end = ptr->nextSibling()))
        ptr = ptr->parent();
    
    // Now go thru all nodes till the end marker
    Q3ListViewItemIterator it( start );
    while ( it.current() && (it.current() != end)) {
        Q3ListViewItem *item = it.current();
        item->setOpen(FALSE);
        ++it;
    }
}

void BasicMibView::ExpandNode( Q3ListViewItem * item)
{
    MibNode *node = (MibNode*)item;
    node->SetPixmap(TRUE);
}

void BasicMibView::CollapseNode( Q3ListViewItem * item)
{
    MibNode *node = (MibNode*)item;
    node->SetPixmap(FALSE);
}

void BasicMibView::SelectedNode( Q3ListViewItem * item)
{
    MibNode *node = (MibNode*)item;
    
    // Could it be null ?
    if (node == NULL)
        return;
    
    emit SelectedOid(node->GetOid());
}

void BasicMibView::contextMenuEvent ( QContextMenuEvent *)
{    
    Q3PopupMenu* contextMenu = new Q3PopupMenu( this );
    QLabel *caption = new QLabel( "<font color=darkblue><b>Operations</b></font>", this );
    caption->setAlignment( Qt::AlignCenter );
    contextMenu->insertItem( caption->text() );
    contextMenu->insertItem( qPixmapFromMimeSource( "expand.png" ), 
                             "&Expand", this, SLOT(ExpandFromNode()));
    contextMenu->insertItem( qPixmapFromMimeSource( "collapse.png" ), 
                             "&Collapse", this, SLOT(CollapseFromNode()));
    
    contextMenu->exec( QCursor::pos() );
    delete contextMenu;
}

//
// MibView class
//
//

MibView::MibView (QWidget * parent, const char * name, Qt::WFlags f) : BasicMibView(parent, name, f)
{
}

void MibView::WalkFromNode(void)
{
    Q3ListViewItem *start = NULL;
    
    // Could it be null ?
    if ((start = currentItem()) == NULL)
        return;
    
    emit WalkFromOid(((MibNode*)start)->GetOid());
}

void MibView::GetFromNode(void)
{
    Q3ListViewItem *start = NULL;
    
    // Could it be null ?
    if ((start = currentItem()) == NULL)
        return;
    
    QString oid(((MibNode*)start)->GetOid());
    oid += ".0";
    emit GetFromOid(oid);
}

void MibView::GetNextFromNode(void)
{
    Q3ListViewItem *start = NULL;
    
    // Could it be null ?
    if ((start = currentItem()) == NULL)
        return;
    
    QString oid(((MibNode*)start)->GetOid());
    oid += ".0";
    emit GetNextFromOid(oid);
}

void MibView::SetFromNode(void)
{
    Q3ListViewItem *start = NULL;
    
    // Could it be null ?
    if ((start = currentItem()) == NULL)
        return;
    
    emit SetFromOid(((MibNode*)start)->GetOid());
}

void MibView::StopFromNode(void)
{
    Q3ListViewItem *start = NULL;
    
    // Could it be null ?
    if ((start = currentItem()) == NULL)
        return;
    
    emit StopFromOid(((MibNode*)start)->GetOid());
}

void MibView::TableViewFromNode(void)
{
    Q3ListViewItem *start = NULL;
    
    // Could it be null ?
    if ((start = currentItem()) == NULL)
        return;
    
    emit TableViewFromOid(((MibNode*)start)->GetOid());
}

void MibView::SelectedNode( Q3ListViewItem * item)
{
    MibNode *node = (MibNode*)item;
    QString text;
    
    node->PrintProperties(text);
    emit NodeProperties(text);
}

void MibView::contextMenuEvent ( QContextMenuEvent *)
{    
    Q3PopupMenu* contextMenu = new Q3PopupMenu( this );
    QLabel *caption = new QLabel( "<font color=darkblue><b>Operations</b></font>", this );
    caption->setAlignment( Qt::AlignCenter );
    contextMenu->insertItem( caption->text() );
    contextMenu->insertItem( qPixmapFromMimeSource( "expand.png" ), 
                             "&Expand", this, SLOT(ExpandFromNode()));
    contextMenu->insertItem( qPixmapFromMimeSource( "collapse.png" ), 
                             "&Collapse", this, SLOT(CollapseFromNode()));
    contextMenu->insertSeparator();
    contextMenu->insertItem("&Walk", this, SLOT(WalkFromNode()));
    contextMenu->insertItem("&Get", this, SLOT(GetFromNode()));
    contextMenu->insertItem("&Get Next", this, SLOT(GetNextFromNode()));
    contextMenu->insertItem("&Set...", this, SLOT(SetFromNode()));
    contextMenu->insertSeparator();
    contextMenu->insertItem("&Stop", this, SLOT(StopFromNode()));
    contextMenu->insertSeparator();
    contextMenu->insertItem("&Table View", this, SLOT(TableViewFromNode()));
    
    contextMenu->exec( QCursor::pos() );
    delete contextMenu;
}

//
// MibViewLoader class
//
//

MibViewLoader::MibViewLoader ()
{    
    pmodc = 0;
    pmodv = NULL;
    ignoreconformance = 0;
    ignoreleafs = 0;
}

void MibViewLoader::Load(Q3StrList &modules)
{
    char *modulename;
    SmiModule *smiModule;
    SmiModule **modv = NULL;
    int modc = 0;
    
    modv = (SmiModule **)malloc(modules.count() * sizeof(SmiModule *));
    modc = 0;
    
    const char * module = NULL;
    
    if ( (module = modules.first()) != 0)
    {
        BasicMibView *mv;
        Q3PtrListIterator<BasicMibView> it( views );
        while ( (mv = it.current()) != 0 )
        {
            mv->SetDirty();
            mv->clear();
            ++it;
        }
        
        do
        {
            modulename = smiLoadModule(module);
            smiModule = modulename ? smiGetModule(modulename) : NULL;
            
            if (smiModule)
                modv[modc++] = smiModule;
            else
                fprintf(stderr, "SnmpB: cannot locate module `%s'\n", module);
        } while ( (module = modules.next()) != 0);
    }
    
    pmodc = modc;
    if (pmodv)
        free(pmodv);
    pmodv = modv;
}

void MibViewLoader::RegisterView(BasicMibView* view)
{
    views.append(view);
}

int MibViewLoader::IsPartOfLoadedModules(SmiNode *smiNode)
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

int MibViewLoader::PruneSubTree(SmiNode *smiNode)
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

enum MibNode::MibType MibViewLoader::SmiKindToMibNodeType(int smikind)
{
    switch(smikind)
    {
    case SMI_NODEKIND_NODE:
        return (MibNode::MIBNODE_NODE);
    case SMI_NODEKIND_SCALAR:
        return (MibNode::MIBNODE_SCALAR);
    case SMI_NODEKIND_TABLE:
        return (MibNode::MIBNODE_TABLE);
    case SMI_NODEKIND_ROW:
        return (MibNode::MIBNODE_ROW);
    case SMI_NODEKIND_COLUMN:
        return (MibNode::MIBNODE_COLUMN);
    case SMI_NODEKIND_NOTIFICATION:
        return (MibNode::MIBNODE_NOTIFICATION);
    case SMI_NODEKIND_GROUP:
        return (MibNode::MIBNODE_GROUP);
    case SMI_NODEKIND_COMPLIANCE:
        return (MibNode::MIBNODE_COMPLIANCE);
    case SMI_NODEKIND_CAPABILITIES:
        return (MibNode::MIBNODE_CAPABILITIES);
    case SMI_NODEKIND_UNKNOWN:
    case SMI_NODEKIND_ANY:
    default:
        break;
    }
    
    return (MibNode::MIBNODE_NODE);
}

MibNode * MibViewLoader::PopulateSubTree (SmiNode *smiNode, MibNode *parent, MibNode *sibling)
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
