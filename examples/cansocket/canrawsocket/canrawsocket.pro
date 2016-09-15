QT += core cansocket

TARGET = canrawsocket
TEMPLATE = app

HEADERS += \
    canrawreader.h

SOURCES += main.cpp \
    canrawreader.cpp


target.path = $$[QT_INSTALL_EXAMPLES]/cansocket/canrawsocket
INSTALLS += target

