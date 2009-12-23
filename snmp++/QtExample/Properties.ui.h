/*_############################################################################
  _## 
  _##  Properties.ui.h  
  _##
  _##  SNMP++v3.2.24
  _##  -----------------------------------------------
  _##  Copyright (c) 2001-2009 Jochen Katz, Frank Fock
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
  _##  Stuttgart, Germany, Fri May 29 22:35:14 CEST 2009 
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

#include <qstringlist.h>
#include <DialogAddUser.h>

void Properties::combo_box_sec_name_activated(const QString &sec_name)
{
  OctetStr sname(sec_name);

  // Get the properties of the user with the given sec_name from USM
  // dont forget to lock/unlock the user table
  usm->lock_user_table();
  const struct UsmUserNameTableEntry *user = usm->get_user(sname);

  if (!user)
  {
    line_edit_auth_pass->setEnabled(false);
    line_edit_priv_pass->setEnabled(false);
    combo_box_auth_prot->setEnabled(false);
    combo_box_priv_prot->setEnabled(false);

    usm->unlock_user_table();

    return;
  }
  line_edit_auth_pass->setText(QString::fromLatin1(
				 (const char*)user->authPassword,
				 user->authPasswordLength));
  line_edit_priv_pass->setText(QString::fromLatin1(
				 (const char*)user->privPassword,
				 user->privPasswordLength));

  line_edit_auth_pass->setEnabled(true);
  line_edit_priv_pass->setEnabled(true);
  combo_box_auth_prot->setEnabled(true);
  combo_box_priv_prot->setEnabled(true);

  switch (user->usmUserAuthProtocol)
  {
    case SNMP_AUTHPROTOCOL_HMACSHA:
      combo_box_auth_prot->setCurrentItem(2); break;
    case SNMP_AUTHPROTOCOL_HMACMD5:
      combo_box_auth_prot->setCurrentItem(1); break;
    case SNMP_AUTHPROTOCOL_NONE:
    default:
      combo_box_auth_prot->setCurrentItem(0);
      line_edit_auth_pass->setEnabled(false);
      break;
  }

  switch (user->usmUserPrivProtocol)
  {
    case SNMP_PRIVPROTOCOL_AES256:
      combo_box_priv_prot->setCurrentItem(5); break;
    case SNMP_PRIVPROTOCOL_AES192:
      combo_box_priv_prot->setCurrentItem(4); break;
    case SNMP_PRIVPROTOCOL_AES128:
      combo_box_priv_prot->setCurrentItem(3); break;
    case SNMP_PRIVPROTOCOL_IDEA:
      combo_box_priv_prot->setCurrentItem(2); break;
    case SNMP_PRIVPROTOCOL_DES:
      combo_box_priv_prot->setCurrentItem(1); break;
    case SNMP_PRIVPROTOCOL_NONE:
    default:
      combo_box_priv_prot->setCurrentItem(0);
      line_edit_priv_pass->setEnabled(false);
      break;
  }

  if (user->usmUserAuthProtocol == SNMP_AUTHPROTOCOL_NONE)
  {
    combo_box_priv_prot->setEnabled(false);
  }

  // unlock user table!
  usm->unlock_user_table();
}

void Properties::set_snmp( Snmp *s )
{
  snmp = s;

  // until now, there can be only one v3MP instance
  v3mp = v3MP::I;
  usm = v3mp->get_usm();
  
  combo_box_sec_name->clear();

  // get all security names
  usm->lock_user_name_table(); // lock table for peek_XXX()

  const struct UsmUserNameTableEntry *user = usm->peek_first_user();
  QStringList names;
  QString initial("initial");
  QString to_add;
  while (user)
  {
    to_add.setAscii((const char*)(user->usmUserSecurityName.data()),
		    user->usmUserSecurityName.len());
    if (!names.contains(to_add) && (to_add != initial))
      names += to_add;
    
    user = usm->peek_next_user(user);
  }
  usm->unlock_user_name_table(); // unlock table

  combo_box_sec_name->insertStringList(names);
  if (combo_box_sec_name->count())
  {
    combo_box_sec_name->setCurrentItem(0);
    combo_box_sec_name_activated(combo_box_sec_name->currentText());
  }
}


void Properties::push_button_reset_clicked()
{
  combo_box_sec_name_activated(combo_box_sec_name->currentText());
}

void Properties::push_button_apply_clicked()
{
  int auth_prot = 0;
  int priv_prot = 0;
  
  switch (combo_box_auth_prot->currentItem())
  {
    // here are all supportet authentcation protocols
    case 0: auth_prot = SNMP_AUTHPROTOCOL_NONE;    break;
    case 1: auth_prot = SNMP_AUTHPROTOCOL_HMACMD5; break;
    case 2: auth_prot = SNMP_AUTHPROTOCOL_HMACSHA; break;
  }
  if (line_edit_auth_pass->text().isEmpty())
    auth_prot = SNMP_AUTHPROTOCOL_NONE;

  if (auth_prot == SNMP_AUTHPROTOCOL_NONE)
  {
    priv_prot = SNMP_PRIVPROTOCOL_NONE;
    line_edit_priv_pass->clear();
  }
  else
  {
    switch (combo_box_priv_prot->currentItem())
    {
      // here are all supported priv protocols
      case 0: priv_prot = SNMP_PRIVPROTOCOL_NONE;   break;
      case 1: priv_prot = SNMP_PRIVPROTOCOL_DES;    break;
      case 2: priv_prot = SNMP_PRIVPROTOCOL_IDEA;   break;
      case 3: priv_prot = SNMP_PRIVPROTOCOL_AES128; break;
      case 4: priv_prot = SNMP_PRIVPROTOCOL_AES192; break;
      case 5: priv_prot = SNMP_PRIVPROTOCOL_AES256; break;
    }
  }
  if (line_edit_priv_pass->text().isEmpty())
    priv_prot = SNMP_PRIVPROTOCOL_NONE;
  else if (priv_prot == SNMP_PRIVPROTOCOL_NONE)
    line_edit_priv_pass->clear();

  // first delete all occurences of that user
  usm->delete_usm_user((const char*)combo_box_sec_name->currentText());
  // then add the user with the new properties
  usm->add_usm_user((const char*)combo_box_sec_name->currentText(),
		    auth_prot, priv_prot,
		    (const char*)line_edit_auth_pass->text(),
		    (const char*)line_edit_priv_pass->text());

  combo_box_sec_name_activated(combo_box_sec_name->currentText());
}


void Properties::combo_box_auth_prot_activated( int  auth_prot)
{
  if (auth_prot)
  {
    line_edit_auth_pass->setEnabled(true);
    combo_box_priv_prot->setEnabled(true);
    if (combo_box_priv_prot->currentItem())
      line_edit_priv_pass->setEnabled(true);
    else
      line_edit_priv_pass->setEnabled(false);
  }
  else
  {
    combo_box_priv_prot->setEnabled(false);
    line_edit_priv_pass->setEnabled(false);
  }
}

void Properties::combo_box_priv_prot_activated(int priv_prot)
{
  if (priv_prot)
    line_edit_priv_pass->setEnabled(true);
  else
    line_edit_priv_pass->setEnabled(false);
}

/// Someone clicked th ebutton to add a new user
void Properties::push_button_add_user_clicked()
{
  DialogAddUser dau;
  dau.exec();

  QString name = dau.line_edit_new_name->text();

  if (name.isEmpty())
    return;

  // first delete the user with that name
  usm->delete_usm_user((const char*)name);

  // then add a user wthout auth and priv
  usm->add_usm_user((const char*)name,
		    SNMP_AUTHPROTOCOL_NONE,
		    SNMP_PRIVPROTOCOL_NONE, "", "");
  for (int i=0; i < combo_box_sec_name->count(); ++i)
  {
    if (combo_box_sec_name->text(i) == name)
    {
      combo_box_sec_name->removeItem(i);
      i--;
    }
  }
  combo_box_sec_name->insertItem(name, 0);
  combo_box_sec_name->setCurrentItem(0);
  combo_box_sec_name_activated(combo_box_sec_name->currentText());
}

/// delete the displayed user
void Properties::push_button_del_user_clicked()
{
  // delete the user from USM
  usm->delete_usm_user((const char*)combo_box_sec_name->currentText());

  combo_box_sec_name->removeItem(combo_box_sec_name->currentItem());
  combo_box_sec_name->setCurrentItem(0);
  combo_box_sec_name_activated(combo_box_sec_name->currentText());
}
