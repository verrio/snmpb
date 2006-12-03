#include <QtGui>
#include <qtextedit.h>
#include <qpainter.h>
#include <qwidget.h>
#include "markerwidget.h"

MarkerWidget::MarkerWidget(QWidget* parent )
    : QWidget( parent )
      ,m_clickChangesBPs(true)
      ,m_changeBookmarksAllowed(false)
      ,m_changeBreakpointsAllowed(false)
      ,m_bookmarkDescr("Bookmark")
      ,m_breakpointDescr("Breakpoint")
{
    setAttribute(Qt::WA_StaticContents);

    m_pixmap = QPixmap( ":/images/scalar.png" );

    setFixedWidth( 15 );
}

MarkerWidget::~MarkerWidget()
{
}

void MarkerWidget::setTextEditor(QTextEdit* editor)
{
    m_editor = editor;

    connect( (QObject*)m_editor->verticalScrollBar(), SIGNAL( valueChanged( int ) ),
             this, SLOT( doRepaint() ) );
    connect( m_editor, SIGNAL( textChanged() ),
             this, SLOT( doRepaint() ) );

    doRepaint();
}

void MarkerWidget::paintEvent( QPaintEvent* /*e*/ )
{
    QPainter mypainter( this );

    QTextBlock currentBlock = m_editor->document()->begin();
    int l = 1;
    int viewPortHeight = m_editor->maximumViewportSize().height();
    int yOffset = m_editor->verticalScrollBar()->value();

    while(currentBlock.isValid())
    {
        int yCoord = (int)currentBlock.layout()->position().y();
        int lineHeight = (int)currentBlock.layout()->boundingRect().height();

//        printf("yOffset: %d, yCoord: %d, %f\n", yOffset, yCoord, currentBlock.layout()->boundingRect().height());
        if(currentBlock.userState() == 0xFF)
            if ((yCoord + lineHeight >= yOffset) && (yCoord - yOffset <= viewPortHeight))
                mypainter.drawPixmap(0, yCoord - yOffset + (((lineHeight-m_pixmap.height())/2)), m_pixmap );

        currentBlock = currentBlock.next();
        l++;
    };

    mypainter.end();
}

void MarkerWidget::setMarker( int line )
{
    QTextBlock currentBlock = m_editor->document()->begin();
    int l = 1;
    int changed = 0;

    while(currentBlock.isValid())
    {
        if (l == line)
        {
            if (currentBlock.userState() != 0xFF)
            {
                changed = 1;
                currentBlock.setUserState(0xFF);
            }
            break;
        }
        currentBlock = currentBlock.next();
        l++;
    };

//    QTextCursor tc = m_editor->cursorForPosition ( QPoint(0, (int)currentBlock.layout()->position().y()) );
    m_editor->verticalScrollBar()->setValue((int)currentBlock.layout()->position().y());
//    m_editor->setTextCursor(tc);
//    m_editor->ensureCursorVisible();

    if (changed)
        doRepaint();
}

void MarkerWidget::setPixmap(int mt, const QPixmap & pm)
{
  if (mt)
    m_pixmap = pm;
}

void MarkerWidget::setDescription(int mt, const QString & s)
{
  switch (mt) {
  case 1: m_bookmarkDescr = s; break;
  case 2: m_breakpointDescr = s; break;
  default: break;
  }
}

void MarkerWidget::setMarksUserChangable(int markMask)
{
  m_changeBookmarksAllowed   = (markMask & 1) ? true : false;
  m_changeBreakpointsAllowed = (markMask & 2) ? true : false;

  doRepaint();
}

