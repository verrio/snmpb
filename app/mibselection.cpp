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
#include <qmessagebox.h>
#include <QDate>
#include <QDialog>
#include <QValidator>

#include "mibselection.h"
#include "agent.h"

MibSelection::MibSelection(Snmpb *snmpb, QWidget *parent, QString title)
{
    s = snmpb;

    validator = NULL;
    validator64 = NULL;
    val_cb = NULL;
    val_le = NULL;

    type = NULL;
    node = NULL;

    dprompt = new QDialog(parent, Qt::WindowTitleHint);
    dprompt->resize(400, 500);

    gl = new QGridLayout(dprompt);
    oidlabel = new QLabel("<b>OID:</b>", dprompt);
    gl->addWidget(oidlabel, 1, 0, 1, 1);

    oid_le = new QLineEdit("", dprompt);
    connect(oid_le, SIGNAL(editingFinished(void)), 
            this, SLOT(GetOidLe(void)));
    gl->addWidget(oid_le, 2, 0, 1, 1);

    bmv = new BasicMibView(dprompt);
    bmv->RegisterToLoader(s->MibLoaderObj());
    bmv->Populate();
    gl->addWidget(bmv, 3, 0, 1, 1);

    syntaxlabel = new QLabel("<b>Syntax:</b>", dprompt);
    gl->addWidget(syntaxlabel, 4, 0, 1, 1);

    syntax_cb = new QComboBox(dprompt);
    gl->addWidget(syntax_cb, 5, 0, 1, 1);
    syntax_cb->addItem("Integer32", QVariant(sNMP_SYNTAX_INT32));
    syntax_cb->addItem("UInteger32", QVariant(sNMP_SYNTAX_UINT32));
    syntax_cb->addItem("Counter32", QVariant(sNMP_SYNTAX_CNTR32));
    syntax_cb->addItem("Gauge32", QVariant(sNMP_SYNTAX_GAUGE32));
    syntax_cb->addItem("Octet String", QVariant(sNMP_SYNTAX_OCTETS));
    syntax_cb->addItem("Bits", QVariant(sNMP_SYNTAX_BITS));
    syntax_cb->addItem("Object Identifier", QVariant(sNMP_SYNTAX_OID));
    syntax_cb->addItem("Counter64", QVariant(sNMP_SYNTAX_CNTR64));
    syntax_cb->addItem("Timeticks", QVariant(sNMP_SYNTAX_TIMETICKS));
    syntax_cb->addItem("IP address", QVariant(sNMP_SYNTAX_IPADDR));
    syntax_cb->addItem("Opaque", QVariant(sNMP_SYNTAX_OPAQUE));

    connect(syntax_cb, SIGNAL(currentIndexChanged(int)), 
            this, SLOT(GetSyntaxCb(int)));

    vallabel = new QLabel("<b>Value:</b>", dprompt);
    gl->addWidget(vallabel, 6, 0, 1, 1);

    val_le = new QLineEdit(dprompt);
    gl->addWidget(val_le, 7, 0, 1, 1);

    val_cb = new QComboBox(dprompt);
    val_cb->setVisible(false);
    gl->addWidget(val_cb, 8, 0, 1, 1);

    infolabel = new QLabel("");
    gl->addWidget(infolabel, 9, 0, 1, 1);

    dprompt->setWindowTitle(QApplication::translate("Dialog", 
                            title.toLatin1().data(), 0, QApplication::UnicodeUTF8));
    box = new QDialogButtonBox(QDialogButtonBox::Ok|QDialogButtonBox::Cancel, 
                               Qt::Horizontal, dprompt);
    gl->addWidget(box, 10, 0, 1, 1);
    connect(box, SIGNAL(accepted()), this, SLOT(OKButtonPressed()));
    connect(box, SIGNAL(rejected()), dprompt, SLOT(reject()));
    QMetaObject::connectSlotsByName(dprompt);
}

MibSelection::~MibSelection()
{
    if (gl)
        delete gl;
    if (oidlabel)
        delete oidlabel;
    if (syntaxlabel)
        delete syntaxlabel;
    if (vallabel)
        delete vallabel;
    if (infolabel)
        delete infolabel;
    if (validator)
        delete validator;
    if (validator64)
        delete validator64;
    if (box)
        delete box;
    if (val_cb)
        delete val_cb;
    if (syntax_cb)
        delete syntax_cb;
    if (val_le)
        delete val_le;
    if (oid_le)
        delete oid_le;
    if (bmv)
        delete bmv;
    if (dprompt)
        delete dprompt;
}

Vb *MibSelection::GetVarbind(void)
{
    return &vb;
}

QString MibSelection::GetSyntaxName(void)
{
    return syntax;
}

int MibSelection::GetSyntax(void)
{
    return result_syntax;
}

QString MibSelection::GetOid(void)
{
    return result_oid;
}

QString MibSelection::GetValue(void)
{
    return result_string;
}

QString MibSelection::GetName(void)
{
    if (node)
    {
        char *b = smiRenderOID(node->oidlen, node->oid, 
                               SMI_RENDER_NUMERIC);
        char *f = result_oid.toLatin1().data();
        while ((*b++ == *f++) && (*b != '\0') && (*f != '\0')) ;
        /* f is now the remaining part */

        if (*f != '\0')
            return QString("%1%2").arg(node->name).arg(f);
        else
            return QString("%1").arg(node->name);
    }
    else
        return  result_oid;
}

SmiNode *MibSelection::GetNode(void)
{
    return node;
}

// Callback when the value combobox value changes 
void MibSelection::GetValueCb(int index)
{
    result_string = val_cb->itemData(index).toString();
    val_le->setText(result_string);
}

// Callback when the value linedit edition is finished
void MibSelection::GetValueLe(void)
{
    result_string = val_le->text();
}

// Callback when the OID linedit edition is finished
void MibSelection::GetOidLe(void)
{
    result_oid = oid_le->text();
}

// Callback when the syntax combobox value changes
void MibSelection::GetSyntaxCb(int index)
{
    result_syntax = syntax_cb->itemData(index).toInt();
    val_le->clear();
    SetValueWidget();
}

// Callback when an item is selected in the MIB tree window 
void MibSelection::GetSelectedOid(const QString& oid)
{
    SetOidInfoType(oid);
    SetSyntax();
}

// Callback when the user presses the OK button 
void MibSelection::OKButtonPressed(void)
{
    // Fill-in pdu and do validation checking
    if (result_oid.isEmpty())
    {
        QMessageBox::critical( NULL, "MIB Operation", 
                "Invalid OID value",
                QMessageBox::Ok, Qt::NoButton);
        return;
    }

    switch (result_syntax)
    {
        case sNMP_SYNTAX_INT32:
            {
                SnmpInt32 v(result_string.toLong());
                vb.set_value(v);
                syntax = "INTEGER32";
                break;
            }
        case sNMP_SYNTAX_CNTR32:
            {
                Counter32 v(result_string.toULong());
                vb.set_value(v);
                syntax = "COUNTER32";
                break;
            }
        /*case sNMP_SYNTAX_UINT32:*/
        case sNMP_SYNTAX_GAUGE32:
            {
                Gauge32 v(result_string.toULong());
                vb.set_value(v);
                syntax = "GAUGE32";
                break;
            }
        case sNMP_SYNTAX_OCTETS:
        case sNMP_SYNTAX_BITS: /* Obsoleted ? */
            {
                if (result_syntax == sNMP_SYNTAX_OCTETS)
                    syntax = "OCTET STRING";
                else
                    syntax = "BITS";
 
                OctetStr octetstr(result_string.toLatin1().data());
                if (octetstr.valid() || result_string.isEmpty())
                    vb.set_value(octetstr);
                else
                {
                    QMessageBox::critical( NULL, "MIB Operation", 
                            "Invalid Octet String value",
                            QMessageBox::Ok, Qt::NoButton);
                    return;
                } 
                break;
            }
        case sNMP_SYNTAX_OPAQUE:
            {
                syntax = "OPAQUE";
                OpaqueStr opaquestr(result_string.toLatin1().data());
                if (opaquestr.valid() || result_string.isEmpty())
                    vb.set_value(opaquestr);
                else
                {
                    QMessageBox::critical( NULL, "MIB Operation", 
                            "Invalid Opaque String value",
                            QMessageBox::Ok, Qt::NoButton);
                    return;
                } 
                break;
            }
        case sNMP_SYNTAX_OID:
            {
                syntax = "OBJECT IDENTIFIER";
                Oid oid(result_string.toLatin1().data());
                if (oid.valid() || result_string.isEmpty())
                    vb.set_value(oid.valid()?oid:"0");
                else
                {
                    QMessageBox::critical( NULL, "MIB Operation", 
                            "Invalid Object Identifier value",
                            QMessageBox::Ok, Qt::NoButton);
                    return;
                }
                break;
            }
        case sNMP_SYNTAX_CNTR64:
            syntax = "COUNTER64";
            vb.set_value(Counter64::ll_to_c64(result_string.toULongLong()));
            break;
        case sNMP_SYNTAX_TIMETICKS:
            {
                syntax = "TIMETICKS";
                TimeTicks timeticks(result_string.toUInt());
                if (timeticks.valid() || result_string.isEmpty())
                    vb.set_value(timeticks);
                else
                {
                    QMessageBox::critical( NULL, "MIB Operation", 
                            "Invalid TimeTick value",
                            QMessageBox::Ok, Qt::NoButton);
                    return;
                }
                break;
            }
        case sNMP_SYNTAX_IPADDR:
            {
                syntax = "IP ADDRESS";
                IpAddress ipaddress(result_string.toLatin1().data());
                if (ipaddress.valid() || result_string.isEmpty())
                    vb.set_value( ipaddress);
                else
                {
                    QMessageBox::critical( NULL, "MIB Operation", 
                            "Invalid IP address value",
                            QMessageBox::Ok, Qt::NoButton);
                    return;
                } 
                break;
            }
        default:
            break;
    }

    dprompt->accept();
}

void MibSelection::SetOidInfoType(const QString& oid)
{
    Oid poid(oid.toLatin1().data());

    QString outoid = oid;
    QString outinfo = "";
    SmiNode *thenode;
    SmiType *thetype;
    type = NULL;
    node = NULL;

    if (poid.valid() == true)
    {
        thenode = Agent::GetNodeFromOid(poid);
        if (thenode && ((thenode->oidlen >= poid.len()) || 
                        (thenode->nodekind == SMI_NODEKIND_COLUMN)))
        {
            thetype = smiGetNodeType(thenode);
            SmiRange *r;

            // If a column object, ask the user to select the instance ...
            if ((thenode->nodekind == SMI_NODEKIND_COLUMN) && 
                (thenode->oidlen >= poid.len()))
            {
                QString inst;
                // Pop-up the selection dialog
                if (s->AgentObj()->SelectTableInstance(oid, inst))
                    outoid = oid + "." + inst;
                else
                    outoid = oid; // Invalid, but no instance selected ...
            }
            else
                outoid = oid;

            node = thenode;

            if (thetype)
            {
                if (smiGetFirstRange(thetype))
                {
                    outinfo += QString("<br><b>Range:</b> ");
                    for (r = smiGetFirstRange(thetype); r; r = smiGetNextRange(r))
                    {
                        outinfo += QString("%1 .. %2")
                                           .arg(r->minValue.value.unsigned64)
                                           .arg(r->maxValue.value.unsigned64);
                        if (smiGetNextRange(r))
                            outinfo += ", ";
                    }
                }
            }

            if (thenode->access != SMI_ACCESS_READ_WRITE)
                outinfo += "<br><b><font color=red>WARNING: object is not writable!</font></b>";

            type = thetype;
        }
    }

    oid_le->setText(outoid);
    result_oid = oid_le->text();

    infolabel->setText(outinfo);
}

void MibSelection::SetValueWidget(void)
{
    SmiNamedNumber  *nn;

    val_cb->clear();
    val_cb->setVisible(false);

    if (type && smiGetFirstNamedNumber(type) && 
        (type->basetype == SMI_BASETYPE_ENUM))
    {
        val_cb->setVisible(true);

        for (nn = smiGetFirstNamedNumber(type); nn; nn = smiGetNextNamedNumber(nn))
            val_cb->addItem(QString("%1 (%2)")
                            .arg(nn->name).arg(nn->value.value.unsigned32), 
                            QVariant((unsigned int)nn->value.value.unsigned32));
        connect(val_cb, SIGNAL(currentIndexChanged(int)), 
                this, SLOT(GetValueCb(int)));
    }

    switch (result_syntax)
    {
        case sNMP_SYNTAX_OCTETS:
        case sNMP_SYNTAX_BITS:
        case sNMP_SYNTAX_OPAQUE:
        case sNMP_SYNTAX_OID:
        case sNMP_SYNTAX_IPADDR:
            val_le->setMaxLength(65535);
            val_le->setValidator(NULL);
            break;
        case sNMP_SYNTAX_INT32:
            if (validator)
                delete validator;
            validator = new IntValidator(false, val_le);
            val_le->setValidator(validator);
            break;
        case sNMP_SYNTAX_CNTR64:
            if (validator64)
                delete validator64;
            validator64 = new UInt64Validator(val_le);
            val_le->setValidator(validator64);
            break;
        default:
            if (validator)
                delete validator;
            validator = new IntValidator(true, val_le);
            val_le->setValidator(validator);
    }

    connect(val_le, SIGNAL(editingFinished(void)), 
            this, SLOT(GetValueLe(void)));
}

void MibSelection::SetSyntax(int st)
{
    if (st != -1) // A syntax is specified in parameter
    {
        switch (st)
        {
            case sNMP_SYNTAX_INT32: syntax_cb->setCurrentIndex(0); break;
                                    /*case sNMP_SYNTAX_UINT32:*/
            case sNMP_SYNTAX_CNTR32: syntax_cb->setCurrentIndex(2); break;
            case sNMP_SYNTAX_GAUGE32: syntax_cb->setCurrentIndex(3); break;
            case sNMP_SYNTAX_OCTETS: syntax_cb->setCurrentIndex(4); break;
            case sNMP_SYNTAX_BITS: syntax_cb->setCurrentIndex(5); break;
            case sNMP_SYNTAX_OID: syntax_cb->setCurrentIndex(6); break;
            case sNMP_SYNTAX_CNTR64: syntax_cb->setCurrentIndex(7); break;
            case sNMP_SYNTAX_TIMETICKS: syntax_cb->setCurrentIndex(8); break;
            case sNMP_SYNTAX_IPADDR: syntax_cb->setCurrentIndex(9); break;
            case sNMP_SYNTAX_OPAQUE: syntax_cb->setCurrentIndex(10); break;
            default: break;
        }
    }
    else // Get the syntax from the MIB information
    {
        if (type)
        {
            switch (type->basetype)
            {
                case SMI_BASETYPE_INTEGER32:
                case SMI_BASETYPE_ENUM:
                    syntax_cb->setCurrentIndex(0);
                    break;
                case SMI_BASETYPE_UNSIGNED32:
                    if (type->name)
                    {
                        if (!strcmp(type->name, "TimeTicks"))
                        {
                            syntax_cb->setCurrentIndex(8);
                            break;
                        }
                        else if (!strcmp(type->name, "Counter32") || 
                                !strcmp(type->name, "COUNTER"))
                        {
                            syntax_cb->setCurrentIndex(2);
                            break;
                        }
                        else if (!strcmp(type->name, "Gauge32") || 
                                !strcmp(type->name, "GAUGE"))
                        {
                            syntax_cb->setCurrentIndex(3);
                            break;
                        }
                    }
                    syntax_cb->setCurrentIndex(1);
                    break;
                case SMI_BASETYPE_OCTETSTRING:
                    if (type->name)
                    {
                        if (!strcmp(type->name, "IpAddress"))
                        {
                            syntax_cb->setCurrentIndex(9);
                            break;
                        }
                        else if (!strcmp(type->name, "Opaque"))
                        {
                            syntax_cb->setCurrentIndex(10);
                            break;
                        }
                    }
                    syntax_cb->setCurrentIndex(4);
                    break;
                case SMI_BASETYPE_BITS: 
                    syntax_cb->setCurrentIndex(5);
                    break;
                case SMI_BASETYPE_OBJECTIDENTIFIER:
                    syntax_cb->setCurrentIndex(6);
                    break;
                case SMI_BASETYPE_UNSIGNED64:
                    syntax_cb->setCurrentIndex(7);
                    break;
                default:
                    break;
            }
        }
        else // default
            syntax_cb->setCurrentIndex(4);
    }

    GetSyntaxCb(syntax_cb->currentIndex());
}

bool MibSelection::run(const QString& init_oid, int init_syntax, const QString& init_val)
{
    bool status = false;

    bmv->SelectFromOid(init_oid);

    connect( bmv, SIGNAL( SelectedOid(const QString&) ), 
             this, SLOT( GetSelectedOid(const QString&) ));

    SetOidInfoType(init_oid); 
    SetSyntax(init_syntax);

    val_le->setFocus(Qt::OtherFocusReason);

    // Set a pre-determined value
    if (!init_val.isEmpty())
    {
        val_le->setText(init_val);
        GetValueLe();
        if (type && (type->basetype == SMI_BASETYPE_ENUM))
        {
            for (int i = 0; i < val_cb->count(); i++)
                if (val_cb->itemData(i).toUInt() == init_val.toUInt())
                    val_cb->setCurrentIndex(i);
        }
    }

    // Show the gui to the user. If OK is pressed, construct and send the packet
    if (dprompt->exec())
    {
        QString tmpoid;
        if (node && (node->nodekind == SMI_NODEKIND_SCALAR))
            tmpoid = result_oid + ".0";
        else
            tmpoid = result_oid;

        vb.set_oid(Oid(tmpoid.toLatin1().data()));

        status = true;
    }

    disconnect( bmv, SIGNAL( SelectedOid(const QString&) ), 0, 0);
    bmv->clearSelection();

    return status;
}

void MibSelection::bgrun(const QString& oid)
{
    bmv->SelectFromOid(oid);
    SetOidInfoType(oid); 
    SetSyntax();
    OKButtonPressed();

    QString tmpoid;
    if (node && (node->nodekind == SMI_NODEKIND_SCALAR))
        tmpoid = result_oid + ".0";
    else
        tmpoid = result_oid;

    vb.set_oid(Oid(tmpoid.toLatin1().data()));
}

