#include "discovery.h"

Discovery::Discovery(Snmpb *snmpb)
{
    s = snmpb;

#if 0 
    connect( s->MainUI()->LogDebug, 
             SIGNAL( stateChanged( int ) ),
             this, SLOT( SetDebugLevelState ( int ) ));
#endif
}

#if 0
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
#endif
