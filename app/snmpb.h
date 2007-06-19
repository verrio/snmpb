#ifndef SNMPB_H
#define SNMPB_H

#include "ui_mainw.h"
#include "ui_agentprofile.h"
#include "ui_usmprofile.h"
#include "ui_preferences.h"

class MibModule;
class Trap;
class Agent;
class Graph;
class MibEditor;
class LogSnmpb;
class Discovery;

class Snmpb: public QObject
{
    Q_OBJECT
    
public:
    Snmpb(QMainWindow *mw);
    Ui_MainW* MainUI(void);
    Ui_AgentProfile* AgentProfileUI(void);
    Ui_USMProfile* USMProfileUI(void);
    Ui_Preferences* PreferencesUI(void);
    Agent* AgentObj(void);
    Trap* TrapObj(void);
    MibViewLoader* MibLoaderObj(void);
    MibModule* MibModuleObj(void);

    void CheckForConfigFiles(void);
    QString GetBootCounterConfigFile(void);
    QString GetMibConfigFile(void);
    QString GetUsmUsersConfigFile(void);

public slots:
    void TreeTabSelected(int index);
    void HorizontalSplit(bool checked);
    void ManageAgentProfiles(bool);
    void ManageUSMProfiles(bool);
    void ManagePreferences(bool);

private:
    Ui_MainW w;
    Ui_AgentProfile ap;
    Ui_USMProfile up;
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

