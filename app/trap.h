#ifndef TRAP_H
#define TRAP_H

#include "snmpb.h"
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
    Trap(Snmpb *snmpb);
    TrapItem* Add(Oid &id, const QStringList &values, 
                  QString &community, QString &seclevel,
                  QString &ctxname, QString &ctxid, QString &msgid);
    
protected slots:
    void SelectedTrap( QTreeWidgetItem * item, QTreeWidgetItem * old);
    
signals:
    void TrapProperties(const QString& text);
    
private:
    Snmpb *s;
};

#endif /* TRAP_H */
