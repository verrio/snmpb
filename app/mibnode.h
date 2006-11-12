#ifndef MIBNODE_H
#define MIBNODE_H

#include <q3listview.h>
#include <q3stylesheet.h>
#include <qstring.h>
#include <qfile.h>
#include <qfileinfo.h>
#include <qtimer.h>
#include <qpixmap.h>
#include <qtreewidget.h>

#include "smi.h"

class MibNode : public QTreeWidgetItem
{
public:
    enum MibType 
    {
        MIBNODE_NODE,
        MIBNODE_SCALAR,
        MIBNODE_TABLE,
        MIBNODE_ROW,
        MIBNODE_COLUMN,
        MIBNODE_NOTIFICATION,
        MIBNODE_GROUP,
        MIBNODE_COMPLIANCE,
        MIBNODE_CAPABILITIES
    };
    
    MibNode(enum MibType mibtype, SmiNode* node, MibNode* parent, MibNode * sibling = NULL);
    MibNode(QString label, QTreeWidget * parent);

    void SetPixmap(bool isOpened);
    void PrintProperties(QString& text);
    char *GetOid();
    
protected:
    char *GetAccess(void);
    char *GetStatus(void);
    char *GetTypeName(void);
    char *GetKindName(void);
    char *GetSmiTypeName(void);
                    
private:
    enum MibType Type;
    SmiNode *Node;
};

#endif /* MIBNODE_H */
