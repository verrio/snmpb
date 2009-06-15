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
#ifndef MIBSELECTION_H
#define MIBSELECTION_H

#include <QValidator>
#include <QDialog>
#include <QDialogButtonBox>

#include "snmpb.h"
#include "snmp_pp/snmp_pp.h"

// Class used to validate set operations on Counter32/Unsigned32/Gauge32/Integer...
class IntValidator: public QValidator
{
public:
    IntValidator(bool is_unsigned, QObject *parent = 0): QValidator(parent)
    {
        is_uint = is_unsigned;
    }

    // -2147483648 .. 4294967295
    State validate (QString& input, int& pos) const
    {
        QRegExp r("\\-?\\d{0,10}");
        if (r.exactMatch(input))
        {
            bool ok;
            if (is_uint == true)
            {
                unsigned int uval = input.toUInt(&ok);
                if ( ((ok == true) && (uval <= 4294967295U)) || 
                     ((ok == false) && (!input.size())) )
                    return Acceptable;
                else
                    return Invalid;
            }
            else
            {
                int val = input.toInt(&ok);
                if ( ((ok == true) && (val >= (-2147483647-1)) && 
                                      (val <= 2147483647)) || 
                     ((ok == false) && (!input.size() || (input.size() == 1))) )
                    return Acceptable;
                else
                    return Invalid;
            }
        }
        else
        {
            if (const_cast<QRegExp &>(r).matchedLength() == input.size())
                return Intermediate;
            else
            {
                pos = input.size();
                return Invalid;
            }
        }
    }

private:
    bool is_uint;
};

// Class used to validate set operations on Counter64
class UInt64Validator: public QValidator
{
public:
    UInt64Validator(QObject *parent = 0): QValidator(parent) {}

    State validate (QString& input, int& pos) const
    {
        QRegExp r("\\-?\\d{0,20}");
        if (r.exactMatch(input))
        {
            bool ok;
            unsigned long long uval = input.toULongLong(&ok);
            if ( ((ok == true) && (uval <= 0xFFFFFFFFFFFFFFFFULL)) || 
                 ((ok == false) && (!input.size())) )
                return Acceptable;
            else
                return Invalid;
        }
        else
        {
            if (const_cast<QRegExp &>(r).matchedLength() == input.size())
                return Intermediate;
            else
            {
                pos = input.size();
                return Invalid;
            }
        }
    }
};

class MibSelection: public QObject
{
    Q_OBJECT
    
public:
    MibSelection(Snmpb *snmpb, QString title);
    ~MibSelection();

    bool run(const QString& init_oid = "", int init_syntax = -1, 
             const QString& init_val = "");
    void bgrun(const QString& oid);

    Vb *GetVarbind(void);
    QString GetSyntaxName(void);
    int GetSyntax(void);
    QString GetOid(void);
    QString GetValue(void);
    QString GetName(void);
    SmiNode *GetNode(void);

public slots:
    void GetValueCb(int index);
    void GetValueLe(void);
    void GetOidLe(void);
    void GetSelectedOid(const QString& oid);
    void GetSyntaxCb(int index);
    void OKButtonPressed(void);

protected:
    void SetOidInfoType(const QString& oid);
    void SetValueWidget(void);
    void SetSyntax(int st = -1);

private:
    Snmpb *s;

    Vb vb;
    SmiType *type;  
    QString syntax;
    SmiNode *node;

    QGridLayout *gl;
    QLabel *oidlabel;
    QLabel *syntaxlabel;
    QLabel *vallabel;
    QLabel *infolabel;
    QDialogButtonBox *box;
    QDialog *dprompt; 
    QComboBox *val_cb;
    QComboBox *syntax_cb;
    QLineEdit *val_le;
    QLineEdit *oid_le;
    BasicMibView *bmv;
 
    QString result_string;
    QString result_oid;
    int     result_syntax;

    IntValidator *validator;
    UInt64Validator *validator64;
};

#endif /* MIBSELECTION_H */
