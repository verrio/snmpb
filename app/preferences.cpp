#include "preferences.h"

Preferences::Preferences(Snmpb *snmpb)
{
    s = snmpb;

    mibtree = new QTreeWidgetItem(s->PreferencesUI()->PreferencesTree);
    mibtree->setText(0, "MIB Tree");
    modules = new QTreeWidgetItem(s->PreferencesUI()->PreferencesTree);
    modules->setText(0, "Modules");
    traps = new QTreeWidgetItem(s->PreferencesUI()->PreferencesTree);
    traps->setText(0, "Traps");

    connect( s->PreferencesUI()->PreferencesTree, 
             SIGNAL( currentItemChanged( QTreeWidgetItem *, QTreeWidgetItem * ) ),
             this, SLOT( SelectedPreferences( QTreeWidgetItem *, QTreeWidgetItem * ) ) );
    connect( s->PreferencesUI()->HorizontalSplit, SIGNAL( toggled(bool) ),
             this, SLOT( HorizontalSplit(bool) ) );
}

void Preferences::HorizontalSplit(bool checked)
{
    s->MainUI()->QuerySplitter->setOrientation(checked==FALSE?Qt::Horizontal:Qt::Vertical);
}

void Preferences::SelectedPreferences(QTreeWidgetItem * item, QTreeWidgetItem *)
{
    if (item == mibtree)
    {
        s->PreferencesUI()->PreferencesProps->setCurrentIndex(0);
    }
    else
    if (item == modules)
    {
        s->PreferencesUI()->PreferencesProps->setCurrentIndex(1);
    }
    else
    if (item == traps)
    {
        s->PreferencesUI()->PreferencesProps->setCurrentIndex(2);
    }
}

