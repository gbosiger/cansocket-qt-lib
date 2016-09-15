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
    PRIVATE_HEADERS += $$PWD/canisotpsocket_p.h
    SOURCES += $$PWD/canisotpsocket.cpp
}

config_isotp {
    message("Including CAN ISO-TP protocol")
} else {
    message("Skipping CAN ISO-TP protocol")
}


HEADERS += $$PUBLIC_HEADERS $$PRIVATE_HEADERS \

