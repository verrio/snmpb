#ifndef PREFERENCES_H
#define PREFERENCES_H

#include "snmpb.h"
#include "ui_preferences.h"
#include <qdialog.h>
#include <qtreewidget.h>
#include <qsettings.h>


class Preferences: public QObject
{
    Q_OBJECT
    
public:
    Preferences(Snmpb *snmpb);
    void Execute(void);

protected slots:
    void SelectedPreferences( QTreeWidgetItem * item, QTreeWidgetItem * old);
    void SetHorizontalSplit(bool checked);
    void SetTrapPort(void);

private:
    Snmpb *s;
    Ui_Preferences p;
    QDialog pw;
    QSettings *settings;

    QTreeWidgetItem *mibtree;
    QTreeWidgetItem *modules;
    QTreeWidgetItem *traps;

    bool horizontalsplit;
    int trapport;
};

#endif /* PREFERENCES_H */
