/**********************************************
**    This file is part of Lorris
**    http://tasssadar.github.com/Lorris/
**
**    See README and COPYING
***********************************************/

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
    SDL_Event event;
    while(m_run)
    {
        while(SDL_PollEvent(&event))
        {
            switch(event.type)
            {
                case SDL_JOYAXISMOTION:
                {
                    JoystickPrivate *joy = sJoyMgr.getJoystickPrivate(event.jaxis.which);
                    if(!joy)
                        continue;
                    joy->axisEvent(event.jaxis.axis, event.jaxis.value);
                    break;
                }
                case SDL_JOYHATMOTION:
                {
                    JoystickPrivate *joy = sJoyMgr.getJoystickPrivate(event.jhat.which);
                    if(!joy)
                        continue;
                    joy->hatEvent(event.jhat.hat, event.jhat.value);
                    break;
                }
                case SDL_JOYBALLMOTION:
                {
                    JoystickPrivate *joy = sJoyMgr.getJoystickPrivate(event.jball.which);
                    if(!joy)
                        continue;
                    joy->ballEvent(event.jball.ball, event.jball.xrel, event.jball.yrel);
                    break;
                }
                case SDL_JOYBUTTONDOWN:
                case SDL_JOYBUTTONUP:
                {
                    JoystickPrivate *joy = sJoyMgr.getJoystickPrivate(event.jbutton.which);
                    if(!joy)
                        continue;
                    joy->buttonEvent(event.jbutton.button, event.jbutton.state);
                    break;
                }
                default: continue;
            }
        }
        msleep(1);
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
