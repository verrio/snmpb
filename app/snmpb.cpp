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
#define BOOT_COUNTER_CONFIG_FILE "boot_counter.conf"
#define USM_USERS_CONFIG_FILE    "usm_users.conf"
#define AGENTS_CONFIG_FILE       "agents.conf"

char default_mib_config[] = {
"level 0\n\
\n\
load IF-MIB\n\
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

    modules = new MibModule(this);
    trap = new Trap(this);
    logsnmpb = new LogSnmpb(this); // Must be created before the agent object
    agent = new Agent(this);
    graph = new Graph(this);
    editor = new MibEditor(this);
    discovery = new Discovery(this);
    apm = new AgentProfileManager(this);
    upm = new USMProfileManager(this);

    // Connect some signals
    connect( w.TabW, SIGNAL( currentChanged(int) ),
             this, SLOT( TreeTabSelected(int) ) );
    connect( w.actionManageAgentProfiles, SIGNAL( triggered(bool) ),
             this, SLOT( ManageAgentProfiles(bool) ) );
    connect( w.actionManageUSMProfiles, SIGNAL( triggered(bool) ),
             this, SLOT( ManageUSMProfiles(bool) ) );
    connect( w.actionPreferences, SIGNAL( triggered(bool) ),
             this, SLOT( ManagePreferences(bool) ) );

    // Register every MIB tree to the MIB loader object
    w.MIBTree->RegisterToLoader(&loader);
    w.PlotMIBTree->RegisterToLoader(&loader);
    
    TreeTabSelected(0);
}

Ui_MainW* Snmpb::MainUI(void)
{
    return (&w);
}

Ui_Preferences* Snmpb::PreferencesUI(void)
{
    return (&p);
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

QString Snmpb::GetUsmUsersConfigFile(void)
{
    return (SnmpbDir.filePath(USM_USERS_CONFIG_FILE));
}

QString Snmpb::GetAgentsConfigFile(void)
{
    return (SnmpbDir.filePath(AGENTS_CONFIG_FILE));
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
    QDialog w;

    p.setupUi(&w);

    // Set some properties for the Preferences TreeView
    p.PreferencesTree->header()->hide();
    p.PreferencesTree->setSortingEnabled( FALSE );
    p.PreferencesTree->header()->setSortIndicatorShown( FALSE );
    p.PreferencesTree->setLineWidth( 2 );
    p.PreferencesTree->setAllColumnsShowFocus( FALSE );
    p.PreferencesTree->setFrameShape(QFrame::WinPanel);
    p.PreferencesTree->setFrameShadow(QFrame::Plain);
    p.PreferencesTree->setRootIsDecorated( TRUE );

    Preferences prefs(this);

    if(w.exec() == QDialog::Accepted)
    {
        printf("Saving preferences ...\n");
    }
}

void Snmpb::TreeTabSelected( int index )
{
    if (w.TabW->tabText(index) == "Tree")
        w.MIBTree->Populate();
    else if (w.TabW->tabText(index) == "Graphs")
        w.PlotMIBTree->Populate();
}

