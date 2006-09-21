# -*- mode: sh -*- ###########################
# Qwt Widget Library
# Copyright (C) 1997   Josef Wilgen
# Copyright (C) 2002   Uwe Rathmann
#
# This library is free software; you can redistribute it and/or
# modify it under the terms of the Qwt License, Version 1.0
##############################################

# pro file for the Qwt designer plugin

TEMPLATE        = lib
MOC_DIR         = moc
OBJECTS_DIR     = obj 
DESTDIR         = plugins/designer
INCLUDEPATH    += ../include 

win32::DEFINES += QWT_DLL

unix:LIBS      += -L../lib -lqwt
win32-msvc:LIBS  += ../lib/qwt5.lib
win32-msvc.net:LIBS  += ../lib/qwt5.lib
win32-msvc2005:LIBS += ../lib/qwt5.lib
win32-g++:LIBS   += -L../lib -lqwt5

# isEmpty(QT_VERSION) does not work with Qt-4.1.0/MinGW

VVERSION = $$[QT_VERSION]
isEmpty(VVERSION) {
    # Qt 3 
    TARGET    = qwtplugin
    CONFIG   += qt warn_on thread release plugin

    UI_DIR    = ui

    HEADERS  += qwtplugin.h
    SOURCES  += qwtplugin.cpp

    target.path = $(QTDIR)/plugins/designer
    INSTALLS += target

    IMAGES  += \
        pixmaps/qwtplot.png \
        pixmaps/qwtanalogclock.png \
        pixmaps/qwtcounter.png \
        pixmaps/qwtcompass.png \
        pixmaps/qwtdial.png \
        pixmaps/qwtknob.png \
        pixmaps/qwtscale.png \
        pixmaps/qwtslider.png \
        pixmaps/qwtthermo.png \
        pixmaps/qwtwheel.png \
        pixmaps/qwtwidget.png 

} else {

    # Qt 4

    TARGET    = qwt_designer_plugin
    CONFIG    += designer warn_on thread plugin 
    CONFIG    += release
    #CONFIG    += debug_and_release

    RCC_DIR   = resources

    HEADERS  += qwt_designer_plugin.h
    HEADERS  += qwt_designer_plotdialog.h
    SOURCES  += qwt_designer_plugin.cpp
    SOURCES  += qwt_designer_plotdialog.cpp
    RESOURCES += qwt_designer_plugin.qrc

    target.path = $$[QT_INSTALL_PLUGINS]/designer
    INSTALLS += target

    CONFIG(debug, debug|release) {
        unix: TARGET = $$join(TARGET,,,_debug)
        else: TARGET = $$join(TARGET,,d)
    }
}
