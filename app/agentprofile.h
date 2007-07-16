#ifndef AGENTPROFILE_H
#define AGENTPROFILE_H

#include "snmpb.h"
#include "ui_agentprofile.h"
#include <qdialog.h>
#include <qtreewidget.h>
#include <qlist.h>
#include <qsettings.h>

class AgentProfile: public QObject
{
    Q_OBJECT
    
public:
    AgentProfile(Ui_AgentProfile *uiap, QString *n = NULL);
    ~AgentProfile();

    int IsPartOfAgentProfile(QTreeWidgetItem *item);
    int SelectAgentProfile(QTreeWidgetItem * item);
    void ProtocolV1Support(bool checked);
    void ProtocolV2Support(bool checked);
    void ProtocolV3Support(bool checked);

    void GetSupportedProtocol(bool *v1, bool *v2, bool *v3);
    void SetSupportedProtocol(bool v1, bool v2, bool v3);
    void ApplySupportedProtocol(void);

    void SetName(QString n);
    QString GetName(void);
    void SetProfileName(void);

    void SetAddress(void);
    QString GetAddress(void);
    void SetPort(void);
    QString GetPort(void);
    void SetTarget(QString a, QString p);

    void SetRetries(void);
    int GetRetries(void);
    void SetTimeout(void);
    int GetTimeout(void);
    void SetRetriesTimeout(int r, int t);
    
    void SetReadComm(void);
    QString GetReadComm(void);
    void SetWriteComm(void);
    QString GetWriteComm(void);
    void SetComms(QString r, QString w);
    
    void SetMaxRepetitions(void);
    int GetMaxRepetitions(void);
    void SetNonRepeaters(void);
    int GetNonRepeaters(void);
    void SetBulk(int mr, int nr);

    void SetSecName(void);
    QString GetSecName(void);
    void SetSecLevel(void);
    int GetSecLevel(void);
    void SetUser(QString u, int l);

    void SetContextName(void);
    QString GetContextName(void);
    void SetContextEngineID(void);
    QString GetContextEngineID(void);
    void SetContext(QString n, QString id);

protected:
    Ui_AgentProfile *ap;

    QTreeWidgetItem *general;
    QTreeWidgetItem *v1v2c;
    QTreeWidgetItem *bulk;
    QTreeWidgetItem *v3;

    bool v1support;
    bool v2support;
    bool v3support;
    QString name;
    QString address;
    QString port;
    int retries;
    int timeout;
    QString readcomm;
    QString writecomm;
    int maxrepetitions;
    int nonrepeaters;
    QString secname;
    int seclevel;
    QString contextname;
    QString contextengineid;
};

class AgentProfileManager: public QObject
{
    Q_OBJECT

public:
    AgentProfileManager(Snmpb *snmpb);
    void Execute(void);
    QStringList GetAgentsList(void);
    void SetSelectedAgent(QString a);
    AgentProfile *GetAgentProfile(QString a);

protected:
    QAction *addAct;
    QAction *deleteAct;

protected slots:
    void ProtocolV1Support(bool checked);
    void ProtocolV2Support(bool checked);
    void ProtocolV3Support(bool checked);
    void SetProfileName(void);
    void SetAddress(void);
    void SetPort(void);
    void SetRetries(void);
    void SetTimeout(void);
    void SetReadComm(void);
    void SetWriteComm(void);
    void SetMaxRepetitions(void);
    void SetNonRepeaters(void);
    void SetSecName(void);
    void SetSecLevel(void);
    void SetContextName(void);
    void SetContextEngineID(void);
    void SelectedAgentProfile( QTreeWidgetItem * item, QTreeWidgetItem * old);
    void AgentProfileNameChange(QTreeWidgetItem * item, int column);
    void Add(void);
    void Delete(void);
    void ContextMenu ( const QPoint & );

private:
    Snmpb *s;
    Ui_AgentProfile ap;
    QDialog apw;
    QSettings *settings;

    AgentProfile* currentprofile;
    QList<AgentProfile *> agents;
};

#endif /* AGENTPROFILE_H */
