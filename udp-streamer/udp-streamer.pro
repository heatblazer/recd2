TEMPLATE = lib

QT += core
QT -= gui
QT += network

CONFIG += c++11

TARGET = udp-streamer
CONFIG += console
CONFIG -= app_bundle


SOURCES += main.cpp \
    server.cpp

HEADERS += \
    plugin-interface.h \
    server.h \
    date-time.h
