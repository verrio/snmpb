/*_############################################################################
  _## 
  _##  eventlist.h  
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

      E V E N T L I S T . H

      CEventList  CLASS DEFINITION

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
        Queue for holding all event sources (snmp messages, user
        defined input sources, user defined timeouts, etc)

      COMPILER DIRECTIVES:
        UNIX - For UNIX build

=====================================================================*/
// $Id$

#ifndef _EVENTLIST
#define _EVENTLIST

//----[ includes ]-----------------------------------------------------
#include <limits.h>
#include <sys/types.h> // NOTE: due to 10.10 bug, order is important
                       //   in that all routines must include types.h
                       //   and time.h in same order otherwise you
                       //   will get conflicting definitions of
                       //   "fd_set" resulting in link time errors.
// CK Ng    added #ifdef WIN32
#ifdef WIN32
#include <time.h>
#include <winsock.h>
#else
#include <sys/time.h>  // time stuff and fd_set
#include <float.h>
#endif

#include "snmp_pp/reentrant.h"

#ifdef SNMP_PP_NAMESPACE
namespace Snmp_pp {
#endif

#define MAX_UINT32 MAXLONG

class msec;
class Pdu;

//----[ CEvents class ]------------------------------------------------
class DLLOPT CEvents: public SnmpSynchronized {
  public:

  // allow destruction of derived classes
  virtual ~CEvents() {};

  // find the next timeout
  virtual int GetNextTimeout(msec &sendTime) = 0;

  // set up parameters for select
  virtual void GetFdSets(int &maxfds, fd_set &readfds, fd_set &writefds,
			   fd_set &exceptfds) = 0;

  // return number of outstanding messages
  virtual int GetCount() = 0;

  // process events pending on the active file descriptors
  virtual int HandleEvents(const int maxfds,
			   const fd_set &readfds,
			   const fd_set &writefds,
			   const fd_set &exceptfds) = 0;

  // process any timeout events
  virtual int DoRetries(const msec &sendtime) = 0;

  // check to see if there is a termination condition
  virtual int Done() = 0;
};


class DLLOPT CEventList: public SnmpSynchronized {
  public:
    CEventList() : m_head(0, 0, 0), m_msgCount(0), m_done(0) {};
    ~CEventList();

  // add an event source to the list
  CEvents *AddEntry(CEvents *events);

  // tell main_loop to exit after one pass
  void SetDone() REENTRANT({ m_done += 1; });

  // see if main loop should terminate
  int GetDone()  { return m_done; };

  // find the time of the next event that will timeout
  int GetNextTimeout(msec &sendTime);

  // set up paramters for select
  void GetFdSets(int &maxfds, fd_set &readfds, fd_set &writefds,
		 fd_set &exceptfds);

  // return number of outstanding messages
  int GetCount() { return m_msgCount; };

  // process events pending on the active file descriptors
  int HandleEvents(const int maxfds,
		   const fd_set &readfds,
		   const fd_set &writefds,
		   const fd_set &exceptfds);

  // process any timeout events
  int DoRetries(const msec &sendtime);

  // check to see if there is a termination condition
  int Done();

  private:

   class DLLOPT CEventListElt
   {
    public:
     CEventListElt(CEvents *events,
		   CEventListElt *next,
		   CEventListElt *previous);

     ~CEventListElt();
     CEventListElt *GetNext() { return m_next; };
     CEvents *GetEvents() { return m_events; };

    private:

     CEvents *m_events;
     class CEventListElt *m_next;
     class CEventListElt *m_previous;
   };

    CEventListElt m_head;
    int m_msgCount;
    int m_done;
};

#ifdef SNMP_PP_NAMESPACE
}; // end of namespace Snmp_pp
#endif 

#endif
