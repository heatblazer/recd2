# commnon stuff for everyone
include($$top_srcdir/recd2.pri)

CONFIG += c++11

TARGET = recd2
CONFIG += console
CONFIG -= app_bundle

RESOURCES += \
    hello_app.qrc


TEMPLATE = app

LIBS += -ldl

SOURCES += main.cpp \
    sapplication.cpp \
    unix/daemon.cpp \
    plugin-manager.cpp \
    utils/json-writer.cpp \
    defs.cpp \
    server.cpp \
    utils/stack-tracer.cpp

HEADERS += \
    sapplication.h \
    types.h \
    unix/daemon.h \
    plugin-manager.h \
    protocols.h \
    defs.h \
    utils/json-writer.h \
    server.h \
    utils/stack-tracer.h


QMAKE_CFLAGS += \
    -Wextra \
    -pedantic \
    -Wmissing-braces \
    -Wparentheses \
    -Wsequence-point \
    -Wswitch \
    -Wuninitialized \
    -Wcast-qual \
    -Wlogical-op \
    -Wnormalized=nfkc \
    -std=gnu11

# custom macrodefs for the tests
QMAKE_CXXFLAGS +=  -DREQUIRE_FLIP_CHANNS_SAMPLES \
                    -DUNSAFE_CONFIG \
                   # -DMAIN_TEST \
                   #-DHEARTATTACK \            # flood the sender for testing packet lost
                   # -DEXPERIMENTAL_WAV \   # test wav renaming
                   #-DPLUGIN_TEST \         # test plugin interface
                   #-DTEST     \
                  #-DXML_TEST  \            # test configuration - unused


# redefine a Makefile INSTALL_PROGRAM variable. We want to instal wit SUID bit
QMAKE_INSTALL_PROGRAM = install -m 4755 -p

target.path = /usr/bin
INSTALLS += target

debug
{
    message("Debug mode")
    QMAKE_CXXFLAGS += -DRECD_DEBUG
}

DISTFILES += \
    ../resources/30-31-13:51:39.wav \
    ../resources/31-32-13:51:39.wav \
    ../resources/usesEntireiXMLSpec.WAV \
    ../resources/wav1 \
    ../resources/wav2 \
    ../recd2.pri

PRE_TARGETDEPS += \
    $$top_builddir/utils/libutils.a
