# shared stuff for all projects
include($$top_srcdir/recd2.pri)

TEMPLATE = lib # it`s a static library!!!

QT += core
QT -= gui

CONFIG += c++11

TARGET = utils
CONFIG += console
CONFIG += staticlib
CONFIG -= app_bundle

SOURCES += main.cpp \
    logger.cpp \
    qwave-writer.cpp \
    recorder-config.cpp \
    wav-writer.cpp \
    writer.cpp \
    ipc-msg.cpp

HEADERS += \
    date-time.h \
    logger.h \
    qwave-writer.h \
    recorder-config.h \
    wav-writer-iface.h \
    wav-writer.h \
    writer.h \
    utils.h \
    types.h \
    ipc-msg.h

#QMAKE_CXXFLAGS += -fPIC
