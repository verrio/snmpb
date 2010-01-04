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
#include "mibselection.h"
#include "agentprofile.h"
#include "ui_varbinds.h"
#include "snmp_pp/snmp_pp.h"

class Agent: public QObject
{
    Q_OBJECT
    
public:
    Agent(Snmpb *snmpb);
    bool BindTrapPort(int Port, QString &Err);
    void StartTrapTimer(void);
    void Init(void);
    void AsyncCallback(int reason, Pdu &pdu, 
                       SnmpTarget &target, int iswalk);
    void AsyncCallbackTrap(int reason, Pdu &pdu, SnmpTarget &target);
    void AsyncCallbackSet(int reason, Pdu &pdu, SnmpTarget &target);
    
    static char *GetPrintableValue(SmiNode *node, Vb *vb);
    void ConfigTargetFromSettings(snmp_version v,
                                  SnmpTarget *t, AgentProfile *ap);
    Oid ConfigPduFromSettings(snmp_version v, const QString& oid, 
                              Pdu *p, AgentProfile *ap, bool usevblist = false);
    
    // Used by graph update timer
    unsigned long GetSyncValue(const QString& oid);

    inline USM *GetUSMObj(void) { return v3mp->get_usm(); };

    int SelectTableInstance(const QString& oid, QString& outinstance);

    static SmiNode* GetNodeFromOid(Oid &oid);

protected:
    int Setup(const QString& oid, SnmpTarget **t, Pdu **p, bool usevblist = false);

private:
    QString GetValueString(MibSelection &ms, Vb* vb);
    void VarbindsBuildList(void);

public slots:
    void WalkFrom(const QString& oid);
    void Get(const QString& oid, bool usevblist = false);
    void GetNext(const QString& oid, bool usevblist = false);
    void GetBulk(const QString& oid, bool usevblist = false);
    void GetFrom(const QString& oid, int op);
    void GetFromPromptInstance(const QString& oid, int op);
    void GetFromSelectInstance(const QString& oid, int op);
    void Set(const QString& oid, bool usevblist = false);
    void SetFrom(const QString& oid);
    void Stop(void);
    void TableViewFrom(const QString& oid);
    void Varbinds(void);
    void VarbindsFrom(const QString& oid);
    void GetTypedTableInstance(void);

protected slots:
    void TimerExpired(void);    
    void ShowAgentSettings(void);
    void SelectAgentProfile(QString *prefprofile = NULL, int prefproto = -1);
    void SelectAgentProto(void);
    void AgentProfileListChange(void);
    void VarbindsNew(void);
    void VarbindsEdit(void);
    void VarbindsDelete(void);
    void VarbindsDeleteAll(void);
    void VarbindsMoveUp(void);
    void VarbindsMoveDown(void);
    void VarbindsQuit(void);
    void VarbindsGet(void);
    void VarbindsGetNext(void);
    void VarbindsGetBulk(void);
    void VarbindsSet(void);
    void VarbindsSelected(void);

signals:
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

    QLineEdit *le;
    QString tinstresult;
 
    bool stop;

    QVector<Vb> vblist;

    Ui_Varbinds *vbui;
    QDialog *vbd;
};

#endif /* AGENT_H */
