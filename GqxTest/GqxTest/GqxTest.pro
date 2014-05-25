#-------------------------------------------------
#
# Project created by QtCreator 2014-04-29T18:32:52
#
#-------------------------------------------------

QT       += core gui

greaterThan(QT_MAJOR_VERSION, 4): QT += widgets

TARGET = GqxTest
TEMPLATE = app

!win32: LIBS += -lrt -lmsgpackc
# -lzmq -lssl -lcrypto -lboost_system -lSTRATFORD -lVolArb

SOURCES += main.cpp\
		MainWindow.cpp

HEADERS  += MainWindow.h

FORMS    += MainWindow.ui

include( ../../Gqx/Gqx.pri )
