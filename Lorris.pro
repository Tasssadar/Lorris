# -------------------------------------------------
# Project created by QtCreator 2011-05-30T19:16:22
# -------------------------------------------------
include( dep/qwt/examples.pri )
QT += network \
    webkit
CONFIG += qwt
TARGET = Lorris
CONFIG(debug, debug|release):DESTDIR = bin/debug
else:DESTDIR = bin/release
TEMPLATE = app
INCLUDEPATH += dep/qwt/src
INCLUDEPATH += src
SOURCES += src/mainwindow.cpp \
    src/main.cpp \
    src/HomeTab.cpp \
    src/WorkTab.cpp \
    src/LorrisProbe/lorrisprobe.cpp \
    src/WorkTabMgr.cpp \
    src/WorkTabInfo.cpp \
    src/LorrisProbe/lorrisprobeinfo.cpp
HEADERS += src/mainwindow.h \
    src/revision.h \
    src/HomeTab.h \
    src/common.h \
    src/WorkTab.h \
    src/LorrisProbe/lorrisprobe.h \
    src/WorkTabMgr.h \
    src/WorkTabInfo.h \
    src/LorrisProbe/lorrisprobeinfo.h
QMAKE_LIBDIR += dep/qwt/lib
LIBS += -lqwt
OBJECTS_DIR = obj
MOC_DIR = moc
