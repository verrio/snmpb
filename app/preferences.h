#ifndef PREFERENCES_H
#define PREFERENCES_H

#include "snmpb.h"
#include "qdialog.h"
#include "qtreewidget.h"

class Preferences: public QObject
{
    Q_OBJECT
    
public:
    Preferences(Snmpb *snmpb);

protected slots:
    void SelectedPreferences( QTreeWidgetItem * item, QTreeWidgetItem * old);
    void HorizontalSplit(bool checked);

private:
    Snmpb *s;

    QTreeWidgetItem *mibtree;
    QTreeWidgetItem *modules;
    QTreeWidgetItem *traps;
};

#endif /* PREFERENCES_H */
