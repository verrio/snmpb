#ifndef TRAP_H
#define TRAP_H

#include <q3listview.h>
//Added by qt3to4:
#include <Q3PtrList>

#include "ui_mainw.h"
#include "snmp_pp/snmp_pp.h"

class TrapItem : public Q3ListViewItem
{
public:
    TrapItem(Oid &id, Q3ListView* parent, QString no, QString date,
             QString time, QString timestamp,
             QString nottype, QString msgtype, QString version, 
             QString agtaddr, QString agtport,
             QString community, QString seclevel,
             QString ctxname, QString ctxid, QString msgid);

    void PrintProperties(QString& text);
    void PrintContent(Q3ListView* TrapContent);
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
    Trap(Q3ListView* TL, Q3ListView* TC, Q3TextEdit* TI);
    TrapItem* Add(Oid &id, QString &no, QString &date, 
                  QString &time, QString &timestamp, 
                  QString &nottype, QString &msgtype, QString &version, 
                  QString &agtaddr, QString &agtport,
                  QString &community, QString &seclevel,
                  QString &ctxname, QString &ctxid, QString &msgid);
    
protected slots:
    void SelectedTrap( Q3ListViewItem * item);
    
signals:
    void TrapProperties(const QString& text);
    
private:
    Q3ListView* TrapLog;
    Q3ListView* TrapContent;
    Q3TextEdit* TrapInfo;
};

#endif /* TRAP_H */
