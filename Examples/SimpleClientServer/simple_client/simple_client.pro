#-------------------------------------------------
#
# Project created by QtCreator 2016-02-21T10:33:41
#
#-------------------------------------------------

QT       += core gui

greaterThan(QT_MAJOR_VERSION, 4): QT += widgets

TARGET = simple_client
TEMPLATE = app

SOURCES += \
  main.cpp \
  main_window.cpp \
  app.cpp \
  about_box.cpp \
  tcp_connector.cpp

HEADERS += \
  main_window.h \
  app.h \
  about_box.h \
  tcp_connector.h \
  tcp_connector_p.h

FORMS += \
  main_window.ui \
  about_box.ui

RESOURCES += \
  resources.qrc

RC_FILE += \
  resources.rc

unix: {
  QMAKE_CXXFLAGS *= -std=c++11
}

include(../../../Gqx/GUiHelpers.pri)
include(../../../Gqx/GIdleHandler.pri)

DEFINES *= GIT_DESCRIBE=\\\"$$system(git describe --long --always)\\\"
