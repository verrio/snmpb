/*_############################################################################
  _## 
  _##  eventlistholder.cpp  
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
#include "snmp_pp/eventlistholder.h"
#include "snmp_pp/eventlist.h"
#include "snmp_pp/msgqueue.h"
#include "snmp_pp/userdefined.h"
#include "snmp_pp/usertimeout.h"
#include "snmp_pp/notifyqueue.h"
#include "snmp_pp/mp_v3.h"
#include "snmp_pp/v3.h"

#ifdef SNMP_PP_NAMESPACE
namespace Snmp_pp {
#endif

#ifdef SNMPX11
XtAppContext global_app_context = CONTEXT_NOT_SET;
XtIntervalId global_interval_id = TIMER_NOT_SET;
msec global_next_timeout;
#endif // SNMPX11


#ifdef WU_APP

extern "C" {

#define PeekMessage PeekMessageA
#define DispatchMessage DispatchMessageA

// MS-Windows types needed
typedef int BOOL;
typedef unsigned short WORD;
typedef unsigned long DWORD;
typedef void*  HWND;
typedef unsigned int UINT;
typedef UINT WPARAM;
typedef long LPARAM;
#define MAKELONG(a,b) ((long)(((WORD)(a))|((DWORD)((WORD)(b)))<<16))
typedef struct tagPOINT
{
   long x;
   long y;
} POINT;

WORD app_hinst;
typedef struct tagMSG {
   HWND hWnd;
   UINT message;
   WPARAM wParam;
   LPARAM lParam;
   DWORD time;
   POINT pt;
} MSG,*LPMSG;

#define PM_NOREMOVE 0x0000
#define PM_REMOVE 0x0001

// prototypes for MS-Windows message pump calls
BOOL PeekMessage( LPMSG lpMsg, HWND hWnd, UINT wMFMin, UINT wMFMAX, UINT wRMsg);
BOOL TranslateMessage( const MSG *lpmsg);
BOOL DispatchMessage( const MSG *lpmsg);
}
#endif

//-------[ blocking MS-Windows Message Pump ]-------
// Pumping messages allows other windows messages
// to be processed.
int yield_pump()
{
#ifdef WU_APP
  MSG msg;

  while ( PeekMessage( &msg,NULL,0,0,PM_NOREMOVE))
  {
    // else peek, xlate, and dispatch it
    PeekMessage( &msg, NULL,0,0,PM_REMOVE);
    if ( msg.message == SNMP_SHUTDOWN_MSG ) return SNMP_CLASS_SHUTDOWN;
    TranslateMessage( &msg);
    DispatchMessage( &msg);
  }
#endif


//--------[ X Motif yield pump ]------------------------
// yield for use with X motif only
// no blocking yield implemented at this time

#ifdef X_MOTIF

#endif

  return SNMP_CLASS_SUCCESS;
}



EventListHolder::EventListHolder(Snmp *snmp_session)
{
  // Automaticly add the SNMP message queue
  m_snmpMessageQueue = new CSNMPMessageQueue(this, snmp_session);
  m_eventList.AddEntry(m_snmpMessageQueue);

  // Automatically add the SNMP notification queue
  m_notifyEventQueue = new CNotifyEventQueue(this, snmp_session);
  m_eventList.AddEntry(m_notifyEventQueue);

#ifdef _USER_DEFINED_EVENTS
  // Automaticly add the user-defined event queue
  m_udEventQueue = new CUDEventQueue(this);
  m_eventList.AddEntry(m_udEventQueue);
#endif

#ifdef _USER_DEFINED_TIMEOUTS
  // Automaticly add the user-defined timeout queue
  m_utEventQueue = new CUTEventQueue(this);
  m_eventList.AddEntry(m_utEventQueue);
#endif
}

EventListHolder::~EventListHolder() 
{
}

//---------[ Block For Response ]-----------------------------------
// Wait for the completion of an outstanding SNMP event (msg).
// Handle any other events as they occur.
int EventListHolder::SNMPBlockForResponse(const unsigned long req_id,
					  Pdu &pdu)
{
  CSNMPMessage *msg;
  int status;

  do {
    yield_pump();
    SNMPProcessEvents(1000);
  } while (!m_snmpMessageQueue->Done(req_id));

  m_snmpMessageQueue->lock();
  msg = m_snmpMessageQueue->GetEntry(req_id);
  if (msg) {
    // we found our response
    msg->GetPdu(status, pdu);

    // Dequeue the message
    m_snmpMessageQueue->DeleteEntry(req_id);
    m_snmpMessageQueue->unlock();
    return  status;
  }
  else {
    // not in the send queue...must have timed out
    m_snmpMessageQueue->unlock();
    return SNMP_CLASS_TIMEOUT;
  }
}

//---------[ Process Pending Events ]-------------------------------
// Pull all available events out of their sockets - do not block
int EventListHolder::SNMPProcessPendingEvents()
{
  int maxfds;
  fd_set readfds;
  fd_set writefds;
  fd_set exceptfds;
  int nfound = 0;
  struct timeval fd_timeout;
  msec now(0, 0);
  int status;

  pevents_mutex.lock();

  // do not allow select to block
  fd_timeout.tv_sec = 0;
  fd_timeout.tv_usec = 10;  // chosen a very small timeout
  // in order to avoid busy looping but keep overall performance

  do {

    // Set up Select
    m_eventList.GetFdSets(maxfds, readfds, writefds, exceptfds);

    nfound = select(maxfds, &readfds, &writefds, &exceptfds, &fd_timeout);

    now.refresh();

    if (nfound > 0)
    { // found something on select
      status = m_eventList.HandleEvents(maxfds, readfds, writefds, exceptfds);
      // TM should we do anything with bad status?
    }
#ifdef WIN32
    /* On Win32 select immediately returns -1 if all fd_sets are empty */
    if (maxfds == 0)
      Sleep(1); /* prevent 100% CPU utilization */
#endif
  } while (nfound > 0);

  // go through the message queue and resend any messages
  // which are past the timeout.
  status = m_eventList.DoRetries(now);

  pevents_mutex.unlock();

  return status;
}

//---------[ Process Events ]------------------------------------------
// Block until an event shows up - then handle the event(s)
int EventListHolder::SNMPProcessEvents(const int max_block_milliseconds)
{
  int maxfds;
  fd_set readfds;
  fd_set writefds;
  fd_set exceptfds;
  int nfound = 0;
  struct timeval fd_timeout;
  msec now; // automatcally calls msec::refresh()
  msec sendTime;
  int status = 0;

  m_eventList.GetNextTimeout(sendTime);
  now.GetDelta(sendTime, fd_timeout);

  m_eventList.GetFdSets(maxfds, readfds, writefds, exceptfds);

  if ((max_block_milliseconds > 0) &&
      ((fd_timeout.tv_sec > max_block_milliseconds / 1000) ||
       ((fd_timeout.tv_sec == max_block_milliseconds / 1000) &&
	(fd_timeout.tv_usec > (max_block_milliseconds % 10000) * 1000))))
  {
    fd_timeout.tv_sec = max_block_milliseconds / 1000;
    fd_timeout.tv_usec = (max_block_milliseconds % 10000) * 1000;
  }

  /* Prevent endless sleep in case no fd is open */
  if ((maxfds == 0) && (fd_timeout.tv_sec > 5))
    fd_timeout.tv_sec = 5; /* sleep at max 5.99 seconds */

  nfound = select(maxfds, &readfds, &writefds, &exceptfds, &fd_timeout);

  status = SNMPProcessPendingEvents();

  return status;
}

//---------[ Main Loop ]------------------------------------------
// Infinite loop which blocks when there is nothing to do and handles
// any events as they occur.
void EventListHolder::SNMPMainLoop(const int max_block_milliseconds)
{
  do {
    SNMPProcessEvents(max_block_milliseconds);
  } while (!m_eventList.Done());
}

//---------[ Exit Main Loop ]---------------------------------------
// Force the SNMP Main Loop to terminate immediately
void EventListHolder::SNMPExitMainLoop()
{
   m_eventList.SetDone();
}


void EventListHolder::SNMPGetFdSets(int    &maxfds,
				    fd_set &readfds,
				    fd_set &writefds,
				    fd_set &exceptfds)
{
  m_eventList.GetFdSets(maxfds, readfds, writefds, exceptfds);
}

Uint32 EventListHolder::SNMPGetNextTimeout()
{
  msec now;
  msec sendTime(now);

//TM: This function used to have an argument of sendTime and
//    would simply call eventList.GetNextTimeout(sendTime) and
//    return the status.  However, to avoid exposing the msec
//    class we now convert the msec to hundreths of seconds
//    and return that as a unsigned long.
// 25-Jan-96 TM

  m_eventList.GetNextTimeout(sendTime);
  if (sendTime.IsInfinite()) {
    return UINT_MAX;
  }
  else {

    // Kludge: When this was first designed the units were millisecs
    // However, later on the units for the target class were changed
    // to hundreths of secs.  Divide millisecs by 10 to create the
    // hundreths of secs which the rest of the objects use.
    // 25-Jan-96 TM

    // 21-May-02 DLD: Add check to avoid returning a negative interval
    // Long eventlists seem to end up with events that are greater
    // than the time when the event loop is started, but less than the
    // time when this function is called.  This check is analagous to
    // what is done in msec::GetDelta() which is used in
    // SNMPProcessEvents(), the library main loop.

    // 21-May-02 DLD: Add 1/100 sec to time out to avoid returning a
    // short time out interval.  A short interval can result due to
    // truncation of the millisec value when dividing by 10.

    if (sendTime > now)
    {
      sendTime -= now;
      return ((((unsigned long) sendTime) / 10) + 1);
    }
    else
      return 0;
  }
}

#ifdef _USER_DEFINED_TIMEOUTS
UtId EventListHolder::SNMPAddTimeOut(const unsigned long interval,
				     const ut_callback callBack,
				     const void * callData)
{
  msec now;
  now += interval;
  return m_utEventQueue->AddEntry(now, callBack, callData);
}
#endif

#ifdef _USER_DEFINED_EVENTS
UdId EventListHolder::SNMPAddInput(const int source,
				   const UdInputMask condition,
				   const ud_callback callBack,
				   const void * callData)
{
  return m_udEventQueue->AddEntry(source, condition, callBack, callData);
}
#endif

#ifdef SNMPX11
// Required for integration with X11

int SNMPX11Initialize(const XtAppContext app_context)
{
  // Save the app_context for future Xt calls.
  global_app_context = app_context;

  return SNMP_CLASS_SUCCESS;
}

void SnmpX11TimerCallback(XtPointer client_data,
			  XtIntervalId */*id*/)
{
  // We have been called because one of our timers popped
  // The timer is automatically unregistered from X11 after it is fired
  global_interval_id = TIMER_NOT_SET;

  if (client_data)
  {
    // Handle the event and any timeouts
    ((EventListHolder*)client_data)->SNMPProcessPendingEvents();

    // Set a timer for the next retransmission
    ((EventListHolder*)client_data)->SnmpX11SetTimer();
  }
}

void SnmpX11InputCallback(XtPointer client_data,
			  int */*source*/, XtInputId */*id*/)
{
  if (client_data)
  {
    // We have been called because there is activity on one of our fds
    // Handle the event and any timeouts
    ((EventListHolder*)client_data)->SNMPProcessPendingEvents();

    // Set a timer for the next retransmission
    ((EventListHolder*)client_data)->SnmpX11SetTimer();
  }
}

void EventListHolder::SnmpX11SetTimer()
{
  msec nextTimeout;

  // if they have not yet initialized there is nothing we can do
  if (global_app_context == CONTEXT_NOT_SET)
    return;

  // Before returning control set a timer with X11 in case we
  // don't get any input before the next retransmission time
  m_eventList.GetNextTimeout(nextTimeout);
  if (global_interval_id != TIMER_NOT_SET) {
    // we already have a timer set
    if (global_next_timeout != nextTimeout) {
      // The timeout registered with X11 is no longer what we want
      XtRemoveTimeOut(global_interval_id);
    }
    else {
      return;      // The current timeout is still valid
    }
  }
  if (nextTimeout.IsInfinite()) {
    // nothing left to wait for
    global_interval_id = TIMER_NOT_SET;
  }
  else {
    timeval alarm;
    msec now;
    unsigned long millisec;
    // calcuate and set the next timer
    now.GetDelta(nextTimeout, alarm);
    millisec = (alarm.tv_sec * 1000) + (alarm.tv_usec / 1000);
    global_next_timeout = nextTimeout;
    global_interval_id = XtAppAddTimeOut(global_app_context, millisec,
					 (XtTimerCallbackProc)SnmpX11TimerCallback, this);
  }
  return;
}

int EventListHolder::SnmpX11AddInput(int inputFd, XtInputId &inputId)
{
  if (global_app_context == CONTEXT_NOT_SET) {
    // They have not called SnmpX11Initialize yet!
    //TM: Need better error define
    return SNMP_CLASS_ERROR;
  }
  // Tell X11 to call us back for any activity on our file descriptor
  inputId = XtAppAddInput(global_app_context, inputFd,
			  (XtPointer) (XtInputReadMask | XtInputExceptMask),
			  (XtInputCallbackProc)(SnmpX11InputCallback),
			  this);
  SnmpX11SetTimer();
  return SNMP_CLASS_SUCCESS;
}

int EventListHolder::SnmpX11RemoveInput(XtInputId &inputId)
{
  // Tell X11 to stop watching our file descriptor
  XtRemoveInput(inputId);
  SnmpX11SetTimer();
  return SNMP_CLASS_SUCCESS;
}
#endif // SNMPX11

#ifdef SNMP_PP_NAMESPACE
}; // end of namespace Snmp_pp
#endif 
