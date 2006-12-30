#ifndef LOG_H
#define LOG_H

#include "snmpb.h"
#include "snmp_pp/snmp_pp.h"
#include "snmp_pp/log.h"

class SnmpbAgentLog: public AgentLog
{
public:
    SnmpbAgentLog(QTextEdit *out):te(out) {};
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

#endif /* LOG_H */

