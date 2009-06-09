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

#include "mibnode.h"
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
    expandAct = new QAction(tr("Expand"), this);
    expandAct->setIcon(QIcon(":/images/expand.png"));
    connect(expandAct, SIGNAL(triggered()), this, SLOT(ExpandFromNode()));
    collapseAct = new QAction(tr("Collapse"), this);
    collapseAct->setIcon(QIcon(":/images/collapse.png"));
    connect(collapseAct, SIGNAL(triggered()), this, SLOT(CollapseFromNode()));
    findAct = new QAction(tr("Find"), this);
    connect(findAct, SIGNAL(triggered()), this, SLOT(FindFromNode()));
    
    // Connect some signals
    connect( this, SIGNAL( itemExpanded( QTreeWidgetItem * ) ),
             this, SLOT( ExpandNode( QTreeWidgetItem * ) ) );
    connect( this, SIGNAL( itemCollapsed( QTreeWidgetItem * ) ),
             this, SLOT( CollapseNode( QTreeWidgetItem * ) ) );
    connect( this, SIGNAL( currentItemChanged( QTreeWidgetItem *, QTreeWidgetItem * ) ),
             this, SLOT( SelectedNode( QTreeWidgetItem *, QTreeWidgetItem * ) ) );

    // Force initial refresh
    isdirty = 1;

    find_string = "";
    find_last = model()->index(0, 0, QModelIndex());
    find_back = false;
    find_cs = false;
    find_word = false;
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

void BasicMibView::FindFromNode(void)
{
    QDialog d;

    find_uid.setupUi(&d);
    connect( find_uid.buttonFindNext, SIGNAL( clicked() ), 
             this, SLOT( ExecuteFind() ));
    find_uid.comboFind->setFocus(Qt::TabFocusReason);

    find_uid.comboFind->addItems(find_strings);
    if (!find_string.isEmpty())
        find_uid.comboFind->setCurrentIndex(find_uid.comboFind->findText(find_string));
    find_last = model()->index(0, 0, QModelIndex());
    d.exec();
}

void BasicMibView::ExecuteFindNext(void)
{
    Find(false);
}

void BasicMibView::ExecuteFind(void)
{
    Find(true);
}

void BasicMibView::Find(bool reevaluate)
{
    if (reevaluate)
    {
        find_string = find_uid.comboFind->currentText();
        if (!find_strings.contains(find_string))
            find_strings.append(find_string);

        if (find_uid.checkWords->isChecked())
            find_word = true;
        else
            find_word = false;
        if (find_uid.checkCase->isChecked())
            find_cs = true;
        else
            find_cs = false;
        if (find_uid.checkBackward->isChecked())
            find_back = true;
        else
            find_back = false;
    }

    QTreeWidgetItem *start = NULL, *begin = NULL, *end = NULL, *cur = NULL;
  
    // Determine begin of tree 
    begin = itemFromIndex(model()->index(0, 0, QModelIndex()));
 
    // Determine end of tree
    if (find_back)
    {
        QTreeWidgetItemIterator it(begin);
        while ( *it ) end = *it++;
    }

    // Determine where we start the find 
    if ((start = itemFromIndex(find_last)) == NULL)
        start = begin;

    // Create iterator
    QTreeWidgetItemIterator it( start );

    goto start_find;

    // Loop thru tree items and break if item is found
    while ( *it && (*it != start))
    {
        cur = *it;

        if ((find_word && !cur->text(0).compare(find_string, 
                           find_cs?Qt::CaseSensitive:Qt::CaseInsensitive)) ||
            (!find_word && cur->text(0).contains(find_string, 
                           find_cs?Qt::CaseSensitive:Qt::CaseInsensitive)))
        {
            // Found item
            setCurrentItem(cur);
            find_last = indexFromItem(cur);
            break;
        }

start_find:
        // Move to next item, handle tree wrap-around
        if (find_back)
        {
            --it;
            if (!*it) it = QTreeWidgetItemIterator(end);
        }
        else
        {
            ++it;
            if (!*it) it = QTreeWidgetItemIterator(begin);
        }
    }
}

void BasicMibView::SelectFromOid(const QString& oid)
{
    QTreeWidgetItem *start = NULL, *cur = NULL;

    // Determine begin of tree 
    start = itemFromIndex(model()->index(0, 0, QModelIndex()));
 
    // Create iterator
    QTreeWidgetItemIterator it( start );

    // Loop thru tree items and break if item is found
    while ( (cur = *it++) != NULL)
    {
        if (((MibNode*)cur)->GetOid() == oid) 
        {
            // Found item
            setCurrentItem(cur);
            break;
        }
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
    menu.addSeparator();
    menu.addAction(findAct);

    menu.exec(event->globalPos());
}

//
// MibView class
//
//

MibView::MibView (QWidget * parent) : BasicMibView(parent)
{
    // Create context menu actions
    walkAct = new QAction(tr("Walk"), this);
    connect(walkAct, SIGNAL(triggered()), this, SLOT(WalkFromNode()));

    getAct = new QAction(tr("Get"), this);
    connect(getAct, SIGNAL(triggered()), this, SLOT(GetFromNode()));
    getPromptAct = new QAction(tr("Prompt for instance..."), this);
    connect(getPromptAct, SIGNAL(triggered()), this, SLOT(GetFromNodePromptInstance()));
    getSelectAct = new QAction(tr("Select Instance"), this);
    connect(getSelectAct, SIGNAL(triggered()), this, SLOT(GetFromNodeSelectInstance()));

    getnextAct = new QAction(tr("Get Next"), this);
    connect(getnextAct, SIGNAL(triggered()), this, SLOT(GetNextFromNode()));
    getnextPromptAct = new QAction(tr("Prompt for instance..."), this);
    connect(getnextPromptAct, SIGNAL(triggered()), this, SLOT(GetNextFromNodePromptInstance()));
    getnextSelectAct = new QAction(tr("Select Instance"), this);
    connect(getnextSelectAct, SIGNAL(triggered()), this, SLOT(GetNextFromNodeSelectInstance()));

    getbulkAct = new QAction(tr("Get Bulk"), this);
    connect(getbulkAct, SIGNAL(triggered()), this, SLOT(GetBulkFromNode()));
    getbulkPromptAct = new QAction(tr("Prompt for instance..."), this);
    connect(getbulkPromptAct, SIGNAL(triggered()), this, SLOT(GetBulkFromNodePromptInstance()));
    getbulkSelectAct = new QAction(tr("Select Instance"), this);
    connect(getbulkSelectAct, SIGNAL(triggered()), this, SLOT(GetBulkFromNodeSelectInstance()));

    setAct = new QAction(tr("Set..."), this);
    connect(setAct, SIGNAL(triggered()), this, SLOT(SetFromNode()));
    stopAct = new QAction(tr("Stop"), this);
    connect(stopAct, SIGNAL(triggered()), this, SLOT(StopNode()));
    tableviewAct = new QAction(tr("Table View"), this);
    connect(tableviewAct, SIGNAL(triggered()), this, SLOT(TableViewFromNode()));
    varbindsAct = new QAction(tr("Multiple Varbinds..."), this);
    connect(varbindsAct, SIGNAL(triggered()), this, SLOT(VarbindsFromNode()));

    walkinprogress = false;
    agentisv1 = true; 
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
    emit GetFromOid(oid, 0);
}

void MibView::GetFromNodePromptInstance(void)
{
    QTreeWidgetItem *start = NULL;
    
    // Could it be null ?
    if ((start = currentItem()) == NULL)
        return;

    QString oid(((MibNode*)start)->GetOid());
    emit GetFromOidPromptInstance(oid, 0);
}

void MibView::GetFromNodeSelectInstance(void)
{
    QTreeWidgetItem *start = NULL;
    
    // Could it be null ?
    if ((start = currentItem()) == NULL)
        return;

    QString oid(((MibNode*)start)->GetOid());
    emit GetFromOidSelectInstance(oid, 0);
}

void MibView::GetNextFromNode(void)
{
    QTreeWidgetItem *start = NULL;
    
    // Could it be null ?
    if ((start = currentItem()) == NULL)
        return;
   
    QString oid(((MibNode*)start)->GetOid());
    oid += ".0";
    emit GetFromOid(oid, 1);
}

void MibView::GetNextFromNodePromptInstance(void)
{
    QTreeWidgetItem *start = NULL;
    
    // Could it be null ?
    if ((start = currentItem()) == NULL)
        return;
   
    QString oid(((MibNode*)start)->GetOid());
    emit GetFromOidPromptInstance(oid, 1);
}

void MibView::GetNextFromNodeSelectInstance(void)
{
    QTreeWidgetItem *start = NULL;
    
    // Could it be null ?
    if ((start = currentItem()) == NULL)
        return;
   
    QString oid(((MibNode*)start)->GetOid());
    emit GetFromOidSelectInstance(oid, 1);
}

void MibView::GetBulkFromNode(void)
{
    QTreeWidgetItem *start = NULL;
    
    // Could it be null ?
    if ((start = currentItem()) == NULL)
        return;
   
    QString oid(((MibNode*)start)->GetOid());
    oid += ".0";
    emit GetFromOid(oid, 2);
}

void MibView::GetBulkFromNodePromptInstance(void)
{
    QTreeWidgetItem *start = NULL;
    
    // Could it be null ?
    if ((start = currentItem()) == NULL)
        return;
   
    QString oid(((MibNode*)start)->GetOid());
    emit GetFromOidPromptInstance(oid, 2);
}

void MibView::GetBulkFromNodeSelectInstance(void)
{
    QTreeWidgetItem *start = NULL;
    
    // Could it be null ?
    if ((start = currentItem()) == NULL)
        return;
   
    QString oid(((MibNode*)start)->GetOid());
    emit GetFromOidSelectInstance(oid, 2);
}

void MibView::SetFromNode(void)
{
    QTreeWidgetItem *start = NULL;
    
    // Could it be null ?
    if ((start = currentItem()) == NULL)
        return;
    
    emit SetFromOid(((MibNode*)start)->GetOid());
}

void MibView::StopNode(void)
{
    QTreeWidgetItem *start = NULL;
    
    // Could it be null ?
    if ((start = currentItem()) == NULL)
        return;
    
    emit Stop();
}

void MibView::TableViewFromNode(void)
{
    QTreeWidgetItem *start = NULL;
    
    // Could it be null ?
    if ((start = currentItem()) == NULL)
        return;
    
    emit TableViewFromOid(((MibNode*)start)->GetOid());
}

void MibView::VarbindsFromNode(void)
{
    QTreeWidgetItem *start = NULL;
    
    // Could it be null ?
    if ((start = currentItem()) == NULL)
        return;
    
    QString oid(((MibNode*)start)->GetOid());
    oid += ".0";
    emit VarbindsFromOid(oid);
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
    /*
       Node kinds:

       MIBNODE_NODE,
       MIBNODE_SCALAR,
       MIBNODE_TABLE,
       MIBNODE_ROW,
       MIBNODE_COLUMN,
       MIBNODE_NOTIFICATION,
       MIBNODE_GROUP,
       MIBNODE_COMPLIANCE,
    */
    enum MibNode::MibType kind = currentItem()?((MibNode*)currentItem())
                                 ->GetKind():MibNode::MIBNODE_NODE;
    QMenu menu(tr("Operations"), this);

    menu.addAction(expandAct);
    menu.addAction(collapseAct);
    menu.addSeparator();

    menu.addAction(walkAct);
    menu.addAction(stopAct);
    if (walkinprogress == true)
        stopAct->setEnabled(true);
    else
        stopAct->setEnabled(false);
    menu.addSeparator();

    if (kind == MibNode::MIBNODE_COLUMN)
    {
        QMenu *get_menu = menu.addMenu("Get");
        get_menu->addAction(getSelectAct);
        get_menu->addAction(getPromptAct);

        QMenu *getnext_menu = menu.addMenu("Get Next");
        getnextAct->setText("No Instance");
        getnext_menu->addAction(getnextAct);
        getnext_menu->addAction(getnextSelectAct);
        getnext_menu->addAction(getnextPromptAct);

        QMenu *getbulk_menu = menu.addMenu("Get Bulk");
        getbulkAct->setText("No Instance");
        getbulk_menu->addAction(getbulkAct);
        getbulk_menu->addAction(getbulkSelectAct);
        getbulk_menu->addAction(getbulkPromptAct);
        if (agentisv1)
            getbulk_menu->setEnabled(false);
        else
            getbulk_menu->setEnabled(true);
    }
    else
    {
        menu.addAction(getAct);
        if (kind == MibNode::MIBNODE_SCALAR)
            getAct->setEnabled(true);
        else
            getAct->setEnabled(false);

        getnextAct->setText("Get Next");
        menu.addAction(getnextAct);

        getbulkAct->setText("Get Bulk");
        if (agentisv1)
            getbulkAct->setEnabled(false);
        else
            getbulkAct->setEnabled(true);
        menu.addAction(getbulkAct);
    }

    menu.addAction(setAct);
    if ((kind == MibNode::MIBNODE_COLUMN) || (kind == MibNode::MIBNODE_SCALAR))
        setAct->setEnabled(true);
    else
        setAct->setEnabled(false);

    menu.addSeparator();

    menu.addAction(tableviewAct);
    if ((kind == MibNode::MIBNODE_TABLE) || (kind == MibNode::MIBNODE_ROW))
        tableviewAct->setEnabled(true);
    else
        tableviewAct->setEnabled(false);
    menu.addAction(varbindsAct);
    if ((kind == MibNode::MIBNODE_COLUMN) || (kind == MibNode::MIBNODE_SCALAR))
        varbindsAct->setEnabled(true);
    else
        varbindsAct->setEnabled(false);
    menu.addSeparator();

    menu.addAction(findAct);

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
