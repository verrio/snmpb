#include <stdio.h>
#include <string.h>
#include <qfileinfo.h>
#include <qdir.h>
#include <qlistview.h>
#include <qptrlist.h>

#include "mibmodule.h"

MibModule::MibModule(MibView* MT, QListView *AM, QListView *LM, 
		          QPushButton *AB, QPushButton *RB)
{
    MibTree = MT;
    AvailM = AM;
    LoadedM = LM;
    AddB = AB;
    RemoveB = RB;
    
    LoadedM->addColumn("Required");
    LoadedM->addColumn("Language");
    LoadedM->addColumn("Path");
    
    InitLib();
    RebuildTotalList();
    
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
    char    *dir, *smipath, *str;
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
	 // Exclude -orig files
	if (!(((str = strstr(fi->fileName(), "-orig")) != NULL) && (strlen(str) == 5)))
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
    LoadedMibModule *lmodule;
    char * required = NULL;
    
    Loaded.clear();
    LoadedM->clear();

    mod = smiGetFirstModule();
    
    while (mod)
    {
        lmodule = new LoadedMibModule(mod);
        Loaded.append(lmodule);
        
        if (Wanted.find(lmodule->name) == -1)
	required = "yes";
        else
	required = "no";
	    
        new QListViewItem(LoadedM, 
			lmodule->name.latin1(), 
			required, 
			lmodule->GetMibLanguage(),
			lmodule->module->path);
        i++;
        mod = smiGetNextModule(mod);
    }
    
    Loaded.sort();
}

void MibModule::RebuildUnloadedList(void)
{
    const char * current = Total.first();
    
    Unloaded.clear();
    AvailM->clear();
    
    if (current != 0) {
        do {
	LoadedMibModule *lmodule = Loaded.first();
	if ( lmodule != NULL) {
	    do {
	        if (lmodule->name == current) break;
                } while ( (lmodule = Loaded.next()) != 0);	
	}
	if (!lmodule) {
	    Unloaded.append(current);
                new QListViewItem(AvailM, current);
	}
        } while ( (current = Total.next()) != 0);
    }
}

void MibModule::AddModule(void)
{
    QListViewItem *item;//, *nextitem  = NULL;
//    char buf[200];
    
    if ((item = AvailM->selectedItem()) != 0)
    { 
        // Save string of next item to restore selection ...
//        nextitem = item->itemBelow() ? item->itemBelow():item->itemAbove();
//        if (nextitem)
//            strcpy(buf, nextitem->text(0).latin1());
	
        Wanted.append(item->text(0).latin1());
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
    QListViewItem *item;
    
    if ((item = LoadedM->selectedItem()) != 0)
    {
        Wanted.remove(item->text(0).latin1());
        Refresh();
    }
}

void MibModule::Refresh(void)
{
    MibTree->clear();
    smiExit();
    InitLib();
    MibTree->Load(Wanted);
    RebuildLoadedList();
    RebuildUnloadedList();
}

void MibModule::InitLib(void)
{
    int smiflags;
    
    smiInit(NULL);
    smiflags = smiGetFlags();
    smiflags |= SMI_FLAG_ERRORS;
    smiSetFlags(smiflags);
    smiSetErrorLevel(0);
}
