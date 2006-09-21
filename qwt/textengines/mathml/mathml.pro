# -*- mode: sh -*- ###########################
# Qwt Widget Library
# Copyright (C) 1997   Josef Wilgen
# Copyright (C) 2002   Uwe Rathmann
#
# This library is free software; you can redistribute it and/or
# modify it under the terms of the Qwt License, Version 1.0
##############################################

include( ../textengines.pri )

TARGET    = qwt_mathml_plugin

HEADERS = \
    qwt_mathml_text_engine.h \
    qwt_mathml_text_plugin.h

SOURCES = \
    qwt_mathml_text_engine.cpp \
    qwt_mathml_text_plugin.cpp

# The files below are modified code from the Qt Solutions package.
# Because of its license they can't be included into the Qwt package
# If you are allowed to use Qt Solutions, move the implementation of 
# the MmlDocument in qtmmlwidget.cpp to qtmmldocument.[h|cpp].
# With a small patch MathMl documents can be rotated, what is necessary
# for the titles of the vertical scales. Unfortunately I can't publish 
# this patch here.

HEADERS += qtmmldocument.h
SOURCES += qtmmldocument.cpp
