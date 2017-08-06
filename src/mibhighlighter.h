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

#ifndef MIBHIGHLIGHTER_H
#define MIBHIGHLIGHTER_H

#include <QSyntaxHighlighter>
#include <QTextCharFormat>
#include <QHash>

class QTextDocument;

class MibHighlighter : public QSyntaxHighlighter
{
    Q_OBJECT

public:
    MibHighlighter(QTextDocument *parent = 0);

protected:
    void highlightBlock(const QString &text);

private:
    struct MibHighlightingRule
    {
        QRegExp pattern;
        QTextCharFormat format;
    };

    QVector<MibHighlightingRule> rules;

    QRegExp string_start;
    QRegExp string_end;

    QTextCharFormat reserved_word;
    QTextCharFormat keyword;
    QTextCharFormat number;
    QTextCharFormat comment;
    QTextCharFormat string;
    QTextCharFormat character;
    QTextCharFormat enumeration;
};

#endif
