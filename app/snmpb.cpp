#include <QtGui>
#include <qfileinfo.h>
#include <qdir.h>
#include <qmessagebox.h>
#include "snmpb.h"
#include "mibmodule.h"
#include "agent.h"
#include "trap.h"
#include "graph.h"

#define SNMPB_CONFIG_DIR         ".snmpb"
#define MIB_CONFIG_FILE          "mib.conf"
#define BOOT_COUNTER_CONFIG_FILE "boot_counter.conf"
#define USM_USERS_CONFIG_FILE    "usm_users.conf"

char default_mib_config[] = {
"level 0\n\
\n\
load IF-MIB\n\
load RFC1213-MIB\n\
load SNMP-FRAMEWORK-MIB\n\
load SNMP-NOTIFICATION-MIB\n\
load SNMPv2-CONF\n\
load SNMPv2-SMI\n\
load SNMPv2-TC\n\
load SNMPv2-TM\n\
load SNMP-VIEW-BASED-ACM-MIB"
};

static QDir SnmpbDir = QDir::homePath() + "/" + SNMPB_CONFIG_DIR;

Snmpb::Snmpb(QMainWindow* mw)
{
    w.setupUi(mw);

    modules = new MibModule(this);
    trap = new Trap(this);
    agent = new Agent(this);
    graph = new Graph(this);

    // Connect some signals
    connect( w.TabW, SIGNAL( currentChanged(int) ),
             this, SLOT( TreeTabSelected(int) ) );
    connect( w.optionsHorizontal_splitAction, SIGNAL( toggled(bool) ),
             this, SLOT( HorizontalSplit(bool) ) );
    connect( w.fileNewAction, SIGNAL( triggered(bool) ),
             this, SLOT( MibFileNew(bool) ) );
    connect( w.fileOpenAction, SIGNAL( triggered(bool) ),
             this, SLOT( MibFileOpen(bool) ) );
    connect( w.fileSaveAction, SIGNAL( triggered(bool) ),
             this, SLOT( MibFileSave(bool) ) );
    connect( w.fileSaveAsAction, SIGNAL( triggered(bool) ),
             this, SLOT( MibFileSaveAs(bool) ) );
    connect( w.actionVerifyMIB, SIGNAL( triggered(bool) ),
             this, SLOT( VerifyMIB(bool) ) );
    connect( w.actionExtractMIBfromRFC, SIGNAL( triggered(bool) ),
             this, SLOT( ExtractMIBfromRFC(bool) ) );
    connect( w.MIBLog, SIGNAL ( itemDoubleClicked ( QListWidgetItem* ) ),
             this, SLOT( SelectedLogEntry ( QListWidgetItem* ) ) );

    TreeTabSelected(0);

    highlighter = new MibHighlighter(w.MIBFile->document());
}

Ui_MainW* Snmpb::MainUI(void)
{
    return (&w);
}

Agent* Snmpb::AgentObj(void)
{
    return (agent);
}

Trap* Snmpb::TrapObj(void)
{
    return (trap);
}

void Snmpb::CheckForConfigFiles(void)
{
    if (!SnmpbDir.exists())
    {
        if(!SnmpbDir.mkdir(SnmpbDir.absolutePath()))
        {
            QString err = QString("Cannot create configuration directory : %1\n")
                          .arg(SnmpbDir.absolutePath().toLatin1().data());
            QMessageBox::warning ( NULL, "SnmpB", err, 
                                   QMessageBox::Ok, Qt::NoButton);
        }
        else
        {
            // Create default mib.conf file.
            QFile file(SnmpbDir.filePath(MIB_CONFIG_FILE));
            if (!file.open(QIODevice::ReadWrite))
            {
                QString err = QString("Cannot create configuration file : %1\n")
                                     .arg(file.fileName());
                QMessageBox::warning ( NULL, "SnmpB", err, 
                                       QMessageBox::Ok, Qt::NoButton);
            }
            else
            {
                file.write(default_mib_config, strlen(default_mib_config));
                file.close();
            }
        }
    }
}

QString Snmpb::GetBootCounterConfigFile(void)
{
    return (SnmpbDir.filePath(BOOT_COUNTER_CONFIG_FILE));
}

QString Snmpb::GetMibConfigFile(void)
{
    return (SnmpbDir.filePath(MIB_CONFIG_FILE));
}

QString Snmpb::GetUsmUsersConfigFile(void)
{
    return (SnmpbDir.filePath(USM_USERS_CONFIG_FILE));
}

void Snmpb::HorizontalSplit(bool checked)
{
    w.QuerySplitter->setOrientation(checked==FALSE?Qt::Horizontal:Qt::Vertical);
}

void Snmpb::TreeTabSelected( int index )
{
    if (w.TabW->tabText(index) == "Tree")
        w.MIBTree->Populate();
    else if (w.TabW->tabText(index) == "Graphs")
        w.PlotMIBTree->Populate();
}

void Snmpb::MibFileNew(bool)
{
    w.MIBFile->clear();
    LoadedFile = "";
}

void Snmpb::MibFileOpen(bool)
{
    QString fileName = NULL;

    if (fileName.isNull())
        fileName = QFileDialog::getOpenFileName(w.MIBFile,
                                tr("Open File"), "", 
                                "MIB Files (*-MIB *-PIB *.mib *.pib *.smi)");

    if (!fileName.isEmpty())
    {
        QFile file(fileName);
        if (file.open(QIODevice::ReadWrite | QFile::Text))
            w.MIBFile->setPlainText(file.readAll());
        LoadedFile = fileName; 
    }
}

void Snmpb::MibFileSave(bool)
{
}

void Snmpb::MibFileSaveAs(bool)
{
}

void Snmpb::ErrorHandler(char *path, int line, int severity, 
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
    item = new QListWidgetItem(message, w.MIBLog);
    item->setForeground(item_brush);
    w.MIBLog->addItem(item);
}

Snmpb *CurrentSnmpbObject = NULL;
static void ErrorHdlr(char *path, int line, int severity, 
                      char *msg, char *tag)
{
    CurrentSnmpbObject->ErrorHandler(path, line, severity, msg, tag);
}

void Snmpb::VerifyMIB(bool)
{
    int flags =  smiGetFlags();
    int saved_flags = flags;
    flags |= SMI_FLAG_ERRORS;
    flags |= SMI_FLAG_NODESCR;
    smiSetFlags(flags);

    w.MIBLog->clear();
    CurrentSnmpbObject = this;
    smiSetErrorHandler(ErrorHdlr);
    smiSetErrorLevel(9);

    smiLoadModule(LoadedFile.toLatin1().data());

    smiSetFlags(saved_flags);
}

void Snmpb::ExtractMIBfromRFC(bool)
{
}

void Snmpb::SelectedLogEntry(QListWidgetItem *item)
{
    printf("Selected!\n");
}

