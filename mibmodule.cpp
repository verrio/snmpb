#include <stdio.h>
#include <string.h>
#include <qfileinfo.h>
#include <qdir.h>
#include <qlistview.h>
#include <qptrlist.h>

#include "mibmodule.h"

MibModule::MibModule(QListView *AM, QListView *LM, 
		          QPushButton *AB, QPushButton *RB)
{
    AvailM = AM;
    LoadedM = LM;
    AddB = AB;
    RemoveB = RB;
    
    // Connect some signals
    connect( (QObject*)AddB, SIGNAL(pressed()),
                 this, SLOT(  AddModule() ) );
    connect( (QObject*)RemoveB, SIGNAL(pressed()),
                 this, SLOT( RemoveModule() ) );
    connect( AvailM, SIGNAL(doubleClicked ( QListViewItem *, const QPoint &, int )),
                 this, SLOT( AddModule() ) );
    connect( LoadedM, SIGNAL(doubleClicked ( QListViewItem *, const QPoint &, int )),
                 this, SLOT( RemoveModule() ) );
}

void MibModule::RebuildTotalList(void)
{
    char    *dir, *smipath;
    char    sep[2] = {':', 0};

    smipath = strdup(smiGetPath());
    
    Total.clear();
    
    for (dir = strtok(smipath, sep); dir; dir = strtok(NULL, sep)) {
        QDir d(dir, QString::null, QDir::Unsorted, 
	       QDir::Files | QDir::Readable | QDir::NoSymLinks);
        const QFileInfoList *list = d.entryInfoList();
        QFileInfoListIterator it( *list );
        QFileInfo *fi;
	
        while ( (fi = it.current()) != 0 ) {
	Total.append(fi->fileName());
            ++it;
        }    
    }
 
    Total.sort();
    
    free(smipath);
}

void MibModule::RebuildLoadedList(void)
{
    SmiModule *mod;
    int i = 0;
    
    Loaded.clear();

    mod = smiGetFirstModule();
    do
    {
        Loaded.append(mod->name);
        i++;
        mod = smiGetNextModule(mod);
    }
    while (mod);
    
    Loaded.sort();
}

void MibModule::RebuildUnloadedList(void)
{
    const char * current = Total.first();
    
    Unloaded.clear();
    
    if (current != 0) {
        do {
            if (Loaded.find(current) == -1)
	    Unloaded.append(current);
        } while ( (current = Total.next()) != 0);
    }
}

void MibModule::RefreshUnloadedView(void)
{
    const char * current = NULL;
    
    AvailM->clear();
    if ( (current = Unloaded.first()) != 0) {
        do {
            new QListViewItem(AvailM, current);
        } while ( (current = Unloaded.next()) != 0);
    }    
}

void MibModule::RefreshLoadedView(void)
{
    const char * current = NULL;
    
    LoadedM->clear();
    if ( (current = Loaded.first()) != 0) {
        do {
            new QListViewItem(LoadedM, current);
	} while ( (current = Loaded.next()) != 0);
    }
}

void MibModule::AddModule(void)
{
    QListViewItem *item;
    
    if ((item = AvailM->selectedItem()) != 0)
    {
        smiLoadModule(item->text(0).latin1());
        
        // Refresh everybody
        RebuildLoadedList();
        RebuildUnloadedList();
        RefreshLoadedView();
        RefreshUnloadedView();
    }
}

void MibModule::RemoveModule(void)
{
    QListViewItem *item;
    
    if ((item = LoadedM->selectedItem()) != 0)
    {
        printf("Removing %s\n", item->text(0).latin1());
    }
}

void MibModule::Load(void)
{
    RebuildTotalList();
    
    RebuildLoadedList();
    RebuildUnloadedList();    
    RefreshLoadedView();
    RefreshUnloadedView();
}
