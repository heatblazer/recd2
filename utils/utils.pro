QT += core
QT -= gui

CONFIG += c++11

TARGET = utils
CONFIG += console
CONFIG -= app_bundle

TEMPLATE = lib

SOURCES += main.cpp \
    logger.cpp \
    qwave-writer.cpp \
    recorder-config.cpp \
    wav-writer.cpp \
    writer.cpp

HEADERS += \
    date-time.h \
    logger.h \
    qwave-writer.h \
    recorder-config.h \
    wav-writer-iface.h \
    wav-writer.h \
    writer.h
