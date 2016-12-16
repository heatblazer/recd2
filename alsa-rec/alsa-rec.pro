# common stuff for everyone
include($$top_srcdir/recd2.pri)

TEMPLATE = lib

LIBS += -lpthread
LIBS += -lasound
LIBS += -lopenal

SOURCES += main.cpp \
    thread.cpp \
    alsarec.cpp

HEADERS += \
    plugin-iface.h \
    thread.h \
    alsarec.h
