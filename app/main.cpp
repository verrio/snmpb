#include <qapplication.h>
#include <qmainwindow.h>
#include "snmpb.h"

int main( int argc, char ** argv )
{
    QApplication a( argc, argv );
    QMainWindow mw;
    Snmpb snmpb(&mw);

    snmpb.CheckForConfigFiles();

    mw.show();
    a.connect( &a, SIGNAL( lastWindowClosed() ), &a, SLOT( quit() ) );
    
    return a.exec();
}
