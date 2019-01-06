# common stuff for everyone
include($$top_srcdir/recd2.pri)

TEMPLATE = lib
QT += multimedia # Audio Recorder
QT -= gui
LIBS += -lpthread
LIBS += -lasound
LIBS += -lopenal

SOURCES += main.cpp \
    qcapdev.cpp

HEADERS += \
    plugin-iface.h \
    qcapdev.h
