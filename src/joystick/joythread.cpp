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
    m_run = true;
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
                    Joystick *joy = sJoyMgr.getJoystick(event.jaxis.which, false);
                    if(!joy)
                        continue;
                    joy->axisEvent(event.jaxis.axis, event.jaxis.value);
                    break;
                }
                case SDL_JOYHATMOTION:
                {
                    Joystick *joy = sJoyMgr.getJoystick(event.jhat.which, false);
                    if(!joy)
                        continue;
                    joy->hatEvent(event.jhat.hat, event.jhat.value);
                    break;
                }
                case SDL_JOYBALLMOTION:
                {
                    Joystick *joy = sJoyMgr.getJoystick(event.jball.which, false);
                    if(!joy)
                        continue;
                    joy->ballEvent(event.jball.ball, event.jball.xrel, event.jball.yrel);
                    break;
                }
                case SDL_JOYBUTTONDOWN:
                case SDL_JOYBUTTONUP:
                {
                    Joystick *joy = sJoyMgr.getJoystick(event.jbutton.which, false);
                    if(!joy)
                        continue;
                    joy->buttonEvent(event.jbutton.button, event.jbutton.state);
                    break;
                }
                default: continue;
            }
        }
        msleep(50);
    }
}

void JoyThread::stop()
{
    m_run = false;
    wait(500);
}
