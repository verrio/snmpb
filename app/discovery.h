#ifndef DISCOVERY_H
#define DISCOVERY_H

#include <qthread.h>
#include "snmpb.h"

class DiscoveryThread: public QThread
{
    Q_OBJECT

public:
    DiscoveryThread(QObject *parent):QThread(parent) { s = (Snmpb*)parent; };
    void run();

public:
    int num_proto;

signals:
    void SendAgent(QString address);

protected:
    Snmpb *s;
};

class Discovery: public QObject
{
    Q_OBJECT
    
public:
    Discovery(Snmpb *snmpb);
    
protected slots:
    void Discover(void);
    void DisplayAgent(QString address);

private:
    Snmpb *s;
    DiscoveryThread *dt;
};

#endif /* DISCOVERY_H */

