#ifndef TRAP_H
#define TRAP_H

#include <q3listview.h>

#include "ui_mainw.h"
#include "snmp_pp/snmp_pp.h"

class TrapItem : public QTreeWidgetItem
{
public:
    TrapItem(Oid &id, QTreeWidget* parent, const QStringList &values,
             QString community, QString seclevel,
             QString ctxname, QString ctxid, QString msgid);

    void PrintProperties(QString& text);
    void PrintContent(QTreeWidget* TrapContent);
    void AddVarBind(Vb& vb);
    
private:
    Oid oid;
    QString _community;
    QString _seclevel;
    QString _ctxname;
    QString _ctxid;
    QString _msgid;

    QList<Vb*> content;
};

class Trap: public QObject
{
    Q_OBJECT
    
public:
    Trap(QTreeWidget* TL, QTreeWidget* TC, QTextEdit* TI);
    TrapItem* Add(Oid &id, const QStringList &values, 
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
