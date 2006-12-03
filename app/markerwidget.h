#ifndef __markerwidget_h
#define __markerwidget_h

#include <qwidget.h>
#include <qpixmap.h>
#include <qmap.h>
#include <qtextedit.h>

struct Mark
{
    int line;
    int type;
};

class MarkerWidget: public QWidget
{
    Q_OBJECT
public:
    MarkerWidget( QWidget* = 0);
    virtual ~MarkerWidget();

    void setTextEditor(QTextEdit*);
    void setMarker(int line);

    virtual void setPixmap(int, const QPixmap &);
    virtual void setDescription(int, const QString &);
    virtual void setMarksUserChangable(int markMask);

public slots:
    void doRepaint() { repaint( ); }

protected:
    virtual void paintEvent( QPaintEvent* );

signals:
    void markChanged( struct Mark mark, int action_type );
    void marksChanged();

private:
    QTextEdit*              m_editor;
    QPixmap                 m_pixmap;
    bool                    m_clickChangesBPs;
    bool                    m_changeBookmarksAllowed;
    bool                    m_changeBreakpointsAllowed;
    QString                 m_bookmarkDescr;
    QString                 m_breakpointDescr;
};

#endif // __markerwidget_h
