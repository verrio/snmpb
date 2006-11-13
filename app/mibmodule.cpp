#include <stdio.h>
#include <string.h>
#include <qfileinfo.h>
#include <qdir.h>

#include "configfiles.h"
#include "mibmodule.h"

#define PATH_SEPARATOR ';'

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

MibModule::MibModule(QTextEdit *MI, QTreeWidget *AM, QTreeWidget *LM)
{
    ModuleInfo = MI;
    UnloadedM = AM;
    LoadedM = LM;

    QStringList columns;
    columns << "Module" << "Required" << "Language" << "Path"; 
    LoadedM->setHeaderLabels(columns);

    InitLib(0);
    RebuildTotalList();

    // Connect some signals
    connect( UnloadedM, SIGNAL(itemDoubleClicked ( QTreeWidgetItem *, int )),
             this, SLOT( AddModule() ) );
    connect( LoadedM, SIGNAL(itemDoubleClicked ( QTreeWidgetItem *, int )),
             this, SLOT( RemoveModule() ) );
    connect( LoadedM, SIGNAL(itemSelectionChanged ()),
             this, SLOT( ShowModuleInfo() ) );
    connect( this, SIGNAL(ModuleProperties(const QString&)),
             (QObject*)ModuleInfo, SLOT(setHtml(const QString&)) );
    
    for(SmiModule *mod = smiGetFirstModule(); 
        mod; mod = smiGetNextModule(mod))
        Wanted.append(mod->name);
    Refresh();
}

void MibModule::ShowModuleInfo(void)
{
    QTreeWidgetItem *item;
    QList<QTreeWidgetItem *> item_list = LoadedM->selectedItems();

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
    LoadedM->clear();
    
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
        new QTreeWidgetItem(LoadedM, values);

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
    UnloadedM->clear();
    
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
            new QTreeWidgetItem(UnloadedM, QStringList(current));
        }
    }
}

void MibModule::AddModule(void)
{
    QTreeWidgetItem *item;//, *nextitem  = NULL;
    //    char buf[200];
    QList<QTreeWidgetItem *> item_list = UnloadedM->selectedItems();

    if ((item_list.count() == 1) && ((item = item_list.first()) != 0))
    {
        // Save string of next item to restore selection ...
        //        nextitem = item->itemBelow() ? item->itemBelow():item->itemAbove();
        //        if (nextitem)
        //            strcpy(buf, nextitem->text(0).toLatin1().data());
        
        Wanted.append(item->text(0).toLatin1().data());
        Refresh();
        
        // Restore selection
        //        if (nextitem) {
        //	nextitem = AvailM->findItem(buf, 0);
        //	AvailM->setSelected(nextitem, TRUE);
        //	AvailM->ensureItemVisible(nextitem);
        //       }    
    }
}

void MibModule::RemoveModule(void)
{
    QTreeWidgetItem *item;
    QList<QTreeWidgetItem *> item_list = LoadedM->selectedItems();

    if ((item_list.count() == 1) && ((item = item_list.first()) != 0))
    {
        Wanted.removeAll(item->text(0));
        Refresh();
    }
}

void MibModule::Refresh(void)
{
    InitLib(1);
    MibLoader.Load(Wanted);
    RebuildLoadedList();
    RebuildUnloadedList();
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
        // Read configuration file
        smiReadConfig(GetMibConfigFile().toLatin1().data(), NULL);
    }

    smiSetFlags(smiflags);
    smiSetErrorLevel(0);
}
