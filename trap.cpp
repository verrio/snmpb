#include <qstylesheet.h>

#include "smi.h"
#include "trap.h"

TrapItem::TrapItem(Oid &id, QListView* parent, QString no, QString date, 
                   QString time, QString timestamp, QString nottype, 
                   QString msgtype, QString version, QString agtaddr) : 
                   QListViewItem(parent, no, date, time, timestamp,
                                 nottype, msgtype, version, agtaddr)
{
    oid = id;
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
    text += QStyleSheet::convertFromPlainText (Node->reference);
    text += QString("</font></td>>/tr>");
       
    // Add the description
    text += QString("<tr><td><b>Description:</b></td><td><font face=fixed size=-1 color=blue>");
    text += QStyleSheet::convertFromPlainText (Node->description);
    text += QString("</font></td>>/tr>");
	   
    text += QString("</table>");
}

void TrapItem::PrintContent(QListView* TrapContent)
{
    new QListViewItem(TrapContent, "allo", "les", "amis");
}

Trap::Trap(QListView* TL, QListView* TC, QTextEdit* TI)
{
    TrapLog = TL;
    TrapContent = TC;
    TrapInfo = TI;
    
    TrapLog->setSortColumn(-1);
    
    connect( TrapLog, SIGNAL( currentChanged( QListViewItem * ) ),
             this, SLOT( SelectedTrap( QListViewItem * ) ) );
    connect( this, SIGNAL(TrapProperties(const QString&)),
             (QObject*)TrapInfo, SLOT(setText(const QString&)) );
}

void Trap::Add(QString &no, QString &date, QString &time, QString &timestamp, 
               QString &nottype, QString &msgtype, QString &version, 
               QString &agtaddr, QString &agtport, Oid &id)
{
    // Create the trap item
    TrapItem *ti = new TrapItem(id, TrapLog, no, date, time, timestamp, 
                                nottype, msgtype, version, agtaddr);
    ti->setText(8, agtport);
}

void Trap::SelectedTrap(QListViewItem * item)
{
    TrapItem *trap = (TrapItem*)item;
    QString text;
    
    trap->PrintContent(TrapContent);
    trap->PrintProperties(text);
    emit TrapProperties(text);
}
