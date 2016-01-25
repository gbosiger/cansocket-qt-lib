QT += core cansocket

DEPENDPATH += $$PWD/../../../src/cansocket/
INCLUDEPATH += $$PWD/../../../include/CanSocket
LIBS += -L$$PWD/../../../lib/ -lCanSocket

CONFIG += console
CONFIG -= app_bundle

TARGET = canisotpsocket
TEMPLATE = app

HEADERS += \
    canisotpreader.h

SOURCES += main.cpp \
    canisotpreader.cpp
