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
  app.cpp

HEADERS += \
  main_window.h \
  app.h

FORMS += \
  main_window.ui

RESOURCES += \
  resources.qrc

RC_FILE += \
  resources.rc

unix: {
  QMAKE_CXXFLAGS *= -std=c++11
}

win32: {
  DEFINES *= Q_COMPILER_INITIALIZER_LISTS
}

include(../../../Gqx/GUiHelpers.pri)

DEFINES *= GIT_DESCRIBE=\\\"$$system(git describe --long --always)\\\"
