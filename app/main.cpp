#include <qapplication.h>
#include <qmainwindow.h>
#include "snmpb.h"
#include "configfiles.h"

int main( int argc, char ** argv )
{
    QApplication a( argc, argv );
    QMainWindow mw;
    Snmpb snmpb(&mw);

    CheckForConfigFiles();

    mw.show();
    a.connect( &a, SIGNAL( lastWindowClosed() ), &a, SLOT( quit() ) );
    
    return a.exec();
}
