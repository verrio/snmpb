/*
    Copyright (C) 2004-2011 Martin Jolicoeur (snmpb1@gmail.com) 

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
#include <stdio.h>
#include <string.h>
#include <qfileinfo.h>
#include <qdir.h>
#include <qmessagebox.h> 
#include <qtextstream.h>

#include "mibmodule.h"
#include "agent.h"
#include "preferences.h"

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
    text += QString("<tr><td><b>Description:</b></td><td><font face=fixed color=blue>");
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
    text += QString("<tr><td><b>Contact Info:</b></td><td><font face=fixed>");
    text += Qt::convertFromPlainText (module->contactinfo);
    text += QString("</font></td></tr>");
             
    text += QString("</table>");
}

char* LoadedMibModule::GetMibLanguage(void)
{
    switch(module->language)
    {
    case SMI_LANGUAGE_SMIV1:
        return (char*)"SMIv1";
    case SMI_LANGUAGE_SMIV2:
        return (char*)"SMIv2";
    case SMI_LANGUAGE_SMING:
        return (char*)"SMIng";
    case SMI_LANGUAGE_SPPI:
        return (char*)"SPPI";
    default:
        return (char*)"Unknown";
    }
}

static MibModule *CurrentModuleObject = NULL;

MibModule::MibModule(Snmpb *snmpb)
{
    s = snmpb;
    Policy = MIBLOAD_DEFAULT;

    QStringList columns;
    columns << "Module" << "Required" << "Language" << "Path"; 
    s->MainUI()->LoadedModules->setHeaderLabels(columns);

    // Must be connected before call to InitLib ...
    connect(this, SIGNAL ( LogError(QString) ),
            s->MainUI()->LogOutput, SLOT ( append (QString) ));

    CurrentModuleObject = this;
    RebuildTotalList(0);
    InitLib(0);

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
    connect( this, SIGNAL( StopAgentTimer() ), 
             s->AgentObj(), SLOT( StopTimer() ));

    for(SmiModule *mod = smiGetFirstModule(); 
        mod; mod = smiGetNextModule(mod))
        Wanted.append(QFileInfo(mod->path).fileName());
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

// For sorting total module list based on name
bool compareModule(QStringList s1, QStringList s2)
{
    return s1[0] < s2[0];
}

char *mystrtok_r(char *s1, const char *s2, char **lasts)
{
    char *ret;
    if (s1 == NULL) s1 = *lasts;
    while(*s1 && strchr(s2, *s1)) ++s1;
    if(*s1 == '\0') return NULL;
    ret = s1;
    while(*s1 && !strchr(s2, *s1)) ++s1;
    if(*s1) *s1++ = '\0';
    *lasts = s1;
    return ret;
}

static void NormalErrorHdlr(char *path, int line, int severity, 
                            char *msg, char *tag)
{
    (void)line; (void)tag;

    if (severity <= 1)
        CurrentModuleObject->SendLogError(QString("ERROR(%1) loading %2: %3")
                                          .arg(severity).arg(path).arg(msg));
}

void MibModule::RebuildTotalList(int restart)
{
    char    *dir, *smipath, *str, *svptr = NULL;
    char    sep[2] = {PATH_SEPARATOR, 0};

    if (!restart) 
    { 
        smiInit(NULL);
        smiReadConfig(s->GetPathConfigFile().toLatin1().data(), NULL);
    }
    
    /* Enable error reporting */
    smiSetFlags(smiGetFlags() | SMI_FLAG_ERRORS | SMI_FLAG_NODESCR);
    smiSetErrorHandler(NormalErrorHdlr);
    smiSetErrorLevel(3);

    smipath = strdup(smiGetPath());
   
    Total.clear();
    
    for (dir = mystrtok_r(smipath, sep, &svptr); dir; 
         dir = mystrtok_r(NULL, sep, &svptr))
    {
        QDir d(dir, QString::null, QDir::Unsorted, 
               QDir::Files | QDir::Readable | QDir::NoSymLinks);
        QStringList list = d.entryList();
        if (list.isEmpty()) continue;
        QStringListIterator it( list );
        const QString *fi;
        
        while ( it.hasNext() ) {
            fi = &it.next();
            // Exclude -orig files
            // Accept only .smi, mib, .pib (and variants)
            QString ext = QFileInfo(fi->toLatin1()).suffix();
            if (!(((str = strstr(fi->toLatin1().data(), "-orig")) != NULL) 
                && (strlen(str) == 5)) &&
                ((ext.isEmpty() && 
                ((((str = strstr(fi->toLatin1().data(), "-MIB")) != NULL)
                && (strlen(str) == 4)) || 
                (((str = strstr(fi->toLatin1().data(), "-SMI")) != NULL)
                && (strlen(str) == 4)) || 
                (((str = strstr(fi->toLatin1().data(), "-TC")) != NULL)
                && (strlen(str) == 3)) || 
                (((str = strstr(fi->toLatin1().data(), "-TYPES")) != NULL)
                && (strlen(str) == 6)) || 
                (((str = strstr(fi->toLatin1().data(), "-PIB")) != NULL)
                && (strlen(str) == 4)))) || 
                (ext == "smi") || (ext == "mib") || (ext == "pib") || 
                (ext == "SMI") || (ext == "MIB") || (ext == "PIB")))
            {
                // Load each module and build a list of possible root oids
                // This is used for module auto-loading on mib walk
                QStringList module;

                // If a module has a fatal error, unload and ignore it
                ErrorWhileLoading = false;
                char *mod = smiLoadModule(fi->toLatin1());
                SmiModule *smiModule = mod?smiGetModule(mod):NULL;
                if (ErrorWhileLoading == true)
                {
                    QMessageBox::warning (s->MainUI()->MIBTree, "SnmpB error", 
                                          QString(
"Fatal error(s) found in MIB file %1. Check log tab.")
                                          .arg( fi->toLatin1().data()), 
                                          QMessageBox::Ok, Qt::NoButton);
// Crashes on win32 x64  if (smiModule) smiFreeModule(smiModule);
                    continue;
                }

                module += QFileInfo(fi->toLatin1()).fileName();

                if (smiModule)
                {
                    SmiNode *node = smiGetModuleIdentityNode(smiModule);
                    if (node)
                        module += smiRenderOID(node->oidlen, 
                                               node->oid, SMI_RENDER_NUMERIC);

                    for(node = smiGetFirstNode(smiModule, SMI_NODEKIND_NODE); 
                        node; node = smiGetNextNode(node, SMI_NODEKIND_NODE))
                    {
                        if (node->decl == SMI_DECL_VALUEASSIGNMENT)
                            module += smiRenderOID(node->oidlen, 
                                                   node->oid, SMI_RENDER_NUMERIC);
                    }
                }
                Total.append(module);
            }
        }
    }

    qSort(Total.begin(), Total.end(), compareModule);
    if (!restart)
        smiExit();
    free(smipath);
}

// Attempts to identify and load a mib module that resolves a specific oid
//
// Returns the mib module's filename if there is a match, otherwise
// returns an empty string
QString MibModule::LoadBestModule(QString oid)
{
    // If automatic loading is disabled, return
    if ((s->PreferencesObj()->GetAutomaticLoading() == 3) || 
        (Policy == MIBLOAD_NONE))
        return "";

    QString best_file = "";
    QString best_oid = "";

    // Loop though all mibs
    for (int k=0;k<Total.count();k++)
    {
        // Loop through all possible root oids for each mib
        for (int l=1;l<Total[k].count();l++)
        {
            // If we have a possible match better than the best so far ...
            if (((Total[k][l] != "") && oid.startsWith(Total[k][l]) && 
                (Total[k][l].size() > best_oid.size())))
            {
                // ...and it is not loaded ...
                for (int m=0;m<Unloaded.count();m++)
                {
                    if (Total[k][0] == Unloaded[m])
                    {
                        // ... note it as best match so far and continue.
                        best_file = Total[k][0];
                        best_oid = Total[k][l];
                    }
                }
                break;
            }
        }
    }

    // We have a match, try to load it
    if (best_file != "")
    {
        // If automatic loading prompt is enabled and load policy is not set to all
        if ((s->PreferencesObj()->GetAutomaticLoading() == 2) &&
            (Policy != MIBLOAD_ALL))
        {
            emit StopAgentTimer();
            int ret = QMessageBox::question (
                        s->MainUI()->MIBTree,
                        "SnmpB automatic MIB loading",
                        QString("Unknown OID %1\nAttempting to load resolving MIB module ?").arg(oid),
                        QMessageBox::Yes | QMessageBox::No | 
                        QMessageBox::YesToAll | QMessageBox::NoToAll,
                        QMessageBox::Yes
                      );

            switch (ret)
            {
                case QMessageBox::NoToAll:
                    Policy = MIBLOAD_NONE;
                    // fallthrough
                case QMessageBox::No:
                    return "";
                case QMessageBox::YesToAll:
                    Policy = MIBLOAD_ALL;
                    break;
                case QMessageBox::Yes:
                default:
                    break;
            }
        }

        // Load the module
        Wanted.append(best_file.toLatin1().data());
        Refresh();
        SaveWantedModules();
        s->TabSelected();
    }

    return best_file;
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
            required = (char*)"no";
        else
            required = (char*)"yes";
    
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
        current = Total[i][0];
        LoadedMibModule *lmodule = NULL;

        for(j = 0; j < Loaded.count(); j++)
        { 
            lmodule = Loaded[j];
            if (QFileInfo(lmodule->module->path).fileName() == current) break;
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
        Wanted.removeAll(QFileInfo(item_list[i]->text(3)).fileName());

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

    RebuildTotalList(1);
    InitLib(1);

    smiReadConfig(s->GetMibConfigFile().toLatin1().data(), NULL);

    Wanted.clear();
    for(SmiModule *mod = smiGetFirstModule(); 
        mod; mod = smiGetNextModule(mod))
        Wanted.append(QFileInfo(mod->path).fileName());

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
        smiSetErrorHandler(NormalErrorHdlr);
        smiSetErrorLevel(0);
        free(smipath);
    }
    else
    {
        smiInit(NULL);
        smiflags = smiGetFlags();
        smiflags |= SMI_FLAG_ERRORS;
        smiSetErrorHandler(NormalErrorHdlr);
        smiSetErrorLevel(0);
        // Read configuration files: order is important
        smiReadConfig(s->GetPathConfigFile().toLatin1().data(), NULL);
        smiReadConfig(s->GetMibConfigFile().toLatin1().data(), NULL);
    }

    smiSetFlags(smiflags);
}
