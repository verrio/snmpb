#include <qapplication.h>
#include "mainw.h"

int main( int argc, char ** argv )
{
    QApplication a( argc, argv );
    MainW w;
    w.show();
    a.connect( &a, SIGNAL( lastWindowClosed() ), &a, SLOT( quit() ) );
    
    w.PopulateTree();
    
    return a.exec();
}
