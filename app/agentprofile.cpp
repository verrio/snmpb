#include "agentprofile.h"

AgentProfile::AgentProfile(Snmpb *snmpb)
{
    s = snmpb;

    general = new QTreeWidgetItem(s->AgentProfileUI()->ProfileTree);
    general->setText(0, "localhost");
    v1v2c = new QTreeWidgetItem(general);
    v1v2c->setText(0, "Snmpv1/v2c");
    bulk = new QTreeWidgetItem(general);
    bulk->setText(0, "Get-Bulk");
    v3 = new QTreeWidgetItem(general);
    v3->setText(0, "SnmpV3");

    connect( s->AgentProfileUI()->ProfileTree, 
             SIGNAL( currentItemChanged( QTreeWidgetItem *, QTreeWidgetItem * ) ),
             this, SLOT( SelectedAgentProfile( QTreeWidgetItem *, QTreeWidgetItem * ) ) );
}

void AgentProfile::SelectedAgentProfile(QTreeWidgetItem * item, QTreeWidgetItem *)
{
    if (item == general)
    {
        s->AgentProfileUI()->ProfileProps->setCurrentIndex(0);
    }
    else
    if (item == v1v2c)
    {
        s->AgentProfileUI()->ProfileProps->setCurrentIndex(1);
    }
    else
    if (item == bulk)
    {
        s->AgentProfileUI()->ProfileProps->setCurrentIndex(2);
    }
    else
    if (item == v3)
    {
        s->AgentProfileUI()->ProfileProps->setCurrentIndex(3);
    }
}

