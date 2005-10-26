#ifndef MIBMODULE_H
#define MIBMODULE_H

#include "mainw.h"
#include "mibview.h"
#include "smi.h"

class LoadedMibModule
{
public:
    LoadedMibModule(SmiModule* mod);
    
    void PrintProperties(QString& text);   
    char* GetMibLanguage(void);

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
    MibModule(QTextEdit *MI, QListView *AM, QListView *LM);
    void Refresh(void);

public slots:
    void AddModule(void);
    void RemoveModule(void);
    void ShowModuleInfo(void);

signals:
    void ModuleProperties(const QString& text);

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
    
    QTextEdit *ModuleInfo;
    QListView *UnloadedM;
    QListView *LoadedM;
};

#endif /* MIBMODULE_H */
