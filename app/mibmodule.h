#ifndef MIBMODULE_H
#define MIBMODULE_H

#include "snmpb.h"
#include "mibview.h"
#include "smi.h"

#define PATH_SEPARATOR ';'

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
    MibModule(Snmpb *snmpb);
    void Refresh(void);
    void RefreshPathChange(void);
    void SendLogError(const QString& text) { emit LogError(text); }

public slots:
    void AddModule(void);
    void RemoveModule(void);
    void ShowModuleInfo(void);

signals:
    void ModuleProperties(const QString& text);
    void LogError(const QString& text);

private:
    void InitLib(int restart);
    void RebuildTotalList(void);
    void RebuildLoadedList(void);
    void RebuildUnloadedList(void);
    void SaveWantedModules(void);

private:
    Snmpb *s;

    QStringList Unloaded;
    QList<LoadedMibModule*> Loaded;
    QStringList Total;
    QStringList Wanted;
};

#endif /* MIBMODULE_H */
