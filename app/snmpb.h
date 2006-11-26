#ifndef SNMPB_H
#define SNMPB_H

#include "ui_mainw.h"
#include "mibhighlighter.h"

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

public slots:
    void TreeTabSelected(int index);
    void HorizontalSplit(bool checked);

private:
    Ui_MainW w;

    MibModule *modules;
    Trap *trap;
    Agent *agent;
    Graph *graph;
    MibHighlighter *highlighter;
};

#endif /* SNMPB_H */

