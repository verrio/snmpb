#include <qmessagebox.h> 
#include <qfileinfo.h>
#include <qtextstream.h>

#include "preferences.h"
#include "mibmodule.h"
// For DEFAULT_SMIPATH
#ifdef WIN32
#include "../libsmi/win/config.h"
#else
#include "../libsmi/config.h"
#endif

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
    connect( p.ModulePathsReset, 
             SIGNAL( clicked() ), this, SLOT( ModuleReset() ));
    connect( p.ModulePathsAdd, 
             SIGNAL( clicked() ), this, SLOT( ModuleAdd() ));
    connect( p.ModulePathsDelete, 
             SIGNAL( clicked() ), this, SLOT( ModuleDelete() ));

    // Load preferences from file
    p.HorizontalSplit->setCheckState(settings->value("horizontalsplit", false)
                                     .toBool() == true  ?Qt::Checked:Qt::Unchecked);
    trapport = settings->value("trapport", 161).toInt();

    char    *dir, *smipath;
    char    sep[2] = {PATH_SEPARATOR, 0};
    smipath = strdup(smiGetPath());
    mibpaths.clear();
    
    for (dir = strtok(smipath, sep); dir; dir = strtok(NULL, sep))
        mibpaths << dir; 
   
    free(smipath);

    pathschanged = false;

    p.PreferencesTree->setCurrentItem(p.PreferencesTree->topLevelItem(0));
}

void Preferences::Execute (void)
{
    if(pw.exec() == QDialog::Accepted)
    {
        // Save preferences
        settings->setValue("horizontalsplit", horizontalsplit);
        settings->setValue("trapport", trapport);

        if (pathschanged == true)
        {
            // Store modules in local list
            mibpaths.clear();
            QList<QListWidgetItem *> l = p.ModulePaths->findItems("*", Qt::MatchWildcard);
            for (int i = 0; i < l.size(); i++)
                mibpaths << l[i]->text(); 

            // Then create the paths in the file
            QFile paths(s->GetPathConfigFile());
            paths.remove();
            if (!paths.open(QIODevice::WriteOnly | QIODevice::Text))
            {
                QString err = QString("Unable to save paths in file %1 !\n")
                    .arg(paths.fileName());
                QMessageBox::critical ( NULL, "SnmpB error", err, 
                        QMessageBox::Ok, Qt::NoButton);
                return;
            }
            QTextStream out(&paths);
            out << "path ";
            for (int j = 0; j < mibpaths.size(); j++)
            {
                out << mibpaths[j];
                if (j == mibpaths.size()-1)
                    out << endl; 
                else
                    out << PATH_SEPARATOR;
            }
            paths.close();

            // and finally refresh the list of loaded MIBS ...
            s->MibModuleObj()->RefreshPathChange();
            s->TreeTabSelected();
        }

        pathschanged = false;
    }
    else
    {
        ModuleRefresh();
        pathschanged = false;
    }
}

void Preferences::ModuleReset(void)
{
    char    *dir, *smipath;
    char    sep[2] = {PATH_SEPARATOR, 0};
    smipath = strdup(DEFAULT_SMIPATH);

    p.ModulePaths->clear();

    for (dir = strtok(smipath, sep); dir; dir = strtok(NULL, sep))
    {
        QListWidgetItem *item = new QListWidgetItem(dir, p.ModulePaths);
        item->setFlags(item->flags() | Qt::ItemIsEditable);
        p.ModulePaths->addItem(item);
    }

    free(smipath);

    pathschanged = true;
}

void Preferences::ModuleAdd(void)
{
    QListWidgetItem *item = new QListWidgetItem("type new path here", p.ModulePaths);
    item->setFlags(item->flags() | Qt::ItemIsEditable);

    p.ModulePaths->addItem(item);
    p.ModulePaths->editItem(item);

    pathschanged = true;
}

void Preferences::ModuleDelete(void)
{
    QList<QListWidgetItem *> todel  = p.ModulePaths->selectedItems();
    QList<QListWidgetItem *> total = p.ModulePaths->findItems("*", Qt::MatchWildcard);

    // Protection
    if (todel.size() >= total.size())
    {
        QMessageBox::warning(NULL, "SnmpB warning", 
                             "Must have at least one defined path. Delete failed.", 
                             QMessageBox::Ok, Qt::NoButton);
        return;
    }

    for (int i = 0; i < todel.size(); i++)
        delete p.ModulePaths->takeItem(p.ModulePaths->row(todel[i]));

    pathschanged = true;
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

void Preferences::ModuleRefresh(void)
{
    p.ModulePaths->clear();
    p.ModulePaths->addItems(mibpaths);

    QList<QListWidgetItem *> l = p.ModulePaths->findItems("*", Qt::MatchWildcard);

    for (int i = 0; i < l.size(); i++)
        l[i]->setFlags(l[i]->flags() | Qt::ItemIsEditable);
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

        ModuleRefresh();
    }
    else
    if (item == traps)
    {
        p.PreferencesProps->setCurrentIndex(2);

        p.TrapPort->setValue(trapport);
    }
}

