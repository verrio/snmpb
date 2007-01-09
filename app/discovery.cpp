#include "discovery.h"
#include "agent.h"
#include "snmp_pp/snmp_pp.h"

Discovery::Discovery(Snmpb *snmpb)
{
    s = snmpb;

    connect( s->MainUI()->DiscoveryButton, 
             SIGNAL( clicked() ), this, SLOT( Discover() ));
}

void Discovery::Discover(void)
{
    int num_proto = 0;

    if (s->MainUI()->DiscoveryV1->isChecked()) num_proto++;
    if (s->MainUI()->DiscoveryV2c->isChecked()) num_proto++;
    if (s->MainUI()->DiscoveryV3->isChecked()) num_proto++;

    if (num_proto < 1)
        return;

    s->MainUI()->DiscoveryProgress->reset();
    s->MainUI()->DiscoveryOutput->clear();
    s->MainUI()->DiscoveryProgress->setRange(1, num_proto);

    if (s->MainUI()->DiscoveryLocal->isChecked())
    { 
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
                        s->AgentObj()->GetSnmpObj()->broadcast_discovery(a, 3, bc, version1);
                        break;
                    }
                    else
                        continue;
                case 1:
                    if (s->MainUI()->DiscoveryV2c->isChecked())
                    {
                        s->AgentObj()->GetSnmpObj()->broadcast_discovery(a, 3, bc, version2c);
                        break;
                    }
                    else
                        continue;
                case 2:
                    if (s->MainUI()->DiscoveryV3->isChecked())
                    {
                        s->AgentObj()->GetSnmpObj()->broadcast_discovery(a, 3, bc, version3);
                        break;
                    }
                    else
                        continue;
                default:
                    break;
            }

            num_proto++;
            s->MainUI()->DiscoveryProgress->setValue(num_proto);

            for (int j = 0; j < a.size(); j++)
            {
                QTreeWidgetItem *val = 
                    new QTreeWidgetItem(s->MainUI()->DiscoveryOutput,
                            QStringList(a[j].get_printable()));
                s->MainUI()->DiscoveryOutput->addTopLevelItem(val);
            }
        }
    }
}

