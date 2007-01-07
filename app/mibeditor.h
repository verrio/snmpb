#ifndef MIBEDITOR_H
#define MIBEDITOR_H

#include <qwidget.h>
#include <qlabel.h>
#include "snmpb.h"
#include "mibhighlighter.h"

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
    void SetLineNumStatus(void); 

private:
    Snmpb *s;
    MibHighlighter *highlighter;
    QLabel *lnum;
    
    QString LoadedFile;

    int num_error;
    int num_warning;
    int num_info;
};

#endif /* MIBEDITOR_H */
