#include <qmessagebox.h>

#include "mibview.h"
#include "agent.h"
#include "snmp_pp/notifyqueue.h"

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
    
    int status;

    debug_set_logfile("snmpb.log"); // Write debug info to a file

    Snmp::socket_startup();  // Initialize socket subsystem

//connect(&timer, SIGNAL(timeout()), this, SLOT(timer_expired()));

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

void Agent::WalkFrom(const QString& oid)
{    
    int status;

    if (!snmp)
        return;

    // Create a Oid and a address object from the entered values
    Oid theoid(oid.latin1());
    UdpAddress address(Address->currentText());

    // check if the address is valid
    // One problem here: if a hostname is entered, a blocking DNS lookup
    // is done by the address object.
    if (!address.valid())
    {
        QString err = QString("Invalid Address or DNS Name: %1\n")
                                       .arg(Address->currentText());
        QMessageBox::warning ( NULL, "SnmpB", err, 
                                            QMessageBox::Ok, QMessageBox::NoButton);
        return;
    }

    Pdu pdu; // empty Pdu
    Vb vb;   // empty Vb
    SnmpTarget *target; // will point to a CTarget(v1/v2c) or UTarget (v3)

    // Set the Oid part of the Vb
    vb.set_oid(theoid);

    // Add the Vb to the Pdu
    pdu += vb;

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
            pdu.set_security_level(SNMP_SECURITY_LEVEL_NOAUTH_NOPRIV);
        else if (SecLevel->currentText() == "authNoPriv")
            pdu.set_security_level(SNMP_SECURITY_LEVEL_AUTH_NOPRIV);
        else
            pdu.set_security_level(SNMP_SECURITY_LEVEL_AUTH_PRIV);

        // Not needed, as snmp++ will set it, if the user does not set it
        pdu.set_context_name(ContextName->text());
        pdu.set_context_engine_id(EngineID->text());
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

        target = ctarget;
    }

    target->set_retry(retries);            // set the number of auto retries
    target->set_timeout(timeout);          // set timeout

    // Clear the Query window ...
    Query->clear();
    
    int requests = 0;        // keep track of # of requests
    int objects  = 0;
    
#define BULK_MAX 10
    // Now loop & get the the stuff ...
    while (( status = snmp->get_bulk( pdu, *target, 0, BULK_MAX)) == SNMP_CLASS_SUCCESS)
    {
        requests++;
        
        for ( int z=0; z < pdu.get_vb_count(); z++)
        {
            pdu.get_vb( vb, z );
            
            if (pdu.get_type() == REPORT_MSG) {
                Oid tmp;
                vb.get_oid(tmp);
                QString msg = QString("<font color=red>Received a reportPdu: %1</font><br>%2 = <font color=blue>%3</font>")
                                                 .arg(snmp->error_msg(tmp))
                                                 .arg(vb.get_printable_oid())
                                                 .arg(vb.get_printable_value());
                Query->append(msg);
                goto end;
            }
            
            objects++;
            
            // look for var bind exception, applies to v2 only   
            if ( vb.get_syntax() != sNMP_SYNTAX_ENDOFMIBVIEW) {
                QString msg = QString("%1 = <font color=blue>%2</font>")
                                                 .arg(vb.get_printable_oid())
                                                 .arg(vb.get_printable_value());
                Query->append(msg);
            }
            else {
                QString msg = QString("<font color=#009000>End of MIB Reached<br>Total # of Requests = %1<br>Total # of Objects  = %2</font>")
                                                 .arg(requests).arg(objects);
                Query->append(msg);
                goto end;
            }
        }
        
        // last vb becomes seed of next rquest
        pdu.set_vblist(&vb, 1);
    }
    
    if ( status != SNMP_ERROR_NO_SUCH_NAME)
    {
        QString msg = QString("<font color=red>SNMP++ snmpWalk Error, %1</font>")
                                         .arg(snmp->error_msg( status));
        Query->append(msg);
    }
    
    {
    QString msg = QString("<font color=#009000>Total # of Requests = %1<br>Total # of Objects  = %2</font>")
                                     .arg(requests).arg(objects);
    Query->append(msg);
    }
    
end:    
    // the target is no longer needed
    delete target;
}
