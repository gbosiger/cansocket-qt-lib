QT = core cansocket
QT += cansocket

DEPENDPATH += $$PWD/../../../src/cansocket/
INCLUDEPATH += $$PWD/../../../include/CanSocket
LIBS += -L$$PWD/../../../lib/ -lCanSocket

CONFIG += console
CONFIG -= app_bundle

TARGET = datasteam
TEMPLATE = app

SOURCES += main.cpp \
    canrawreader.cpp

HEADERS += \
    canrawreader.h
