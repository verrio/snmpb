#include <qmessagebox.h>

#include "discovery.h"
#include "agent.h"
#include "snmp_pp/snmp_pp.h"
#include "snmp_pp/snmpmsg.h"

/* Internal SNMP++ routines */
extern int send_snmp_request(SnmpSocket sock, unsigned char *send_buf,
                             size_t send_len, Address &address);
extern int receive_snmp_response(SnmpSocket sock, Snmp &snmp_session,
                                 Pdu &pdu, UdpAddress &fromaddress,
                                 bool process_msg = true);


#define DISCOVERY_WAIT_TIME_PER_PROTO     3 /* In secs, should be configurable */

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
    snmp = new DiscoverySnmp(status);
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
    protocol = (v == version3)?"SNMPv3": ((v == version2c)?"SNMPv2c":"SNMPv1");

    agent_info.clear();
    agent_info << name << address << protocol << uptime 
               << contact << location << description;

    emit SendAgent(agent_info);
}

void DiscoveryThread::Progress(void)
{
    emit SignalProgress(++current_progress);
}

void DiscoverySnmp::discover(const UdpAddress &start_addr, int num_addr,
                             const int timeout_sec, const snmp_version version,
                             DiscoveryThread* thread)
{
    unsigned char *message;
    int message_length;
    SnmpSocket sock = iv_snmp_session;
    SnmpMessage snmpmsg;
    Pdu pdu;
    OctetStr get_community;

#ifdef _SNMPv3
    unsigned char snmpv3_broadcast_message[60] = {
        0x30, 0x3a,
        0x02, 0x01, 0x03,                   // Version: 3
        0x30, 0x0f,                         // global header length 15
        0x02, 0x03, 0x01, 0x00, 0x00, // message id
        0x02, 0x02, 0x10, 0x00,       // message max size
        0x04, 0x01, 0x04,             // flags (reportable set)
        0x02, 0x01, 0x03,             // security model USM
        0x04, 0x10,                         // security params
        0x30, 0x0e,
        0x04, 0x00,             // no engine id
        0x02, 0x01, 0x00,       // boots 0
        0x02, 0x01, 0x00,       // time 0
        0x04, 0x00,             // no user name
        0x04, 0x00,             // no auth par
        0x04, 0x00,             // no priv par
        0x30, 0x12,
        0x04, 0x00,                   // no context engine id
        0x04, 0x00,                   // no context name
        0xa0, 0x0c,                         // GET PDU
        0x02, 0x02, 0x34, 0x26,       // request id
        0x02, 0x01, 0x00,             // error status no error
        0x02, 0x01, 0x00,             // error index 0
        0x30, 0x00                    // no data
    };

    if (version == version3)
    {
        message = (unsigned char *)snmpv3_broadcast_message;
        message_length = 60;
    }
    else
#endif
    {
        Vb vb;

        char * info_oids[] = {
            "1.3.6.1.2.1.1.1.0", 
            "1.3.6.1.2.1.1.3.0", 
            "1.3.6.1.2.1.1.4.0", 
            "1.3.6.1.2.1.1.5.0", 
            "1.3.6.1.2.1.1.6.0"};

        for (int k = 0; k < 5; k++)
        { 
            vb.set_oid(Oid(info_oids[k]));
            pdu += vb;
        }

        pdu.set_error_index(0);            // set error index to none
        pdu.set_type(sNMP_PDU_GET);        // set pdu type
        get_community = "public";
    }

    lock();

    UdpAddress cur_address = start_addr;
    for(int j = 0; j < num_addr; j++)
    {
        UdpAddress uaddr(cur_address);

        if (uaddr.get_ip_version() == Address::version_ipv4)
        {
            if (iv_snmp_session != INVALID_SOCKET)
                sock = iv_snmp_session;
            else
            {
                uaddr.map_to_ipv6();
                sock = iv_snmp_session_ipv6;
            }
        }
        else
            sock = iv_snmp_session_ipv6;

        if (version != version3)
        {
            pdu.set_request_id(MyMakeReqId()); // determine request id to use

            int status = snmpmsg.load(pdu, get_community, version);
            if (status != SNMP_CLASS_SUCCESS)
            {
                debugprintf(0, "Error encoding broadcast pdu (%i).", status);
                unlock();
                return;
            }
            message        = snmpmsg.data();
            message_length = snmpmsg.len();
        }
#ifdef _SNMPv3
        else
        {
            unsigned short v3reqid = MyMakeReqId();
            message[50] = v3reqid >> 8;
            message[51] = v3reqid & 0xFF;
        }
#endif

        if (send_snmp_request(sock, message, message_length, uaddr) < 0)
        {
            debugprintf(0, "Error sending broadast.");
            unlock();
            return;
        }

        cur_address[3] = cur_address[3]++;
    }

    // now wait for the responses
    Pdu in_pdu;
    fd_set readfds;
    int nfound = 0;
    struct timeval fd_timeout;
    msec end_time;

    end_time += 1000;
    int num_sec = 1;

    do
    {
new_loop:
        FD_ZERO(&readfds);
        FD_SET(sock, &readfds);

        end_time.GetDeltaFromNow(fd_timeout);

        nfound = select((int)(sock + 1), &readfds, NULL, NULL, &fd_timeout);

        if ((nfound > 0) && (FD_ISSET(sock, &readfds)))
        {
            // receive message
            UdpAddress from;
            if (receive_snmp_response(sock, *this, in_pdu, from, true) 
                                                 == SNMP_CLASS_SUCCESS)
                thread->SendAgentInfo(in_pdu, from, version);
        }

        /* A second as elapsed, show progress */
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

    if (status != SNMP_CLASS_SUCCESS)
        return;

    emit SignalStartStop(1);

    UdpAddress start_address;

    if (s->MainUI()->DiscoveryLocal->isChecked())
    {
        start_address = UdpAddress("255.255.255.255/161");
    }
    else
    {
        QString tmp_addr(s->MainUI()->DiscoveryFrom->text() + "/161");
        UdpAddress cur_address(tmp_addr.toLatin1().data());

        start_address = cur_address;
    }

    current_progress = 0;

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

        snmp->discover(start_address, num_addresses, 
                       DISCOVERY_WAIT_TIME_PER_PROTO, 
                       cur_version, this);
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

        dt->num_addresses = addr_to[3] - addr_from[3] + 1;
    }

    s->MainUI()->DiscoveryProgress->setRange(0, dt->num_proto*DISCOVERY_WAIT_TIME_PER_PROTO);

    dt->start();
}

