/*
    Copyright (C) 2004-2008  Martin Jolicoeur (snmpb1@gmail.com) 

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
#include "logsnmpb.h"

LogSnmpb::LogSnmpb(Snmpb *snmpb)
{
    s = snmpb;
 
    connect( s->MainUI()->LogEnable, 
             SIGNAL( stateChanged( int ) ),
             this, SLOT( SetLoggingState ( int ) ));
    connect( s->MainUI()->LogError, 
             SIGNAL( stateChanged( int ) ),
             this, SLOT( SetErrorLevelState ( int ) ));
    connect( s->MainUI()->LogWarning, 
             SIGNAL( stateChanged( int ) ),
             this, SLOT( SetWarningLevelState ( int ) ));
    connect( s->MainUI()->LogEvent, 
             SIGNAL( stateChanged( int ) ),
             this, SLOT( SetEventLevelState ( int ) ));
    connect( s->MainUI()->LogInfo, 
             SIGNAL( stateChanged( int ) ),
             this, SLOT( SetInfoLevelState ( int ) ));
    connect( s->MainUI()->LogDebug, 
             SIGNAL( stateChanged( int ) ),
             this, SLOT( SetDebugLevelState ( int ) ));

    // Initialize SNMP++ logging system
    DefaultLog::init(new SnmpbAgentLog(s->MainUI()->LogOutput));

    SetLoggingState (s->MainUI()->LogEnable->checkState());
}

void LogSnmpb::SetLoggingState ( int state )
{
    if (state == Qt::Checked)
    {
        s->MainUI()->LogOutput->setEnabled(true);
        s->MainUI()->LogLevels->setEnabled(true);

        SetErrorLevelState(s->MainUI()->LogError->checkState());
        SetWarningLevelState(s->MainUI()->LogWarning->checkState());
        SetEventLevelState(s->MainUI()->LogEvent->checkState());
        SetInfoLevelState(s->MainUI()->LogInfo->checkState());
        SetDebugLevelState(s->MainUI()->LogDebug->checkState());
    }
    else 
    if (state == Qt::Unchecked)
    {
        s->MainUI()->LogOutput->setEnabled(false);
        s->MainUI()->LogLevels->setEnabled(false);

        for (int i = 1; i <= LOG_TYPES; i++)
            DefaultLog::log()->set_filter(i<<4, 0); 
    }
}

void LogSnmpb::SetErrorLevelState ( int state )
{
    if (state == Qt::Checked)
    {
        DefaultLog::log()->set_filter(ERROR_LOG, 15); 
    }
    else 
    if (state == Qt::Unchecked)
    {
        DefaultLog::log()->set_filter(ERROR_LOG, 0); 
    }
}

void LogSnmpb::SetWarningLevelState ( int state )
{
    if (state == Qt::Checked)
    {
        DefaultLog::log()->set_filter(WARNING_LOG, 15); 
    }
    else 
    if (state == Qt::Unchecked)
    {
        DefaultLog::log()->set_filter(WARNING_LOG, 0); 
    }
}

void LogSnmpb::SetEventLevelState ( int state )
{
    if (state == Qt::Checked)
    {
        DefaultLog::log()->set_filter(EVENT_LOG, 15); 
    }
    else 
    if (state == Qt::Unchecked)
    {
        DefaultLog::log()->set_filter(EVENT_LOG, 0); 
    }
}

void LogSnmpb::SetInfoLevelState ( int state )
{
    if (state == Qt::Checked)
    {
        DefaultLog::log()->set_filter(INFO_LOG, 15); 
    }
    else 
    if (state == Qt::Unchecked)
    {
        DefaultLog::log()->set_filter(INFO_LOG, 0); 
    }
}

void LogSnmpb::SetDebugLevelState ( int state )
{
    if (state == Qt::Checked)
    {
        DefaultLog::log()->set_filter(DEBUG_LOG, 15); 
    }
    else 
    if (state == Qt::Unchecked)
    {
        DefaultLog::log()->set_filter(DEBUG_LOG, 0); 
    }
}

