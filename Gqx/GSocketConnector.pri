QT *= network

INCLUDEPATH *= $$PWD

HEADERS *= \
  $$PWD/GSocketConnector.h \
  $$PWD/private/GSocketConnector_p.h \
  $$PWD/GSocketConnectorAbstractEngine.h \
  $$PWD/GSocketConnectorTcpSocketEngine.h \
  $$PWD/private/GSocketConnectorTcpSocketEngine_p.h

SOURCES *= \
  $$PWD/GSocketConnector.cpp \
  $$PWD/GSocketConnectorAbstractEngine.cpp \
  $$PWD/GSocketConnectorTcpSocketEngine.cpp

