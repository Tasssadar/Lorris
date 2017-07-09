win32 {
    # Comment out following line if you want to disable QScintilla editor for ScriptWidget
    CONFIG += qsci_editor

    # comment out following line to disable python in Lorris
    # There is no debug python library on windows, so do not compile it in debug mode
    CONFIG(release, debug|release): CONFIG += python
}

unix {
    # Uncomment following line if you want to use system-wide installation of libqwt-dev.
    # You must have version >= 6.0.0 installed
    #CONFIG += system_qwt

    # Comment out following line if you want to disable Kate editor for ScriptWidget in analyzer
    CONFIG += kate_editor

    # Comment out following lines if you want to disable QScintilla editor for ScriptWidget
    # Lorris' qscintilla
    CONFIG += qsci_editor

    # System qscintilla lib
    #CONFIG += qsci_system

    # comment out following line to disable python in Lorris
    CONFIG += python
}

python {
    # Change this variable to your python version (2.5, 2.6, 2.7)
    win32:PYTHON_VERSION=27
    unix:PYTHON_VERSION=2.7
}

# Comment out following line if you do not want joystick support
!macx:CONFIG += joystick

# Comment out following line if you don't want parts which require OpenGL
CONFIG += opengl
