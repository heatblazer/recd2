#common settings for everyone
include($$top_srcdir/recd2.pri)


TARGET = recorder
CONFIG += console
CONFIG -= app_bundle

TEMPLATE = lib # it`s a plugin

SOURCES += main.cpp \
    recorder.cpp

HEADERS += \
    recorder.h \
    types.h \
    plugin-iface.h

PRE_TARGETDEPS += \
    $$top_builddir/utils/libutils.a
