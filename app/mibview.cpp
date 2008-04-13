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
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include <qlabel.h>
#include <qmenu.h>
#include <qcursor.h>
#include <QContextMenuEvent>
#include <qtreewidget.h>
#include <QTreeWidgetItemIterator>

#include "mibview.h"

//
// BasicMibView class
//
//

BasicMibView::BasicMibView (QWidget * parent) : QTreeWidget(parent)
{
    // Set some properties for the TreeView
    header()->hide();
    setSortingEnabled( FALSE );
    setHorizontalScrollBarPolicy ( Qt::ScrollBarAlwaysOn );
    header()->setSortIndicatorShown( FALSE );
    setLineWidth( 2 );
    setAllColumnsShowFocus( FALSE );
    setFrameShape(QFrame::WinPanel);
    setFrameShadow(QFrame::Plain);
    setRootIsDecorated( TRUE );
    
    // Create context menu actions
    expandAct = new QAction(tr("&Expand"), this);
    expandAct->setIcon(QIcon(":/images/expand.png"));
    connect(expandAct, SIGNAL(triggered()), this, SLOT(ExpandFromNode()));
    collapseAct = new QAction(tr("&Collapse"), this);
    collapseAct->setIcon(QIcon(":/images/collapse.png"));
    connect(collapseAct, SIGNAL(triggered()), this, SLOT(CollapseFromNode()));
    
    // Connect some signals
    connect( this, SIGNAL( itemExpanded( QTreeWidgetItem * ) ),
             this, SLOT( ExpandNode( QTreeWidgetItem * ) ) );
    connect( this, SIGNAL( itemCollapsed( QTreeWidgetItem * ) ),
             this, SLOT( CollapseNode( QTreeWidgetItem * ) ) );
    connect( this, SIGNAL( currentItemChanged( QTreeWidgetItem *, QTreeWidgetItem * ) ),
             this, SLOT( SelectedNode( QTreeWidgetItem *, QTreeWidgetItem * ) ) );

    // Force initial refresh
    isdirty = 1;
}

void BasicMibView::SetDirty(void)
{
    isdirty = 1;
}

void BasicMibView::RegisterToLoader(MibViewLoader *loader)
{
    MibLoader = loader;
    MibLoader->RegisterView(this);
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
            MibLoader->PopulateSubTree(smiNode, root, NULL);
    }
}

void BasicMibView::ExpandFromNode(void)
{
    QTreeWidgetItem *start = NULL, *end = NULL;
    
    // Could it be null ?
    if ((start = currentItem()) == NULL)
        return;

    // Go back in the tree till we find a sibling to mark the end
    // If end is NULL, we expanded from the root
    QTreeWidgetItem *ptr = start;
    while (ptr && ptr->parent() && 
           !(end = ptr->parent()->child(ptr->parent()->indexOfChild(ptr) + 1)))
        ptr = ptr->parent();
 
    // Now go thru all nodes till the end marker
    QTreeWidgetItemIterator it( start );
    while ( *it && (*it != end)) {
        QTreeWidgetItem *item = *it;
        item->setExpanded(TRUE);
        ++it;
    }
}

void BasicMibView::CollapseFromNode(void)
{
    QTreeWidgetItem *start = NULL, *end = NULL;
    
    // Could it be null ?
    if ((start = currentItem()) == NULL)
        return;

    // Go back in the tree till we find a sibling to mark the end
    // If end is NULL, we collapsed from the root
    QTreeWidgetItem *ptr = start;
    while (ptr && ptr->parent() && 
           !(end = ptr->parent()->child(ptr->parent()->indexOfChild(ptr) + 1)))
        ptr = ptr->parent();
    
    // Now go thru all nodes till the end marker
    QTreeWidgetItemIterator it( start );
    while ( *it && (*it != end)) {
        QTreeWidgetItem *item = *it;
        item->setExpanded(FALSE);
        ++it;
    }
}

void BasicMibView::ExpandNode( QTreeWidgetItem * item)
{
    MibNode *node = (MibNode*)item;
    node->SetPixmap(TRUE);
}

void BasicMibView::CollapseNode( QTreeWidgetItem * item)
{
    MibNode *node = (MibNode*)item;
    node->SetPixmap(FALSE);
}

void BasicMibView::SelectedNode( QTreeWidgetItem * item, QTreeWidgetItem *)
{
    MibNode *node = (MibNode*)item;
    
    if (node)
        emit SelectedOid(node->GetOid());
}

void BasicMibView::contextMenuEvent ( QContextMenuEvent *event)
{    
    QMenu menu(tr("Operations"), this);

    menu.addAction(expandAct);
    menu.addAction(collapseAct);

    menu.exec(event->globalPos());
}

//
// MibView class
//
//

MibView::MibView (QWidget * parent) : BasicMibView(parent)
{
    // Create context menu actions
    walkAct = new QAction(tr("&Walk"), this);
    connect(walkAct, SIGNAL(triggered()), this, SLOT(WalkFromNode()));
    getAct = new QAction(tr("&Get"), this);
    connect(getAct, SIGNAL(triggered()), this, SLOT(GetFromNode()));
    getnextAct = new QAction(tr("&Get Next"), this);
    connect(getnextAct, SIGNAL(triggered()), this, SLOT(GetNextFromNode()));
    setAct = new QAction(tr("&Set..."), this);
    connect(setAct, SIGNAL(triggered()), this, SLOT(SetFromNode()));
    stopAct = new QAction(tr("&Stop"), this);
    connect(stopAct, SIGNAL(triggered()), this, SLOT(StopFromNode()));
    tableviewAct = new QAction(tr("&Table View"), this);
    connect(tableviewAct, SIGNAL(triggered()), this, SLOT(TableViewFromNode()));
}

void MibView::WalkFromNode(void)
{
    QTreeWidgetItem *start = NULL;
    
    // Could it be null ?
    if ((start = currentItem()) == NULL)
        return;
    
    emit WalkFromOid(((MibNode*)start)->GetOid());
}

void MibView::GetFromNode(void)
{
    QTreeWidgetItem *start = NULL;
    
    // Could it be null ?
    if ((start = currentItem()) == NULL)
        return;
    
    QString oid(((MibNode*)start)->GetOid());
    oid += ".0";
    emit GetFromOid(oid);
}

void MibView::GetNextFromNode(void)
{
    QTreeWidgetItem *start = NULL;
    
    // Could it be null ?
    if ((start = currentItem()) == NULL)
        return;
    
    QString oid(((MibNode*)start)->GetOid());
    oid += ".0";
    emit GetNextFromOid(oid);
}

void MibView::SetFromNode(void)
{
    QTreeWidgetItem *start = NULL;
    
    // Could it be null ?
    if ((start = currentItem()) == NULL)
        return;
    
    emit SetFromOid(((MibNode*)start)->GetOid());
}

void MibView::StopFromNode(void)
{
    QTreeWidgetItem *start = NULL;
    
    // Could it be null ?
    if ((start = currentItem()) == NULL)
        return;
    
    emit StopFromOid(((MibNode*)start)->GetOid());
}

void MibView::TableViewFromNode(void)
{
    QTreeWidgetItem *start = NULL;
    
    // Could it be null ?
    if ((start = currentItem()) == NULL)
        return;
    
    emit TableViewFromOid(((MibNode*)start)->GetOid());
}

void MibView::SelectedNode( QTreeWidgetItem * item, QTreeWidgetItem *)
{
    MibNode *node = (MibNode*)item;
    QString text;

    if (node)
    {
        node->PrintProperties(text);
        emit NodeProperties(text);
    }
}

void MibView::contextMenuEvent ( QContextMenuEvent *event)
{
    QMenu menu(tr("Operations"), this);

    menu.addAction(expandAct);
    menu.addAction(collapseAct);
    menu.addSeparator();
    menu.addAction(walkAct);
    menu.addAction(getAct);
    menu.addAction(getnextAct);
#ifdef NOTYET
    menu.addAction(setAct);
    menu.addSeparator();
    menu.addAction(stopAct);
#endif
    menu.addSeparator();
    menu.addAction(tableviewAct);

    menu.exec(event->globalPos());
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

void MibViewLoader::Load(QStringList &modules)
{
    char *modulename;
    SmiModule *smiModule;
    SmiModule **modv = NULL;
    int modc = 0;
    
    modv = (SmiModule **)malloc(modules.count() * sizeof(SmiModule *));
    modc = 0;
    
    QString module;

    for (int j=0; j < views.count(); j++)
    {
        views[j]->SetDirty();
        views[j]->clear();
    }

    for (int i=0; i < modules.count(); i++) 
    {
        module = modules[i];
        modulename = smiLoadModule(module.toLatin1().data());
        smiModule = modulename ? smiGetModule(modulename) : NULL;

        if (smiModule)
            modv[modc++] = smiModule;
        else
        {
            emit LogError(QString("Error: `%1` module cannot be loaded (not in PATHS)")
                                  .arg(module.toLatin1().data()));
        }
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
