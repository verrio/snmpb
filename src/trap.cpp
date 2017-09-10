/**
 * Copyright (c) 2004-2017 Martin Jolicoeur and contributors
 *
 * This file is part of the snmpb project and is free software: you can
 * redistribute it and/or modify it under the terms of the GNU General
 * Public License as published by the Free Software Foundation, either
 * version 3 of the License, or (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <http://www.gnu.org/licenses/>.
 */

#include "smi.h"
#include "trap.h"
#include "agent.h"
#include "preferences.h"

const std::string TrapItem::failure_indicators[4] = { "err",
    "fail", "critical", "lost" };
const std::string TrapItem::warning_indicators[6] = { "warn",
    "disappear", "down", "low", "disconnect", "alert" };
const std::string TrapItem::success_indicators[6] = { "success",
    "complete", "done", "start", "connect", "join" };

TrapItem::TrapItem(Oid &id, QTreeWidget* parent, const QStringList &values,
                   QString community, QString seclevel, 
                   QString ctxname, QString ctxid, QString msgid, bool expand):
                   QTreeWidgetItem(parent, values)
{
    oid = id;
    
    _type = GetTrapType(values[5]);
    _community = community;
    _seclevel = seclevel;
    _ctxname = ctxname;
    _ctxid = ctxid;
    _msgid = msgid;
    _expand = expand;

    setIcon();
}

TrapItem::~TrapItem(void)
{
    for (int i = 0; i < content.count(); i++)
    {
        delete content[i];
    }
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
            char *bh = smiRenderOID(node->oidlen, node->oid,
                                   SMI_RENDER_NUMERIC);
            char *f = (char*)vb->get_printable_oid();
            char *b = bh;
            while ((*b++ == *f++) && (*b != '\0') && (*f != '\0')) ;
            smiFree(bh);
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

void TrapItem::setIcon(void)
{
    switch (_type)
    {
        case TRAPTYPE_SUCCESS: {
            QTreeWidgetItem::setIcon(0, QIcon(":/images/bullet-green.png"));
            break;
        }

        case TRAPTYPE_WARNING: {
            QTreeWidgetItem::setIcon(0, QIcon(":/images/bullet-yellow.png"));
            break;
        }

        case TRAPTYPE_ERROR: {
            QTreeWidgetItem::setIcon(0, QIcon(":/images/bullet-red.png"));
            break;
        }

        default: {
            QTreeWidgetItem::setIcon(0, QIcon(":/images/bullet-grey.png"));
        }
    }
}

enum TrapType TrapItem::GetTrapType(const QString &name)
{
    std::string val = name.toUtf8().constData();
    std::transform(val.begin(), val.end(), val.begin(), ::tolower);

    for (std::string str : failure_indicators)
        if (val.find(str) != std::string::npos)
            return TRAPTYPE_ERROR;
    for (std::string str : warning_indicators)
        if (val.find(str) != std::string::npos)
            return TRAPTYPE_WARNING;
    for (std::string str : success_indicators)
        if (val.find(str) != std::string::npos)
            return TRAPTYPE_SUCCESS;
    return TRAPTYPE_INFO;
}

Trap::Trap(Snmpb *snmpb) : _trap_count{0}
{
    s = snmpb;
    s->MainUI()->TrapLog->setColumnWidth(0, 24);
    s->MainUI()->TrapContent->header()->hide();
    s->MainUI()->TrapContent->setSortingEnabled( false );

    connect( s->MainUI()->TrapLog, 
             SIGNAL( currentItemChanged( QTreeWidgetItem *, QTreeWidgetItem * ) ),
             this, SLOT( SelectedTrap( QTreeWidgetItem *, QTreeWidgetItem * ) ) );
    connect( this, SIGNAL(TrapProperties(const QString&)),
             (QObject*)s->MainUI()->TrapInfo, SLOT(setHtml(const QString&)) );
    connect( s->MainUI()->ClearTraps, SIGNAL( clicked() ), this, SLOT( Clear() ));
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

void Trap::Clear(void)
{
    const QString emptyString;
    s->MainUI()->TrapContent->clear();
    s->MainUI()->TrapInfo->setHtml(emptyString);
    s->MainUI()->TrapLog->clear();
    _trap_count = 0;
}

void Trap::SelectedTrap(QTreeWidgetItem * item, QTreeWidgetItem *)
{
    if (item == NULL)
        return;

    TrapItem *trap = (TrapItem*)item;
    QString text;
    
    trap->PrintContent(s->MainUI()->TrapContent);
    trap->PrintProperties(text);
    emit TrapProperties(text);
}

int Trap::GetNextId(void)
{
    return _trap_count++;
}
