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

#include <QtGui>
#include <QScrollBar>
#include <qpainter.h>

#include "markerwidget.h"

MarkerWidget::MarkerWidget(QWidget* parent ):QWidget( parent )
{
    setAttribute(Qt::WA_StaticContents);

    m_pixmap = QPixmap( ":/images/marker.png" );

    setFixedWidth( 15 );
}

MarkerWidget::~MarkerWidget()
{
}

void MarkerWidget::setTextEditor(QTextEdit* editor)
{
    m_editor = editor;

    connect( m_editor->verticalScrollBar(), SIGNAL( valueChanged( int ) ),
             this, SLOT( doRepaint() ) );
    connect( m_editor, SIGNAL( textChanged() ),
             this, SLOT( doRepaint() ) );

    doRepaint();
}

void MarkerWidget::paintEvent( QPaintEvent* /*e*/ )
{
    QPainter painter( this );

    QTextBlock currentBlock = m_editor->document()->begin();
    int viewPortHeight = m_editor->maximumViewportSize().height();
    int yOffset = m_editor->verticalScrollBar()->value();

    // Loop through the blocks
    while(currentBlock.isValid())
    {
        // Found a line that needs a marker
        if(currentBlock.userState() == 0xFF)
        {
            int yCoord = (int)currentBlock.layout()->position().y();
            int lineHeight = (int)currentBlock.layout()->boundingRect().height();
            int imageOffset = (lineHeight-m_pixmap.height())/2;

            // Draw the marker pixmap if visible
            if ((yCoord + lineHeight >= yOffset) && 
                (yCoord - yOffset <= viewPortHeight))
                painter.drawPixmap(0, yCoord - yOffset + imageOffset, m_pixmap);
        }

        currentBlock = currentBlock.next();
    };

    painter.end();
}

void MarkerWidget::setMarker( int line )
{
    QTextBlock currentBlock = m_editor->document()->begin();
    QTextBlock foundBlock;
    int l = 1;
    int changed = 0, found = 0;

    // Loop through the blocks
    while(currentBlock.isValid())
    {
        if (l == line)
        {
            if (currentBlock.userState() != 0xFF)
            {
                changed = 1;
                currentBlock.setUserState(0xFF);
            }

            found = 1;
            foundBlock = currentBlock;
        }
        else
        {
            // Clear all other lines
            if (currentBlock.userState() == 0xFF)
            {
                changed = 1;
                currentBlock.setUserState(0x00);
            }
        }

        currentBlock = currentBlock.next();
        l++;
    };

    if (found)
    {
        // Change scrollbar to put the marker visible in the middle of the editor
        int halfViewPortHeight = m_editor->maximumViewportSize().height()/2;
        int yCoord = (int)foundBlock.layout()->position().y();
        int yAdjust = (yCoord < halfViewPortHeight)?yCoord : halfViewPortHeight;
        int halfLineHeight = (int)foundBlock.layout()->boundingRect().height()/2;

        m_editor->verticalScrollBar()->setValue(yCoord - yAdjust);

        // Set the cursor position to the marker line
        QPoint cursorPos(0, yAdjust+halfLineHeight);
        QTextCursor tc = m_editor->cursorForPosition(cursorPos);

        m_editor->setTextCursor(tc);

        // Finally, set the focus to the editor
        m_editor->setFocus(Qt::OtherFocusReason);
    }

    if (changed)
        doRepaint();
}

