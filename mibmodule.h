#ifndef MIBMODULE_H
#define MIBMODULE_H

#include "mainw.h"
#include "mibview.h"
#include "smi.h"

class LoadedMibModule
{
public:
    LoadedMibModule(SmiModule* mod)
    {
	name = mod->name;
	module = mod;
    }
    
    char* GetMibLanguage(void)
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

    QString name;
    SmiModule *module;
};

class LoadedModule: public QPtrList<LoadedMibModule>
{
public:
    LoadedModule() { setAutoDelete(TRUE); }
    
protected:
    int compareItems ( QPtrCollection::Item item1, QPtrCollection::Item item2 )
    {
	return (strcmp(((LoadedMibModule*)item1)->name.latin1(), 
		        ((LoadedMibModule*)item2)->name.latin1()));
    }
};

class MibModule: public QObject
{
    Q_OBJECT
    
public:
    MibModule(MibView* MT, QListView *AM, QListView *LM, 
	         QPushButton *AB, QPushButton *RB);
    void Refresh(void);

public slots:
    void AddModule(void);
    void RemoveModule(void);

private:
    void InitLib(void);
    void RebuildTotalList(void);
    void RebuildLoadedList(void);
    void RebuildUnloadedList(void);
    
private:
    QStrList Unloaded;
    LoadedModule Loaded;
    QStrList Total;
    QStrList Wanted;
    
    MibView *MibTree;
    QListView *AvailM;
    QListView *LoadedM;
    QPushButton *AddB;
    QPushButton *RemoveB;
};

#endif /* MIBMODULE_H */
