QT += core
QT -= gui

CONFIG += c++11

TARGET = recorder
CONFIG += console
CONFIG -= app_bundle

TEMPLATE = lib

SOURCES += main.cpp \
    recorder.cpp

HEADERS += \
    recorder.h \
    types.h \
    plugin-iface.h

win32:CONFIG(release, debug|release): LIBS += -L$$OUT_PWD/../utils/release/ -lutils
else:win32:CONFIG(debug, debug|release): LIBS += -L$$OUT_PWD/../utils/debug/ -lutils
else:unix: LIBS += -L$$OUT_PWD/../utils/ -lutils

INCLUDEPATH += $$PWD/../utils
DEPENDPATH += $$PWD/../utils
