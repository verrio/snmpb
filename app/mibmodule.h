#ifndef MIBMODULE_H
#define MIBMODULE_H

#include "ui_mainw.h"
#include "mibview.h"
#include "smi.h"
//Added by qt3to4:
#include <Q3StrList>
#include <Q3PtrList>

class LoadedMibModule
{
public:
    LoadedMibModule(SmiModule* mod);
    
    void PrintProperties(QString& text);   
    char* GetMibLanguage(void);

    QString name;
    SmiModule *module;
};

class LoadedModule: public Q3PtrList<LoadedMibModule>
{
public:
    LoadedModule() { setAutoDelete(TRUE); }
    
protected:
    int compareItems ( Q3PtrCollection::Item item1, Q3PtrCollection::Item item2 )
    {
	return (strcmp(((LoadedMibModule*)item1)->name.latin1(), 
		        ((LoadedMibModule*)item2)->name.latin1()));
    }
};

class MibModule: public QObject
{
    Q_OBJECT
    
public:
    MibModule(QTextEdit *MI, Q3ListView *AM, Q3ListView *LM);
    void Refresh(void);

public slots:
    void AddModule(void);
    void RemoveModule(void);
    void ShowModuleInfo(void);

signals:
    void ModuleProperties(const QString& text);

private:
    void InitLib(int restart);
    void RebuildTotalList(void);
    void RebuildLoadedList(void);
    void RebuildUnloadedList(void);
    
private:
    Q3StrList Unloaded;
    LoadedModule Loaded;
    Q3StrList Total;
    Q3StrList Wanted;
    
    QTextEdit *ModuleInfo;
    Q3ListView *UnloadedM;
    Q3ListView *LoadedM;
};

#endif /* MIBMODULE_H */
