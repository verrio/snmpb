#ifndef MIBMODULE_H
#define MIBMODULE_H

#include "mainw.h"
#include "smi.h"

class MibModule: public QObject
{
    Q_OBJECT
    
public:
    MibModule(QListView *AM, QListView *LM, QPushButton *AB, QPushButton *RB);
    void Load(void);

public slots:
    void AddModule(void);
    void RemoveModule(void);

private:
    void RebuildTotalList(void);
    void RebuildLoadedList(void);
    void RebuildUnloadedList(void);
    
    void RefreshLoadedView(void);
    void RefreshUnloadedView(void);
    
private:
    QStrList Unloaded;
    QStrList Loaded;
    QStrList Total;
    
    QListView *AvailM;
    QListView *LoadedM;
    QPushButton *AddB;
    QPushButton *RemoveB;
};

#endif /* MIBMODULE_H */
