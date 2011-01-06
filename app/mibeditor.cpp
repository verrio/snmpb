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
#include <qpainter.h>
#include "mibeditor.h"
#include "mibmodule.h"

MibEditor::MibEditor(Snmpb *snmpb)
{
    s = snmpb;

    // Menu items
    connect( s->MainUI()->fileNewAction, SIGNAL( triggered() ),
             this, SLOT( MibFileNew() ) );
    connect( s->MainUI()->fileOpenAction, SIGNAL( triggered() ),
             this, SLOT( MibFileOpen() ) );
    connect( s->MainUI()->fileSaveAction, SIGNAL( triggered() ),
             this, SLOT( MibFileSave() ) );
    connect( s->MainUI()->fileSaveAsAction, SIGNAL( triggered() ),
             this, SLOT( MibFileSaveAs() ) );
    connect( s->MainUI()->actionVerifyMIB, SIGNAL( triggered() ),
             this, SLOT( VerifyMIB() ) );
    connect( s->MainUI()->actionExtractMIBfromRFC, SIGNAL( triggered() ),
             this, SLOT( ExtractMIBfromRFC() ) );
    connect( s->MainUI()->MIBLog, SIGNAL ( itemDoubleClicked ( QListWidgetItem* ) ),
             this, SLOT( SelectedLogEntry ( QListWidgetItem* ) ) );
    connect( s->MainUI()->MIBFile->document(), SIGNAL(modificationChanged(bool)),
             this, SLOT( MibFileModified(bool) ));
    connect( s->MainUI()->MIBFile, SIGNAL( FileLoaded( const QString& ) ),
             this, SLOT( SetCurrentFileName( const QString& ) ));
    connect( s->MainUI()->actionGotoLine, SIGNAL( triggered() ),
             this, SLOT( GotoLine() ) );
    connect( s->MainUI()->actionFind, SIGNAL( triggered() ),
             this, SLOT( Find() ) );
    connect( s->MainUI()->actionReplace, SIGNAL( triggered() ),
             this, SLOT( Replace() ) );
    connect( s->MainUI()->actionFindNext, SIGNAL( triggered() ),
             this, SLOT( ExecuteFindNext() ) );

    // Syntax highlighter
    highlighter = new MibHighlighter(s->MainUI()->MIBFile->document());

    // Marker widget
    s->MainUI()->MIBFileMarker->setTextEditor(s->MainUI()->MIBFile);

    // Line number statusbar widget
    lnum = new QLabel();
    s->MainUI()->MIBFileStatus->addPermanentWidget(lnum);

    // Filename statusbar widget
    lfn = new QLabel();
    s->MainUI()->MIBFileStatus->addWidget(lfn);

    SetCurrentFileName("");

    connect( s->MainUI()->MIBFile, SIGNAL( cursorPositionChanged() ),
             this, SLOT( SetLineNumStatus() ) );

    SetLineNumStatus();

    s->MainUI()->MIBFile->setAcceptDrops(true);

    find_string = "";
    replace_string = "";
}

void MibEditor::SetCurrentFileName(const QString &FileName)
{
    LoadedFile = FileName;
    s->MainUI()->MIBFile->document()->setModified(false);
    MibFileModified(false);
}

void MibEditor::MibFileModified(bool modified)
{
    s->MainUI()->fileSaveAction->setEnabled(modified);

    QString ShownName;
    if (LoadedFile.isEmpty())
        ShownName = "UNTITLED-MIB";
    else
        ShownName = QFileInfo(LoadedFile).fileName();

    if (modified)
        lfn->setText(tr("%1 *").arg(ShownName));
    else
        lfn->setText(tr("%1").arg(ShownName));
}

void MibEditor::MibFileNew(void)
{
    s->MainUI()->MIBFile->clear();
    SetCurrentFileName("");
}

void MibEditor::GotoLine(void)
{
    QDialog d(s->MainUI()->MIBFile);

    goto_uid.setupUi(&d);
    connect( goto_uid.PushButton2, SIGNAL( clicked() ), 
             this, SLOT( ExecuteGotoLine() ));
    connect( goto_uid.PushButton2, SIGNAL( clicked() ), 
             &d, SLOT( accept() ));
    goto_uid.spinLine->setFocus(Qt::TabFocusReason);
    d.exec();
}

void MibEditor::ExecuteGotoLine(void)
{
    QTextBlock currentBlock = s->MainUI()->MIBFile->document()->begin();
    QTextBlock foundBlock;
    int l = 1;
    int found = 0;

    int line = goto_uid.spinLine->value();

    // Loop through the blocks
    while(currentBlock.isValid())
    {
        if (l == line)
        {
            found = 1;
            foundBlock = currentBlock;
            break;
        }

        currentBlock = currentBlock.next();
        l++;
    };

    if (found)
    {
        // Change scrollbar to put the marker visible in the middle of the editor
        int halfViewPortHeight = s->MainUI()->MIBFile->maximumViewportSize().height()/2;
        int yCoord = (int)foundBlock.layout()->position().y();
        int yAdjust = (yCoord < halfViewPortHeight)?yCoord : halfViewPortHeight;
        int halfLineHeight = (int)foundBlock.layout()->boundingRect().height()/2;

        s->MainUI()->MIBFile->verticalScrollBar()->setValue(yCoord - yAdjust);

        // Set the cursor position to the marker line
        QPoint cursorPos(0, yAdjust+halfLineHeight);
        QTextCursor tc = s->MainUI()->MIBFile->cursorForPosition(cursorPos);

        s->MainUI()->MIBFile->setTextCursor(tc);

        // Finally, set the focus to the editor
        s->MainUI()->MIBFile->setFocus(Qt::OtherFocusReason);
    }
}

void MibEditor::Find(void)
{
    QDialog d(s->MainUI()->MIBFile);

    find_uid.setupUi(&d);
    connect( find_uid.buttonFindNext, SIGNAL( clicked() ), 
             this, SLOT( ExecuteFind() ));
    find_uid.comboFind->setFocus(Qt::TabFocusReason);

    find_uid.comboFind->addItems(find_strings);
    if (!find_string.isEmpty())
        find_uid.comboFind->setCurrentIndex(find_uid.comboFind->findText(find_string));
    d.exec();
}

void MibEditor::ExecuteFindNext(void)
{
    Find(false);
}

void MibEditor::ExecuteFind(void)
{
    Find(true);
}

void MibEditor::Find(bool reevaluate)
{
    QTextCursor tc;

    if (reevaluate)
    {
        ff = 0;
        find_string = find_uid.comboFind->currentText();
        if (!find_strings.contains(find_string))
            find_strings.append(find_string);

        if (find_uid.checkWords->isChecked())
            ff |= QTextDocument::FindWholeWords;
        if (find_uid.checkCase->isChecked())
            ff |= QTextDocument::FindCaseSensitively;
        if (find_uid.checkBackward->isChecked())
            ff |= QTextDocument::FindBackward;
    }

    tc = s->MainUI()->MIBFile->document()->find(find_string, 
                                                s->MainUI()->MIBFile->textCursor(), 
                                                ff);

    if (!tc.isNull())
    {
        s->MainUI()->MIBFile->setTextCursor(tc);
        tc.select(QTextCursor::WordUnderCursor);
    }
}

void MibEditor::Replace(void)
{
    QDialog d(s->MainUI()->MIBFile);

    replace_uid.setupUi(&d);
    connect( replace_uid.buttonReplace, SIGNAL( clicked() ), 
             this, SLOT( ExecuteReplace() ));
    connect( replace_uid.buttonFindNext, SIGNAL( clicked() ), 
             this, SLOT( ExecuteFindNextReplace() ));
    connect( replace_uid.buttonReplaceAll, SIGNAL( clicked() ), 
             this, SLOT( ExecuteReplaceAll() ));
    replace_uid.comboFind->setFocus(Qt::TabFocusReason);

    replace_uid.comboFind->addItems(find_strings);
    if (!find_string.isEmpty())
        replace_uid.comboFind->setCurrentIndex(replace_uid.comboFind->findText(find_string));
    replace_uid.comboReplace->addItems(replace_strings);
    if (!replace_string.isEmpty())
        replace_uid.comboReplace->setCurrentIndex(replace_uid.comboReplace->findText(replace_string));
    d.exec();
}

void MibEditor::ExecuteReplace(void)
{
    Replace(true);
}

void MibEditor::ExecuteFindNextReplace(void)
{
    Replace(false);
}

void MibEditor::ExecuteReplaceAll(void)
{
    while(!Replace(true)) ;
}

// 
// Replace text if doreplace is true, otherwise only do "find next"
// Returns true if the find next failed (end of document)
//
bool MibEditor::Replace(bool doreplace)
{
    QTextCursor tc;

    ff = 0;
    find_string = replace_uid.comboFind->currentText(); 
    if (!find_strings.contains(find_string))
        find_strings.append(find_string);
    replace_string = replace_uid.comboReplace->currentText();
    if (!replace_strings.contains(replace_string))
            replace_strings.append(replace_string);

    if (replace_uid.checkWords->isChecked())
        ff |= QTextDocument::FindWholeWords;
    if (replace_uid.checkCase->isChecked())
        ff |= QTextDocument::FindCaseSensitively;
    if (replace_uid.checkBackward->isChecked())
        ff |= QTextDocument::FindBackward;

    tc = s->MainUI()->MIBFile->textCursor();
    if(doreplace && !tc.isNull() && tc.hasSelection() && 
       (tc.selectedText().compare(find_string, 
                                  (replace_uid.checkCase->isChecked()?
                                  Qt::CaseSensitive:Qt::CaseInsensitive))==0))
        tc.insertText(replace_uid.comboReplace->currentText());

    tc = s->MainUI()->MIBFile->document()->find(find_string,
                                                s->MainUI()->MIBFile->textCursor(),
                                                ff);

    if (!tc.isNull())
    {
        s->MainUI()->MIBFile->setTextCursor(tc);
        tc.select(QTextCursor::WordUnderCursor);
    }

    return (tc.isNull()?true:false);
}

void MibEditor::MibFileOpen(QString fileName)
{
    if (!fileName.isEmpty())
    {
        QFile file(fileName);
        if (file.open(QIODevice::ReadOnly | QFile::Text))
        {
            s->MainUI()->MIBFile->setPlainText(file.readAll());
            SetCurrentFileName(fileName);
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

void MibEditor::MibFileOpen(void)
{
    QString fileName = NULL;

    fileName = QFileDialog::getOpenFileName(s->MainUI()->MIBFile,
                                            tr("Open File"), "", 
                                            "MIB Files (*-MIB *-PIB *-SMI *-TC *-TYPES *.mib *.pib *.smi *.MIB *.PIB *.SMI);;All Files (*)");
    MibFileOpen(fileName);
}

void MibEditor::MibFileSave(void)
{
    if (LoadedFile.isEmpty())
        return MibFileSaveAs();

    QFile file(LoadedFile);
    if (!file.open(QFile::WriteOnly))
    {
        QMessageBox::warning(NULL, tr("SnmpB: Save MIB File"),
                             tr("Cannot save file %1: %2\n")
                             .arg(file.fileName())
                             .arg(file.errorString()));
        return;
    }

    QTextStream ts(&file);

    ts << s->MainUI()->MIBFile->toPlainText();
    SetCurrentFileName(LoadedFile);
    file.close();
}

void MibEditor::MibFileSaveAs(void)
{
    QString fileName = NULL;

    fileName  = QFileDialog::getSaveFileName(s->MainUI()->MIBFile, 
                                             tr("Save as..."), "", 
                                             "MIB Files (*-MIB *-PIB *-SMI *-TC *-TYPES *.mib *.pib *.smi *.MIB *.PIB *.SMI);;All Files (*)");
    if (fileName.isEmpty())
        return;
    SetCurrentFileName(fileName);
    return MibFileSave();
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
            num_error++;
            item_brush.setColor(Qt::red);
            break;
        case 4:
        case 5:
            message += "Warning ";
            num_warning++;
            item_brush.setColor(Qt::darkYellow);
            break;
        case 6:
        case 7:
        case 8:
        case 9:
            message += "Info ";
            num_info++;
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

void MibEditor::VerifyMIB(void)
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

    num_error = 0;
    num_warning = 0;
    num_info = 0;

    QString start_msg = QString("Starting MIB verification ...");
    s->MainUI()->MIBLog->addItem(new QListWidgetItem(start_msg, 
                                                     s->MainUI()->MIBLog));

    smiLoadModule(QDir::toNativeSeparators(LoadedFile).toLatin1().data());

    QString stop_msg = QString("Verification completed. %1 errors, %2 warnings, %3 infos")
                               .arg(num_error).arg(num_warning).arg(num_info);
    s->MainUI()->MIBLog->addItem(new QListWidgetItem(stop_msg, 
                                                     s->MainUI()->MIBLog));

    smiSetFlags(saved_flags);

    // Reload everything
    s->MibModuleObj()->Refresh();
}

void MibEditor::ExtractMIBfromRFC(void)
{
    QRegExp module_regexp("^[ \t]*([A-Za-z0-9-]*) *(PIB-)?DEFINITIONS *(::=)? *(BEGIN)? *$");
    QRegExp page_regexp("\\[[pP]age [iv0-9]*\\] *");
    QRegExp macro_regexp("^[ \t]*[A-Za-z0-9-]* *MACRO *::=");
    QRegExp end_regexp("^[ \t]*END[ \t]*$");
    QRegExp blankline_regexp("^[ \t]*$");
    QRegExp blank_regexp("[^ \t]");
    QRegExp leadingspaces_regexp("^([ ]*)");
    QRegExp draft_regexp("^[ ]*Internet[ \\-]Draft");

    QFile file_in("empty");
    QFile file_tmpout("empty");
    QFile file_out("empty");
    QTextStream in(&file_in);
    QTextStream tmpout; 
    QTextStream out; 

    QString line;
    QString module;
    QStringList modules;

    QString dir = NULL, filename = NULL;
    int skipmibfile = 0, skip = 0, skipped = 0, macro = 0, n = 0;

    // Open RFC file
    filename = QFileDialog::getOpenFileName(s->MainUI()->MIBFile,
                                        tr("Open RFC file"), "", 
                                        "RFC files (*.txt);;All Files (*.*)");

    if (!filename.isEmpty())
    {
        file_in.setFileName(filename);
        if (!file_in.open(QFile::ReadOnly | QFile::Text))
        {
            QMessageBox::warning(NULL, tr("SnmpB: Extract MIB from RFC"),
                                 tr("Cannot read file %1: %2\n")
                                 .arg(file_in.fileName())
                                 .arg(file_in.errorString()));
            return;
        }
    }
    else
        return;

    // Ask for directory where to save MIB files 
    dir = QFileDialog::getExistingDirectory(s->MainUI()->MIBFile,
                           tr("Select destination folder for MIB files"), "");

    if (dir.isEmpty())
    {
        QMessageBox::warning(NULL, tr("SnmpB: Extract MIB from RFC"),
                             tr("No directory selected. Aborting.\n"));
        file_in.close();
        return;
    }

    if (!QFileInfo(dir).isWritable())
    {
        QMessageBox::warning(NULL, tr("SnmpB: Extract MIB from RFC"),
                tr("Directory not writable by this user. Aborting.\n"));
        file_in.close();
        return;
    }

    // Extract & save each modules ...

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

            // Create temporary output file
            file_tmpout.setFileName(QDir::tempPath()+"/"+module+".tmp");
            file_tmpout.remove();
            if (!file_tmpout.open(QFile::ReadWrite | QFile::Text))
            {
                QMessageBox::warning(NULL, tr("SnmpB: Extract MIB from RFC"),
                                     tr("Cannot create file %1: %2. Abort.\n")
                                     .arg(file_tmpout.fileName())
                                     .arg(file_tmpout.errorString()));
                file_in.close();
                return;
            }

            tmpout.setDevice(&file_tmpout); 

            // Create output file
            file_out.setFileName(dir+"/"+module);
            if (file_out.exists())
            {
                QMessageBox mb(QMessageBox::Question, 
                               tr("SnmpB: Extract MIB from RFC"), 
                               tr("The file %1 already exist.\n")
                               .arg(file_out.fileName()));
                QPushButton *ob = mb.addButton(tr("Overwrite"), 
                                               QMessageBox::YesRole);
                QPushButton *sb = mb.addButton(tr("Skip"), 
                                               QMessageBox::NoRole);
                mb.exec();

                if (mb.clickedButton() == ob)
                {
                    // overwrite 
                    skipmibfile = 0;
                }
                else if (mb.clickedButton() == sb)
                {
                    // skip 
                    skipmibfile = 1;
                }
            }
            else
                skipmibfile = 0;

            if (!skipmibfile)
            {
                file_out.remove();
                if (!file_out.open(QFile::ReadWrite | QFile::Text))
                {
                    QMessageBox::warning(NULL, tr("SnmpB: Extract MIB from RFC"),
                                         tr("Cannot create file %1: %2. Skipping.\n")
                                         .arg(file_out.fileName())
                                         .arg(file_out.errorString()));
                    skipmibfile = 1;
                }
                else
                    out.setDevice(&file_out);
            }
        }

        // At the end of a page we start the counter skipped to skip the
        // next few lines.
        if (page_regexp.indexIn(line) != -1)
            skipped = 0;

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
            tmpout << line << endl;
        }

        // Remember when we enter a macro definition
        if (macro_regexp.indexIn(line) != -1)
            macro = 1;

        // End of module
        if (end_regexp.indexIn(line) != -1)
        {
            if (macro == 0)
            {
                tmpout.flush(); 
                tmpout.seek(0);

                int strip = 99, p = 0;

                while (tmpout.atEnd() != true)
                {
                    line = tmpout.readLine();

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

                tmpout.seek(0);

                if (!skipmibfile)
                {
                    int num_bl = 0;

                    while (tmpout.atEnd() != true)
                    {
                        line = tmpout.readLine();
                        // For each block of consecutive blank lines,
                        // remove all lines but one.
                        if (blankline_regexp.indexIn(line) != -1)
                        {
                            num_bl++;
                            continue;
                        }
                        else
                        {
                            if (num_bl > 0)
                                out << endl;
                            num_bl = 0;
                        }

                        out << line.remove(0, strip) << endl;
                    }

                    out.flush(); 
                    file_out.close();

                    modules << module;
                }

                file_tmpout.remove();

                module = "";
            }
            else
            {
                macro = 0;
            }
        }
    }

    file_in.close();
 
    if(modules.size() > 0)
    {
        QString module_list;
        for (int i = 0; i < modules.size(); i++)
        {
            module_list += "\n\t";
            module_list +=  modules[i];
        }

        QMessageBox::information(NULL, tr("SnmpB: Extract MIB from RFC"),
                                 tr("%1 MIB module(s) have been extracted. \
The following MIB file(s) were created: %2")
                                 .arg(modules.size())
                                 .arg(module_list));
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
