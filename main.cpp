#include <qapplication.h>
#include "ui_mainw.h"
#include "mibmodule.h"
#include "agent.h"
#include "trap.h"
#include "graph.h"

int main( int argc, char ** argv )
{
    QApplication a( argc, argv );
    Ui_MainW w;
    Q3MainWindow mw;
    w.setupUi(&mw);

    MibModule modules(w.ModuleInfo,
                      w.UnloadedModules, w.LoadedModules);
    Trap trap (w.TrapLog, w.TrapContent, w.TrapInfo);
    Agent agent(w.UserName, w.SecLevel, w.ContextName,
                w.EngineID, w.AuthProtocol, w.AuthPass,
                w.PrivProtocol, w.PrivPass, w.Address,
                w.Port, w.Retries, w.Timeout,
                w.V1, w.V2, w.V3,
                w.ReadComm, w.WriteComm,
                w.DeleteUser, w.AddUser, w.SaveUser,
                w.MIBTree, w.Query, &trap);
    Graph graph(w.GraphTab, w.GraphCreate, w.GraphDelete,
                w.GraphName, w.PlotObject, w.PlotAdd, w.PlotDelete,
                w.PlotIndex, w.PlotColor, w.PlotShape, w.PlotWidth, 
                w.PlotMIBTree);
 
    CurrentAgent = &agent;

    // TEMP, mart
    // Connect some signals
    QObject::connect( w.TabW, SIGNAL( currentChanged(int index) ),
             w.MIBTree, SLOT( TreeTabSelected(int index) ) );
    w.MIBTree->TreeTabSelected(0);

    mw.show();
    a.connect( &a, SIGNAL( lastWindowClosed() ), &a, SLOT( quit() ) );
    
    return a.exec();
}
