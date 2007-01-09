#ifndef AGENT_H
#define AGENT_H

#include <qtimer.h>

#include "snmpb.h"
#include "mibview.h"
#include "trap.h"
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
    
    // Used by graph update timer
    unsigned long GetSyncValue(const QString& oid);

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
