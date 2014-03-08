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
#include <QtGui>
#include <qfileinfo.h>
#include <qdir.h>
#include <qmessagebox.h>
#include "snmpb.h"
#include "mibmodule.h"
#include "agent.h"
#include "trap.h"
#include "graph.h"
#include "logsnmpb.h"
#include "mibeditor.h"
#include "discovery.h"

#include "agentprofile.h"
#include "usmprofile.h"
#include "preferences.h"

// These are needed to get the libraries version strings for the about box
#include "smi.h"
#include "tomcrypt.h"
#include "qwt.h"
#include "snmp_pp/config_snmp_pp.h"

#define SNMPB_CONFIG_DIR         ".snmpb"
#define MIB_CONFIG_FILE          "mib.conf"
#define PATH_CONFIG_FILE         "path.conf"
#define BOOT_COUNTER_CONFIG_FILE "boot_counter.conf"
#define USM_USERS_CONFIG_FILE    "usm_users.conf"
#define AGENTS_CONFIG_FILE       "agents.conf"
#define PREFS_CONFIG_FILE        "preferences.conf"
#define LOG_CONFIG_FILE          "log.conf"
#define GRAPHS_CONFIG_FILE       "graphs.conf"

#define STANDARD_TRAP_PORT       162 

char default_mib_config[] = {
"load IF-MIB\n\
load RFC1213-MIB\n\
load SNMP-FRAMEWORK-MIB\n\
load SNMP-NOTIFICATION-MIB\n\
load SNMPv2-CONF\n\
load SNMPv2-SMI\n\
load SNMPv2-TC\n\
load SNMPv2-TM\n\
load SNMP-VIEW-BASED-ACM-MIB"
};

static QDir SnmpbDir = QDir::homePath() + "/" + SNMPB_CONFIG_DIR;

Snmpb::Snmpb(void)
{
    // First thing to do is to give up root privileges that allow permission to
    // bind on privileged ports (<1024). This is needed to bind on 
    // the RFC-defined trap port number 162 on UNIX machines.
    prefs = new Preferences(this);
#ifndef WIN32 
    // Allows to bind on privileged ports only if it is the standard trap port...
    if (! (((prefs->GetEnableIPv4() == true) && 
            (prefs->GetTrapPort() == STANDARD_TRAP_PORT)) ||
           ((prefs->GetEnableIPv6() == true) && 
            (prefs->GetTrapPort6() == STANDARD_TRAP_PORT))))
        setuid(getuid());
#endif
    agent = new Agent(this);
    start_issuccess = agent->GetStartupResult(start_msg);    
#ifndef WIN32 
    // Drop root privileges
    if (setuid(getuid()) < 0)
    {
        printf("Unable to drop root privileges: %m\n");
    }
#endif
    // Note: beware as anything BEFORE this point is run as root on UNIX ... 

    CheckForConfigFiles();
}

void Snmpb::BindToGUI(QMainWindow* mw)
{
    if (start_issuccess == false)
    {
        QMessageBox::critical( NULL, "SnmpB", start_msg,
                               QMessageBox::Ok, Qt::NoButton);

        // Desperate measures: delete the preferences file so 
        // at the next startup, the app might have a chance to start
        QFile prefs(GetPrefsConfigFile());
        prefs.remove();
        exit (-1);
    }
    else
    {
        if (start_msg != "")
            QMessageBox::warning ( NULL, "SnmpB", start_msg,
                                   QMessageBox::Ok, Qt::NoButton);
        agent->StartTrapTimer();
    }

    w.setupUi(mw);

    connect(&loader, SIGNAL ( LogError(QString) ),
            w.LogOutput, SLOT ( append (QString) ));

    // Creation order is VERY important here
    logsnmpb = new LogSnmpb(this);
    modules = new MibModule(this);
    apm = new AgentProfileManager(this);
    prefs->Init();
    trap = new Trap(this);
    agent->Init();
    upm = new USMProfileManager(this);
    gm = new GraphManager(this);
    editor = new MibEditor(this);
    discovery = new Discovery(this);

    // Connect some signals
    connect( w.TabW, SIGNAL( currentChanged(int) ),
             this, SLOT( TabSelected() ) );
    connect( w.actionManageAgentProfiles, SIGNAL( triggered(bool) ),
             this, SLOT( ManageAgentProfiles(bool) ) );
    connect( w.actionManageUSMProfiles, SIGNAL( triggered(bool) ),
             this, SLOT( ManageUSMProfiles(bool) ) );
    connect( w.actionPreferences, SIGNAL( triggered(bool) ),
             this, SLOT( ManagePreferences(bool) ) );
    connect( w.helpAboutAction, SIGNAL( triggered(bool) ),
             this, SLOT( AboutBox(bool) ) );

    // Register every MIB tree to the MIB loader object
    w.MIBTree->RegisterToLoader(&loader);
#if 0 //MART
    w.PlotMIBTree->RegisterToLoader(&loader);
#endif    
    TabSelected();
}

Ui_MainW* Snmpb::MainUI(void)
{
    return (&w);
}

Agent* Snmpb::AgentObj(void)
{
    return (agent);
}

Trap* Snmpb::TrapObj(void)
{
    return (trap);
}

MibViewLoader* Snmpb::MibLoaderObj(void)
{
    return (&loader);
}

MibModule* Snmpb::MibModuleObj(void)
{
    return (modules);
}

MibEditor* Snmpb::MibEditorObj(void)
{
    return (editor);
}

AgentProfileManager* Snmpb::APManagerObj(void)
{
    return (apm);
}

USMProfileManager* Snmpb::UPManagerObj(void)
{
    return (upm);
}

Preferences* Snmpb::PreferencesObj(void)
{
    return (prefs);
}

void Snmpb::CheckForConfigFiles(void)
{
    if (!SnmpbDir.exists())
    {
        if(!SnmpbDir.mkdir(SnmpbDir.absolutePath()))
        {
            QString err = QString("Cannot create configuration directory : %1\n")
                          .arg(SnmpbDir.absolutePath().toLatin1().data());
            QMessageBox::warning ( NULL, "SnmpB", err, 
                                   QMessageBox::Ok, Qt::NoButton);
        }
        else
        {
            // Create default mib.conf file.
            QFile file(GetMibConfigFile());
            if (!file.open(QIODevice::ReadWrite))
            {
                QString err = QString("Cannot create configuration file : %1\n")
                                     .arg(file.fileName());
                QMessageBox::warning ( NULL, "SnmpB", err, 
                                       QMessageBox::Ok, Qt::NoButton);
            }
            else
            {
                file.write(default_mib_config, strlen(default_mib_config));
                file.close();
            }
        }
    }
}

QString Snmpb::GetBootCounterConfigFile(void)
{
    return (SnmpbDir.filePath(BOOT_COUNTER_CONFIG_FILE));
}

QString Snmpb::GetMibConfigFile(void)
{
    return (SnmpbDir.filePath(MIB_CONFIG_FILE));
}

QString Snmpb::GetPathConfigFile(void)
{
    return (SnmpbDir.filePath(PATH_CONFIG_FILE));
}

QString Snmpb::GetUsmUsersConfigFile(void)
{
    return (SnmpbDir.filePath(USM_USERS_CONFIG_FILE));
}

QString Snmpb::GetAgentsConfigFile(void)
{
    return (SnmpbDir.filePath(AGENTS_CONFIG_FILE));
}

QString Snmpb::GetPrefsConfigFile(void)
{
    return (SnmpbDir.filePath(PREFS_CONFIG_FILE));
}

QString Snmpb::GetLogConfigFile(void)
{
    return (SnmpbDir.filePath(LOG_CONFIG_FILE));
}

QString Snmpb::GetGraphsConfigFile(void)
{
    return (SnmpbDir.filePath(GRAPHS_CONFIG_FILE));
}

void Snmpb::ManageAgentProfiles(bool)
{
    apm->Execute();
}

void Snmpb::ManageUSMProfiles(bool)
{
    upm->Execute();
}

void Snmpb::ManagePreferences(bool)
{
    prefs->Execute();
}

void Snmpb::SetEditorMenus(bool value)
{
    MainUI()->fileNewAction->setEnabled(value);
    MainUI()->fileOpenAction->setEnabled(value);
    MainUI()->fileSaveAction->setEnabled(value);
    MainUI()->fileSaveAsAction->setEnabled(value);
    MainUI()->actionVerifyMIB->setEnabled(value);
    MainUI()->actionExtractMIBfromRFC->setEnabled(value);
    MainUI()->actionGotoLine->setEnabled(value);
    MainUI()->actionFind->setEnabled(value);
    MainUI()->actionReplace->setEnabled(value);
    MainUI()->actionFindNext->setEnabled(value);
}

/* 
 * This is where anything related to a tab being selected happens:
 * graying-out GUI parts, refreshing MIB trees, ...
 */
void Snmpb::TabSelected(void)
{
    switch (w.TabW->currentIndex())
    {
    case 0: // Tree
        SetEditorMenus(false);
        // Set find func to MIB tree
        MainUI()->actionFind->setEnabled(true);
        MainUI()->actionFindNext->setEnabled(true);
        disconnect(MainUI()->actionFind, SIGNAL( triggered() ), 0, 0);
        disconnect(MainUI()->actionFindNext, SIGNAL( triggered() ), 0, 0);
        connect( MainUI()->actionFind, SIGNAL( triggered() ),
                w.MIBTree, SLOT( FindFromNode() ) );
        connect( MainUI()->actionFindNext, SIGNAL( triggered() ),
                w.MIBTree, SLOT( ExecuteFindNext() ) );
        MainUI()->actionMultipleVarbinds->setEnabled(true);
        // Refresh MIB tree if needed
        w.MIBTree->Populate();
        break;
    case 1: // Modules
        SetEditorMenus(false);
        MainUI()->actionMultipleVarbinds->setEnabled(false);
        break;
    case 2: // Editor
        SetEditorMenus(true);
        // Set find func to MIB editor 
        disconnect(MainUI()->actionFind, SIGNAL( triggered() ), 0, 0);
        disconnect(MainUI()->actionFindNext, SIGNAL( triggered() ), 0, 0);
        connect( MainUI()->actionFind, SIGNAL( triggered() ),
                editor, SLOT( Find() ) );
        connect( MainUI()->actionFindNext, SIGNAL( triggered() ),
                editor, SLOT( ExecuteFindNext() ) );
        MainUI()->actionMultipleVarbinds->setEnabled(false);
        break;
    case 3: // Discovery
        SetEditorMenus(false);
        MainUI()->actionMultipleVarbinds->setEnabled(false);
        break;
    case 4: // Traps
        SetEditorMenus(false);
        MainUI()->actionMultipleVarbinds->setEnabled(false);
        break;
    case 5: // Graphs
        SetEditorMenus(false);
        // Set find func to MIB tree
        MainUI()->actionFind->setEnabled(true);
        MainUI()->actionFindNext->setEnabled(true);
        disconnect(MainUI()->actionFind, SIGNAL( triggered() ), 0, 0);
        disconnect(MainUI()->actionFindNext, SIGNAL( triggered() ), 0, 0);
#if 0 //MART
        connect( MainUI()->actionFind, SIGNAL( triggered() ),
                w.PlotMIBTree, SLOT( FindFromNode() ) );
        connect( MainUI()->actionFindNext, SIGNAL( triggered() ),
                w.PlotMIBTree, SLOT( ExecuteFindNext() ) );
#endif
        MainUI()->actionMultipleVarbinds->setEnabled(false);
#if 0 //MART
        // Refresh MIB tree if needed
        w.PlotMIBTree->Populate();
#endif
        break;
    case 6: // Log
        SetEditorMenus(false);
        MainUI()->actionMultipleVarbinds->setEnabled(false);
        break;
    default:
        break;
    }
}

void Snmpb::AboutBox(bool)
{
    QMessageBox::about(MainUI()->TabW, "About SnmpB", QString(
"<H2><b>SnmpB</b></H2><br>                                                       \
Version %1<br>                                                                   \
<a href=http://sourceforge.net/projects/snmpb> \
http://sourceforge.net/projects/snmpb</a><br><br>                                \
                                                                                 \
Copyright (c) <b>Martin Jolicoeur</b> (<a href=\"mailto:snmpb1@gmail.com\">\
snmpb1@gmail.com</a>), 2004-2014<br><br>                                         \
                                                                                 \
SnmpB is an SNMP MIB browser (Simple Network Management Protocol) written in QT. \
It supports SNMPv1, SNMPv2c and SNMPv3. SnmpB can browse/edit/load/add MIB files \
and can query SNMP agents. It also supports agent discovery, trap events, and    \
graph plotting.<br><br>                                                          \
                                                                                 \
This program is covered by the GNU General Public License, version 2 (GPLv2),    \
<a href=http://www.gnu.org/licenses>http://www.gnu.org/licenses</a><br><br>      \
                                                                                 \
This program uses the following libraries, covered by their respective license:\
<br><br>                                                                         \
                                                                                 \
Snmp++ [v%2] (<a href=http://www.agentpp.com>http://www.agentpp.com</a>)<br>     \
Libtomcrypt [v%3] (<a href=http://libtom.org>http://libtom.org</a>)<br>          \
Libsmi [v%4] (<a href=http://www.ibr.cs.tu-bs.de/projects/libsmi>\
http://www.ibr.cs.tu-bs.de/projects/libsmi</a>)<br>                              \
QWT [v%5] (<a href=http://qwt.sourceforge.net>http://qwt.sourceforge.net</a>)<br>\
QT [v%6] (<a href=http://qt-project.org>\
http://qt-project.org</a>)")
        .arg(SNMPB_VERSION_STRING)
        .arg(SNMP_PP_VERSION_STRING)
        .arg(SCRYPT)
        .arg(SMI_VERSION_STRING)
        .arg(QWT_VERSION_STR)
        .arg(QT_VERSION_STR));
}

