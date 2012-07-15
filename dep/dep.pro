TEMPLATE = subdirs
CONFIG += ordered

include(../config.pri)

SUBDIRS = qextserialport

!system_qwt {
    SUBDIRS += qwt
}

python:unix {
    SUBDIRS += pythonqt
}



