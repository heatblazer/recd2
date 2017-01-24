# commnon stuff for everyone
include($$top_srcdir/recd2.pri)


QT += core
QT += gui
QT += widgets

# use the kiss or the default code from "C numerical algorithms"
TEMPLATE = lib
CONFIG += console c++11
CONFIG -= app_bundle



SOURCES += main.cpp \
    External/tools/fftutil.c \
    External/tools/kfc.c \
    External/tools/kiss_fastfir.c \
    External/tools/kiss_fftnd.c \
    External/tools/kiss_fftndr.c \
    External/tools/kiss_fftr.c \
    External/tools/psdpng.c \
    External/kiss_fft.c \
    iz_fft.cpp

HEADERS += \
    plugin-interface.h \
    External/tools/kfc.h \
    External/tools/kiss_fftnd.h \
    External/tools/kiss_fftndr.h \
    External/tools/kiss_fftr.h \
    External/_kiss_fft_guts.h \
    External/kiss_fft.h \
    External/kissfft.hh \
    iz_fft.h

DISTFILES += \
    External/TIPS \
    External/CHANGELOG \
    External/COPYING \
    External/README \
    External/README.simd
