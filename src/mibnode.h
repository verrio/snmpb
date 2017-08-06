/**
 * Copyright (c) 2004-2017 Martin Jolicoeur and contributors
 *
 * This file is part of the snmpb project and is free software: you can
 * redistribute it and/or modify it under the terms of the GNU General
 * Public License as published by the Free Software Foundation, either
 * version 3 of the License, or (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <http://www.gnu.org/licenses/>.
 */

#ifndef MIBNODE_H
#define MIBNODE_H

#include <smi.h>
#include <qstring.h>
#include <qfile.h>
#include <qfileinfo.h>
#include <qtimer.h>
#include <qpixmap.h>
#include <qtreewidget.h>

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
    enum MibNode::MibType GetKind(void) { return Type; };
    
protected:
    char *GetAccess(void);
    char *GetStatus(void);
    char *GetTypeName(void);
    char *GetBaseTypeName(void);
    char *GetKindName(void);
    char *GetSmiTypeName(void);
    QString GetRowIndex(SmiNode *smiNode);
    QString GetSizeRange(void);
    QString GetValueList(void);
                    
private:
    enum MibType Type;
    SmiNode *Node;
};

#endif /* MIBNODE_H */
