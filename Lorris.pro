# -------------------------------------------------
# Project created by QtCreator 2011-05-30T19:16:22
# -------------------------------------------------
QT += network gui core
CONFIG += qwt
TARGET = Lorris
CONFIG(debug, debug|release):DESTDIR = bin/debug
else:DESTDIR = bin/release
TRANSLATIONS = translations/Lorris.cs_CZ.ts
TEMPLATE = app
INCLUDEPATH += dep/qwt/src
INCLUDEPATH += dep/qserialdevice/src
INCLUDEPATH += src
SOURCES += src/ui/mainwindow.cpp \
    src/main.cpp \
    src/ui/HomeTab.cpp \
    src/WorkTab/WorkTab.cpp \
    src/LorrisProbe/lorrisprobe.cpp \
    src/WorkTab/WorkTabMgr.cpp \
    src/WorkTab/WorkTabInfo.cpp \
    src/LorrisProbe/lorrisprobeinfo.cpp \
    src/connection/connectionmgr.cpp \
    src/ui/tabdialog.cpp \
    src/LorrisTerminal/lorristerminal.cpp \
    src/LorrisTerminal/lorristerminalinfo.cpp \
    src/connection/connection.cpp \
    src/connection/serialport.cpp \
    src/LorrisTerminal/hexfile.cpp \
    src/LorrisTerminal/terminal.cpp \
    src/connection/serialportthread.cpp \
    src/LorrisTerminal/eeprom.cpp \
    src/LorrisAnalyzer/lorrisanalyzerinfo.cpp \
    src/LorrisAnalyzer/lorrisanalyzer.cpp \
    src/LorrisAnalyzer/sourcedialog.cpp \
    src/utils.cpp \
    src/LorrisAnalyzer/labellayout.cpp \
    src/LorrisAnalyzer/packet.cpp \
    src/LorrisAnalyzer/analyzerdatastorage.cpp \
    src/LorrisAnalyzer/devicetabwidget.cpp \
    src/LorrisAnalyzer/cmdtabwidget.cpp \
    src/LorrisAnalyzer/analyzermdi.cpp \
    src/LorrisAnalyzer/DataWidgets/datawidget.cpp \
    src/connection/fileconnection.cpp
HEADERS += src/ui/mainwindow.h \
    src/revision.h \
    src/ui/HomeTab.h \
    src/WorkTab/WorkTab.h \
    src/LorrisProbe/lorrisprobe.h \
    src/WorkTab/WorkTabMgr.h \
    src/WorkTab/WorkTabInfo.h \
    src/LorrisProbe/lorrisprobeinfo.h \
    src/connection/connectionmgr.h \
    src/ui/tabdialog.h \
    src/singleton.h \
    src/LorrisTerminal/lorristerminal.h \
    src/LorrisTerminal/lorristerminalinfo.h \
    src/connection/connection.h \
    src/connection/serialport.h \
    src/LorrisTerminal/hexfile.h \
    src/LorrisTerminal/deviceinfo.h \
    src/LorrisTerminal/terminal.h \
    src/connection/serialportthread.h \
    src/LorrisTerminal/eeprom.h \
    src/LorrisAnalyzer/lorrisanalyzer.h \
    src/LorrisAnalyzer/lorrisanalyzerinfo.h \
    src/LorrisAnalyzer/lorrisanalyzer.h \
    src/common.h \
    src/LorrisAnalyzer/sourcedialog.h \
    src/utils.h \
    src/LorrisAnalyzer/labellayout.h \
    src/LorrisAnalyzer/packet.h \
    src/LorrisAnalyzer/analyzerdatastorage.h \
    src/LorrisAnalyzer/devicetabwidget.h \
    src/LorrisAnalyzer/cmdtabwidget.h \
    src/LorrisAnalyzer/analyzermdi.h \
    src/LorrisAnalyzer/DataWidgets/datawidget.h \
    src/connection/fileconnection.h
QMAKE_LIBDIR += dep/qwt/lib

# LIBS += -lqwt
OBJECTS_DIR = obj
MOC_DIR = moc
win32 { 
    LIBS += -lsetupapi \
        -luuid \
        -ladvapi32
}
unix:!macx { 
    LIBS += -ludev
    QMAKE_POST_LINK = mkdir \
        "$$DESTDIR/translations" \
        & \
        cp \
        translations/*.qm \
        "$$DESTDIR/translations/"
}
macx { 
    LIBS += -framework \
        IOKit \
        -framework \
        CoreFoundation
    SOURCES += dep/qserialdevice/src/qserialdeviceenumerator/serialdeviceenumerator_p_mac.cpp \
        dep/qserialdevice/src/qserialdevice/nativeserialnotifier_unix.cpp \
        dep/qserialdevice/src/qserialdevice/nativeserialengine_unix.cpp
    QMAKE_POST_LINK = mkdir \
        "$$DESTDIR/translations" \
        & \
        cp \
        translations/*.qm \
        "$$DESTDIR/translations/"
}

FORMS += \
    src/LorrisAnalyzer/sourcedialog.ui \
    src/LorrisAnalyzer/lorrisanalyzer.ui



win32:CONFIG(release, debug|release): LIBS += -L$$PWD/dep/qserialdevice/src/build/release/ -lqserialdeviced
else:win32:CONFIG(debug, debug|release): LIBS += -L$$PWD/dep/qserialdevice/src/build/debug/ -lqserialdeviced
else:unix:!symbian:CONFIG(debug, debug|release): LIBS += -L$$PWD/dep/qserialdevice/src/build/debug/ -lqserialdeviced
else:unix:!symbian:CONFIG(release, debug|release): LIBS += -L$$PWD/dep/qserialdevice/src/build/release/ -lqserialdeviced

INCLUDEPATH += $$PWD/dep/qserialdevice/src
DEPENDPATH += $$PWD/dep/qserialdevice/src

win32:CONFIG(release, debug|release): PRE_TARGETDEPS += $$PWD/dep/qserialdevice/src/build/release/qserialdeviced.lib
else:win32:CONFIG(debug, debug|release): PRE_TARGETDEPS += $$PWD/dep/qserialdevice/src/build/debug/qserialdeviced.lib
else:unix:!symbian:CONFIG(release, debug|release): PRE_TARGETDEPS += $$PWD/dep/qserialdevice/src/build/release/libqserialdeviced.a
else:unix:!symbian:CONFIG(debug, debug|release): PRE_TARGETDEPS += $$PWD/dep/qserialdevice/src/build/debug/libqserialdeviced.a
