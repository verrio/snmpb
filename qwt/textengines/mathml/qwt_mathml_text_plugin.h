/* -*- mode: C++ ; c-file-style: "stroustrup" -*- *****************************
 * Qwt Widget Library
 * Copyright (C) 1997   Josef Wilgen
 * Copyright (C) 2003   Uwe Rathmann
 *
 * This library is free software; you can redistribute it and/or
 * modify it under the terms of the Qwt License, Version 1.0
 *****************************************************************************/

// vim: expandtab

#ifndef QWT_MATHML_TEXT_PLUGIN_H
#define QWT_MATHML_TEXT_PLUGIN_H 1

#include <qobject.h>
#include <qwt_text_plugin.h>

class QwtMathMLTextPlugin : public QObject, public QwtTextPlugin
{
    Q_OBJECT
    Q_INTERFACES(QwtTextPlugin)

public:
    virtual int format() const;
    virtual QwtTextEngine *engine() const;
};

#endif
