#ifndef USMPROFILE_H
#define USMPROFILE_H

#include "snmpb.h"
#include "qdialog.h"
#include "qtreewidget.h"

class USMProfile: public QObject
{
    Q_OBJECT
    
public:
    USMProfile(Snmpb *snmpb);

protected slots:
    void SelectedUSMProfile( QTreeWidgetItem * item, QTreeWidgetItem * old);

private:
    Snmpb *s;

    QTreeWidgetItem *general;
};

#endif /* USMPROFILE_H */
