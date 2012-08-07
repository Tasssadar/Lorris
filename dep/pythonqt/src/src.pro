# --------- PythonQt profile -------------------
# Last changed by $Author: florian $
# $Id: PythonQt.pro 35381 2006-03-16 13:05:52Z florian $
# $Source$
# --------------------------------------------------

TARGET   = PythonQt
TEMPLATE = lib


DESTDIR    = "$$PWD/../"

CONFIG += qt
win32:CONFIG += dll
else:CONFIG+= static
CONFIG -= flat

include ( ../build/common.prf )  
include ( ../build/python.prf )  

include ( src.pri )  
