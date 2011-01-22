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
#include <QDate>
#include <QDialog>

#include "mibview.h"
#include "agent.h"
#include "mibmodule.h"
#include "snmp_pp/notifyqueue.h"
#include "preferences.h"
#include "mibselection.h"

#define ASYNC_TIMER_MSEC 5
#define TRAP_TIMER_MSEC 100

typedef struct
{
    Vb vb;
    int syntax;
    QString val;
} vb_data;

Q_DECLARE_METATYPE(vb_data);

// C Callback functions for snmp++
void callback_walk(int reason, Snmp *, Pdu &pdu, SnmpTarget &target, void *cd)
{
    if (cd)
    {
        // just call the real callback member function...
        ((Agent*)cd)->AsyncCallback(reason, pdu, target, 1);
    }
}

void callback_set(int reason, Snmp *, Pdu &pdu, SnmpTarget &target, void *cd)
{
    if (cd)
    {
        // just call the real callback member function...
        ((Agent*)cd)->AsyncCallbackSet(reason, pdu, target);
    }
}

void callback_trap(int reason, Snmp *, Pdu &pdu, SnmpTarget &target, void *cd)
{
    if (cd)
    {
        // just call the real callback member function...
        ((Agent*)cd)->AsyncCallbackTrap(reason, pdu, target);
    }
}

void callback(int reason, Snmp *, Pdu &pdu, SnmpTarget &target, void *cd)
{
    if (cd)
    {
        // just call the real callback member function...
        ((Agent*)cd)->AsyncCallback(reason, pdu, target, 0);
    }
}

Agent::Agent(Snmpb *snmpb)
{
    s = snmpb;

    int status, status2;

    Snmp::socket_startup();  // Initialize socket subsystem

    bool v4 = s->PreferencesObj()->GetEnableIPv4();
    bool v6 = s->PreferencesObj()->GetEnableIPv6();
    int port4 = s->PreferencesObj()->GetTrapPort();
    int port6 = s->PreferencesObj()->GetTrapPort6();

    start_err = ""; 
    start_result = true;

    // Create our SNMP session object
    if (v4 && v6)
    {
        snmp = new Snmp(status, UdpAddress("0.0.0.0"), UdpAddress("::"));
        if (status != SNMP_CLASS_SUCCESS)
        {
            status2 = status;
            // Try dropping IPv6
            snmp = new Snmp(status, UdpAddress("0.0.0.0"));
            if (status != SNMP_CLASS_SUCCESS)
            {
                start_err = QString("Could not create IPv4 session.\n%1")
                                    .arg(Snmp::error_msg(status));
                // Disable IPv4, for the current run only
                s->PreferencesObj()->SetEnableIPv4(false);
                // Try dropping IPv4
                snmp = new Snmp(status, UdpAddress("::"));
                if (status != SNMP_CLASS_SUCCESS)
                {
                    start_err = QString("Could not create IPv4 and IPv6 sessions.\n%1\nAborting.")
                                        .arg(Snmp::error_msg(status));
                    start_result = false;
                    return;
                }
            }
            else
            {
                start_err = QString("Could not create IPv6 session.\n%1")
                                    .arg(Snmp::error_msg(status2));
                // Disable IPv6, for the current run only
                s->PreferencesObj()->SetEnableIPv6(false);
            }
        }
    }
    else if (v4)
    {
        snmp = new Snmp(status, UdpAddress("0.0.0.0"));
        if (status != SNMP_CLASS_SUCCESS)
        {
            start_err = QString("Could not create IPv4 session.\n%1\nAborting.")
                                .arg(Snmp::error_msg(status));
            start_result = false;
            return;
        }
    }
    else if (v6)
    {
        snmp = new Snmp(status, UdpAddress("::"));
        if (status != SNMP_CLASS_SUCCESS)
        {
            start_err = QString("Could not create IPv6 session.\n%1\nAborting.")
                                .arg(Snmp::error_msg(status));
            start_result = false;
            return;
        }
    }
    else
    {
        start_err = QString("No transport protocol enabled. Aborting.");
        start_result = false;
        return;
    }

    // Bind on the SNMP trap ports
    snmp->notify_set_listen_port(port4);
    snmp->notify_set_listen_port6(port6);

    OidCollection oidc;
    TargetCollection targetc;

    status = snmp->notify_register(oidc, targetc, callback_trap, this);
    if (status != SNMP_CLASS_SUCCESS)
    {
        start_err = QString("Could not bind on either IPv4 trap\nport \
%1 or IPv6 trap port %2.\n\n%3\nTrap reception disabled.")
            .arg(port4)
            .arg(port6)
            .arg(Snmp::error_msg(status));
    }
}

bool Agent::GetStartupResult(QString &err)
{
    err = start_err;
    return start_result;
}

void Agent::StartTrapTimer(void)
{
    // Start the timer
    timer.start(TRAP_TIMER_MSEC);
}

void Agent::StopTimer(void)
{
    // Stop the timer
    timer.stop();
}

void Agent::Init(void)
{
    int status;

    // Connect some signals
    connect( s->MainUI()->MIBTree, SIGNAL( WalkFromOid(const QString&) ),
             this, SLOT( WalkFrom(const QString&) ) );
    connect( s->MainUI()->MIBTree, SIGNAL( GetFromOid(const QString&, int) ),
             this, SLOT( GetFrom(const QString&, int) ) );
    connect( s->MainUI()->MIBTree, SIGNAL( GetFromOidPromptInstance(const QString&, int) ),
             this, SLOT( GetFromPromptInstance(const QString&, int) ) );
    connect( s->MainUI()->MIBTree, SIGNAL( GetFromOidSelectInstance(const QString&, int) ),
             this, SLOT( GetFromSelectInstance(const QString&, int) ) );
    connect( s->MainUI()->MIBTree, SIGNAL( SetFromOid(const QString&) ),
             this, SLOT( SetFrom(const QString&) ) );
    connect( s->MainUI()->MIBTree, SIGNAL( Stop() ),
             this, SLOT( Stop() ));
    connect( s->MainUI()->MIBTree, SIGNAL( TableViewFromOid(const QString&) ),
             this, SLOT( TableViewFrom(const QString&) ) );
    connect( s->MainUI()->MIBTree, SIGNAL( VarbindsFromOid(const QString&) ),
             this, SLOT( VarbindsFrom(const QString&) ) );
    connect( s->MainUI()->AgentSettings, 
             SIGNAL( clicked() ), this, SLOT( ShowAgentSettings() ));
    connect( s->APManagerObj(), SIGNAL( AgentProfileListChanged() ), 
             this, SLOT ( AgentProfileListChange() ) );
    connect( this, SIGNAL( StartWalk(bool) ), 
             s->MainUI()->MIBTree, SLOT ( SetWalkInProgress(bool) ) );
    connect( s->MainUI()->actionStop, SIGNAL( triggered() ),
             this, SLOT( Stop() ) );
    connect( s->MainUI()->actionMultipleVarbinds, SIGNAL( triggered() ),
             this, SLOT( Varbinds() ) );

    // Select the default profile from preferences
    QString cp;
    int prefproto = s->PreferencesObj()->GetCurrentProfile(cp);
    // Fill-in the list of agent profiles from profiles manager
    AgentProfileListChange();
    SelectAgentProfile(&cp, prefproto);
    s->MainUI()->MIBTree->SetCurrentAgentIsV1(
        s->MainUI()->AgentProtoV1->isChecked()?true:false);

    // then connect the signals (order is important)
    connect( s->MainUI()->AgentProfile, SIGNAL( currentIndexChanged( int ) ), 
             this, SLOT ( SelectAgentProfile() ) );
    connect( s->MainUI()->AgentProtoV1, SIGNAL( toggled(bool) ),
             this, SLOT( SelectAgentProto() ) );
    connect( s->MainUI()->AgentProtoV2, SIGNAL( toggled(bool) ),
             this, SLOT( SelectAgentProto() ) );
    connect( s->MainUI()->AgentProtoV3, SIGNAL( toggled(bool) ),
             this, SLOT( SelectAgentProto() ) );

    vbui = new Ui_Varbinds();
    vbd = new QDialog(s->MainUI()->MIBTree); 
    vbui->setupUi(vbd);
    connect( vbui->NewOp, SIGNAL( clicked() ), this, SLOT( VarbindsNew() ));
    connect( vbui->EditOp, SIGNAL( clicked() ), this, SLOT( VarbindsEdit() ));
    connect( vbui->DeleteOp, SIGNAL( clicked() ), this, SLOT( VarbindsDelete() ));
    connect( vbui->DeleteAllOp, SIGNAL( clicked() ), this, SLOT( VarbindsDeleteAll() ));
    connect( vbui->MoveUpOp, SIGNAL( clicked() ), this, SLOT( VarbindsMoveUp() ));
    connect( vbui->MoveDownOp, SIGNAL( clicked() ), this, SLOT( VarbindsMoveDown() ));
    connect( vbui->QuitOp, SIGNAL( clicked() ), this, SLOT( VarbindsQuit() ));
    connect( vbui->GetOp, SIGNAL( clicked() ), this, SLOT( VarbindsGet() ));
    connect( vbui->GetNextOp, SIGNAL( clicked() ), this, SLOT( VarbindsGetNext() ));
    connect( vbui->GetBulkOp, SIGNAL( clicked() ), this, SLOT( VarbindsGetBulk() ));
    connect( vbui->SetOp, SIGNAL( clicked() ), this, SLOT( VarbindsSet() ));
    connect( vbui->VarbindsList, SIGNAL( itemSelectionChanged() ), 
             this, SLOT( VarbindsSelected() ));
    connect( vbui->VarbindsList, SIGNAL( itemDoubleClicked(QTreeWidgetItem*, int) ), 
             this, SLOT( VarbindsEdit() ));

    connect(&timer, SIGNAL(timeout()), this, SLOT(TimerExpired()));
    
    // get the Boot counter (you may use any own method for this)
    char *engineId = (char*)"SnmpB_engine";
    unsigned int snmpEngineBoots = 0;

    status = getBootCounter(s->GetBootCounterConfigFile().toLatin1().data(), 
                            engineId, snmpEngineBoots);
    if ((status != SNMPv3_OK) && (status < SNMPv3_FILEOPEN_ERROR))
    {
        QString err = QString("Error loading snmpEngineBoots counter: %1\n")
                              .arg(status);
        QMessageBox::warning ( NULL, "SnmpB", err, 
                               QMessageBox::Ok, Qt::NoButton);
    }
    
    // increase the boot counter
    snmpEngineBoots++;
    
    // save the boot counter
    status = saveBootCounter(s->GetBootCounterConfigFile().toLatin1().data(), 
                             engineId, snmpEngineBoots);
    if (status != SNMPv3_OK)
    {
        QString err = QString("Error saving snmpEngineBoots counter: %1\n")
                              .arg(status);
        QMessageBox::warning ( NULL, "SnmpB", err, 
                               QMessageBox::Ok, Qt::NoButton);
    }
    
    // If _SNMPv3 is enabled we MUST create ONE v3MP object!
    v3mp = new v3MP(engineId, snmpEngineBoots, status);
    if (status != SNMPv3_MP_OK)
    {
        QString err = QString("Could not create v3MP object:\n")
                              .arg(Snmp::error_msg(status));
        QMessageBox::warning ( NULL, "SnmpB", err, 
                               QMessageBox::Ok, Qt::NoButton);
    }
    
    // The v3MP creates a USM object, get the pointer to it
    USM *usm = v3mp->get_usm();
    
    // Load the USM users from a file, if any
    usm->load_users(s->GetUsmUsersConfigFile().toLatin1().data());
}

void Agent::ShowAgentSettings(void)
{
     s->APManagerObj()->SetSelectedAgent(s->MainUI()->AgentProfile->currentText()); 
     s->APManagerObj()->Execute();
}

void Agent::AgentProfileListChange(void)
{
    int prefproto = -1;
    if (s->MainUI()->AgentProtoV1->isChecked()) prefproto = 0;
    else if (s->MainUI()->AgentProtoV2->isChecked()) prefproto = 1;
    else if (s->MainUI()->AgentProtoV3->isChecked()) prefproto = 2;

    QString cap = s->MainUI()->AgentProfile->currentText();
    s->MainUI()->AgentProfile->clear();
    s->MainUI()->AgentProfile->addItems(s->APManagerObj()->GetAgentsList());
    if (cap.isEmpty() == false)
    {
        int idx = s->MainUI()->AgentProfile->findText(cap);
        s->MainUI()->AgentProfile->setCurrentIndex(idx>0?idx:0);
        if (idx < 0) prefproto = -1;
    }
    else
        prefproto = -1;

    SelectAgentProfile(NULL, prefproto);
}

void Agent::SelectAgentProto(void)
{
    int prefproto = -1;
    if (s->MainUI()->AgentProtoV1->isChecked()) prefproto = 0;
    else if (s->MainUI()->AgentProtoV2->isChecked()) prefproto = 1;
    else if (s->MainUI()->AgentProtoV3->isChecked()) prefproto = 2;

    s->MainUI()->MIBTree->SetCurrentAgentIsV1(prefproto==0?true:false);

    SelectAgentProfile(NULL, prefproto);
}

void Agent::SelectAgentProfile(QString *prefprofile, int prefproto)
{
    AgentProfile *ap = s->APManagerObj()->GetAgentProfile
                        (prefprofile?prefprofile->toLatin1():
                         s->MainUI()->AgentProfile->currentText());
    if (ap)
    {
        bool v1,v2,v3;
        int selectedproto = -1;
        ap->GetSupportedProtocol(&v1, &v2, &v3);

        s->MainUI()->AgentProtoV1->setEnabled(v1);
        s->MainUI()->AgentProtoV2->setEnabled(v2);
        s->MainUI()->AgentProtoV3->setEnabled(v3);

        if ((prefproto == 0) && v1)
        {
            s->MainUI()->AgentProtoV1->setChecked(true);
            selectedproto = 0;
        }
        else 
        if ((prefproto == 1) && v2)
        {
            s->MainUI()->AgentProtoV2->setChecked(true);
            selectedproto = 1;
        }
        else
        if ((prefproto == 2) && v3)
        {
            s->MainUI()->AgentProtoV3->setChecked(true);
            selectedproto = 2;
        }
        else
        if (v1)
        {
            s->MainUI()->AgentProtoV1->setChecked(true);
            selectedproto = 0;
        }
        else
        if (v2)
        {
            s->MainUI()->AgentProtoV2->setChecked(true);
            selectedproto = 1;
        }
        else
        if (v3)
        {
            s->MainUI()->AgentProtoV3->setChecked(true);
            selectedproto = 2;
        }

        if (prefprofile)
        {
            // The agent profile is loaded from the preference file, 
            // update the combobox
            int index = s->MainUI()->AgentProfile->findText(prefprofile->toLatin1());
            s->MainUI()->AgentProfile->setCurrentIndex(index);
            s->PreferencesObj()->SaveCurrentProfile(*prefprofile, prefproto);
        }
        else
        {
            // The agent is selected by the user, save it in the preference file
            QString selectedprofile = s->MainUI()->AgentProfile->currentText();
            s->PreferencesObj()->SaveCurrentProfile(selectedprofile, selectedproto);
        }
    }
    else
    {
        s->MainUI()->AgentProtoV1->setEnabled(false);
        s->MainUI()->AgentProtoV2->setEnabled(false);
        s->MainUI()->AgentProtoV3->setEnabled(false);
    }
}

int Agent::Setup(const QString& oid, SnmpTarget **t, Pdu **p, bool usevblist)
{    
    if (!snmp)
        return -1;

    AgentProfile *ap = s->APManagerObj()->GetAgentProfile
                        (s->MainUI()->AgentProfile->currentText());
    if (!ap)
        return -1;
    
    // Create an address object from the entered values
    QString address_str(ap->GetAddress() + "/" + ap->GetPort());
    UdpAddress address(address_str.toLatin1().data());
    
    // check if the address is valid
    // One problem here: if a hostname is entered, a blocking DNS lookup
    // is done by the address object.
    if (!address.valid())
    {
        QString err = QString("Invalid Address or DNS Name: %1\n")
                              .arg(ap->GetAddress());
        QMessageBox::warning ( NULL, "SnmpB", err, 
                               QMessageBox::Ok, Qt::NoButton);
        return -1;
    }

    Pdu *pdu = new Pdu();

    if (s->MainUI()->AgentProtoV3->isChecked())
    {
        // For SNMPv3 we need a UTarget object
        UTarget *utarget = new UTarget(address);

        ConfigTargetFromSettings(version3, utarget, ap);
        theoid = ConfigPduFromSettings(version3, oid, pdu, ap, usevblist);
        *t = utarget;
    }
    else
    {
        // For SNMPv1/v2c we need a CTarget
        CTarget *ctarget = new CTarget(address);

        if (s->MainUI()->AgentProtoV2->isChecked())
        {
            ConfigTargetFromSettings(version2c, ctarget, ap);
            theoid = ConfigPduFromSettings(version2c, oid, pdu, ap, usevblist);
        }
        else
        {
            ConfigTargetFromSettings(version1, ctarget, ap);
            theoid = ConfigPduFromSettings(version1, oid, pdu, ap, usevblist);
        }

        *t = ctarget;
    }

    *p = pdu;
    
    return 0;
}

void Agent::ConfigTargetFromSettings(snmp_version v, SnmpTarget *t, 
                                     AgentProfile *ap)
{
    if (v == version3)
    {
        UTarget *utarget = (UTarget *)t;
        utarget->set_security_model(SNMP_SECURITY_MODEL_USM);
        utarget->set_security_name(ap->GetSecName().toLatin1().data());
    }
    else if ((v == version1) || (v == version2c))
    {
        CTarget *ctarget = (CTarget *)t;
        // set the community
        ctarget->set_readcommunity( ap->GetReadComm().toLatin1().data());
        ctarget->set_writecommunity( ap->GetWriteComm().toLatin1().data());
    }

    // Set the version
    t->set_version(v);

    // Set retries and timeout values
    t->set_retry(ap->GetRetries());
    t->set_timeout(100 * ap->GetTimeout());
}

Oid Agent::ConfigPduFromSettings(snmp_version v, const QString& oid, 
                                 Pdu *p, AgentProfile *ap, bool usevblist)
{
    Vb vb;
    Oid oidobj(oid.toLatin1().data());

    if (usevblist == true)
    {
        p->clear();
        p->set_vblist(vblist.data(), vblist.size());
    }
    else
    {
        // Set the Oid part of the Vb & add it to pdu
        vb.set_oid(oidobj);
        p->clear();
        *p += vb;
    }

    if (v == version3)
    {
        // set the security level to use
        if (ap->GetSecLevel() == 0/*"noAuthNoPriv"*/)
            p->set_security_level(SNMP_SECURITY_LEVEL_NOAUTH_NOPRIV);
        else if (ap->GetSecLevel() == 1/*"authNoPriv"*/)
            p->set_security_level(SNMP_SECURITY_LEVEL_AUTH_NOPRIV);
        else
            p->set_security_level(SNMP_SECURITY_LEVEL_AUTH_PRIV);

        // Not needed, as snmp++ will set it, if the user does not set it
        p->set_context_name(ap->GetContextName().toLatin1().data());
        p->set_context_engine_id(ap->GetContextEngineID().toLatin1().data());
    }

    return oidobj;
}

void Agent::TimerExpired(void)
{
  // When using async requests or if we are waiting for traps or
  // informs, we must call this member function periodically, as
  // snmp++ does not use an internal thread.
  snmp->get_eventListHolder()->SNMPProcessPendingEvents();
}

char *Agent::GetPrintableValue(SmiNode *node, Vb *vb)
{  
    SmiValue myvalue;
    SmiType *type = node?smiGetNodeType(node):NULL;
     
    if (type && (type->name == NULL) && 
        (type->basetype != SMI_BASETYPE_ENUM) && 
        (type->basetype != SMI_BASETYPE_BITS))
        type = smiGetParentType(type);
            
    if (type)
    {                
        myvalue.basetype = type->basetype;
        myvalue.len = 0;
        switch (myvalue.basetype)
        {
        case SMI_BASETYPE_UNSIGNED32:
            vb->get_value(myvalue.value.unsigned32);
            if (vb->get_syntax() == sNMP_SYNTAX_TIMETICKS)
                return (char*)vb->get_printable_value();
            else
                return smiRenderValue(&myvalue, type, SMI_RENDER_ALL);
            break;
        case SMI_BASETYPE_INTEGER32:
            vb->get_value(myvalue.value.integer32);
            return smiRenderValue(&myvalue, type, SMI_RENDER_ALL);
        case SMI_BASETYPE_ENUM:
            vb->get_value(myvalue.value.integer32);
            return smiRenderValue(&myvalue, type, SMI_RENDER_ALL);
        case SMI_BASETYPE_OBJECTIDENTIFIER:
        {
            Oid val;
            vb->get_value(val);

            int oidlen = val.len();
            if (oidlen <= 0) return (char*)""; 
            SmiSubid *ioid = new SmiSubid[oidlen];
            for (int idx = 0; idx < oidlen; idx++) ioid[idx] = val[idx];

            myvalue.value.oid = ioid;
            myvalue.len = oidlen;
            char *ret = smiRenderValue(&myvalue, type, SMI_RENDER_NAME);

            delete [] ioid;
            return ret;
        }
        case SMI_BASETYPE_OCTETSTRING:
        case SMI_BASETYPE_BITS: /* Always OCTETS case in the switch below */
        {
            switch(vb->get_syntax())
            {
            case sNMP_SYNTAX_OCTETS:
            {
                static unsigned char buf[5000];
                unsigned long len;
                vb->get_value(buf, len, 5000);
                myvalue.len = len;
                myvalue.value.ptr = (char*)&buf[0];
                myvalue.value.ptr[len] = '\0';
                return smiRenderValue(&myvalue, type, SMI_RENDER_ALL);
            }
            case sNMP_SYNTAX_OPAQUE:
            case sNMP_SYNTAX_IPADDR:
                return (char*)vb->get_printable_value();
            default:
                break;
            }
        }
        case SMI_BASETYPE_UNSIGNED64:
        {
            Counter64 cntr64;
            if (vb->get_value(cntr64) == SNMP_CLASS_SUCCESS)
            {
                myvalue.value.unsigned64 = Counter64::c64_to_ll(cntr64);
                return smiRenderValue(&myvalue, type, SMI_RENDER_ALL);
            }
        }
        case SMI_BASETYPE_UNKNOWN:
        default:
            break;
        }
    }
    
    // Last resort ...
    return (char*)vb->get_printable_value();
}

// This routine get the sminode pointer based on the oid
// Note that this routine must create a temporary buffer
// because of 64 bits platform issues where an "unsigned long"
// might be 8 bytes long ...
SmiNode* Agent::GetNodeFromOid(Oid &oid)
{
    SmiNode *node = NULL;
    int oidlen = oid.len();

    if (oidlen <= 0)
        return node; 

    SmiSubid *ioid = new SmiSubid[oidlen];

    for (int idx = 0; idx < oidlen; idx++)
        ioid[idx] = oid[idx];

    node = smiGetNodeByOID(oidlen, &ioid[0]);

    delete [] ioid;

    return node;
}

void Agent::AsyncCallbackTrap(int reason, Pdu &pdu, SnmpTarget &target)
{
    static unsigned int nbr = 1;
    Vb vb;
    GenAddress addr;
    TimeTicks ts;
    Oid id;
    int status = 0;
                
    // Bad message type or if there's an error in the pdu, bail out ...
    if ((reason != SNMP_CLASS_NOTIFICATION) || pdu.get_error_status())
        return;
    
    // Create string objects and collect info below
    QString no, date, time, timestamp, nottype, 
            msgtype, version, agtaddr, agtport, 
            community, seclevel, ctxname, ctxid, msgid;
    
    target.get_address(addr);
    IpAddress agent(addr);
    UdpAddress agentUDP(addr);
    
    char buf[10];
    sprintf(buf, "%.4u", nbr);
    no = QString("%1").arg(buf);
    date = QDate::currentDate().toString(Qt::ISODate);
    time = QTime::currentTime().toString(Qt::ISODate);  
    pdu.get_notify_timestamp(ts);
    timestamp = ts.get_printable();
  
    pdu.get_notify_id(id);
    SmiNode *node = GetNodeFromOid(id);
    if (node)
    {
        char *b = smiRenderOID(node->oidlen, node->oid, 
                               SMI_RENDER_NUMERIC);
        char *f = (char*)id.get_printable();
        while ((*b++ == *f++) && (*b != '\0') && (*f != '\0')) ;
        /* f is now the remaining part */
      
        // Print the OID part
        nottype = node->name;
        if (*f != '\0') nottype += QString(f);
    }
    else
        nottype = id.get_printable();
      
    switch(pdu.get_type())
    {
    case sNMP_PDU_V1TRAP:
        msgtype = "Trap(v1)";
        break;
    case sNMP_PDU_TRAP:
        msgtype = "Trap(v2)";
        break;
    case sNMP_PDU_INFORM:
        msgtype = "Inform";
        break;
    case sNMP_PDU_REPORT:
        msgtype = "Report";
        break;
    default:
        msgtype = "Unknown";
        break;
    }
  
    switch(target.get_version())
    {
    case version1:
        version = "SNMPv1";
        break;
    case version2c:
        version = "SNMPv2c";
        break;
    case version3:
        version = "SNMPv3";
        break;
    default:
        version = "Unknown";
        break;
    }
    
    char *add = (char*)agent.get_printable();
    char *name;

    if ((s->PreferencesObj()->GetShowAgentName() == true) &&
        ((name = agent.friendly_name(status)) != NULL) &&
        (strlen(name) != 0))
        agtaddr = QString("%1/%2").arg(name).arg(add);
    else
        agtaddr = add;
    
    agtport = QString("%1").arg(agentUDP.get_port());
            
    if (target.get_type() == SnmpTarget::type_ctarget)
    {
        community = ((CTarget*)&target)->get_readcommunity();
    }
    else
    {
        ctxname = pdu.get_context_name().get_printable();
        ctxid = pdu.get_context_engine_id().get_printable();
        msgid = pdu.get_message_id();
        switch(pdu.get_security_level())
        {
            case SNMP_SECURITY_LEVEL_NOAUTH_NOPRIV:
                seclevel = "NoAuthNoPriv";
            break;
            case SNMP_SECURITY_LEVEL_AUTH_NOPRIV:
                seclevel = "AuthNoPriv";
            break;
            case SNMP_SECURITY_LEVEL_AUTH_PRIV:
                seclevel = "AuthPriv";
            break;
            default:
                seclevel = "Unknown";
        }
    }

    // Add the trap ...
    QStringList values;
    values << no << date << time << timestamp << nottype 
           << msgtype << version << agtaddr << agtport;
    TrapItem *ti = s->TrapObj()->Add(id, values, community, seclevel, 
                                     ctxname, ctxid, msgid);
 
    // Now, loop thru all varbinds and extract info ...
    for (int i=0; i < pdu.get_vb_count(); i++)
    {
        pdu.get_vb(vb, i);
        ti->AddVarBind(vb);
    }
  
    // If its an inform, we have to reply ...
    if (pdu.get_type() == sNMP_PDU_INFORM)
    {
        // Copy the PDU object to feed back in the response
        Pdu ipdu = pdu;
        Vb t(Oid("1.3.6.1.2.1.1.3.0"));
        t.set_value(ts);
        Vb d(Oid("1.3.6.1.6.3.1.1.4.1.0"));
        d.set_value(id);
        ipdu.trim(pdu.get_vb_count()); // Remove all varbinds first
        ipdu += t; ipdu += d;
        for (int i=0; i < pdu.get_vb_count(); i++)
            ipdu += pdu[i];

        snmp->response(ipdu, target, snmp->get_notify_callback_fd());
    }
  
    nbr++;
}

void Agent::AsyncCallback(int reason, Pdu &pdu,
                          SnmpTarget &target, int iswalk)
{
    int pdu_error;
    int vb_error = 0;
    int pdu_index = 0;
    int start_index = 0;
    int status;
    Vb vb;   // empty Vb
    int z = 0;

    switch(reason)
    {
    case SNMP_CLASS_NOTIFICATION:
    case SNMP_CLASS_ASYNC_RESPONSE:
    case SNMP_CLASS_SESSION_DESTROYED:
        break;
    case SNMP_CLASS_TIMEOUT:
        msg = "<font color=red>Timeout</font>";
        goto cleanup;
    default:
        msg = QString("<font color=red>No response received: (%1) %2</font>")
                       .arg(reason).arg(Snmp::error_msg(reason));
        goto cleanup;
    }
    
    // Look at the error status of the Pdu
    pdu_error = pdu.get_error_status();

    if (pdu_error)
    {
        if (iswalk && (pdu_error == SNMP_ERROR_NO_SUCH_NAME))
            goto end;

        pdu_index = pdu.get_error_index();
        if (pdu_index > 0)
            start_index = objects = pdu_index-1;
        else
        {
            msg = QString("<font color=red>%1</font><br>")
                .arg(Snmp::error_msg(pdu_error));
            goto cleanup;
        }
    }

    // The Pdu must contain at least one Vb
    if (pdu.get_vb_count() == 0)
    {
        msg = "<font color=red>Pdu is empty</font>";
        goto cleanup;
    }

    requests++;
 
    for ( z=start_index; z < pdu.get_vb_count(); z++)
    {
        pdu.get_vb( vb, z );

        // look for var bind exception, applies to v2 only   
        if ( (vb_error = vb.get_syntax()) != sNMP_SYNTAX_ENDOFMIBVIEW )
        {
            Oid tmp;
            vb.get_oid(tmp);

            if ((vb_error != sNMP_SYNTAX_NOSUCHOBJECT) && 
                (vb_error != sNMP_SYNTAX_NOSUCHINSTANCE))
                vb_error = 0;

            // Stop there if we're out of scope
            if (iswalk && tmp.nCompare(theoid.len(), theoid))
            {
                goto end;
            }
            else
            {
                objects++;

node_restart:
                SmiNode *node = GetNodeFromOid(tmp);

                // Oid not fully resolved, attempting to load mib that will
                if (!node)
                {
                    QString mod = 
                        s->MibModuleObj()->LoadBestModule(tmp.get_printable());
                    if (mod != "")
                    {
                        msg += QString("[<font color=red>Loading %1</font>]<br>").arg(mod);
                        goto node_restart;
                    }
                }

                if (node)
                {
                    char *b = smiRenderOID(node->oidlen, node->oid, 
                                           SMI_RENDER_NUMERIC);
                    char *f = (char*)vb.get_printable_oid();
                    while ((*b++ == *f++) && (*b != '\0') && (*f != '\0')) ;
                    /* f is now the remaining part */

                    // Oid not fully resolved, attempting to load mib that will
                    if (strcmp(f,".0"))
                    {
                        QString mod = 
                            s->MibModuleObj()->LoadBestModule(tmp.get_printable());
                        if (mod != "")
                        {
                            msg += QString("[<font color=red>Loading %1</font>]<br>").arg(mod);
                            goto node_restart;
                        }
                    }
         
                    // If the VB type is an OID, make sure the best module 
                    // resolving it is loaded
                    SmiType *type = smiGetNodeType(node);
                    if (type && (type->basetype == SMI_BASETYPE_OBJECTIDENTIFIER))
                    {
                        Oid val_oid;
                        vb.get_value(val_oid);
                        QString mod = 
                            s->MibModuleObj()->LoadBestModule(val_oid.get_printable());
                        if (mod != "")
                        {
                            msg += QString("[<font color=red>Loading %1</font>]<br>").arg(mod);
                            goto node_restart;
                        }
                    }

                    if (vb_error || (pdu_error && (z+1 == pdu_index)))
                        msg += QString("<font color=red>ERROR on varbind #</font>");

                    // Print the OID part
                    msg += QString("%1: %2").arg(objects).arg(node->name);
                    if (*f != '\0') msg += QString("%1").arg(f);

                    if (vb_error || (pdu_error && (z+1 == pdu_index)))
                    {
                        if (pdu_error)
                            msg += QString("<font color=red><br>%1</font><br>")
                                           .arg(Snmp::error_msg(pdu_error));
                        else
                            msg += QString("<font color=red><br>%1</font><br>")
                                           .arg(vb_error==sNMP_SYNTAX_NOSUCHOBJECT?
                                            "No Such Object":"No Such Instance");
                        goto end;
                    }

                    // Print the value part
                    msg += QString("    <font color=blue>%1</font>")
                                   .arg(Qt::escape(GetPrintableValue(node, &vb)));
                }
                else
                {
                    if (vb_error || (pdu_error && (z+1 == pdu_index)))
                    {
                        msg += QString("<font color=red>ERROR on varbind #</font>%1: %2")
                                       .arg(objects).arg(vb.get_printable_oid());
                        if (pdu_error)
                            msg += QString("<font color=red><br>%3</font><br>")
                                           .arg(Snmp::error_msg(pdu_error));
                        else
                            msg += QString("<font color=red><br>%3</font><br>")
                                           .arg(vb_error==sNMP_SYNTAX_NOSUCHOBJECT?
                                            "No Such Object":"No Such Instance");
                    }
                    else
                    {
                        /* Unknown OID */
                        msg += QString("%1: %2    <font color=blue>%3</font>")
                            .arg(objects)
                            .arg(vb.get_printable_oid())
                            .arg(vb.get_printable_value());
                    }
                }
            }
        }
        else
            goto end;
        
        // TextEdit append is too slow ... :-( 
        // Buffer each 10 objects before displaying.
        if (!(objects%10))
        {
            s->MainUI()->Query->append(msg);
            msg = "";
        }
        else
            msg += "<br>";
    } // for  

    // Walk request, reissue a get_bulk ...
    if (iswalk && (stop == false))
    {
        // Issue next get_bulk ...
        // last vb becomes seed of next request
        pdu.set_vblist(&vb, 1);
 
        // Now do an async get_bulk
        AgentProfile *ap = s->APManagerObj()->GetAgentProfile
                            (s->MainUI()->AgentProfile->currentText());
        status = snmp->get_bulk(pdu, target, ap?ap->GetNonRepeaters():0, 
                                ap?ap->GetMaxRepetitions():10, 
                                callback_walk, this);

        // Could we send it?
        if (status == SNMP_CLASS_SUCCESS)
        {
            timer.start(ASYNC_TIMER_MSEC);
            return;
        }
        else
        {
            msg = QString("<font color=red>Could not send GETBULK request: %1</font>")
                           .arg(Snmp::error_msg(status));
            goto cleanup;
        }
    }
   
end:
    if (stop == true)
        msg += "<font color=red>-----SNMP query stopped-----</font><br>";
    else
        msg += "-----SNMP query finished-----<br>";
    msg += "<font color=#009000>Total # of Requests = ";    
    msg += QString("%1<br>Total # of Objects = %2</font>")
                    .arg(requests).arg(objects);
cleanup:
    s->MainUI()->Query->append(msg);
    // Dont stop the timer, but put it back to the lower-rate trap timer value
    timer.start(TRAP_TIMER_MSEC);
    s->MibModuleObj()->SetLoadingPolicy(MibModule::MIBLOAD_DEFAULT);
    emit StartWalk(false);
    s->MainUI()->actionStop->setEnabled(false);
}

void Agent::AsyncCallbackSet(int reason, Pdu &pdu, SnmpTarget &target)
{
    int pdu_error;
    int vb_error = 0;
    int pdu_index = 0;
    int start_index = 0;
    int z = 0;
    Vb vb;   // empty Vb
    (void)target;

    switch(reason)
    {
    case SNMP_CLASS_NOTIFICATION:
    case SNMP_CLASS_ASYNC_RESPONSE:
    case SNMP_CLASS_SESSION_DESTROYED:
        break;
    case SNMP_CLASS_TIMEOUT:
        msg = "<font color=red>Timeout</font>";
        goto cleanup;
    default:
        msg = QString("<font color=red>No response received: (%1) %2</font>")
                       .arg(reason).arg(Snmp::error_msg(reason));
        goto cleanup;
    }
    
    // Look at the error status of the Pdu
    pdu_error = pdu.get_error_status();

    if (pdu_error)
    {
        pdu_index = pdu.get_error_index();
        if (pdu_index > 0)
            start_index = objects = pdu_index-1;
        else
        {
            msg = QString("<font color=red>%1</font><br>")
                .arg(Snmp::error_msg(pdu_error));
            goto cleanup;
        }
    }

    // The Pdu must contain at least one Vb
    if (pdu.get_vb_count() == 0)
    {
        msg = "<font color=red>Pdu is empty</font>";
        goto cleanup;
    }

    for ( z=start_index; z < pdu.get_vb_count(); z++)
    {
        pdu.get_vb( vb, z );
         
        // look for var bind exception, applies to v2 only   
        if ( (vb_error = vb.get_syntax()) != sNMP_SYNTAX_ENDOFMIBVIEW )
        {          
            Oid tmp;
            vb.get_oid(tmp);

            if ((vb_error != sNMP_SYNTAX_NOSUCHOBJECT) && 
                (vb_error != sNMP_SYNTAX_NOSUCHINSTANCE))
                vb_error = 0;

            objects++;

            SmiNode *node = GetNodeFromOid(tmp);
            if (node)
            {
                char *b = smiRenderOID(node->oidlen, node->oid, 
                        SMI_RENDER_NUMERIC);
                char *f = (char*)vb.get_printable_oid();
                while ((*b++ == *f++) && (*b != '\0') && (*f != '\0')) ;
                /* f is now the remaining part */

                if (vb_error || (pdu_error && (z+1 == pdu_index)))
                    msg += QString("<font color=red>ERROR on varbind #</font>");

                // Print the OID part
                msg += QString("%1: %2").arg(objects).arg(node->name);
                if (*f != '\0') msg += QString("%1").arg(f);

                if (vb_error || (pdu_error && (z+1 == pdu_index)))
                {
                    if (pdu_error)
                        msg += QString("<font color=red><br>%1</font><br>")
                                       .arg(Snmp::error_msg(pdu_error));
                    else
                        msg += QString("<font color=red><br>%1</font><br>")
                                       .arg(vb_error==sNMP_SYNTAX_NOSUCHOBJECT?
                                            "No Such Object":"No Such Instance");
                    goto end;
                }

                // Print the value part
                msg += QString("    <font color=blue>%1</font>")
                           .arg(Qt::escape(GetPrintableValue(node, &vb)));
            }
            else
            {
                /* Unknown OID */
                if (vb_error || (pdu_error && (z+1 == pdu_index)))
                {
                    msg += QString("<font color=red>ERROR on varbind #</font>%1: %2")
                                   .arg(objects).arg(vb.get_printable_oid());
                    if (pdu_error)
                        msg += QString("<font color=red><br>%3</font><br>")
                                       .arg(Snmp::error_msg(pdu_error));
                    else
                        msg += QString("<font color=red><br>%3</font><br>")
                                       .arg(vb_error==sNMP_SYNTAX_NOSUCHOBJECT?
                                            "No Such Object":"No Such Instance");
                }
                else
                {
                    msg += QString("%1: %2    <font color=blue>%3</font>")
                        .arg(objects)
                        .arg(vb.get_printable_oid())
                        .arg(vb.get_printable_value());
                }
            }
        }
        else
            goto end;
        
        // TextEdit append is too slow ... :-( 
        // Buffer each 10 objects before displaying.
        if (!(objects%10))
        {
            s->MainUI()->Query->append(msg);
            msg = "";
        }
        else
            msg += "<br>";
    } // for  

end:
    msg += "-----SNMP set finished-----<br>";

cleanup:
    s->MainUI()->Query->append(msg);
    // Dont stop the timer, but put it back to the lower-rate trap timer value
    timer.start(TRAP_TIMER_MSEC);
}


void Agent::WalkFrom(const QString& oid)
{
    int status;
    
    // Initialize agent & pdu objects
    SnmpTarget *target;
    Pdu *pdu;
    if (Setup(oid, &target, &pdu) < 0)
        return;
    
    // Clear the Query window ...
    s->MainUI()->Query->clear();
    s->MainUI()->Query->append("<font color=black>-----SNMP query started-----</font>");
    
    // Clear some global vars
    requests = 0;
    objects  = 0;
    msg = "";
    stop = false;
    emit StartWalk(true);
    s->MainUI()->actionStop->setEnabled(true);
 
    // Now do an async get_bulk
    AgentProfile *ap = s->APManagerObj()->GetAgentProfile
                        (s->MainUI()->AgentProfile->currentText());
    status = snmp->get_bulk(*pdu, *target, ap?ap->GetNonRepeaters():0, 
                            ap?ap->GetMaxRepetitions():10, 
                            callback_walk, this);

    // Could we send it?
    if (status == SNMP_CLASS_SUCCESS)
    {
        timer.start(ASYNC_TIMER_MSEC);
    }
    else
    {
        msg = QString("<font color=red>Could not send GETBULK request: %1</font>")
                       .arg(Snmp::error_msg(status));
        s->MainUI()->Query->append(msg);
    }
    
    delete target;
    delete pdu;
}

void Agent::GetFrom(const QString& oid, int op)
{
    switch(op)
    {
    case 0:
        Get(oid);
        break;
    case 1:
        GetNext(oid);
        break;
    case 2:
        GetBulk(oid);
        break;
    default:
        break;
    }
}

void Agent::Get(const QString& oid, bool usevblist)
{
    int status;
    
    // Initialize agent & pdu objects
    SnmpTarget *target;
    Pdu *pdu;
    if (Setup(oid, &target, &pdu, usevblist) < 0)
        return;
    
    // Clear the Query window ...
    s->MainUI()->Query->clear();
    s->MainUI()->Query->append("<font color=black>-----SNMP query started-----</font>");
    
    // Clear some global vars
    requests = 0;
    objects  = 0;
    msg = "";
    stop = false;

    // Now do an async get
    status = snmp->get(*pdu, *target, callback, this);

    // Could we send it?
    if (status == SNMP_CLASS_SUCCESS)
    {
        timer.start(ASYNC_TIMER_MSEC);
    }
    else
    {
        msg = QString("<font color=red>Could not send GET request: %1</font>")
                       .arg(Snmp::error_msg(status));
        s->MainUI()->Query->append(msg);
    }
    
    delete target;
    delete pdu;
}

void Agent::GetNext(const QString& oid, bool usevblist)
{
    int status;
    
    // Initialize agent & pdu objects
    SnmpTarget *target;
    Pdu *pdu;    
    if (Setup(oid, &target, &pdu, usevblist) < 0)
        return;
        
    // Clear the Query window ...
    s->MainUI()->Query->clear();
    s->MainUI()->Query->append("<font color=black>-----SNMP query started-----</font>");
    
    // Clear some global vars
    requests = 0;
    objects  = 0;
    msg = "";
    stop = false;
 
    // Now do an async get_next
    status = snmp->get_next(*pdu, *target, callback, this);

    // Could we send it?
    if (status == SNMP_CLASS_SUCCESS)
    {
        timer.start(ASYNC_TIMER_MSEC);
    }
    else
    {
        msg = QString("<font color=red>Could not send GETNEXT request: %1</font>")
                       .arg(Snmp::error_msg(status));
        s->MainUI()->Query->append(msg);
    }
    
    delete target;
    delete pdu;
}

void Agent::GetBulk(const QString& oid, bool usevblist)
{
    int status;
    
    // Initialize agent & pdu objects
    SnmpTarget *target;
    Pdu *pdu;    
    if (Setup(oid, &target, &pdu, usevblist) < 0)
        return;
        
    // Clear the Query window ...
    s->MainUI()->Query->clear();
    s->MainUI()->Query->append("<font color=black>-----SNMP query started-----</font>");
    
    // Clear some global vars
    requests = 0;
    objects  = 0;
    msg = "";
    stop = false;
 
    // Now do an async get_bulk
    AgentProfile *ap = s->APManagerObj()->GetAgentProfile
                        (s->MainUI()->AgentProfile->currentText());
    status = snmp->get_bulk(*pdu, *target, ap?ap->GetNonRepeaters():0, 
                            ap?ap->GetMaxRepetitions():10, 
                            callback, this);

    // Could we send it?
    if (status == SNMP_CLASS_SUCCESS)
    {
        timer.start(ASYNC_TIMER_MSEC);
    }
    else
    {
        msg = QString("<font color=red>Could not send GETBULK request: %1</font>")
                       .arg(Snmp::error_msg(status));
        s->MainUI()->Query->append(msg);
    }
    
    delete target;
    delete pdu;
}

void Agent::Set(const QString& oid, bool usevblist)
{
    int status;

    // Initialize agent & pdu objects
    SnmpTarget *target;
    Pdu *pdu;    
    if (Setup(oid, &target, &pdu, usevblist) < 0)
        return;

    // Clear the Query window ...
    s->MainUI()->Query->clear();
    s->MainUI()->Query->append("<font color=black>-----SNMP set started-----</font>");

    // Clear some global vars
    requests = 0;
    objects = 0;
    msg = "";
    stop = false;

    // Now do an async set 
    status = snmp->set(*pdu, *target, callback_set, this);

    // Could we send it?
    if (status == SNMP_CLASS_SUCCESS)
    {
        timer.start(ASYNC_TIMER_MSEC);
    }
    else
    {
        msg = QString("<font color=red>Could not send SET request: %1</font>")
            .arg(Snmp::error_msg(status));
        s->MainUI()->Query->append(msg);
    }


    delete target;
    delete pdu;
}

void Agent::SetFrom(const QString& oid)
{
    // Create and run the mib selection dialog
    MibSelection ms(s, s->MainUI()->MIBTree, "Set");

    if (ms.run(oid))
    {
        Vb *vb = ms.GetVarbind();
        if (vb)
        {
            vblist.clear();
            vblist += *vb;
            Set(ms.GetOid(), true);
        }
    }
}

void Agent::Stop(void)
{
    stop = true;
}

/* TODO: make it async */
void Agent::TableViewFrom(const QString& oid)
{
    // Initialize agent & pdu objects
    SnmpTarget *target;
    Pdu *pdu;
    Vb tvb;
    Oid toid;
    int rows = 0;
    
    if (Setup(oid, &target, &pdu) < 0)
        return;
    
    // Clear the Query window ...
    s->MainUI()->Query->clear();
    s->MainUI()->Query->append("<font color=black>-----SNMP query started-----</font>");
    
    // Clear some global vars
    requests = 0;
    objects  = 0;
    msg = "";

    s->MainUI()->Query->append("Collecting table objects, please wait ...<br>");
    
    /* Set the parent oid & parent node */
    Oid poid(oid.toLatin1().data());
    SmiNode *pnode = GetNodeFromOid(poid);

    /* Make sure the parent is a table or row entry ... */
    if ((pnode->nodekind != SMI_NODEKIND_ROW) && 
        (pnode->nodekind != SMI_NODEKIND_TABLE))
    {
        delete target;
        delete pdu;
        s->MainUI()->Query->append("<font color=red>Abort, not a table or row entry</font>");
        return;
    }
   
    /* If the oid is the table element, get the row entry element */ 
    if (pnode->nodekind == SMI_NODEKIND_TABLE)
    {
        pnode = smiGetFirstChildNode(pnode);
        poid = Oid (smiRenderOID(pnode->oidlen, pnode->oid, SMI_RENDER_NUMERIC));
    }

    /* Build the table header */
    msg += QString("<table border=\"1\"><tr bgcolor=yellow><td>Instance</td>");
    for (SmiNode *node = smiGetFirstChildNode(pnode); node != NULL;
         node = smiGetNextChildNode(node))
    {
        msg += QString("<td>%1</td>").arg(node->name);
    }    
    msg += QString("</tr>");
    
    /* Get next on the parent to get the first entry ... */
    tvb.set_oid(poid);
    pdu->set_vblist(&tvb, 1);
    
    Oid roid(poid);
    int first = 1;
    
    // Now do a sync get_next
    while (snmp->get_next(*pdu, *target) == SNMP_CLASS_SUCCESS)
    {
        pdu->get_vb(tvb, 0);
        toid = tvb.get_oid();
        
        if (first) /* Get the column id we'll iterate on */
        {
            first = 0; /* reset the flag */
            roid += toid[poid.len()];
        }

        // look for var bind exception, applies to v2 only   
        if ( tvb.get_syntax() == sNMP_SYNTAX_ENDOFMIBVIEW )
            break;
        
        /* Make sure we dont get out of table scope ... */
        if (toid.nCompare(roid.len(), roid))
            break;
        
        /* Get & print the instance part */
        char *b = (char*)roid.get_printable();
        char *f = (char*)tvb.get_printable_oid();
        while ((*b++ == *f++) && (*b != '\0') && (*f != '\0')) ;
        /* f is now the remaining part */
        f++; /* skip . */
        if (*f != '\0') msg += QString("<tr><td bgcolor=pink>%1</td>").arg(f);
        
        /* Loop thru all columns of that instance and build the row */
        for (SmiNode *node = smiGetFirstChildNode(pnode); node != NULL;
             node = smiGetNextChildNode(node))
        {
            Vb svb;
            toid = Oid (smiRenderOID(node->oidlen, node->oid, SMI_RENDER_NUMERIC));
            toid += f;
            svb.set_oid(toid);    
            pdu->set_vblist(&svb, 1);
            
            // Now do an sync get
            int status = snmp->get(*pdu, *target);

            pdu->get_vb(svb, 0);

            if ((pdu->get_error_status() == SNMP_ERROR_NO_SUCH_NAME) || // For v1
                (svb.get_syntax() == sNMP_SYNTAX_NOSUCHOBJECT) ||       // For v2
                (svb.get_syntax() == sNMP_SYNTAX_NOSUCHINSTANCE)) 
            {
                msg += QString("<td>not available</td>");
            }
            else
            if (status != SNMP_CLASS_SUCCESS)
            {
                msg += QString("<td>not available</td>");
            }
            else
                msg += QString("<td>%1</td>").arg(Qt::escape(GetPrintableValue(node, &svb)));
        }
        msg += QString("</tr>");
        rows++;
        
        // Next get_next ...
        pdu->set_vblist(&tvb, 1);   
    }
    
    msg += QString("</table>");
    msg += "-----SNMP query finished-----<br>";
    msg += QString("<font color=#009000>Total # of rows = %1<br>").arg(rows);
    s->MainUI()->Query->append(msg);
    
    delete target;
    delete pdu;
}

QString Agent::GetValueString(MibSelection &ms, Vb* vb)
{
    // Get the printable value, with an exception for the ENUMs and C64
    if (!ms.GetValue().isEmpty())
    {
        if (ms.GetNode() && 
            (smiGetNodeType(ms.GetNode())->basetype == SMI_BASETYPE_ENUM) && 
            (ms.GetSyntax() == sNMP_SYNTAX_INT32))
        {
            return GetPrintableValue(ms.GetNode(), vb);
        }
        else
            if (ms.GetSyntax() == sNMP_SYNTAX_CNTR64)
            {    
                SmiValue myvalue;
                myvalue.basetype = SMI_BASETYPE_UNSIGNED64;
                myvalue.len = 0;
                Counter64 cntr64;
                SmiType mytype;
                mytype.basetype = SMI_BASETYPE_UNSIGNED64;
                mytype.format = 0;
                if (vb->get_value(cntr64) == SNMP_CLASS_SUCCESS)
                {
                    myvalue.value.unsigned64 = Counter64::c64_to_ll(cntr64);
                    return smiRenderValue(&myvalue, &mytype, SMI_RENDER_ALL);
                }
            }
            else
                return vb->get_printable_value();
    }

    return "";
}

void Agent::Varbinds(void)
{
    vbui->GetBulkOp->setEnabled(s->MainUI()->AgentProtoV1->isChecked()!=true);
    vbd->exec(); 
}

void Agent::VarbindsFrom(const QString& oid)
{
    // Do a background run of the mib selection dialog
    MibSelection ms(s, vbd, "New VarBind");

    ms.bgrun(oid);

    Vb *vb = ms.GetVarbind();
    if (vb)
    {
        QStringList sl;
        sl << ms.GetName() << ms.GetOid() 
           << ms.GetSyntaxName() << GetValueString(ms, vb);

        vb_data data;
        data.vb = *vb;
        data.syntax = ms.GetSyntax();
        data.val = ms.GetValue();
        QTreeWidgetItem *qtwi = new QTreeWidgetItem(sl);
        QVariant qv;
        qv.setValue(data);
        qtwi->setData(0, Qt::UserRole, qv);
        vbui->VarbindsList->addTopLevelItem(qtwi);
        
        vbui->VarbindsList->setCurrentItem(qtwi); 

        vbui->SNMPOps->setEnabled(true);
        vbui->GetBulkOp->setEnabled(s->MainUI()->AgentProtoV1->isChecked()!=true);
    }

    vbd->exec(); 
}

void Agent::VarbindsNew(void)
{
    // Create and run the mib selection dialog
    MibSelection ms(s, vbd, "New VarBind");

    if (ms.run())
    {
        Vb *vb = ms.GetVarbind();
        if (vb)
        {
            QStringList sl;
            sl << ms.GetName() << ms.GetOid() 
               << ms.GetSyntaxName() << GetValueString(ms, vb);

            vb_data data;
            data.vb = *vb;
            data.syntax = ms.GetSyntax();
            data.val = ms.GetValue();
            QTreeWidgetItem *qtwi = new QTreeWidgetItem(sl);
            QVariant qv;
            qv.setValue(data);
            qtwi->setData(0, Qt::UserRole, qv);
            vbui->VarbindsList->addTopLevelItem(qtwi);

            vbui->SNMPOps->setEnabled(true);
            vbui->GetBulkOp->setEnabled(s->MainUI()->AgentProtoV1->isChecked()!=true);
        }
    }
}

void Agent::VarbindsEdit(void)
{
    QTreeWidget *vbl = vbui->VarbindsList;
    QList<QTreeWidgetItem *> items = vbl->selectedItems();
    if (items.size() != 1)
    {
        QMessageBox::critical( NULL, "Edit VarBind", 
                "Please select only one VarBind",
                QMessageBox::Ok, Qt::NoButton);
        return;
    }

    // Create and run the mib selection dialog
    MibSelection ms(s, vbd, "Edit VarBind");

    vb_data data = items[0]->data(0, Qt::UserRole).value<vb_data>();

    if (ms.run(items[0]->text(1), data.syntax, data.val))
    {
        Vb *vb = ms.GetVarbind();
        if (vb)
        {
            items[0]->setText(0, ms.GetName());
            items[0]->setText(1, ms.GetOid());
            items[0]->setText(2, ms.GetSyntaxName());
            items[0]->setText(3, GetValueString(ms, vb));
 
            data.vb = *vb;
            data.syntax = ms.GetSyntax();
            data.val = ms.GetValue();
            QVariant qv;
            qv.setValue(data);
            items[0]->setData(0, Qt::UserRole, qv);
        }
    }
}

void Agent::VarbindsDelete(void)
{
    QTreeWidget *vbl = vbui->VarbindsList;
    QList<QTreeWidgetItem *> items = vbl->selectedItems();

    QTreeWidgetItem *next = vbl->itemBelow(items[items.size()-1]);
    if (!next)
        next = vbl->itemAbove(items[items.size()-1]);

    for (int i = 0; i < items.size(); i++)
        delete vbl->takeTopLevelItem(vbl->indexOfTopLevelItem(items[i]));

    if (next)
        vbl->setCurrentItem(next); 

    if (vbui->VarbindsList->topLevelItemCount() == 0)
        vbui->SNMPOps->setEnabled(false);
}

void Agent::VarbindsDeleteAll(void)
{
    vbui->VarbindsList->clear();

    if (vbui->VarbindsList->topLevelItemCount() == 0)
        vbui->SNMPOps->setEnabled(false);
}

// External C function
bool CompareItemPositions(QTreeWidgetItem *i1, QTreeWidgetItem *i2)
{
    return (i1->treeWidget()->indexOfTopLevelItem(i1) < 
            i1->treeWidget()->indexOfTopLevelItem(i2));
}

void Agent::VarbindsMoveUp(void)
{
    QTreeWidget *vbl = vbui->VarbindsList;
    QList<QTreeWidgetItem *> items = vbl->selectedItems();
    bool cleared = false;

    qSort(items.begin(), items.end(), CompareItemPositions);

    for (int i = 0; i < items.size(); i++)
    {
        int idx = vbl->indexOfTopLevelItem(items[i]);
        int previdx = vbl->indexOfTopLevelItem(vbl->itemAbove(items[i]));

        if (previdx >= 0)
        {
            if (!cleared)
            {
                vbl->clearSelection();
                cleared = true;
            }

            QTreeWidgetItem *item = vbl->takeTopLevelItem(idx);

            vbl->insertTopLevelItem(previdx, item);
            item->setSelected(true);
        }
        else
            break;
    }
}

void Agent::VarbindsMoveDown(void)
{
    QTreeWidget *vbl = vbui->VarbindsList;
    QList<QTreeWidgetItem *> items = vbl->selectedItems();
    bool cleared = false;

    qSort(items.begin(), items.end(), CompareItemPositions);

    for (int i = items.size()-1; i >= 0; i--)
    {
        int idx = vbl->indexOfTopLevelItem(items[i]);
        int nextidx = vbl->indexOfTopLevelItem(vbl->itemBelow(items[i]));

        if (nextidx > 0)
        {
            if (!cleared)
            {
                vbl->clearSelection();
                cleared = true;
            }

            QTreeWidgetItem *item = vbl->takeTopLevelItem(idx);

            vbl->insertTopLevelItem(nextidx, item);
            item->setSelected(true);
        }
        else
            break;
    }
}

void Agent::VarbindsQuit(void)
{
    vbd->accept();
}

void Agent::VarbindsBuildList(void)
{
    QTreeWidget *vbl = vbui->VarbindsList;

    vblist.clear();

    for(int i = 0; i < vbl->topLevelItemCount(); i++)
        vblist += vbl->topLevelItem(i)->data(0, Qt::UserRole).value<vb_data>().vb; 
}

void Agent::VarbindsGet(void)
{
    // Build the vblist
    VarbindsBuildList();

    // Do the operation 
    Get("", true);
}

void Agent::VarbindsGetNext(void)
{
    // Build the vblist
    VarbindsBuildList();

    // Do the operation 
    GetNext("", true);
}

void Agent::VarbindsGetBulk(void)
{
    // Build the vblist
    VarbindsBuildList();

    // Do the operation 
    GetBulk("", true);
}

void Agent::VarbindsSet(void)
{
    // Build the vblist
    VarbindsBuildList();

    // Do the operation 
    Set("", true);
}

// Controls buttons to gray out
void Agent::VarbindsSelected(void)
{
    QList<QTreeWidgetItem *> items = vbui->VarbindsList->selectedItems();

    if (items.isEmpty())
    {
        vbui->EditOp->setEnabled(false);
        vbui->DeleteOp->setEnabled(false);
        vbui->MoveUpOp->setEnabled(false);
        vbui->MoveDownOp->setEnabled(false);
    }
    else
    {
        vbui->EditOp->setEnabled(true);
        vbui->DeleteOp->setEnabled(true);
        vbui->MoveUpOp->setEnabled(true);
        vbui->MoveDownOp->setEnabled(true);
    }
}

int Agent::SelectTableInstance(const QString& oid, QString& outinstance)
{
    // Initialize agent & pdu objects
    SnmpTarget *target;
    Pdu *pdu;
    Vb tvb;
    Oid toid;
    int res = 0;

    if (Setup(oid, &target, &pdu) < 0)
        return res;

    /* Set the oid & node */
    Oid roid(oid.toLatin1().data());
    SmiNode *pnode = GetNodeFromOid(roid);
    
    /* Make sure the node is a column entry ... */
    if (pnode->nodekind != SMI_NODEKIND_COLUMN)
    {
        delete target;
        delete pdu;
        return res;
    }

    /* Get next on the parent to get the first entry ... */
    tvb.set_oid(roid);
    pdu->set_vblist(&tvb, 1);

    // Build the instance selection dialog and show it ...
    QDialog dlist(s->MainUI()->MIBTree, Qt::WindowTitleHint);
    dlist.resize(220, 250);
    QGridLayout gl1(&dlist);
    QGridLayout gl2;
    QLabel label("Please select table instance to query", &dlist);
    gl2.addWidget(&label, 0, 0, 1, 1);
    QListWidget ilist(&dlist);
    gl2.addWidget(&ilist, 1, 0, 1, 1);
    QDialogButtonBox box(QDialogButtonBox::Ok, Qt::Horizontal, &dlist);
    gl2.addWidget(&box, 2, 0, 1, 1);
    gl1.addLayout(&gl2, 0, 0, 1, 1);
    dlist.setWindowTitle(QApplication::translate("Dialog", 
                         "Select Instance", 0, QApplication::UnicodeUTF8));
    QMetaObject::connectSlotsByName(&dlist);
    connect(&ilist, SIGNAL(itemDoubleClicked(QListWidgetItem *)), 
            &dlist, SLOT(accept()));
    connect(&box, SIGNAL(accepted()), &dlist, SLOT(accept()));
    dlist.setModal(true);
    dlist.show();
    dlist.raise();
    dlist.activateWindow();

    // Now do a sync get_next
    while (snmp->get_next(*pdu, *target) == SNMP_CLASS_SUCCESS)
    {
        pdu->get_vb(tvb, 0);
        toid = tvb.get_oid();

        // look for var bind exception, applies to v2 only   
        if ( tvb.get_syntax() == sNMP_SYNTAX_ENDOFMIBVIEW )
            break;

        /* Make sure we dont get out of table scope ... */
        if (toid.nCompare(roid.len(), roid))
            break;

        /* Get & print the instance part */
        char *b = (char*)roid.get_printable();
        char *f = (char*)tvb.get_printable_oid();
        while ((*b++ == *f++) && (*b != '\0') && (*f != '\0')) ;
        /* f is now the remaining part */
        if (*++f != '\0')
            ilist.addItem(f);
        // Next get_next ...
        pdu->set_vblist(&tvb, 1);   
    }

    // Wait for the result
    dlist.exec();

    if (ilist.selectedItems().size() != 0)
    {
        outinstance = ilist.selectedItems().at(0)->text();
        res = 1;
    }
    else
    {
        outinstance = "";
        res = 0;
    }

    delete target;
    delete pdu;

    return res;
}

void Agent::GetFromSelectInstance(const QString& oid, int op)
{
    int res = 0;
    QString inst;

    // Pop-up the selection dialog
    res = SelectTableInstance(oid, inst);

    // Then query the proper instance
    switch(op)
    {
    case 0:
        Get(oid + (res?("." + inst):".0"));
        break;
    case 1:
        GetNext(oid + (res?("." + inst):".0"));
        break;
    case 2:
        GetBulk(oid + (res?("." + inst):".0"));
        break;
    default:
        break;
    }
}

// Callback when the linedit edition is finished in the prompt dialog.
void Agent::GetTypedTableInstance(void)
{
    tinstresult = le->text();
}

void Agent::GetFromPromptInstance(const QString& oid, int op)
{
    int res;

    QDialog dprompt(s->MainUI()->MIBTree, Qt::WindowTitleHint);
    dprompt.resize(370, 60);
    QGridLayout gl(&dprompt);
    QLabel label("Please type table instance to query", &dprompt);
    gl.addWidget(&label, 0, 0, 1, 1);
    QDialogButtonBox box(QDialogButtonBox::Ok, Qt::Vertical, &dprompt);
    gl.addWidget(&box, 0, 1, 2, 1);
    le = new QLineEdit(&dprompt);
    le->setFocus(Qt::OtherFocusReason);
    gl.addWidget(le, 1, 0, 1, 1);
    dprompt.setWindowTitle(QApplication::translate("Dialog", 
                           "Type Instance", 0, QApplication::UnicodeUTF8));
    QObject::connect(&box, SIGNAL(accepted()), &dprompt, SLOT(accept()));
    QMetaObject::connectSlotsByName(&dprompt);
    connect(le, SIGNAL(editingFinished(void)), 
            this, SLOT(GetTypedTableInstance(void)));

    // Wait for the result and then query the proper instance
    res = dprompt.exec();
    switch(op)
    {
    case 0:
        Get(oid + (res?("." + tinstresult):".0"));
        break;
    case 1:
        GetNext(oid + (res?("." + tinstresult):".0"));
        break;
    case 2:
        GetBulk(oid + (res?("." + tinstresult):".0"));
        break;
    default:
        break;
    }

    delete le;
}

unsigned long Agent::GetSyncValue(const QString& oid)
{
    // Initialize agent & pdu objects
    SnmpTarget *target;
    Pdu *pdu;
    Vb vb;
    if (Setup(oid, &target, &pdu) < 0)
        return 0;
        
    // Now do a sync get
    if (snmp->get(*pdu, *target) == SNMP_CLASS_SUCCESS)
    {
        pdu->get_vb(vb, 0);
        
        unsigned long _uint32 = 0;
        long _int32 = 0;
        
        switch(vb.get_syntax())
        {
        case sNMP_SYNTAX_INT32:
            vb.get_value(_int32);
            return _int32;
        case sNMP_SYNTAX_CNTR32:
        case sNMP_SYNTAX_GAUGE32: /* also sNMP_SYNTAX_UINT32*/
        case sNMP_SYNTAX_TIMETICKS:
            vb.get_value(_uint32);
            return _uint32;
        /* TODO: case sNMP_SYNTAX_CNTR64: */
        default:
            break;
        }
    }
    
    return 0;
}

