#ifndef TRAP_H
#define TRAP_H

#include <qlistview.h>

#include "mainw.h"
#include "snmp_pp/snmp_pp.h"

class TrapItem : public QListViewItem
{
public:
    TrapItem(Oid &id, QListView* parent, 
             QString no, QString date = QString::null,
             QString time = QString::null, QString timestamp = QString::null,
             QString nottype = QString::null, QString msgtype = QString::null,
             QString version = QString::null, QString agtaddr = QString::null);

    void PrintProperties(QString& text);  
    void PrintContent(QListView* TrapContent);  
    
private:
    Oid oid;
};

class Trap: public QObject
{
    Q_OBJECT
    
public:
    Trap(QListView* TL, QListView* TC, QTextEdit* TI);
    void Add(QString &no, QString &date, QString &time, QString &timestamp, 
             QString &nottype, QString &msgtype, QString &version, 
             QString &agtaddr, QString &agtport, Oid &id);
    
protected slots:
    void SelectedTrap( QListViewItem * item);
    
signals:
    void TrapProperties(const QString& text);
    
private:
    QListView* TrapLog;
    QListView* TrapContent;
    QTextEdit* TrapInfo;
};

#endif /* TRAP_H */
