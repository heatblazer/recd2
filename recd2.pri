#
# Common settings for all supprojects.
# You must inclue this in all .pro files.
#
QT += core
QT += script
QT += xml
QT += network
QT -= gui

CONFIG += c++11 strict_c++  # enable cpp11 support.


CONFIG (debug, debug|release) {
    CONFIG += separate_debug_info   # make debug_info only in debug mode.
}

QMAKE_CXXFLAGS  += \
    -Wextra \
    -pedantic \
    -Wmissing-braces \
    -Wparentheses \
    -Wsequence-point \
    -Wswitch \
    -Wuninitialized \
    -Wcast-qual \
    -Wlogical-op \
    -Wnormalized=nfkc

INCLUDEPATH += \
    $$top_srcdir/utils

LIBS += \
    -L$$top_builddir/utils -lutils

# will use  it later just hint me
#RADIS_INSTALL_PATH = "/opt/radis-ui"
