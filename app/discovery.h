#ifndef DISCOVERY_H
#define DISCOVERY_H

#include <qthread.h>
#include "snmpb.h"
#include "snmp_pp/snmp_pp.h"

class DiscoveryThread;

class DiscoverySnmp: public Snmp
{
public:
    DiscoverySnmp(int &status):Snmp(status) {};

    void discover(const UdpAddress &start_addr, int num_addr,
                  const int timeout_sec, const snmp_version version,
                  DiscoveryThread* thread);
};

class DiscoveryThread: public QThread
{
    Q_OBJECT

public:
    DiscoveryThread(QObject *parent);
    void run();
    void SendAgentInfo(Pdu pdu, UdpAddress a, snmp_version v);
    void Progress(void);

public:
    int num_proto;
    int num_addresses;

signals:
    void SendAgent(QStringList agent_info);
    void SignalStartStop(int isstart);
    void SignalProgress(int value);

protected:
    Snmpb *s;
    DiscoverySnmp *snmp;
    int status;
    int current_progress;
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

