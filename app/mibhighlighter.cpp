/*
    Copyright (C) 2004-2010  Martin Jolicoeur (snmpb1@gmail.com) 

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
#include <QtGui>
#include "mibhighlighter.h"

MibHighlighter::MibHighlighter(QTextDocument *parent): QSyntaxHighlighter(parent)
{
    MibHighlightingRule rule;

    reserved_word.setForeground(Qt::darkBlue);
    reserved_word.setFontWeight(QFont::Bold);
    QStringList reserved_words;
    reserved_words 
        << "\\bACCESS\\b"
        << "\\bAGENT-CAPABILITIES\\b"
        << "\\bAPPLICATION\\b"
        << "\\bAUGMENTS\\b"
        << "\\bBEGIN\\b"
        << "\\bBITS\\b"
        << "\\bCONTACT-INFO\\b"
        << "\\bCREATION-REQUIRES\\b"
        << "\\bDEFINITIONS\\b"
        << "\\bDEFVAL\\b"
        << "\\bDESCRIPTION\\b"
        << "\\bDISPLAY-HINT\\b"
        << "\\bEND\\b"
        << "\\bENTERPRISE\\b"
        << "\\bEXTENDS\\b"
        << "\\bFROM\\b"
        << "\\bGROUP\\b"
        << "\\bIDENTIFIER\\b"
        << "\\bIMPLICIT\\b"
        << "\\bIMPLIED\\b"
        << "\\bIMPORTS\\b"
        << "\\bINCLUDES\\b"
        << "\\bINDEX\\b"
        << "\\bINSTALL-ERRORS\\b"
        << "\\bINTEGER\\b"
        << "\\bLAST-UPDATED\\b"
        << "\\bMANDATORY-GROUPS\\b"
        << "\\bMAX-ACCESS\\b"
        << "\\bMIN-ACCESS\\b"
        << "\\bMODULE\\b"
        << "\\bMODULE-COMPLIANCE\\b"
        << "\\bMODULE-IDENTITY\\b"
        << "\\bNOTIFICATION-GROUP\\b"
        << "\\bNOTIFICATION-TYPE\\b"
        << "\\bNOTIFICATIONS\\b"
        << "\\bOBJECT\\b"
        << "\\bOBJECT-GROUP\\b"
        << "\\bOBJECT-IDENTITY\\b"
        << "\\bOBJECT-TYPE\\b"
        << "\\bOBJECTS\\b"
        << "\\bOCTET\\b"
        << "\\bOF\\b"
        << "\\bORGANIZATION\\b"
        << "\\bPIB-ACCESS\\b"
        << "\\bPIB-DEFINITIONS\\b"
        << "\\bPIB-INDEX\\b"
        << "\\bPIB-MIN-ACCESS\\b"
        << "\\bPIB-REFERENCES\\b"
        << "\\bPIB-TAG\\b"
        << "\\bPOLICY-ACCESS\\b"
        << "\\bPRODUCT-RELEASE\\b"
        << "\\bREFERENCE\\b"
        << "\\bREVISION\\b"
        << "\\bSEQUENCE\\b"
        << "\\bSIZE\\b"
        << "\\bSTATUS\\b"
        << "\\bSTRING\\b"
        << "\\bSUBJECT-CATEGORIES\\b"
        << "\\bSUPPORTS\\b"
        << "\\bSYNTAX\\b"
        << "\\bTEXTUAL-CONVENTION\\b"
        << "\\bTRAP-TYPE\\b"
        << "\\bUNIQUENESS\\b"
        << "\\bUNITS\\b"
        << "\\bUNIVERSAL\\b"
        << "\\bVALUE\\b"
        << "\\bVARIABLES\\b"
        << "\\bVARIATION\\b"
        << "\\bWRITE-SYNTAX\\b";

    foreach (QString pattern, reserved_words)
    {
        rule.pattern = QRegExp(pattern);
        rule.format = reserved_word;
        rules.append(rule);
    }

    keyword.setForeground(Qt::darkYellow);
    keyword.setFontWeight(QFont::Bold);
    QStringList keywords;
    keywords 
        << "\\baccessible-for-notify\\b"
        << "\\bCounter\\b"
        << "\\bCounter32\\b"
        << "\\bCounter64\\b"
        << "\\bcurrent\\b"
        << "\\bdeprecated\\b"
        << "\\bGauge\\b"
        << "\\bGauge32\\b"
        << "\\bInteger32\\b"
        << "\\bIpAddress\\b"
        << "\\bmandatory\\b"
        << "\\bNetworkAddress\\b"
        << "\\bnot-accessible\\b"
        << "\\bobsolete\\b"
        << "\\bOpaque\\b"
        << "\\boptional\\b"
        << "\\bread-create\\b"
        << "\\bread-only\\b"
        << "\\bread-write\\b"
        << "\\bTimeTicks\\b"
        << "\\bUnsigned32\\b"
        << "\\bwrite-only\\b"
        << "\\binstall\\b"
        << "\\binstall-notify\\b"
        << "\\bnotify\\b"
        << "\\breport-only\\b"
        << "\\bnot-implemented\\b"
        << "\\bReferenceId\\b"
        << "\\bTagId\\b"
        << "\\bTagReferenceId\\b"
        << "\\bInteger64\\b"
        << "\\bUnsigned64\\b";

    foreach (QString pattern, keywords)
    {
        rule.pattern = QRegExp(pattern);
        rule.format = keyword;
        rules.append(rule);
    }

    number.setForeground(Qt::darkRed);
    rule.pattern = QRegExp("\\b([1-9][0-9]*|0)\\b");
    rule.format = number;
    rules.append(rule);

    comment.setFontItalic(true);
    comment.setForeground(Qt::red);
    rule.pattern = QRegExp("--[^\n]*");
    rule.format = comment;
    rules.append(rule);

    character.setForeground(Qt::magenta);
    rule.pattern = QRegExp("'.*'");
    rule.format = character;
    rules.append(rule);

    enumeration.setForeground(Qt::blue);
    rule.pattern = QRegExp("\\b[A-Za-z0-9_]+(?=\\()");
    rule.format = enumeration;
    rules.append(rule);

    string.setForeground(Qt::darkGreen);
    string_start = QRegExp("\"");
    string_end = QRegExp("\"");
}

void MibHighlighter::highlightBlock(const QString &text)
{
    foreach (MibHighlightingRule rule, rules)
    {
        QRegExp expression(rule.pattern);
        int index = text.indexOf(expression);

        while (index >= 0)
        {
            int length = expression.matchedLength();
            setFormat(index, length, rule.format);
            index = text.indexOf(expression, index + length);
        }
    }

    setCurrentBlockState(0);

    int start_index = 0, end_index = 0, offset = 0, string_length = 0;
    
    if (previousBlockState() != 1)
    {
        start_index = text.indexOf(string_start);
        offset = 1;
    }

    while (start_index >= 0)
    {
        end_index = text.indexOf(string_end, start_index + offset);

        if (end_index == -1)
        {
            setCurrentBlockState(1);
            string_length = text.length() - start_index;
        }
        else
        {
            string_length = end_index - start_index
                            + string_end.matchedLength();
        }

        setFormat(start_index, string_length, string);
        start_index = text.indexOf(string_start,
                                   start_index + string_length);

        if (previousBlockState() != 1) offset = 1;
        else
            if (currentBlockState() == 1) offset = 0;
    }
}

