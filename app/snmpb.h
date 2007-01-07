#ifndef SNMPB_H
#define SNMPB_H

#include "ui_mainw.h"

class MibModule;
class Trap;
class Agent;
class Graph;
class MibEditor;
class LogSnmpb;

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

    void CheckForConfigFiles(void);
    QString GetBootCounterConfigFile(void);
    QString GetMibConfigFile(void);
    QString GetUsmUsersConfigFile(void);

public slots:
    void TreeTabSelected(int index);
    void HorizontalSplit(bool checked);

private:
    Ui_MainW w;

    MibModule *modules;
    MibViewLoader loader;
    Trap *trap;
    Agent *agent;
    Graph *graph;
    MibEditor *editor;
    LogSnmpb *logsnmpb;
};

#endif /* SNMPB_H */

