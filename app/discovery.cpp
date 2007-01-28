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

DiscoveryThread::DiscoveryThread(QObject *parent):QThread(parent)
{
    s = (Snmpb*)parent;

    // Create our SNMP session object
    snmp = new Snmp(status);
};

void DiscoveryThread::QueryAgentInfo(UdpAddress a, snmp_version v)
{
    QStringList agent_info;
    QString name;
    QString address;
    QString protocol;
    QString uptime;
    QString contact;
    QString location;
    QString description;

    SnmpTarget *target;
    UTarget utarget;
    CTarget ctarget;
    Pdu pdu;
    Vb vb;

    char * info_oids[] = {
        "1.3.6.1.2.1.1.1.0", 
        "1.3.6.1.2.1.1.3.0", 
        "1.3.6.1.2.1.1.4.0", 
        "1.3.6.1.2.1.1.5.0", 
        "1.3.6.1.2.1.1.6.0"};

    // Setup the target object
    if (v == version3)
        target = &utarget;
    else
        target = &ctarget;

    Agent::ConfigTargetFromSettings(v, s, target);
    target->set_address(a);
    Agent::ConfigPduFromSettings(v, s, info_oids[0], &pdu);
    for (int k = 1; k < 5; k++)
    { 
        vb.set_oid(Oid(info_oids[k]));
        pdu += vb;
    }

    // Now do a sync get
    if (snmp->get(pdu, *target) == SNMP_CLASS_SUCCESS)
    {
        for (int k = 0; k < 5; k++)
        {
            pdu.get_vb(vb, k);
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
    }
    else
        return;

    address = a.get_printable();
    protocol = (v == version3)?"SNMPv3": ((v == version2c)?"SNMPv2c":"SNMPv1");

    agent_info.clear();
    agent_info << name << address << protocol << uptime 
               << contact << location << description;

    emit SendAgent(agent_info);
}

void DiscoveryThread::run(void)
{
    snmp_version cur_version  = version1;

    if (status != SNMP_CLASS_SUCCESS)
        return;

    emit SignalStartStop(1);

    if (s->MainUI()->DiscoveryLocal->isChecked())
    {
        UdpAddressCollection a;
        UdpAddress bc("255.255.255.255/161");

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

            emit SignalProgress(i+1);

            for (int j = 0; j < a.size(); j++)
                QueryAgentInfo(a[j], cur_version);
        }
    }
    else
    {
        QString tmp_addr(s->MainUI()->DiscoveryFrom->text() + "/161");
        UdpAddress cur_address(tmp_addr.toLatin1().data());
        UdpAddress start_address = cur_address;

        for (int i = 0; i < 3; i++)
        {
            switch(i)
            {
                case 0:
                    if (s->MainUI()->DiscoveryV1->isChecked())
                    {
                        cur_version  = version1;
                        break;
                    }
                    else
                        continue;
                case 1:
                    if (s->MainUI()->DiscoveryV2c->isChecked())
                    {
                        cur_version  = version2c;
                        break;
                    }
                    else
                        continue;
                case 2:
                    if (s->MainUI()->DiscoveryV3->isChecked())
                    {
                        cur_version  = version3;
                        break;
                    }
                    else
                        continue;
                default:
                    break;
            }

            cur_address = start_address;
            for(int j = 0; j < num_addresses; j++)
            {
                QueryAgentInfo(cur_address, cur_version);
                emit SignalProgress((i*num_addresses)+(j+1));
                cur_address[3] = cur_address[3]++;
            }

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
    dt->num_addresses = 0;

    if (s->MainUI()->DiscoveryV1->isChecked()) dt->num_proto++;
    if (s->MainUI()->DiscoveryV2c->isChecked()) dt->num_proto++;
    if (s->MainUI()->DiscoveryV3->isChecked()) dt->num_proto++;

    if (dt->num_proto < 1)
        return;

    s->MainUI()->DiscoveryProgress->reset();
    s->MainUI()->DiscoveryOutput->clear();

    if (s->MainUI()->DiscoveryLocal->isChecked())
    {
        s->MainUI()->DiscoveryProgress->setRange(0, dt->num_proto);
        dt->start();
    }
    else
    {
        IpAddress addr_from(s->MainUI()->DiscoveryFrom->text().toLatin1().data());
        IpAddress addr_to(s->MainUI()->DiscoveryTo->text().toLatin1().data());

        // From must a valid IPv4 address
        if (!addr_from.valid() || 
            (addr_from.get_ip_version() != Address::version_ipv4) || 
            (addr_from[0] == 0))
        {
            QString err = QString("Invalid Address or DNS Name: %1")
                                  .arg(s->MainUI()->DiscoveryFrom->text());
            QMessageBox::critical ( NULL, "From address", err, 
                                    QMessageBox::Ok, Qt::NoButton);
            return;
        }

        // To must a valid IPv4 address
        if (!addr_to.valid() || 
            (addr_to.get_ip_version() != Address::version_ipv4) || 
            (addr_to[0] == 0))
        {
            QString err = QString("Invalid Address or DNS Name: %1")
                                  .arg(s->MainUI()->DiscoveryTo->text());
            QMessageBox::critical ( NULL, "To address", err, 
                                    QMessageBox::Ok, Qt::NoButton);
            return;
        }

        // Must have a valid address range
        if (addr_from > addr_to)
        {
            QString err = QString("'To address' must be greater than 'From address'");
            QMessageBox::critical ( NULL, "Invalid address range", err, 
                                    QMessageBox::Ok, Qt::NoButton);
            return;
        }

        // Address must be in the same /24 subnet (limitation for now)
        if ((addr_to[0] != addr_from[0]) ||
            (addr_to[1] != addr_from[1]) ||
            (addr_to[2] != addr_from[2]))
        {
            QString err = QString("'To address' must be in same /24 subnet than 'From address'");
            QMessageBox::critical ( NULL, "Invalid address range", err, 
                                    QMessageBox::Ok, Qt::NoButton);
            return;
        }

        dt->num_addresses = addr_to[3] - addr_from[3] + 1;

        s->MainUI()->DiscoveryProgress->setRange(0, dt->num_proto*dt->num_addresses);

        dt->start();
    }
}

