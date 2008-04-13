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
#ifndef MIBVIEW_H
#define MIBVIEW_H

#include <qstring.h>
#include <qfile.h>
#include <qfileinfo.h>
#include <qtimer.h>
#include <qpixmap.h>
#include <QContextMenuEvent>
#include <qtreewidget.h>
#include <qheaderview.h>
#include "mibnode.h"
#include "smi.h"

class MibViewLoader;

class BasicMibView : public QTreeWidget
{
    Q_OBJECT
    
public:
    BasicMibView ( QWidget * parent = 0 );
    void Populate (void);
    void SetDirty(void);
    void RegisterToLoader(MibViewLoader *loader); 

protected slots:
    void ExpandNode( QTreeWidgetItem * item);
    void CollapseNode( QTreeWidgetItem * item);
    void ExpandFromNode(void);
    void CollapseFromNode(void);
    virtual void SelectedNode( QTreeWidgetItem * item, QTreeWidgetItem * old);

signals:
    void SelectedOid(const QString& oid);

protected:
    virtual void contextMenuEvent ( QContextMenuEvent *event);
    QAction *expandAct;
    QAction *collapseAct;
    
private:
    int isdirty;
    MibViewLoader *MibLoader;
};

class MibView : public BasicMibView
{    
    Q_OBJECT
    
public:
    MibView ( QWidget * parent = 0 );
    
protected slots:
    void SelectedNode( QTreeWidgetItem * item, QTreeWidgetItem * old);
    void WalkFromNode(void);
    void GetFromNode(void);
    void GetNextFromNode(void);
    void SetFromNode(void);
    void StopFromNode(void);
    void TableViewFromNode(void);
    
signals:
    void NodeProperties(const QString& text);
    void WalkFromOid(const QString& oid);
    void GetFromOid(const QString& oid);
    void GetNextFromOid(const QString& oid);
    void SetFromOid(const QString& oid);
    void StopFromOid(const QString& oid);
    void TableViewFromOid(const QString& oid);
    
protected:
    void contextMenuEvent ( QContextMenuEvent *event);

private:
    QAction *walkAct;
    QAction *getAct;
    QAction *getnextAct;
    QAction *setAct;
    QAction *stopAct;
    QAction *tableviewAct;
};

class MibViewLoader: public QObject
{
    Q_OBJECT

public:
    MibViewLoader();
    void Load (QStringList &);
    MibNode *PopulateSubTree (SmiNode *smiNode, MibNode *parent, MibNode *sibling);    
    void RegisterView(BasicMibView* view);

signals:
    void LogError(const QString& text);
    
private:
    enum MibNode::MibType SmiKindToMibNodeType(int smikind);
    int PruneSubTree(SmiNode *smiNode);
    int IsPartOfLoadedModules(SmiNode *smiNode);
    
    int pmodc;
    SmiModule **pmodv;
    int ignoreconformance;
    int ignoreleafs;
    
    QList<BasicMibView*> views;
};

#endif /* MIBVIEW_H */
