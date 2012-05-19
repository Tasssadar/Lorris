TEMPLATE = subdirs
CONFIG += ordered
SUBDIRS = dep \
          src

# Use .depends to specify that a project depends on another.
src.depends = dep
