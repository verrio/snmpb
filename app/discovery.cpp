#include <qmessagebox.h>

#include "discovery.h"
#include "agent.h"
#include "snmp_pp/snmp_pp.h"

Discovery::Discovery(Snmpb *snmpb)
{
    s = snmpb;

    QStringList columns;
    columns << "Name" << "Address/Port" << "Protocol" << "Up Time" 
            << "Contact Person" << "System Location" << "System Description";
    s->MainUI()->DiscoveryOutput->setHeaderLabels(columns);

    connect( s->MainUI()->DiscoveryButton, 
             SIGNAL( clicked() ), this, SLOT( Discover() ));

    // Create the discovery thread (not started)
    dt = new DiscoveryThread(s);

    connect( dt, SIGNAL( SendAgent(QStringList) ), 
             this, SLOT( DisplayAgent(QStringList) ));
    connect( dt, SIGNAL( SignalStartStop(int) ), 
             this, SLOT( StartStop(int) ));
    connect( dt, SIGNAL( SignalProgress(int) ), 
             this, SLOT( DisplayProgress(int) ));
}

void DiscoveryThread::run(void)
{
    int status;
    snmp_version cur_version  = version1;

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
                    cur_version  = version1;
                    break;
                }
                else
                    continue;
            case 1:
                if (s->MainUI()->DiscoveryV2c->isChecked())
                {
                    snmp->broadcast_discovery(a, 3, bc, version2c);
                    cur_version  = version2c;
                    break;
                }
                else
                    continue;
            case 2:
                if (s->MainUI()->DiscoveryV3->isChecked())
                {
                    snmp->broadcast_discovery(a, 3, bc, version3);
                    cur_version  = version3;
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
        {
            QStringList agent_info;
            QString name;
            QString address;
            QString protocol;
            QString uptime;
            QString contact;
            QString location;
            QString description;

            char * info_oids[] = {
                "1.3.6.1.2.1.1.1.0", 
                "1.3.6.1.2.1.1.3.0", 
                "1.3.6.1.2.1.1.4.0", 
                "1.3.6.1.2.1.1.5.0", 
                "1.3.6.1.2.1.1.6.0"};

            for (int k = 0; k < 5; k++)
            {
                // Initialize agent & pdu objects
                SnmpTarget *target;
                Pdu *pdu;
                Vb vb;
                if (s->AgentObj()->Setup(info_oids[k], &target, &pdu) >= 0)
                {
                    target->set_address(a[j]);
                    // Now do a sync get
                    if (snmp->get(*pdu, *target) == SNMP_CLASS_SUCCESS)
                    {
                        pdu->get_vb(vb, 0);
                        if (k == 1)
                        {
                            unsigned long time = 0;
                            vb.get_value(time);
                            TimeTicks ut(time);
                            uptime = ut.get_printable(); 
                        }
                        else
                        {
                            static unsigned char buf[5000];
                            unsigned long len;
                            vb.get_value(buf, len, 5000);
                            buf[len] = '\0';
                            switch(k)
                            {
                                case 0: description = (const char*)buf; break;
                                case 2: contact = (const char*)buf; break;
                                case 3: name = (const char*)buf; break;
                                case 4: location = (const char*)buf; break;
                                default: break;
                            }
                        }
                    }

                    delete target;
                    delete pdu;
                }
            }

            address = a[j].get_printable();
            protocol = (cur_version == version3)?"SNMPv3":
                      ((cur_version == version2c)?"SNMPv2c":"SNMPv1");

            agent_info << name << address << protocol << uptime 
                       << contact << location << description;

            emit SendAgent(agent_info);
        }
    }

    emit SignalStartStop(0);
}

void Discovery::DisplayAgent(QStringList agent_info)
{
    QTreeWidgetItem *val = new QTreeWidgetItem(s->MainUI()->DiscoveryOutput,
                                               agent_info);
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

