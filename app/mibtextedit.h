/*
    Copyright (C) 2004-2008  Martin Jolicoeur (snmpb1@gmail.com) 

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
#ifndef MIBTEXTEDIT_H
#define MIBTEXTEDIT_H

#include <qwidget.h>
#include <qtextedit.h>

class MibTextEdit: public QTextEdit
{
    Q_OBJECT

public:
    MibTextEdit(QWidget *parent = 0);

    bool canInsertFromMimeData( const QMimeData *source ) const;
    void insertFromMimeData( const QMimeData *source );

signals:
    void FileLoaded( const QString& fileName );
};

#endif /* MIBTEXTEDIT_H */
