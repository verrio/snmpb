/*_############################################################################
  _## 
  _##  notifyqueue.h  
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
  purpose. It is provided "AS-IS without warranty of any kind,either express
  or implied. User hereby grants a royalty-free license to any and all
  derivatives based upon this software code base.

      N O T I F Y Q U E U E. H

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
        Queue for holding sessions waiting for notifiactions

      COMPILER DIRECTIVES:
        UNIX - For UNIX build

=====================================================================*/
// $Id$

#ifndef _NOTIFYQUEUE
#define _NOTIFYQUEUE

//----[ includes ]-----------------------------------------------------
#include <sys/types.h>          // NOTE: due to 10.10 bug, order is important
                                //   in that all routines must include types.h
                                //   and time.h in same order otherwise you
                                //   will get conflicting definitions of
                                //   "fd_set" resulting in link time errors.
#ifndef WIN32
#include <sys/time.h>	// time stuff and fd_set
#else
#include <winsock.h>
#endif
#ifdef SNMPX11
#include <X11/Intrinsic.h>
#endif

//----[ snmp++ includes ]----------------------------------------------

#include "snmp_pp/oid.h"
#include "snmp_pp/target.h"
#include "snmp_pp/eventlist.h"

#ifdef SNMP_PP_NAMESPACE
namespace Snmp_pp {
#endif

class Snmp; // instead of snmp_pp.h
class msec;
class EventListHolder;

//----[ defines ]------------------------------------------------------

//----[ CNotifyEvent class ]-------------------------------------------

/*----------------------------------------------------------------*/
/* CNotifyEvent                                                   */
/*   a description of a sessions waiting for async notifiactions. */
/*----------------------------------------------------------------*/
class DLLOPT CNotifyEvent
{
 public:

  CNotifyEvent(Snmp* snmp,
	       const OidCollection &trapids,
	       const TargetCollection &targets,
	       const AddressCollection &addresses);
  ~CNotifyEvent();
  Snmp * GetId() { return m_snmp; };
  int notify_filter(const Oid &trapid, SnmpTarget &target) const;
  int Callback(SnmpTarget & target, Pdu & pdu, int status);
  void get_filter(OidCollection &o, TargetCollection &t,
		  AddressCollection &a)
    { o = *notify_ids; t = *notify_targets; a = *notify_addresses; };

 protected:
  Snmp              *m_snmp;
  TargetCollection  *notify_targets;
  OidCollection     *notify_ids;
  AddressCollection *notify_addresses;
};

  /*-----------------------------------------------------------*/
  /* CNotifyEventQueue                                         */
  /*   class describing a collection of outstanding SNMP msgs. */
  /*-----------------------------------------------------------*/
class DLLOPT CNotifyEventQueue: public CEvents
{
  public:
    CNotifyEventQueue(EventListHolder *holder, Snmp *session);
    ~CNotifyEventQueue();
    int AddEntry(Snmp * snmp,
		 const OidCollection &trapids,
		 const TargetCollection &targets,
		 const AddressCollection &addresses);
    CNotifyEvent * GetEntry(Snmp * snmp);
    void DeleteEntry(Snmp * snmp);

    // find the next timeout
    int GetNextTimeout(msec &/*timeout*/) { return 1; }; // we have no timeouts
    // set up parameters for select
    void GetFdSets(int &maxfds, fd_set &readfds, fd_set &writefds,
		   fd_set &exceptfds);
    // return number of outstanding messages
    int GetCount() { return m_msgCount; };

    int HandleEvents(const int maxfds,
                     const fd_set &readfds,
                     const fd_set &writefds,
                     const fd_set &exceptfds);
    int DoRetries(const msec &/*sendtime*/) { return 0; }; // nothing to retry

    int Done() { return 0; }; // we are never done
    static void set_listen_port(int port) { m_listen_port = port; };
    static int get_listen_port() { return m_listen_port; };
    int get_notify_fd() const { return m_notify_fd; };

  protected:

    /*-----------------------------------------------------------*/
    /* CNotifyEventQueueElt                                      */
    /*   a container for a single item on a linked lists of      */
    /*  CNotifyEvents.                                           */
    /*-----------------------------------------------------------*/
    class DLLOPT CNotifyEventQueueElt
    {
     public:
      CNotifyEventQueueElt(CNotifyEvent *notifyevent,
			   CNotifyEventQueueElt *next,
			   CNotifyEventQueueElt *previous);

      ~CNotifyEventQueueElt();
      CNotifyEventQueueElt *GetNext() { return m_next; };
      CNotifyEvent *GetNotifyEvent() { return m_notifyevent; };
      CNotifyEvent *TestId(Snmp * snmp);

    private:

      CNotifyEvent *m_notifyevent;
      class CNotifyEventQueueElt *m_next;
      class CNotifyEventQueueElt *m_previous;
    };

    CNotifyEventQueueElt m_head;
    int                  m_msgCount;
    int                  m_notify_fd;
    static int           m_listen_port;
#ifdef SNMPX11
    XtInputId            m_inputId;
#endif // SNMPX11
    EventListHolder *my_holder;
    Snmp *m_snmpSession;
};

#ifdef SNMP_PP_NAMESPACE
}; // end of namespace Snmp_pp
#endif 

#endif // NOTIFYQUEUE
