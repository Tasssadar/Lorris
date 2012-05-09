# -------------------------------------------------
# Project created by QtCreator 2011-05-30T19:16:22
# -------------------------------------------------
QT += gui core network script
TARGET = Lorris
CONFIG += uitools
CONFIG(debug, debug|release):DESTDIR = $$PWD/bin/debug
else:DESTDIR = $$PWD/bin/release
OBJECTS_DIR = $$PWD/obj
MOC_DIR = $$PWD/moc
UI_DIR = $$PWD/ui
RCC_DIR = $$PWD/qrc
CONFIG += qwt
LIBS += -L"$$PWD/dep/qwt/lib"
LIBS += -L"$$PWD/dep/qextserialport/lib"
TRANSLATIONS = translations/Lorris.cs_CZ.ts
TEMPLATE = app
INCLUDEPATH += dep/qwt/src
INCLUDEPATH += dep/qserialdevice/src
INCLUDEPATH += dep/qhexedit2/src
INCLUDEPATH += dep
INCLUDEPATH += dep/qextserialport/src
SOURCES += src/ui/mainwindow.cpp \
    src/main.cpp \
    src/ui/HomeTab.cpp \
    src/WorkTab/WorkTab.cpp \
    src/WorkTab/WorkTabMgr.cpp \
    src/WorkTab/WorkTabInfo.cpp \
    src/LorrisTerminal/lorristerminal.cpp \
    src/LorrisTerminal/lorristerminalinfo.cpp \
    src/connection/connection.cpp \
    src/connection/serialport.cpp \
    src/LorrisTerminal/eeprom.cpp \
    src/LorrisAnalyzer/lorrisanalyzerinfo.cpp \
    src/LorrisAnalyzer/lorrisanalyzer.cpp \
    src/LorrisAnalyzer/sourcedialog.cpp \
    src/utils.cpp \
    src/LorrisAnalyzer/labellayout.cpp \
    src/LorrisAnalyzer/packet.cpp \
    src/LorrisAnalyzer/devicetabwidget.cpp \
    src/LorrisAnalyzer/cmdtabwidget.cpp \
    src/LorrisAnalyzer/DataWidgets/datawidget.cpp \
    src/LorrisAnalyzer/DataWidgets/numberwidget.cpp \
    src/LorrisAnalyzer/DataWidgets/barwidget.cpp \
    src/LorrisAnalyzer/sourceselectdialog.cpp \
    src/LorrisShupito/lorrisshupito.cpp \
    src/LorrisShupito/lorrisshupitoinfo.cpp \
    src/LorrisShupito/shupito.cpp \
    src/LorrisShupito/shupitodesc.cpp \
    src/LorrisAnalyzer/datafileparser.cpp \
    src/LorrisAnalyzer/DataWidgets/colorwidget.cpp \
    src/LorrisAnalyzer/DataWidgets/GraphWidget/graphwidget.cpp \
    src/LorrisAnalyzer/DataWidgets/GraphWidget/graph.cpp \
    src/LorrisAnalyzer/DataWidgets/GraphWidget/graphdialogs.cpp \
    src/connection/shupitotunnel.cpp \
    src/config.cpp \
    dep/qhexedit2/src/xbytearray.cpp \
    dep/qhexedit2/src/qhexedit_p.cpp \
    dep/qhexedit2/src/qhexedit.cpp \
    dep/qhexedit2/src/commands.cpp \
    src/LorrisShupito/fusewidget.cpp \
    src/shared/hexfile.cpp \
    src/shared/chipdefs.cpp \
    src/LorrisAnalyzer/DataWidgets/GraphWidget/graphdata.cpp \
    src/LorrisAnalyzer/DataWidgets/GraphWidget/graphcurve.cpp \
    src/LorrisShupito/flashbuttonmenu.cpp \
    src/LorrisShupito/modes/shupitospi.cpp \
    src/LorrisShupito/modes/shupitopdi.cpp \
    src/LorrisShupito/modes/shupitomode.cpp \
    src/LorrisShupito/modes/shupitocc25xx.cpp \
    src/LorrisShupito/shupitopacket.cpp \
    src/connection/tcpsocket.cpp \
    src/LorrisProxy/lorrisproxyinfo.cpp \
    src/LorrisProxy/lorrisproxy.cpp \
    src/LorrisProxy/tcpserver.cpp \
    src/LorrisShupito/progressdialog.cpp \
    src/LorrisAnalyzer/DataWidgets/ScriptWidget/scriptwidget.cpp \
    src/LorrisAnalyzer/DataWidgets/ScriptWidget/scriptenv.cpp \
    src/LorrisAnalyzer/DataWidgets/ScriptWidget/scripteditor.cpp \
    src/shared/terminal.cpp \
    dep/qscriptsyntaxhighlighter.cpp \
    src/LorrisAnalyzer/playback.cpp \
    src/LorrisAnalyzer/DataWidgets/inputwidget.cpp \
    src/LorrisAnalyzer/DataWidgets/ScriptWidget/scriptagent.cpp \
    src/shared/rotatebutton.cpp \
    src/joystick/joymgr.cpp \
    src/joystick/joystick.cpp \
    src/LorrisAnalyzer/DataWidgets/terminalwidget.cpp \
    src/joystick/joythread.cpp \
    src/LorrisAnalyzer/DataWidgets/buttonwidget.cpp \
    src/ui/tabview.cpp \
    src/ui/tabwidget.cpp \
    src/LorrisAnalyzer/DataWidgets/ScriptWidget/scriptstorage.cpp \
    src/ui/chooseconnectiondlg.cpp \
    src/ui/connectbutton.cpp \
    dep/qextserialport/src/qextserialport.cpp \
    dep/qextserialport/src/qextserialenumerator.cpp \
    src/connection/connectionmgr2.cpp \
    src/LorrisAnalyzer/packetparser.cpp \
    src/ui/plustabbar.cpp \
    src/ui/homedialog.cpp \
    src/LorrisAnalyzer/widgetarea.cpp \
    src/LorrisAnalyzer/storage.cpp
HEADERS += src/ui/mainwindow.h \
    src/revision.h \
    src/ui/HomeTab.h \
    src/WorkTab/WorkTab.h \
    src/WorkTab/WorkTabMgr.h \
    src/WorkTab/WorkTabInfo.h \
    src/singleton.h \
    src/LorrisTerminal/lorristerminal.h \
    src/LorrisTerminal/lorristerminalinfo.h \
    src/connection/connection.h \
    src/connection/serialport.h \
    src/LorrisTerminal/eeprom.h \
    src/LorrisAnalyzer/lorrisanalyzer.h \
    src/LorrisAnalyzer/lorrisanalyzerinfo.h \
    src/common.h \
    src/LorrisAnalyzer/sourcedialog.h \
    src/utils.h \
    src/LorrisAnalyzer/labellayout.h \
    src/LorrisAnalyzer/packet.h \
    src/LorrisAnalyzer/devicetabwidget.h \
    src/LorrisAnalyzer/cmdtabwidget.h \
    src/LorrisAnalyzer/DataWidgets/datawidget.h \
    src/LorrisAnalyzer/DataWidgets/numberwidget.h \
    src/LorrisAnalyzer/DataWidgets/barwidget.h \
    src/LorrisAnalyzer/sourceselectdialog.h \
    src/LorrisShupito/lorrisshupito.h \
    src/LorrisShupito/lorrisshupitoinfo.h \
    src/LorrisShupito/shupito.h \
    src/LorrisShupito/shupitodesc.h \
    src/LorrisAnalyzer/datafileparser.h \
    src/LorrisAnalyzer/DataWidgets/colorwidget.h \
    src/LorrisAnalyzer/DataWidgets/GraphWidget/graphwidget.h \
    src/LorrisAnalyzer/DataWidgets/GraphWidget/graph.h \
    src/LorrisAnalyzer/DataWidgets/GraphWidget/graphdialogs.h \
    src/connection/shupitotunnel.h \
    src/config.h \
    dep/qhexedit2/src/xbytearray.h \
    dep/qhexedit2/src/qhexedit_p.h \
    dep/qhexedit2/src/qhexedit.h \
    dep/qhexedit2/src/commands.h \
    src/LorrisShupito/fusewidget.h \
    src/shared/hexfile.h \
    src/shared/chipdefs.h \
    src/LorrisAnalyzer/DataWidgets/GraphWidget/graphdata.h \
    src/LorrisAnalyzer/DataWidgets/GraphWidget/graphcurve.h \
    src/LorrisShupito/flashbuttonmenu.h \
    src/LorrisShupito/modes/shupitospi.h \
    src/LorrisShupito/modes/shupitopdi.h \
    src/LorrisShupito/modes/shupitomode.h \
    src/LorrisShupito/modes/shupitocc25xx.h \
    src/LorrisShupito/shupitopacket.h \
    src/connection/tcpsocket.h \
    src/LorrisProxy/lorrisproxyinfo.h \
    src/LorrisProxy/lorrisproxy.h \
    src/LorrisProxy/tcpserver.h \
    src/LorrisShupito/progressdialog.h \
    src/LorrisAnalyzer/DataWidgets/ScriptWidget/scriptwidget.h \
    src/LorrisAnalyzer/DataWidgets/ScriptWidget/scriptenv.h \
    src/LorrisAnalyzer/DataWidgets/ScriptWidget/scripteditor.h \
    src/shared/terminal.h \
    dep/qscriptsyntaxhighlighter_p.h \
    src/LorrisAnalyzer/playback.h \
    src/LorrisAnalyzer/DataWidgets/inputwidget.h \
    src/LorrisAnalyzer/DataWidgets/ScriptWidget/scriptagent.h \
    src/shared/rotatebutton.h \
    src/joystick/joymgr.h \
    src/joystick/joystick.h \
    src/LorrisAnalyzer/DataWidgets/terminalwidget.h \
    src/joystick/joythread.h \
    src/LorrisAnalyzer/DataWidgets/buttonwidget.h \
    src/ui/tabview.h \
    src/ui/tabwidget.h \
    src/LorrisAnalyzer/DataWidgets/ScriptWidget/scriptstorage.h \
    src/ui/chooseconnectiondlg.h \
    src/ui/connectbutton.h \
    dep/qextserialport/src/qextserialport_p.h \
    dep/qextserialport/src/qextserialport_global.h \
    dep/qextserialport/src/qextserialport.h \
    dep/qextserialport/src/qextserialenumerator_p.h \
    dep/qextserialport/src/qextserialenumerator.h \
    src/connection/connectionmgr2.h \
    src/LorrisAnalyzer/packetparser.h \
    src/ui/plustabbar.h \
    src/ui/homedialog.h \
    src/LorrisAnalyzer/datafileparser.h \
    src/LorrisAnalyzer/widgetarea.h \
    src/LorrisAnalyzer/storage.h

win32 {
    INCLUDEPATH += dep/SDL/include

    DEFINES += QT_DLL QWT_DLL QESP_NO_QT4_PRIVATE
    QMAKE_LFLAGS = -enable-stdcall-fixup -Wl,-enable-auto-import -Wl,-enable-runtime-pseudo-reloc

    HEADERS += \
        dep/qextserialport/src/qextwineventnotifier_p.h
    SOURCES += \
        dep/qextserialport/src/qextserialenumerator_win.cpp \
        dep/qextserialport/src/qextwineventnotifier_p.cpp \
        dep/qextserialport/src/qextserialport_win.cpp

    LIBS += -L"$$PWD/dep/SDL/lib" -lsdl -lsetupapi
    CONFIG(debug, debug|release):LIBS += -lqwtd
    else:LIBS += -lqwt
}
unix:!macx:!symbian {
    SOURCES += \
        dep/qextserialport/src/qextserialport_unix.cpp \
        dep/qextserialport/src/qextserialenumerator_unix.cpp

    LIBS += -lqwt -ludev -lSDL
    QMAKE_POST_LINK = mkdir \
        "$$DESTDIR/translations" \
        & \
        cp \
        translations/*.qm \
        "$$DESTDIR/translations/"
}
macx {
    SOURCES += \
        dep/qextserialport/src/qextserialenumerator_osx.cpp

    LIBS += -lqwt -lqextserialport -lSDL
    QMAKE_POST_LINK = mkdir \
        "$$DESTDIR/translations" \
        & \
        cp \
        translations/*.qm \
        "$$DESTDIR/translations/"
}

FORMS += \
    src/LorrisAnalyzer/sourcedialog.ui \
    src/LorrisAnalyzer/lorrisanalyzer.ui \
    src/LorrisAnalyzer/DataWidgets/rangeselectdialog.ui \
    src/LorrisAnalyzer/sourceselectdialog.ui \
    src/LorrisShupito/lorrisshupito.ui \
    src/LorrisAnalyzer/DataWidgets/GraphWidget/graphcurveadddialog.ui \
    src/LorrisAnalyzer/DataWidgets/GraphWidget/graphcurveeditwidget.ui \
    src/LorrisTerminal/lorristerminal.ui \
    src/ui/hometab.ui \
    src/LorrisProxy/lorrisproxy.ui \
    src/LorrisAnalyzer/DataWidgets/ScriptWidget/scripteditor.ui \
    src/LorrisAnalyzer/playback.ui \
    src/ui/chooseconnectiondlg.ui

RESOURCES += \
    src/LorrisAnalyzer/DataWidgetIcons.qrc \
    src/LorrisShupito/shupitoicons.qrc \
    src/icons.qrc \
    src/actions.qrc

RC_FILE = src/winicon.rc

OTHER_FILES += \
    dep/qextserialport/src/qextserialport.pri
