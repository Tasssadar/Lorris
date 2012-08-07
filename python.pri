python {
    unix {
        LIBS += $$system(pkg-config --libs python-$${PYTHON_VERSION} --silence-errors)
        QMAKE_CXXFLAGS += $$system(pkg-config --cflags python-$${PYTHON_VERSION} --silence-errors)
    }
    win32 {
        INCLUDEPATH += "$$PWD/dep/python2.7/"
    }
}
