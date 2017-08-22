/*_############################################################################
  _## 
  _##  test_app.cpp  
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
char test_app_cpp_version[]="@(#) SNMP++ $Id: test_app.cpp 3200 2017-04-25 19:53:16Z katz $";
#include <libsnmp.h>

#include "snmp_pp/snmp_pp.h"

#ifdef WIN32
#define strcasecmp _stricmp
#endif

#ifdef SNMP_PP_NAMESPACE
using namespace Snmp_pp;
#endif

// default request oids
#define NUM_SYS_VBS	6
#define sysDescr	"1.3.6.1.2.1.1.1.0"
#define sysObjectID	"1.3.6.1.2.1.1.2.0"
#define sysUpTime	"1.3.6.1.2.1.1.3.0"
#define sysContact	"1.3.6.1.2.1.1.4.0"
#define sysName		"1.3.6.1.2.1.1.5.0"
#define sysLocation	"1.3.6.1.2.1.1.6.0"
//#define sysServices	"1.3.6.1.2.1.1.7.0" // not all agents support this...

// default notification oid
#define coldStart	"1.3.6.1.6.3.1.1.4.3.0.1"

int main(int argc, char **argv)
{
  int status;
  char *req_str      = (char*) "get";
  //  char *dflt_req_oid = (char*) sysDescr;
  char *dflt_trp_oid = (char*) coldStart;
  char *genAddrStr   = (char*) "127.0.0.1" ;		  // localhost
  char *oid_str      = (char*) NULL;

  if (argc > 1) genAddrStr = argv[1];
  if (argc > 2) req_str    = argv[2];
  if (argc > 3) oid_str    = argv[3];

  Snmp::socket_startup();  // Initialize socket subsystem

  IpAddress ipAddr(genAddrStr);
  if (!ipAddr.valid()) {
    std::cout << "Invalid destination: " << genAddrStr << std::endl;
    return(1);
  }

  // bind to any port and use IPv6 if needed
  Snmp snmp(status, 0, (ipAddr.get_ip_version() == Address::version_ipv6));
  if (status){
    std::cout << "Failed to create SNMP Session: " << status << std::endl;
    return(1);
  }
  std::cout << "Created session successfully" << std::endl;


  CTarget target(ipAddr);
  if (! target.valid()) {
    std::cout << "Invalid target" << std::endl;
    return(1);
  }

  Pdu pdu;
  Vb vb;
  if ( strcmp(req_str, "get") == 0 ) {

    Vb vbl[NUM_SYS_VBS];
    vbl[0].set_oid(sysDescr);
    vbl[1].set_oid(sysObjectID);
    vbl[2].set_oid(sysUpTime);
    vbl[3].set_oid(sysContact);
    vbl[4].set_oid(sysName);
    vbl[5].set_oid(sysLocation);
//    vbl[6].set_oid(sysServices);

    std::cout << "Send a GET-REQUEST to: " << ipAddr.get_printable() << std::endl;
    if ( ! oid_str ) {
      if ( strcmp(genAddrStr,"localhost" ) == 0 ||
	   strcmp(genAddrStr, "127.0.0.1") == 0 ){
	pdu.set_vblist(vbl, NUM_SYS_VBS);
      } else {
	for (int i=0; i<NUM_SYS_VBS;i++)
	  pdu += vbl[i];
      }
    }
    else {
      Oid req_oid(oid_str);
      if ( ! req_oid.valid() ) {
	std::cout << "Request oid constructor failed for:" << oid_str << std::endl;
	return(1);
      }
      vb.set_oid(req_oid);
      pdu += vb;
    }
    status = snmp.get(pdu, target);
    if (status){
      std::cout << "Failed to issue SNMP Get: (" << status  << ") "
	   << snmp.error_msg(status) << std::endl;
      return(1);
    }
    else{
      std::cout << "Issued get successfully" << std::endl;
      int vbcount = pdu.get_vb_count();
      if ( vbcount == NUM_SYS_VBS ) {
	pdu.get_vblist(vbl, vbcount);
	for ( int i=0; i<vbcount ; i++ )  {
	  std::cout << vbl[i].get_printable_oid() << " : " <<
	    vbl[i].get_printable_value() << std::endl;
	}
      } else {
	for ( int i=0; i<vbcount ; i++ )  {
	  pdu.get_vb(vb, i);
	  std::cout << vb.get_printable_oid() << " : " <<
	    vb.get_printable_value() << std::endl;
	}
      }
    }
  }
  else if ( strcmp(req_str, "trap") == 0 ) {
    std::cout << "Send a TRAP to: " << ipAddr.get_printable() << std::endl;

    if ( ! oid_str )
      oid_str = dflt_trp_oid;

    Oid notify_oid(oid_str);
    if ( ! notify_oid.valid() ) {
      std::cout << "Notify oid constructor failed for:" << oid_str << std::endl;
      return(1);
    }

    pdu.set_notify_id(notify_oid);

    // Use a simple payload
    vb.set_oid(sysLocation);
    vb.set_value("This is a test");
    pdu += vb;

    status = snmp.trap(pdu, target);

    if (status){
      std::cout << "Failed to issue SNMP Trap: (" << status  << ") "
	   << snmp.error_msg(status) << std::endl;
      return(1);
    } else {
      std::cout << "Success" << std::endl;
    }

  }
  else {
    std::cout << "Invalid SNMP operation: " << req_str  << std::endl ;
    std::cout << "Usage: " << argv[0] << " hostname [get | trap]" << std::endl;
    return(1);
  }

  Snmp::socket_cleanup();  // Shut down socket subsystem

  return(0);
}
