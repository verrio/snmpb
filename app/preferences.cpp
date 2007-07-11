#include "preferences.h"

Preferences::Preferences(Snmpb *snmpb)
{
    s = snmpb;

    settings = new QSettings(s->GetPrefsConfigFile(), QSettings::IniFormat, this);

    p.setupUi(&pw);

    // Set some properties for the Preferences TreeView
    p.PreferencesTree->header()->hide();
    p.PreferencesTree->setSortingEnabled( FALSE );
    p.PreferencesTree->header()->setSortIndicatorShown( FALSE );
    p.PreferencesTree->setLineWidth( 2 );
    p.PreferencesTree->setAllColumnsShowFocus( FALSE );
    p.PreferencesTree->setFrameShape(QFrame::WinPanel);
    p.PreferencesTree->setFrameShadow(QFrame::Plain);
    p.PreferencesTree->setRootIsDecorated( TRUE );

    mibtree = new QTreeWidgetItem(p.PreferencesTree);
    mibtree->setText(0, "MIB Tree");
    modules = new QTreeWidgetItem(p.PreferencesTree);
    modules->setText(0, "Modules");
    traps = new QTreeWidgetItem(p.PreferencesTree);
    traps->setText(0, "Traps");

    connect( p.PreferencesTree, 
             SIGNAL( currentItemChanged( QTreeWidgetItem *, QTreeWidgetItem * ) ),
             this, SLOT( SelectedPreferences( QTreeWidgetItem *, QTreeWidgetItem * ) ) );
    connect( p.HorizontalSplit, SIGNAL( toggled(bool) ),
             this, SLOT( SetHorizontalSplit(bool) ) );
    connect( p.TrapPort, SIGNAL( valueChanged( int ) ), 
             this, SLOT ( SetTrapPort() ) );

    // Load preferences from file
    p.HorizontalSplit->setCheckState(settings->value("horizontalsplit", false)
                                     .toBool() == true  ?Qt::Checked:Qt::Unchecked);
    trapport = settings->value("trapport", 161).toInt();

    p.PreferencesTree->setCurrentItem(p.PreferencesTree->topLevelItem(0));
}

void Preferences::Execute (void)
{
    if(pw.exec() == QDialog::Accepted)
    {
        // Save preferences
        settings->setValue("horizontalsplit", horizontalsplit);
        settings->setValue("trapport", trapport);
    }
}

void Preferences::SetHorizontalSplit(bool checked)
{
    horizontalsplit = checked;
    s->MainUI()->QuerySplitter->setOrientation(checked==true?Qt::Vertical:Qt::Horizontal);
}

void Preferences::SetTrapPort(void)
{
    trapport = p.TrapPort->value();
}

void Preferences::SelectedPreferences(QTreeWidgetItem * item, QTreeWidgetItem *)
{
    if (item == mibtree)
    {
        p.PreferencesProps->setCurrentIndex(0);

        p.HorizontalSplit->setCheckState(horizontalsplit==true?Qt::Checked:Qt::Unchecked);

    }
    else
    if (item == modules)
    {
        p.PreferencesProps->setCurrentIndex(1);
    }
    else
    if (item == traps)
    {
        p.PreferencesProps->setCurrentIndex(2);

        p.TrapPort->setValue(trapport);
    }
}

