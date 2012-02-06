#
DESTDIR = $$PWD/lib
TEMPLATE = subdirs
CONFIG   += ordered
SUBDIRS  = src \
           examples/enumerator \
           examples/event

