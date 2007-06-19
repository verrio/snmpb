#ifndef AGENTPROFILE_H
#define AGENTPROFILE_H

#include "snmpb.h"
#include "qdialog.h"
#include "qtreewidget.h"

class AgentProfile: public QObject
{
    Q_OBJECT
    
public:
    AgentProfile(Snmpb *snmpb);

protected slots:
    void SelectedAgentProfile( QTreeWidgetItem * item, QTreeWidgetItem * old);

private:
    Snmpb *s;

    QTreeWidgetItem *general;
    QTreeWidgetItem *v1v2c;
    QTreeWidgetItem *bulk;
    QTreeWidgetItem *v3;
};

#endif /* AGENTPROFILE_H */
