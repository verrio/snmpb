/* -*- mode: C++ ; c-file-style: "stroustrup" -*- *****************************
 * Qwt Widget Library
 * Copyright (C) 1997   Josef Wilgen
 * Copyright (C) 2003   Uwe Rathmann
 *
 * This library is free software; you can redistribute it and/or
 * modify it under the terms of the Qwt License, Version 1.0
 *****************************************************************************/

// vim: expandtab

#include <qstring.h>
#include <qpainter.h>
#include "qwt_mathml_text_engine.h"

#include <qtmmldocument.h>

QwtMathMLTextEngine::QwtMathMLTextEngine()
{
}

QwtMathMLTextEngine::~QwtMathMLTextEngine()
{
}

int QwtMathMLTextEngine::heightForWidth(const QFont& font, int flags,
        const QString& text, int) const
{
    return textSize(font, flags, text).height();
}

QSize QwtMathMLTextEngine::textSize(const QFont &font,
    int, const QString& text) const
{
    static QString t;
    static QSize sz;

    if ( text != t )
    {
        MmlDocument doc;
        doc.setContent(text);
        doc.setBaseFontPointSize(font.pointSize());

        sz = doc.size();
        t = text;
    }

    return sz;
}

void QwtMathMLTextEngine::textMargins(const QFont &, const QString &,
    int &left, int &right, int &top, int &bottom) const
{
    left = right = top = bottom = 0;
}

void QwtMathMLTextEngine::draw(QPainter *painter, const QRect &rect,
    int flags, const QString& text) const
{
    MmlDocument doc;
    doc.setContent(text);
    doc.setBaseFontPointSize(painter->font().pointSize());

    const QSize docSize = doc.size();

    QPoint pos = rect.topLeft();
    if ( rect.width() > docSize.width() )
    {
        if ( flags & Qt::AlignRight )
            pos.setX(rect.right() - docSize.width());
        if ( flags & Qt::AlignHCenter )
            pos.setX(rect.center().x() - docSize.width() / 2);
    }
    if ( rect.height() > docSize.height() )
    {
        if ( flags & Qt::AlignBottom )
            pos.setY(rect.bottom() - docSize.height());
        if ( flags & Qt::AlignVCenter )
            pos.setY(rect.center().y() - docSize.height() / 2);
    }

    doc.paint(painter, pos);
}

bool QwtMathMLTextEngine::mightRender(const QString &text) const
{
    return text.startsWith("<math>");
}
