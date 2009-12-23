/*_############################################################################
  _## 
  _##  MainWindow.cpp  
  _##
  _##  SNMP++v3.2.24
  _##  -----------------------------------------------
  _##  Copyright (c) 2001-2009 Jochen Katz, Frank Fock
  _##
  _##  This software is based on SNMP++2.6 from Hewlett Packard:
  _##  
  _##    Copyright (c) 1996
  _##    Hewlett-Packard Company
  _##  
  _##  ATTENTION: USE OF THIS SOFTWARE IS SUBJECT TO THE FOLLOWING TERMS.
  _##  Permission to use, copy, modify, distribute and/or sell this software 
  _##  and/or its documentation is hereby granted without fee. User agrees 
  _##  to display the above copyright notice and this license notice in all 
  _##  copies of the software and any documentation of the software. User 
  _##  agrees to assume all liability for the use of the software; 
  _##  Hewlett-Packard and Jochen Katz make no representations about the 
  _##  suitability of this software for any purpose. It is provided 
  _##  "AS-IS" without warranty of any kind, either express or implied. User 
  _##  hereby grants a royalty-free license to any and all derivatives based
  _##  upon this software code base. 
  _##  
  _##  Stuttgart, Germany, Fri May 29 22:35:14 CEST 2009 
  _##  
  _##########################################################################*/
#ifndef _MSC_VER
#include <unistd.h>
#endif
#include <iostream>

#include <qstring.h>
#include <qlineedit.h>
#include <qmessagebox.h>
#include <qtimer.h>
#include <qtextedit.h>
#include <qpushbutton.h>
#include <qcombobox.h>
#include <qspinbox.h>
#include <qradiobutton.h>

#include <MainWindow.h>
#include <Properties.h>

#include "snmp_pp/snmp_pp.h"
#include "snmp_pp/notifyqueue.h"
#include "snmp_pp/log.h"

using std::cout;
using std::cerr;
using std::endl;

#define FILE_USERS "usm_users.bin"
#define FILE_LOCALIZED_USERS "usm_localized_users.bin"

/// C Callback function for snmp++
void callback(int reason, Snmp *snmp, Pdu &pdu, SnmpTarget &target, void *cd)
{
  if (cd)
  {
    // just call the real callback member function...
    ((MainWindow*)cd)->async_callback(reason, snmp, pdu, target);
  }
}

void MainWindow::async_callback(int reason, Snmp * /*snmp*/, Pdu &pdu,
				SnmpTarget &target)
{
  Vb nextVb;
  int pdu_error;
  QString prefix_text;
  QString notify_text;

  push_button_get_next->setEnabled(true);

  // What is the reason for this callback?
  if (reason == SNMP_CLASS_NOTIFICATION)
  {
    prefix_text = "Trap:    ";

    // get the notify id for traps
    Oid id;
    pdu.get_notify_id(id);
    notify_text = QString(" ID: %1 Type %2 -- ").arg(id.get_printable())
                  .arg(pdu.get_type());
  }
  else if (reason == SNMP_CLASS_ASYNC_RESPONSE)
  {
    prefix_text = "Response ";
  }
  else if (reason == SNMP_CLASS_TIMEOUT)
  {
    prefix_text = "Timeout  ";
  }
  else
  {
    QString err = QString("\nDid not receive async response/trap: (%1) %2\n")
                  .arg(reason).arg(Snmp::error_msg(reason));
    text_edit_output->append(err);
  }


  // Look at the error status of the Pdu
  pdu_error = pdu.get_error_status();
  if (pdu_error)
  {
    QString err = "\nResponse contains error:\n";
    err += Snmp::error_msg(pdu_error);
    text_edit_output->append(err);
    return;
  }

  // The Pdu must contain at least one Vb
  if (pdu.get_vb_count() == 0)
  {
    QString err = "\nPdu is empty\n";
    text_edit_output->append(err);
    return;
  }

  for (int i=0; i<pdu.get_vb_count(); i++)
  {
    // Get the Vb of the Pdu
    pdu.get_vb(nextVb, i);

    // Get Oid and value from the Vb and display it
    line_edit_obj_id->setText(nextVb.get_printable_oid());
    line_edit_value->setText(nextVb.get_printable_value());

    text_edit_output->append(prefix_text +
			     target.get_address().get_printable() +
			     " -- " +
			     notify_text +
			     line_edit_obj_id->text() + " = " +
			     line_edit_value->text() + "\n");
  }

  // If we received a inform pdu, we have to send a response
  if (pdu.get_type() == sNMP_PDU_INFORM)
  {
    text_edit_output->append("Sending response to inform.\n");

    // just change the value of the first vb
    pdu.get_vb(nextVb, 0);
    nextVb.set_value("This is the response.");
    pdu.set_vb(nextVb, 0);
    snmp->response(pdu, target);
  }
}

MainWindow::MainWindow(QWidget* parent, const char* name, WFlags fl)
  : MainWindowPrivate(parent, name, fl)
{
  int status;

#ifndef _NO_LOGGING
  DefaultLog::log()->set_filter(ERROR_LOG, 5);
  DefaultLog::log()->set_filter(WARNING_LOG, 5);
  DefaultLog::log()->set_filter(EVENT_LOG, 5);
  DefaultLog::log()->set_filter(INFO_LOG, 5);
  DefaultLog::log()->set_filter(DEBUG_LOG, 8);

  // Write debug info to a file
  DefaultLog::init(new AgentLogImpl("QtExample.log"));
#endif

  Snmp::socket_startup();  // Initialize socket subsystem

  connect(&timer, SIGNAL(timeout()), this, SLOT(timer_expired()));

  // get the Boot counter (you may use any own method for this)
  char *engineId = "not_needed";
  char *filename = "snmpv3_boot_counter";
  unsigned int snmpEngineBoots = 0;

  status = getBootCounter(filename, engineId, snmpEngineBoots);
  if ((status != SNMPv3_OK) && (status < SNMPv3_FILEOPEN_ERROR))
  {
    QString err = QString("Error loading snmpEngineBoots counter: %1\n")
                         .arg(status);
    text_edit_output->append(err);
  }

  // increase the boot counter
  snmpEngineBoots++;

  // save the boot counter
  status = saveBootCounter(filename, engineId, snmpEngineBoots);
  if (status != SNMPv3_OK)
  {
    QString err = QString("Error saving snmpEngineBoots counter: %1\n")
                         .arg(status);
    text_edit_output->append(err);
  }

  // Create our SNMP session object
  snmp = new Snmp(status);
  if (status != SNMP_CLASS_SUCCESS)
  {
    QString err = "\nCould not create SNMP++ session:\n";
    err += Snmp::error_msg(status);
    text_edit_output->append(err);
  }

  // If _SNMPv3 is enabled we MUST create ONE v3MP object!
  v3mp = new v3MP(engineId, snmpEngineBoots, status);
  if (status != SNMPv3_MP_OK)
  {
    QString err = "\nCould not create v3MP object:\n";
    err += Snmp::error_msg(status);
    text_edit_output->append(err);
  }

  // The v3MP creates a USM object, get the pointer to it
  USM *usm = v3mp->get_usm();

  // Load the USM users from a file
  if (SNMPv3_USM_OK != usm->load_users(FILE_USERS))
  {
    QString err = "\nCould not load users from file.\n";
    text_edit_output->append(err);
  }    

#if 0 // Localized users will be created automatically!
  if (SNMPv3_USM_OK != usm->load_localized_users(FILE_LOCALIZED_USERS))
  {
    QString err = "\nCould not load localized users from file.\n";
    text_edit_output->append(err);
  }    
#endif
  update_combobox_sec_name();
}

void MainWindow::update_combobox_sec_name()
{
  USM *usm = v3mp->get_usm();

  combo_box_sec_name->clear();

  // get all security names
  usm->lock_user_name_table(); // lock table for peek_XXX()

  const struct UsmUserNameTableEntry *user = usm->peek_first_user();
  QStringList names;
  QString initial("initial");
  QString to_add;
  while (user)
  {
    to_add.setAscii((const char*)(user->usmUserSecurityName.data()),
		     user->usmUserSecurityName.len());
    if (!names.contains(to_add) && (to_add != initial))
      names += to_add;
    
    user = usm->peek_next_user(user);
  }
  usm->unlock_user_name_table(); // unlock table

  combo_box_sec_name->insertStringList(names);
}

MainWindow::~MainWindow()
{
  if (snmp)
  {
    delete snmp;
    snmp = 0;
  }
  if (v3mp)
  {
    USM *usm = v3mp->get_usm();

    // Save USM users with their passwords into a file
    // The passwords are not encrypted!
    if (SNMPv3_USM_OK != usm->save_users(FILE_USERS))
    {
      QString err = "\nCould not save users to file file.\n";
      text_edit_output->append(err);
    }
    
    delete v3mp;
    v3mp = 0;
  }

  Snmp::socket_cleanup();  // Shut down socket subsystem
}

// issue a GET-NEXT request
void MainWindow::push_button_get_next_clicked()
{
  int status;

  if (!snmp)
    return;

  push_button_get_next->setEnabled(false);

  // Create a Oid and a address object from the entered values
  Oid oid(line_edit_obj_id->text());
  UdpAddress address(line_edit_target->text());

  // check if the address is valid
  // One problem here: if a hostname is entered, a blocking DNS lookup
  // is done by the address object.
  if (!address.valid())
  {
    QString err = QString("\nInvalid Address or DNS Name: %1\n")
                .arg(line_edit_target->text());
    text_edit_output->append(err);
    push_button_get_next->setEnabled(true);
    return;
  }

  Pdu pdu; // empty Pdu
  Vb vb;   // empty Vb
  SnmpTarget *target; // will point to a CTarget(v1/v2c) or UTarget (v3)

  // Set the Oid part of the Vb
  vb.set_oid(oid);

  // Add the Vb to the Pdu
  pdu += vb;

  // Get retries and timeout values
  int retries = spin_box_retries->value();
  int timeout = 100 * spin_box_timeout->value();
  
  if (radio_button_v3->isChecked())
  {
    // For SNMPv3 we need a UTarget object
    UTarget *utarget = new UTarget(address);

    utarget->set_version(version3);

    utarget->set_security_model(SNMP_SECURITY_MODEL_USM);
    utarget->set_security_name(combo_box_sec_name->currentText());
    
    target = utarget;

    // set the security level to use
    if (combo_box_sec_level->currentText() == "noAuthNoPriv")
      pdu.set_security_level(SNMP_SECURITY_LEVEL_NOAUTH_NOPRIV);
    else if (combo_box_sec_level->currentText() == "authNoPriv")
      pdu.set_security_level(SNMP_SECURITY_LEVEL_AUTH_NOPRIV);
    else
      pdu.set_security_level(SNMP_SECURITY_LEVEL_AUTH_PRIV);

    // Not needed, as snmp++ will set it, if the user does not set it
    pdu.set_context_name(line_edit_context_name->text());
    pdu.set_context_engine_id(line_edit_context_engine_id->text());
  }
  else
  {
    // For SNMPv1/v2c we need a CTarget
    CTarget *ctarget = new CTarget(address);

    if (radio_button_v2->isChecked())
      ctarget->set_version(version2c);
    else
      ctarget->set_version(version1);

    // set the community
    ctarget->set_readcommunity( line_edit_community->text());

    target = ctarget;
  }

  target->set_retry(retries);            // set the number of auto retries
  target->set_timeout(timeout);          // set timeout

  // Now do an async get_next
  status = snmp->get_next(pdu, *target, callback, this);

  // Could we send it?
  if (status == SNMP_CLASS_SUCCESS)
  {
    timer.start(100);
  }
  else
  {
    QString err = QString("\nCould not send async GETNEXT request: %1\n")
                         .arg(Snmp::error_msg(status));
    text_edit_output->append(err);
    push_button_get_next->setEnabled(true);
  }

  // the target is no longer needed
  delete target;
}

void MainWindow::timer_expired()
{
  // When using async requests or if we are waiting for traps or
  // informs, we must call this member function periodically, as
  // snmp++ does not use an internal thread.
  snmp->eventListHolder->SNMPProcessPendingEvents();
}


// Set the properties
void MainWindow::edit_properties_action_activated()
{
  Properties p;
  p.set_snmp(snmp);
  p.exec();
  update_combobox_sec_name();
}

// Send out a SNMP Broadcast for discovery
void MainWindow::push_button_broadcast_clicked()
{
  int status;

  // Create a new SNMP session object, as while waiting for
  // broadcast responses, all other responses are counted as
  // broadcast responses
  Snmp session(status);
  if (status != SNMP_CLASS_SUCCESS)
  {
    QString err = "\nCould not create SNMP++ session:\n";
    err += Snmp::error_msg(status);
    text_edit_output->append(err);
  }

  // Create a address object from the entered value
  UdpAddress address(line_edit_target->text());

  // check if the address is valid
  // One problem here: if a hostname is entered, a blocking DNS lookup
  // is done by the address object.
  if (!address.valid())
  {
    QString err = QString("\nInvalid Address or DNS Name: %1\n")
                .arg(line_edit_target->text());
    text_edit_output->append(err);
    return;
  }

  // Get retries, timeout and community
  int retries = spin_box_retries->value();
  int timeout = 100 * spin_box_timeout->value();
  OctetStr community(line_edit_community->text());

  // Get the version
  snmp_version version;
  if (radio_button_v3->isChecked())
    version = version3;
  else if (radio_button_v2->isChecked())
    version = version2c;
  else
    version = version1;

  UdpAddressCollection addresses;

  // Now send the broadcast
  // do multiple loops as requested from "retries"
  for (int loops=1; loops<= retries + 1; ++loops)
  {
    status = session.broadcast_discovery(addresses,
					 (timeout + 99) / 100,
					 address, version, &community);

    if (status == SNMP_CLASS_SUCCESS)
    {
      QString err = QString("\nSuccess sending broadcast %1.\n").arg(loops);
      text_edit_output->append(err);
    }
    else
    {
      QString err = QString("\nCould not send broadcast: %1\n")
	.arg(Snmp::error_msg(status));
      text_edit_output->append(err);
    }
  }

  // filter out duplicates
  UdpAddressCollection filtered_addrs;
  int dummy_pos;

  for (int n=0; n < addresses.size(); ++n)
    if (filtered_addrs.find(addresses[n], dummy_pos) == FALSE)
      filtered_addrs += addresses[n];

  // print out all addressess
  text_edit_output->append(QString("Found %1  agents:\n")
			   .arg(filtered_addrs.size()));
  for (int m=0; m < filtered_addrs.size(); ++m)
    text_edit_output->append(QString("Answer received from: %1\n")
			     .arg(filtered_addrs[m].get_printable()));
}

// Handle start/stop of traps
void MainWindow::push_button_traps_toggled(bool isOn)
{
  if (!snmp)
    return;

  // lock port input field while waiting for traps
  line_edit_trap_port->setEnabled(!isOn);

  if (isOn)
  {
    // Start receiving traps

    // get the port
    int port = line_edit_trap_port->text().toUInt();

    // Set the trap listen port for this Snmp object
    snmp->notify_set_listen_port(port);

    OidCollection oidc;
    TargetCollection targetc;

    text_edit_output->append(
      QString("Trying to register for traps on port %1.\n").arg(port));

    int status = snmp->notify_register(oidc, targetc, callback, this);
    if (status != SNMP_CLASS_SUCCESS)
    {
      text_edit_output->append(
	QString("Error register for traps (%1): %2.\n").arg(status)
	       .arg(snmp->error_msg(status)));

      line_edit_trap_port->setEnabled(true);
      push_button_traps->setOn(false);
      return;
    }
    else
      text_edit_output->append("Registered success.\n");

    // Start the timer
    timer.start(100);
  }
  else
  {
    // stop receiving traps
    snmp->notify_unregister();

    text_edit_output->append("Stopped receiving traps.\n");
  }
}
