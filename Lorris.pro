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
SOURCES += src/mainwindow.cpp \
    src/main.cpp \
    src/plot.cpp \
    src/curvedata.cpp \
    src/signaldata.cpp \
    src/HomeTab.cpp
HEADERS += src/mainwindow.h \
    src/plot.h \
    src/curvedata.h \
    src/signaldata.h \
    src/revision.h \
    src/HomeTab.h
QMAKE_LIBDIR += dep/qwt/lib
LIBS += -lqwt
OBJECTS_DIR = obj
MOC_DIR = moc
