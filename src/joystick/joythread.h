/**********************************************
**    This file is part of Lorris
**    http://tasssadar.github.com/Lorris/
**
**    See README and COPYING
***********************************************/

#ifndef JOYTHREAD_H
#define JOYTHREAD_H

#include <QThread>

class JoyThread : public QThread
{
    Q_OBJECT

public:
    explicit JoyThread(QObject *parent = 0);
    
    void setStopped(bool stop, bool waitForIt = false);

protected:
    void run();

private:
    volatile bool m_run;
};

#endif // JOYTHREAD_H
