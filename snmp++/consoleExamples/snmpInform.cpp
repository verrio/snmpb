/*_############################################################################
  _## 
  _##  snmpInform.cpp  
  _##
  _##  SNMP++v3.2.21
  _##  -----------------------------------------------
  _##  Copyright (c) 2001-2006 Jochen Katz, Frank Fock
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
  _##  Stuttgart, Germany, Fri Jun 16 17:48:57 CEST 2006 
  _##  
  _##########################################################################*/
/*
  snmpInform.cpp 

  Copyright (c) 1996
  Hewlett-Packard Company

  ATTENTION: USE OF THIS SOFTWARE IS SUBJECT TO THE FOLLOWING TERMS.
  Permission to use, copy, modify, distribute and/or sell this software
  and/or its documentation is hereby granted without fee. User agrees
  to display the above copyright notice and this license notice in all
  copies of the software and any documentation of the software. User
  agrees to assume all liability for the use of the software; Hewlett-Packard
  makes no representations about the suitability of this software for any
  purpose. It is provided "AS-IS" without warranty of any kind,either express
  or implied. User hereby grants a royalty-free license to any and all
  derivatives based upon this software code base.

  Peter E. Mellquist
*/
char snmpget_cpp_version[]="@(#) SNMP++ $Id$";

#include "snmp_pp/snmp_pp.h"
#include <stdlib.h>
#include <stdio.h>

#ifdef WIN32
#define strcasecmp stricmp
#endif

#ifdef SNMP_PP_NAMESPACE
using namespace Snmp_pp;
#endif

#if (__GNUC__ > 2)
#include <iostream>
using std::cerr;
using std::cout;
using std::endl;
using std::flush;
#else
#include <iostream.h>
#endif

#define COLDSTART "1.3.6.1.6.3.1.1.5.1"
#define PAYLOADID "1.3.6.1.4.1.11.2.16.2"
#define PAYLOAD "SNMP++ Trap Send Test"
#define ENTERPRISE "1.3.6.1.2.1.1.1.2.0.1"

int main(int argc, char **argv)
{
   //---------[ check the arg count ]----------------------------------------
   if ( argc < 2) {
	  cout << "Usage:\n";
	  cout << "snmpInform IpAddress | DNSName [Id] [options]\n";
	  cout << "Id = default is 1.3.6.1.6.3.1.1.5.1 = ColdStart";
	  cout << "options: -vN , use SNMP version 1, 2 or 3, default is 1\n";
	  cout << "         -PPort , remote port to use\n";
	  cout << "         -CCommunity_name, specify community default is 'public' \n";
	  cout << "         -rN , retries default is N = 1 retry\n";
	  cout << "         -tN , timeout in hundredths of seconds; default is N = 100\n";
#ifdef _SNMPv3
          cout << "         -snSecurityName, " << endl;
          cout << "         -slN , securityLevel to use, default N = 3 = authPriv" << endl;
          cout << "         -smN , securityModel to use, only default N = 3 = USM possible\n";
          cout << "         -cnContextName, default empty string" << endl;
          cout << "         -ceContextEngineID, as hex e.g. 800007E580, default empty string" << endl;
          cout << "         -authPROT, use authentication protocol NONE, SHA or MD5\n";
          cout << "         -privPROT, use privacy protocol NONE, DES, 3DESEDE, IDEA, AES128, AES192 or AES256\n";
          cout << "         -uaAuthPassword\n";
          cout << "         -upPrivPassword\n";
#endif
	  return 1;
   }

   Snmp::socket_startup();  // Initialize socket subsystem

   //---------[ make a GenAddress and Oid object to retrieve ]---------------
   UdpAddress address( argv[1]);      // make a SNMP++ Generic address
   if ( !address.valid()) {           // check validity of address
	  cout << "Invalid Address or DNS Name, " << argv[1] << "\n";
	  return 1;
   }
   Oid oid( COLDSTART);    // default is ColdStart 
   if ( argc >= 3) {                  // if 3 args, then use the callers Oid
	  if ( strstr( argv[2],"-")==0) {
	     oid = argv[2];
	     if ( !oid.valid()) {            // check validity of user oid
		    cout << "Invalid Oid, " << argv[2] << "\n";
		    return 1;
         }
      }
   }

   //---------[ determine options to use ]-----------------------------------
   snmp_version version=version1;                  // default is v1
   int retries=1;                                  // default retries is 1
   int timeout=100;                                // default is 1 second
   u_short port=161;                               // default snmp port is 161
   OctetStr community("public");                   // community name
   Oid ent(ENTERPRISE);                            // default enterprise

#ifdef _SNMPv3
   OctetStr privPassword("");
   OctetStr authPassword("");
   OctetStr securityName("");
   int securityModel = SecurityModel_USM;
   int securityLevel = SecurityLevel_authPriv;
   OctetStr contextName("");
   OctetStr contextEngineID("");
   long authProtocol = SNMPv3_usmNoAuthProtocol;
   long privProtocol = SNMPv3_usmNoPrivProtocol;
   v3MP *v3_MP;
#endif

   char *ptr;

   for(int x=1;x<argc;x++) {                           // parse for version
     if ( strstr( argv[x],"-v2")!= 0) {
       version = version2c;
       continue;
     }
     if ( strstr( argv[x],"-r")!= 0) {                 // parse for retries
       ptr = argv[x]; ptr++; ptr++;
       retries = atoi(ptr);
       if (( retries<0)|| (retries>5)) retries=1; 
       continue;
     }
     if ( strstr( argv[x], "-t")!=0) {                 // parse for timeout
       ptr = argv[x]; ptr++; ptr++;
       timeout = atoi( ptr);
       if (( timeout < 100)||( timeout>500)) timeout=100;
       continue;
     }
     if ( strstr( argv[x],"-C")!=0) {
       ptr = argv[x]; ptr++; ptr++;
       community = ptr;
       continue;
     }
     if ( strstr( argv[x],"-P")!=0) {
       ptr = argv[x]; ptr++; ptr++;
       sscanf(ptr, "%hu", &port);
       continue;
     }

#ifdef _SNMPv3
     if ( strstr( argv[x],"-v3")!= 0) {
       version = version3;
       continue;
     }
     if ( strstr( argv[x],"-auth") != 0) {
       ptr = argv[x]; ptr+=5;
       if (strcasecmp(ptr, "SHA") == 0)
	   authProtocol = SNMP_AUTHPROTOCOL_HMACSHA;
       else if (strcasecmp(ptr, "MD5") == 0)
	   authProtocol = SNMP_AUTHPROTOCOL_HMACMD5;
       else
	   authProtocol = SNMP_AUTHPROTOCOL_NONE;
       continue;
     }
     if ( strstr( argv[x],"-priv") != 0) {
       ptr = argv[x]; ptr+=5;
       if (strcasecmp(ptr, "DES") == 0)
	   privProtocol = SNMP_PRIVPROTOCOL_DES;
       else if (strcasecmp(ptr, "3DESEDE") == 0)
	   privProtocol = SNMP_PRIVPROTOCOL_3DESEDE;
       else if (strcasecmp(ptr, "IDEA") == 0)
	   privProtocol = SNMP_PRIVPROTOCOL_IDEA;
       else if (strcasecmp(ptr, "AES128") == 0)
	   privProtocol = SNMP_PRIVPROTOCOL_AES128;
       else if (strcasecmp(ptr, "AES192") == 0)
	   privProtocol = SNMP_PRIVPROTOCOL_AES192;
       else if (strcasecmp(ptr, "AES256") == 0)
	   privProtocol = SNMP_PRIVPROTOCOL_AES256;
       else
	   privProtocol = SNMP_PRIVPROTOCOL_NONE;
       printf("\n\nPrivProt : %ld\n", privProtocol);
       continue;
     }
     if ( strstr( argv[x],"-sn")!=0) {
       ptr = argv[x]; ptr+=3;
       securityName = ptr;
       continue;
      }
     if ( strstr( argv[x], "-sl")!=0) {
       ptr = argv[x]; ptr+=3;
       securityLevel = atoi( ptr);
       if (( securityLevel < SecurityLevel_noAuthNoPriv) ||
           ( securityLevel > SecurityLevel_authPriv))
         securityLevel = SecurityLevel_authPriv;
       continue;
     }
     if ( strstr( argv[x], "-sm")!=0) {
       ptr = argv[x]; ptr+=3;
       securityModel = atoi( ptr);
       if (( securityModel < SecurityModel_v1) ||
           ( securityModel > SecurityModel_USM))
         securityModel = SecurityModel_USM;
       continue;
     }
     if ( strstr( argv[x],"-cn")!=0) {
       ptr = argv[x]; ptr+=3;
       contextName = ptr;
       continue;
     }
     if ( strstr( argv[x],"-ce")!=0) {
       ptr = argv[x]; ptr+=3;
       contextEngineID = OctetStr::from_hex_string(ptr);
       continue;
     }
     if ( strstr( argv[x],"-ua")!=0) {
       ptr = argv[x]; ptr+=3;
       authPassword = ptr;
       continue;
     }
     if ( strstr( argv[x],"-up")!=0) {
       ptr = argv[x]; ptr+=3;
       privPassword = ptr;
       continue;
     }
#endif
  }

   //----------[ create a SNMP++ session ]-----------------------------------
   int status;
   Snmp *snmp;

   if (address.get_ip_version() == Address::version_ipv4)
     snmp = new Snmp(status, "0.0.0.0");
   else
     snmp = new Snmp(status, "::");

   if ( status != SNMP_CLASS_SUCCESS) {
      cout << "SNMP++ Session Create Fail, " << snmp->error_msg(status) << "\n";
      return 1;
   }

   //---------[ init SnmpV3 ]--------------------------------------------
#ifdef _SNMPv3
   if (version == version3) {
     char *engineId = "InformSender";
     char *filename = "snmpv3_boot_counter";
     unsigned int snmpEngineBoots = 0;
     int status;

     status = getBootCounter(filename, engineId, snmpEngineBoots);
     if ((status != SNMPv3_OK) && (status < SNMPv3_FILEOPEN_ERROR))
     {
       cout << "Error loading snmpEngineBoots counter: " << status << endl;
       return 1;
     }
     snmpEngineBoots++;
     status = saveBootCounter(filename, engineId, snmpEngineBoots);
     if (status != SNMPv3_OK)
     {
       cout << "Error saving snmpEngineBoots counter: " << status << endl;
       return 1;
     }

     int construct_status;
     v3_MP = new v3MP(engineId, snmpEngineBoots, construct_status);

     USM *usm = v3_MP->get_usm();
     usm->add_usm_user(securityName,
		       authProtocol, privProtocol,
		       authPassword, privPassword);
   }
   else
   {
     // MUST create a dummy v3MP object if _SNMPv3 is enabled!
     int construct_status;
     v3_MP = new v3MP("dummy", 0, construct_status);
   }
#endif

   //--------[ build up SNMP++ object needed ]-------------------------------
   Pdu pdu;                               // construct a Pdu object
   Vb vb;                                 // variable binding object to use
   vb.set_oid(PAYLOADID);                 // example oid for trap payload
   vb.set_value(PAYLOAD);                 // example string for payload
   pdu += vb;                             // append the vb to the pdu
   pdu.set_notify_id( oid);               // set the id of the trap
   pdu.set_notify_enterprise( ent);       // set up the enterprise of the trap
   address.set_port(port);
   CTarget ctarget( address);             // make a target using the address
#ifdef _SNMPv3
   UTarget utarget( address);

   if (version == version3) {
     utarget.set_version( version);          // set the SNMP version SNMPV1 or V2 or V3
     utarget.set_retry( retries);            // set the number of auto retries
     utarget.set_timeout( timeout);          // set timeout
     utarget.set_security_model( securityModel);
     utarget.set_security_name( securityName);
     pdu.set_security_level( securityLevel);
     pdu.set_context_name (contextName);
     pdu.set_context_engine_id(contextEngineID);
   }
   else {
#endif
     ctarget.set_version( version);         // set the SNMP version SNMPV1 or V2
     ctarget.set_retry( retries);           // set the number of auto retries
     ctarget.set_timeout( timeout);         // set timeout
     ctarget.set_readcommunity( community); // set the read community name
#ifdef _SNMPv3
   }
#endif

   //-------[ Send the trap  ]------------------------------------------------
   cout << "SNMP++ Trap to " << argv[1] << " SNMPV"
#ifdef _SNMPv3
        << ((version==version3) ? (version) : (version+1));
#else
        << (version+1);
#endif
#ifdef _SNMPv3
   if (version == version3)
     cout << endl
          << "securityName= " << securityName.get_printable()
          << ", securityLevel= " << securityLevel
          << ", securityModel= " << securityModel << endl
          << "contextName= " << contextName.get_printable()
          << ", contextEngineID= " << contextEngineID.get_printable()
          << endl;
   else
#endif
     cout << " Community=" << community.get_printable() << endl << flush;

   SnmpTarget *target;
#ifdef _SNMPv3
   if (version == version3)
     target = &utarget;
   else
#endif
     target = &ctarget;

   status = snmp->inform( pdu,*target);

   if (status == SNMP_CLASS_SUCCESS)
   {
     pdu.get_vb( vb,0);
#ifdef _SNMPv3
     if (pdu.get_type() == REPORT_MSG) {
       cout << "Received a report pdu: "
            << snmp->error_msg(vb.get_printable_oid()) << endl;
     }
#endif
     cout << "Oid = " << vb.get_printable_oid() << endl
	  << "Value = " << vb.get_printable_value() << endl;
   }
   else
   {
     cout << "SNMP++ Inform Error, " << snmp->error_msg( status)
	  << " (" << status <<")" << endl ;
   }

   Snmp::socket_cleanup();  // Shut down socket subsystem
}
