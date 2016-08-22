#-------------------------------------------------
#
# Project created by QtCreator 2016-02-21T10:31:45
#
#-------------------------------------------------

QT       += core network
QT       -= gui

TARGET = simple_server
CONFIG   += console
CONFIG   -= app_bundle

TEMPLATE = app

SOURCES += \
  main.cpp \
  app.cpp \
  client_servicing_thread.cpp

HEADERS += \
  logging.h \
  app.h \
  client_servicing_thread_p.h \
  client_servicing_thread.h

unix: {
  QMAKE_CXXFLAGS *= -std=c++11
}

include(../../../Gqx/GLogger.pri)
include(../../../Gqx/GSelfOwnedThread.pri)

DEFINES *= GIT_DESCRIBE=\\\"$$system(git describe --long --always)\\\"
