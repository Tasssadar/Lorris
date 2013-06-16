TEMPLATE = subdirs
CONFIG += ordered

include(../config.pri)

win32|!system_qwt:SUBDIRS += qwt
!win32:SUBDIRS += qextserialport
python:SUBDIRS += pythonqt
qsci_editor:SUBDIRS += qscintilla2
