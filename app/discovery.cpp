/*
    Copyright (C) 2004-2010  Martin Jolicoeur (snmpb1@gmail.com) 

    This file is part of the SnmpB project 
    (http://sourceforge.net/projects/snmpb)

    This program is free software: you can redistribute it and/or modify
    it under the terms of the GNU General Public License as published by
    the Free Software Foundation, either version 2 of the License, or
    (at your option) any later version.

    This program is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
    GNU General Public License for more details.

    You should have received a copy of the GNU General Public License
    along with this program.  If not, see <http://www.gnu.org/licenses/>.
*/
#include <qmessagebox.h>
#include <QContextMenuEvent>

#include "discovery.h"
#include "preferences.h"
#include "agent.h"
#include "snmp_pp/snmp_pp.h"
#include "snmp_pp/snmpmsg.h"

#define DISC_SNMP_V1  "V1"
#define DISC_SNMP_V2C "V2c"
#define DISC_SNMP_V3  "V3"

/* Internal SNMP++ routine */
extern int receive_snmp_response(SnmpSocket sock, Snmp &snmp_session,
                                 Pdu &pdu, UdpAddress &fromaddress,
                                 OctetStr &engine_id, bool process_msg = true);

static unsigned char snmpv3_broadcast_message[] =
{
    0x30, 0x3a,
    0x02, 0x01, 0x03,             // Version: 3
    0x30, 0x0f,                   // global header length 15
    0x02, 0x03, 0x01, 0x00, 0x00, // message id
    0x02, 0x02, 0x10, 0x00,       // message max size
    0x04, 0x01, 0x04,             // flags (reportable set)
    0x02, 0x01, 0x03,             // security model USM
    0x04, 0x10,                   // security params
    0x30, 0x0e,
    0x04, 0x00,                   // no engine id
    0x02, 0x01, 0x00,             // boots 0
    0x02, 0x01, 0x00,             // time 0
    0x04, 0x00,                   // no user name
    0x04, 0x00,                   // no auth par
    0x04, 0x00,                   // no priv par
    0x30, 0x12,
    0x04, 0x00,                   // no context engine id
    0x04, 0x00,                   // no context name
    0xa0, 0x0c,                   // GET PDU
    0x02, 0x02, 0x34, 0x26,       // request id
    0x02, 0x01, 0x00,             // error status no error
    0x02, 0x01, 0x00,             // error index 0
    0x30, 0x00                    // no data
};

static const char *info_oids[] =
{
    "1.3.6.1.2.1.1.1.0", // Description
    "1.3.6.1.2.1.1.3.0", // Uptime
    "1.3.6.1.2.1.1.4.0", // Contact
    "1.3.6.1.2.1.1.5.0", // Name
    "1.3.6.1.2.1.1.6.0"  // Location
};

Discovery::Discovery(Snmpb *snmpb)
{
    s = snmpb;

    QStringList columns;
    columns << "Name" << "Address/Port" << "Protocol" << "Up Time" 
            << "Contact Person" << "System Location" << "System Description";
    s->MainUI()->DiscoveryOutput->setHeaderLabels(columns);

    connect( s->MainUI()->DiscoveryButton, 
             SIGNAL( clicked() ), this, SLOT( Discover() ));
    connect( s->MainUI()->DiscoveryAbortButton, 
             SIGNAL( clicked() ), this, SLOT( Abort() ));
    connect( s->MainUI()->DiscoveryAgentSettings, 
             SIGNAL( clicked() ), this, SLOT( ShowAgentSettings() ));
    connect( s->APManagerObj(), SIGNAL( AgentProfileListChanged() ), 
             this, SLOT ( AgentProfileListChange() ) );

    // Fill-in the list of agent profiles from profiles manager
    AgentProfileListChange();

    // Create the discovery thread (not started)
    dt = new DiscoveryThread(s);

    connect( dt, SIGNAL( SendAgent(QStringList) ), 
             this, SLOT( DisplayAgent(QStringList) ));
    connect( dt, SIGNAL( SignalStartStop(int) ), 
             this, SLOT( StartStop(int) ));
    connect( dt, SIGNAL( SignalProgress(int) ), 
             this, SLOT( DisplayProgress(int) ));

    // Create context menu actions
    s->MainUI()->DiscoveryOutput->setContextMenuPolicy (Qt::CustomContextMenu);
    connect( s->MainUI()->DiscoveryOutput, 
             SIGNAL( customContextMenuRequested ( const QPoint & ) ),
             this, SLOT( ContextMenu ( const QPoint & ) ) );
    addAgentAct = new QAction(tr("&Add agent(s) to profile list"), this);
    connect(addAgentAct, SIGNAL(triggered()), this, SLOT(AddAgentToProfiles()));
}

void Discovery::ShowAgentSettings(void)
{
     s->APManagerObj()->SetSelectedAgent(s->MainUI()->DiscoveryAgentProfile->currentText()); 
     s->APManagerObj()->Execute();
}

void Discovery::AgentProfileListChange(void)
{
    QString cap = s->MainUI()->DiscoveryAgentProfile->currentText();
    s->MainUI()->DiscoveryAgentProfile->clear();
    s->MainUI()->DiscoveryAgentProfile->addItems(s->APManagerObj()->GetAgentsList());
    if (cap.isEmpty() == false)
    {
        int idx = s->MainUI()->DiscoveryAgentProfile->findText(cap);
        s->MainUI()->DiscoveryAgentProfile->setCurrentIndex(idx>0?idx:0);
    }
}

DiscoveryThread::DiscoveryThread(QObject *parent):QThread(parent)
{
    s = (Snmpb*)parent;

    // Create our SNMP session object
    bool v4 = s->PreferencesObj()->GetEnableIPv4();
    bool v6 = s->PreferencesObj()->GetEnableIPv6();

    if (v4 && v6)
        snmp = new DiscoverySnmp(status, UdpAddress("0.0.0.0"), UdpAddress("::"));
    else if (v4)
        snmp = new DiscoverySnmp(status, UdpAddress("0.0.0.0"));
    else if (v6)
        snmp = new DiscoverySnmp(status, UdpAddress("::"));
    else
        status = SNMP_CLASS_ERROR;

    snmp->aborting = false;
};

void DiscoveryThread::SendAgentInfo(Pdu pdu, UdpAddress a, snmp_version v)
{
    QStringList agent_info;
    QString name;
    QString address;
    QString protocol;
    QString uptime;
    QString contact;
    QString location;
    QString description;

    Vb vb;

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

    address = a.get_printable();
    protocol = (v == version3)?DISC_SNMP_V3: 
               ((v == version2c)?DISC_SNMP_V2C:DISC_SNMP_V1);

    agent_info.clear();
    agent_info << name << address << protocol << uptime 
               << contact << location << description;

    emit SendAgent(agent_info);
}

void DiscoveryThread::Progress(void)
{
    emit SignalProgress(++current_progress);
}

void DiscoveryThread::Abort(void)
{
    snmp->aborting = true;
}

DiscoverySnmp::DiscoverySnmp(int &status, const UdpAddress &addr)
    :Snmp(status, addr)
{
}

DiscoverySnmp::DiscoverySnmp(int &status, const UdpAddress& addr_v4, 
    const UdpAddress& addr_v6):Snmp(status, addr_v4, addr_v6)
{
}

void DiscoverySnmp::discover(const UdpAddress &start_addr, int num_addr,
                             const int timeout_sec, const snmp_version version,
                             QString readcomm, QString secname, int seclevel, 
                             QString ctxname, QString ctxengineid, 
                             bool use_snmpv3_probe, DiscoveryThread* thread)
{
    unsigned char *message = NULL;
    int message_length = 0;
    unsigned int sock = iv_snmp_session;
    SnmpMessage *snmpmsg = NULL;
    Pdu pdu;
    OctetStr get_community;

    // Prepare pdu
    if ((version != version3) || (use_snmpv3_probe == false))
    {
        Vb vb;

        for (int k = 0; k < 5; k++)
        { 
            vb.set_oid(Oid(info_oids[k]));
            pdu += vb;
        }

        pdu.set_error_index(0);            // set error index to none
        pdu.set_type(sNMP_PDU_GET);        // set pdu type

        if (version != version3)
        {
            get_community = readcomm.toLatin1().data();
        }
        else
        {
            // set the security level to use
            if (seclevel == 0/*"noAuthNoPriv"*/)
                pdu.set_security_level(SNMP_SECURITY_LEVEL_NOAUTH_NOPRIV);
            else if (seclevel == 1/*"authNoPriv"*/)
                pdu.set_security_level(SNMP_SECURITY_LEVEL_AUTH_NOPRIV);
            else
                pdu.set_security_level(SNMP_SECURITY_LEVEL_AUTH_PRIV);

            pdu.set_context_name(ctxname.toLatin1().data());
            pdu.set_context_engine_id(ctxengineid.toLatin1().data());
        }
    }

    // Send probe packets
    UdpAddress cur_address = start_addr;
    for(int j = 0; j < num_addr; j++)
    {
        UdpAddress uaddr(cur_address);

        if (uaddr.get_ip_version() == Address::version_ipv4)
        {
            if (iv_snmp_session != (int)INVALID_SOCKET)
                sock = iv_snmp_session;
            else
            {
                uaddr.map_to_ipv6();
                sock = iv_snmp_session_ipv6;
            }
        }
        else
        {
            sock = iv_snmp_session_ipv6;
        }

        if (version != version3)
        {
            pdu.set_request_id(MyMakeReqId()); // determine request id to use

            snmpmsg = new SnmpMessage();
            if (snmpmsg->load(pdu, get_community, version) != SNMP_CLASS_SUCCESS)
                goto next_addr;

            message        = snmpmsg->data();
            message_length = snmpmsg->len();
        }
        else
        {
            if (use_snmpv3_probe == true)
            {
                unsigned short v3reqid = MyMakeReqId();
                message = (unsigned char *)snmpv3_broadcast_message;
                message_length = sizeof(snmpv3_broadcast_message);
                message[50] = v3reqid >> 8;
                message[51] = v3reqid & 0xFF;
            }
            else
            {
                OctetStr engine_id;
                pdu.set_request_id(MyMakeReqId()); // determine request id to use
                v3MP::I->get_from_engine_id_table(engine_id, 
                                                  uaddr.get_printable());

                snmpmsg = new SnmpMessage();
                if (snmpmsg->loadv3( pdu, engine_id, secname.toLatin1().data(),
                                     SNMP_SECURITY_MODEL_USM, 
                                     version) != SNMP_CLASS_SUCCESS)
                    goto next_addr;

                message        = snmpmsg->data();
                message_length = snmpmsg->len();
            }
        }

        if (send_raw_data(message, message_length, uaddr) < 0)
        {
            if (snmpmsg) delete snmpmsg;
            return;
        }

next_addr:
        if (snmpmsg)
        {
            delete snmpmsg;
            snmpmsg = NULL;
        }

        cur_address[3]++;

        if (aborting == true)
            return;
    }

    // Now wait for the responses
    Pdu in_pdu;
    fd_set readfds;
    int nfound = 0;
    struct timeval fd_timeout;
    msec end_time;

    end_time += 1000;
    int num_sec = 1;

    lock();
    do
    {
new_loop:
        FD_ZERO(&readfds);
        FD_SET(sock, &readfds);

        end_time.GetDeltaFromNow(fd_timeout);

        nfound = select((int)(sock + 1), &readfds, NULL, NULL, &fd_timeout);

        if ((nfound > 0) && (FD_ISSET(sock, &readfds)))
        {
            // Received a message
            UdpAddress from;
            OctetStr engine_id;
            int res = receive_snmp_response(sock, *this, in_pdu, from, 
                                            engine_id, true);

            if((res == SNMPv3_MP_UNKNOWN_PDU_HANDLERS) || // SNMPv3
               (res == SNMP_CLASS_SUCCESS)) // SNMPv1 or SNMPv2c
                thread->SendAgentInfo(in_pdu, from, version);
        }

        if (aborting == true)
        {
            unlock();
            return;
        }

        // A second as elapsed, show progress
        if ((fd_timeout.tv_sec == 0) && (fd_timeout.tv_usec == 0))
        {
            thread->Progress();
            if (num_sec < timeout_sec)
            {
                end_time += 1000;
                num_sec++;
                goto new_loop;
            }
        }

    } while ((fd_timeout.tv_sec > 0) || (fd_timeout.tv_usec > 0));
    unlock();
}

void DiscoveryThread::run(void)
{
    snmp_version cur_version  = version1;
    AgentProfile *ap = s->APManagerObj()->GetAgentProfile
                        (s->MainUI()->DiscoveryAgentProfile->currentText());

    if (!ap)
        return;

    if (status != SNMP_CLASS_SUCCESS)
        return;

    emit SignalStartStop(1);

    bool v4 = s->PreferencesObj()->GetEnableIPv4();
    bool v6 = s->PreferencesObj()->GetEnableIPv6();

    current_progress = 0;

    for (int t = 0; t < 2; t++) // for all transports
    {
        QString address_str;

        if (s->MainUI()->DiscoveryLocal->isChecked())
        {
            if ((t == 0) && (v4 == true))
                address_str = "255.255.255.255/";
            else if ((t == 1) && (v6 == true))
                address_str = "ff02::1/";
            else 
                continue;
            address_str += ap->GetPort();
        }
        else
        {
            address_str = s->MainUI()->DiscoveryFrom->text() + "/" + ap->GetPort();

            Address::version_type v = 
                UdpAddress(address_str.toLatin1().data()).get_ip_version(); 
            if (!(((t == 0) && (v == Address::version_ipv4) && (v4 == true)) ||
                  ((t == 1) && (v == Address::version_ipv6) && (v6 == true))))
                continue;
        }

        UdpAddress start_address(address_str.toLatin1().data());

        for (int i = 0; i < 3; i++) // For all SNMP protocols
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

            snmp->aborting = false;
            snmp->discover(start_address, num_addresses, 
                    wait_time, cur_version, ap->GetReadComm(), 
                    ap->GetSecName(), ap->GetSecLevel(), 
                    ap->GetContextName(), ap->GetContextEngineID(), 
                    s->MainUI()->DiscoverySNMPv3Probe->isChecked(), this);
            if (snmp->aborting == true) goto discover_end;
        }
    }

discover_end:
    emit SignalStartStop(0);
}

void Discovery::DisplayAgent(QStringList agent_info)
{
    // Check if the agent already exists in the list
    QList<QTreeWidgetItem *> laddr = 
        s->MainUI()->DiscoveryOutput->findItems(agent_info[1], 
                                                Qt::MatchExactly, 1);

    // If it exists, add the new supported protocol to its list.
    if (!laddr.isEmpty())
    {
        if (!((agent_info[2] == DISC_SNMP_V1) && 
              strstr(laddr[0]->text(2).toLatin1().data(), DISC_SNMP_V1)) && 
            !((agent_info[2] == DISC_SNMP_V2C) && 
              strstr(laddr[0]->text(2).toLatin1().data(), DISC_SNMP_V2C)) && 
            !((agent_info[2] == DISC_SNMP_V3) && 
              strstr(laddr[0]->text(2).toLatin1().data(), DISC_SNMP_V3)))
        laddr[0]->setText(2,  laddr[0]->text(2) + "/" + agent_info[2]);
    }
    else
    {
        // Else add the new agent to the list, as is.
        QTreeWidgetItem *val = new QTreeWidgetItem(s->MainUI()->DiscoveryOutput,
                                                   agent_info);
        s->MainUI()->DiscoveryOutput->addTopLevelItem(val);
    }
}

void Discovery::StartStop(int isstart)
{
    if (isstart)
    {
        s->MainUI()->DiscoveryButton->setEnabled(false);
        s->MainUI()->DiscoveryAbortButton->setEnabled(true);
    }
    else
    {
        s->MainUI()->DiscoveryButton->setEnabled(true);
        s->MainUI()->DiscoveryAbortButton->setEnabled(false);
    }
}

void Discovery::DisplayProgress(int value)
{
    s->MainUI()->DiscoveryProgress->setValue(value);
}

void Discovery::Abort(void)
{
    dt->Abort();
}

void Discovery::ContextMenu ( const QPoint &pos )
{    
    QMenu menu(tr("Actions"), s->MainUI()->DiscoveryOutput);

    menu.addAction(addAgentAct);

    menu.exec(s->MainUI()->DiscoveryOutput->mapToGlobal(pos));
}

void Discovery::AddAgentToProfiles(void)
{
    QList<QTreeWidgetItem *> item_list = 
                             s->MainUI()->DiscoveryOutput->selectedItems();
    char buf[52]; // for IPv6 addr/port = 45+/+5+NULL

    for (int i = 0; i < item_list.size(); i++)
    {
        strcpy(buf, item_list[i]->text(1).toLatin1().data());
        QString address(strtok(buf, "/"));

        s->APManagerObj()->Add(item_list[i]->text(0).isEmpty()?
                               address:item_list[i]->text(0), 
                               address, 
                               QString(strstr(item_list[i]->text(1).toLatin1().data(), "/") + 1), 
                               strstr(item_list[i]->text(2).toLatin1().data(), 
                                      DISC_SNMP_V1)?true:false, 
                               strstr(item_list[i]->text(2).toLatin1().data(), 
                                      DISC_SNMP_V2C)?true:false, 
                               strstr(item_list[i]->text(2).toLatin1().data(), 
                                      DISC_SNMP_V3)?true:false, 
                               s->MainUI()->DiscoveryAgentProfile->currentText());
    }
}


void Discovery::Discover(void)
{
    int num_transport = 0;
    dt->num_proto = 0;
    dt->num_addresses = 0;

    if (s->MainUI()->DiscoveryV1->isChecked()) dt->num_proto++;
    if (s->MainUI()->DiscoveryV2c->isChecked()) dt->num_proto++;
    if (s->MainUI()->DiscoveryV3->isChecked()) dt->num_proto++;

    if (dt->num_proto < 1)
        return;

    dt->wait_time = s->MainUI()->DiscoveryWaitTime->value();

    s->MainUI()->DiscoveryProgress->reset();
    s->MainUI()->DiscoveryOutput->clear();

    if (s->MainUI()->DiscoveryLocal->isChecked())
    {
        if (s->PreferencesObj()->GetEnableIPv4()) num_transport++;
        if (s->PreferencesObj()->GetEnableIPv6()) num_transport++;
        dt->num_addresses = 1;
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

        num_transport = 1;
        dt->num_addresses = addr_to[3] - addr_from[3] + 1;
    }

    s->MainUI()->DiscoveryProgress->setRange(0, 
        num_transport*dt->num_proto*dt->wait_time);

    dt->start();
}

