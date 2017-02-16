# commnon stuff for everyone
include($$top_srcdir/recd2.pri)


QT += core
QT += gui
QT += widgets

TEMPLATE = lib
CONFIG += console c++11
CONFIG -= app_bundle


SOURCES += main.cpp \
    dtmf.cpp \
    DtmfDetector.cpp \
    DtmfGenerator.cpp

HEADERS += \
    plugin-interface.h \
    dtmf.h \
    DtmfDetector.hpp \
    DtmfGenerator.hpp \
    types_cpp.hpp

