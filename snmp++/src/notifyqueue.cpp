/*_############################################################################
  _## 
  _##  notifyqueue.cpp  
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

  Copyright (c) 1999
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

      U S E R T I M E O U T . C P P

      CNotifyEventQueue CLASS DEFINITION

      COPYRIGHT HEWLETT PACKARD COMPANY 1999

      INFORMATION NETWORKS DIVISION

      NETWORK MANAGEMENT SECTION


      DESIGN + AUTHOR:
        Tom Murray

      LANGUAGE:
        ANSI C++

      OPERATING SYSTEMS:
        DOS/WINDOWS 3.1
        BSD UNIX

      DESCRIPTION:
        Queue for holding callback associated with user defined
        timeouts


      COMPILER DIRECTIVES:
        UNIX - For UNIX build

=====================================================================*/
char notifyqueue_version[]="#(@) SNMP++ $Id$";

//-----[ includes ]----------------------------------------------------
#ifdef WIN32
#include <winsock.h>
#endif
#include <errno.h>

//----[ snmp++ includes ]----------------------------------------------

#include "snmp_pp/config_snmp_pp.h"
#include "snmp_pp/v3.h"
#include "snmp_pp/notifyqueue.h" // queue for holding sessions waiting for async notifications
#include "snmp_pp/eventlistholder.h"
#include "snmp_pp/uxsnmp.h"
#include "snmp_pp/snmperrs.h"
#include "snmp_pp/pdu.h"

#ifdef SNMP_PP_NAMESPACE
namespace Snmp_pp {
#endif

//--------[ externs ]---------------------------------------------------
extern int receive_snmp_notification(int sock, Snmp &snmp_session,
                                     Pdu &pdu, SnmpTarget **target);

//-----[ macros ]------------------------------------------------------
// should be in snmp.h...
#define SNMP_PORT 161	      // standard port # for SNMP
#define SNMP_TRAP_PORT 162    // standard port # for SNMP traps

#ifdef WIN32
#define close closesocket
#elif _AIX
#include <unistd.h>
#endif

//----[ CNotifyEvent class ]------------------------------------------------

CNotifyEvent::CNotifyEvent(Snmp *snmp,
			   const OidCollection &trapids,
			   const TargetCollection &targets,
			   const AddressCollection &addresses)
  : m_snmp(snmp)
{
  // create new collections using parms passed in
  notify_ids       = new OidCollection( trapids);
  notify_targets   = new TargetCollection( targets);
  notify_addresses = new AddressCollection( addresses);
}

CNotifyEvent::~CNotifyEvent()
{
  // free up local collections
  if (notify_ids)       { delete notify_ids;       notify_ids       = 0; }
  if (notify_targets)   { delete notify_targets;   notify_targets   = 0; }
  if (notify_addresses) { delete notify_addresses; notify_addresses = 0; }
}

int CNotifyEvent::notify_filter(const Oid &trapid, SnmpTarget &target) const
{
  int target_count, has_target = FALSE, target_matches = FALSE;
  int trapid_count, has_trapid = FALSE, trapid_matches = FALSE;
  GenAddress targetaddr, tmpaddr;

  // figure out how many targets, handle empty case as all targets
  if ((notify_targets) && (target_count = notify_targets->size()))
  {
    SnmpTarget *tmptarget = 0;
    has_target = TRUE;

    target.get_address(targetaddr);

    if (targetaddr.valid()) {
      // loop through all targets in the collection
      SnmpTarget::target_type target_type = target.get_type();
      SnmpTarget::target_type tmptarget_type;

      for ( int x = 0; x < target_count; x++)       // for all targets
      {
	if (notify_targets->get_element(tmptarget, x))
	  continue;

	tmptarget->get_address(tmpaddr);
	if ((tmpaddr.valid())) {
          int addr_equal = 0;

          /* check for types of Address */
          if ((tmpaddr.get_type() == Address::type_ip) &&
              (targetaddr.get_type() == Address::type_udp))
          {
            /* special case that works for UdpAddress == IpAddress */
            IpAddress ip1(targetaddr);
            IpAddress ip2(tmpaddr);

            addr_equal = (ip1.valid() && ip2.valid() && (ip1 == ip2));
          }
          else
          {
            addr_equal = (targetaddr == tmpaddr);
          }

          if (addr_equal) {
            tmptarget_type = tmptarget->get_type();
            if (target_type == SnmpTarget::type_utarget) {
              // target is a UTarget
              if (tmptarget_type == SnmpTarget::type_utarget) {
                // both are UTarget
                if ((((UTarget*)(&target))->get_security_name() ==
                     ((UTarget*)tmptarget)->get_security_name()) &&
                    (((UTarget*)(&target))->get_security_model() ==
                     ((UTarget*)tmptarget)->get_security_model())) {
		  target_matches = TRUE;
                  break;
                }
              }
              else
                if (tmptarget_type == SnmpTarget::type_ctarget)
                  // in case utarget is used with v1 or v2:
                  if ((tmptarget->get_version() == target.get_version()) &&
                      (((UTarget*)(&target))->get_security_name() ==
                       OctetStr(((CTarget*)tmptarget)->
                                get_readcommunity()))) {
                    target_matches = TRUE;
                    break;
                  }
            }
            else {
              if (target_type == SnmpTarget::type_ctarget) {
                // target is a CTarget
                if (tmptarget_type == SnmpTarget::type_ctarget) {
                  // both are CTarget
                  if (!strcmp(((CTarget*)(&target))->get_readcommunity(),
                              ((CTarget*)tmptarget)->get_readcommunity())) {
                    target_matches = TRUE;
                    break;
                  }
                }
                else
                  if (tmptarget_type == SnmpTarget::type_utarget) {
                    if ((tmptarget->get_version() == target.get_version()) &&
                        (OctetStr(((CTarget*)(&target))->get_readcommunity()) ==
                         ((UTarget*)tmptarget)->get_security_name())) {
                      target_matches = TRUE;
                      break;
                    }
                  }
              }
            }
          } // end if (add_equal)
        } // end if tmpaddr.valid()...
      }
    }
  }
  // else no targets means all targets

  // figure out how many trapids, handle empty case as all trapids
  if ((notify_ids) && (trapid_count = notify_ids->size())) {
    Oid tmpoid;
    has_trapid = TRUE;
    // loop through all trapids in the collection
    for (int y=0; y < trapid_count; y++)       // for all trapids
    {
      if (notify_ids->get_element(tmpoid, y))
	continue;
      if (trapid == tmpoid) {
	trapid_matches = TRUE;
	break;
      }
    }
  }
  // else no trapids means all traps

  // Make the callback if the trap passed the filters
  if ((has_target && !target_matches) || (has_trapid && !trapid_matches))
    return FALSE;
  return TRUE;
}


int CNotifyEvent::Callback(SnmpTarget &target, Pdu &pdu, int status)
{
  Oid trapid;
  int reason;
  pdu.get_notify_id(trapid);

  // Make the callback if the trap passed the filters
  if ((m_snmp) && (notify_filter(trapid, target)))
  {
    if (SNMP_CLASS_TL_FAILED == status)
      reason = SNMP_CLASS_TL_FAILED;
    else
      reason = SNMP_CLASS_NOTIFICATION;

    //------[ call into the callback function ]-------------------------
    (m_snmp->get_notify_callback())(
      reason,
      m_snmp,			// snmp++ session who owns the req
      pdu,			// trap pdu
      target,			// target
      m_snmp->get_notify_callback_data()); // callback data
  }
  return SNMP_CLASS_SUCCESS;
}


//----[ CNotifyEventQueueElt class ]--------------------------------------

CNotifyEventQueue::CNotifyEventQueueElt::CNotifyEventQueueElt(
                                           CNotifyEvent *notifyevent,
					   CNotifyEventQueueElt *next,
					   CNotifyEventQueueElt *previous)
  : m_notifyevent(notifyevent), m_next(next), m_previous(previous)
{
  /* Finish insertion into doubly linked list */
  if (m_next)     m_next->m_previous = this;
  if (m_previous) m_previous->m_next = this;
}

CNotifyEventQueue::CNotifyEventQueueElt::~CNotifyEventQueueElt()
{
  /* Do deletion form doubly linked list */
  if (m_next)        m_next->m_previous = m_previous;
  if (m_previous)    m_previous->m_next = m_next;
  if (m_notifyevent) delete m_notifyevent;
}

CNotifyEvent *CNotifyEventQueue::CNotifyEventQueueElt::TestId(Snmp *snmp)
{
  if (m_notifyevent && (m_notifyevent->GetId() == snmp))
    return m_notifyevent;
  return 0;
}


//----[ CNotifyEventQueue class ]--------------------------------------
int CNotifyEventQueue::m_listen_port = SNMP_TRAP_PORT;

CNotifyEventQueue::CNotifyEventQueue(EventListHolder *holder, Snmp *session)
  : m_head(NULL,NULL,NULL), m_msgCount(0), m_notify_fd(-1),
    my_holder(holder), m_snmpSession(session)
{
//TM: could do the trap registration setup here but seems better to
//wait until the app actually requests trap receives by calling
//notify_register().
}

CNotifyEventQueue::~CNotifyEventQueue()
{
  CNotifyEventQueueElt *leftOver;

  /* walk the list deleting any elements still on the queue */
  lock();
  while ((leftOver = m_head.GetNext()))
    delete leftOver;
  unlock();
}

int CNotifyEventQueue::AddEntry(Snmp *snmp,
				const OidCollection &trapids,
				const TargetCollection &targets,
				const AddressCollection &addresses)
{
  if (snmp != m_snmpSession)
  {
    debugprintf(0, "WARNING: Adding notification event for other Snmp object");
  }

  lock();
  if (!m_msgCount)
  {
    // This is the first request to receive notifications
    // Set up the socket for the snmp trap port (162) or the
    // specified port through set_listen_port()
    struct sockaddr_in mgr_addr;

    // open a socket to be used for the session
    if (( m_notify_fd = (int) socket( AF_INET, SOCK_DGRAM,0)) < 0) {
      unlock();
      return SNMP_CLASS_TL_FAILED;
    }
    else
    {
      // set up the manager socket attributes
      unsigned long inaddr = inet_addr(snmp->get_listen_address().get_printable());
      memset(&mgr_addr, 0, sizeof(mgr_addr));
      mgr_addr.sin_family = AF_INET;
      mgr_addr.sin_addr.s_addr = inaddr; // was htonl( INADDR_ANY);
      mgr_addr.sin_port = htons( m_listen_port);

      // bind the socket
      if (bind(m_notify_fd, (struct sockaddr *) &mgr_addr,
	       sizeof(mgr_addr)) < 0)
      {
        debugprintf(0, "Fatal: could not bind to port %d", m_listen_port);
	close(m_notify_fd);
	m_notify_fd = -1;

        unlock();
	// TODO: should probably check errno...
	return SNMP_CLASS_TL_IN_USE;
      }
#ifdef SNMPX11
      // Tell X11 to watch our file descriptor
      my_holder->SnmpX11AddInput(m_notify_fd, m_inputId);
#endif // SNMPX11
    }
  }


  CNotifyEvent *newEvent = new CNotifyEvent(snmp, trapids, targets, addresses);

    /*---------------------------------------------------------*/
    /* Insert entry at head of list, done automagically by the */
    /* constructor function, so don't use the return value.    */
    /*---------------------------------------------------------*/
  (void) new CNotifyEventQueueElt(newEvent, m_head.GetNext(), &m_head);
  m_msgCount++;
  unlock();
  return SNMP_CLASS_SUCCESS;
}


CNotifyEvent *CNotifyEventQueue::GetEntry(Snmp * snmp) REENTRANT ({
  CNotifyEventQueueElt *msgEltPtr = m_head.GetNext();
  CNotifyEvent *returnVal = NULL;

  while (msgEltPtr){
    if ((returnVal = msgEltPtr->TestId(snmp)))
      return returnVal;
    msgEltPtr = msgEltPtr->GetNext();
  }
  return 0;
})

void CNotifyEventQueue::DeleteEntry(Snmp * snmp) {
  lock();
  CNotifyEventQueueElt *msgEltPtr = m_head.GetNext();

  while (msgEltPtr){
    if (msgEltPtr->TestId(snmp)){
      delete msgEltPtr;
      m_msgCount--;
      break;
    }
    msgEltPtr = msgEltPtr->GetNext();
  }

  // shut down the trap socket (if valid) if not using it.
  if ((m_msgCount <= 0) && (m_notify_fd >= 0))
  {
    close(m_notify_fd);
    m_notify_fd = -1;
#ifdef SNMPX11
    // Tell X11 to stop watching our file descriptor
    my_holder->SnmpX11RemoveInput(m_inputId);
#endif // SNMPX11
  }
  unlock();
}

void CNotifyEventQueue::GetFdSets(int &maxfds,
				  fd_set &readfds,
				  fd_set &/*writefds*/,
				  fd_set &/*exceptfds*/) REENTRANT ({
  if (m_notify_fd >= 0) {
    FD_SET(m_notify_fd, &readfds);
    if (maxfds < (m_notify_fd+1))
      maxfds = m_notify_fd+1;
  }
  return;
})

int CNotifyEventQueue::HandleEvents(const int /*maxfds*/,
				    const fd_set &readfds,
				    const fd_set &/*writefds*/,
				    const fd_set &/*exceptfds*/) REENTRANT ({
  Pdu pdu;
  SnmpTarget *target = NULL;
  CNotifyEventQueueElt *notifyEltPtr = m_head.GetNext();
  int status = SNMP_CLASS_SUCCESS;

  if (m_notify_fd < 0)
    return status;

  // pull the notifiaction off the socket
  if (FD_ISSET(m_notify_fd, &readfds)) {
    status = receive_snmp_notification(m_notify_fd, *m_snmpSession,
                                       pdu, &target);

    if (SNMP_CLASS_SUCCESS == status ||
	SNMP_CLASS_TL_FAILED == status) {
      // If we have transport layer failure, the app will want to
      // know about it.
      // Go through each snmp object and check the filters, making
      // callbacks as necessary

      // LiorK: on failure target will be NULL
      if (!target)
        target = new SnmpTarget();

      while (notifyEltPtr){

	notifyEltPtr->GetNotifyEvent()->Callback(*target, pdu,
						 status);
	notifyEltPtr = notifyEltPtr->GetNext();
      } // for each snmp object
    }
  }
  if (target!=NULL) // receive_snmp_notification calls new
    delete target;
  return status;
})

#ifdef SNMP_PP_NAMESPACE
}; // end of namespace Snmp_pp
#endif 
