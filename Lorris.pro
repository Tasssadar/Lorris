# -------------------------------------------------
# Project created by QtCreator 2011-05-30T19:16:22
# -------------------------------------------------
QT += gui core
TARGET = Lorris
CONFIG(debug, debug|release):DESTDIR = $$PWD/bin/debug
else:DESTDIR = $$PWD/bin/release
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
    dep/qserialdevice/src/qserialdeviceenumerator/serialdeviceenumerator.cpp \
    dep/qserialdevice/src/qserialdevice/nativeserialengine.cpp \
    dep/qserialdevice/src/qserialdevice/abstractserialnotifier.cpp \
    dep/qserialdevice/src/qserialdevice/abstractserialengine.cpp \
    dep/qserialdevice/src/qserialdevice/abstractserial.cpp \
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
    src/connection/fileconnection.cpp \
    src/LorrisAnalyzer/analyzerdataarea.cpp \
    src/LorrisAnalyzer/DataWidgets/datawidget.cpp \
    src/LorrisAnalyzer/DataWidgets/numberwidget.cpp
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
    src/connection/fileconnection.h \
    src/LorrisAnalyzer/analyzerdataarea.h \
    src/LorrisAnalyzer/DataWidgets/datawidget.h \
    src/LorrisAnalyzer/DataWidgets/numberwidget.h

OBJECTS_DIR = $$PWD/obj
MOC_DIR = $$PWD/moc
win32 {
    LIBS += -lsetupapi \
        -luuid \
        -ladvapi32
    SOURCES += dep/qserialdevice/src/qserialdeviceenumerator/serialdeviceenumerator_p_win.cpp \
        dep/qserialdevice/src/qserialdevice/nativeserialnotifier_win.cpp \
        dep/qserialdevice/src/qserialdevice/nativeserialengine_win.cpp
    HEADERS += dep/qserialdevice/src/qwineventnotifier_p.h
    QMAKE_POST_LINK = mkdir \
        "$$DESTDIR/translations" \
        & \
        copy \
        /y \
        translations\\*.qm \
        "$$DESTDIR/translations/"
}
unix:!macx:!symbian {
    LIBS += -ludev
    SOURCES += dep/qserialdevice/src/qserialdeviceenumerator/serialdeviceenumerator_p_unix.cpp \
        dep/qserialdevice/src/qserialdevice/nativeserialnotifier_unix.cpp \
        dep/qserialdevice/src/qserialdevice/nativeserialengine_unix.cpp \
        dep/qserialdevice/src/unix/ttylocker.cpp
    HEADERS += dep/qserialdevice/src/unix/ttylocker.h \
        dep/qserialdevice/src/unix/qcore_unix_p.h
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

RESOURCES += \
    src/LorrisAnalyzer/DataWidgetIcons.qrc

