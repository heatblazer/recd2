# common stuff for everyone
include($$top_srcdir/recd2.pri)

TEMPLATE = lib
CONFIG += console c++11
CONFIG -= app_bundle

QT += core

SOURCES += \
    null.cpp

HEADERS += \
    plugin-iface.h

PRE_TARGETDEPS += \
    $$top_builddir/utils/libutils.a
