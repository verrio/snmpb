#include <qapplication.h>
#include "mainw.h"
#include "mibmodule.h"
#include "agent.h"

int main( int argc, char ** argv )
{
    QApplication a( argc, argv );
    MainW w;
    
    MibModule modules(w.MIBTree, w.ModuleInfo, 
                      w.UnloadedModules, w.LoadedModules);
    Agent agent(w.UserName, w.SecLevel, w.ContextName,
                w.EngineID, w.AuthProtocol, w.AuthPass,
                w.PrivProtocol, w.PrivPass, w.Address,
                w.Port, w.Retries, w.Timeout,
                w.V1, w.V2, w.V3,
                w.ReadComm, w.WriteComm, 
                w.DeleteUser, w.AddUser, w.SaveUser,
                w.MIBTree, w.Query, w.TrapLog);
    
    w.show();
    a.connect( &a, SIGNAL( lastWindowClosed() ), &a, SLOT( quit() ) );
    
    modules.Refresh();
    
    return a.exec();
}
