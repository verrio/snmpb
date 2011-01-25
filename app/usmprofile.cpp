/*
    Copyright (C) 2004-2011 Martin Jolicoeur (snmpb1@gmail.com) 

    This file is part of the SnmpB project 
    (http://sourceforge.net/projects/snmpb)

    This program is free software: you can redistribute it and/or modify
    it under the terms of the GNU General Public License as published by
    the Free Software Foundation, either version 2 of the License, or
    (at your option) any later version.

    This program is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
    GNU General Public License for more details.

    You should have received a copy of the GNU General Public License
    along with this program.  If not, see <http://www.gnu.org/licenses/>.
*/
#include "usmprofile.h"
#include "agent.h"

USMProfileManager::USMProfileManager(Snmpb *snmpb)
{
    s = snmpb;

    up.setupUi(&upw);

    // Set some properties for the USM Profile TreeView
    up.ProfileTree->header()->hide();
    up.ProfileTree->setSortingEnabled( FALSE );
    up.ProfileTree->header()->setSortIndicatorShown( FALSE );
    up.ProfileTree->setLineWidth( 2 );
    up.ProfileTree->setAllColumnsShowFocus( FALSE );
    up.ProfileTree->setFrameShape(QFrame::WinPanel);
    up.ProfileTree->setFrameShadow(QFrame::Plain);
    up.ProfileTree->setRootIsDecorated( TRUE );

    // Create context menu actions
    up.ProfileTree->setContextMenuPolicy (Qt::CustomContextMenu);
    connect( up.ProfileTree, 
             SIGNAL( customContextMenuRequested ( const QPoint & ) ),
             this, SLOT( ContextMenu ( const QPoint & ) ) );
    addAct = new QAction(tr("&New USM profile"), this);
    connect(addAct, SIGNAL(triggered()), this, SLOT(Add()));
    deleteAct = new QAction(tr("&Delete USM profile"), this);
    connect(deleteAct, SIGNAL(triggered()), this, SLOT(Delete()));

    connect( up.ProfileTree, 
             SIGNAL( currentItemChanged( QTreeWidgetItem *, QTreeWidgetItem * ) ),
             this, SLOT( SelectedUSMProfile( QTreeWidgetItem *, QTreeWidgetItem * ) ) );
    connect( up.ProfileTree, 
             SIGNAL( itemChanged( QTreeWidgetItem *, int ) ),
             this, SLOT( USMSecNameChange( QTreeWidgetItem *, int ) ) );
    connect( up.SecName, SIGNAL( editingFinished() ), 
             this, SLOT ( SetSecName() ) );
    connect( up.AuthProtocol, SIGNAL( currentIndexChanged( int ) ), 
             this, SLOT ( SetAuthProto() ) );
    connect( up.AuthPass, SIGNAL( editingFinished() ), 
             this, SLOT ( SetAuthPass() ) );
    connect( up.PrivProtocol, SIGNAL( currentIndexChanged( int ) ), 
             this, SLOT ( SetPrivProto() ) );
    connect( up.PrivPass, SIGNAL( editingFinished() ), 
             this, SLOT ( SetPrivPass() ) );

    // Loop & load all stored USM profiles
    currentprofile = NULL;

    USM* usm = s->AgentObj()->GetUSMObj();
    int numusers = 0;
    const UsmUserNameTableEntry * v3user = usm->peek_first_user();
    while (v3user)
    {
        QString _name(v3user->usmUserSecurityName.get_printable());
        USMProfile *newuser = new USMProfile(&up, &_name);

        QString ap = QString::fromLatin1((const char*)v3user->authPassword, 
                                         v3user->authPasswordLength);
        QString pp = QString::fromLatin1((const char*)v3user->privPassword, 
                                         v3user->privPasswordLength);
        newuser->SetSecurity(LibAuthToUiAuth(v3user->usmUserAuthProtocol), ap,
                             LibPrivToUiPriv(v3user->usmUserPrivProtocol), pp);

        users.append(newuser);
        numusers++;

        v3user = usm->peek_next_user(v3user);
    }

    if (numusers != 0)
       up.ProfileTree->setCurrentItem(up.ProfileTree->topLevelItem(0));
}

void USMProfileManager::Execute (void)
{
    if(upw.exec() == QDialog::Accepted)
    {
        USM* usm = s->AgentObj()->GetUSMObj();

        // First, loop & delete the entire user table
        const UsmUserNameTableEntry * v3user = usm->peek_first_user();
        while (v3user)
        {
            const UsmUserNameTableEntry *v3temp = v3user;

            v3user = usm->peek_next_user(v3temp);

            usm->delete_usm_user(v3temp->usmUserSecurityName);
        }

        // ... then rebuild it, using the stored profiles
        for (int i = 0; i < users.size(); i++)
        {
            usm->add_usm_user(OctetStr(users[i]->GetName().toLatin1().data()),
                              UiAuthToLibAuth(users[i]->GetAuthProto()),
                              UiPrivToLibPriv(users[i]->GetPrivProto()),
                              OctetStr((const unsigned char *)users[i]->GetAuthPass().toLatin1().data(), 
                                       users[i]->GetAuthPass().toLatin1().length()), 
                              OctetStr((const unsigned char *)users[i]->GetPrivPass().toLatin1().data(), 
                                       users[i]->GetPrivPass().toLatin1().length()));
        }

        // ... then save it to file.    
        usm->save_users(s->GetUsmUsersConfigFile().toLatin1().data());
    }
}

void USMProfileManager::SetSecName(void)
{
    if (currentprofile)
        currentprofile->SetSecName();
}

void USMProfileManager::SetAuthProto(void)
{
    if (currentprofile)
        currentprofile->SetAuthProto();
}

void USMProfileManager::SetAuthPass(void)
{
    if (currentprofile)
        currentprofile->SetAuthPass();
}

void USMProfileManager::SetPrivProto(void)
{
    if (currentprofile)
        currentprofile->SetPrivProto();
}

void USMProfileManager::SetPrivPass(void)
{
    if (currentprofile)
        currentprofile->SetPrivPass();
}

void USMProfileManager::ContextMenu ( const QPoint &pos )
{    
    QMenu menu(tr("Actions"), up.ProfileTree);

    menu.addAction(addAct);
    menu.addAction(deleteAct);

    menu.exec(up.ProfileTree->mapToGlobal(pos));
}

void USMProfileManager::Add(void)
{
    USMProfile * newuser = new USMProfile(&up);
    // Set default values
    newuser->SetSecurity(0, "", 0, "");
 
    users.append(newuser);

    // Select the new item and change the focus to change its name ...
    up.ProfileTree->setCurrentItem(newuser->GetUserWidgetItem());
    up.SecName->setFocus(Qt::OtherFocusReason);  
    up.SecName->selectAll();  
}

void USMProfileManager::Delete(void)
{
    QTreeWidgetItem *p = NULL;

    for (int i = 0; i < users.count(); i++) 
    {
        if (currentprofile && (users[i] == currentprofile))
        {
            // Delete the profile (removes from the list)
            delete users.takeAt(i);
            currentprofile = NULL;
            // Readjust the currentprofile pointer with the new current widget item
            if ((p = up.ProfileTree->currentItem()) != NULL)
            {
                for (int i = 0; i < users.count(); i++) 
                {
                    if (users[i]->IsPartOfUSMProfile(p))
                    {
                        currentprofile = users[i];
                        break;
                    }
                }
            }
            break;
        }
    }
}

void USMProfileManager::SelectedUSMProfile(QTreeWidgetItem * item, QTreeWidgetItem *)
{
    for (int i = 0; i < users.count(); i++) 
    {
        if (users[i]->IsPartOfUSMProfile(item))
        {
            currentprofile = users[i];
            users[i]->SelectUSMProfile(item);
            return;
        }
    }
}

void USMProfileManager::USMSecNameChange(QTreeWidgetItem * item, int column)
{
    if (column != 0)
        return;

    for (int i = 0; i < users.count(); i++) 
    {
        if (users[i]->IsPartOfUSMProfile(item))
        {
            users[i]->SetName(item->text(0));
            return;
        }
    }
}

int USMProfileManager::UiAuthToLibAuth(int prot)
{
    switch(prot)
    {
        case 0: return SNMP_AUTHPROTOCOL_NONE;
        case 1: return SNMP_AUTHPROTOCOL_HMACMD5;
        case 2: return SNMP_AUTHPROTOCOL_HMACSHA;
        default:
            break;
    }

    return SNMP_AUTHPROTOCOL_NONE;
}

int USMProfileManager::LibAuthToUiAuth(int prot)
{
    switch(prot)
    {
        case SNMP_AUTHPROTOCOL_NONE: return 0;
        case SNMP_AUTHPROTOCOL_HMACMD5: return 1;
        case SNMP_AUTHPROTOCOL_HMACSHA: return 2;
        default:
            break;
    }

    return 0;
}

int USMProfileManager::UiPrivToLibPriv(int prot)
{
    switch(prot)
    {
        case 0: return SNMP_PRIVPROTOCOL_NONE;
        case 1: return SNMP_PRIVPROTOCOL_DES;
        case 2: return SNMP_PRIVPROTOCOL_3DESEDE;
        case 3: return SNMP_PRIVPROTOCOL_IDEA;
        case 4: return SNMP_PRIVPROTOCOL_AES128;
        case 5: return SNMP_PRIVPROTOCOL_AES192;
        case 6: return SNMP_PRIVPROTOCOL_AES256;
        default:
            break;
    }

    return SNMP_PRIVPROTOCOL_NONE;
}

int USMProfileManager::LibPrivToUiPriv(int prot)
{
    switch(prot)
    {
        case SNMP_PRIVPROTOCOL_NONE: return 0;
        case SNMP_PRIVPROTOCOL_DES: return 1;
        case SNMP_PRIVPROTOCOL_3DESEDE: return 2;
        case SNMP_PRIVPROTOCOL_IDEA: return 3;
        case SNMP_PRIVPROTOCOL_AES128: return 4;
        case SNMP_PRIVPROTOCOL_AES192: return 5;
        case SNMP_PRIVPROTOCOL_AES256: return 6;
        default:
            break;
    }

    return 0;
}

QStringList USMProfileManager::GetUsersList(void)
{
    QStringList sl;

    for(int i = 0; i < users.size(); i++)
        sl << users[i]->GetName();

    return sl;
}

USMProfile::USMProfile(Ui_USMProfile *uiup, QString *n)
{
    up = uiup;

    user = new QTreeWidgetItem(up->ProfileTree);

    if (n)
    {
        user->setText(0, n->toLatin1().data());
        SetName(*n);
    }
    else
    {
        user->setText(0, "newuser");
        SetName("newuser");
    }
}

USMProfile::~USMProfile()
{
    delete user;
}

int USMProfile::SelectUSMProfile(QTreeWidgetItem * item)
{
    if (item == user)
    {
        up->SecName->setText(name);
        up->AuthProtocol->setCurrentIndex(authproto);
        up->AuthPass->setText(authpass);
        up->PrivProtocol->setCurrentIndex(privproto);
        up->PrivPass->setText(privpass);

        return 1;
    }

    return 0;
}

int USMProfile::IsPartOfUSMProfile(QTreeWidgetItem * item)
{
    if (item == user)
        return 1;
    else
        return 0;
}

QTreeWidgetItem *USMProfile::GetUserWidgetItem(void)
{
    return user;
}

void USMProfile::SetName(QString n)
{
    name = n;
    up->SecName->setText(name);  
}

QString USMProfile::GetName(void)
{
    return name;
}

void USMProfile::SetSecName(void)
{
    name = up->SecName->text();
    user->setText(0, name);
}

void USMProfile::SetAuthProto(void)
{
    authproto = up->AuthProtocol->currentIndex();
}

int USMProfile::GetAuthProto(void)
{
    return authproto;
}

void USMProfile::SetAuthPass(void)
{
    authpass = up->AuthPass->text();
}

QString USMProfile::GetAuthPass(void)
{
    return authpass;
}

void USMProfile::SetPrivProto(void)
{
    privproto = up->PrivProtocol->currentIndex();
}

int USMProfile::GetPrivProto(void)
{
    return privproto;
}

void USMProfile::SetPrivPass(void)
{
    privpass = up->PrivPass->text();
}

QString USMProfile::GetPrivPass(void)
{
    return privpass;
}

void USMProfile::SetSecurity(int aprot, QString apass, int pprot, QString ppass)
{
    authproto = aprot;
    authpass = apass;
    privproto = pprot;
    privpass = ppass;
}

