# comment out following line to disable python in Lorris
!win32-msvc* {
    CONFIG += python
}

python {
    # Change this variable to your python version (2.5, 2.6, 2.7)
    win32:PYTHON_VERSION=27
    unix:PYTHON_VERSION=2.7
}

unix {
    # Uncomment following line if you want to use system-wide installation of libqwt-dev.
    # You must have version >= 6.0.0 installed
    #CONFIG += system_qwt

    # Comment out following line if you want to disable Kate editor for ScriptWidget in analyzer
    CONFIG += kate_editor
}

# Comment out following line if you want to disable QScintilla editor for ScriptWidget
CONFIG += qsci_editor

