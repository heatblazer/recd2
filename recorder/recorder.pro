QT += core
QT -= gui

CONFIG += c++11

TARGET = recorder
CONFIG += console
CONFIG -= app_bundle

TEMPLATE = app

SOURCES += main.cpp \
    utils/logger.cpp \
    utils/qwave-writer.cpp \
    utils/recorder-config.cpp \
    utils/wav-writer.cpp \
    utils/writer.cpp \
    config.cpp \
    recorder.cpp

HEADERS += \
    unix/date-time.h \
    utils/logger.h \
    utils/qwave-writer.h \
    utils/recorder-config.h \
    utils/wav-writer-iface.h \
    utils/wav-writer.h \
    utils/writer.h \
    config.h \
    recorder.h \
    types.h
