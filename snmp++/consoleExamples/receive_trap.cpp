/*_############################################################################
  _## 
  _##  receive_trap.cpp  
  _##
  _##  SNMP++ v3.3
  _##  -----------------------------------------------
  _##  Copyright (c) 2001-2013 Jochen Katz, Frank Fock
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
  _##########################################################################*/

char receivetrap_cpp_version[]="@(#) SNMP++ $Id: receive_trap.cpp 3200 2017-04-25 19:53:16Z katz $";
#include <libsnmp.h>

#include "snmp_pp/snmp_pp.h"
#include "snmp_pp/collect.h"
#include "snmp_pp/notifyqueue.h"

#ifdef SNMP_PP_NAMESPACE
using namespace Snmp_pp;
#endif

void callback(int reason, Snmp *snmp, Pdu &pdu, SnmpTarget &target, void *cd)
{
  Vb nextVb;
  int pdu_error;
  GenAddress addr;

  target.get_address(addr);
  UdpAddress from(addr);

  std::cout << "reason: " << reason << std::endl
       << "msg: " << snmp->error_msg(reason) << std::endl
       << "from: " << from.get_printable() << std::endl;

  pdu_error = pdu.get_error_status();
  if (pdu_error){
    std::cout << "Response contains error: " 
	 << snmp->error_msg(pdu_error)<< std::endl;
  }
  Oid id;
  pdu.get_notify_id(id);
  std::cout << "ID:  " << id.get_printable() << std::endl;
  std::cout << "Type:" << pdu.get_type() << std::endl;

  for (int i=0; i<pdu.get_vb_count(); i++)
  {
    pdu.get_vb(nextVb, i);

    std::cout << "Oid: " << nextVb.get_printable_oid() << std::endl
	 << "Val: " <<  nextVb.get_printable_value() << std::endl;
  }
  if (pdu.get_type() == sNMP_PDU_INFORM) {
      std::cout << "pdu type: " << pdu.get_type() << std::endl;
      std::cout << "sending response to inform: " << std::endl;
      nextVb.set_value("This is the response.");
      pdu.set_vb(nextVb, 0);
      snmp->response(pdu, target);
  }
  std::cout << std::endl;
}

int main(int argc, char **argv)
{
  int trap_port;
  if (argc < 2)
    trap_port = 10162; // no need to be root
  else
    trap_port = atoi(argv[1]);

  //----------[ create a SNMP++ session ]-----------------------------------
  int status; 
  Snmp::socket_startup();  // Initialize socket subsystem
  Snmp snmp(status);                // check construction status
  if (status != SNMP_CLASS_SUCCESS)
  {
    std::cout << "SNMP++ Session Create Fail, " << snmp.error_msg(status) << "\n";
    return 1;
  }

   //---------[ init SnmpV3 ]--------------------------------------------
#ifdef _SNMPv3
   const unsigned char engineId[] = 
       { 0x80, 0x00, 0x13, 0x70, 0xC0, 0xA8, 0x01, 0x0D };
   OctetStr engineID(engineId, 8);
   const char *filename = "snmpv3_boot_counter";
   unsigned int snmpEngineBoots = 0;
   v3MP *v3_MP;

   status = getBootCounter(filename, engineID, snmpEngineBoots);
   if ((status != SNMPv3_OK) && (status < SNMPv3_FILEOPEN_ERROR))
   {
     std::cout << "Error loading snmpEngineBoots counter: " << status << std::endl;
     return 1;
   }
   snmpEngineBoots++;
   status = saveBootCounter(filename, engineID, snmpEngineBoots);
   if (status != SNMPv3_OK)
   {
     std::cout << "Error saving snmpEngineBoots counter: " << status << std::endl;
     return 1;
   }

   int construct_status;
   v3_MP = new v3MP(engineID, snmpEngineBoots, construct_status);
   if (construct_status != SNMPv3_MP_OK)
   {
     std::cout << "Error initializing v3MP: " << construct_status << std::endl;
     return 1;
   }

   USM *usm = v3_MP->get_usm();

   // users at UCD
   usm->add_usm_user("SHADESUser",
		     SNMP_AUTHPROTOCOL_HMACSHA, SNMP_PRIVPROTOCOL_DES,
		     "The UCD Demo Password", "The UCD Demo Password");

  usm->add_usm_user("SHAMD5User",
		    SNMP_AUTHPROTOCOL_HMACMD5, SNMP_PRIVPROTOCOL_DES,
		    "The UCD Demo Password", "The UCD Demo Password");

  usm->add_usm_user("noAuthUser",
		    SNMP_AUTHPROTOCOL_HMACMD5, SNMP_PRIVPROTOCOL_DES,
		    "Password", "Password");

  // testusers
  usm->add_usm_user("unsecureUser",
		    SNMP_AUTHPROTOCOL_NONE, SNMP_PRIVPROTOCOL_NONE, 
		    "", "");

  usm->add_usm_user("MD5",
		    SNMP_AUTHPROTOCOL_HMACMD5, SNMP_PRIVPROTOCOL_NONE,
		    "MD5UserAuthPassword", "");

  usm->add_usm_user("SHA",
		    SNMP_AUTHPROTOCOL_HMACSHA, SNMP_PRIVPROTOCOL_NONE,
		    "SHAUserAuthPassword", "");

  usm->add_usm_user("MD5DES",
		    SNMP_AUTHPROTOCOL_HMACMD5, SNMP_PRIVPROTOCOL_DES,
		    "MD5DESUserAuthPassword", "MD5DESUserPrivPassword");

  usm->add_usm_user("SHADES",
		    SNMP_AUTHPROTOCOL_HMACSHA, SNMP_PRIVPROTOCOL_DES,
		    "SHADESUserAuthPassword", "SHADESUserPrivPassword");

  usm->add_usm_user("MD5IDEA",
		    SNMP_AUTHPROTOCOL_HMACMD5, SNMP_PRIVPROTOCOL_IDEA,
		    "MD5IDEAUserAuthPassword", "MD5IDEAUserPrivPassword");

  usm->add_usm_user("SHAIDEA",
		    SNMP_AUTHPROTOCOL_HMACSHA, SNMP_PRIVPROTOCOL_IDEA,
		    "SHAIDEAUserAuthPassword", "SHAIDEAUserPrivPassword");

#endif
   OidCollection oidc;
   TargetCollection targetc;

   std::cout << "Trying to register for traps on port " << trap_port << "." << std::endl;
   snmp.notify_set_listen_port(trap_port);
   status = snmp.notify_register(oidc, targetc, callback, NULL);
   if (status != SNMP_CLASS_SUCCESS)
   {
     std::cout << "Error register for notify (" << status << "): "
	  << snmp.error_msg(status) << std::endl;
     exit(1);
   }
   else
     std::cout << "Waiting for traps/informs..." << std::endl;


  snmp.start_poll_thread(1000);

  std::cout << "press return to stop\n";
  getc(stdin);

  snmp.stop_poll_thread();

  Snmp::socket_cleanup();  // Shut down socket subsystem
}
  
