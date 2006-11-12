#include <qmessagebox.h>
#include <QDate>

#include "mibview.h"
#include "agent.h"
#include "configfiles.h"
#include "snmp_pp/notifyqueue.h"

#define BULK_MAX 10
#define ASYNC_TIMER_MSEC 5
#define TRAP_TIMER_MSEC 100
#define TRAP_PORT 8888

Agent* CurrentAgent = NULL;

/// C Callback functions for snmp++
void callback_walk(int reason, Snmp *, Pdu &pdu, SnmpTarget &target, void *cd)
{
  if (cd)
  {
    // just call the real callback member function...
    ((Agent*)cd)->AsyncCallback(reason, pdu, target, 1);
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

Agent::Agent(QComboBox* UN, QComboBox* SL, QLineEdit* CN,
             QLineEdit* EID, QComboBox* AProt, QLineEdit* APass,
             QComboBox* PProt, QLineEdit* PPass, QComboBox* A,
             QComboBox* P, QSpinBox* R, QSpinBox* T,
             QRadioButton* v1, QRadioButton* v2, QRadioButton* v3,
             QLineEdit* RC, QLineEdit* WC, 
             QPushButton* DU, QPushButton* AU, QPushButton* SU,
             MibView* MV, QTextEdit* Q, Trap* TR)
{
    // Save all widget pointers in this class ... (ugly, I know ...)
    UserName = UN;
    SecLevel = SL;
    ContextName = CN;
    EngineID = EID;
    AuthProtocol = AProt;
    AuthPass = APass;
    PrivProtocol = PProt;
    PrivPass = PPass;
    Address = A;
    Port = P;
    Retries = R;
    Timeout = T;
    V1 = v1;
    V2 = v2;
    V3 = v3;
    ReadComm = RC;
    WriteComm = WC;
    DeleteUser = DU;
    AddUser = AU;
    SaveUser = SU;
    Query = Q;
    Tr = TR;
    
    // Connect some signals
    connect( MV, SIGNAL( WalkFromOid(const QString&) ),
             this, SLOT( WalkFrom(const QString&) ) );
    connect( MV, SIGNAL( GetFromOid(const QString&) ),
             this, SLOT( GetFrom(const QString&) ) );
    connect( MV, SIGNAL( GetNextFromOid(const QString&) ),
             this, SLOT( GetNextFrom(const QString&) ) );
    connect( MV, SIGNAL( SetFromOid(const QString&) ),
             this, SLOT( SetFrom(const QString&) ) );
    connect( MV, SIGNAL( StopFromOid(const QString&) ),
             this, SLOT( StopFrom(const QString&) ) );
    connect( MV, SIGNAL( TableViewFromOid(const QString&) ),
             this, SLOT( TableViewFrom(const QString&) ) );
    
    int status;
   
#if 0 
    debug_set_logfile("snmpb.log"); // Write debug info to a file
#endif
    
    Snmp::socket_startup();  // Initialize socket subsystem
    
    connect(&timer, SIGNAL(timeout()), this, SLOT(TimerExpired()));
    
    // get the Boot counter (you may use any own method for this)
    char *engineId = "not_needed";
    unsigned int snmpEngineBoots = 0;

    status = getBootCounter(GetBootCounterConfigFile(), engineId, snmpEngineBoots);
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
    status = saveBootCounter(GetBootCounterConfigFile(), engineId, snmpEngineBoots);
    if (status != SNMPv3_OK)
    {
        QString err = QString("Error saving snmpEngineBoots counter: %1\n")
                      .arg(status);
        QMessageBox::warning ( NULL, "SnmpB", err, 
                               QMessageBox::Ok, Qt::NoButton);
    }
    
    // Create our SNMP session object
    snmp = new Snmp(status);
    if (status != SNMP_CLASS_SUCCESS)
    {
        QString err = QString("Could not create SNMP++ session:\n")
                      .arg(Snmp::error_msg(status));
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
    usm->load_users(GetUsmUsersConfigFile());
    
    // Bind on the SNMP trap port
    snmp->notify_set_listen_port(TRAP_PORT);

    OidCollection oidc;
    TargetCollection targetc;

    status = snmp->notify_register(oidc, targetc, callback_trap, this);
    if (status != SNMP_CLASS_SUCCESS)
    {
        QString err = QString("Could not bind on trap port %1:\n%2\n")
                      .arg(TRAP_PORT).arg(Snmp::error_msg(status));
        QMessageBox::warning ( NULL, "SnmpB", err,
                               QMessageBox::Ok, Qt::NoButton);
        return;
    }

    // Start the timer
    timer.start(TRAP_TIMER_MSEC);
}

int Agent::Setup(const QString& oid, SnmpTarget **t, Pdu **p)
{    
    if (!snmp)
        return -1;
    
    Pdu *pdu = new Pdu();
    Vb vb;
    // Set the Oid part of the Vb & add it to pdu
    theoid = Oid(oid.latin1());    
    vb.set_oid(theoid);    
    *pdu += vb;
    
    // Create an address object from the entered values
    UdpAddress address(Address->currentText() + "/" + Port->currentText());
    
    // check if the address is valid
    // One problem here: if a hostname is entered, a blocking DNS lookup
    // is done by the address object.
    if (!address.valid())
    {
        QString err = QString("Invalid Address or DNS Name: %1\n")
                      .arg(Address->currentText());
        QMessageBox::warning ( NULL, "SnmpB", err, 
                               QMessageBox::Ok, Qt::NoButton);
        return -1;
    }
    
    SnmpTarget *target; // will point to a CTarget(v1/v2c) or UTarget (v3)
        
    // Get retries and timeout values
    int retries = Retries->value();
    int timeout = 100 * Timeout->value();
    
    if (V3->isChecked())
    {
        // For SNMPv3 we need a UTarget object
        UTarget *utarget = new UTarget(address);
        
        utarget->set_version(version3);
        
        utarget->set_security_model(SNMP_SECURITY_MODEL_USM);
        utarget->set_security_name(UserName->currentText());
        
        target = utarget;
        
        // set the security level to use
        if (SecLevel->currentText() == "noAuthNoPriv")
            pdu->set_security_level(SNMP_SECURITY_LEVEL_NOAUTH_NOPRIV);
        else if (SecLevel->currentText() == "authNoPriv")
            pdu->set_security_level(SNMP_SECURITY_LEVEL_AUTH_NOPRIV);
        else
            pdu->set_security_level(SNMP_SECURITY_LEVEL_AUTH_PRIV);
        
        // Not needed, as snmp++ will set it, if the user does not set it
        pdu->set_context_name(ContextName->text());
        pdu->set_context_engine_id(EngineID->text());
    }
    else
    {
        // For SNMPv1/v2c we need a CTarget
        CTarget *ctarget = new CTarget(address);
        
        if (V2->isChecked())
            ctarget->set_version(version2c);
        else
            ctarget->set_version(version1);
        
        // set the community
        ctarget->set_readcommunity( ReadComm->text());
        ctarget->set_writecommunity( WriteComm->text());
        
        target = ctarget;
    }
    
    target->set_retry(retries);            // set the number of auto retries
    target->set_timeout(timeout);          // set timeout
    
    *t = target;
    *p = pdu;
    
    return 0;
}

void Agent::TimerExpired(void)
{
  // When using async requests or if we are waiting for traps or
  // informs, we must call this member function periodically, as
  // snmp++ does not use an internal thread.
  snmp->eventListHolder->SNMPProcessPendingEvents();
}

char *Agent::GetPrintableValue(SmiNode *node, Vb *vb)
{  
    SmiValue myvalue;
    SmiType *type = smiGetNodeType(node);
            
    if (type && (type->name == NULL) && 
        (type->basetype != SMI_BASETYPE_ENUM) && 
        (type->basetype != SMI_BASETYPE_BITS))
        type = smiGetParentType(type);
            
    if (type)
    {                
        //printf("        Type: %s\n", smiRenderType(type, SMI_RENDER_ALL));
        
        myvalue.basetype = type->basetype;
        myvalue.len = 0;
        switch (myvalue.basetype)
        {
        case SMI_BASETYPE_UNKNOWN:
            break;
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
            myvalue.value.oid = (SmiSubid*)&(val[0]);
            myvalue.len = val.len();
            return smiRenderValue(&myvalue, type, SMI_RENDER_NAME);
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
                myvalue.value.ptr = buf;
                return smiRenderValue(&myvalue, type, SMI_RENDER_ALL);
            }
            case sNMP_SYNTAX_OPAQUE:
            case sNMP_SYNTAX_IPADDR:
                return (char*)vb->get_printable_value();
            default:
                break;
            }
        }
        
        /* TODO */
        case SMI_BASETYPE_UNSIGNED64:
            // myvalue.value.unsigned64
        case SMI_BASETYPE_INTEGER64:
            // myvalue.value.integer64
        default:
            break;
        }
    }
    
    // Last resort ...
    return (char*)vb->get_printable_value();
}

void Agent::AsyncCallbackTrap(int reason, Pdu &pdu, SnmpTarget &target)
{
    static unsigned long nbr = 1;
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
    sprintf(buf, "%.4lu", nbr);
    no = QString("%1").arg(buf);
    date = QDate::currentDate().toString(Qt::ISODate);
    time = QTime::currentTime().toString(Qt::ISODate);  
    pdu.get_notify_timestamp(ts);
    timestamp = ts.get_printable();
  
    pdu.get_notify_id(id);
    unsigned long* oid = &(id[0]);
    unsigned long  oidlen = id.len();
    SmiNode *node = smiGetNodeByOID(oidlen, (unsigned int *)oid);
    if (node)
    {
        char *b = smiRenderOID(node->oidlen, node->oid, 
                               SMI_RENDER_NUMERIC);
        char *f = (char*)id.get_printable();
        while ((*b++ == *f++) && (*b != '\0') && (*f != '\0'));
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
  
    char *name = agent.friendly_name(status);
    char *add = (char*)agent.get_printable();
  
    if (strlen(name))
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
    TrapItem *ti = Tr->Add(id, values, community, seclevel, 
                           ctxname, ctxid, msgid);
 
    // Now, loop thru all varbinds and extract info ...
    for (int i=0; i < pdu.get_vb_count(); i++)
    {
        pdu.get_vb(vb, i);
        ti->AddVarBind(vb);
    }
  
    // If its a inform, we have to reply ...
    if (pdu.get_type() == sNMP_PDU_INFORM)
    {
        vb.set_value("OK, message received.");
        pdu.set_vb(vb, 0);
        snmp->response(pdu, target);
    }
  
    nbr++;
}

void Agent::AsyncCallback(int reason, Pdu &pdu,
                          SnmpTarget &target, int iswalk)
{
    int pdu_error;
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
        if (pdu_error == SNMP_ERROR_NO_SUCH_NAME)
        {
            goto end;
        }
        else
        {
            msg = QString("<font color=red>Answer contains error: %1</font>")
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
        
    for ( z=0; z < pdu.get_vb_count(); z++)
    {
        pdu.get_vb( vb, z );
            
        objects++;
            
        // look for var bind exception, applies to v2 only   
        if ( vb.get_syntax() != sNMP_SYNTAX_ENDOFMIBVIEW)
        {          
            Oid tmp;
            vb.get_oid(tmp);
            unsigned long* oid = &(tmp[0]);
            unsigned long  oidlen = tmp.len();
                
            // Stop there if we're out of scope
            if (iswalk && tmp.nCompare(theoid.len(), theoid))
            {
                goto end;
            }
            else
            {
                SmiNode *node = smiGetNodeByOID(oidlen, (unsigned int *)oid);
                if (node)
                {
                    char *b = smiRenderOID(node->oidlen, node->oid, 
                                           SMI_RENDER_NUMERIC);
                    char *f = (char*)vb.get_printable_oid();
                    while ((*b++ == *f++) && (*b != '\0') && (*f != '\0'));
                    /* f is now the remaining part */
                    
                    // Print the OID part
                    msg += QString("%1: %2").arg(objects).arg(node->name);
                    if (*f != '\0') msg += QString("%1").arg(f);
                    
                    // Print the value part
                    msg += QString("    <font color=blue>%1</font>")
                                   .arg(GetPrintableValue(node, &vb));
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
        else
            goto end;
        
        // TextEdit append is too slow ... :-( 
        // Buffer each 10 objects before displaying.
        if (!(objects%10))
        {
            Query->append(msg);
            msg = "";
        }
        else
            msg += "<br>";
    } // for  

    // Walk request, reissue a get_bulk ...
    if (iswalk)
    {
        // Issue next get_bulk ...
        // last vb becomes seed of next request
        pdu.set_vblist(&vb, 1);
 
        // Now do an async get_bulk
        status = snmp->get_bulk(pdu, target, 0, BULK_MAX, callback_walk, this);

        // Could we send it?
        if (status == SNMP_CLASS_SUCCESS)
        {
            timer.start(ASYNC_TIMER_MSEC);
            return;
        }
        else
        {
            msg = QString("<font color=red>Could not send GETBULK request: %1<font>")
                           .arg(Snmp::error_msg(status));
            goto cleanup;
        }
    }
    
end:
    msg += "-----SNMP query finished-----<br>";
    msg += "<font color=#009000>Total # of Requests = ";    
    msg += QString("%1<br>Total # of Objects = %2</font>")
                    .arg(requests).arg(objects);
cleanup:
    Query->append(msg);
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
    Query->clear();
    Query->append("-----SNMP query started-----");
    
    // Clear some global vars
    requests = 0;
    objects  = 0;
    msg = "";
    
    // Now do an async get_bulk
    status = snmp->get_bulk(*pdu, *target, 0, BULK_MAX, callback_walk, this);

    // Could we send it?
    if (status == SNMP_CLASS_SUCCESS)
    {
        timer.start(ASYNC_TIMER_MSEC);
    }
    else
    {
        msg = QString("<font color=red>Could not send GETBULK request: %1<font>")
                       .arg(Snmp::error_msg(status));
        Query->append(msg);
    }
    
    delete target;
    delete pdu;
}

void Agent::GetFrom(const QString& oid)
{
    int status;
    
    // Initialize agent & pdu objects
    SnmpTarget *target;
    Pdu *pdu;
    if (Setup(oid, &target, &pdu) < 0)
        return;
    
    // Clear the Query window ...
    Query->clear();
    Query->append("-----SNMP query started-----");
    
    // Clear some global vars
    requests = 0;
    objects  = 0;
    msg = "";

    // Now do an async get
    status = snmp->get(*pdu, *target, callback, this);

    // Could we send it?
    if (status == SNMP_CLASS_SUCCESS)
    {
        timer.start(ASYNC_TIMER_MSEC);
    }
    else
    {
        msg = QString("<font color=red>Could not send GET request: %1<font>")
                       .arg(Snmp::error_msg(status));
        Query->append(msg);
    }
    
    delete target;
    delete pdu;
}

void Agent::GetNextFrom(const QString& oid)
{
    int status;
    
    // Initialize agent & pdu objects
    SnmpTarget *target;
    Pdu *pdu;    
    if (Setup(oid, &target, &pdu) < 0)
        return;
        
    // Clear the Query window ...
    Query->clear();
    Query->append("-----SNMP query started-----");
    
    // Clear some global vars
    requests = 0;
    objects  = 0;
    msg = "";
    
    // Now do an async get_next
    status = snmp->get_next(*pdu, *target, callback, this);

    // Could we send it?
    if (status == SNMP_CLASS_SUCCESS)
    {
        timer.start(ASYNC_TIMER_MSEC);
    }
    else
    {
        msg = QString("<font color=red>Could not send GETNEXT request: %1<font>")
                       .arg(Snmp::error_msg(status));
        Query->append(msg);
    }
    
    delete target;
    delete pdu;
}

void Agent::SetFrom(const QString& oid)
{
    printf("Set %s!\n", oid.latin1());
}

void Agent::StopFrom(const QString& oid)
{
    printf("Stop %s!\n", oid.latin1());
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
    Query->clear();
    Query->append("-----SNMP query started-----");
    
    // Clear some global vars
    requests = 0;
    objects  = 0;
    msg = "";

    Query->append("Collecting table objects, please wait ...<br>");
    
    /* Set the parent oid & parent node */
    Oid poid(oid.latin1());
    SmiNode *pnode = smiGetNodeByOID(poid.len(), (SmiSubid*)&(poid[0]));
    
    /* Make sure the parent is a row entry ... */
    if (pnode->nodekind != SMI_NODEKIND_ROW)
    {
        delete target;
        delete pdu;
        Query->append("<font color=red>Abort, not a row entry</font>");
        return;
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
        
        /* Make sure we dont get out of table scope ... */
        if (toid.nCompare(roid.len(), roid))
            break;
        
        /* Get & print the instance part */
        char *b = (char*)roid.get_printable();
        char *f = (char*)tvb.get_printable_oid();
        while ((*b++ == *f++) && (*b != '\0') && (*f != '\0'));
        /* f is now the remaining part */
        f++; /* skip . */
        if (*f != '\0') msg += QString("<tr><td bgcolor=pink>%1</td>").arg(f);
        
        /* Loop thru all colums of that instance and build the row */
        for (SmiNode *node = smiGetFirstChildNode(pnode); node != NULL;
             node = smiGetNextChildNode(node))
        {
            Vb svb;
            toid.set_data((unsigned long *)node->oid, node->oidlen);
            toid += f;
            svb.set_oid(toid);    
            pdu->set_vblist(&svb, 1);
            
            // Now do an sync get
            if (snmp->get(*pdu, *target) == SNMP_CLASS_SUCCESS)
            {
                pdu->get_vb(svb, 0);
                msg += QString("<td>%1</td>").arg(GetPrintableValue(node, &svb));
            }
            else
            {
                msg += QString("<td>not available</td>");
            }
        }
        
        msg += QString("</tr>");
        rows++;
        
        // Next get_next ...
        pdu->set_vblist(&tvb, 1);   
    }
    
    msg += QString("</table>");
    msg += "-----SNMP query finished-----<br>";
    msg += QString("<font color=#009000>Total # of rows = %1<br>").arg(rows);
    Query->append(msg);
    
    delete target;
    delete pdu;
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

