#include "smi.h"
#include "trap.h"
#include "agent.h"

TrapItem::TrapItem(Oid &id, QTreeWidget* parent, const QStringList &values,
                   QString community, QString seclevel, 
                   QString ctxname, QString ctxid, QString msgid):
                   QTreeWidgetItem(parent, values)
{
    oid = id;
    
    _community = community;
    _seclevel = seclevel;
    _ctxname = ctxname;
    _ctxid = ctxid;
    _msgid = msgid;
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
    text += Qt::convertFromPlainText (Node->reference);
    text += QString("</font></td></tr>");
       
    // Add the description
    text += QString("<tr><td><b>Description:</b></td><td><font face=fixed size=-1 color=blue>");
    text += Qt::convertFromPlainText (Node->description);
    text += QString("</font></td></tr>");
	   
    text += QString("</table>");
}

void TrapItem::PrintContent(QTreeWidget* TrapContent)
{
    TrapContent->clear();
     
    QString bd_title = QString("Bindings (%1)").arg(content.count());
    QTreeWidgetItem *bd = new QTreeWidgetItem(TrapContent, QStringList(bd_title));
     
    Vb *vb;
    Oid id;
    QString bd_val;
   
    for (int i = 0; i < content.count(); i++) 
    {    
        vb = content[i];
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
        
        new QTreeWidgetItem(bd, QStringList(bd_val));
    }

    QString com_title = QString("Community: %1").arg(_community);
    new QTreeWidgetItem(TrapContent, QStringList(com_title));
}

void TrapItem::AddVarBind(Vb& vb)
{
    content.append(new Vb(vb));
}
   
Trap::Trap(Snmpb *snmpb)
{
    s = snmpb;
 
    s->MainUI()->TrapContent->header()->hide();
    s->MainUI()->TrapContent->setSortingEnabled( FALSE );

    connect( s->MainUI()->TrapLog, 
             SIGNAL( currentItemChanged( QTreeWidgetItem *, QTreeWidgetItem * ) ),
             this, SLOT( SelectedTrap( QTreeWidgetItem *, QTreeWidgetItem * ) ) );
    connect( this, SIGNAL(TrapProperties(const QString&)),
             (QObject*)s->MainUI()->TrapInfo, SLOT(setHtml(const QString&)) );
}

TrapItem* Trap::Add(Oid &id, const QStringList &values,
                    QString &community, QString &seclevel,
                    QString &ctxname, QString &ctxid, QString &msgid)
{
    // Create the trap item
    TrapItem *ti = new TrapItem(id, s->MainUI()->TrapLog, 
                                values, community, seclevel, 
                                ctxname, ctxid, msgid);
    
    return (ti);
}

void Trap::SelectedTrap(QTreeWidgetItem * item, QTreeWidgetItem *)
{
    TrapItem *trap = (TrapItem*)item;
    QString text;
    
    trap->PrintContent(s->MainUI()->TrapContent);
    trap->PrintProperties(text);
    emit TrapProperties(text);
}
