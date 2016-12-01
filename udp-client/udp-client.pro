# remove that project later when we no longer need
# test to the server
QT += core
QT -= gui
QT += network

CONFIG += c++11

TARGET = udp-client
CONFIG += console
CONFIG -= app_bundle

TEMPLATE = app

SOURCES += main.cpp \
    client.cpp

HEADERS += \
    client.h

