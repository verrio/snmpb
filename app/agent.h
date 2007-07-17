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
    void AsyncCallback(int reason, Pdu &pdu, 
                       SnmpTarget &target, int iswalk);
    void AsyncCallbackTrap(int reason, Pdu &pdu, SnmpTarget &target);
    
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

public slots:
    void WalkFrom(const QString& oid);
    void GetFrom(const QString& oid);
    void GetNextFrom(const QString& oid);
    void SetFrom(const QString& oid);
    void StopFrom(const QString& oid);
    void TableViewFrom(const QString& oid);

protected slots:
    void TimerExpired(void);    
    void ShowAgentSettings(void);
    void SelectAgentProfile(int prefproto = -1);
    void AgentProfileListChange(void);

private:
    Snmpb *s;
    
    Snmp *snmp;
    v3MP *v3mp;
    QTimer timer;
    
    int requests;
    int objects;
    QString msg;
    Oid theoid;
};

#endif /* AGENT_H */
