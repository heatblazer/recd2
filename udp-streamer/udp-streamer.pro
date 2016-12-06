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
    server.h

win32:CONFIG(release, debug|release): LIBS += -L$$OUT_PWD/../utils/release/ -lutils
else:win32:CONFIG(debug, debug|release): LIBS += -L$$OUT_PWD/../utils/debug/ -lutils
else:unix: LIBS += -L$$OUT_PWD/../utils/ -lutils

INCLUDEPATH += $$PWD/../utils
DEPENDPATH += $$PWD/../utils
