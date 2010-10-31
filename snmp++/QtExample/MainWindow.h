/*_############################################################################
  _## 
  _##  MainWindow.h  
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
#include "MainWindowPrivate.h"

#include "snmp_pp/snmp_pp.h"
#include <qtimer.h>

#ifdef SNMP_PP_NAMESPACE
using namespace Snmp_pp;
#endif

class MainWindow : public MainWindowPrivate
{
  Q_OBJECT

 public:
  MainWindow( QWidget* parent = 0, const char* name = 0, Qt::WFlags fl = Qt::WType_TopLevel );
  ~MainWindow();
  void async_callback(int reason, Snmp *snmp, Pdu &pdu, SnmpTarget &target);

 protected slots:
  virtual void push_button_get_next_clicked();
  virtual void push_button_broadcast_clicked();
  virtual void timer_expired();
  virtual void edit_properties_action_activated();
  virtual void push_button_traps_toggled(bool isOn);

 private:
  void update_combobox_sec_name();

  Snmp *snmp;
  v3MP *v3mp;
  QTimer timer;
};
