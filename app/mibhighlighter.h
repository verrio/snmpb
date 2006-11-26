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
