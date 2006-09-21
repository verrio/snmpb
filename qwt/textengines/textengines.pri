# -*- mode: sh -*- ###########################
# Qwt Widget Library
# Copyright (C) 1997   Josef Wilgen
# Copyright (C) 2002   Uwe Rathmann
#
# This library is free software; you can redistribute it and/or
# modify it under the terms of the Qwt License, Version 1.0
##############################################

TEMPLATE  = lib
CONFIG    += qt plugin
CONFIG    += warn_on thread plugin
CONFIG    += debug

QT += xml

MOC_DIR         = moc
OBJECTS_DIR     = obj
DESTDIR         = ../plugins/textengines
INCLUDEPATH    += ../../include

win32::DEFINES += QWT_DLL

unix:LIBS      += -L../../lib -lqwt
win32-msvc:LIBS  += ../../lib/qwt5.lib
win32-msvc.net:LIBS  += ../../lib/qwt5.lib
win32-msvc2005:LIBS += ../../lib/qwt5.lib
win32-g++:LIBS   += -L../../lib -lqwt5

#CONFIG    += debug_and_release
CONFIG    += debug

target.path = $$[QT_INSTALL_PLUGINS]/qwttextengines
INSTALLS += target

CONFIG(debug, debug|release) {
    unix: TARGET = $$join(TARGET,,,_debug)
    else: TARGET = $$join(TARGET,,d)
}
