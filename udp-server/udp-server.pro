QT += core
QT += xml
QT += network
QT -= gui

CONFIG += c++11

TARGET = recd2
CONFIG += console
CONFIG -= app_bundle

TEMPLATE = app

LIBS += -ldl

SOURCES += main.cpp \
    server.cpp \
    sapplication.cpp \
    utils/recorder-config.cpp \
    recorder.cpp \
    unix/daemon.cpp \
    utils/wav-writer.cpp \
    utils/qwave-writer.cpp \
    utils/writer.cpp \
    plugin-manager.cpp \
    utils/logger.cpp \
    utils/json-writer.cpp

HEADERS += \
    server.h \
    sapplication.h \
    utils/recorder-config.h \
    types.h \
    recorder.h \
    unix/daemon.h \
    utils/wav-writer.h \
    utils/qwave-writer.h \
    utils/writer.h \
    utils/wav-writer-iface.h \
    plugin-manager.h \
    recorder-iface.h \
    utils/logger.h \
    unix/date-time.h \
    protocols.h \
    defs.h \
    utils/json-writer.h


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
