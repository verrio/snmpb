#include <qapplication.h>
#include "mainw.h"
#include "mibmodule.h"

int main( int argc, char ** argv )
{
    QApplication a( argc, argv );
    MainW w;
    MibModule modules;
    
    w.show();
    a.connect( &a, SIGNAL( lastWindowClosed() ), &a, SLOT( quit() ) );
    
    w.PopulateTree();
    modules.Load(&w);
    
    return a.exec();
}
