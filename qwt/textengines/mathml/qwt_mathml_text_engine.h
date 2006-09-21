/* -*- mode: C++ ; c-file-style: "stroustrup" -*- *****************************
 * Qwt Widget Library
 * Copyright (C) 1997   Josef Wilgen
 * Copyright (C) 2003   Uwe Rathmann
 *
 * This library is free software; you can redistribute it and/or
 * modify it under the terms of the Qwt License, Version 1.0
 *****************************************************************************/

// vim: expandtab

#ifndef QWT_MATHML_TEXT_ENGINE_H
#define QWT_MATHML_TEXT_ENGINE_H 1

#include "qwt_text_engine.h"

class QWT_EXPORT QwtMathMLTextEngine: public QwtTextEngine
{
public:
    QwtMathMLTextEngine();
    virtual ~QwtMathMLTextEngine();

    virtual int heightForWidth(const QFont &font, int flags, 
        const QString &text, int width) const;

    virtual QSize textSize(const QFont &font, int flags,
        const QString &text) const;

    virtual void draw(QPainter *painter, const QRect &rect,
        int flags, const QString &text) const;

    virtual bool mightRender(const QString &) const;

    virtual void textMargins(const QFont &, const QString &,
        int &left, int &right, int &top, int &bottom) const;
};

#endif
