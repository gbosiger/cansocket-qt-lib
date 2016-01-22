QT += core cansocket

DEPENDPATH += $$PWD/../../../src/cansocket/
INCLUDEPATH += $$PWD/../../../include/CanSocket
LIBS += -L$$PWD/../../../lib/ -lCanSocket

CONFIG += console
CONFIG -= app_bundle

TARGET = canrawsocket
TEMPLATE = app

HEADERS += \
    canrawreader.h

SOURCES += main.cpp \
    canrawreader.cpp
