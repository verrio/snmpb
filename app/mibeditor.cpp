#include <QtGui>
#include <qfileinfo.h>
#include <qpainter.h>
#include "mibeditor.h"

MibEditor::MibEditor(Snmpb *snmpb)
{
    s = snmpb;
 
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

    highlighter = new MibHighlighter(s->MainUI()->MIBFile->document());

    s->MainUI()->MIBFileMarker->setTextEditor(s->MainUI()->MIBFile);

    LineNumberWidget *lnum = new LineNumberWidget(s->MainUI()->MIBFile,
                                                  s->MainUI()->MIBFileStatus);
    s->MainUI()->MIBFileStatus->addPermanentWidget(lnum);
}

void MibEditor::MibFileNew(bool)
{
    s->MainUI()->MIBFile->clear();
    LoadedFile = "";
}

void MibEditor::MibFileOpen(bool)
{
    QString fileName = NULL;

    if (fileName.isNull())
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
}

void MibEditor::SelectedLogEntry(QListWidgetItem *item)
{
    QRegExp expression(" line ([1-9][0-9]*|0): ");
    expression.indexIn(item->text());
    int line = expression.capturedTexts()[1].toInt();

    s->MainUI()->MIBFileMarker->setMarker(line); 
}


LineNumberWidget::LineNumberWidget(QTextEdit *editor, QWidget* parent)
                                   :QWidget( parent )
{
//    setAttribute(Qt::WA_StaticContents);

    setFixedWidth( 100 );

    m_editor = editor;

    connect( m_editor, SIGNAL( cursorPositionChanged() ),
             this, SLOT( doRepaint() ) );

    doRepaint();
}

LineNumberWidget::~LineNumberWidget()
{
}

void LineNumberWidget::paintEvent( QPaintEvent* /*e*/ )
{
    QPainter painter( this );

    QString numbers = QString("Line: %1, Col: %2").
                              arg(m_editor->textCursor().blockNumber()+1).
                              arg(m_editor->textCursor().columnNumber()+1);

    printf("%s\n", numbers.toLatin1().data());

    painter.drawText(0, 0,
            /*Qt::AlignRight | Qt::AlignBottom,*/
            numbers);

    painter.end();
}

