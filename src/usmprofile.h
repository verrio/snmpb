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

#ifndef USMPROFILE_H
#define USMPROFILE_H

#include "snmpb.h"
#include "ui_usmprofile.h"
#include <qdialog.h>
#include <qtreewidget.h>
#include <qlist.h>

class USMProfile: public QObject
{
    Q_OBJECT
    
public:
    USMProfile(Ui_USMProfile *uiup, QString *n = NULL);
    ~USMProfile();

    int IsPartOfUSMProfile(QTreeWidgetItem *item);
    int SelectUSMProfile(QTreeWidgetItem * item);
    QTreeWidgetItem *GetUserWidgetItem(void);

    void SetName(QString n);
    QString GetName(void);
    void SetSecName(void);

    void SetAuthProto(void);
    int GetAuthProto(void);
    void SetAuthPass(void);
    QString GetAuthPass(void);
    void SetPrivProto(void);
    int GetPrivProto(void);
    void SetPrivPass(void);
    QString GetPrivPass(void);
    void SetSecurity(int aprot, QString apass, int pprot, QString ppass);

protected:
    Ui_USMProfile *up;

    QTreeWidgetItem *user;

    QString name;
    int authproto;
    QString authpass;
    int privproto;
    QString privpass;
};

class USMProfileManager: public QObject
{
    Q_OBJECT

public:
    USMProfileManager(Snmpb *snmpb);
    void Execute(void);

    QStringList GetUsersList(void);

protected:
    QAction *addAct;
    QAction *deleteAct;

protected slots:
    void SetSecName(void);
    void SetAuthProto(void);
    void SetAuthPass(void);
    void SetPrivProto(void);
    void SetPrivPass(void);
    void SelectedUSMProfile( QTreeWidgetItem * item, QTreeWidgetItem * old);
    void USMSecNameChange(QTreeWidgetItem * item, int column);
    void Add(void);
    void Delete(void);
    void ContextMenu ( const QPoint & );

    // Convertion utilities
    int UiAuthToLibAuth(int prot);
    int LibAuthToUiAuth(int prot);
    int UiPrivToLibPriv(int prot);
    int LibPrivToUiPriv(int prot);

private:
    Snmpb *s;
    Ui_USMProfile up;
    QDialog upw;

    USMProfile* currentprofile;
    QList<USMProfile *> users;
};

#endif /* USMPROFILE_H */
