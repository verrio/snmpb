#ifndef AGENT_H
#define AGENT_H

#include <qpushbutton.h>
#include <qwidget.h>
#include <qlabel.h>
#include <qtextedit.h>
#include <qcombobox.h>
#include <qlineedit.h>
#include <qbuttongroup.h>
#include <qspinbox.h>
#include <qradiobutton.h>

#include "mibview.h"
#include "snmp_pp/snmp_pp.h"

class Agent: public QObject
{
    Q_OBJECT
    
public:
    Agent(QComboBox* UN, QComboBox* SL, QLineEdit* CN,
             QLineEdit* EID, QComboBox* AProt, QLineEdit* APass,
             QComboBox* PProt, QLineEdit* PPass, QComboBox* A,
             QComboBox* P, QSpinBox* R, QSpinBox* T,
             QRadioButton* v1, QRadioButton* v2, QRadioButton* v3,
             QLineEdit* RC, QLineEdit* WC, 
             QPushButton* DU, QPushButton* AU, QPushButton* SU,
             MibView* MV, QTextEdit* Q);
    
public slots:
    void WalkFrom(const QString& oid);
    
private:
    QComboBox* UserName;
    QComboBox* SecLevel;
    QLineEdit* ContextName;
    QLineEdit* EngineID;
    QComboBox* AuthProtocol;
    QLineEdit* AuthPass;
    QComboBox* PrivProtocol;
    QLineEdit* PrivPass;
    QComboBox* Address;
    QComboBox* Port;
    QSpinBox* Retries;
    QSpinBox* Timeout;
    QRadioButton* V1;
    QRadioButton* V2;
    QRadioButton* V3;
    QLineEdit* ReadComm;
    QLineEdit* WriteComm;
    QPushButton* DeleteUser;
    QPushButton* AddUser;
    QPushButton* SaveUser;
    QTextEdit* Query;
    
    Snmp *snmp;
    v3MP *v3mp;
};

#endif /* AGENT_H */
