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
                  QString readcomm, QString secname, int seclevel, 
                  QString cxtname, QString ctxengineid, bool use_snmpv3_probe,
                  DiscoveryThread* thread);

    bool aborting;
};

class DiscoveryThread: public QThread
{
    Q_OBJECT

public:
    DiscoveryThread(QObject *parent);
    void run();
    void SendAgentInfo(Pdu pdu, UdpAddress a, snmp_version v);
    void Progress(void);
    void Abort(void);

public:
    int num_proto;
    int num_addresses;
    int wait_time;

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
    void Abort(void);
    void DisplayAgent(QStringList agent_info);
    void StartStop(int isstart);
    void DisplayProgress(int value);
    void ShowAgentSettings(void);
    void AgentProfileListChange(void);

private:
    Snmpb *s;
    DiscoveryThread *dt;
};

#endif /* DISCOVERY_H */

