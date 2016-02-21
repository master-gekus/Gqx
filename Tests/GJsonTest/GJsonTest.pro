#-------------------------------------------------
#
# Project created by QtCreator 2016-02-21T09:39:24
#
#-------------------------------------------------

QT       += testlib

QT       -= gui

TARGET = gjson_test
CONFIG   += console
CONFIG   -= app_bundle

TEMPLATE = app


SOURCES += gjson_test.cpp
DEFINES += SRCDIR=\\\"$$PWD/\\\"

include(../../Gqx/GJson.pri)
