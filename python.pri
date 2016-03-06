python {
    unix {
        LIBS += $$system(pkg-config --libs python-$${PYTHON_VERSION} --silence-errors)
        QMAKE_CXXFLAGS += $$system(pkg-config --cflags python-$${PYTHON_VERSION} --silence-errors)
    }
    win32 {
        INCLUDEPATH += "$$PWD/dep/python2.7/"
        !contains(QMAKE_HOST.arch, x86_64) {
            LIBS += -L"$$PWD/dep/python2.7/lib/" -lpython27
        } else {
            LIBS += -L"$$PWD/dep/python2.7/lib64/" -lpython27
            DEFINES += MS_WIN64
        }
    }
}
