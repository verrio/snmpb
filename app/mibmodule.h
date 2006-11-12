#ifndef MIBMODULE_H
#define MIBMODULE_H

#include "ui_mainw.h"
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

class MibModule: public QObject
{
    Q_OBJECT
    
public:
    MibModule(QTextEdit *MI, QTreeWidget *AM, QTreeWidget *LM);
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
    QStringList Unloaded;
    QList<LoadedMibModule*> Loaded;
    QStringList Total;
    QStringList Wanted;
    
    QTextEdit *ModuleInfo;
    QTreeWidget *UnloadedM;
    QTreeWidget *LoadedM;
};

#endif /* MIBMODULE_H */
