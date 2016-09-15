QT += core cansocket

TARGET = canisotpsocket
TEMPLATE = app

HEADERS += \
    canisotpreader.h

SOURCES += main.cpp \
    canisotpreader.cpp

target.path = $$[QT_INSTALL_EXAMPLES]/cansocket/canisotpsocket
INSTALLS += target
