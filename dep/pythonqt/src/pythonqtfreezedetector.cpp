#include <QTimer>
#include <PythonQt.h>
#include "pythonqtfreezedetector.h"

static int killPythonExecution(void *)
{
    PyErr_SetString(PyExc_RuntimeError, "FreezeDetector: killing, has been in python code for too long. You can change the timeout in the global settings.");
    return -1;
}

PythonQtFreezeDetector::PythonQtFreezeDetector(int timeout, QThread *thread) : QObject()
{
    QTimer *t = new QTimer(NULL);
    connect(t, SIGNAL(timeout()), this, SLOT(timeout()), Qt::DirectConnection);
    t->setSingleShot(true);
    t->setInterval(timeout);
    t->moveToThread(thread);

    connect(this, SIGNAL(destroyed()), t, SLOT(deleteLater()));
    connect(this, SIGNAL(startTimer()), t, SLOT(start()));
    emit startTimer();
}

void PythonQtFreezeDetector::timeout()
{
    Py_AddPendingCall(&killPythonExecution, NULL);
}
