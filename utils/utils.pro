# shared stuff for all projects
include($$top_srcdir/recd2.pri)

TEMPLATE = lib # it`s a static library!!!

QT += core
QT -= gui

CONFIG += c++11

TARGET = utils
CONFIG += console
CONFIG += staticlib     # makes it an .a static lib!!!
CONFIG -= app_bundle

SOURCES += main.cpp \
    logger.cpp \
    recorder-config.cpp \
    types.cpp \
    wav-writer.cpp \
    writer.cpp \
    ipc-msg.cpp \
    thread.cpp

HEADERS += \
    date-time.h \
    logger.h \
    recorder-config.h \
    wav-writer-iface.h \
    wav-writer.h \
    writer.h \
    utils.h \
    types.h \
    ipc-msg.h \
    thread.h \
    ring-buffer.h

#QMAKE_CXXFLAGS += -fPIC
