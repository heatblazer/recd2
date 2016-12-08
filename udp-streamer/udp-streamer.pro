# common stuff for all projects
include($$top_srcdir/recd2.pri)

TEMPLATE = lib

CONFIG += c++11

TARGET = udp-streamer
CONFIG += console
CONFIG -= app_bundle


SOURCES += main.cpp \
    server.cpp

HEADERS += \
    plugin-interface.h \
    server.h

PRE_TARGETDEPS += \
    $$top_builddir/utils/libutils.a
