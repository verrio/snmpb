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
    void MibFileNew(void);
    void MibFileOpen(void);
    void MibFileSave(void);
    void MibFileSaveAs(void);
    void VerifyMIB(void);
    void ExtractMIBfromRFC(void);
    void SelectedLogEntry(QListWidgetItem *item);
    void SetLineNumStatus(void); 
    void MibFileModified(bool modified);

private:
    void SetCurrentFileName(const QString &FileName);

private:
    Snmpb *s;
    MibHighlighter *highlighter;
    QLabel *lnum;
    QLabel *lfn;
    
    QString LoadedFile;

    int num_error;
    int num_warning;
    int num_info;
};

#endif /* MIBEDITOR_H */

