TEMPLATE = app
CONFIG += console c++11
CONFIG -= app_bundle
CONFIG -= qt

LIBS += -lasound
LIBS += -lpthread

SOURCES += main.cpp \
    thread.cpp

HEADERS += \
    plugin-iface.h \
    thread.h \
    alsarec.h
