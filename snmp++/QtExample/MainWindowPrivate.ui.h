/*_############################################################################
  _## 
  _##  MainWindowPrivate.ui.h  
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
/****************************************************************************
** ui.h extension file, included from the uic-generated form implementation.
**
** If you wish to add, delete or rename functions or slots use
** Qt Designer which will update this file, preserving your code. Create an
** init() function in place of a constructor, and a destroy() function in
** place of a destructor.
*****************************************************************************/
#include <qmessagebox.h>
#include <qstring.h>
#include <qapplication.h>

void MainWindowPrivate::fileExit()
{
  QApplication::exit(0);
}


void MainWindowPrivate::helpAbout()
{
  QString caption("About");
  QString content("\nSimple SNMP++ Example\n\nAuthor: Jochen Katz\n\n");

  QMessageBox msgbox(caption, content, QMessageBox::NoIcon, QMessageBox::Ok,
		     QMessageBox::NoButton, QMessageBox::NoButton);
  msgbox.exec();
}



void MainWindowPrivate::push_button_get_next_clicked()
{

}


void MainWindowPrivate::edit_properties_action_activated()
{
}

/// enable and disable version specific options
void MainWindowPrivate::button_group_version_clicked( int version)
{
  switch (version)
  {
    case 1:
    case 2:
      group_box_v1->setEnabled(true);
      group_box_v3->setEnabled(false);
      break;
    case 3:
      group_box_v1->setEnabled(false);
      group_box_v3->setEnabled(true);
      break;
    default:
      qWarning("Internal error in MainWindowPrivate::button_group_version_clicked.\n");
      break;
  }
}


void MainWindowPrivate::push_button_broadcast_clicked()
{

}


void MainWindowPrivate::push_button_traps_toggled(bool)
{

}
