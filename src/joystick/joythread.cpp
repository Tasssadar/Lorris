/**********************************************
**    This file is part of Lorris
**    http://tasssadar.github.com/Lorris/
**
**    See README and COPYING
***********************************************/

#include <libenjoy.h>

#include "joythread.h"
#include "joymgr.h"
#include "joystick.h"

JoyThread::JoyThread(QObject *parent) :
    QThread(parent)
{
    m_run = false;
}

void JoyThread::run()
{
    libenjoy_event ev;
    while(m_run)
    {
        while(libenjoy_poll(&ev) == 0)
        {
            JoystickPrivate *joy = sJoyMgr.getJoystickPrivate(ev.joy_id);
            if(!joy)
                continue;

            switch(ev.type)
            {
                case LIBENJOY_EV_AXIS:
                {
                    joy->axisEvent(ev.part_id, ev.data);
                    break;
                }
                case LIBENJOY_EV_BUTTON:
                {
                    joy->buttonEvent(ev.part_id, ev.data);
                    break;
                }
                default: continue;
            }
        }
        msleep(10);
    }
}

void JoyThread::setStopped(bool stop, bool waitForIt)
{
    if(stop == !m_run)
        return;

    m_run = !stop;
    if(m_run)          start();
    else if(waitForIt) wait(500);
}
