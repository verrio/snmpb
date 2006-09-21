TEMPLATE     = app
CONFIG       += qt

CONFIG      += thread
CONFIG      += warn_on
CONFIG      += release

MOC_DIR      = moc
OBJECTS_DIR  = obj
INCLUDEPATH += ../../include
DEPENDPATH  += ../../include

unix:LIBS        += -L../../lib -lqwt

win32:DEFINES    += QT_DLL QWT_DLL
win32-msvc:LIBS  += ../../lib/qwt5.lib
win32-msvc.net:LIBS  += ../../lib/qwt5.lib
win32-msvc2005:LIBS += ../../lib/qwt5.lib
win32-g++:LIBS   += -L../../lib -lqwt5

# DEFINES          += QT_NO_CAST_ASCII
# DEFINES          += QT_NO_CAST_TO_ASCII

