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
