/*
    Copyright (C) 2004-2010  Martin Jolicoeur (snmpb1@gmail.com) 

    This file is part of the SnmpB project 
    (http://sourceforge.net/projects/snmpb)

    This program is free software: you can redistribute it and/or modify
    it under the terms of the GNU General Public License as published by
    the Free Software Foundation, either version 2 of the License, or
    (at your option) any later version.

    This program is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
    GNU General Public License for more details.

    You should have received a copy of the GNU General Public License
    along with this program.  If not, see <http://www.gnu.org/licenses/>.
*/
#include <qapplication.h>
#include <qmainwindow.h>
#include "snmpb.h"
#include "mibeditor.h"

#include <qmessagebox.h>

int main( int argc, char ** argv )
{
    Snmpb snmpb;

    QApplication a( argc, argv );
    QMainWindow mw;
    snmpb.BindToGUI(&mw);

    mw.show();
    a.connect( &a, SIGNAL( lastWindowClosed() ), &a, SLOT( quit() ) );

    // Load a file specified as argument in the Mib Editor
    if (QCoreApplication::arguments().count() > 1)
    {
        snmpb.MibEditorObj()->MibFileOpen(QCoreApplication::arguments().at(1));
        snmpb.MainUI()->TabW->setCurrentIndex(2); // Select the Editor Tab
    }

    return a.exec();
}
