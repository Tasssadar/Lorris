TEMPLATE = subdirs
CONFIG += ordered

SUBDIRS = dep \
          src
src.depends = dep

CONFIG(debug, debug|release):TARGET = "$$PWD/bin/debug/Lorris"
else:TARGET = "$$PWD/bin/release/Lorris"
