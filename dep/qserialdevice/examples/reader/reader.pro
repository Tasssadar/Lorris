TEMPLATE = app
CONFIG += console
QT -= gui
OBJECTS_DIR = obj
MOC_DIR = moc
INCLUDEPATH += ../../src/qserialdevice
SOURCES += main.cpp

CONFIG(debug, debug|release) {
    QMAKE_LIBDIR += ../../src/build/debug
    LIBS += -lqserialdeviced
    DESTDIR = debug
    TARGET = readerd
} else {
    QMAKE_LIBDIR += ../../src/build/release
    LIBS += -lqserialdevice
    DESTDIR = release
    TARGET = reader
}
