#ifndef SNMPB_H
#define SNMPB_H

#include "ui_mainw.h"

class MibModule;
class Trap;
class Agent;
class Graph;
class MibEditor;
class LogSnmpb;
class Discovery;
class AgentProfileManager;
class USMProfileManager;
class Preferences;

class Snmpb: public QObject
{
    Q_OBJECT
    
public:
    Snmpb(QMainWindow *mw);
    Ui_MainW* MainUI(void);
    Agent* AgentObj(void);
    Trap* TrapObj(void);
    MibViewLoader* MibLoaderObj(void);
    MibModule* MibModuleObj(void);
    AgentProfileManager* APManagerObj(void);
    USMProfileManager* UPManagerObj(void);
    Preferences* PreferencesObj(void);

    void CheckForConfigFiles(void);
    QString GetBootCounterConfigFile(void);
    QString GetMibConfigFile(void);
    QString GetPathConfigFile(void);
    QString GetUsmUsersConfigFile(void);
    QString GetAgentsConfigFile(void);
    QString GetPrefsConfigFile(void);

public slots:
    void TreeTabSelected(void);
    void ManageAgentProfiles(bool);
    void ManageUSMProfiles(bool);
    void ManagePreferences(bool);
    void AboutBox(bool);

private:
    Ui_MainW w;
    AgentProfileManager *apm;
    USMProfileManager *upm;
    Preferences *prefs;

    MibModule *modules;
    MibViewLoader loader;
    Trap *trap;
    Agent *agent;
    Graph *graph;
    MibEditor *editor;
    LogSnmpb *logsnmpb;
    Discovery *discovery;
};

#endif /* SNMPB_H */

