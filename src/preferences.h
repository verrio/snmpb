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

#ifndef PREFERENCES_H
#define PREFERENCES_H

#include <qdialog.h>
#include <qtreewidget.h>
#include <qsettings.h>

#include "snmpb.h"
#include "ui_preferences.h"

#define DEFAULT_SMIPATH "/usr/share/snmp/mibs/"

class Preferences: public QObject
{
    Q_OBJECT
    
public:
    Preferences(Snmpb *snmpb);
    void Init(void);
    void Execute(void);
    int GetTrapPort(void);
    int GetTrapPort6(void);
    bool GetEnableIPv4(void);
    bool GetEnableIPv6(void);
    bool GetExpandTrapBinding(void);
    bool GetShowAgentName(void);
    int GetAutomaticLoading(void);
    void SaveCurrentProfile(QString &name, int proto);
    int GetCurrentProfile(QString &name);

public slots:
    void SetEnableIPv4(bool checked);
    void SetEnableIPv6(bool checked);

protected slots:
    void SelectedPreferences( QTreeWidgetItem * item, QTreeWidgetItem * old);
    void SetHorizontalSplit(bool checked);
    void SetTrapPort(void);
    void SetTrapPort6(void);
    void SetExpandTrapBinding(bool checked);
    void SetShowAgentName(bool checked);
    void SelectAutomaticLoading(void);
    void ModuleReset(void);
    void ModuleAdd(void);
    void ModuleDelete(void);

protected:
    void ModuleRefresh(void);

private:
    Snmpb *s;
    Ui_Preferences *p;
    QDialog *pw;
    QSettings *settings;

    QTreeWidgetItem *transport;
    QTreeWidgetItem *mibtree;
    QTreeWidgetItem *modules;
    QTreeWidgetItem *traps;

    bool horizontalsplit;
    int trapport;
    int trapport6;
    bool enableipv4;
    bool enableipv6;
    bool expandtrapbinding;
    bool showagentname;
    int automaticloading;
    QString curprofile;
    int curproto;
    QStringList mibpaths;
    bool pathschanged;
};

#endif /* PREFERENCES_H */
