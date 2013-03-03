TEMPLATE = subdirs
CONFIG += ordered

SUBDIRS = dep \
          src
src.depends = dep

CONFIG(debug, debug|release):DESTDIR = "$$PWD/bin/debug"
else:DESTDIR = "$$PWD/bin/release"

TARGET = "Lorris"
