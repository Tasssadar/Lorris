/**********************************************
**    This file is part of Lorris
**    http://tasssadar.github.com/Lorris/
**
**    See README and COPYING
***********************************************/

#ifndef JOYTHREAD_H
#define JOYTHREAD_H

#include <QThread>

#ifdef Q_OS_WIN
    #include <SDL.h>
#else // use lib from OS on other systems
    #include <SDL/SDL.h>
#endif

class JoyThread : public QThread
{
    Q_OBJECT

public:
    explicit JoyThread(QObject *parent = 0);
    
    void stop();

protected:
    void run();

private:
    volatile bool m_run;
};

#endif // JOYTHREAD_H
