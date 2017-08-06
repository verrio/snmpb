/**
 * Copyright (c) 2004-2017 Martin Jolicoeur and contributors
 *
 * This file is part of the snmpb project and is free software: you can
 * redistribute it and/or modify it under the terms of the GNU General
 * Public License as published by the Free Software Foundation, either
 * version 3 of the License, or (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <http://www.gnu.org/licenses/>.
 */

#include <qapplication.h>
#include <qmainwindow.h>
#include "snmpb.h"
#include "mibeditor.h"
#include "snmpbapp.h"

QString file_to_open;

int main( int argc, char ** argv )
{
    Snmpb snmpb;
    snmpbApplication a( argc, argv );
    QMainWindow mw;
    snmpb.BindToGUI(&mw);

    mw.show();
    a.connect( &a, SIGNAL( lastWindowClosed() ), &a, SLOT( quit() ) );

    // Load a file specified as argument in the Mib Editor
    if (!file_to_open.isEmpty() || QCoreApplication::arguments().count() > 1)
    {
        snmpb.MibEditorObj()->MibFileOpen(file_to_open.isEmpty()?QCoreApplication::arguments().at(1):file_to_open);
        snmpb.MainUI()->TabW->setCurrentIndex(2); // Select the Editor Tab
    }

    return a.exec();
}
