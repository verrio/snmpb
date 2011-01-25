/*
    Copyright (C) 2004-2011 Martin Jolicoeur (snmpb1@gmail.com) 

    This file is part of the SnmpB project 
    (http://sourceforge.net/projects/snmpb)

    This program is free software: you can redistribute it and/or modify
    it under the terms of the GNU General Public License as published by
    the Free Software Foundation, either version 2 of the License, or
    (at your option) any later version.

    This program is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
    GNU General Public License for more details.

    You should have received a copy of the GNU General Public License
    along with this program.  If not, see <http://www.gnu.org/licenses/>.
*/
#ifndef LOGSNMPB_H
#define LOGSNMPB_H

#include <qsettings.h>

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
    QSettings *settings;
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

