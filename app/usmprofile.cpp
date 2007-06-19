#include "usmprofile.h"

USMProfile::USMProfile(Snmpb *snmpb)
{
    s = snmpb;

    general = new QTreeWidgetItem(s->USMProfileUI()->ProfileTree);
    general->setText(0, "default");

    connect( s->USMProfileUI()->ProfileTree, 
             SIGNAL( currentItemChanged( QTreeWidgetItem *, QTreeWidgetItem * ) ),
             this, SLOT( SelectedUSMProfile( QTreeWidgetItem *, QTreeWidgetItem * ) ) );
}

void USMProfile::SelectedUSMProfile(QTreeWidgetItem * item, QTreeWidgetItem *)
{
    if (item == general)
    {
    //    s->USMProfileUI()->ProfileProps->setCurrentIndex(0);
    }
}

