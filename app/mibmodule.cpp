#include <stdio.h>
#include <string.h>
#include <qfileinfo.h>
#include <qdir.h>
#include <qmessagebox.h> 
#include <qtextstream.h>

#include "mibmodule.h"

LoadedMibModule::LoadedMibModule(SmiModule* mod)
{
    name = mod->name;
    module = mod;
}

void LoadedMibModule::PrintProperties(QString& text)
{
    // Create a table and add elements ...
    text = QString("<table border=\"1\" cellpadding=\"0\" cellspacing=\"0\" align=\"left\">");  
    
    // Add the name
    text += QString("<tr><td><b>Name:</b></td><td><font color=#009000><b>%1</b></font></td>").arg(module->name);
    
    // Add last revision
    SmiRevision * rev = smiGetFirstRevision(module);
    if(rev)
        text += QString("<tr><td><b>Last revision:</b></td><td>%1</td></tr>").arg(asctime(gmtime(&rev->date)));
    
    // Add the description
    text += QString("<tr><td><b>Description:</b></td><td><font face=fixed size=-1 color=blue>");
    text += Qt::convertFromPlainText (module->description);
    text += QString("</font></td></tr>");
    
    // Add root node name
    SmiNode *node = smiGetModuleIdentityNode(module);
    if (node)
        text += QString("<tr><td><b>Root node:</b></td><td>%1</td>").arg(node->name);
    
    // Add required modules
    text += QString("<tr><td><b>Requires:</b></td><td><font color=red>");
    SmiImport * ip = smiGetFirstImport(module);
    SmiImport * ipprev = NULL;
    int first = 1;
    while(ip)    
    {
        if (!ipprev || strcmp(ip->module, ipprev->module))
        {
            if (!first) text += QString("<br>");
            first = 0;
            text += QString("%1").arg(ip->module);
        }
        ipprev = ip;
        ip = smiGetNextImport(ip);
    }
    text += QString("</font></td></tr>");
    
    // Add organization
    text += QString("<tr><td><b>Organization:</b></td><td>");
    text += Qt::convertFromPlainText (module->organization);
    text += QString("</td></tr>");
    
    // Add contact info
    text += QString("<tr><td><b>Contact Info:</b></td><td><font face=fixed size=-1>");
    text += Qt::convertFromPlainText (module->contactinfo);
    text += QString("</font></td></tr>");
             
    text += QString("</table>");
}

char* LoadedMibModule::GetMibLanguage(void)
{
    switch(module->language)
    {
    case SMI_LANGUAGE_SMIV1:
        return "SMIv1";
    case SMI_LANGUAGE_SMIV2:
        return "SMIv2";
    case SMI_LANGUAGE_SMING:
        return "SMIng";
    case SMI_LANGUAGE_SPPI:
        return "SPPI";
    default:
        return "Unknown";
    }
}

MibModule::MibModule(Snmpb *snmpb)
{
    s = snmpb;

    QStringList columns;
    columns << "Module" << "Required" << "Language" << "Path"; 
    s->MainUI()->LoadedModules->setHeaderLabels(columns);

    InitLib(0);
    RebuildTotalList();

    // Connect some signals
    connect( s->MainUI()->UnloadedModules, 
             SIGNAL(itemDoubleClicked ( QTreeWidgetItem *, int )),
             this, SLOT( AddModule() ) );
    connect( s->MainUI()->LoadedModules, 
             SIGNAL(itemDoubleClicked ( QTreeWidgetItem *, int )),
             this, SLOT( RemoveModule() ) );
    connect( s->MainUI()->LoadedModules, SIGNAL(itemSelectionChanged ()),
             this, SLOT( ShowModuleInfo() ) );
    connect( this, SIGNAL(ModuleProperties(const QString&)),
             (QObject*)s->MainUI()->ModuleInfo, 
             SLOT(setHtml(const QString&)) );
    connect( s->MainUI()->ModuleAdd, 
             SIGNAL( clicked() ), this, SLOT( AddModule() ));
    connect( s->MainUI()->ModuleDelete, 
             SIGNAL( clicked() ), this, SLOT( RemoveModule() ));

    
    for(SmiModule *mod = smiGetFirstModule(); 
        mod; mod = smiGetNextModule(mod))
        Wanted.append(mod->name);
    Refresh();
}

void MibModule::ShowModuleInfo(void)
{
    QTreeWidgetItem *item;
    QList<QTreeWidgetItem *> item_list = 
                             s->MainUI()->LoadedModules->selectedItems();

    if ((item_list.count() == 1) && ((item = item_list.first()) != 0))
    {
        QString text;
        LoadedMibModule *lmodule;
        for(int i = 0; i < Loaded.count(); i++)
        { 
            lmodule = Loaded[i];
            if (lmodule->name == item->text(0))
            {
                lmodule->PrintProperties(text);
                emit ModuleProperties(text);
                break;
            }
        }
    }    
}

void MibModule::RebuildTotalList(void)
{
    char    *dir, *smipath, *str;
    char    sep[2] = {PATH_SEPARATOR, 0};
 
    smipath = strdup(smiGetPath());
   
    Total.clear();
    
    for (dir = strtok(smipath, sep); dir; dir = strtok(NULL, sep)) {
        QDir d(dir, QString::null, QDir::Unsorted, 
               QDir::Files | QDir::Readable | QDir::NoSymLinks);
        QStringList list = d.entryList();
        if (list.isEmpty()) continue;
        QStringListIterator it( list );
        const QString *fi;
        
        while ( it.hasNext() ) {
            fi = &it.next();
            // Exclude -orig files
            if (!(((str = strstr(fi->toLatin1(), "-orig")) != NULL) && (strlen(str) == 5)))
                Total.append(fi->toLatin1());
        }    
    }
    
    Total.sort();
    
    free(smipath);
}

bool lessThanLoadedMibModule(const LoadedMibModule *lm1, 
                             const LoadedMibModule *lm2)
{
    return lm1->name < lm2->name;
}

void MibModule::RebuildLoadedList(void)
{
    SmiModule *mod;
    int i = 0;
    LoadedMibModule *lmodule;
    char * required = NULL;
    
    Loaded.clear();
    s->MainUI()->LoadedModules->clear();
    
    mod = smiGetFirstModule();
    
    while (mod)
    {
        lmodule = new LoadedMibModule(mod);
        Loaded.append(lmodule);
        
        if (Wanted.contains(lmodule->name))
            required = "no";
        else
            required = "yes";
    
        QStringList values;
        values << lmodule->name.toLatin1().data() << required
               << lmodule->GetMibLanguage() << lmodule->module->path; 
        new QTreeWidgetItem(s->MainUI()->LoadedModules, values);

        i++;
        mod = smiGetNextModule(mod);
    }
    
    qSort(Loaded.begin(), Loaded.end(), lessThanLoadedMibModule);
}

void MibModule::RebuildUnloadedList(void)
{
    QString current;
    int j;
 
    Unloaded.clear();
    s->MainUI()->UnloadedModules->clear();
    
    for(int i=0; i < Total.count(); i++)
    {
        current = Total[i];
        LoadedMibModule *lmodule = NULL;

        for(j = 0; j < Loaded.count(); j++)
        { 
            lmodule = Loaded[j];
            if (lmodule->name == current) break;
        }

        if (!lmodule || (j >= Loaded.count())) {
            Unloaded.append(current);
            new QTreeWidgetItem(s->MainUI()->UnloadedModules, 
                                QStringList(current));
        }
    }
}

void MibModule::AddModule(void)
{
    QList<QTreeWidgetItem *> item_list = 
                             s->MainUI()->UnloadedModules->selectedItems();

    for (int i = 0; i < item_list.size(); i++)
        Wanted.append(item_list[i]->text(0).toLatin1().data());

    if (item_list.size())
        Refresh();

    SaveWantedModules();
}

void MibModule::RemoveModule(void)
{
    QList<QTreeWidgetItem *> item_list = 
                             s->MainUI()->LoadedModules->selectedItems();

    for (int i = 0; i < item_list.size(); i++)
        Wanted.removeAll(item_list[i]->text(0));

    if (item_list.size())
        Refresh();

    SaveWantedModules();
}

void MibModule::SaveWantedModules(void)
{
    QFile mibs(s->GetMibConfigFile());
    mibs.remove();
    if (!mibs.open(QIODevice::WriteOnly | QIODevice::Text))
    {
        QString err = QString("Unable to save mibs in file %1 !\n")
                              .arg(mibs.fileName());
        QMessageBox::critical ( NULL, "SnmpB error", err, 
                                QMessageBox::Ok, Qt::NoButton);
        return;
    }
    QTextStream out(&mibs);
    for (int i = 0; i < Wanted.size(); i++)
        out << "load " << Wanted[i] << endl;
    mibs.close();
}

void MibModule::Refresh(void)
{
    InitLib(1);
    s->MibLoaderObj()->Load(Wanted);
    RebuildLoadedList();
    RebuildUnloadedList();
    s->MainUI()->LoadedModules->resizeColumnToContents(0);
    s->MainUI()->UnloadedModules->resizeColumnToContents(0);
    s->MainUI()->LoadedModules->sortByColumn(0, Qt::AscendingOrder);
    s->MainUI()->UnloadedModules->sortByColumn(0, Qt::AscendingOrder);
}


void MibModule::RefreshPathChange(void)
{
    smiReadConfig(s->GetPathConfigFile().toLatin1().data(), NULL);

    InitLib(1);

    RebuildTotalList();

    smiReadConfig(s->GetMibConfigFile().toLatin1().data(), NULL);

    Wanted.clear();
    for(SmiModule *mod = smiGetFirstModule(); 
        mod; mod = smiGetNextModule(mod))
        Wanted.append(mod->name);

    Refresh();
}

void MibModule::InitLib(int restart)
{
    int smiflags;
    char *smipath;

    if (restart)
    {
        smipath = strdup(smiGetPath());
        smiExit();
        smiflags = smiGetFlags();
        smiInit(NULL);
        smiSetPath(smipath);
        free(smipath);
    }
    else
    {
        smiInit(NULL);
        smiflags = smiGetFlags();
        smiflags |= SMI_FLAG_ERRORS;
        // Read configuration files
        smiReadConfig(s->GetMibConfigFile().toLatin1().data(), NULL);
        smiReadConfig(s->GetPathConfigFile().toLatin1().data(), NULL);
    }

    smiSetFlags(smiflags);
    smiSetErrorLevel(0);
}
