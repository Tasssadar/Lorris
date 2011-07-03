TEMPLATE = app
CONFIG += console
QT -= gui
OBJECTS_DIR = obj
MOC_DIR = moc
INCLUDEPATH += ../../src/qserialdevice
SOURCES += main.cpp

include(../../src/qserialdevice/qserialdevice.pri)

CONFIG(debug, debug|release) {
    DESTDIR = debug
    TARGET = testd
} else {
    DESTDIR = release
    TARGET = test
}
