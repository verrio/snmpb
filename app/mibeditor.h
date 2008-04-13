#ifndef MIBEDITOR_H
#define MIBEDITOR_H

#include <qwidget.h>
#include <qlabel.h>
#include "snmpb.h"
#include "mibhighlighter.h"
#include "ui_gotoline.h"
#include "ui_find.h"
#include "ui_replace.h"

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
    void GotoLine(void);
    void Find(void);
    void Replace(void);
    void ExecuteGotoLine(void);
    void ExecuteFind(void);
    void ExecuteFindNext(void);
    void ExecuteReplace(void);
    void ExecuteFindNextReplace(void);
    void ExecuteReplaceAll(void);

private:
    void SetCurrentFileName(const QString &FileName);
    void Find(bool reevaluate);
    bool Replace(bool doreplace);

private:
    Snmpb *s;
    Ui_GotoLineDialog goto_uid;
    Ui_FindDialog find_uid;
    Ui_ReplaceDialog replace_uid;

    MibHighlighter *highlighter;
    QLabel *lnum;
    QLabel *lfn;
    
    QString LoadedFile;

    int num_error;
    int num_warning;
    int num_info;

    QTextDocument::FindFlags ff;
    QStringList find_strings;
    QString find_string;
    QStringList replace_strings;
    QString replace_string;
};

#endif /* MIBEDITOR_H */

