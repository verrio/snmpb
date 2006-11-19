#include "snmpb.h"
#include "mibmodule.h"
#include "agent.h"
#include "trap.h"
#include "graph.h"

Snmpb::Snmpb(QMainWindow* mw)
{
    w.setupUi(mw);

    modules = new MibModule(this);
    trap = new Trap(this);
    agent = new Agent(this);
    graph = new Graph(this);
 
    // Connect some signals
    QObject::connect( w.TabW, SIGNAL( currentChanged(int) ),
             w.MIBTree, SLOT( TreeTabSelected(int) ) );
    QObject::connect( w.TabW, SIGNAL( currentChanged(int) ),
             w.PlotMIBTree, SLOT( TreeTabSelected(int) ) );
    w.MIBTree->TreeTabSelected(0);
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

