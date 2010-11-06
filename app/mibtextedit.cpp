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
#include <QtGui>
#include <qfileinfo.h>
#include <qmessagebox.h>
#include "mibtextedit.h"

MibTextEdit::MibTextEdit(QWidget *parent):QTextEdit(parent)
{
}

bool MibTextEdit::canInsertFromMimeData( const QMimeData *source ) const
{
    if (source->hasUrls() || source->hasText())
        return true;
    else
        return QTextEdit::canInsertFromMimeData(source);
}

void MibTextEdit::insertFromMimeData( const QMimeData *source )
{
    // This is simply pasted text, just add it to the textedit
    if ((source->hasText() == true) && (source->hasUrls() == false))
    {
        insertPlainText(source->text());
        return;
    }

    // Else, this is a dragged & dropped file (or a list of),
    // loop through all dropped files and open them in editor

    // Note: right now, only one opened file at a time is
    // supported so the loop below will overwrite each file in the
    // list with the previous one, leaving only the last one opened.
    QList<QUrl> url_list = source->urls();

    for (int i = 0; i < url_list.size(); i++)
    {
        QString fileName = url_list[i].toLocalFile();

        // Accept only .smi, mib, .pib, or no extension files
        QString ext = QFileInfo(fileName.toLatin1()).suffix();
        if (!fileName.isEmpty() && 
            (ext.isEmpty() || (ext == "smi") || (ext == "mib") || (ext == "pib")))
        {
            QFile file(fileName);

            if (file.open(QIODevice::ReadOnly | QFile::Text))
            {
                setPlainText(file.readAll());
                emit FileLoaded(fileName);
            }
            else
            {
                QMessageBox::critical(NULL, tr("SnmpB: Open MIB File"),
                        tr("Cannot open file %1: %2.\n")
                        .arg(file.fileName())
                        .arg(file.errorString()));
            }

            file.close();
        }
    }
}

