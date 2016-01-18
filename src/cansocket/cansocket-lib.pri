INCLUDEPATH += $$PWD

PUBLIC_HEADERS += \
    $$PWD/cansocketglobal.h \
    $$PWD/canabstractsocket.h \
    $$PWD/canframe.h \
    $$PWD/canrawsocket.h

PRIVATE_HEADERS += \
    $$PWD/canabstractsocket_p.h \
    $$PWD/canframe_p.h \
    $$PWD/canrawsocket_p.h

SOURCES += \
    $$PWD/canabstractsocket.cpp \
    $$PWD/canframe.cpp \
    $$PWD/canrawsocket.cpp

config_isotp {
    PUBLIC_HEADERS += $$PWD/canisotpsocket.h
}

HEADERS += $$PUBLIC_HEADERS $$PRIVATE_HEADERS \


