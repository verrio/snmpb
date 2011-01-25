/*
    Copyright (C) 2004-2011 Martin Jolicoeur (snmpb1@gmail.com) 

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
#ifndef MARKERWIDGET_H
#define MARKERWIDGET_H

#include <qwidget.h>
#include <qpixmap.h>
#include <qtextedit.h>

class MarkerWidget: public QWidget
{
    Q_OBJECT
public:
    MarkerWidget(QWidget* = 0);
    virtual ~MarkerWidget();

    void setTextEditor(QTextEdit*);
    void setMarker(int line);

public slots:
    void doRepaint() { repaint(); }

protected:
    virtual void paintEvent( QPaintEvent* );

private:
    QTextEdit  *m_editor;
    QPixmap    m_pixmap;
};

#endif /* MARKERWIDGET_H */
