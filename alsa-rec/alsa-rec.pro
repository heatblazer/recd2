QT += core

TEMPLATE = lib
CONFIG += console c++11
CONFIG -= app_bundle

LIBS += -lpthread
LIBS += -lasound


SOURCES += main.cpp \
    thread.cpp \
    alsarec.cpp

HEADERS += \
    plugin-iface.h \
    thread.h \
    alsarec.h

win32:CONFIG(release, debug|release): LIBS += -L$$OUT_PWD/../utils/release/ -lutils
else:win32:CONFIG(debug, debug|release): LIBS += -L$$OUT_PWD/../utils/debug/ -lutils
else:unix: LIBS += -L$$OUT_PWD/../utils/ -lutils

INCLUDEPATH += $$PWD/../utils
DEPENDPATH += $$PWD/../utils
