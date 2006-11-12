#ifndef TRAP_H
#define TRAP_H

#include <q3listview.h>
//Added by qt3to4:
#include <Q3PtrList>

#include "ui_mainw.h"
#include "snmp_pp/snmp_pp.h"

class TrapItem : public QTreeWidgetItem
{
public:
    TrapItem(Oid &id, QTreeWidget* parent, QString no, QString date,
             QString time, QString timestamp,
             QString nottype, QString msgtype, QString version, 
             QString agtaddr, QString agtport,
             QString community, QString seclevel,
             QString ctxname, QString ctxid, QString msgid);

    void PrintProperties(QString& text);
    void PrintContent(QTreeWidget* TrapContent);
    void AddVarBind(Vb& vb);
    
private:
    Oid oid;
    QString _no;
    QString _date;
    QString _time;
    QString _timestamp;
    QString _nottype;
    QString _msgtype;
    QString _version;
    QString _agtaddr;
    QString _agtport;
    QString _community;
    QString _seclevel;
    QString _ctxname;
    QString _ctxid;
    QString _msgid;

    Q3PtrList<Vb> content;
};

class Trap: public QObject
{
    Q_OBJECT
    
public:
    Trap(QTreeWidget* TL, QTreeWidget* TC, QTextEdit* TI);
    TrapItem* Add(Oid &id, QString &no, QString &date, 
                  QString &time, QString &timestamp, 
                  QString &nottype, QString &msgtype, QString &version, 
                  QString &agtaddr, QString &agtport,
                  QString &community, QString &seclevel,
                  QString &ctxname, QString &ctxid, QString &msgid);
    
protected slots:
    void SelectedTrap( QTreeWidgetItem * item, QTreeWidgetItem * old);
    
signals:
    void TrapProperties(const QString& text);
    
private:
    QTreeWidget* TrapLog;
    QTreeWidget* TrapContent;
    QTextEdit* TrapInfo;
};

#endif /* TRAP_H */
