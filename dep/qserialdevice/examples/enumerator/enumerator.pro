TEMPLATE = app
CONFIG += console
QT -= gui
OBJECTS_DIR = obj
MOC_DIR = moc
INCLUDEPATH += ../../src/qserialdeviceenumerator
HEADERS += enumerator.h
SOURCES += main.cpp enumerator.cpp

CONFIG(debug, debug|release) {
    QMAKE_LIBDIR += ../../src/build/debug
    LIBS += -lqserialdeviced
    DESTDIR = debug
    TARGET = enumeratord
} else {
    QMAKE_LIBDIR += ../../src/build/release
    LIBS += -lqserialdevice
    DESTDIR = release
    TARGET = enumerator
}

win32 {
    LIBS += -lsetupapi -luuid -ladvapi32
}
unix:!macx {
    LIBS += -ludev
}
