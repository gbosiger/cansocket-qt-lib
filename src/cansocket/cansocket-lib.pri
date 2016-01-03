INCLUDEPATH += $$PWD

PUBLIC_HEADERS += \
    $$PWD/cansocketglobal.h \
    $$PWD/canabstractsocket.h \
    $$PWD/canrawsocket.h \
    $$PWD/canframe.h


PRIVATE_HEADERS += \
    $$PWD/canabstractsocket_p.h \
    $$PWD/canrawsocket_p.h \
    $$PWD/canframe_p.h

SOURCES += \
    $$PWD/canrawsocket.cpp \
    $$PWD/canabstractsocket.cpp \
    $$PWD/canframe.cpp

HEADERS += $$PUBLIC_HEADERS $$PRIVATE_HEADERS \


