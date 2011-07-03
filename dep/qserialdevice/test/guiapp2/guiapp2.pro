#-------------------------------------------------
#
# Project created by QtCreator 2011-01-10T12:51:05
#
#-------------------------------------------------

QT       += core gui

TARGET = guiapp2
TEMPLATE = app

include(../../src/qserialdeviceenumerator/qserialdeviceenumerator.pri)
include(../../src/qserialdevice/qserialdevice.pri)

SOURCES += main.cpp\
        mainwidget.cpp \
    infowidget.cpp \
    optionswidget.cpp \
    tracewidget.cpp
HEADERS  += mainwidget.h \
    infowidget.h \
    optionswidget.h \
    tracewidget.h
FORMS    += mainwidget.ui \
    infowidget.ui \
    optionswidget.ui \
    tracewidget.ui

win32 {
    LIBS += -lsetupapi -luuid -ladvapi32
}
unix:!macx {
    LIBS += -ludev
}
