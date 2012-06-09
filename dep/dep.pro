TEMPLATE = subdirs
CONFIG += ordered

include(../config.pri)

SUBDIRS = qwt \
          qextserialport

python:unix {
    SUBDIRS += pythonqt
}



