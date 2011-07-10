# -------------------------------------------------
# Project created by QtCreator 2011-05-30T19:16:22
# -------------------------------------------------
QT += network \
    webkit
CONFIG += qwt
TARGET = Lorris
CONFIG(debug, debug|release):DESTDIR = bin/debug
else:DESTDIR = bin/release
TEMPLATE = app
INCLUDEPATH += dep/qwt/src
INCLUDEPATH += dep/qserialdevice/src
INCLUDEPATH += src
SOURCES += src/mainwindow.cpp \
    src/main.cpp \
    src/HomeTab.cpp \
    src/WorkTab.cpp \
    src/LorrisProbe/lorrisprobe.cpp \
    src/WorkTabMgr.cpp \
    src/WorkTabInfo.cpp \
    src/LorrisProbe/lorrisprobeinfo.cpp \
    src/connection/connectionmgr.cpp \
    src/ui/tabdialog.cpp \
    src/LorrisTerminal/lorristerminal.cpp \
    src/LorrisTerminal/lorristerminalinfo.cpp \
    src/connection/connection.cpp \
    src/connection/serialport.cpp \
    dep/qserialdevice/src/qserialdeviceenumerator/serialdeviceenumerator.cpp \
    dep/qserialdevice/src/qserialdevice/nativeserialengine.cpp \
    dep/qserialdevice/src/qserialdevice/abstractserialnotifier.cpp \
    dep/qserialdevice/src/qserialdevice/abstractserialengine.cpp \
    dep/qserialdevice/src/qserialdevice/abstractserial.cpp \
    src/LorrisTerminal/hexfile.cpp \
    src/LorrisTerminal/terminal.cpp \
    src/connection/serialportthread.cpp
HEADERS += src/mainwindow.h \
    src/revision.h \
    src/HomeTab.h \
    src/WorkTab.h \
    src/LorrisProbe/lorrisprobe.h \
    src/WorkTabMgr.h \
    src/WorkTabInfo.h \
    src/LorrisProbe/lorrisprobeinfo.h \
    src/connection/connectionmgr.h \
    src/ui/tabdialog.h \
    src/singleton.h \
    src/LorrisTerminal/lorristerminal.h \
    src/LorrisTerminal/lorristerminalinfo.h \
    src/connection/connection.h \
    src/connection/serialport.h \
    dep/qserialdevice/src/qserialdeviceenumerator/serialdeviceenumerator_p.h \
    dep/qserialdevice/src/qserialdeviceenumerator/serialdeviceenumerator.h \
    dep/qserialdevice/src/qserialdevice/nativeserialnotifier.h \
    dep/qserialdevice/src/qserialdevice/nativeserialengine_p.h \
    dep/qserialdevice/src/qserialdevice/nativeserialengine.h \
    dep/qserialdevice/src/qserialdevice/abstractserialnotifier.h \
    dep/qserialdevice/src/qserialdevice/abstractserialengine_p.h \
    dep/qserialdevice/src/qserialdevice/abstractserialengine.h \
    dep/qserialdevice/src/qserialdevice/abstractserial_p.h \
    dep/qserialdevice/src/qserialdevice/abstractserial.h \
    dep/qserialdevice/src/qserialdevice_global.h \
    src/LorrisTerminal/hexfile.h \
    src/LorrisTerminal/deviceinfo.h \
    src/LorrisTerminal/terminal.h \
    src/connection/serialportthread.h
QMAKE_LIBDIR += dep/qwt/lib

# LIBS += -lqwt
OBJECTS_DIR = obj
MOC_DIR = moc
win32 { 
    LIBS += -lsetupapi \
        -luuid \
        -ladvapi32
    SOURCES += dep/qserialdevice/src/qserialdeviceenumerator/serialdeviceenumerator_p_win.cpp \
        dep/qserialdevice/src/qserialdevice/nativeserialnotifier_win.cpp \
        dep/qserialdevice/src/qserialdevice/nativeserialengine_win.cpp
    HEADERS += dep/qserialdevice/src/qwineventnotifier_p.h
}
unix:!macx { 
    LIBS += -ludev
    SOURCES += dep/qserialdevice/src/qserialdeviceenumerator/serialdeviceenumerator_p_unix.cpp \
        dep/qserialdevice/src/qserialdevice/nativeserialnotifier_unix.cpp \
        dep/qserialdevice/src/qserialdevice/nativeserialengine_unix.cpp \
        dep/qserialdevice/src/unix/ttylocker.cpp
    HEADERS += dep/qserialdevice/src/unix/ttylocker.h \
        dep/qserialdevice/src/unix/qcore_unix_p.h
}
macx { 
    LIBS += -framework \
        IOKit \
        -framework \
        CoreFoundation
    SOURCES += dep/qserialdevice/src/qserialdeviceenumerator/serialdeviceenumerator_p_mac.cpp \
        dep/qserialdevice/src/qserialdevice/nativeserialnotifier_unix.cpp \
        dep/qserialdevice/src/qserialdevice/nativeserialengine_unix.cpp
}
