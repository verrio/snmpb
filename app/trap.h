/*
    Copyright (C) 2004-2010  Martin Jolicoeur (snmpb1@gmail.com) 

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
#ifndef TRAP_H
#define TRAP_H

#include "snmpb.h"
#include "snmp_pp/snmp_pp.h"

class TrapItem : public QTreeWidgetItem
{
public:
    TrapItem(Oid &id, QTreeWidget* parent, const QStringList &values,
             QString community, QString seclevel,
             QString ctxname, QString ctxid, QString msgid);

    void PrintProperties(QString& text);
    void PrintContent(QTreeWidget* TrapContent);
    void AddVarBind(Vb& vb);
    
private:
    Oid oid;
    QString _community;
    QString _seclevel;
    QString _ctxname;
    QString _ctxid;
    QString _msgid;

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
    
protected slots:
    void SelectedTrap( QTreeWidgetItem * item, QTreeWidgetItem * old);
    
signals:
    void TrapProperties(const QString& text);
    
private:
    Snmpb *s;
};

#endif /* TRAP_H */
