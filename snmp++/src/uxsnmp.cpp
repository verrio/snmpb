/*_############################################################################
  _## 
  _##  uxsnmp.cpp  
  _##
  _##  SNMP++v3.2.14
  _##  -----------------------------------------------
  _##  Copyright (c) 2001-2004 Jochen Katz, Frank Fock
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
  _##  Stuttgart, Germany, Tue Sep  7 21:25:32 CEST 2004 
  _##  
  _##########################################################################*/
/*===================================================================

      U X S N M P . C P P

      UXSNMP CLASS DECLARATION


      Description:
        HPUX version of Snmp class

      Language:
        ANSI C++

      Operating System:
        HP-UX 9.0

      Author:
        Peter E Mellquist
        HP RND R & D

=====================================================================*/

char snmp_cpp_version[]="#(@) SNMP++ $Id$";

/* CK Ng    added support for WIN32 in the whole file */

//-----[ includes ]----------------------------------------------------
#ifdef WIN32
#include <winsock.h>
#include <sys/types.h>     // system types
#include <sys/timeb.h>     // _timeb and _ftime
#else
#include <unistd.h>        // unix
#include <sys/socket.h>    // bsd socket stuff
#include <netinet/in.h>    // network types
#include <arpa/inet.h>     // arpa types
#include <sys/types.h>     // system types
#include <sys/time.h>      // time stuff
#endif
#ifdef _AIX
#define ss_family __ss_family
#endif

#include <stdlib.h>        // need for malloc
#include <errno.h>         // ux errs

#define _INCLUDE_SNMP_ERR_STRINGS

//----[ snmp++ includes ]----------------------------------------------
#include "snmp_pp/uxsnmp.h"        // class def for this module
#include "snmp_pp/oid_def.h"       // class def for well known trap oids
#include "snmp_pp/v3.h"
#include "snmp_pp/msgqueue.h"      // message queue
#include "snmp_pp/notifyqueue.h"   // notification queue
#include "snmp_pp/snmpmsg.h"       // asn serialization class
#include "snmp_pp/eventlistholder.h"
#include "snmp_pp/usm_v3.h"
#include "snmp_pp/vb.h"

#ifdef SNMP_PP_NAMESPACE
namespace Snmp_pp {
#endif

//-----[ special includes ]-------------------------------------------
extern "C"
{
  //------------[ if using Wind-U, then bring in the ms-windows header ]
#ifndef WIN32
#ifdef WU_APP
  WORD app_hinst;
#else
  typedef short WORD;
  typedef long DWORD;
#endif
#endif
}

//-----[ macros ]------------------------------------------------------
#define DEFAULT_TIMEOUT 1000  // one second default timeout
#define DEFAULT_RETRIES 1     // no retry default
#define SNMP_PORT 161         // port # for SNMP
#define SNMP_TRAP_PORT 162    // port # for SNMP traps

#ifdef WIN32
#ifdef __BCPLUSPLUS__
#define _timeb timeb
#define _ftime ftime
#endif
#define close closesocket
#endif

//--------[ globals ]---------------------------------------------------

//--------------[ well known trap ids ]-----------------------------------
const coldStartOid coldStart;
const warmStartOid warmStart;
const linkDownOid linkDown;
const linkUpOid linkUp;
const authenticationFailureOid authenticationFailure;
const egpNeighborLossOid egpNeighborLoss;
const snmpTrapEnterpriseOid snmpTrapEnterprise;


#ifdef _SNMPv3

SnmpSynchronized Snmp::v3Lock;

void v3CallBack( int reason, Snmp *snmp, Pdu &pdu, SnmpTarget &target, void *v3cd)
{
  Snmp::V3CallBackData *cbData;
  cbData = (Snmp::V3CallBackData*)v3cd;

  Vb tmpvb;
  pdu.get_vb(tmpvb,0);

  debugprintf(5, "v3CallBack: received oid: %s with value: %s",
              tmpvb.get_printable_oid(), tmpvb.get_printable_value());
  debugprintf(5, "v3CallBack: error_msg (%s), pdu_type (%i)",
	      snmp->error_msg(tmpvb.get_oid()), pdu.get_type());

  if ((pdu.get_type() == REPORT_MSG) &&
      ((tmpvb.get_oid() == oidUsmStatsUnknownEngineIDs) ||
       (tmpvb.get_oid() == oidUsmStatsNotInTimeWindows))) {
    // hide those reports from user
    int rc;
    if ((cbData->pdu) && (cbData->target)) {
      rc = snmp->snmp_engine(*(cbData->pdu), cbData->non_reps,
                             cbData->max_reps, *(cbData->target),
                             (snmp_callback)(cbData->oldCallback),
                             (void *)cbData->cbd);
      debugprintf(3,"v3CallBack: snmp_engine called, rc (%i)", rc);
    }
    else
      rc = SNMP_CLASS_ERROR;

    if (rc != SNMP_CLASS_SUCCESS) {
      // call callback if snmp_engine failed or pdu or target was 0
      debugprintf(3,"v3CallBack: calling user callback");
      snmp_callback tmp_callBack;
      tmp_callBack = (snmp_callback)(cbData->oldCallback);
      tmp_callBack(rc, snmp, pdu, target, (void *)cbData->cbd);
    }
  }
  else {
    debugprintf(3,"v3CallBack: calling user callback");
    snmp_callback tmp_callBack;
    tmp_callBack = (snmp_callback)(cbData->oldCallback);
    tmp_callBack(reason, snmp, pdu, target, (void *)cbData->cbd);
  }
  // save to delete it here, because either snmp_engine created a new
  // callback entry or the user specified callback has been called
  if (cbData->pdu) {
    delete cbData->pdu;
    cbData->pdu = 0;
  }
  if (cbData->target) {
    delete cbData->target;
    cbData->target = 0;
  }
  delete cbData;
  return;
}

#endif

//--------[ make the pdu request id ]-----------------------------------
// return a unique rid, clock can be too slow , so use current_rid
long Snmp::MyMakeReqId()
{
  long rid;
  eventListHolder->snmpEventList()->lock();
  do {
    rid = ++current_rid;

#ifdef INVALID_REQID
    debugprintf(-10, "\nWARNING: Using constand RequestID!\n");
    rid = 0xc0de;
#endif

    if ( current_rid > PDU_MAX_RID)
    {
      current_rid = rid = PDU_MIN_RID;
      // let other tasks proceed
      eventListHolder->snmpEventList()->unlock();
      struct timeval tv;
      tv.tv_sec = 0;
      tv.tv_usec = 100;
      select(0, 0, 0, 0, &tv);
      eventListHolder->snmpEventList()->lock();
    }
  } while (eventListHolder->snmpEventList()->GetEntry(rid));
  eventListHolder->snmpEventList()->unlock();

  return rid;
}

//---------[ Send SNMP Request ]---------------------------------------
// Send out a snmp request
DLLOPT int send_snmp_request(int sock, unsigned char *send_buf,
                             size_t send_len, Address & address)
{
  // UX only supports UDP type addresses (addr and port) right now
  if (address.get_type() != Address::type_udp)
    return -1;// unsupported address type

  debugprintf(1, "++ SNMP++: sending to %s:",
              ((UdpAddress &)address).UdpAddress::get_printable());
  debughexprintf(5, send_buf, send_len);

  int send_result;

  if (((UdpAddress &)address).get_ip_version() == Address::version_ipv4)
  {
    // prepare the destination address
    struct sockaddr_in agent_addr;  // send socket struct
    memset(&agent_addr, 0, sizeof(agent_addr));
    agent_addr.sin_family = AF_INET;
    agent_addr.sin_addr.s_addr
              = inet_addr(((IpAddress &)address).IpAddress::get_printable());
    agent_addr.sin_port = htons(((UdpAddress &)address).get_port());

    send_result = sendto( sock, (char*) send_buf, send_len, 0,
                          (struct sockaddr*) &agent_addr, sizeof(agent_addr));
  }
  else
  {
#ifdef SNMP_PP_IPv6
    struct sockaddr_in6 agent_addr;
    memset(&agent_addr, 0, sizeof(agent_addr));
    inet_pton(AF_INET6, ((IpAddress &)address).IpAddress::get_printable(),
              &agent_addr.sin6_addr);
    agent_addr.sin6_family = AF_INET6;
    agent_addr.sin6_port = htons(((UdpAddress &)address).get_port());

    send_result = sendto( sock, (char*) send_buf, send_len, 0,
                          (struct sockaddr*) &agent_addr, sizeof(agent_addr));
#else
    debugprintf(0, "User error: Enable IPv6 and recompile snmp++.");
    return -1;
#endif
  }

  if (send_result < 0)
  {
    debugprintf(0, "Error sending packet: %s", strerror(errno));
    return -1; // send error!
  }

  return 0;
}

//---------[ receive a snmp response ]---------------------------------
// Receive a response from the specified socket.
// This function does not set the request id in the pdu if
// any error occur in receiving or parsing.  This is important
// because the caller initializes this to zero and checks it to
// see whether it has been changed to a valid value.  The
// return value is the normal PDU status or SNMP_CLASS_SUCCESS.
// when we are successful in receiving a pdu.  Otherwise it
// is an error status.

int receive_snmp_response(int sock, Snmp &snmp_session,
                          Pdu &pdu, UdpAddress &fromaddress,
			  bool process_msg = true)
{
  unsigned char receive_buffer[MAX_SNMP_PACKET];
  long receive_buffer_len; // len of received data
#ifdef SNMP_PP_IPv6
  struct sockaddr_storage from_addr;
#else
  struct sockaddr_in from_addr;
#endif
#if defined __GNUC__ || defined __FreeBSD__ || defined _AIX
  socklen_t fromlen;
#else
  int fromlen;
#endif
  fromlen = sizeof(from_addr);

  // do the read
  do {
    receive_buffer_len = (long) recvfrom(sock, (char *) receive_buffer,
                                         MAX_SNMP_PACKET, 0,
                                         (struct sockaddr*) &from_addr,
                                         &fromlen);
    debugprintf(2, "++ SNMP++: something received...");
  } while ((receive_buffer_len < 0) && (EINTR == errno));

  if (receive_buffer_len <= 0 )                // error or no data pending
    return SNMP_CLASS_TL_FAILED;
  debugprintf(6, "Length received %i from socket %i; fromlen %i",
              receive_buffer_len, sock, fromlen);

  if (((sockaddr_in&)from_addr).sin_family == AF_INET)
  {
    // IPv4
    fromaddress = inet_ntoa(((sockaddr_in&)from_addr).sin_addr);
    fromaddress.set_port(ntohs(((sockaddr_in&)from_addr).sin_port));
  }
#ifdef SNMP_PP_IPv6
  else if (from_addr.ss_family == AF_INET6)
  {
    // IPv6
    char tmp_buffer[INET6_ADDRSTRLEN+1];

    inet_ntop(AF_INET6, &(((sockaddr_in6&)from_addr).sin6_addr),
              tmp_buffer, INET6_ADDRSTRLEN);

    fromaddress = tmp_buffer;
    fromaddress.set_port(ntohs(((sockaddr_in6&)from_addr).sin6_port));
  }
#endif // SNMP_PP_IPv6
  else
  {
    debugprintf(0, "Unknown socket address family (%i).",
                ((sockaddr_in&)from_addr).sin_family);
    return SNMP_CLASS_ERROR;
  }

  debugprintf(1, "++ SNMP++: data received from %s.",
              fromaddress.get_printable());
  debughexprintf(5, receive_buffer, receive_buffer_len);

  if (process_msg == false)
    return SNMP_CLASS_SUCCESS;   // return success

  SnmpMessage snmpmsg;
  if ( snmpmsg.load( receive_buffer, receive_buffer_len) != SNMP_CLASS_SUCCESS)
    return SNMP_CLASS_ERROR;

  OctetStr community_name;
  snmp_version version;
  OctetStr engine_id;
  OctetStr security_name;

#ifdef _SNMPv3
  long int security_model;
  if (snmpmsg.is_v3_message() == TRUE)
  {
    int returncode = snmpmsg.unloadv3(pdu, version, engine_id,
                                      security_name, security_model,
                                      fromaddress, snmp_session);
    if (returncode != SNMP_CLASS_SUCCESS)
      return returncode;
  }
  else
  {
#endif
    int returncode = snmpmsg.unload( pdu, community_name, version);
    if (returncode != SNMP_CLASS_SUCCESS)
      return SNMP_CLASS_ERROR;
#ifdef _SNMPv3
  }
  if (version == version3) {
    Snmp::v3Lock.lock();
    v3MP::I->add_to_engine_id_table(engine_id,
			       (char*)fromaddress.IpAddress::get_printable(),
			       fromaddress.get_port());
    Snmp::v3Lock.unlock();

    debugprintf(4,"receive_snmp_response: engine_id (%s), security_name (%s), "
		"security_model (%i), security_level (%i)",
                engine_id.get_printable(), security_name.get_printable(),
                security_model, pdu.get_security_level());
    debugprintf(5," addtoengineidtable: (%s)",
                (unsigned char*)fromaddress.get_printable());
  }
#endif

  //-----[ check for error status stuff..]
  // an error status is a valid pdu,
  // the caller needs to know about it
  if ( pdu.get_error_status() != 0)
    return pdu.get_error_status();

  debugprintf(5,"receive_snmp_response requestID = %li, "
	      "returning SUCCESS.", pdu.get_request_id());

  return SNMP_CLASS_SUCCESS;   // Success! return
}


//---------[ receive a snmp trap ]---------------------------------
// Receive a trap from the specified socket
// note: caller has to delete target!
int receive_snmp_notification(int sock, Snmp &snmp_session,
                              Pdu &pdu, SnmpTarget **target)
{
  unsigned char receive_buffer[MAX_SNMP_PACKET];
  long receive_buffer_len; // len of received data

#ifdef SNMP_PP_IPv6
  struct sockaddr_storage from_addr;
#else
  struct sockaddr_in from_addr;
#endif // SNMP_PP_IPv6

#if defined __GNUC__ || defined __FreeBSD__ || defined _AIX
  socklen_t fromlen;
#else
  int fromlen;
#endif
  fromlen = sizeof(from_addr);

  // do the read
  do {
    receive_buffer_len = (long) recvfrom(sock, (char *) receive_buffer,
                                         MAX_SNMP_PACKET, 0,
                                         (struct sockaddr*)&from_addr,
                                         &fromlen);
  } while (receive_buffer_len < 0 && EINTR == errno);

  if (receive_buffer_len <= 0 )                // error or no data pending
    return SNMP_CLASS_TL_FAILED;

  // copy fromaddress and remote port
  UdpAddress fromaddress;

  if (((sockaddr_in&)from_addr).sin_family == AF_INET)
  {
    // IPv4
    fromaddress = inet_ntoa(((sockaddr_in&)from_addr).sin_addr);
    fromaddress.set_port(ntohs(((sockaddr_in&)from_addr).sin_port));
  }
#ifdef SNMP_PP_IPv6
  else if (from_addr.ss_family == AF_INET6)
  {
    // IPv6
    char tmp_buffer[INET6_ADDRSTRLEN+1];

    inet_ntop(AF_INET6, &(((sockaddr_in6&)from_addr).sin6_addr),
              tmp_buffer, INET6_ADDRSTRLEN);

    fromaddress = tmp_buffer;
    fromaddress.set_port(ntohs(((sockaddr_in6&)from_addr).sin6_port));
  }
#endif // SNMP_PP_IPv6
  else
  {
    debugprintf(0, "Unknown socket address family (%i).",
                ((sockaddr_in&)from_addr).sin_family);
    return SNMP_CLASS_TL_FAILED;
  }

  debugprintf(1, "++ SNMP++: data received from %s.",
              fromaddress.get_printable());
  debughexprintf(5, receive_buffer, receive_buffer_len);

  SnmpMessage snmpmsg;
  if ( snmpmsg.load( receive_buffer, receive_buffer_len) != SNMP_CLASS_SUCCESS)
    return SNMP_CLASS_ERROR;

  OctetStr community_name;
  snmp_version version;
  OctetStr engine_id;
  OctetStr security_name;

#ifdef _SNMPv3
  long int security_model;
  if (snmpmsg.is_v3_message() == TRUE)
  {
    int returncode = snmpmsg.unloadv3(pdu, version, engine_id,
                                      security_name, security_model,
                                      fromaddress, snmp_session);
    if (returncode != SNMP_CLASS_SUCCESS)
      return returncode;
  }
  else
  {
#endif
    int returncode = snmpmsg.unload( pdu, community_name, version);
    if (returncode != SNMP_CLASS_SUCCESS)
      return SNMP_CLASS_ERROR;
#ifdef _SNMPv3
  }

  if (version == version3) {
    *target = new UTarget();
    (*target)->set_address(fromaddress);
    (*target)->set_version(version);
    ((UTarget*)*target)->set_engine_id(engine_id);
    ((UTarget*)*target)->set_security_name(security_name);
    ((UTarget*)*target)->set_security_model(security_model);
    Snmp::v3Lock.lock();
    v3MP::I->add_to_engine_id_table(engine_id,
                         (char*)((IpAddress &)fromaddress).get_printable(),
                         fromaddress.get_port());
    Snmp::v3Lock.unlock();

    debugprintf(4,"receive_snmp_notification: engine_id (%s), security_name "
		"(%s), security_model (%i), security_level (%i)",
                engine_id.get_printable(), security_name.get_printable(),
                security_model, pdu.get_security_level());
  }
  else
  {
#endif
    *target = new CTarget();
    (*target)->set_version(version);
    (*target)->set_address(fromaddress);
    ((CTarget*)*target)->set_readcommunity( community_name);
    ((CTarget*)*target)->set_writecommunity( community_name);
#ifdef _SNMPv3
  }
#endif
  return SNMP_CLASS_SUCCESS;   // Success! return
}


//--------[ map action ]------------------------------------------------
// map the snmp++ action to a SMI pdu type
void Snmp::map_action( unsigned short action, unsigned short & pdu_action)
{
  switch( action)
    {
    case sNMP_PDU_GET:
    case sNMP_PDU_GET_ASYNC:
      pdu_action = sNMP_PDU_GET;
      break;

    case sNMP_PDU_SET:
    case sNMP_PDU_SET_ASYNC:
      pdu_action = sNMP_PDU_SET;
      break;

    case sNMP_PDU_GETNEXT:
    case sNMP_PDU_GETNEXT_ASYNC:
      pdu_action = sNMP_PDU_GETNEXT;
      break;

    case sNMP_PDU_GETBULK:
    case sNMP_PDU_GETBULK_ASYNC:
      pdu_action = sNMP_PDU_GETBULK;
      break;

    case sNMP_PDU_RESPONSE:
      pdu_action = sNMP_PDU_RESPONSE;
      break;

    case sNMP_PDU_INFORM:
    case sNMP_PDU_INFORM_ASYNC:
      pdu_action = sNMP_PDU_INFORM;
      break;

    case sNMP_PDU_REPORT:
      pdu_action = sNMP_PDU_REPORT;
      break;

    default:
      pdu_action = sNMP_PDU_GET;  // TM ?? error ??
      break;

    };  // end switch
}

//------[ Snmp Class Constructor ]--------------------------------------

Snmp::Snmp(int &status,  const unsigned short port,
           const bool bind_ipv6) : SnmpSynchronized()
{
  IpAddress *addresses[2];

  if (bind_ipv6)
  {
    listen_address = "::";

    addresses[0] = NULL;
    addresses[1] = &listen_address;

    init(status, addresses, 0, port);
  }
  else
  {
    listen_address = "0.0.0.0";

    addresses[0] = &listen_address;
    addresses[1] = NULL;

    init(status, addresses, port, 0);
  }

}

Snmp::Snmp( int &status,  const UdpAddress& addr): SnmpSynchronized()
{
  IpAddress *addresses[2];

  listen_address = addr;

  if (listen_address.get_ip_version() == Address::version_ipv4)
  {
    addresses[0] = &listen_address;
    addresses[1] = NULL;
    init(status, addresses, addr.get_port(), 0);
  }
  else
  {
    addresses[0] = NULL;
    addresses[1] = &listen_address;
    init(status, addresses, 0, addr.get_port());
  }
}

Snmp::Snmp( int &status,  const UdpAddress& addr_v4,
            const UdpAddress& addr_v6): SnmpSynchronized()
{
  IpAddress *addresses[2];

  listen_address = addr_v4;
  IpAddress address_v6((IpAddress)addr_v6);
  addresses[0] = &listen_address;
  addresses[1] = &address_v6;

  init(status, addresses, addr_v4.get_port(), addr_v6.get_port());
}

void Snmp::socket_startup()
{
#ifdef WIN32
  WSADATA WSAData;
  (void)WSAStartup(0x0101, &WSAData);
#endif
}

void Snmp::socket_cleanup()
{
#ifdef WIN32
  int iRetValue = WSACleanup();
  debugprintf(4, "WSACleanup: ReturnValue (%i)", iRetValue);
#endif
}

void Snmp::init(int& status, IpAddress *addresses[2],
                const unsigned short port_v4,
                const unsigned short port_v6)
{
  eventListHolder = new EventListHolder(this);
  // initialize the request_id
  eventListHolder->snmpEventList()->lock();
//  srand(time(0)); // better than nothing
  current_rid = (rand() % (PDU_MAX_RID - PDU_MIN_RID -1)) + PDU_MIN_RID;
  debugprintf(4, "Initialized request_id to %i.", current_rid);
  eventListHolder->snmpEventList()->unlock();

  // intialize all the trap receiving member variables
  notifycallback = 0;
  notifycallback_data = 0;
#ifdef HPUX
  int errno = 0;
#endif

  construct_status = SNMP_CLASS_ERROR;
  construct_status_ipv6 = SNMP_CLASS_ERROR;
  status = SNMP_CLASS_ERROR;
  iv_snmp_session = INVALID_SOCKET;
  iv_snmp_session_ipv6 = INVALID_SOCKET;

  /* Open IPv4 socket */
  if (addresses[0])
  {
    struct sockaddr_in mgr_addr;

    // open a socket to be used for the session
    if (( iv_snmp_session = socket( AF_INET, SOCK_DGRAM,0)) == INVALID_SOCKET)
    {
#ifdef WIN32
      int werr = WSAGetLastError();
      if (EMFILE == werr ||WSAENOBUFS == werr || ENFILE == werr)
        status = SNMP_CLASS_RESOURCE_UNAVAIL;
      else if (WSAEHOSTDOWN == werr)
        status = SNMP_CLASS_TL_FAILED;
      else
        status = SNMP_CLASS_TL_UNSUPPORTED;
#else
      if (EMFILE == errno || ENOBUFS == errno || ENFILE == errno)
        status = SNMP_CLASS_RESOURCE_UNAVAIL;
      else if (EHOSTDOWN == errno)
        status = SNMP_CLASS_TL_FAILED;
      else
        status = SNMP_CLASS_TL_UNSUPPORTED;
#endif
    }
    else
    {
      // set up the manager socket attributes
      unsigned long inaddr = inet_addr(addresses[0]->get_printable());
      memset(&mgr_addr, 0, sizeof(mgr_addr));
      mgr_addr.sin_family = AF_INET;
      mgr_addr.sin_addr.s_addr = inaddr;
      mgr_addr.sin_port = htons( port_v4);
      // bind the socket
      if ( bind( (int) iv_snmp_session, (struct sockaddr*) &mgr_addr,
                 sizeof(mgr_addr)) < 0)
      {
#ifdef WIN32
        int werr = WSAGetLastError();
        if (WSAEADDRINUSE  == werr)
          status = SNMP_CLASS_TL_IN_USE;
        else if (WSAENOBUFS == werr)
          status = SNMP_CLASS_RESOURCE_UNAVAIL;
        else if (werr == WSAEAFNOSUPPORT)
          status = SNMP_CLASS_TL_UNSUPPORTED;
        else if (werr == WSAENETUNREACH || werr == EACCES)
          status = SNMP_CLASS_TL_FAILED;
        else
          status = SNMP_CLASS_INTERNAL_ERROR;
#else
        if (EADDRINUSE  == errno)
          status = SNMP_CLASS_TL_IN_USE;
        else if (ENOBUFS == errno)
          status = SNMP_CLASS_RESOURCE_UNAVAIL;
        else if (errno == EAFNOSUPPORT)
          status = SNMP_CLASS_TL_UNSUPPORTED;
        else if (errno == ENETUNREACH || errno == EACCES)
          status = SNMP_CLASS_TL_FAILED;
        else
	{
	  debugprintf(0, "Uncatched errno value %d, returning internal error.",
		      errno);
          status = SNMP_CLASS_INTERNAL_ERROR;
	}
#endif
      }
      else
      {
        status = SNMP_CLASS_SUCCESS;
#ifdef SNMP_BROADCAST
        int enable_broadcast = 1;
        setsockopt((int)iv_snmp_session, SOL_SOCKET, SO_BROADCAST,
                   (char*)&enable_broadcast, sizeof(enable_broadcast));
#endif
#ifdef SNMPX11
        // Tell X11 to watch our file descriptor
        status = eventListHolder->SnmpX11AddInput((int)iv_snmp_session,
						  pdu_handler);
#endif // SNMPX11
      }
    }
    construct_status = status;
    if (status != SNMP_CLASS_SUCCESS)
      return;
  }

  /* Open IPv6 socket */
  if (addresses[1])
  {
#ifdef SNMP_PP_IPv6
    // open a socket to be used for the session
    if (( iv_snmp_session_ipv6 = socket( AF_INET6, SOCK_DGRAM,0)) 
	== INVALID_SOCKET)
    {
#ifdef WIN32
      int werr = WSAGetLastError();
      if (EMFILE == werr ||WSAENOBUFS == werr || ENFILE == werr)
        status = SNMP_CLASS_RESOURCE_UNAVAIL;
      else if (WSAEHOSTDOWN == werr)
        status = SNMP_CLASS_TL_FAILED;
      else
        status = SNMP_CLASS_TL_UNSUPPORTED;
#else
      if (EMFILE == errno || ENOBUFS == errno || ENFILE == errno)
        status = SNMP_CLASS_RESOURCE_UNAVAIL;
      else if (EHOSTDOWN == errno)
        status = SNMP_CLASS_TL_FAILED;
      else
        status = SNMP_CLASS_TL_UNSUPPORTED;
#endif
    }
    else
    {
      // set up the manager socket attributes
      struct sockaddr_in6 mgr_addr;

      memset(&mgr_addr, 0, sizeof(mgr_addr));
      inet_pton(AF_INET6, addresses[1]->get_printable(), &mgr_addr.sin6_addr);
      mgr_addr.sin6_family = AF_INET6;
      mgr_addr.sin6_port = htons( port_v6);
      // bind the socket
      if ( bind( (int) iv_snmp_session_ipv6, (struct sockaddr*) &mgr_addr,
                 sizeof(mgr_addr)) < 0)
      {
#ifdef WIN32
        int werr = WSAGetLastError();
        if (WSAEADDRINUSE  == werr)
          status = SNMP_CLASS_TL_IN_USE;
        else if (WSAENOBUFS == werr)
          status = SNMP_CLASS_RESOURCE_UNAVAIL;
        else if (werr == WSAEAFNOSUPPORT)
          status = SNMP_CLASS_TL_UNSUPPORTED;
        else if (werr == WSAENETUNREACH || werr == EACCES)
          status = SNMP_CLASS_TL_FAILED;
        else
          status = SNMP_CLASS_INTERNAL_ERROR;
#else
        if (EADDRINUSE  == errno)
          status = SNMP_CLASS_TL_IN_USE;
        else if (ENOBUFS == errno)
          status = SNMP_CLASS_RESOURCE_UNAVAIL;
        else if (errno == EAFNOSUPPORT)
          status = SNMP_CLASS_TL_UNSUPPORTED;
        else if (errno == ENETUNREACH || errno == EACCES)
          status = SNMP_CLASS_TL_FAILED;
        else
          status = SNMP_CLASS_INTERNAL_ERROR;
#endif
      }
      else
      {
        status = SNMP_CLASS_SUCCESS;
#ifdef SNMP_BROADCAST
        int enable_broadcast = 1;
        setsockopt((int)iv_snmp_session_ipv6, SOL_SOCKET, SO_BROADCAST,
                   (char*)&enable_broadcast, sizeof(enable_broadcast));
#endif
#ifdef SNMPX11
        // Tell X11 to watch our file descriptor
        status = eventListHolder->SnmpX11AddInput((int)iv_snmp_session_ipv6,
						  pdu_handler_ipv6);
#endif // SNMPX11
      }
    }
    construct_status_ipv6 = status;
#else
    construct_status_ipv6 = SNMP_CLASS_ERROR;
    debugprintf(0, "User error: Enable IPv6 and recompile snmp++.");
#endif // SNMP_PP_IPv6
  }
  return;
}


//---------[ Snmp Class Destructor ]----------------------------------
Snmp::~Snmp()
{
  // if we failed during construction then don't try
  // to free stuff up that was not allocated
  if ( construct_status == SNMP_CLASS_SUCCESS)
  {
#ifdef SNMPX11
    // Tell X11 to stop watching our file descriptor
    eventListHolder->SnmpX11RemoveInput(pdu_handler);
#endif // SNMPX11
    // go through the snmpEventList and delete any outstanding
    // events on this socket
    eventListHolder->snmpEventList()->DeleteSocketEntry((int)iv_snmp_session);

    close( (int) iv_snmp_session);    // close the dynamic socket
  }
  // if we failed during construction then don't try
  // to free stuff up that was not allocated
  if ( construct_status_ipv6 == SNMP_CLASS_SUCCESS)
  {
#ifdef SNMPX11
    // Tell X11 to stop watching our file descriptor
    eventListHolder->SnmpX11RemoveInput(pdu_handler_ipv6);
#endif // SNMPX11

    // go through the snmpEventList and delete any outstanding
    // events on this socket
    eventListHolder->snmpEventList()->DeleteSocketEntry((int)iv_snmp_session_ipv6);

    close( (int) iv_snmp_session_ipv6);    // close the dynamic socket
  }

  if ((construct_status != SNMP_CLASS_SUCCESS) &&
      ( construct_status != SNMP_CLASS_SUCCESS))
    return;

  // shut down trap reception if used
  notify_unregister();

  delete eventListHolder;
}

// Get the version of the snmp++ library at runtime
// This function MUST stay in the cpp file!
const char *Snmp::get_version()
{
  return SNMP_PP_VERSION_STRING;
}

//-------------------[ returns error string ]--------------------------
const char *Snmp::error_msg(const int c)
{
#ifdef _SNMPv3
  if (c>=SNMPv3_USM_MIN_ERROR)
    return ((c>SNMPv3_USM_MAX_ERROR)?pv3Errs[SNMPv3_USM_ERRORCOUNT]:pv3Errs[c-SNMPv3_USM_MIN_ERROR]);
  if (c<=SNMPv3_MP_MAX_ERROR)
    return ((c<SNMPv3_MP_MIN_ERROR)?nv3Errs[SNMPv3_MP_ERRORCOUNT]:nv3Errs[SNMPv3_MP_MAX_ERROR - c]);
#endif
  return ((c<0)?
          ((c<MAX_NEG_ERROR)?nErrs[-(MAX_NEG_ERROR)+1]:nErrs[-c]):
          ((c>MAX_POS_ERROR)?pErrs[MAX_POS_ERROR+1]:pErrs[c]));
}

#ifdef _SNMPv3
const char* Snmp::error_msg(const Oid& v3Oid)
{
  // UsmStats
  if (v3Oid == oidUsmStatsUnsupportedSecLevels)
    return error_msg(SNMPv3_USM_UNSUPPORTED_SECURITY_LEVEL);

  if (v3Oid == oidUsmStatsNotInTimeWindows)
    return error_msg(SNMPv3_USM_NOT_IN_TIME_WINDOW);

  if (v3Oid == oidUsmStatsUnknownUserNames )
    return error_msg(SNMPv3_USM_UNKNOWN_SECURITY_NAME);

  if (v3Oid == oidUsmStatsUnknownEngineIDs)
    return error_msg(SNMPv3_USM_UNKNOWN_ENGINEID);

  if (v3Oid == oidUsmStatsWrongDigests)
    return error_msg(SNMPv3_USM_AUTHENTICATION_FAILURE);

  if (v3Oid == oidUsmStatsDecryptionErrors)
    return error_msg(SNMPv3_USM_DECRYPTION_ERROR);

  // MPDstats
  if (v3Oid == oidSnmpUnknownSecurityModels)
    return error_msg(SNMPv3_MP_UNSUPPORTED_SECURITY_MODEL);

  if (v3Oid == oidSnmpInvalidMsgs)
    return error_msg(SNMPv3_MP_INVALID_MESSAGE);

  if (v3Oid == oidSnmpUnknownPDUHandlers)
    return error_msg(SNMPv3_MP_UNKNOWN_PDU_HANDLERS);

  if (v3Oid == oidSnmpUnavailableContexts)
    return error_msg(SNMPv3_MP_UNAVAILABLE_CONTEXT);

  if (v3Oid == oidSnmpUnknownContexts)
    return error_msg(SNMPv3_MP_UNKNOWN_CONTEXT);

  return error_msg(MAX_POS_ERROR + 1);
}
#endif

//------------------------[ get ]---------------------------------------
int Snmp::get(Pdu &pdu, const SnmpTarget &target)
{
  pdu.set_type( sNMP_PDU_GET);
  return snmp_engine( pdu, 0, 0, target, NULL, 0);
}

//------------------------[ get async ]----------------------------------
int Snmp::get(Pdu &pdu, const SnmpTarget &target,
              const snmp_callback callback,
              const void * callback_data)
{
  pdu.set_type( sNMP_PDU_GET_ASYNC);
  return snmp_engine( pdu, 0, 0, target, callback, callback_data);
}

//------------------------[ get next ]-----------------------------------
int Snmp::get_next(Pdu &pdu, const SnmpTarget &target)
{
  pdu.set_type( sNMP_PDU_GETNEXT);
  return snmp_engine( pdu, 0, 0, target, NULL, 0);
}

//------------------------[ get next async ]-----------------------------
int Snmp::get_next(Pdu &pdu, const SnmpTarget &target,
                   const snmp_callback callback,
                   const void * callback_data)
{
  pdu.set_type( sNMP_PDU_GETNEXT_ASYNC);
  return snmp_engine( pdu, 0, 0, target, callback, callback_data);
}

//-------------------------[ set ]---------------------------------------
int Snmp::set(Pdu &pdu, const SnmpTarget &target)
{
  pdu.set_type( sNMP_PDU_SET);
  return snmp_engine( pdu, 0, 0, target, NULL, 0);
}

//------------------------[ set async ]----------------------------------
int Snmp::set(Pdu &pdu, const SnmpTarget &target,
              const snmp_callback callback,
              const void * callback_data)
{
  pdu.set_type( sNMP_PDU_SET_ASYNC);
  return snmp_engine( pdu, 0, 0, target, callback, callback_data);
}

//-----------------------[ get bulk ]------------------------------------
int Snmp::get_bulk(Pdu &pdu,                // pdu to use
                   const SnmpTarget &target,// destination target
                   const int non_repeaters, // number of non repeaters
                   const int max_reps)      // maximum number of repetitions
{
  pdu.set_type( sNMP_PDU_GETBULK);
  return snmp_engine( pdu, non_repeaters, max_reps, target, NULL, 0);
}

//-----------------------[ get bulk async ]------------------------------
int Snmp::get_bulk(Pdu &pdu,                 // pdu to use
                   const SnmpTarget &target, // destination target
                   const int non_repeaters,  // number of non repeaters
                   const int max_reps,       // maximum number of repetitions
                   const snmp_callback callback,// callback to use
                   const void * callback_data)  // callback data
{
  pdu.set_type( sNMP_PDU_GETBULK_ASYNC);
  return snmp_engine( pdu, non_repeaters, max_reps, target,
                      callback, callback_data);
}

//------------------------[ inform_response ]----------------------------
int Snmp::response(Pdu &pdu,                 // pdu to use
                   const SnmpTarget &target) // response target
{
  pdu.set_type( sNMP_PDU_RESPONSE);
  return snmp_engine( pdu, 0, 0, target, NULL, 0);
}

int Snmp::send_raw_data(unsigned char *send_buf,
                        size_t send_len, UdpAddress &address, int fd)
  REENTRANT(
{
  if (fd > 0)
    return send_snmp_request(fd, send_buf, send_len, address);
  else
  {
    if (address.get_ip_version() == Address::version_ipv4)
    {
      if (iv_snmp_session != INVALID_SOCKET)
	return send_snmp_request((int) iv_snmp_session, send_buf,
				 send_len, address);
      else
	address.map_to_ipv6();
    }
    return send_snmp_request((int) iv_snmp_session_ipv6, send_buf,
			     send_len, address);
  }
})

//-----------------------[ cancel ]--------------------------------------
int Snmp::cancel( const unsigned long request_id)
{
  eventListHolder->snmpEventList()->lock();
  int status = eventListHolder->snmpEventList()->DeleteEntry(request_id);
  eventListHolder->snmpEventList()->unlock();

  return status;
}


//----------------------[ sending report, V3 only]-----------------------
int Snmp::report(Pdu &pdu,                // pdu to send
                 const SnmpTarget &target)// destination target
{
  pdu.set_type( sNMP_PDU_REPORT);
  return snmp_engine( pdu, 0, 0, target, NULL, 0);
}

//----------------------[ blocking inform, V2 only]------------------------
int Snmp::inform(Pdu &pdu,                // pdu to send
                 const SnmpTarget &target)// destination target
{
  pdu.set_type( sNMP_PDU_INFORM);
  check_notify_timestamp(pdu);
  return snmp_engine( pdu, 0, 0, target, NULL, 0);
}

//----------------------[ asynch inform, V2 only]------------------------
int Snmp::inform(Pdu &pdu,                // pdu to send
                 const SnmpTarget &target,      // destination target
                 const snmp_callback callback,  // callback function
                 const void * callback_data)    // callback data
{
  pdu.set_type( sNMP_PDU_INFORM);
  check_notify_timestamp(pdu);
  return snmp_engine( pdu, 0, 0, target, callback, callback_data);
}


//---------------------[ send a trap ]-----------------------------------
int Snmp::trap(Pdu &pdu,                        // pdu to send
               const SnmpTarget &target)        // destination target
{
  OctetStr my_get_community;
  OctetStr my_set_community;
  GenAddress address;
  unsigned long my_timeout;
  int my_retry;
  unsigned char version;
  int status;

  debugprintf(1, "++ SNMP++, Send a Trap");
  //---------[ make sure pdu is valid ]---------------------------------
  if ( !pdu.valid())
    {
      debugprintf(0, "-- SNMP++, PDU Object Invalid");
      return  SNMP_CLASS_INVALID_PDU;
    }

  //---------[ make sure target is valid ]------------------------------
  if ( !target.valid())
    {
      debugprintf(0, "-- SNMP++, Target Object Invalid");
      return SNMP_CLASS_INVALID_TARGET;
    }

  CTarget* ctarget = NULL;
  UTarget* utarget = NULL;
  OctetStr security_name;
  int security_model;

  switch (target.get_type()) {
    case SnmpTarget::type_ctarget:
      ctarget = (CTarget*)(&target);
      break;
    case SnmpTarget::type_utarget:
      utarget = (UTarget*)(&target);
      break;
    case SnmpTarget::type_base:
      debugprintf(0, "-- SNMP++, do not use SnmpTarget, use a  CTarget or UTarget");
      return SNMP_CLASS_INVALID_TARGET;
    default:
      // target is not known
      debugprintf(0, "-- SNMP++, type of target is unknown!");
      return SNMP_CLASS_UNSUPPORTED;
  }

  if (ctarget) {
    debugprintf(3, "snmp::trap called with CTarget");
    if (!ctarget->resolve_to_C( my_get_community, my_set_community, address,
                                my_timeout, my_retry, version))
    {
      debugprintf(0, "-- SNMP++, Resolve Fail (CTarget)");
      return SNMP_CLASS_UNSUPPORTED;
    }
#ifdef _SNMPv3
    if (version == version3)
    {
      debugprintf(0, "-- SNMP++, use UTarget for SNMPv3");
      return SNMP_CLASS_INVALID_TARGET;
    }
#endif
  }
  else { // target is not a CTarget:
    if (utarget) {
      debugprintf(3, "trap called with UTarget");
      if (!utarget->resolve_to_U( security_name, security_model, address,
				  my_timeout, my_retry, version))
      {
        debugprintf(0, "-- SNMP++, Resolve Fail (UTarget)");
        return SNMP_CLASS_UNSUPPORTED;
      }
#ifdef _SNMPv3
      if (version != version3) {
#endif
        my_get_community = security_name;
        if ((security_model != SecurityModel_v1) &&
            (security_model != SecurityModel_v2)) {
          debugprintf(0, "-- SNMP++, Target contains invalid security_model/version combination");
          return SNMP_CLASS_INVALID_TARGET;
        }
#ifdef _SNMPv3
      } // end if (version != version3)
#endif
    }
    else { // target is neither CTarget nor UTarget:
      debugprintf(0, "-- SNMP++, Resolve Fail");
      return SNMP_CLASS_INVALID_TARGET;
    }
  }

  //--------[ determine request id to use ]------------------------------
  pdu.set_request_id( MyMakeReqId());

  //--------[ check timestamp, if null use system time ]-----------------
  check_notify_timestamp(pdu);

  //------[ validate address to use ]-------------------------------------
  if (!address.valid()) {
    debugprintf(0, "-- SNMP++, Bad address");
    return SNMP_CLASS_INVALID_TARGET;
  }

  if ((address.get_type() != Address::type_ip) &&
      (address.get_type() != Address::type_udp) )
    {
      debugprintf(0, "-- SNMP++, Bad address type");
      return SNMP_CLASS_TL_UNSUPPORTED;
    }

  UdpAddress udp_address(address);
  if (!udp_address.valid()) {
    debugprintf(0, "-- SNMP++, copy address failed");
    return SNMP_CLASS_RESOURCE_UNAVAIL;
  }

  //----------[ choose the target address port ]-----------------------
  if ((address.get_type() == Address::type_ip) || !udp_address.get_port())
    udp_address.set_port(SNMP_TRAP_PORT);

  //----------[ based on the target type, choose v1 or v1 trap type ]-----
  if ( version == version1)
    pdu.set_type( sNMP_PDU_V1TRAP);
  else // v2 and v3 use v2TRAP
    pdu.set_type( sNMP_PDU_TRAP);

  SnmpMessage snmpmsg;

#ifdef _SNMPv3
  if ( version == version3) {

    OctetStr engine_id = v3MP::I->get_local_engine_id();
    if (!utarget) {
      debugprintf(0, "-- SNMP++, dont know how to handle SNMPv3 without UTarget!");
      return SNMP_CLASS_INVALID_TARGET;
    }

    // set context_engine_id of pdu, if it is not set
    if (pdu.get_context_engine_id().len() == 0)
    {
      debugprintf(8, "Setting contextEngineID of Pdu to (%s)",
                  engine_id.get_printable());
      pdu.set_context_engine_id(engine_id);
    }

    debugprintf(4,"Snmp::trap:");
    debugprintf(4," engineID (%s), securityName (%s)\n securityModel (%i) security_level (%i)",
                engine_id.get_printable(), security_name.get_printable(),
                security_model, pdu.get_security_level());
    debugprintf(4," Addr/Port (%s)",udp_address.get_printable());

    status = snmpmsg.loadv3( pdu, engine_id, security_name,
                             security_model, (snmp_version)version);
  }
  else
#endif
    status = snmpmsg.load( pdu, my_get_community, (snmp_version) version);

  if ( status != SNMP_CLASS_SUCCESS) {
    debugprintf(0, "snmp message load error!");
    return status;
  }

  lock();
  //------[ send the trap ]
  if (udp_address.get_ip_version() == Address::version_ipv4)
  {
    if (iv_snmp_session != INVALID_SOCKET)
      status = send_snmp_request((int) iv_snmp_session,
				 snmpmsg.data(), (size_t)snmpmsg.len(),
				 udp_address);
    else
    {
      udp_address.map_to_ipv6();
      status = send_snmp_request((int) iv_snmp_session_ipv6,
				 snmpmsg.data(), (size_t)snmpmsg.len(),
				 udp_address);
    }
  }
  else
    status = send_snmp_request((int) iv_snmp_session_ipv6,
                               snmpmsg.data(), (size_t)snmpmsg.len(),
                               udp_address);

  unlock();
  if (status != 0)
    return SNMP_CLASS_TL_FAILED;

  return SNMP_CLASS_SUCCESS;
}

//----------------[ set notify_timestamp if it is null ]-------------
void Snmp::check_notify_timestamp(Pdu &pdu)
{
  // As we don't know, when the application was started,
  // use a continuously increasing notify_timestamp
  TimeTicks timestamp;
  pdu.get_notify_timestamp( timestamp);
  if (timestamp <= 0) {
#ifdef WIN32
    struct _timeb timebuffer;
    _ftime( &timebuffer );
    timebuffer.time -= 989518848;   // knock off 31 years worth of seconds
    timestamp = (timebuffer.time * 100) + (timebuffer.millitm / 10);
#else
    struct timeval tp;
    gettimeofday(&tp, NULL);
    tp.tv_sec -= 989518848;   // knock off 31 years worth of seconds
    timestamp = (tp.tv_sec * 100) + (tp.tv_usec / 10000);
#endif
    pdu.set_notify_timestamp( timestamp);
  }
}

//-----------------------[ read the notification filters ]----------------
int Snmp::get_notify_filter( OidCollection &trapids,
                             TargetCollection &targets,
			     AddressCollection &listen_addresses)
{
  CNotifyEvent *e = eventListHolder->notifyEventList()->GetEntry(this);

  if (!e)  return SNMP_CLASS_INVALID;

  e->get_filter(trapids, targets, listen_addresses);

  return SNMP_CLASS_SUCCESS;
}



//-----------------------[ register to get traps]-------------------------
int Snmp::notify_register( const OidCollection     &trapids,
                           const TargetCollection  &targets,
                           const AddressCollection &addresses,
                           const snmp_callback      callback,
                           const void              *callback_data)
{
  int status;

  // assign callback and callback data info
  notifycallback = callback;
  notifycallback_data = (void *)callback_data;

  // remove any previous filters for this session
  eventListHolder->notifyEventList()->DeleteEntry(this);

  // add to the notify queue
  status = eventListHolder->notifyEventList()->AddEntry(this, trapids,
							targets, addresses);

  iv_notify_fd = eventListHolder->notifyEventList()->get_notify_fd(); // DLD

  return status;
}

// alternate form for local listen specification
//-----------------------[ register to get traps]-------------------------
int Snmp::notify_register( const OidCollection    &trapids,
                           const TargetCollection &targets,
                           const snmp_callback     callback,
                           const void             *callback_data)
{
  AddressCollection addresses;

  return notify_register(trapids, targets, addresses,
                         callback, callback_data);
}

//-----------------------[ un-register to get traps]----------------------
int Snmp::notify_unregister()
{
  /// @todo Return error if not registered before!

  // null out callback information
  notifycallback = 0;
  notifycallback_data = 0;

  // remove from the notify queue
  eventListHolder->notifyEventList()->DeleteEntry(this);

  iv_notify_fd = -1; // DLD

  return SNMP_CLASS_SUCCESS;
}

//---------[ get / set engine ]-----------------------------------------
// The main snmp engine used for all requests
// async requests return out early and don't wait in here for
// the response
int Snmp::snmp_engine( Pdu &pdu,              // pdu to use
                       long int non_reps,     // # of non repititions
                       long int max_reps,     // # of max repititions
                       const SnmpTarget &target,    // from this target
                       const snmp_callback cb,// callback for async calls
                       const void * cbd)      // callback data

{
  long req_id;                       // pdu request id
  int status;                        // send status

#ifdef _SNMPv3
  // save original PDU for later reference
  Pdu backupPdu = pdu;

  for (int maxloops=0; maxloops<3; maxloops++)
  {
#endif

    unsigned short pdu_action;        // type of pdu to build
    unsigned short action;        // type of pdu to build
    unsigned long my_timeout;        // target specific timeout
    int my_retry;                // target specific retry

    OctetStr my_get_community;
    OctetStr my_set_community;
    GenAddress address;
    unsigned char version;

    //---------[ make sure pdu is valid ]--------------------------
    if ( !pdu.valid())
      return  SNMP_CLASS_INVALID_PDU;

    //---------[ depending on user action, map the correct pdu action]
    action = pdu.get_type();
    map_action(action, pdu_action);

    //---------[ check for correct mode ]---------------------------
    // if the class was constructed as a blocked model, callback=0
    // and async calls are attempted, an error is returned
    if (( cb == 0) &&
        ((action == sNMP_PDU_GET_ASYNC) ||
         (action == sNMP_PDU_SET_ASYNC) ||
         (action == sNMP_PDU_GETNEXT_ASYNC) ||
         (action == sNMP_PDU_GETBULK_ASYNC) ||
         (action == sNMP_PDU_INFORM_ASYNC)))
      return SNMP_CLASS_INVALID_CALLBACK;

    //---------[ more mode checking ]--------------------------------
    // if the class was constructed as an async model, callback = something
    // and blocked calls are attempted, an error is returned
    if (( cb != 0) &&
        ((action == sNMP_PDU_GET) ||
         (action == sNMP_PDU_SET) ||
         (action == sNMP_PDU_GETNEXT) ||
         (action == sNMP_PDU_GETBULK) ||
         (action == sNMP_PDU_INFORM)))
      return SNMP_CLASS_INVALID_CALLBACK;

    //---------[ make sure target is valid ]-------------------------
    // make sure that the target is valid
    if ( ! target.valid())
      return SNMP_CLASS_INVALID_TARGET;

    OctetStr community_string;
    OctetStr security_name;
    int security_model;
    const CTarget* ctarget = NULL;
    const UTarget* utarget = NULL;

    switch (target.get_type())
    {
      case SnmpTarget::type_ctarget:
        ctarget = (CTarget*)(&target);
        break;
      case SnmpTarget::type_utarget:
        utarget = (UTarget*)(&target);
        break;
      case SnmpTarget::type_base:
        debugprintf(0, "-- SNMP++, do not use SnmpTarget,"
                    "use a  CTarget or UTarget");
        return SNMP_CLASS_INVALID_TARGET;
      default:
        /* target is not known */
        debugprintf(0, "-- SNMP++, type of target is unknown!");
        return SNMP_CLASS_UNSUPPORTED;
    }

    if (ctarget) /* Is is a CTarget? */
    {
      debugprintf(3, "snmp_engine called with CTarget");
      if (!ctarget->resolve_to_C( my_get_community, my_set_community,
                                  address, my_timeout, my_retry, version))
      {
        debugprintf(0, "-- SNMP++, Resolve Fail (CTarget)");
        return SNMP_CLASS_UNSUPPORTED;
      }
#ifdef _SNMPv3
      if ((version == version3) ||
          (action == sNMP_PDU_REPORT))
      {
        debugprintf(0, "-- SNMP++, use UTarget for SNMPv3");
        return SNMP_CLASS_INVALID_TARGET;
      }
#endif
      //----------[ use the appropriate community string ]-----------------
      if (( action == sNMP_PDU_GET) ||
          ( action == sNMP_PDU_GET_ASYNC) ||
          ( action == sNMP_PDU_GETNEXT) ||
          ( action == sNMP_PDU_GETNEXT_ASYNC) ||
          ( action == sNMP_PDU_GETBULK) ||
          ( action == sNMP_PDU_GETBULK_ASYNC) ||
          ( action == sNMP_PDU_INFORM) ||
          ( action == sNMP_PDU_INFORM_ASYNC) ||
          ( action == sNMP_PDU_RESPONSE))
        community_string = my_get_community;
      else /* got to be a set */
        community_string = my_set_community;
    }
    else if (utarget)  /* Is is a UTarget? */
    {
      debugprintf(3, "snmp_engine called with UTarget");
      if (!utarget->resolve_to_U( security_name, security_model,
                                  address, my_timeout,
                                  my_retry, version))
      {
        debugprintf(0, "-- SNMP++, Resolve Fail (UTarget)");
        return SNMP_CLASS_UNSUPPORTED;
      }
#ifdef _SNMPv3
      if (version != version3)
      {
#endif
        community_string = security_name;
        if ((security_model != SecurityModel_v1) &&
            (security_model != SecurityModel_v2)) {
          debugprintf(0, "-- SNMP++, Target contains invalid security_model/version combination");
          return SNMP_CLASS_INVALID_TARGET;
        }
#ifdef _SNMPv3
      } // end if (version != version3)
#endif
    }
    else
    { // target is neither CTarget nor UTarget (should not happen)
      debugprintf(0, "-- SNMP++, Resolve Fail");
      return SNMP_CLASS_INVALID_TARGET;
    }

    if (!address.valid())
    {
      debugprintf(0, "-- SNMP++, Target contains invalid address");
      return SNMP_CLASS_INVALID_TARGET;
    }

    //----------[ validate the target address ]--------------------------
    if ((address.get_type() != Address::type_ip) &&
        (address.get_type() != Address::type_udp) )
    {
      debugprintf(0, "-- SNMP++, Bad address type");
      return SNMP_CLASS_TL_UNSUPPORTED;
    }

    UdpAddress udp_address(address);
    if (!udp_address.valid())
    {
      debugprintf(0, "-- SNMP++, Bad address");
      return SNMP_CLASS_RESOURCE_UNAVAIL;
    }

    //----------[ choose the target address port ]-----------------------
    if ((address.get_type() == Address::type_ip) || !udp_address.get_port())
    {
      if (pdu_action == sNMP_PDU_INFORM)
        udp_address.set_port(SNMP_TRAP_PORT);
      else
        udp_address.set_port(SNMP_PORT);
    }
    // otherwise port was already set

    if ((pdu_action != sNMP_PDU_RESPONSE) &&
        (pdu_action != sNMP_PDU_REPORT))
    {
      // set error index to none
      pdu.set_error_index(0);

      // determine request id to use
      req_id = MyMakeReqId();
      pdu.set_request_id(req_id);
    }

    //---------[ map GetBulk over v1 to GetNext ]-------------------------
    if (( pdu_action == sNMP_PDU_GETBULK)&&( (snmp_version)version== version1))
      pdu_action = sNMP_PDU_GETNEXT;
    if ( pdu_action == sNMP_PDU_GETBULK) {
      pdu.set_error_status((int) non_reps);
      pdu.set_error_index((int) max_reps);
    }

    pdu.set_type( pdu_action);
    SnmpMessage snmpmsg;

#ifdef _SNMPv3
    if (version == version3)
    {
      if (!utarget)
      {
        debugprintf(0, "-- SNMP++, need UTarget to send SNMPv3 message!");
        return SNMP_CLASS_INVALID_TARGET;
      }
      OctetStr engine_id;
      utarget->get_engine_id(engine_id);
      if (engine_id.len() == 0)
      {
        v3Lock.lock();
        if (v3MP::I->get_from_engine_id_table(engine_id,
					    (char*)udp_address.get_printable())
            == SNMPv3_MP_OK )
        {
	  // Override const here
          ((UTarget*)utarget)->set_engine_id(engine_id);
        } // else: engineID="" --> automatic engineID discovery
        v3Lock.unlock();
      }
      // set context_engine_id of pdu, if it is not set
      if (pdu.get_context_engine_id().len() == 0)
      {
        debugprintf(8, "Setting contextEngineID of Pdu to (%s)",
                    engine_id.get_printable());
        pdu.set_context_engine_id(engine_id);
        backupPdu.set_context_engine_id(engine_id);
      }

      debugprintf(4,"Snmp::snmp_engine:");
      debugprintf(4," engineID (%s), securityName (%s)\n"
                  "securityModel (%i) security_level (%i)",
                  engine_id.get_printable(), security_name.get_printable(),
                  security_model, pdu.get_security_level());
      debugprintf(4," Addr/Port (%s)",udp_address.get_printable());

      // TODO: fine grain locking within mpV3
      v3Lock.lock(); // lock for possible side effects on v3 cache information
      status = snmpmsg.loadv3( pdu, engine_id, security_name,
                               security_model, (snmp_version)version);
      v3Lock.unlock();
    }
    else
#endif
      status = snmpmsg.load( pdu, community_string,(snmp_version) version);

    if ( status != SNMP_CLASS_SUCCESS)
    {
      debugprintf(0, "snmp message load error!");
      return status;
    }

    lock();
    int iv_session_used;

    //------[ send the request ]
    if (udp_address.get_ip_version() == Address::version_ipv4)
    {
      if (iv_snmp_session != INVALID_SOCKET)
      {
	status = send_snmp_request((int) iv_snmp_session,
				   snmpmsg.data(), (size_t) snmpmsg.len(),
				   udp_address);
	iv_session_used = iv_snmp_session;
      }
      else
      {
	udp_address.map_to_ipv6();
	status = send_snmp_request((int) iv_snmp_session_ipv6,
				   snmpmsg.data(), (size_t) snmpmsg.len(),
				   udp_address);
	iv_session_used = iv_snmp_session_ipv6;
      }
    }
    else
    {
      status = send_snmp_request((int) iv_snmp_session_ipv6,
                                 snmpmsg.data(), (size_t) snmpmsg.len(),
                                 udp_address);
      iv_session_used = iv_snmp_session_ipv6;
    }
    unlock();

    if ( status != 0)
      return SNMP_CLASS_TL_FAILED;

    if ((pdu_action == sNMP_PDU_RESPONSE) ||
        (pdu_action == sNMP_PDU_REPORT))
      return SNMP_CLASS_SUCCESS; // don't wait for an answer

#ifdef _SNMPv3
    if ((version == version3) && ((action == sNMP_PDU_GET_ASYNC) ||
                                  (action == sNMP_PDU_SET_ASYNC) ||
                                  (action == sNMP_PDU_GETNEXT_ASYNC) ||
                                  (action == sNMP_PDU_GETBULK_ASYNC) ||
                                  (action == sNMP_PDU_INFORM_ASYNC))) {
      // add callback for v3
      struct V3CallBackData *v3CallBackData;
      v3CallBackData = new struct V3CallBackData;

      v3CallBackData->pdu = new Pdu(pdu);
      v3CallBackData->pdu->set_type(backupPdu.get_type());
      v3CallBackData->non_reps = non_reps;
      v3CallBackData->max_reps = max_reps;

      v3CallBackData->target = new UTarget(*utarget);
      v3CallBackData->oldCallback = cb;
      v3CallBackData->cbd = cbd;

      // Add the message to the message queue
      eventListHolder->snmpEventList()->AddEntry(req_id, this, iv_session_used,
						 target, pdu,
						 snmpmsg.data(),
						 (size_t) snmpmsg.len(),
						 udp_address,
						 v3CallBack,
						 (void *)v3CallBackData);
    }
    else
#endif
    {
      eventListHolder->snmpEventList()->AddEntry( req_id, this,
						  (int)iv_session_used,
						  target, pdu,
						  snmpmsg.data(),
						  (size_t) snmpmsg.len(),
						  udp_address,
						  cb, (void *)cbd);
    }
    //----[ if an async mode request then return success ]-----
    if (( action == sNMP_PDU_GET_ASYNC) ||
        ( action == sNMP_PDU_SET_ASYNC) ||
        ( action == sNMP_PDU_GETNEXT_ASYNC) ||
        ( action == sNMP_PDU_GETBULK_ASYNC) ||
        ( action == sNMP_PDU_INFORM_ASYNC))
      return SNMP_CLASS_SUCCESS;

    // Now wait for the response (or timeout) for our message.
    // This handles any necessary retries.
    status = eventListHolder->SNMPBlockForResponse(req_id, pdu);

    if (pdu.get_type() != REPORT_MSG) {
#ifdef _SNMPv3
      if (status == SNMPv3_MP_OK)
        return SNMP_CLASS_SUCCESS;
      else
#endif
        return status;
    }
#ifdef _SNMPv3
    else
      if (status == SNMPv3_USM_DECRYPTION_ERROR)
        return status;

    // We received a REPORT-MSG, check if we should try another time
    Vb first_vb;
    Oid first_oid;
    pdu.get_vb(first_vb,0);
    first_vb.get_oid(first_oid);

    debugprintf(1,"received oid: %s with value: %s",
                first_vb.get_printable_oid(), first_vb.get_printable_value());
    debugprintf(1, "%s", error_msg(first_oid));

    switch (maxloops)
    {
      case 0:
      {
	// This was our first try, so we may receive a unknown engine id 
	// report or a not in time window report
        if (first_oid == oidUsmStatsUnknownEngineIDs)
        {
	  pdu = backupPdu; // restoere pdu and try again
	  break;
        }
        else if (first_oid == oidUsmStatsNotInTimeWindows)
        {
          ++maxloops; // increase it, as the next request must succeed
	  pdu = backupPdu; // restore pdu and try again
	  break;
        }
        return (status == SNMPv3_MP_OK) ? SNMP_CLASS_SUCCESS : status;
      }
      case 1:
      {
	// This was the second try, engine id discovery should be ok
	// so test only for not in time report
        if (first_oid == oidUsmStatsNotInTimeWindows)
        {
	  pdu = backupPdu; // restore pdu and try again
	  break;
        }
        return (status == SNMPv3_MP_OK) ? SNMP_CLASS_SUCCESS : status;
      }
      case 2:
      {
	// We tried three times: one for engine id discovery, one for
	// time sync and we still get a report --> somethings wrong!
        return (status == SNMPv3_MP_OK) ? SNMP_CLASS_SUCCESS : status;
      }
    }
  }
#endif
  return status;
}

// Send a SNMP Broadcast message.
int Snmp::broadcast_discovery(UdpAddressCollection &addresses,
			      const int timeout_sec,
			      const UdpAddress &addr,
			      const snmp_version version,
			      const OctetStr *community)
{
  unsigned char *message;
  int message_length;
  int socket;
  SnmpMessage snmpmsg;

#ifdef _SNMPv3
  unsigned char snmpv3_broadcast_message[60] = {
    0x30, 0x3a,
          0x02, 0x01, 0x03,                   // Version: 3
          0x30, 0x0f,                         // global header length 15
                0x02, 0x03, 0x01, 0x00, 0x00, // message id
                0x02, 0x02, 0x10, 0x00,       // message max size
                0x04, 0x01, 0x04,             // flags (reportable set)
                0x02, 0x01, 0x03,             // security model USM
          0x04, 0x10,                         // security params
                0x30, 0x0e,
                      0x04, 0x00,             // no engine id
                      0x02, 0x01, 0x00,       // boots 0
                      0x02, 0x01, 0x00,       // time 0
                      0x04, 0x00,             // no user name
                      0x04, 0x00,             // no auth par
                      0x04, 0x00,             // no priv par
          0x30, 0x12,
                0x04, 0x00,                   // no context engine id
                0x04, 0x00,                   // no context name
          0xa0, 0x0c,                         // GET PDU
                0x02, 0x02, 0x34, 0x26,       // request id
                0x02, 0x01, 0x00,             // error status no error
                0x02, 0x01, 0x00,             // error index 0
                0x30, 0x00                    // no data
  };

  if (version == version3)
  {
    message = (unsigned char *)snmpv3_broadcast_message;
    message_length = 60;
  }
  else
#endif
  {
    Pdu pdu;
    Vb vb;
    OctetStr get_community;

    vb.set_oid("1.3.6.1.2.1.1.1.0");
    pdu +=vb;
    pdu.set_error_index(0);            // set error index to none
    pdu.set_request_id(MyMakeReqId()); // determine request id to use
    pdu.set_type(sNMP_PDU_GET);        // set pdu type

    if (community)
      get_community = *community;
    else
      get_community = "public";

    int status = snmpmsg.load(pdu, get_community, version);
    if (status != SNMP_CLASS_SUCCESS)
    {
      debugprintf(0, "Error encoding broadcast pdu (%i).", status);
      return status;
    }
    message        = snmpmsg.data();
    message_length = snmpmsg.len();
  }

  UdpAddress uaddr(addr);

  if (uaddr.get_ip_version() == Address::version_ipv4)
  {
    if (iv_snmp_session != INVALID_SOCKET)
      socket = (int) iv_snmp_session;
    else
    {
      uaddr.map_to_ipv6();
      socket = (int) iv_snmp_session_ipv6;
    }
  }
  else
    socket = (int) iv_snmp_session_ipv6;

  lock();
  if (send_snmp_request(socket, message, message_length, uaddr) < 0)
  {
    debugprintf(0, "Error sending broadast.");
    unlock();
    return SNMP_CLASS_TL_FAILED;
  }

  // now wait for the responses
  Pdu dummy_pdu;
  fd_set readfds;
  int nfound = 0;
  struct timeval fd_timeout;
  msec end_time;

  end_time += timeout_sec * 1000;

  do
  {
    FD_ZERO(&readfds);

    FD_SET(socket, &readfds);

    end_time.GetDeltaFromNow(fd_timeout);

    nfound = select(socket + 1, &readfds, NULL, NULL, &fd_timeout);

    if ((nfound > 0) && (FD_ISSET(socket, &readfds)))
    {
      // receive message
      UdpAddress from;
      if (receive_snmp_response(socket, *this, dummy_pdu,
				from, false /* process_msg */)
	  == SNMP_CLASS_SUCCESS)
      {
	addresses += from;
      }
      else
      {
	debugprintf(0, "Error receiving broadcast response.");
      }
    }
  } while ((nfound > 0) ||
	   (fd_timeout.tv_sec > 0) || (fd_timeout.tv_usec > 0));
  unlock();

#ifdef __DEBUG
  for (int i=0; i < addresses.size(); ++i)
  {
    debugprintf(3, "Broadcast response received from (%s).",
		addresses[i].get_printable());
  }
#endif
  return 0;
}

#ifdef SNMP_PP_NAMESPACE
}; // end of namespace Snmp_pp
#endif 
