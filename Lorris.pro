TEMPLATE = subdirs
CONFIG += ordered

SUBDIRS = dep \
          src
src.depends = dep

CONFIG(debug, debug|release):TARGET = bin/debug/Lorris
else:TARGET = bin/release/Lorris
