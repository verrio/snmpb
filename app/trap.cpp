#include <q3stylesheet.h>

#include "smi.h"
#include "trap.h"
#include "agent.h"

TrapItem::TrapItem(Oid &id, Q3ListView* parent, QString no, QString date,
                   QString time, QString timestamp,
                   QString nottype, QString msgtype, QString version, 
                   QString agtaddr, QString agtport,
                   QString community, QString seclevel, 
                   QString ctxname, QString ctxid, QString msgid) : 
                   Q3ListViewItem(parent, no, date, time, timestamp,
                                 nottype, msgtype, version, agtaddr)
{
    setText(8, agtport);
    oid = id;
    
    _no = no;
    _date = date;
    _time = time;
    _timestamp = timestamp;
    _nottype = nottype;
    _msgtype = msgtype;
    _version = version;
    _agtaddr = agtaddr;
    _agtport = agtport;
    _community = community;
    _seclevel = seclevel;
    _ctxname = ctxname;
    _ctxid = ctxid;
    _msgid = msgid;
                   
    content.setAutoDelete(TRUE);
}

void TrapItem::PrintProperties(QString& text)
{
    SmiNode *Node = smiGetNodeByOID(oid.len(), (unsigned int *)&(oid[0]));
    if (!Node || (Node->nodekind != SMI_NODEKIND_NOTIFICATION))
        return;
        
    // Create a table and add elements ...
    text = QString("<table border=\"1\" cellpadding=\"0\" cellspacing=\"0\" align=\"left\">");  
       
    // Add the name
    text += QString("<tr><td><b>Name:</b></td><td><font color=#009000><b>%1</b></font></td>").arg(Node->name);
	   
    // Add the full Oid
    text += QString("<tr><td><b>Oid:</b></td><td>%1</td></tr>").arg(smiRenderOID(Node->oidlen, Node->oid, SMI_RENDER_NUMERIC));
              
    // Add units (seconds, bits, ....)
    text += QString("<tr><td><b>Units:</b></td><td>%1</td></tr>").arg(Node->units);
       
    // Add module
    text += QString("<tr><td><b>Module:</b></td><td>%1</td></tr>").arg(smiGetNodeModule(Node)->name);

    // Add the reference
    text += QString("<tr><td><b>Reference:</b></td><td><font face=fixed size=-1 color=blue>");
    text += Q3StyleSheet::convertFromPlainText (Node->reference);
    text += QString("</font></td></tr>");
       
    // Add the description
    text += QString("<tr><td><b>Description:</b></td><td><font face=fixed size=-1 color=blue>");
    text += Q3StyleSheet::convertFromPlainText (Node->description);
    text += QString("</font></td></tr>");
	   
    text += QString("</table>");
}

void TrapItem::PrintContent(Q3ListView* TrapContent)
{
    TrapContent->clear();
        
    QString bd_title = QString("Bindings (%1)").arg(content.count());
    Q3ListViewItem *bd = new Q3ListViewItem(TrapContent, bd_title);
    
    Q3PtrListIterator<Vb> it( content );
    Vb *vb;
    Oid id;
    QString bd_val;
    int i = 0;
    
    while ( (vb = it.current()) != 0 )
    {    
        bd_val = QString("");
        vb->get_oid(id);
        unsigned long* moid = &(id[0]);
        unsigned long  oidlen = id.len();
        
        SmiNode *node = smiGetNodeByOID(oidlen, (unsigned int *)moid);
        if (node)
        {
            char *b = smiRenderOID(node->oidlen, node->oid, 
                                   SMI_RENDER_NUMERIC);
            char *f = (char*)vb->get_printable_oid();
            while ((*b++ == *f++) && (*b != '\0') && (*f != '\0'));
            /* f is now the remaining part */
                    
            // Print the OID part
            bd_val += QString("#%1 %2").arg(i).arg(node->name);
            if (*f != '\0') bd_val += QString("%1").arg(f);
            
            // Print the value part
            bd_val += QString(": %1").arg(Agent::GetPrintableValue(node, vb));            
        }
        else
        {
            /* Unknown OID */
            bd_val += QString("#%1 %2: %3").arg(i).arg(vb->get_printable_oid())
                                           .arg(vb->get_printable_value());
        }
        
        new Q3ListViewItem(bd, bd_val);
        ++it;
        i++;
    }
    
    QString com_title = QString("Community: %1").arg(_community);
    new Q3ListViewItem(TrapContent, com_title);
}

void TrapItem::AddVarBind(Vb& vb)
{
    content.append(new Vb(vb));
}
   
Trap::Trap(Q3ListView* TL, Q3ListView* TC, QTextEdit* TI)
{
    TrapLog = TL;
    TrapContent = TC;
    TrapInfo = TI;
    
    TrapContent->header()->hide();
    TrapContent->setSortColumn(-1);

    connect( TrapLog, SIGNAL( currentChanged( Q3ListViewItem * ) ),
             this, SLOT( SelectedTrap( Q3ListViewItem * ) ) );
    connect( this, SIGNAL(TrapProperties(const QString&)),
             (QObject*)TrapInfo, SLOT(setText(const QString&)) );
}

TrapItem* Trap::Add(Oid &id, QString &no, QString &date, 
                    QString &time, QString &timestamp, 
                    QString &nottype, QString &msgtype, QString &version, 
                    QString &agtaddr, QString &agtport,
                    QString &community, QString &seclevel,
                    QString &ctxname, QString &ctxid, QString &msgid)
{
    // Create the trap item
    TrapItem *ti = new TrapItem(id, TrapLog, no, date, time, timestamp,
                                nottype, msgtype, version, agtaddr, agtport,
                                community, seclevel, ctxname, ctxid, msgid);
    
    return (ti);
}

void Trap::SelectedTrap(Q3ListViewItem * item)
{
    TrapItem *trap = (TrapItem*)item;
    QString text;
    
    trap->PrintContent(TrapContent);
    trap->PrintProperties(text);
    emit TrapProperties(text);
}
