/*_############################################################################
  _## 
  _##  snmpNext.cpp  
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
/*
  snmpNext.cpp
  
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
char snmpnext_cpp_version[]="@(#) SNMP++ $Id: snmpNext.cpp 3200 2017-04-25 19:53:16Z katz $";
#include <libsnmp.h>

#ifdef WIN32
#define strcasecmp _stricmp
#endif

#include "snmp_pp/snmp_pp.h"

#ifdef SNMP_PP_NAMESPACE
using namespace Snmp_pp;
#endif

static void
usage()
{
    std::cout << "Usage:\n";
    std::cout << "snmpNext IpAddress | DNSName [Oid] [options]\n";
    exit(1);
}

static void
help()
{
    std::cout << "Usage:\n";
    std::cout << "snmpNext IpAddress | DNSName [Oid] [options]\n";
    std::cout << "Oid: sysDescr object is default\n";
    std::cout << "options: -vN , use SNMP version 1, 2 or 3, default is 1\n";
    std::cout << "         -PPort , remote port to use\n";
    std::cout << "         -CCommunity_name, specify community default is 'public' \n";
    std::cout << "         -rN , retries default is N = 1 retry\n";
    std::cout << "         -tN , timeout in hundredths of seconds; default is N = 100\n";
#ifdef _SNMPv3
    std::cout << "         -snSecurityName,\n";
    std::cout << "         -slN , securityLevel to use, default N = 3 = authPriv\n";
    std::cout << "         -smN , securityModel to use, only default N = 3 = USM possible\n";
    std::cout << "         -cnContextName, default empty string\n";
    std::cout << "         -ceContextEngineID, as hex e.g. 800007E580, default empty string\n";
    std::cout << "         -authPROT, use authentication protocol NONE, SHA or MD5\n";
    std::cout << "         -privPROT, use privacy protocol NONE, DES, 3DESEDE, IDEA, AES128, AES192 or AES256\n";
    std::cout << "         -uaAuthPassword\n";
    std::cout << "         -upPrivPassword\n";
#endif
#ifdef WITH_LOG_PROFILES
    std::cout << "         -Lprofile , log profile to use, default is '"
#ifdef DEFAULT_LOG_PROFILE
         << DEFAULT_LOG_PROFILE
#else
         << "original"
#endif
         << "'" << std::endl;
#endif
    std::cout << "         -h, -? - prints this help" << std::endl;
    exit(1);
   }

int main(int argc, char **argv)
{
   //---------[ check the arg count ]----------------------------------------
   if (argc < 2)
     usage();
   if (strstr(argv[1],"-h") != 0)
     help();
   if (strstr(argv[1],"-?") != 0)
     usage();

#if !defined(_NO_LOGGING) && !defined(WITH_LOG_PROFILES)
   // Set filter for logging
   DefaultLog::log()->set_filter(ERROR_LOG, 7);
   DefaultLog::log()->set_filter(WARNING_LOG, 7);
   DefaultLog::log()->set_filter(EVENT_LOG, 7);
   DefaultLog::log()->set_filter(INFO_LOG, 7);
   DefaultLog::log()->set_filter(DEBUG_LOG, 7);
#endif

   Snmp::socket_startup();  // Initialize socket subsystem

   //---------[ make a GenAddress and Oid object to retrieve ]---------------
   UdpAddress address(argv[1]);      // make a SNMP++ Generic address
   if (!address.valid()) {           // check validity of address
          std::cout << "Invalid Address or DNS Name, " << argv[1] << "\n";
          usage();
   }
   Oid oid("1.3.6.1.2.1.1.1");      // default is sysDescr
   if (argc >= 3) {                  // if 3 args, then use the callers Oid
          if (strstr(argv[2],"-")==0) {
             oid = argv[2];
             if (!oid.valid()) {            // check validity of user oid
                    std::cout << "Invalid Oid, " << argv[2] << "\n";
                    usage();
         }
      }
   }
   // std::cout << "GETNEXT for OID " << oid.get_printable() << "\n";

   //---------[ determine options to use ]-----------------------------------
   snmp_version version=version1;                  // default is v1
   int retries=1;                                  // default retries is 1
   int timeout=100;                                // default is 1 second
   u_short port=161;                               // default snmp port is 161
   OctetStr community("public");                   // community name

#ifdef _SNMPv3
   OctetStr privPassword("");
   OctetStr authPassword("");
   OctetStr securityName("");
   int securityModel = SNMP_SECURITY_MODEL_USM;
   int securityLevel = SNMP_SECURITY_LEVEL_AUTH_PRIV;
   OctetStr contextName("");
   OctetStr contextEngineID("");
   long authProtocol = SNMP_AUTHPROTOCOL_NONE;
   long privProtocol = SNMP_PRIVPROTOCOL_NONE;
   v3MP *v3_MP;
#endif

   char *ptr;

   for(int x=1;x<argc;x++) {                           // parse for version
     if (strstr(argv[x],"-v2")!= 0) {
       version = version2c;
       continue;
     }
     if (strstr(argv[x],"-r")!= 0) {                 // parse for retries
       ptr = argv[x]; ptr++; ptr++;
       retries = atoi(ptr);
       if ((retries<0)|| (retries>5)) retries=1; 
       continue;
     }
     if (strstr(argv[x], "-t")!=0) {                 // parse for timeout
       ptr = argv[x]; ptr++; ptr++;
       timeout = atoi(ptr);
       if ((timeout < 100)||(timeout>500)) timeout=100;
       continue;
     }
     if (strstr(argv[x],"-C")!=0) {
       ptr = argv[x]; ptr++; ptr++;
       community = ptr;
       continue;
     }
     if (strstr(argv[x],"-P")!=0) {
       ptr = argv[x]; ptr++; ptr++;
       sscanf(ptr, "%hu", &port);
       continue;
     }

#ifdef WITH_LOG_PROFILES
     if (strstr(argv[x], "-L") != 0) {
       ptr = argv[x]; ptr++; ptr++;
       DefaultLog::log()->set_profile(ptr);
     }
#endif

#ifdef _SNMPv3
     if (strstr(argv[x],"-v3")!= 0) {
       version = version3;
       continue;
     }
     if (strstr(argv[x],"-auth") != 0) {
       ptr = argv[x]; ptr+=5;
       if (strcasecmp(ptr, "SHA") == 0)
         authProtocol = SNMP_AUTHPROTOCOL_HMACSHA;
       else if (strcasecmp(ptr, "MD5") == 0)
         authProtocol = SNMP_AUTHPROTOCOL_HMACMD5;
       else if (strcasecmp(ptr, "NONE") == 0)
         authProtocol = SNMP_AUTHPROTOCOL_NONE;
       else
         std::cout << "Warning: ignoring unknown auth protocol: " << ptr << std::endl;
       continue;
     }
     if (strstr(argv[x],"-priv") != 0) {
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
       else if (strcasecmp(ptr, "NONE") == 0)
           privProtocol = SNMP_PRIVPROTOCOL_NONE;
       else
         std::cout << "Warning: ignoring unknown priv protocol: " << ptr << std::endl;
       continue;
     }
     if (strstr(argv[x],"-sn")!=0) {
       ptr = argv[x]; ptr+=3;
       securityName = ptr;
       continue;
      }
     if (strstr(argv[x], "-sl")!=0) {
       ptr = argv[x]; ptr+=3;
       securityLevel = atoi(ptr);
       if ((securityLevel < SNMP_SECURITY_LEVEL_NOAUTH_NOPRIV) ||
           (securityLevel > SNMP_SECURITY_LEVEL_AUTH_PRIV))
         securityLevel = SNMP_SECURITY_LEVEL_AUTH_PRIV;
       continue;
     }
     if (strstr(argv[x], "-sm")!=0) {
       ptr = argv[x]; ptr+=3;
       securityModel = atoi(ptr);
       if ((securityModel < SNMP_SECURITY_MODEL_V1) ||
           (securityModel > SNMP_SECURITY_MODEL_USM))
         securityModel = SNMP_SECURITY_MODEL_USM;
       continue;
     }
     if (strstr(argv[x],"-cn")!=0) {
       ptr = argv[x]; ptr+=3;
       contextName = ptr;
       continue;
     }
     if (strstr(argv[x],"-ce")!=0) {
       ptr = argv[x]; ptr+=3;
       contextEngineID = OctetStr::from_hex_string(ptr);
       continue;
     }
     if (strstr(argv[x],"-ua")!=0) {
       ptr = argv[x]; ptr+=3;
       authPassword = ptr;
       continue;
     }
     if (strstr(argv[x],"-up")!=0) {
       ptr = argv[x]; ptr+=3;
       privPassword = ptr;
       continue;
     }
#endif
  }

   //----------[ create a SNMP++ session ]-----------------------------------
   int status;
   // bind to any port and use IPv6 if needed
   Snmp snmp(status, 0, (address.get_ip_version() == Address::version_ipv6));

   if (status != SNMP_CLASS_SUCCESS) {
      std::cout << "SNMP++ Session Create Fail, " << snmp.error_msg(status) << "\n";
      return 1;
   }

   //---------[ init SnmpV3 ]--------------------------------------------
#ifdef _SNMPv3
   if (version == version3) {
     const char *engineId = "snmpNext";
     const char *filename = "snmpv3_boot_counter";
     unsigned int snmpEngineBoots = 0;
     int status;

     status = getBootCounter(filename, engineId, snmpEngineBoots);
     if ((status != SNMPv3_OK) && (status < SNMPv3_FILEOPEN_ERROR))
     {
       std::cout << "Error loading snmpEngineBoots counter: " << status << std::endl;
       return 1;
     }
     snmpEngineBoots++;
     status = saveBootCounter(filename, engineId, snmpEngineBoots);
     if (status != SNMPv3_OK)
     {
       std::cout << "Error saving snmpEngineBoots counter: " << status << std::endl;
       return 1;
     }

     int construct_status;
     v3_MP = new v3MP(engineId, snmpEngineBoots, construct_status);
     if (construct_status != SNMPv3_MP_OK)
     {
       std::cout << "Error initializing v3MP: " << construct_status << std::endl;
       return 1;
     }

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
   Vb vb;                                 // construct a Vb object
   vb.set_oid(oid);                      // set the Oid portion of the Vb
   pdu += vb;                             // add the vb to the Pdu

   address.set_port(port);
   CTarget ctarget(address);             // make a target using the address
#ifdef _SNMPv3
   UTarget utarget(address);

   if (version == version3) {
     utarget.set_version(version);          // set the SNMP version SNMPV1 or V2 or V3
     utarget.set_retry(retries);            // set the number of auto retries
     utarget.set_timeout(timeout);          // set timeout
     utarget.set_security_model(securityModel);
     utarget.set_security_name(securityName);
     pdu.set_security_level(securityLevel);
     pdu.set_context_name (contextName);
     pdu.set_context_engine_id(contextEngineID);
   }
   else {
#endif
     ctarget.set_version(version);         // set the SNMP version SNMPV1 or V2
     ctarget.set_retry(retries);           // set the number of auto retries
     ctarget.set_timeout(timeout);         // set timeout
     ctarget.set_readcommunity(community); // set the read community name
#ifdef _SNMPv3
   }
#endif

   //-------[ issue the request, blocked mode ]-----------------------------
   std::cout << "SNMP++ GetNext to " << argv[1] << " SNMPV"
#ifdef _SNMPv3
        << ((version==version3) ? (version) : (version+1))
#else
        << (version+1)
#endif
        << " Retries=" << retries
        << " Timeout=" << timeout * 10 <<"ms";
#ifdef _SNMPv3
   if (version == version3)
     std::cout << std::endl
          << "securityName= " << securityName.get_printable()
          << ", securityLevel= " << securityLevel
          << ", securityModel= " << securityModel << std::endl
          << "contextName= " << contextName.get_printable()
          << ", contextEngineID= " << contextEngineID.get_printable()
          << std::endl;
   else
#endif
     std::cout << " Community=" << community.get_printable() << std::endl << std::flush;

   SnmpTarget *target;
#ifdef _SNMPv3
   if (version == version3)
     target = &utarget;
   else
#endif
     target = &ctarget;

   status = snmp.get_next(pdu, *target);

   if (status == SNMP_CLASS_SUCCESS)
   {
     pdu.get_vb(vb,0);
#ifdef _SNMPv3
     if (pdu.get_type() == REPORT_MSG) {
       std::cout << "Received a report pdu: "
            << snmp.error_msg(vb.get_printable_oid()) << std::endl;
     }
#endif
     std::cout << "Oid = " << vb.get_printable_oid() << std::endl
               << "Value = " << vb.get_printable_value() << std::endl;
   }
   else
   {
     std::cout << "SNMP++ GetNext Error, " << snmp.error_msg(status)
               << " (" << status <<")" << std::endl;
   }

   Snmp::socket_cleanup();  // Shut down socket subsystem
}
