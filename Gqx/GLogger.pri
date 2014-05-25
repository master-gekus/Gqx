INCLUDEPATH *= $$PWD

SOURCES *= \
	$$PWD/GLogger.cpp \
	$$PWD/GLoggerEvent.cpp

HEADERS *= \
	$$PWD/GLogger.h \
	$$PWD/GLoggerEvent.h \
	$$PWD/private/GLogger_p.h \
	$$PWD/private/GLoggerEvent_p.h

include( GSelfOwnedThread.pri )
