# common stuff for all projects
include($$top_srcdir/recd2.pri)

TEMPLATE = lib

CONFIG += c++11

TARGET = udp-streamer
CONFIG += console
CONFIG -= app_bundle


SOURCES += \
    server.cpp \
    tcp-server.cpp \
    helper.cpp

HEADERS += \
    plugin-interface.h \
    server.h \
    tcp-server.h \
    helper.h

PRE_TARGETDEPS += \
    $$top_builddir/utils/libutils.a

DEFINES += "REQ_FLIP"
