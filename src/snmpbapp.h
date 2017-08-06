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
#include <qstring.h>

#include "snmpb.h"
#include "mibeditor.h"

extern QString file_to_open;

// For MACOSX: when double-clicking on an associated file, the OS will send 
// an event to the app instead of passing the file as a parameter: 
// we have to process the event here.
class snmpbApplication : public QApplication
{
    Q_OBJECT

public:
    snmpbApplication(int & argc, char ** argv): QApplication(argc, argv)
    {
    }

protected:
    bool event(QEvent *event)
    {
        switch (event->type()) {
        case QEvent::FileOpen:
            file_to_open = ((QFileOpenEvent *)event)->file();
            return true;
        default:
            return QApplication::event(event);
        }
    }
};
