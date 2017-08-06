/**
 * Copyright (c) 2004-2017 Martin Jolicoeur and contributors
 *
 * This file is part of the snmpb project and is free software: you can
 * redistribute it and/or modify it under the terms of the GNU General
 * Public License as published by the Free Software Foundation, either
 * version 3 of the License, or (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <http://www.gnu.org/licenses/>.
 */

#include <qmessagebox.h>
#include <qfileinfo.h>
#include <qtextstream.h>

#include "preferences.h"
#include "mibmodule.h"

Preferences::Preferences(Snmpb *snmpb)
{
    s = snmpb;

    settings = new QSettings(s->GetPrefsConfigFile(), QSettings::IniFormat, this);

    enableipv4 = settings->value("enableipv4", true).toBool();
    trapport = settings->value("trapport", 162).toInt();
    enableipv6 = settings->value("enableipv6", true).toBool();
    trapport6 = settings->value("trapport6", 162).toInt();
}

void Preferences::Init(void)
{
    p = new Ui_Preferences();
    pw = new QDialog(s->MainUI()->TabW);
    p->setupUi(pw);

    // Set some properties for the Preferences TreeView
    p->PreferencesTree->header()->hide();
    p->PreferencesTree->setSortingEnabled( false );
    p->PreferencesTree->header()->setSortIndicatorShown( false );
    p->PreferencesTree->setLineWidth( 2 );
    p->PreferencesTree->setAllColumnsShowFocus( false );
    p->PreferencesTree->setFrameShape(QFrame::WinPanel);
    p->PreferencesTree->setFrameShadow(QFrame::Plain);
    p->PreferencesTree->setRootIsDecorated( true );

    transport = new QTreeWidgetItem(p->PreferencesTree);
    transport->setText(0, "Transport");
    mibtree = new QTreeWidgetItem(p->PreferencesTree);
    mibtree->setText(0, "MIB Tree");
    modules = new QTreeWidgetItem(p->PreferencesTree);
    modules->setText(0, "Modules");
    traps = new QTreeWidgetItem(p->PreferencesTree);
    traps->setText(0, "Traps");

    connect( p->PreferencesTree, 
             SIGNAL( currentItemChanged( QTreeWidgetItem *, QTreeWidgetItem * ) ),
             this, SLOT( SelectedPreferences( QTreeWidgetItem *, QTreeWidgetItem * ) ) );
    connect( p->HorizontalSplit, SIGNAL( toggled(bool) ),
             this, SLOT( SetHorizontalSplit(bool) ) );
    connect( p->TrapPort, SIGNAL( valueChanged( int ) ), 
             this, SLOT ( SetTrapPort() ) );
    connect( p->TrapPort6, SIGNAL( valueChanged( int ) ), 
             this, SLOT ( SetTrapPort6() ) );
    connect( p->EnableIPv4, SIGNAL( toggled(bool) ),
             this, SLOT( SetEnableIPv4(bool) ) );
    connect( p->EnableIPv6, SIGNAL( toggled(bool) ),
             this, SLOT( SetEnableIPv6(bool) ) );
    connect( p->ExpandTrapBinding, SIGNAL( toggled(bool) ),
             this, SLOT( SetExpandTrapBinding(bool) ) );
    connect( p->MibLoadingEnable, SIGNAL( toggled(bool) ),
             this, SLOT( SelectAutomaticLoading() ) );
    connect( p->MibLoadingEnablePrompt, SIGNAL( toggled(bool) ),
             this, SLOT( SelectAutomaticLoading() ) );
    connect( p->MibLoadingDisable, SIGNAL( toggled(bool) ),
             this, SLOT( SelectAutomaticLoading() ) );
    connect( p->ShowAgentName, SIGNAL( toggled(bool) ),
             this, SLOT( SetShowAgentName(bool) ) );
    connect( p->ModulePathsReset, 
             SIGNAL( clicked() ), this, SLOT( ModuleReset() ));
    connect( p->ModulePathsAdd, 
             SIGNAL( clicked() ), this, SLOT( ModuleAdd() ));
    connect( p->ModulePathsDelete, 
             SIGNAL( clicked() ), this, SLOT( ModuleDelete() ));

    // Load preferences from file
    horizontalsplit = settings->value("horizontalsplit", false).toBool();
    p->HorizontalSplit->setCheckState((horizontalsplit == true)?
                                     Qt::Checked:Qt::Unchecked);

    p->EnableIPv4->setCheckState((enableipv4 == true)?
                                 Qt::Checked:Qt::Unchecked);
    p->EnableIPv6->setCheckState((enableipv6 == true)?
                                 Qt::Checked:Qt::Unchecked);

    expandtrapbinding = settings->value("expandtrapbinding", true).toBool();
    p->ExpandTrapBinding->setCheckState((expandtrapbinding == true)?
                                       Qt::Checked:Qt::Unchecked);

    showagentname = settings->value("showagentname", false).toBool();
    p->ShowAgentName->setCheckState((showagentname == true)?
                                    Qt::Checked:Qt::Unchecked);

    automaticloading = settings->value("automaticloading", 2).toInt();
    if (automaticloading == 1) p->MibLoadingEnable->setChecked(true);
    else if (automaticloading == 2) p->MibLoadingEnablePrompt->setChecked(true);
    else if (automaticloading == 3) p->MibLoadingDisable->setChecked(true);

    char    *dir, *smipath;
    char    sep[2] = {PATH_SEPARATOR, 0};
    smipath = strdup(smiGetPath());
    mibpaths.clear();
    
    for (dir = strtok(smipath, sep); dir; dir = strtok(NULL, sep))
        mibpaths << dir; 
   
    free(smipath);

    pathschanged = false;

    p->PreferencesTree->setCurrentItem(p->PreferencesTree->topLevelItem(0));

    curprofile = settings->value("curprofile", "localhost").toString();
    curproto = settings->value("curproto", 0).toInt();
}

void Preferences::Execute (void)
{
    if(pw->exec() == QDialog::Accepted)
    {
        // Warn if trap port or transport changed ...
        if((trapport != settings->value("trapport", 162).toInt()) ||
           (trapport6 != settings->value("trapport6", 162).toInt()) ||
           (enableipv4 != settings->value("enableipv4", true).toBool()) ||
           (enableipv6 != settings->value("enableipv6", true).toBool()))
            QMessageBox::information(NULL, "snmpb transport protocol or trap port changed", 
                                     "Please restart snmpb for the change to take effect.", 
                                     QMessageBox::Ok, Qt::NoButton);

        // Save preferences
        settings->setValue("horizontalsplit", horizontalsplit);
        settings->setValue("trapport", trapport);
        settings->setValue("trapport6", trapport6);
        settings->setValue("enableipv4", enableipv4);
        settings->setValue("enableipv6", enableipv6);
        settings->setValue("expandtrapbinding", expandtrapbinding);
        settings->setValue("showagentname", showagentname);
        settings->setValue("automaticloading", automaticloading);

        if (pathschanged == true)
        {
            // Store modules in local list
            mibpaths.clear();
            QList<QListWidgetItem *> l = p->ModulePaths->findItems("*", Qt::MatchWildcard);
            for (int i = 0; i < l.size(); i++)
                mibpaths << l[i]->text(); 

            // Then create the paths in the file
            QFile paths(s->GetPathConfigFile());
            paths.remove();
            if (!paths.open(QIODevice::WriteOnly | QIODevice::Text))
            {
                QString err = QString("Unable to save paths in file %1 !\n")
                    .arg(paths.fileName());
                QMessageBox::critical ( NULL, "snmpb error", err, 
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
            s->TabSelected();
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

    p->ModulePaths->clear();

    for (dir = strtok(smipath, sep); dir; dir = strtok(NULL, sep))
    {
        QListWidgetItem *item = new QListWidgetItem(dir, p->ModulePaths);
        item->setFlags(item->flags() | Qt::ItemIsEditable);
        p->ModulePaths->addItem(item);
    }

    free(smipath);

    pathschanged = true;
}

void Preferences::ModuleAdd(void)
{
    QListWidgetItem *item = new QListWidgetItem("type new path here", p->ModulePaths);
    item->setFlags(item->flags() | Qt::ItemIsEditable);

    p->ModulePaths->addItem(item);
    p->ModulePaths->editItem(item);

    pathschanged = true;
}

void Preferences::ModuleDelete(void)
{
    QList<QListWidgetItem *> todel  = p->ModulePaths->selectedItems();
    QList<QListWidgetItem *> total = p->ModulePaths->findItems("*", Qt::MatchWildcard);

    // Protection
    if (todel.size() >= total.size())
    {
        QMessageBox::warning(NULL, "snmpb warning", 
                             "Must have at least one defined path. Delete failed.", 
                             QMessageBox::Ok, Qt::NoButton);
        return;
    }

    for (int i = 0; i < todel.size(); i++)
        delete p->ModulePaths->takeItem(p->ModulePaths->row(todel[i]));

    pathschanged = true;
}

void Preferences::SetHorizontalSplit(bool checked)
{
    horizontalsplit = checked;
    s->MainUI()->QuerySplitter->setOrientation(checked==true?Qt::Vertical:Qt::Horizontal);
}

void Preferences::SetTrapPort(void)
{
    trapport = p->TrapPort->value();
}

void Preferences::SetTrapPort6(void)
{
    trapport6 = p->TrapPort6->value();
}

void Preferences::SetEnableIPv4(bool checked)
{
    if ((checked == false) && (enableipv6 == false))
    {
        QMessageBox::critical(NULL, "snmpb error", 
                              "Must enable at least one transport protocol.", 
                              QMessageBox::Ok, Qt::NoButton);
        p->EnableIPv4->setCheckState(Qt::Checked);
        return;
    }

    enableipv4 = checked;
}

void Preferences::SetEnableIPv6(bool checked)
{
    if ((checked == false) && (enableipv4 == false))
    {
        QMessageBox::critical(NULL, "snmpb error", 
                              "Must enable at least one transport protocol.", 
                              QMessageBox::Ok, Qt::NoButton);
        p->EnableIPv6->setCheckState(Qt::Checked);
        return;
    }

    enableipv6 = checked;
}

void Preferences::SetExpandTrapBinding(bool checked)
{
    expandtrapbinding = checked;
}

void Preferences::SetShowAgentName(bool checked)
{
    showagentname = checked;
}

void Preferences::SelectAutomaticLoading(void)
{
    if (p->MibLoadingEnable->isChecked()) automaticloading = 1;
    else if (p->MibLoadingEnablePrompt->isChecked()) automaticloading = 2;
    else if (p->MibLoadingDisable->isChecked()) automaticloading = 3;
}

bool Preferences::GetExpandTrapBinding(void)
{
    return expandtrapbinding;
}

bool Preferences::GetShowAgentName(void)
{
    return showagentname;
}

bool Preferences::GetEnableIPv4(void)
{
    return enableipv4;
}

bool Preferences::GetEnableIPv6(void)
{
    return enableipv6;
}

int Preferences::GetAutomaticLoading(void)
{
    return automaticloading;
}

int Preferences::GetTrapPort(void)
{
    return trapport;
}

int Preferences::GetTrapPort6(void)
{
    return trapport6;
}

void Preferences::SaveCurrentProfile(QString &name, int proto)
{
    curprofile = name;
    curproto = proto;
    settings->setValue("curprofile", curprofile);
    settings->setValue("curproto", curproto);
}

int Preferences::GetCurrentProfile(QString &name)
{
    name = curprofile;
    return curproto;
}

void Preferences::ModuleRefresh(void)
{
    p->ModulePaths->clear();
    p->ModulePaths->addItems(mibpaths);

    QList<QListWidgetItem *> l = p->ModulePaths->findItems("*", Qt::MatchWildcard);

    for (int i = 0; i < l.size(); i++)
        l[i]->setFlags(l[i]->flags() | Qt::ItemIsEditable);
}

void Preferences::SelectedPreferences(QTreeWidgetItem * item, QTreeWidgetItem *)
{
    if (item == transport)
    {
        p->PreferencesProps->setCurrentIndex(0);

        p->EnableIPv4->setCheckState(enableipv4==true?Qt::Checked:Qt::Unchecked);
        p->EnableIPv6->setCheckState(enableipv6==true?Qt::Checked:Qt::Unchecked);
    }
    else
    if (item == mibtree)
    {
        p->PreferencesProps->setCurrentIndex(1);

        p->HorizontalSplit->setCheckState(horizontalsplit==true?Qt::Checked:Qt::Unchecked);
        if (automaticloading == 1) p->MibLoadingEnable->setChecked(true);
        else if (automaticloading == 2) p->MibLoadingEnablePrompt->setChecked(true);
        else if (automaticloading == 3) p->MibLoadingDisable->setChecked(true);
    }
    else
    if (item == modules)
    {
        p->PreferencesProps->setCurrentIndex(2);

        ModuleRefresh();
    }
    else
    if (item == traps)
    {
        p->PreferencesProps->setCurrentIndex(3);

        p->TrapPort->setValue(trapport);
        p->TrapPort6->setValue(trapport6);
        p->ExpandTrapBinding->setCheckState(expandtrapbinding==true?Qt::Checked:Qt::Unchecked);
        p->ShowAgentName->setCheckState(showagentname==true?Qt::Checked:Qt::Unchecked);
    }
}

