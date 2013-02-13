# -------------------------------------------------
# Project created by QtCreator 2011-05-30T19:16:22
# -------------------------------------------------
include(../config.pri)

QT += gui core network script
TARGET = Lorris
CONFIG += uitools precompile_header
CONFIG(debug, debug|release):DESTDIR = $$PWD/../bin/debug
else:DESTDIR = $$PWD/../bin/release
OBJECTS_DIR = $$PWD/../obj
MOC_DIR = $$PWD/../moc
UI_DIR = $$PWD/../ui
RCC_DIR = $$PWD/../qrc
CONFIG += qwt

LIBS += -L"$$PWD/../dep/qwt/lib"
LIBS += -L"$$PWD/../dep/qextserialport/lib"
TRANSLATIONS = ../translations/Lorris.cs_CZ.ts
TEMPLATE = app

INCLUDEPATH += ..
INCLUDEPATH += ../dep/qwt/src
INCLUDEPATH += ../dep/qserialdevice/src
INCLUDEPATH += ../dep/qhexedit2/src
INCLUDEPATH += ../dep
INCLUDEPATH += ../dep/qextserialport/src

SOURCES += ui/mainwindow.cpp \
    main.cpp \
    ui/HomeTab.cpp \
    WorkTab/WorkTab.cpp \
    WorkTab/WorkTabMgr.cpp \
    WorkTab/WorkTabInfo.cpp \
    LorrisTerminal/lorristerminal.cpp \
    LorrisTerminal/lorristerminalinfo.cpp \
    connection/connection.cpp \
    connection/serialport.cpp \
    LorrisAnalyzer/lorrisanalyzerinfo.cpp \
    LorrisAnalyzer/lorrisanalyzer.cpp \
    LorrisAnalyzer/sourcedialog.cpp \
    LorrisAnalyzer/labellayout.cpp \
    LorrisAnalyzer/packet.cpp \
    LorrisAnalyzer/DataWidgets/datawidget.cpp \
    LorrisAnalyzer/DataWidgets/numberwidget.cpp \
    LorrisAnalyzer/DataWidgets/barwidget.cpp \
    LorrisAnalyzer/sourceselectdialog.cpp \
    LorrisAnalyzer/DataWidgets/colorwidget.cpp \
    LorrisAnalyzer/DataWidgets/GraphWidget/graphwidget.cpp \
    LorrisAnalyzer/DataWidgets/GraphWidget/graph.cpp \
    LorrisAnalyzer/DataWidgets/GraphWidget/graphdialogs.cpp \
    connection/shupitotunnel.cpp \
    ../dep/qhexedit2/src/xbytearray.cpp \
    ../dep/qhexedit2/src/qhexedit_p.cpp \
    ../dep/qhexedit2/src/qhexedit.cpp \
    ../dep/qhexedit2/src/commands.cpp \
    shared/hexfile.cpp \
    shared/chipdefs.cpp \
    LorrisAnalyzer/DataWidgets/GraphWidget/graphdata.cpp \
    LorrisAnalyzer/DataWidgets/GraphWidget/graphcurve.cpp \
    connection/tcpsocket.cpp \
    LorrisProxy/lorrisproxyinfo.cpp \
    LorrisProxy/lorrisproxy.cpp \
    LorrisProxy/tcpserver.cpp \
    LorrisAnalyzer/DataWidgets/ScriptWidget/scriptwidget.cpp \
    LorrisAnalyzer/DataWidgets/ScriptWidget/scripteditor.cpp \
    ../dep/qscriptsyntaxhighlighter.cpp \
    LorrisAnalyzer/playback.cpp \
    LorrisAnalyzer/DataWidgets/inputwidget.cpp \
    joystick/joymgr.cpp \
    LorrisAnalyzer/DataWidgets/terminalwidget.cpp \
    LorrisAnalyzer/DataWidgets/buttonwidget.cpp \
    ui/tabview.cpp \
    ui/tabwidget.cpp \
    LorrisAnalyzer/DataWidgets/ScriptWidget/scriptstorage.cpp \
    ui/chooseconnectiondlg.cpp \
    ui/connectbutton.cpp \
    connection/connectionmgr2.cpp \
    LorrisAnalyzer/packetparser.cpp \
    ui/plustabbar.cpp \
    ui/homedialog.cpp \
    LorrisAnalyzer/widgetarea.cpp \
    LorrisAnalyzer/storage.cpp \
    shared/fuse_desc.cpp \
    shared/defmgr.cpp \
    ../dep/ecwin7/ecwin7.cpp \
    LorrisAnalyzer/DataWidgets/ScriptWidget/engines/scriptagent.cpp \
    LorrisAnalyzer/DataWidgets/ScriptWidget/engines/qtscriptengine.cpp \
    LorrisAnalyzer/DataWidgets/ScriptWidget/engines/scriptengine.cpp \
    LorrisAnalyzer/DataWidgets/circlewidget.cpp \
    ui/rangeselectdialog.cpp \
    ui/progressbar.cpp \
    ui/tooltipwarn.cpp \
    LorrisAnalyzer/DataWidgets/GraphWidget/graphexport.cpp \
    connection/shupitoconn.cpp \
    misc/utils.cpp \
    misc/config.cpp \
    ui/rotatebutton.cpp \
    ui/terminalsettings.cpp \
    ui/terminal.cpp \
    misc/sessionmgr.cpp \
    misc/datafileparser.cpp \
    LorrisAnalyzer/DataWidgets/sliderwidget.cpp \
    ui/settingsdialog.cpp \
    ui/shortcutinputbox.cpp \
    LorrisAnalyzer/DataWidgets/canvaswidget.cpp \
    LorrisAnalyzer/widgetfactory.cpp \
    ui/resizeline.cpp \
    WorkTab/childtab.cpp \
    WorkTab/tab.cpp \
    connection/proxytunnel.cpp \
    ui/editorwidget.cpp \
    ui/pythonhighlighter.cpp \
    ui/colordialog.cpp \
    misc/gestureidentifier.cpp \
    LorrisAnalyzer/DataWidgets/statuswidget.cpp \
    ui/colorbutton.cpp \
    ui/resettablelineedit.cpp \
    ui/formuladialog.cpp \
    misc/formulaevaluation.cpp \
    LorrisAnalyzer/undostack.cpp \
    LorrisAnalyzer/undoactions.cpp \
    LorrisAnalyzer/filtertabwidget.cpp \
    LorrisAnalyzer/datafilter.cpp \
    misc/qobjectpointer.cpp \
    misc/threadchannel.cpp \
    ui/hookedlineedit.cpp \
    LorrisProgrammer/shupitopacket.cpp \
    LorrisProgrammer/shupitodesc.cpp \
    LorrisProgrammer/shupito.cpp \
    LorrisProgrammer/lorrisprogrammerinfo.cpp \
    LorrisProgrammer/lorrisprogrammer.cpp \
    LorrisProgrammer/programmers/shupitoprogrammer.cpp \
    LorrisProgrammer/programmers/avr232bootprogrammer.cpp \
    LorrisProgrammer/programmers/atsamprogrammer.cpp \
    LorrisProgrammer/modes/shupitospi.cpp \
    LorrisProgrammer/modes/shupitopdi.cpp \
    LorrisProgrammer/modes/shupitomode.cpp \
    LorrisProgrammer/modes/shupitocc25xx.cpp \
    LorrisProgrammer/ui/progressdialog.cpp \
    LorrisProgrammer/ui/programmerui.cpp \
    LorrisProgrammer/ui/overvccdialog.cpp \
    LorrisProgrammer/ui/miniprogrammerui.cpp \
    LorrisProgrammer/ui/fusewidget.cpp \
    LorrisProgrammer/ui/fullprogrammerui.cpp

HEADERS += ui/mainwindow.h \
    revision.h \
    ui/HomeTab.h \
    WorkTab/WorkTab.h \
    WorkTab/WorkTabMgr.h \
    WorkTab/WorkTabInfo.h \
    LorrisTerminal/lorristerminal.h \
    LorrisTerminal/lorristerminalinfo.h \
    connection/connection.h \
    connection/serialport.h \
    LorrisAnalyzer/lorrisanalyzer.h \
    LorrisAnalyzer/lorrisanalyzerinfo.h \
    common.h \
    LorrisAnalyzer/sourcedialog.h \
    LorrisAnalyzer/labellayout.h \
    LorrisAnalyzer/packet.h \
    LorrisAnalyzer/DataWidgets/datawidget.h \
    LorrisAnalyzer/DataWidgets/numberwidget.h \
    LorrisAnalyzer/DataWidgets/barwidget.h \
    LorrisAnalyzer/sourceselectdialog.h \
    LorrisAnalyzer/DataWidgets/colorwidget.h \
    LorrisAnalyzer/DataWidgets/GraphWidget/graphwidget.h \
    LorrisAnalyzer/DataWidgets/GraphWidget/graph.h \
    LorrisAnalyzer/DataWidgets/GraphWidget/graphdialogs.h \
    connection/shupitotunnel.h \
    ../dep/qhexedit2/src/xbytearray.h \
    ../dep/qhexedit2/src/qhexedit_p.h \
    ../dep/qhexedit2/src/qhexedit.h \
    ../dep/qhexedit2/src/commands.h \
    shared/hexfile.h \
    shared/chipdefs.h \
    LorrisAnalyzer/DataWidgets/GraphWidget/graphdata.h \
    LorrisAnalyzer/DataWidgets/GraphWidget/graphcurve.h \
    connection/tcpsocket.h \
    LorrisProxy/lorrisproxyinfo.h \
    LorrisProxy/lorrisproxy.h \
    LorrisProxy/tcpserver.h \
    LorrisAnalyzer/DataWidgets/ScriptWidget/scriptwidget.h \
    LorrisAnalyzer/DataWidgets/ScriptWidget/scripteditor.h \
    ../dep/qscriptsyntaxhighlighter_p.h \
    LorrisAnalyzer/playback.h \
    LorrisAnalyzer/DataWidgets/inputwidget.h \
    joystick/joymgr.h \
    joystick/joystick.h \
    LorrisAnalyzer/DataWidgets/terminalwidget.h \
    LorrisAnalyzer/DataWidgets/buttonwidget.h \
    ui/tabview.h \
    ui/tabwidget.h \
    LorrisAnalyzer/DataWidgets/ScriptWidget/scriptstorage.h \
    ui/chooseconnectiondlg.h \
    ui/connectbutton.h \
    connection/connectionmgr2.h \
    LorrisAnalyzer/packetparser.h \
    ui/plustabbar.h \
    ui/homedialog.h \
    LorrisAnalyzer/widgetarea.h \
    LorrisAnalyzer/storage.h \
    pch.h \
    shared/fuse_desc.h \
    shared/defmgr.h \
    shared/programmer.h \
    ../dep/ecwin7/ecwin7.h \
    LorrisAnalyzer/DataWidgets/ScriptWidget/engines/scriptagent.h \
    LorrisAnalyzer/DataWidgets/ScriptWidget/engines/qtscriptengine.h \
    LorrisAnalyzer/DataWidgets/ScriptWidget/engines/scriptengine.h \
    LorrisAnalyzer/DataWidgets/circlewidget.h \
    ui/rangeselectdialog.h \
    ui/progressbar.h \
    ui/tooltipwarn.h \
    LorrisAnalyzer/DataWidgets/GraphWidget/graphexport.h \
    connection/shupitoconn.h \
    misc/utils.h \
    misc/singleton.h \
    misc/config.h \
    ui/rotatebutton.h \
    ui/terminalsettings.h \
    ui/terminal.h \
    misc/sessionmgr.h \
    misc/datafileparser.h \
    LorrisAnalyzer/DataWidgets/sliderwidget.h \
    ui/settingsdialog.h \
    ui/shortcutinputbox.h \
    LorrisAnalyzer/DataWidgets/canvaswidget.h \
    LorrisAnalyzer/widgetfactory.h \
    ui/resizeline.h \
    WorkTab/childtab.h \
    WorkTab/tab.h \
    connection/proxytunnel.h \
    ui/editorwidget.h \
    ui/pythonhighlighter.h \
    ui/colordialog.h \
    misc/gestureidentifier.h \
    misc/qtpointerarray.h \
    LorrisAnalyzer/DataWidgets/statuswidget.h \
    ui/colorbutton.h \
    ui/resettablelineedit.h \
    ui/formuladialog.h \
    misc/formulaevaluation.h \
    LorrisAnalyzer/undostack.h \
    LorrisAnalyzer/undoactions.h \
    LorrisAnalyzer/filtertabwidget.h \
    LorrisAnalyzer/datafilter.h \
    misc/qobjectpointer.h \
    misc/threadchannel.h \
    ui/hookedlineedit.h \
    LorrisProgrammer/shupitopacket.h \
    LorrisProgrammer/shupitodesc.h \
    LorrisProgrammer/shupito.h \
    LorrisProgrammer/lorrisprogrammerinfo.h \
    LorrisProgrammer/lorrisprogrammer.h \
    LorrisProgrammer/programmers/shupitoprogrammer.h \
    LorrisProgrammer/programmers/avr232bootprogrammer.h \
    LorrisProgrammer/programmers/atsamprogrammer.h \
    LorrisProgrammer/modes/shupitospi.h \
    LorrisProgrammer/modes/shupitopdi.h \
    LorrisProgrammer/modes/shupitomode.h \
    LorrisProgrammer/modes/shupitocc25xx.h \
    LorrisProgrammer/ui/progressdialog.h \
    LorrisProgrammer/ui/programmerui.h \
    LorrisProgrammer/ui/overvccdialog.h \
    LorrisProgrammer/ui/miniprogrammerui.h \
    LorrisProgrammer/ui/fusewidget.h \
    LorrisProgrammer/ui/fullprogrammerui.h

FORMS += \
    LorrisAnalyzer/sourcedialog.ui \
    LorrisAnalyzer/lorrisanalyzer.ui \
    LorrisAnalyzer/sourceselectdialog.ui \
    LorrisAnalyzer/DataWidgets/GraphWidget/graphcurveadddialog.ui \
    LorrisAnalyzer/DataWidgets/GraphWidget/graphcurveeditwidget.ui \
    LorrisTerminal/lorristerminal.ui \
    ui/hometab.ui \
    LorrisProxy/lorrisproxy.ui \
    LorrisAnalyzer/DataWidgets/ScriptWidget/scripteditor.ui \
    LorrisAnalyzer/playback.ui \
    ui/chooseconnectiondlg.ui \
    ui/rangeselectdialog.ui \
    updatecheck.ui \
    LorrisAnalyzer/DataWidgets/GraphWidget/graphexport.ui \
    ui/terminalsettings.ui \
    misc/sessiondialog.ui \
    ui/settingsdialog.ui \
    LorrisAnalyzer/DataWidgets/sliderwidget_horizontal.ui \
    LorrisAnalyzer/DataWidgets/sliderwidget_vertical.ui \
    ui/tabswitchwidget.ui \
    LorrisAnalyzer/DataWidgets/GraphWidget/graphmarkerdialog.ui \
    LorrisAnalyzer/DataWidgets/statusmanager.ui \
    LorrisAnalyzer/DataWidgets/formuladialog.ui \
    LorrisAnalyzer/filterdialog.ui \
    LorrisProgrammer/ui/overvccdialog.ui \
    LorrisProgrammer/ui/miniprogrammerui.ui \
    LorrisProgrammer/ui/fullprogrammerui.ui

RESOURCES += \
    LorrisAnalyzer/DataWidgetIcons.qrc \
    icons.qrc \
    actions.qrc \
    shared/definitions.qrc \
    LorrisAnalyzer/DataWidgets/ScriptWidget/examples/examples.qrc \
    LorrisProgrammer/programmericons.qrc

include(../dep/qtsingleapplication/qtsingleapplication.pri)

RC_FILE = winicon.rc

OTHER_FILES += \
    ../dep/qextserialport/qextserialport.pri \
    shared/fusedesc.txt \
    shared/chipdefs.txt \
    LorrisAnalyzer/DataWidgets/ScriptWidget/examples/snake.py \
    LorrisAnalyzer/DataWidgets/ScriptWidget/examples/snake.js \
    LorrisAnalyzer/DataWidgets/ScriptWidget/examples/default.js \
    LorrisAnalyzer/DataWidgets/ScriptWidget/examples/default.py \
    LorrisAnalyzer/DataWidgets/ScriptWidget/examples/slider.js \
    LorrisAnalyzer/DataWidgets/ScriptWidget/examples/slider.py \
    LorrisAnalyzer/DataWidgets/ScriptWidget/examples/canvas.js \
    LorrisAnalyzer/DataWidgets/ScriptWidget/examples/canvas.py \
    LorrisAnalyzer/DataWidgets/ScriptWidget/examples/graph.js \
    LorrisAnalyzer/DataWidgets/ScriptWidget/examples/graph.py \
    LorrisAnalyzer/DataWidgets/ScriptWidget/examples/joystick.py \
    LorrisAnalyzer/DataWidgets/ScriptWidget/examples/joystick.js \
    LorrisAnalyzer/DataWidgets/ScriptWidget/examples/terminal.js \
    LorrisAnalyzer/DataWidgets/ScriptWidget/examples/terminal.py \
    LorrisAnalyzer/DataWidgets/ScriptWidget/examples/input.js \
    LorrisAnalyzer/DataWidgets/ScriptWidget/examples/input.py

PRECOMPILED_HEADER  = pch.h
precompile_header:!isEmpty(PRECOMPILED_HEADER) {
    DEFINES += USING_PCH
}

win32 {
    CONFIG -= flat
    CONFIG += libenjoy
    
    win32-msvc* {
        CONFIG += libyb
    }

    INCLUDEPATH += ../dep/SDL/include

    CONFIG(debug, debug|release):LIBS += -lqwtd
    else:LIBS += -lqwt

    DEFINES += QT_DLL QWT_DLL QESP_NO_QT4_PRIVATE

    HEADERS += \
        ../dep/qextserialport/src/qextwineventnotifier_p.h \
        ../dep/qextserialport/src/qextserialport_p.h \
        ../dep/qextserialport/src/qextserialport_global.h \
        ../dep/qextserialport/src/qextserialport.h \
        ../dep/qextserialport/src/qextserialenumerator_p.h \
        ../dep/qextserialport/src/qextserialenumerator.h \
        misc/updater.h
    SOURCES += \
        ../dep/qextserialport/src/qextserialenumerator_win.cpp \
        ../dep/qextserialport/src/qextwineventnotifier_p.cpp \
        ../dep/qextserialport/src/qextserialport_win.cpp \
        ../dep/qextserialport/src/qextserialport.cpp \
        ../dep/qextserialport/src/qextserialenumerator.cpp \
        misc/updater.cpp

    LIBS += -lsetupapi -lwinmm -lole32 -ladvapi32 -luser32
}
unix:!macx:!symbian {
    CONFIG += libenjoy libyb
    LIBS += -lqextserialport_lorris

    system_qwt {
        LIBS += -lqwt
    } else {
        LIBS += -lqwt_lorris
    }

    QMAKE_POST_LINK = mkdir \
        "$$DESTDIR/translations" 2> /dev/null \
        ; \
        cp \
        ../translations/*.qm \
        "$$DESTDIR/translations/ 2> /dev/null"

    translations.path = /usr/share/lorris/
    translations.files = ../translations/Lorris.*.qm
    target.path = /usr/bin/
    INSTALLS += target translations
}
macx {
    INCLUDEPATH += ../dep/SDL/include
    LIBS += -lqwt_lorris -lqextserialport -lqextserialport_lorris

    translations.path = /usr/share/lorris/
    translations.files = ../translations/Lorris.*.qm
    qext.path = /usr/lib/
    qext.files = ../dep/qextserialport/lib/libqextserialport_lorris.*
    target.path = /Applications/
    INSTALLS += target translations qext
}

python {
    LIBS += -L"$$PWD/../dep/pythonqt" -lPythonQt
    DEFINES += WITH_PYTHON
    SOURCES += LorrisAnalyzer/DataWidgets/ScriptWidget/engines/pythonengine.cpp
    HEADERS += LorrisAnalyzer/DataWidgets/ScriptWidget/engines/pythonengine.h

    win32 {
        QMAKE_POST_LINK += copy \""$$PWD\\..\\dep\\pythonqt\\PythonQt.dll\"" \""$$DESTDIR\\PythonQt.dll\"" &
    }
}

# must be after lPythonQt, else it will not link properly on some compilers
include(../python.pri)

libyb {
    include(../dep/libyb/libyb.pri)
    DEFINES += HAVE_LIBYB

    SOURCES += \
        connection/genericusbconn.cpp \
        connection/usbacmconn.cpp \
        connection/usbshupito22conn.cpp \
        connection/usbshupito23conn.cpp \
        LorrisProgrammer/programmers/flipprogrammer.cpp

    HEADERS += \
        connection/genericusbconn.h \
        connection/usbacmconn.h \
        connection/usbshupito22conn.h \
        connection/usbshupito23conn.h \
        LorrisProgrammer/programmers/flipprogrammer.h
}

kate_editor:unix {
    DEFINES += USE_KATE
    LIBS += -lktexteditor -lkdecore
}

qsci_editor:win32 {
    DEFINES += USE_QSCI
    win32-msvc* {
        LIBS += -L"$$PWD/../dep/qscintilla2/msvc"
        CONFIG(debug, debug|release):LIBS += -lqscintilla2d
        else:LIBS += -lqscintilla2

        QMAKE_POST_LINK += copy \""$$PWD\\..\\dep\\qscintilla2\\msvc\\qscintilla2.dll\"" \""$$DESTDIR\\qscintilla2.dll\"" &
    } else {
        LIBS += -L"$$PWD/../dep/qscintilla2/" -lqscintilla2
        QMAKE_POST_LINK += copy \""$$PWD\\..\\dep\\qscintilla2\\qscintilla2.dll\"" \""$$DESTDIR\\qscintilla2.dll\"" &
    }
    INCLUDEPATH += "$$PWD/../dep/qscintilla2/"
}

qsci_editor:unix {
    DEFINES += USE_QSCI
    LIBS += -lqscintilla2
}

joystick:libenjoy {
    include(../dep/libenjoy/libenjoy.pri)
    DEFINES += HAVE_LIBENJOY

    SOURCES += joystick/joystick.cpp \
        joystick/joythread.cpp
    HEADERS += joystick/joythread.h
}
