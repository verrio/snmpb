#include <QtGui>
#include <qfileinfo.h>
#include <qpainter.h>
#include "mibeditor.h"

MibEditor::MibEditor(Snmpb *snmpb)
{
    s = snmpb;

    // Menu items
    connect( s->MainUI()->fileNewAction, SIGNAL( triggered(bool) ),
             this, SLOT( MibFileNew(bool) ) );
    connect( s->MainUI()->fileOpenAction, SIGNAL( triggered(bool) ),
             this, SLOT( MibFileOpen(bool) ) );
    connect( s->MainUI()->fileSaveAction, SIGNAL( triggered(bool) ),
             this, SLOT( MibFileSave(bool) ) );
    connect( s->MainUI()->fileSaveAsAction, SIGNAL( triggered(bool) ),
             this, SLOT( MibFileSaveAs(bool) ) );
    connect( s->MainUI()->actionVerifyMIB, SIGNAL( triggered(bool) ),
             this, SLOT( VerifyMIB(bool) ) );
    connect( s->MainUI()->actionExtractMIBfromRFC, SIGNAL( triggered(bool) ),
             this, SLOT( ExtractMIBfromRFC(bool) ) );
    connect( s->MainUI()->MIBLog, SIGNAL ( itemDoubleClicked ( QListWidgetItem* ) ),
             this, SLOT( SelectedLogEntry ( QListWidgetItem* ) ) );
    connect( s->MainUI()->fileNewAction, SIGNAL( triggered(bool) ),
             this, SLOT( MibFileNew(bool) ) );

    // Syntax highlighter
    highlighter = new MibHighlighter(s->MainUI()->MIBFile->document());

    // Marker widget
    s->MainUI()->MIBFileMarker->setTextEditor(s->MainUI()->MIBFile);

    // Line number statusbar widget
    lnum = new QLabel();
    s->MainUI()->MIBFileStatus->addPermanentWidget(lnum);

    connect( s->MainUI()->MIBFile, SIGNAL( cursorPositionChanged() ),
             this, SLOT( SetLineNumStatus() ) );

    SetLineNumStatus();
}

void MibEditor::MibFileNew(bool)
{
    s->MainUI()->MIBFile->clear();
    LoadedFile = "";
}

void MibEditor::MibFileOpen(bool)
{
    QString fileName = NULL;

    fileName = QFileDialog::getOpenFileName(s->MainUI()->MIBFile,
                                tr("Open File"), "", 
                                "MIB Files (*-MIB *-PIB *.mib *.pib *.smi)");

    if (!fileName.isEmpty())
    {
        QFile file(fileName);
        if (file.open(QIODevice::ReadWrite | QFile::Text))
            s->MainUI()->MIBFile->setPlainText(file.readAll());
        LoadedFile = fileName; 
    }
}

void MibEditor::MibFileSave(bool)
{
}

void MibEditor::MibFileSaveAs(bool)
{
}

void MibEditor::ErrorHandler(char *path, int line, int severity, 
                         char *msg, char *tag)
{
    QString message = NULL;
    QListWidgetItem *item;
    QBrush item_brush;
    (void)path;

    switch (severity)
    {
        case 0:
        case 1:
        case 2:
        case 3:
            message += "Error ";
            item_brush.setColor(Qt::red);
            break;
        case 4:
        case 5:
            message += "Warning ";
            item_brush.setColor(Qt::darkYellow);
            break;
        case 6:
        case 7:
        case 8:
        case 9:
            message += "Info ";
            item_brush.setColor(Qt::blue);
            break;
    }

    message += QString("(level %1), line %2: [%3] %4")
                       .arg(severity).arg(line).arg(tag).arg(msg);
    item = new QListWidgetItem(message, s->MainUI()->MIBLog);
    item->setForeground(item_brush);
    s->MainUI()->MIBLog->addItem(item);
}

MibEditor *CurrentEditorObject = NULL;
static void ErrorHdlr(char *path, int line, int severity, 
                      char *msg, char *tag)
{
    CurrentEditorObject->ErrorHandler(path, line, severity, msg, tag);
}

void MibEditor::VerifyMIB(bool)
{
    int flags =  smiGetFlags();
    int saved_flags = flags;
    flags |= SMI_FLAG_ERRORS;
    flags |= SMI_FLAG_NODESCR;
    smiSetFlags(flags);

    s->MainUI()->MIBLog->clear();
    CurrentEditorObject = this;
    smiSetErrorHandler(ErrorHdlr);
    smiSetErrorLevel(9);

    smiLoadModule(LoadedFile.toLatin1().data());

    smiSetFlags(saved_flags);
}

void MibEditor::ExtractMIBfromRFC(bool)
{
    QRegExp module_regexp("^[ \t]*([A-Za-z0-9-]*) *(PIB-)?DEFINITIONS *(::=)? *(BEGIN)? *$");
    QRegExp page_regexp("\\[[pP]age [iv0-9]*\\] *");
    QRegExp macro_regexp("^[ \t]*[A-Za-z0-9-]* *MACRO *::=");
    QRegExp end_regexp("^[ \t]*END[ \t]*$");
    QRegExp blankline_regexp("^[ \t]*$");
    QRegExp blank_regexp("[^ \t]");
    QRegExp leadingspaces_regexp("^([ ]*)");
    QRegExp draft_regexp("^[ ]*Internet[ \\-]Draft");

    QFile file("empty");
    QFile file2("empty");
    QFile file3("empty");
    QTextStream in(&file);
    QTextStream out; 
    QTextStream out_final; 

    QString line;
    QString module;

    QString fileName = NULL;
    int skip = 0, skipped = 0, macro = 0, n = 0, single = 0;

    // Open RFC file
    fileName = QFileDialog::getOpenFileName(s->MainUI()->MIBFile,
                                        tr("Open RFC file"), "", 
                                        "RFC files (*.txt);;All Files (*.*)");

    if (!fileName.isEmpty())
    {
        file.setFileName(fileName);
        if (!file.open(QFile::ReadOnly | QFile::Text))
        {
            QMessageBox::warning(NULL, tr("SnmpB"),
                                 tr("Cannot read file: %1\n")
                                 .arg(file.errorString()));
            return;
        }
    }

    // First, check how many MIB modules in the RFC file ...
    int num_modules = 0;

    // Process each line 
    while (in.atEnd() != true)
    {
        line = in.readLine();

        // Start of module
        if (module_regexp.indexIn(line) != -1)
        {
            module = module_regexp.cap(1); 
        }

        // Remember when we enter a macro definition
        if (macro_regexp.indexIn(line) != -1)
            macro = 1;

        // End of module
        if (end_regexp.indexIn(line) != -1)
        {
            if (macro == 0)
            {
                num_modules++;
                module = "";
            }
            else
            {
                macro = 0;
            }
        }
    }

    //printf("Number of modules in RFC: %d\n", num_modules);
    QMessageBox::information(NULL, tr("SnmpB"),
                             tr("Found %1 MIB modules in RFC file.\n")
                             .arg(num_modules));

    in.seek(0);

    // Then process & save the modules ...

    // Process each line
    while (in.atEnd() != true)
    {
        line = in.readLine();

        if (draft_regexp.indexIn(line) != -1)
            continue;

        // Start of module
        if (module_regexp.indexIn(line) != -1)
        {
            module = module_regexp.cap(1); 
            skip = 9;
            skipped = -1;
            macro = 0;
            n = 0;

            file2.setFileName(module+".tmp");
            if (!file2.open(QFile::ReadWrite | QFile::Text))
            {
                QMessageBox::warning(NULL, tr("SnmpB"),
                        tr("Cannot create file: %1\n")
                        .arg(file2.errorString()));
                return;
            }

            out.setDevice(&file2); 

            file3.setFileName(module);
            if (!file3.open(QFile::ReadWrite | QFile::Text))
            {
                QMessageBox::warning(NULL, tr("SnmpB"),
                        tr("Cannot create file: %1\n")
                        .arg(file3.errorString()));
                return;
            }

            out_final.setDevice(&file3);
        }

        // At the end of a page we go back one line (which is expected to
        // be a separator line), and start the counter skipped to skip the
        // next few lines.
        if (page_regexp.indexIn(line) != -1)
        {
#if 0
            // Some drafts do not use that separator line. so keep it if
            // there are non-blank characters.
            if (!(line[n] ~ /^[ \t]*$/)) 
            {
                printf("WARNING: the line ::%s:: should be a separator before a page break. It was kept.", line[n]); 
                n--;
            }
#endif
            skipped = 0;
        }

        // If we are skipping...
        if (skipped >= 0)
        {
            skipped++;

            // If we have skipped enough lines to the top of the next page...
            if (skipped >= skip)
            {
                skipped = -1;
            }
            else
            {
                // Finish skipping, if we find a non-empty line, but not before
                // we have skipped four lines. remember the miminum of lines
                // we have ever skipped to keep empty lines in a modules that
                // appear near the top of a page.
                if ((skipped >= 4) && (blank_regexp.indexIn(line) != -1))
                {
                    if (skipped < skip)
                        skip = skipped;

                    skipped = -1;
                }   
            }
        }

        // So, if we are not skipping and inside a module, remember the line.
        if ((skipped == -1) && (module.length() > 0))
        {
            n++;
            out << line << endl;
        }

        // Remember when we enter a macro definition
        if (macro_regexp.indexIn(line) != -1)
            macro = 1;

        // End of module
        if (end_regexp.indexIn(line) != -1)
        {
            if (macro == 0)
            {
                if (single == 0)
                {
                    out.flush(); 
                    out.seek(0);

                    int strip = 99, p = 0;

                    while (out.atEnd() != true)
                    {
                        line = out.readLine();

                        // Find the minimum column that contains non-blank
                        // characters in order to cut a blank prefix off.
                        // Ignore lines that only contain white spaces.
                        if (blankline_regexp.indexIn(line) == -1)
                        {
                            if (leadingspaces_regexp.indexIn(line) != -1)
                            {
                                p = leadingspaces_regexp.cap(1).length(); 
                                if ((p < strip) && (line.length() > p))
                                    strip = p;
                            }
                        }
                    }

                    out.seek(0);

                    while (out.atEnd() != true)
                    {

                        line = out.readLine();
                        out_final << line.remove(0, strip) << endl;
                    }

                    out_final.flush(); 
                    file3.close();
                    file2.remove();

                    printf("%s: %d lines.\n", module.toLatin1().data(), n);
                }

                module = "";
            }
            else
            {
                macro = 0;
            }
        }
    }
}

void MibEditor::SelectedLogEntry(QListWidgetItem *item)
{
    QRegExp expression(" line ([1-9][0-9]*|0): ");
    expression.indexIn(item->text());
    int line = expression.capturedTexts()[1].toInt();

    s->MainUI()->MIBFileMarker->setMarker(line); 
}

void MibEditor::SetLineNumStatus(void)
{
    QString lc = QString("Line: %1, Col: %2").
                     arg(s->MainUI()->MIBFile->textCursor().blockNumber()+1).
                     arg(s->MainUI()->MIBFile->textCursor().columnNumber()+1);

    lnum->setText(lc);
}
