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
#ifndef AGENT_H
#define AGENT_H

#include <qtimer.h>

#include "snmpb.h"
#include "mibview.h"
#include "trap.h"
#include "agentprofile.h"
#include "snmp_pp/snmp_pp.h"

class Agent: public QObject
{
    Q_OBJECT
    
public:
    Agent(Snmpb *snmpb);
    void BindTrapPort(int Port);
    void Init(void);
    void AsyncCallback(int reason, Pdu &pdu, 
                       SnmpTarget &target, int iswalk);
    void AsyncCallbackTrap(int reason, Pdu &pdu, SnmpTarget &target);
    void AsyncCallbackSet(int reason, Pdu &pdu, SnmpTarget &target);
    
    static char *GetPrintableValue(SmiNode *node, Vb *vb);
    static void ConfigTargetFromSettings(snmp_version v,
                                         SnmpTarget *t, AgentProfile *ap);
    static Oid ConfigPduFromSettings(snmp_version v, const QString& oid, 
                                     Pdu *p, AgentProfile *ap);
    
    // Used by graph update timer
    unsigned long GetSyncValue(const QString& oid);

    inline USM *GetUSMObj(void) { return v3mp->get_usm(); };

protected:
    int Setup(const QString& oid, SnmpTarget **t, Pdu **p);
    int SelectTableInstance(const QString& oid);

public slots:
    void WalkFrom(const QString& oid);
    void Get(const QString& oid);
    void GetNext(const QString& oid);
    void GetBulk(const QString& oid);
    void GetFrom(const QString& oid, int op);
    void GetFromPromptInstance(const QString& oid, int op);
    void GetFromSelectInstance(const QString& oid, int op);
    void SetFrom(const QString& oid);
    void Stop(void);
    void TableViewFrom(const QString& oid);
    void GetSelectedTableInstance(QListWidgetItem * item);
    void VarbindsFrom(const QString& oid);
    void GetTypedTableInstance(void);
    void GetTypedSetValueCb(int index);
    void GetTypedSetValueLe(void);
    void GetTypedSetValueOidLe(void);

protected slots:
    void TimerExpired(void);    
    void ShowAgentSettings(void);
    void SelectAgentProfile(int prefproto = -1);
    void AgentProfileListChange(void);
    void VarbindsMoveUp(void);
    void VarbindsMoveDown(void);

signals:
    void TableInstanceSelected(int r);
    void StartWalk(bool);

private:
    Snmpb *s;
    
    Snmp *snmp;
    v3MP *v3mp;
    QTimer timer;
    
    int requests;
    int objects;
    QString msg;
    Oid theoid;

    QComboBox *cb;
    QLineEdit *le;
    QLineEdit *oidle;
    QString tinstresult;
    QString setresult_string;
    int setresult_int;
    QString oid_to_set;

    bool stop;
};

#endif /* AGENT_H */
