# common stuff for everyone
include($$top_srcdir/recd2.pri)

TEMPLATE = lib
CONFIG += console c++11
CONFIG -= app_bundle

QT += core

SOURCES += \
    md5-generator.cpp

HEADERS += \
    plugin-iface.h \
    md5-generator.h

PRE_TARGETDEPS += \
    $$top_builddir/utils/libutils.a
