#include <qmessagebox.h>

#include "mibview.h"
#include "agent.h"
#include "snmp_pp/notifyqueue.h"

#define BULK_MAX 10
#define ASYNC_TIMER_MSEC 5

/// C Callback function for snmp++
void callback(int reason, Snmp *snmp, Pdu &pdu, SnmpTarget &target, void *cd)
{
  if (cd)
  {
    // just call the real callback member function...
    ((Agent*)cd)->AsyncCallback(reason, snmp, pdu, target);
  }
}

Agent::Agent(QComboBox* UN, QComboBox* SL, QLineEdit* CN,
             QLineEdit* EID, QComboBox* AProt, QLineEdit* APass,
             QComboBox* PProt, QLineEdit* PPass, QComboBox* A,
             QComboBox* P, QSpinBox* R, QSpinBox* T,
             QRadioButton* v1, QRadioButton* v2, QRadioButton* v3,
             QLineEdit* RC, QLineEdit* WC, 
             QPushButton* DU, QPushButton* AU, QPushButton* SU,
             MibView* MV, QTextEdit* Q)
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
    
    int status;
    
    debug_set_logfile("snmpb.log"); // Write debug info to a file
    
    Snmp::socket_startup();  // Initialize socket subsystem
    
    connect(&timer, SIGNAL(timeout()), this, SLOT(TimerExpired()));
    
    // get the Boot counter (you may use any own method for this)
    char *engineId = "not_needed";
    char *filename = "snmpv3_boot_counter";
    unsigned int snmpEngineBoots = 0;
    
    status = getBootCounter(filename, engineId, snmpEngineBoots);
    if ((status != SNMPv3_OK) && (status < SNMPv3_FILEOPEN_ERROR))
    {
        QString err = QString("Error loading snmpEngineBoots counter: %1\n")
                      .arg(status);
        QMessageBox::warning ( NULL, "SnmpB", err, 
                               QMessageBox::Ok, QMessageBox::NoButton);
    }
    
    // increase the boot counter
    snmpEngineBoots++;
    
    // save the boot counter
    status = saveBootCounter(filename, engineId, snmpEngineBoots);
    if (status != SNMPv3_OK)
    {
        QString err = QString("Error saving snmpEngineBoots counter: %1\n")
                      .arg(status);
        QMessageBox::warning ( NULL, "SnmpB", err, 
                               QMessageBox::Ok, QMessageBox::NoButton);
    }
    
    // Create our SNMP session object
    snmp = new Snmp(status);
    if (status != SNMP_CLASS_SUCCESS)
    {
        QString err = QString("Could not create SNMP++ session:\n")
                      .arg(Snmp::error_msg(status));
        QMessageBox::warning ( NULL, "SnmpB", err, 
                               QMessageBox::Ok, QMessageBox::NoButton);
    }
    
    // If _SNMPv3 is enabled we MUST create ONE v3MP object!
    v3mp = new v3MP(engineId, snmpEngineBoots, status);
    if (status != SNMPv3_MP_OK)
    {
        QString err = QString("Could not create v3MP object:\n")
                      .arg(Snmp::error_msg(status));
        QMessageBox::warning ( NULL, "SnmpB", err, 
                               QMessageBox::Ok, QMessageBox::NoButton);
    }
    
    // The v3MP creates a USM object, get the pointer to it
    USM *usm = v3mp->get_usm();
    
    // Load the USM users from a file
    if (SNMPv3_USM_OK != usm->load_users("usm_users.bin"))
    {
        QString err = QString("Could not load users from file.");
        QMessageBox::warning ( NULL, "SnmpB", err, 
                               QMessageBox::Ok, QMessageBox::NoButton);
    }
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
                               QMessageBox::Ok, QMessageBox::NoButton);
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
    
    // Clear the Query window ...
    Query->clear();
    Query->append("-----SNMP query started-----");
    
    // Clear some global vars
    requests = 0;
    objects  = 0;
    msg = "";
    
    return 0;
}

void Agent::WalkFrom(const QString& oid)
{
    int status;
    
    // Initialize agent & pdu objects
    SnmpTarget *target;
    Pdu *pdu;
    if (Setup(oid, &target, &pdu) < 0)
        return;
    
    // Now do an async get_bulk
    status = snmp->get_bulk(*pdu, *target, 0, BULK_MAX, callback, this);

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

void Agent::TimerExpired(void)
{
  // When using async requests or if we are waiting for traps or
  // informs, we must call this member function periodically, as
  // snmp++ does not use an internal thread.
  snmp->eventListHolder->SNMPProcessPendingEvents();
}

void Agent::AsyncCallback(int reason, Snmp * /*snmp*/, Pdu &pdu,
                                SnmpTarget &target)
{
    int pdu_error;
    int status;
    Vb vb;   // empty Vb
        
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
        
    for ( int z=0; z < pdu.get_vb_count(); z++)
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
            if (tmp.nCompare(theoid.len(), theoid))
            {
                goto end;
            }
            else
            {
                SmiNode *node = smiGetNodeByOID(oidlen, (unsigned int *)oid);
                if (node)
                {
                    char* base_oid = smiRenderOID(node->oidlen, node->oid, 
                                                  SMI_RENDER_NUMERIC);
                    char* full_oid = (char*)vb.get_printable_oid();
                    char *b = base_oid;
                    char *f = full_oid;                    
                    while ((*b++ == *f++) && (*b != '\0') && (*f != '\0'));
                    /* f is now the remaining part */
                        
                    msg += QString("%1: %2").arg(objects).arg(node->name);
                    if (*f != '\0') msg += QString("%1").arg(f);
                    msg += QString("    <font color=blue>%1</font>")
                                   .arg(vb.get_printable_value());   
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

    // Issue next get_bulk ...
        
    // last vb becomes seed of next request
    pdu.set_vblist(&vb, 1);
 
    // Now do an async get_bulk
    status = snmp->get_bulk(pdu, target, 0, BULK_MAX, callback, this);

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

end:    
    msg += "-----SNMP query finished-----<br>";
    msg += "<font color=#009000>Total # of Requests = ";    
    msg += QString("%1<br>Total # of Objects = %2</font>")
                    .arg(requests).arg(objects);

cleanup:
    Query->append(msg);
    timer.stop();
}

void Agent::GetFrom(const QString& oid)
{
    int status;
    
    // Initialize agent & pdu objects
    SnmpTarget *target;
    Pdu *pdu;
    if (Setup(oid, &target, &pdu) < 0)
        return;
    
    // Now do a sync get
    status = snmp->get(*pdu, *target);

    // Could we send it?
    if (status == SNMP_CLASS_SUCCESS)
    {
        // TODO
        printf("Get completed!\n");
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
    printf("GetNext!\n");
}

void Agent::SetFrom(const QString& oid)
{
    printf("Set!\n");
}

void Agent::StopFrom(const QString& oid)
{
    printf("Stop!\n");
}
