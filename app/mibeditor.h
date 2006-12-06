#ifndef MIBEDITOR_H
#define MIBEDITOR_H

#include <qwidget.h>
#include "snmpb.h"
#include "mibhighlighter.h"

class LineNumberWidget: public QWidget
{
    Q_OBJECT
public:
    LineNumberWidget(QTextEdit*, QWidget* = 0);
    virtual ~LineNumberWidget();

public slots:
    void doRepaint() { repaint(); }

protected:
    virtual void paintEvent( QPaintEvent* );

private:
    QTextEdit  *m_editor;
};

class MibEditor: public QObject
{
    Q_OBJECT
    
public:
    MibEditor(Snmpb *snmpb);
    void ErrorHandler(char *path, int line, int severity, 
                      char *msg, char *tag);

public slots:
    void MibFileNew(bool checked);
    void MibFileOpen(bool checked);
    void MibFileSave(bool checked);
    void MibFileSaveAs(bool checked);
    void VerifyMIB(bool checked);
    void ExtractMIBfromRFC(bool checked);
    void SelectedLogEntry(QListWidgetItem *item);
    
private:
    Snmpb *s;
    MibHighlighter *highlighter;

    QString LoadedFile;
};

#endif /* MIBEDITOR_H */

