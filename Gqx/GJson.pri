INCLUDEPATH *= $$PWD

# По-хорошему, проверить бы здесь наличие файла msgpack.h - и в зависимости от него уже
# включать или не включать поддержку. Но пока мы не знаем, как.
#!exists(msgpack.h): {
#	message( "msgpack does not exists!" )
#} else {
#	message( "USING msgpack" )
#}

!win32: {
# LIBS *= -lrt
  !contains( DEFINES, GQX_JSON_NO_MSGPACK ): LIBS *= -lmsgpackc
}

SOURCES *= \
  $$PWD/GJson.cpp

HEADERS *= \
  $$PWD/GJson.h
