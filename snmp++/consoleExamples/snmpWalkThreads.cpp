/*_############################################################################
  _## 
  _##  snmpWalkThreads.cpp  
  _##
  _##  SNMP++v3.2.25
  _##  -----------------------------------------------
  _##  Copyright (c) 2001-2010 Jochen Katz, Frank Fock
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
  _##  Stuttgart, Germany, Thu Sep  2 00:07:47 CEST 2010 
  _##  
  _##########################################################################*/
/*
  snmpWalk.cpp 
  
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
char snmpwalkthreads_cpp_version[]="@(#) SNMP++ $Id$";

#include "snmp_pp/snmp_pp.h"
#include "snmp_pp/reentrant.h"

#include <stdlib.h>
#include <stdio.h>

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

#ifdef _THREADS
static pthread_attr_t* attr = 0;
#ifdef _WIN32THREADS
#error "This example is not available on WIN32 platforms!"
#endif
#endif

UdpAddress address[1000];
Snmp* snmp = 0;
snmp_version version=version1;                       // default is v1
int retries=1;                                       // default retries is 1
int timeout=100;                                     // default is 1 second
u_short port=161;                                    // default snmp port is 161
OctetStr community("public");                        // read community
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

#endif
#define BULK_MAX 10

SnmpSynchronized ssync;

void* runable(void *data) {
  //--------[ build up SNMP++ object needed ]-------------------------------
  ssync.lock();
  printf("HELLO:%s\n", community.get_printable());
  int t = *((int*)data);
  Pdu pdu;                              // construct a Pdu object
  Vb vb;                                // construct a Vb object
  vb.set_oid("1");                     // set the Oid portion of the Vb
  pdu += vb;                            // add the vb to the Pdu
  CTarget ctarget(address[t]);            // make a target using the address
#ifdef _SNMPv3
  UTarget utarget(address[t]);

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
    ctarget.set_version( version);          // set the SNMP version SNMPV1 or V2 or V3
    ctarget.set_retry( retries);            // set the number of auto retries
    ctarget.set_timeout( timeout);          // set timeout
    ctarget.set_readcommunity( community);  // set the read community to use
    ctarget.set_writecommunity( community);
#ifdef _SNMPv3
  }
#endif

  //-------[ issue the request, blocked mode ]-----------------------------
  cout << "(" << t << "): " 
       << "SNMP++ snmpWalk to " << address[t].get_printable() << " SNMPV" 
#ifdef _SNMPv3
       << ((version==version3) ? (version) : (version+1)) 
#else
       << (version+1) 
#endif
       << " Retries=" << retries
       << " Timeout=" << timeout * 10 <<"ms";
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

  int status = 0;
  int requests = 0;
  int objects = 0;
  ssync.unlock();
  while (( status = snmp->get_bulk( pdu,*target,0,BULK_MAX))
	 == SNMP_CLASS_SUCCESS)
  {
    requests++;
    ssync.lock();
    for ( int z=0;z<pdu.get_vb_count(); z++) {
      pdu.get_vb( vb,z);
#ifdef _SNMPv3
      if (pdu.get_type() == REPORT_MSG) {
	Oid tmp;
	vb.get_oid(tmp);
	cout << "(" << t << "): " << "Received a reportPdu: "
	     << snmp->error_msg( tmp) 
	     << endl
	     << vb.get_printable_oid() << " = "
	     << vb.get_printable_value() << endl;
	ssync.unlock();
	return 0;
      }
#endif
      objects++;
      // look for var bind exception, applies to v2 only   
      if ( vb.get_syntax() != sNMP_SYNTAX_ENDOFMIBVIEW) {
	cout <<  "(" << t << "): " 
	     << vb.get_printable_oid() << " = ";
	cout << vb.get_printable_value() << "\n";
      }
      else {
	cout <<  "(" << t << "): " 
	     << "End of MIB Reached\n";
	cout <<  "(" << t << "): " 
	     << "Total # of Requests = " << requests << "\n";
	cout <<  "(" << t << "): "
	     << "Total # of Objects  = " << objects  << "\n";
	ssync.unlock();
	return 0;
      }
    }
    ssync.unlock();
    // last vb becomes seed of next rquest
    pdu.set_vblist(&vb, 1);
  }
  if ( status != SNMP_ERROR_NO_SUCH_NAME)
    cout <<  "(" << t << "): "
	 << "SNMP++ snmpWalk Error, " << snmp->error_msg( status) << "\n";
  cout <<  "(" << t << "): "
       << "Total # of Requests = " << requests << "\n";
  cout <<  "(" << t << "): "
       << "Total # of Objects  = " << objects  << "\n";
  return 0;
}  // end Walk 



int main(int argc, char **argv)
{
  //---------[ check the arg count ]----------------------------------------
  if ( argc < 2) {
	  cout << "Usage:\n";
	  cout << "snmpWalkThreads host/port [host/port]... [options]\n";
	  cout << "StartOid: 1\n";
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
          return 0;
  }

  Snmp::socket_startup();  // Initialize socket subsystem

   // Set filter for logging
   DefaultLog::log()->set_filter(ERROR_LOG, 15);
   DefaultLog::log()->set_filter(WARNING_LOG, 15);
   DefaultLog::log()->set_filter(EVENT_LOG, 8);
   DefaultLog::log()->set_filter(INFO_LOG, 8);
   DefaultLog::log()->set_filter(DEBUG_LOG, 0);

  //---------[ make a GenAddress and Oid object to retrieve ]---------------
  address[0] = UdpAddress(argv[1]);
  if ( !address[0].valid()) {           // check validity of address
    cout << "Invalid Address or DNS Name, " << argv[1] << "\n";
    return -1;
  }
  int x=2;
  while ((x<argc) && (x<100) && (strstr(argv[x],"-")==0)) {
    address[x-1] = UdpAddress(argv[x]);
    if ( !address[x-1].valid()) {           // check validity of address
      cout << "Invalid Address or DNS Name, " << argv[x] << "\n";
      return -1;
    }     
    x++;
  }
  int threads = x-1;

  cout << community.get_printable() << endl;

   //---------[ determine options to use ]-----------------------------------
   char *ptr;
   for(;x<argc;x++) {                           // parse for version
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
       else if (strcasecmp(ptr, "NONE") == 0)
	 authProtocol = SNMP_AUTHPROTOCOL_NONE;
       else
	 cout << "Warning: ignoring unknown auth protocol: " << ptr << endl;
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
       else if (strcasecmp(ptr, "NONE") == 0)
	   privProtocol = SNMP_PRIVPROTOCOL_NONE;
       else
	 cout << "Warning: ignoring unknown priv protocol: " << ptr << endl;
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
       if (( securityLevel < SNMP_SECURITY_LEVEL_NOAUTH_NOPRIV) ||
           ( securityLevel > SNMP_SECURITY_LEVEL_AUTH_PRIV))
         securityLevel = SNMP_SECURITY_LEVEL_AUTH_PRIV;
       continue;
     }
     if ( strstr( argv[x], "-sm")!=0) {
       ptr = argv[x]; ptr+=3;
       securityModel = atoi( ptr);
       if (( securityModel < SNMP_SECURITY_MODEL_V1) ||
           ( securityModel > SNMP_SECURITY_MODEL_USM))
         securityModel = SNMP_SECURITY_MODEL_USM;
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
   // bind to any port and use IPv6 if enabled
#ifdef SNMP_PP_IPv6
   snmp = new Snmp(status, 0, true);
#else
   snmp = new Snmp(status);
#endif

   if ( status != SNMP_CLASS_SUCCESS) {
     cout << "SNMP++ Session Create Fail, " 
	  << snmp->error_msg(status) << "\n";
     return -3;
   }

#ifdef _SNMPv3
   //---------[ init SnmpV3 ]--------------------------------------------
   v3MP *v3_MP;
   if (version == version3) {
     const char *engineId = "snmpWalk";
     const char *filename = "snmpv3_boot_counter";
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
     if (construct_status != SNMPv3_MP_OK)
     {
       cout << "Error initializing v3MP: " << construct_status << endl;
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

#ifdef _THREADS
  pthread_t thread[100];
  int started = threads;
#endif

  while (threads) {
#ifdef _THREADS
    if (!attr) {
      attr = new pthread_attr_t;
      pthread_attr_init(attr);
      pthread_attr_setdetachstate(attr, PTHREAD_CREATE_JOINABLE);
    }
    pthread_create(&thread[threads-1], 0, 
		   &runable,
		   (void*)new int(threads-1));
#else
    int n = threads - 1;
    runable(&n);
#endif
    threads--;
  }
#ifdef _THREADS
  // wait for threads to terminate
  for (int i=0; i<started; i++) {
    cout << "JOINING THREAD " << i << endl;
    pthread_join(thread[i], 0);
  }
#endif
  cout << "END" << endl;

#ifdef _SNMPv3
  delete v3_MP;
#endif

  Snmp::socket_cleanup();  // Shut down socket subsystem
}

