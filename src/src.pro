# -------------------------------------------------
# Project created by QtCreator 2011-05-30T19:16:22
# -------------------------------------------------
include(../config.pri)

greaterThan(QT_MAJOR_VERSION, 4) {
    QT += widgets concurrent uitools
} else {
    CONFIG += uitools
}

QT += gui core network script
TARGET = lorris
CONFIG += precompile_header
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
    shared/programmer.cpp \
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
    misc/threadchannel.cpp \
    ui/hookedlineedit.cpp \
    LorrisProgrammer/shupitopacket.cpp \
    LorrisProgrammer/shupitodesc.cpp \
    LorrisProgrammer/shupito.cpp \
    LorrisProgrammer/lorrisprogrammerinfo.cpp \
    LorrisProgrammer/lorrisprogrammer.cpp \
    LorrisProgrammer/programmers/shupitoprogrammer.cpp \
    LorrisProgrammer/programmers/avr232bootprogrammer.cpp \
    LorrisProgrammer/modes/shupitospi.cpp \
    LorrisProgrammer/modes/shupitospiflash.cpp \
    LorrisProgrammer/modes/shupitopdi.cpp \
    LorrisProgrammer/modes/shupitomode.cpp \
    LorrisProgrammer/modes/shupitocc25xx.cpp \
    LorrisProgrammer/modes/shupitods89c.cpp \
    LorrisProgrammer/ui/progressdialog.cpp \
    LorrisProgrammer/ui/programmerui.cpp \
    LorrisProgrammer/ui/overvccdialog.cpp \
    LorrisProgrammer/ui/miniprogrammerui.cpp \
    LorrisProgrammer/ui/fusewidget.cpp \
    LorrisProgrammer/ui/fullprogrammerui.cpp \
    LorrisProgrammer/programmers/avr109programmer.cpp \
    LorrisProgrammer/programmers/atsamprogrammer.cpp \
    ui/bytevalidator.cpp \
    misc/qtobjectpointer.cpp \
    LorrisAnalyzer/searchwidget.cpp \
    LorrisAnalyzer/confirmwidget.cpp \
    LorrisAnalyzer/DataWidgets/RotationWidget/rotationwidget.cpp \
    LorrisAnalyzer/storagedata.cpp \
    ui/floatingwidget.cpp \
    ui/floatinginputdialog.cpp \
    LorrisProgrammer/modes/shupitospitunnel.cpp \
    connection/shupitospitunnelconn.cpp \
    LorrisProgrammer/programmers/arduinoprogrammer.cpp \
    connection/udpsocket.cpp \
    LorrisProgrammer/programmers/zmodemprogrammer.cpp \
    ../dep/qextserialport/src/qextserialport.cpp \
    ../dep/qextserialport/src/qextserialenumerator.cpp

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
    misc/threadchannel.h \
    ui/hookedlineedit.h \
    LorrisProgrammer/shupitopacket.h \
    LorrisProgrammer/shupitodesc.h \
    LorrisProgrammer/shupito.h \
    LorrisProgrammer/lorrisprogrammerinfo.h \
    LorrisProgrammer/lorrisprogrammer.h \
    LorrisProgrammer/programmers/shupitoprogrammer.h \
    LorrisProgrammer/programmers/avr232bootprogrammer.h \
    LorrisProgrammer/modes/shupitospi.h \
    LorrisProgrammer/modes/shupitospiflash.h \
    LorrisProgrammer/modes/shupitopdi.h \
    LorrisProgrammer/modes/shupitomode.h \
    LorrisProgrammer/modes/shupitocc25xx.h \
    LorrisProgrammer/modes/shupitods89c.h \
    LorrisProgrammer/ui/progressdialog.h \
    LorrisProgrammer/ui/programmerui.h \
    LorrisProgrammer/ui/overvccdialog.h \
    LorrisProgrammer/ui/miniprogrammerui.h \
    LorrisProgrammer/ui/fusewidget.h \
    LorrisProgrammer/ui/fullprogrammerui.h \
    LorrisProgrammer/programmers/avr109programmer.h \
    LorrisProgrammer/programmers/atsamprogrammer.h \
    ui/bytevalidator.h \
    misc/qtobjectpointer.h \
    LorrisAnalyzer/searchwidget.h \
    LorrisAnalyzer/confirmwidget.h \
    ui/floatingwidget.h \
    ui/floatinginputdialog.h \
    LorrisAnalyzer/DataWidgets/RotationWidget/rotationwidget.h \
    LorrisAnalyzer/storagedata.h \
    LorrisProgrammer/modes/shupitospitunnel.h \
    connection/shupitospitunnelconn.h \
    LorrisProgrammer/programmers/arduinoprogrammer.h \
    connection/udpsocket.h \
    LorrisProgrammer/programmers/zmodemprogrammer.h \
    LorrisProgrammer/programmers/zmodemprogrammer-defines.h \
    ui/termina-colors.h \
    ../dep/qextserialport/src/qextserialport_p.h \
    ../dep/qextserialport/src/qextserialport_global.h \
    ../dep/qextserialport/src/qextserialport.h \
    ../dep/qextserialport/src/qextserialenumerator_p.h \
    ../dep/qextserialport/src/qextserialenumerator.h

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
    LorrisProgrammer/ui/fullprogrammerui.ui \
    ui/sessionsavedialog.ui \
    LorrisProgrammer/ui/shupitospitunnelwidget.ui \
    ui/qscisearchbar.ui

RESOURCES += \
    LorrisAnalyzer/DataWidgetIcons.qrc \
    icons.qrc \
    actions.qrc \
    shared/definitions.qrc \
    LorrisAnalyzer/DataWidgets/ScriptWidget/examples/examples.qrc \
    LorrisProgrammer/programmericons.qrc \
    LorrisAnalyzer/DataWidgets/RotationWidget/models.qrc \
    LorrisProgrammer/programmers/atsamprogrammer.qrc

include(../dep/qtsingleapplication/qtsingleapplication.pri)

RC_FILE = winicon.rc

OTHER_FILES += \
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
    LorrisAnalyzer/DataWidgets/ScriptWidget/examples/input.py \
    LorrisAnalyzer/DataWidgets/ScriptWidget/examples/rotation.js \
    LorrisAnalyzer/DataWidgets/ScriptWidget/examples/rotation.py

PRECOMPILED_HEADER  = pch.h
precompile_header:!isEmpty(PRECOMPILED_HEADER) {
    DEFINES += USING_PCH
}

win32 {
    CONFIG -= flat
    CONFIG += libenjoy
    CONFIG += libyb

    CONFIG(debug, debug|release) {
        LIBS += -lqwtd
        QWT_L=qwtd.dll
    } else {
        LIBS += -lqwt
        QWT_L=qwt.dll
    }

    cross_build:QMAKE_POST_LINK += cp \""$$PWD/../dep/qwt/lib/$$QWT_L\"" \""$$DESTDIR/$$QWT_L\"" ;
    else:QMAKE_POST_LINK += copy \""$$PWD\\..\\dep\\qwt\\lib\\$$QWT_L\"" \""$$DESTDIR\\$$QWT_L\"" &

    DEFINES += QT_DLL QWT_DLL QESP_NO_QT4_PRIVATE

    HEADERS += \
        ../dep/qextserialport/src/qextwineventnotifier_p.h \
        misc/updater.h
    SOURCES += \
        ../dep/qextserialport/src/qextserialenumerator_win.cpp \
        ../dep/qextserialport/src/qextwineventnotifier_p.cpp \
        ../dep/qextserialport/src/qextserialport_win.cpp \
        misc/updater.cpp

    LIBS += -lsetupapi -lwinmm -lole32 -ladvapi32 -luser32
}
unix:!macx:!symbian {
    CONFIG += libenjoy libyb

    system_qwt {
        LIBS += -lqwt
    } else {
        LIBS += -lqwt_lorris
    }

    SOURCES += \
        ../dep/qextserialport/src/qextserialenumerator_unix.cpp \
        ../dep/qextserialport/src/qextserialport_unix.cpp

    QMAKE_POST_LINK = mkdir \
        "$$DESTDIR/translations" 2> /dev/null \
        ; \
        cp \
        ../translations/*.qm \
        "$$DESTDIR/translations/ 2> /dev/null" || true

    translations.path = /usr/local/share/lorris/
    translations.files = ../translations/Lorris.*.qm
    target.path = /usr/local/bin/
    INSTALLS += target translations
}
macx {
    #Note: Mac apps are designed as bundles, so using shared library is aganst mac conventions
    LIBS += -lqwt_lorris -framework IOKit -framework CoreFoundation
    #CONFIG += libyb
    ICON = icon.icns
    QT += macextras

    SOURCES += \
        ../dep/qextserialport/src/qextserialenumerator_osx.cpp \
        ../dep/qextserialport/src/qextserialport_unix.cpp

    #translations.path = /usr/share/lorris/
    #translations.files = ../translations/Lorris.*.qm
    target.path = /Applications/
    INSTALLS += target translations
}

python {
    LIBS += -L"$$PWD/../dep/pythonqt" -lPythonQt
    DEFINES += WITH_PYTHON
    SOURCES += LorrisAnalyzer/DataWidgets/ScriptWidget/engines/pythonengine.cpp
    HEADERS += LorrisAnalyzer/DataWidgets/ScriptWidget/engines/pythonengine.h

    win32 {
        cross_build:QMAKE_POST_LINK += cp \""$$PWD/../dep/pythonqt/PythonQt.dll\"" \""$$DESTDIR/PythonQt.dll\"";
        else:QMAKE_POST_LINK += copy \""$$PWD\\..\\dep\\pythonqt\\PythonQt.dll\"" \""$$DESTDIR\\PythonQt.dll\"" &
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
        LorrisProgrammer/modes/shupitojtag.cpp \
        LorrisProgrammer/programmers/flipprogrammer.cpp \
        LorrisProgrammer/programmers/stm32programmer.cpp \
        connection/stm32connection.cpp

    HEADERS += \
        connection/genericusbconn.h \
        connection/usbacmconn.h \
        connection/usbshupito22conn.h \
        connection/usbshupito23conn.h \
        LorrisProgrammer/modes/shupitojtag.h \
        LorrisProgrammer/programmers/flipprogrammer.h \
        LorrisProgrammer/programmers/stm32programmer.h \
        connection/stm32connection.h \
        connection/stm32defines.h
}

kate_editor:unix {
    !contains(QT_MAJOR_VERSION, 5) {
        DEFINES += USE_KATE
        LIBS += -lktexteditor -lkdecore
    }
}

qsci_editor {
    DEFINES += USE_QSCI
    LIBS += -L"$$PWD/../dep/qscintilla2/lib"
    INCLUDEPATH += "$$PWD/../dep/qscintilla2/"
    CONFIG(debug, debug|release) {
        LIBS += -lqscintilla2_lorrisd
        QSCI_L=qscintilla2_lorrisd.dll
    } else {
        LIBS += -lqscintilla2_lorris
        QSCI_L=qscintilla2_lorris.dll
    }

    win32 {
        cross_build:QMAKE_POST_LINK += cp \""$$PWD/../dep/qscintilla2/lib/$$QSCI_L\"" \""$$DESTDIR/$$QSCI_L\"" ;
        else:QMAKE_POST_LINK += copy \""$$PWD\\..\\dep\\qscintilla2\\lib\\$$QSCI_L\"" \""$$DESTDIR\\$$QSCI_L\"" &
    }
}

qsci_system:unix {
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

opengl {
    QT += opengl
    DEFINES += HAVE_OPENGL

    win32 {
        LIBS += -lopengl32
    }

    SOURCES += LorrisAnalyzer/DataWidgets/RotationWidget/renderwidget.cpp \
        LorrisAnalyzer/DataWidgets/RotationWidget/objfileloader.cpp \
        LorrisAnalyzer/DataWidgets/RotationWidget/glutils.cpp \
        LorrisAnalyzer/DataWidgets/RotationWidget/glmodel.cpp
    HEADERS += LorrisAnalyzer/DataWidgets/RotationWidget/renderwidget.h \
        LorrisAnalyzer/DataWidgets/RotationWidget/objfileloader.h \
        LorrisAnalyzer/DataWidgets/RotationWidget/glutils.h \
        LorrisAnalyzer/DataWidgets/RotationWidget/glmodel.h
}
