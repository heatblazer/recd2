TEMPLATE = lib
CONFIG += console c++11
CONFIG -= app_bundle
CONFIG -= qt

LIBS += -lpthread
LIBS += -lasound


SOURCES += main.cpp \
    thread.cpp \
    alsarec.cpp

HEADERS += \
    plugin-iface.h \
    thread.h \
    alsarec.h
