#ifndef SNMPB_H
#define SNMPB_H

#include "ui_mainw.h"
#include "ui_usmprofile.h"
#include "ui_preferences.h"

class MibModule;
class Trap;
class Agent;
class Graph;
class MibEditor;
class LogSnmpb;
class Discovery;
class AgentProfileManager;
class USMProfileManager;

class Snmpb: public QObject
{
    Q_OBJECT
    
public:
    Snmpb(QMainWindow *mw);
    Ui_MainW* MainUI(void);
    Ui_Preferences* PreferencesUI(void);
    Agent* AgentObj(void);
    Trap* TrapObj(void);
    MibViewLoader* MibLoaderObj(void);
    MibModule* MibModuleObj(void);
    AgentProfileManager* APManagerObj(void);
    USMProfileManager* UPManagerObj(void);

    void CheckForConfigFiles(void);
    QString GetBootCounterConfigFile(void);
    QString GetMibConfigFile(void);
    QString GetUsmUsersConfigFile(void);
    QString GetAgentsConfigFile(void);

public slots:
    void TreeTabSelected(int index);
    void ManageAgentProfiles(bool);
    void ManageUSMProfiles(bool);
    void ManagePreferences(bool);

private:
    Ui_MainW w;
    AgentProfileManager *apm;
    USMProfileManager *upm;
    Ui_Preferences p;

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

