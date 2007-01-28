#ifndef DISCOVERY_H
#define DISCOVERY_H

#include <qthread.h>
#include "snmpb.h"
#include "snmp_pp/snmp_pp.h"

class DiscoveryThread: public QThread
{
    Q_OBJECT

public:
    DiscoveryThread(QObject *parent);
    void run();

public:
    int num_proto;
    int num_addresses;

signals:
    void SendAgent(QStringList agent_info);
    void SignalStartStop(int isstart);
    void SignalProgress(int value);

protected:
    void QueryAgentInfo(UdpAddress a, snmp_version v);

    Snmpb *s;
    Snmp *snmp;
    int status;
};

class Discovery: public QObject
{
    Q_OBJECT
    
public:
    Discovery(Snmpb *snmpb);
    
protected slots:
    void Discover(void);
    void DisplayAgent(QStringList agent_info);
    void StartStop(int isstart);
    void DisplayProgress(int value);

private:
    Snmpb *s;
    DiscoveryThread *dt;
};

#endif /* DISCOVERY_H */

