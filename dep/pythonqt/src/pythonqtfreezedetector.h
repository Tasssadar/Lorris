#ifndef PYTHONQTFREEZEDETECTOR_H
#define PYTHONQTFREEZEDETECTOR_H

#include <QObject>
#include <QTimer>

class QThread;

class PythonQtFreezeDetector : public QObject
{
    Q_OBJECT
public:
    explicit PythonQtFreezeDetector(int timeout, QThread *thread);

signals:
    void startTimer();

private slots:
    void timeout();
};

#endif // PYTHONQTFREEZEDETECTOR_H
