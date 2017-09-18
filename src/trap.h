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

#ifndef TRAP_H
#define TRAP_H

#include "snmpb.h"
#include "agentprofile.h"
#include "snmp_pp/snmp_pp.h"

enum TrapType
{
    TRAPTYPE_INFO,
    TRAPTYPE_SUCCESS,
    TRAPTYPE_WARNING,
    TRAPTYPE_ERROR
};

class TrapItem : public QTreeWidgetItem
{
public:
    TrapItem(Oid &id, QTreeWidget* parent, const QStringList &values,
             QString community, QString seclevel,
             QString ctxname, QString ctxid, QString msgid, bool expand);
    ~TrapItem(void);

    void PrintProperties(QString& text);
    void PrintContent(QTreeWidget* TrapContent);
    void AddVarBind(Vb& vb);
    
private:
    static const std::string failure_indicators[4];
    static const std::string warning_indicators[6];
    static const std::string success_indicators[6];
    static enum TrapType GetTrapType(const QString &name);
    void setIcon(void);

    Oid oid;
    enum TrapType _type;
    QString _community;
    QString _seclevel;
    QString _ctxname;
    QString _ctxid;
    QString _msgid;
    bool _expand;

    QList<Vb*> content;
};

class Trap: public QObject
{
    Q_OBJECT
    
public:
    Trap(Snmpb *snmpb);
    TrapItem* Add(Oid &id, const QStringList &values, 
                  QString &community, QString &seclevel,
                  QString &ctxname, QString &ctxid, QString &msgid);
    int GetNextId(void);
    bool FilterTrap(IpAddress &address);
    
protected slots:
    void SelectedTrap( QTreeWidgetItem * item, QTreeWidgetItem * old);
    void ShowAgentSettings(void);
    void AgentFilterListChange(void);
    void Clear(void);
    
signals:
    void TrapProperties(const QString& text);
    
private:
    Snmpb *s;
    std::atomic<int> _trap_count;
};

#endif /* TRAP_H */
