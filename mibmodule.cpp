#include <stdio.h>
#include <string.h>
#include <qfileinfo.h>
#include <qdir.h>
#include <qlistview.h>

#include "mibmodule.h"

MibModule::MibModule()
{
}

void MibModule::Load(MainW *mw)
{
    SmiModule *mod;
    int i = 0;
    
    mod = smiGetFirstModule();
    do    
    {
        new QListViewItem(mw->LoadedModules, mod->name);
        i++;
        mod = smiGetNextModule(mod);
    }
    while (mod);
    
    printf("There are %d loaded modules\n", i);
    
    char    *dir, *smipath;
    char    sep[2] = {':', 0};

    smipath = strdup(smiGetPath());
    
    for (dir = strtok(smipath, sep); dir; dir = strtok(NULL, sep)) {
        QDir d(dir, QString::null, QDir::Unsorted, 
	       QDir::Files | QDir::Readable | QDir::NoSymLinks);
        const QFileInfoList *list = d.entryInfoList();
        QFileInfoListIterator it( *list );
        QFileInfo *fi;
	
        while ( (fi = it.current()) != 0 ) {
	new QListViewItem(mw->AvailableModules, fi->fileName());
            ++it;
        }    
    }
    
    free(smipath);	     
}

