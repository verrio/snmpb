#ifndef SNMPB_H
#define SNMPB_H

#include "ui_mainw.h"

class MibModule;
class Trap;
class Agent;
class Graph;

class Snmpb: public QObject
{
    Q_OBJECT
    
public:
    Snmpb(QMainWindow *mw);
    Ui_MainW* MainUI(void);
    Agent* AgentObj(void);
    Trap* TrapObj(void);

    void CheckForConfigFiles(void);
    QString GetBootCounterConfigFile(void);
    QString GetMibConfigFile(void);
    QString GetUsmUsersConfigFile(void);

private:
    Ui_MainW w;

    MibModule *modules;
    Trap *trap;
    Agent *agent;
    Graph *graph;
};

#endif /* SNMPB_H */

