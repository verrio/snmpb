#include <qmessagebox.h>

#include "discovery.h"
#include "agent.h"
#include "snmp_pp/snmp_pp.h"

Discovery::Discovery(Snmpb *snmpb)
{
    s = snmpb;

    connect( s->MainUI()->DiscoveryButton, 
             SIGNAL( clicked() ), this, SLOT( Discover() ));

    // Create the discovery thread (not started)
    dt = new DiscoveryThread(s);

    connect( dt, SIGNAL( SendAgent(QString) ), 
             this, SLOT( DisplayAgent(QString) ));
    connect( dt, SIGNAL( SignalStartStop(int) ), 
             this, SLOT( StartStop(int) ));
    connect( dt, SIGNAL( SignalProgress(int) ), 
             this, SLOT( DisplayProgress(int) ));
}

void DiscoveryThread::run(void)
{
    int status;

    // Create our SNMP session object
    Snmp *snmp = new Snmp(status);

    if (status != SNMP_CLASS_SUCCESS)
        return;

    emit SignalStartStop(1);

    UdpAddressCollection a;
    UdpAddress bc("255.255.255.255/161");
    OctetStr comm("errorcomm");

    num_proto = 0;

    for (int i = 0; i < 3; i++)
    {
        a.clear();

        switch(i)
        {
            case 0:
                if (s->MainUI()->DiscoveryV1->isChecked())
                {
                    snmp->broadcast_discovery(a, 3, bc, version1);
                    break;
                }
                else
                    continue;
            case 1:
                if (s->MainUI()->DiscoveryV2c->isChecked())
                {
                    snmp->broadcast_discovery(a, 3, bc, version2c);
                    break;
                }
                else
                    continue;
            case 2:
                if (s->MainUI()->DiscoveryV3->isChecked())
                {
                    snmp->broadcast_discovery(a, 3, bc, version3);
                    break;
                }
                else
                    continue;
            default:
                break;
        }

        num_proto++;
        emit SignalProgress(num_proto);

        for (int j = 0; j < a.size(); j++)
            emit SendAgent(QString(a[j].get_printable()));
    }

    emit SignalStartStop(0);
}

void Discovery::DisplayAgent(QString address)
{
    QTreeWidgetItem *val = 
        new QTreeWidgetItem(s->MainUI()->DiscoveryOutput,
                            QStringList(address));
    s->MainUI()->DiscoveryOutput->addTopLevelItem(val);
}

void Discovery::StartStop(int isstart)
{
    if (isstart)
        s->MainUI()->DiscoveryButton->setEnabled(false);
    else
        s->MainUI()->DiscoveryButton->setEnabled(true);
}

void Discovery::DisplayProgress(int value)
{
    s->MainUI()->DiscoveryProgress->setValue(value);
}

void Discovery::Discover(void)
{
    dt->num_proto = 0;

    if (s->MainUI()->DiscoveryV1->isChecked()) dt->num_proto++;
    if (s->MainUI()->DiscoveryV2c->isChecked()) dt->num_proto++;
    if (s->MainUI()->DiscoveryV3->isChecked()) dt->num_proto++;

    if (dt->num_proto < 1)
        return;

    s->MainUI()->DiscoveryProgress->reset();
    s->MainUI()->DiscoveryOutput->clear();
    s->MainUI()->DiscoveryProgress->setRange(0, dt->num_proto);

    if (s->MainUI()->DiscoveryLocal->isChecked())
        dt->start();
}

