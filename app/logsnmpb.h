#ifndef LOGSNMPB_H
#define LOGSNMPB_H

#include "snmpb.h"
#include "snmp_pp/snmp_pp.h"
#include "snmp_pp/log.h"

class LogSnmpb: public QObject
{
    Q_OBJECT
    
public:
    LogSnmpb(Snmpb *snmpb);
    
protected slots:
    void SetLoggingState ( int state );
    void SetErrorLevelState ( int state );
    void SetWarningLevelState ( int state );
    void SetEventLevelState ( int state );
    void SetInfoLevelState ( int state );
    void SetDebugLevelState ( int state );

private:
    Snmpb *s;
};

class SnmpbAgentLog: public QObject, public AgentLog
{
    Q_OBJECT

public:
    SnmpbAgentLog(QTextEdit *out)
    {
        connect(this, SIGNAL ( SendLog(QString) ), 
                out, SLOT ( append (QString) ));
    };

    ~SnmpbAgentLog() {};

    LogEntry* create_log_entry(unsigned char t) const
    {
        return new LogEntryImpl(t);
    };

    AgentLog& operator+=(const LogEntry *log)
    {
        emit SendLog(log->get_value());
        return *this;
    };

signals:
    void SendLog(QString str);
};

#endif /* LOGSNMPB_H */

