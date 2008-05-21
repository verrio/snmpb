/*
    Copyright (C) 2004-2008  Martin Jolicoeur (snmpb1@gmail.com) 

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

Snmpb::Snmpb(QMainWindow* mw)
{
    // First thing to do is to give up root privileges and allow permission to
    // bind on privileged ports (<1024). This is needed to bind on 
    // the RFC-defined trap port number 162 on UNIX machines.
    agent = new Agent(this);
    prefs = NULL;

    // If the preferences file exists, get the trap port number from it, otherwise
    // bind to the standard trap port
    if (QFile(GetPrefsConfigFile()).exists())
    {
        prefs = new Preferences(this);
#ifndef WIN32 
        // Allows to bind on privileged ports only if it is the standard trap port...
        if (prefs->GetTrapPort() != STANDARD_TRAP_PORT)
            setuid(getuid());
#endif
        agent->BindTrapPort(prefs->GetTrapPort());
    }
    else
    {
        // The default trap port is the standard one ...
        agent->BindTrapPort(STANDARD_TRAP_PORT);
    }
#ifndef WIN32 
    // Drop root privileges
    if (setuid(getuid()) < 0)
    {
        QMessageBox::warning ( NULL, "SnmpB", 
                               tr("Unable to drop root privileges: %m\n"),
                               QMessageBox::Ok, Qt::NoButton);
    }
#endif
    // Note: beware as anything before this point is run as root on UNIX ... 

    CheckForConfigFiles();

    if (!prefs)
        prefs = new Preferences(this);

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
    graph = new Graph(this);
    editor = new MibEditor(this);
    discovery = new Discovery(this);

    // Connect some signals
    connect( w.TabW, SIGNAL( currentChanged(int) ),
             this, SLOT( TreeTabSelected() ) );
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
    w.PlotMIBTree->RegisterToLoader(&loader);
    
    TreeTabSelected();
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

void Snmpb::TreeTabSelected(void)
{
    if (w.TabW->tabText(w.TabW->currentIndex()) == "Tree")
        w.MIBTree->Populate();
    else if (w.TabW->tabText(w.TabW->currentIndex()) == "Graphs")
        w.PlotMIBTree->Populate();
}

void Snmpb::AboutBox(bool)
{
    QMessageBox mb;

    mb.setWindowTitle("About SnmpB");
    mb.setIconPixmap(QPixmap(":/images/snmpb.png"));

    mb.setText(QString(
"<H2><b>SnmpB</b></H2><br>                                                       \
Version %1<br>                                                                   \
<a href=http://sourceforge.net/projects/snmpb> \
http://sourceforge.net/projects/snmpb</a><br><br>                                \
                                                                                 \
Copyright (c) <b>Martin Jolicoeur</b> (<a href=\"mailto:snmpb1@gmail.com\">\
snmpb1@gmail.com</a>), 2004-2008<br><br>                                         \
                                                                                 \
SnmpB is an SNMP MIB browser (Simple Network Management Protocol) written in QT. \
It supports SNMPv1, SNMPv2c and SNMPv3. SnmpB can browse/edit/load/add MIB files \
and can query SNMP agents. It also supports agent discovery, trap events, and \
graph plotting.<br><br>                                                          \
                                                                                 \
This program is covered by the GNU General Public License, version 2 (GPLv2), \
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
QT [v%6] (<a href=http://trolltech.com/products/qt>\
http://trolltech.com/products/qt</a>)")
        .arg(SNMPB_VERSION_STRING)
        .arg(SNMP_PP_VERSION_STRING)
        .arg(SCRYPT)
        .arg(SMI_VERSION_STRING)
        .arg(QWT_VERSION_STR)
        .arg(QT_VERSION_STR));

    mb.exec();
}

