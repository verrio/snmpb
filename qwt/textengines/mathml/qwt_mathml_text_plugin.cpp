/* -*- mode: C++ ; c-file-style: "stroustrup" -*- *****************************
 * Qwt Widget Library
 * Copyright (C) 1997   Josef Wilgen
 * Copyright (C) 2003   Uwe Rathmann
 *
 * This library is free software; you can redistribute it and/or
 * modify it under the terms of the Qwt License, Version 1.0
 *****************************************************************************/

// vim: expandtab

#include <qobject.h>
#include <qwt_global.h>
#include <qwt_text.h>
#include <qwt_text_plugin.h>
#include "qwt_mathml_text_engine.h"

class QwtMathMLTextPlugin: public QObject, public QwtTextPlugin
{
    Q_OBJECT
    Q_INTERFACES(QwtTextPlugin)

public:
    virtual int format() const;
    virtual QwtTextEngine *engine() const;
};

int QwtMathMLTextPlugin::format() const 
{ 
    return QwtText::MathMLText; 
}

QwtTextEngine *QwtMathMLTextPlugin::engine() const 
{ 
    return new QwtMathMLTextEngine(); 
}

Q_EXPORT_PLUGIN2(mathml_plugin, QwtMathMLTextPlugin);
