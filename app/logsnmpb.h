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

class SnmpbAgentLog: public AgentLog
{
public:
    SnmpbAgentLog(QTextEdit *out) : te(out) {};
    ~SnmpbAgentLog() {};

    void lock()
    {
#ifdef _THREADS
        logLock.lock();
#endif
    }

    void unlock()
    {
#ifdef _THREADS
        logLock.unlock();
#endif
    }

    LogEntry* create_log_entry(unsigned char t) const
    {
        return new LogEntryImpl(t);
    };

    AgentLog& operator+=(const LogEntry *log)
    {
        te->append(log->get_value());
        return *this;
    };

private:
    QTextEdit *te;
    SnmpSynchronized logLock;
};

#endif /* LOGSNMPB_H */

