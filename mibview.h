#ifndef MIBVIEW_H
#define MIBVIEW_H

#include <qlistview.h>
#include <qscrollview.h>
#include <qstring.h>
#include <qfile.h>
#include <qfileinfo.h>
#include <qtimer.h>
#include <qpixmap.h>
#include <qheader.h> 

#include "mibnode.h"
#include "smi.h"

class MibView : public QListView
{
    Q_OBJECT
    
public:
    MibView ( QWidget * parent = 0, const char * name = 0, WFlags f = 0 );
    void Load (QStrList &);
    void Populate (void);

protected slots:
    void ExpandNode( QListViewItem * item);
    void CollapseNode( QListViewItem * item);
    void SelectedNode( QListViewItem * item);
    void ExpandFromNode(void);
    void CollapseFromNode(void);
    void WalkFromNode(void);
    void GetFromNode(void);
    void GetNextFromNode(void);
    void SetFromNode(void);
    void StopFromNode(void);
    
signals:
    void NodeProperties(const QString& text);
    void WalkFromOid(const QString& oid);
    void GetFromOid(const QString& oid);
    void GetNextFromOid(const QString& oid);
    void SetFromOid(const QString& oid);
    void StopFromOid(const QString& oid);
    
protected:
    void contextMenuEvent ( QContextMenuEvent *);     
    
private:
    MibNode *PopulateSubTree (SmiNode *smiNode, MibNode *parent, MibNode *sibling);    
    enum MibNode::MibType SmiKindToMibNodeType(int smikind);
    int PruneSubTree(SmiNode *smiNode);
    int IsPartOfLoadedModules(SmiNode *smiNode);
    
    int pmodc;
    SmiModule **pmodv;
    int ignoreconformance;
    int ignoreleafs;
    int isdirty;
};

#endif /* MIBVIEW_H */
