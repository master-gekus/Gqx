#-------------------------------------------------
#
# Project created by QtCreator 2016-02-21T10:13:45
#
#-------------------------------------------------

QT       += core gui

greaterThan(QT_MAJOR_VERSION, 4): QT += widgets

TARGET = JsonEditor
TEMPLATE = app


SOURCES += main.cpp\
        main_window.cpp

HEADERS  += main_window.h

FORMS    += main_window.ui

include(../../Gqx/GJson.pri)
include(../../Gqx/GUiHelpers.pri)
