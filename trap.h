#ifndef TRAP_H
#define TRAP_H

#include <qlistview.h>

#include "mainw.h"
#include "snmp_pp/snmp_pp.h"

class TrapItem : public QListViewItem
{
public:
    TrapItem(Oid &id, QListView* parent, QString no, QString date,
             QString time, QString timestamp,
             QString nottype, QString msgtype, QString version, 
             QString agtaddr, QString agtport,
             QString community, QString seclevel,
             QString ctxname, QString ctxid, QString msgid);

    void PrintProperties(QString& text);
    void PrintContent(QListView* TrapContent);
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

    QPtrList<Vb> content;
};

class Trap: public QObject
{
    Q_OBJECT
    
public:
    Trap(QListView* TL, QListView* TC, QTextEdit* TI);
    TrapItem* Add(Oid &id, QString &no, QString &date, 
                  QString &time, QString &timestamp, 
                  QString &nottype, QString &msgtype, QString &version, 
                  QString &agtaddr, QString &agtport,
                  QString &community, QString &seclevel,
                  QString &ctxname, QString &ctxid, QString &msgid);
    
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
