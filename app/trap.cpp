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
#include "smi.h"
#include "trap.h"
#include "agent.h"
#include "preferences.h"

TrapItem::TrapItem(Oid &id, QTreeWidget* parent, const QStringList &values,
                   QString community, QString seclevel, 
                   QString ctxname, QString ctxid, QString msgid, bool expand):
                   QTreeWidgetItem(parent, values)
{
    oid = id;
    
    _community = community;
    _seclevel = seclevel;
    _ctxname = ctxname;
    _ctxid = ctxid;
    _msgid = msgid;

    _expand = expand;
}

void TrapItem::PrintProperties(QString& text)
{
    int oidlen = oid.len();

    if (oidlen <= 0)
        return; 

    /* Convert from unsigned long to unsigned int for 64 bit platforms */
    SmiSubid *ioid = new SmiSubid[oidlen];

    for (int idx = 0; idx < oidlen; idx++)
        ioid[idx] = oid[idx];

    SmiNode *Node = smiGetNodeByOID(oidlen, &ioid[0]);

    delete [] ioid;

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
    text += QString("<tr><td><b>Reference:</b></td><td><font face=fixed color=blue>");
    text += Qt::convertFromPlainText (Node->reference);
    text += QString("</font></td></tr>");
       
    // Add the description
    text += QString("<tr><td><b>Description:</b></td><td><font face=fixed color=blue>");
    text += Qt::convertFromPlainText (Node->description);
    text += QString("</font></td></tr>");
	   
    text += QString("</table>");
}

void TrapItem::PrintContent(QTreeWidget* TrapContent)
{
    TrapContent->clear();

    QString com_title = QString("Community: %1").arg(_community);
    new QTreeWidgetItem(TrapContent, QStringList(com_title));
     
    QString bd_title = QString("Bindings (%1)").arg(content.count());
    QTreeWidgetItem *bd = new QTreeWidgetItem(TrapContent, QStringList(bd_title));
    bd->setExpanded(_expand);
 
    Vb *vb;
    Oid id;
    QString bd_val;
   
    for (int i = 0; i < content.count(); i++) 
    {    
        vb = content[i];
        bd_val = QString("");
        vb->get_oid(id);
        
        SmiNode *node = Agent::GetNodeFromOid(id);
        if (node)
        {
            char *b = smiRenderOID(node->oidlen, node->oid, 
                                   SMI_RENDER_NUMERIC);
            char *f = (char*)vb->get_printable_oid();
            while ((*b++ == *f++) && (*b != '\0') && (*f != '\0')) ;
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
}

void TrapItem::AddVarBind(Vb& vb)
{
    content.append(new Vb(vb));
}
   
Trap::Trap(Snmpb *snmpb)
{
    s = snmpb;
 
    s->MainUI()->TrapContent->header()->hide();
    s->MainUI()->TrapContent->setSortingEnabled( false );

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
                                ctxname, ctxid, msgid,
                                s->PreferencesObj()->GetExpandTrapBinding());
    
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
