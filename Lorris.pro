TEMPLATE = subdirs
CONFIG += ordered

win32 {
    SUBDIRS = src
}

unix {
    SUBDIRS = dep \
              src
    src.depends = dep
}
