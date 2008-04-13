#include "agentprofile.h"
#include "usmprofile.h"
#include <qmessagebox.h>

AgentProfileManager::AgentProfileManager(Snmpb *snmpb)
{
    s = snmpb;

    settings = new QSettings(s->GetAgentsConfigFile(), QSettings::IniFormat, this);

    ap.setupUi(&apw);

    // Set some properties for the Agent Profile TreeView
    ap.ProfileTree->header()->hide();
    ap.ProfileTree->setSortingEnabled( FALSE );
    ap.ProfileTree->header()->setSortIndicatorShown( FALSE );
    ap.ProfileTree->setLineWidth( 2 );
    ap.ProfileTree->setAllColumnsShowFocus( FALSE );
    ap.ProfileTree->setFrameShape(QFrame::WinPanel);
    ap.ProfileTree->setFrameShadow(QFrame::Plain);
    ap.ProfileTree->setRootIsDecorated( TRUE );

    // Create context menu actions
    ap.ProfileTree->setContextMenuPolicy (Qt::CustomContextMenu);
    connect( ap.ProfileTree, 
             SIGNAL( customContextMenuRequested ( const QPoint & ) ),
             this, SLOT( ContextMenu ( const QPoint & ) ) );
    addAct = new QAction(tr("&New agent profile"), this);
    connect(addAct, SIGNAL(triggered()), this, SLOT(Add()));
    deleteAct = new QAction(tr("&Delete agent profile"), this);
    connect(deleteAct, SIGNAL(triggered()), this, SLOT(Delete()));

    connect( ap.ProfileTree, 
             SIGNAL( currentItemChanged( QTreeWidgetItem *, QTreeWidgetItem * ) ),
             this, SLOT( SelectedAgentProfile( QTreeWidgetItem *, QTreeWidgetItem * ) ) );
    connect( ap.ProfileTree, 
             SIGNAL( itemChanged( QTreeWidgetItem *, int ) ),
             this, SLOT( AgentProfileNameChange( QTreeWidgetItem *, int ) ) );
    connect( ap.V1, SIGNAL( toggled(bool) ),
             this, SLOT( ProtocolV1Support(bool) ) );
    connect( ap.V2, SIGNAL( toggled(bool) ),
             this, SLOT( ProtocolV2Support(bool) ) );
    connect( ap.V3, SIGNAL( toggled(bool) ),
             this, SLOT( ProtocolV3Support(bool) ) );
    connect( ap.ProfileName, SIGNAL( editingFinished() ), 
             this, SLOT ( SetProfileName() ) );
    connect( ap.Address, SIGNAL( editingFinished() ), 
             this, SLOT ( SetAddress() ) );
    connect( ap.Port, SIGNAL( editingFinished() ), 
             this, SLOT ( SetPort() ) );
    connect( ap.Retries, SIGNAL( valueChanged( int ) ),
             this, SLOT ( SetRetries() ) );
    connect( ap.Timeout, SIGNAL( valueChanged( int ) ), 
             this, SLOT ( SetTimeout() ) );
    connect( ap.ReadComm, SIGNAL( editingFinished() ), 
             this, SLOT ( SetReadComm() ) );
    connect( ap.WriteComm, SIGNAL( editingFinished() ), 
             this, SLOT ( SetWriteComm() ) );
    connect( ap.MaxRepetitions, SIGNAL( valueChanged( int ) ), 
             this, SLOT ( SetMaxRepetitions() ) );
    connect( ap.NonRepeaters, SIGNAL( valueChanged( int ) ), 
             this, SLOT ( SetNonRepeaters() ) );
    connect( ap.SecName, SIGNAL( currentIndexChanged( int ) ), 
             this, SLOT ( SetSecName() ) );
    connect( ap.SecLevel, SIGNAL( currentIndexChanged( int ) ), 
             this, SLOT ( SetSecLevel() ) );
    connect( ap.ContextName, SIGNAL( editingFinished() ), 
             this, SLOT ( SetContextName() ) );
    connect( ap.ContextEngineID, SIGNAL( editingFinished() ), 
             this, SLOT ( SetContextEngineID() ) );

    // First time the app is started, populate with a minimum config of localhost
    if (!QFile::exists(s->GetAgentsConfigFile()))
    {
        currentprofile = NULL; 
        Add();
        currentprofile = GetAgentProfile("newagent");
        currentprofile->SetName("localhost");
        WriteConfigFile();
        Delete();
    }

    currentprofile = NULL; 

    // Loop & load all stored agent profiles
    ReadConfigFile();

    if (agents.size() != 0)
       ap.ProfileTree->setCurrentItem(ap.ProfileTree->topLevelItem(0));
}

void AgentProfileManager::ReadConfigFile (void)
{
    int size = settings->beginReadArray("agents");
    for (int i = 0; i < size; i++)
    {
        settings->setArrayIndex(i);
        QString _name = settings->value("name").toString();
        AgentProfile *newagent = new AgentProfile(&ap, &_name);
        newagent->SetSupportedProtocol(settings->value("v1").toBool(), 
                                       settings->value("v2").toBool(), 
                                       settings->value("v3").toBool());
        newagent->SetTarget(settings->value("address").toString(),
                            settings->value("port").toString());
        newagent->SetRetriesTimeout(settings->value("retries").toInt(),
                                    settings->value("timeout").toInt());
        newagent->SetComms(settings->value("readcomm").toString(),
                           settings->value("writecomm").toString());
        newagent->SetBulk(settings->value("maxrepetitions").toInt(),
                          settings->value("nonrepeaters").toInt());
        newagent->SetUser(settings->value("secname").toString(), 
                          settings->value("seclevel").toInt());
        newagent->SetContext(settings->value("contextname").toString(), 
                             settings->value("contextengineid").toString());
        agents.append(newagent);
    }
    settings->endArray();
}

void AgentProfileManager::WriteConfigFile (void)
{
    bool v1, v2, v3;
    settings->beginWriteArray("agents");
    settings->remove("");
    for (int i = 0; i < agents.size(); i++)
    {
        settings->setArrayIndex(i);
        settings->setValue("name", agents[i]->GetName());
        agents[i]->GetSupportedProtocol(&v1, &v2, &v3);
        settings->setValue("v1", v1);
        settings->setValue("v2", v2);
        settings->setValue("v3", v3);
        settings->setValue("address", agents[i]->GetAddress());
        settings->setValue("port", agents[i]->GetPort());
        settings->setValue("retries", agents[i]->GetRetries());
        settings->setValue("timeout", agents[i]->GetTimeout());
        settings->setValue("readcomm", agents[i]->GetReadComm());
        settings->setValue("writecomm", agents[i]->GetWriteComm());
        settings->setValue("maxrepetitions", agents[i]->GetMaxRepetitions());
        settings->setValue("nonrepeaters", agents[i]->GetNonRepeaters());
        settings->setValue("secname", agents[i]->GetSecName());
        settings->setValue("seclevel", agents[i]->GetSecLevel());
        settings->setValue("contextname", agents[i]->GetContextName());
        settings->setValue("contextengineid", agents[i]->GetContextEngineID());
    }
    settings->endArray();
}

void AgentProfileManager::Execute (void)
{
    // Fill-in loaded user names
    QString cpn;
    if (currentprofile)
        cpn = currentprofile->GetSecName();
    ap.SecName->clear();
    ap.SecName->addItems(s->UPManagerObj()->GetUsersList());
    if (currentprofile)
    {
        int idx = ap.SecName->findText(cpn);
        ap.SecName->setCurrentIndex(idx>0?idx:0);
    }

    if(apw.exec() == QDialog::Accepted)
    {
        WriteConfigFile();
        emit AgentProfileListChanged();
    }
}

void AgentProfileManager::SetSelectedAgent(QString a)
{
    QTreeWidgetItem *item;
    for (int i = 0; i < ap.ProfileTree->topLevelItemCount(); i++)
    {
        item = ap.ProfileTree->topLevelItem(i);
        if (item->text(0) == a)
        {
            ap.ProfileTree->setCurrentItem(item);
            break;
        }
    }
}

AgentProfile *AgentProfileManager::GetAgentProfile(QString a)
{
    for (int i = 0; i < agents.size(); i++)
    {
        if (agents[i]->GetName() == a)
            return agents[i];
    }
    return NULL;
}

void AgentProfileManager::ProtocolV1Support(bool checked)
{
    if (currentprofile)
        currentprofile->ProtocolV1Support(checked);
}

void AgentProfileManager::ProtocolV2Support(bool checked)
{
    if (currentprofile)
        currentprofile->ProtocolV2Support(checked);
}

void AgentProfileManager::ProtocolV3Support(bool checked)
{
    if (currentprofile)
        currentprofile->ProtocolV3Support(checked);
}

void AgentProfileManager::SetProfileName(void)
{
    if (currentprofile)
        currentprofile->SetProfileName();
}

void AgentProfileManager::SetAddress(void)
{
    if (currentprofile)
        currentprofile->SetAddress();
}

void AgentProfileManager::SetPort(void)
{
    if (currentprofile)
        currentprofile->SetPort();
}

void AgentProfileManager::SetRetries(void)
{
    if (currentprofile)
        currentprofile->SetRetries();
}

void AgentProfileManager::SetTimeout(void)
{
    if (currentprofile)
        currentprofile->SetTimeout();
}

void AgentProfileManager::SetReadComm(void)
{
    if (currentprofile)
        currentprofile->SetReadComm();
}

void AgentProfileManager::SetWriteComm(void)
{
    if (currentprofile)
        currentprofile->SetWriteComm();
}

void AgentProfileManager::SetMaxRepetitions(void)
{
    if (currentprofile)
        currentprofile->SetMaxRepetitions();
}

void AgentProfileManager::SetNonRepeaters(void)
{
    if (currentprofile)
        currentprofile->SetNonRepeaters();
}

void AgentProfileManager::SetSecName(void)
{
    if (currentprofile)
        currentprofile->SetSecName();
}

void AgentProfileManager::SetSecLevel(void)
{
    if (currentprofile)
        currentprofile->SetSecLevel();
}

void AgentProfileManager::SetContextName(void)
{
    if (currentprofile)
        currentprofile->SetContextName();
}

void AgentProfileManager::SetContextEngineID(void)
{
    if (currentprofile)
        currentprofile->SetContextEngineID();
}

void AgentProfileManager::ContextMenu ( const QPoint &pos )
{    
    QMenu menu(tr("Actions"), ap.ProfileTree);

    menu.addAction(addAct);
    menu.addAction(deleteAct);

    menu.exec(ap.ProfileTree->mapToGlobal(pos));
}

void AgentProfileManager::Add(void)
{
    AgentProfile * newagent = new AgentProfile(&ap);
    // Set default values
    newagent->SetSupportedProtocol(true, false, false); // SNMPV1 only
    newagent->SetTarget("127.0.0.1", "161");
    newagent->SetRetriesTimeout(1, 3);
    newagent->SetComms("public", "private");
    newagent->SetBulk(10, 0);
    newagent->SetUser("", 0);
    newagent->SetContext("", "");
    agents.append(newagent);
}

void AgentProfileManager::Delete(void)
{
    QTreeWidgetItem *p = NULL;

    for (int i = 0; i < agents.count(); i++) 
    {
        if (currentprofile && (agents[i] == currentprofile))
        {
            // Delete the profile (removes from the list)
            delete agents.takeAt(i);
            currentprofile = NULL;
            // Readjust the currentprofile pointer with the new current widget item
            if ((p = ap.ProfileTree->currentItem()) != NULL)
            {
                for (int i = 0; i < agents.count(); i++) 
                {
                    if (agents[i]->IsPartOfAgentProfile(p))
                    {
                        currentprofile = agents[i];
                        break;
                    }
                }
            }
            break;
        }
    }
}

void AgentProfileManager::SelectedAgentProfile(QTreeWidgetItem * item, QTreeWidgetItem *)
{
    for (int i = 0; i < agents.count(); i++) 
    {
        if (agents[i]->IsPartOfAgentProfile(item))
        {
            currentprofile = agents[i];
            agents[i]->SelectAgentProfile(item);
            return;
        }
    }
}

void AgentProfileManager::AgentProfileNameChange(QTreeWidgetItem * item, int column)
{
    if (column != 0)
        return;

    for (int i = 0; i < agents.count(); i++) 
    {
        if (agents[i]->IsPartOfAgentProfile(item))
        {
            agents[i]->SetName(item->text(0));
            return;
        }
    }
}

QStringList AgentProfileManager::GetAgentsList(void)
{
    QStringList sl;

    for(int i = 0; i < agents.size(); i++)
        sl << agents[i]->GetName();

    return sl;
}

AgentProfile::AgentProfile(Ui_AgentProfile *uiap, QString *n)
{
    ap = uiap;

    general = new QTreeWidgetItem(ap->ProfileTree);
    general->setFlags(general->flags() | Qt::ItemIsEditable);

    if (n)
    {
        general->setText(0, n->toLatin1().data());
        SetName(*n);
    }
    else
    {
        general->setText(0, "newagent");
        SetName("newagent");
        ap->ProfileTree->editItem(general, 0);
    }

    v1v2c = new QTreeWidgetItem(general);
    v1v2c->setText(0, "Snmpv1/v2c");
    bulk = new QTreeWidgetItem(general);
    bulk->setText(0, "Get-Bulk");
    v3 = new QTreeWidgetItem(general);
    v3->setText(0, "SnmpV3");
}

AgentProfile::~AgentProfile()
{
    delete v1v2c;
    delete bulk;
    delete v3;
    delete general;
}

void AgentProfile::ProtocolV1Support(bool checked)
{
    if (!checked && 
        (ap->V2->checkState() == Qt::Unchecked) && 
        (ap->V3->checkState() == Qt::Unchecked))
    {
        QString err = QString("At least one protocol must be selected\n");
        QMessageBox::critical ( NULL, "SnmpB error", err, 
                                QMessageBox::Ok, Qt::NoButton);
        ap->V1->setCheckState(Qt::Checked); 
        return;
    }

    if (ap->V2->checkState() == Qt::Unchecked)
    {
        if (checked)
            v1v2c->setHidden(false);
        else
            v1v2c->setHidden(true);
    }

    v1support = checked;
}

void AgentProfile::ProtocolV2Support(bool checked)
{
    if (!checked && 
        (ap->V1->checkState() == Qt::Unchecked) && 
        (ap->V3->checkState() == Qt::Unchecked))
    {
        QString err = QString("At least one protocol must be selected\n");
        QMessageBox::critical ( NULL, "SnmpB error", err, 
                                QMessageBox::Ok, Qt::NoButton);
        ap->V2->setCheckState(Qt::Checked); 
        return;
    }

    if (ap->V1->checkState() == Qt::Unchecked)
    {
        if (checked)
            v1v2c->setHidden(false);
        else
            v1v2c->setHidden(true);
    }

    if (ap->V3->checkState() == Qt::Unchecked)
    {
        if (checked)
            bulk->setHidden(false);
        else
            bulk->setHidden(true);
    }

    v2support = checked;
}

void AgentProfile::ProtocolV3Support(bool checked)
{
    if (!checked && 
        (ap->V1->checkState() == Qt::Unchecked) && 
        (ap->V2->checkState() == Qt::Unchecked))
    {
        QString err = QString("At least one protocol must be selected\n");
        QMessageBox::critical ( NULL, "SnmpB error", err, 
                                QMessageBox::Ok, Qt::NoButton);
        ap->V3->setCheckState(Qt::Checked); 
        return;
    }

    if (checked)
        v3->setHidden(false);
    else
        v3->setHidden(true);

    if (ap->V2->checkState() == Qt::Unchecked)
    {
        if (checked)
            bulk->setHidden(false);
        else
            bulk->setHidden(true);
    }

    v3support = checked;
}

int AgentProfile::SelectAgentProfile(QTreeWidgetItem * item)
{
    if (item == general)
    {
        ap->ProfileProps->setCurrentIndex(0);

        ApplySupportedProtocol();
        ap->ProfileName->setText(name);
        ap->Address->setText(address);
        ap->Port->setText(port);
        ap->Retries->setValue(retries);
        ap->Timeout->setValue(timeout);

        return 1;
    }
    else
    if (item == v1v2c)
    {
        ap->ProfileProps->setCurrentIndex(1);

        ap->ReadComm->setText(readcomm);
        ap->WriteComm->setText(writecomm);

        return 1;
    }
    else
    if (item == bulk)
    {
        ap->ProfileProps->setCurrentIndex(2);

        ap->MaxRepetitions->setValue(maxrepetitions);
        ap->NonRepeaters->setValue(nonrepeaters);

        return 1;
    }
    else
    if (item == v3)
    {
        ap->ProfileProps->setCurrentIndex(3);

        ap->SecName->setCurrentIndex(ap->SecName->findText(secname));
        ap->SecLevel->setCurrentIndex(seclevel);
        ap->ContextName->setText(contextname);
        ap->ContextEngineID->setText(contextengineid);

        return 1;
    }

    return 0;
}

void AgentProfile::ApplySupportedProtocol(void)
{
    // Order is important: first set the ones that are supported ...
    if (v1support)
    {
        ap->V1->setCheckState(Qt::Checked); 
        ProtocolV1Support(v1support);
    }
    if (v2support)
    {
        ap->V2->setCheckState(Qt::Checked); 
        ProtocolV2Support(v2support);
    }
    if (v3support)
    {
        ap->V3->setCheckState(Qt::Checked); 
        ProtocolV3Support(v3support);
    }
    // ... then the ones that are not. This avoids the message box protection
    if (!v1support)
    {
        ap->V1->setCheckState(Qt::Unchecked);
        ProtocolV1Support(v1support);
    }
    if (!v2support)
    {
        ap->V2->setCheckState(Qt::Unchecked);
        ProtocolV2Support(v2support);
    }
    if (!v3support)
    {
        ap->V3->setCheckState(Qt::Unchecked);
        ProtocolV3Support(v3support);
    }
}

int AgentProfile::IsPartOfAgentProfile(QTreeWidgetItem * item)
{
    if ((item == general) || (item == v1v2c) || (item == bulk) || (item == v3))
        return 1;
    else
        return 0;
}

void AgentProfile::GetSupportedProtocol(bool *v1, bool *v2, bool *v3)
{
    if (v1) *v1 = v1support;
    if (v2) *v2 = v2support;
    if (v3) *v3 = v3support;
}

void AgentProfile::SetSupportedProtocol(bool v1, bool v2, bool v3)
{
    v1support = v1;
    v2support = v2;
    v3support = v3;

    ApplySupportedProtocol();
}

void AgentProfile::SetName(QString n)
{
    name = n;
    ap->ProfileName->setText(name);  
}

QString AgentProfile::GetName(void)
{
    return name;
}

void AgentProfile::SetProfileName(void)
{
    name = ap->ProfileName->text();
    general->setText(0, name);
}

void AgentProfile::SetAddress(void)
{
    address = ap->Address->text();
}

QString AgentProfile::GetAddress(void)
{
    return address;
}

void AgentProfile::SetPort(void)
{
    port = ap->Port->text();
}

QString AgentProfile::GetPort(void)
{
    return port;
}

void AgentProfile::SetTarget(QString a, QString p)
{
    address = a;
    port = p; 
}

void AgentProfile::SetRetries(void)
{
    retries = ap->Retries->value();
}

int AgentProfile::GetRetries(void)
{
    return retries;
}

void AgentProfile::SetTimeout(void)
{
    timeout = ap->Timeout->value();
}

int AgentProfile::GetTimeout(void)
{
    return timeout;
}

void AgentProfile::SetRetriesTimeout(int r, int t)
{
    retries = r;
    timeout = t;
}

void AgentProfile::SetReadComm(void)
{
    readcomm = ap->ReadComm->text();
}

QString AgentProfile::GetReadComm(void)
{
    return readcomm;
}

void AgentProfile::SetWriteComm(void)
{
    writecomm = ap->WriteComm->text();
}

QString AgentProfile::GetWriteComm(void)
{
    return writecomm;
}

void AgentProfile::SetComms(QString r, QString w)
{
    readcomm = r;
    writecomm = w;
}

void AgentProfile::SetMaxRepetitions(void)
{
    maxrepetitions = ap->MaxRepetitions->value();
}

int AgentProfile::GetMaxRepetitions(void)
{
    return maxrepetitions;
}

void AgentProfile::SetNonRepeaters(void)
{
    nonrepeaters = ap->NonRepeaters->value();
}

int AgentProfile::GetNonRepeaters(void)
{
    return nonrepeaters;
}

void AgentProfile::SetBulk(int mr, int nr)
{
    maxrepetitions = mr;
    nonrepeaters = nr;
}

void AgentProfile::SetSecName(void)
{
    secname = ap->SecName->itemText(ap->SecName->currentIndex());
}

QString AgentProfile::GetSecName(void)
{
    return secname;
}

void AgentProfile::SetSecLevel(void)
{
    seclevel = ap->SecLevel->currentIndex();
}

int AgentProfile::GetSecLevel(void)
{
    return seclevel;
}

void AgentProfile::SetUser(QString u, int l)
{
    secname = u;
    seclevel = l;
}

void AgentProfile::SetContextName(void)
{
    contextname = ap->ContextName->text();
}

QString AgentProfile::GetContextName(void)
{
    return contextname;
}

void AgentProfile::SetContextEngineID(void)
{
    contextengineid = ap->ContextEngineID->text();
}

QString AgentProfile::GetContextEngineID(void)
{
    return contextengineid;
}

void AgentProfile::SetContext(QString n, QString id)
{
    contextname = n;
    contextengineid = id;
}

