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

#define SNMPB_CONFIG_DIR         ".snmpb"
#define MIB_CONFIG_FILE          "mib.conf"
#define PATH_CONFIG_FILE         "path.conf"
#define BOOT_COUNTER_CONFIG_FILE "boot_counter.conf"
#define USM_USERS_CONFIG_FILE    "usm_users.conf"
#define AGENTS_CONFIG_FILE       "agents.conf"
#define PREFS_CONFIG_FILE        "preferences.conf"

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
    w.setupUi(mw);

    CheckForConfigFiles();

    connect(&loader, SIGNAL ( LogError(QString) ),
            w.LogOutput, SLOT ( append (QString) ));

    // Creation order is VERY important here
    logsnmpb = new LogSnmpb(this);
    modules = new MibModule(this);
    apm = new AgentProfileManager(this);
    prefs = new Preferences(this);
    trap = new Trap(this);
    agent = new Agent(this);
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
            QFile file(SnmpbDir.filePath(MIB_CONFIG_FILE));
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
    QMessageBox::aboutQt(NULL, "About SnmpB");
}

