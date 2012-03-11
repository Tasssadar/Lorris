/****************************************************************************
**
**    This file is part of Lorris.
**    Copyright (C) 2012 Vojtěch Boček
**
**    Contact: <vbocek@gmail.com>
**             https://github.com/Tasssadar
**
**    Lorris is free software: you can redistribute it and/or modify
**    it under the terms of the GNU General Public License as published by
**    the Free Software Foundation, either version 3 of the License, or
**    (at your option) any later version.
**
**    Lorris is distributed in the hope that it will be useful,
**    but WITHOUT ANY WARRANTY; without even the implied warranty of
**    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
**    GNU General Public License for more details.
**
**    You should have received a copy of the GNU General Public License
**    along with Lorris.  If not, see <http://www.gnu.org/licenses/>.
**
****************************************************************************/

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
                    Joystick *joy = sJoyMgr.getJoystick(event.jaxis.which);
                    if(!joy)
                        continue;
                    joy->axisEvent(event.jaxis.axis, event.jaxis.value);
                    break;
                }
                case SDL_JOYHATMOTION:
                {
                    Joystick *joy = sJoyMgr.getJoystick(event.jhat.which);
                    if(!joy)
                        continue;
                    joy->hatEvent(event.jhat.hat, event.jhat.value);
                    break;
                }
                case SDL_JOYBALLMOTION:
                {
                    Joystick *joy = sJoyMgr.getJoystick(event.jball.which);
                    if(!joy)
                        continue;
                    joy->ballEvent(event.jball.ball, event.jball.xrel, event.jball.yrel);
                    break;
                }
                case SDL_JOYBUTTONDOWN:
                case SDL_JOYBUTTONUP:
                {
                    Joystick *joy = sJoyMgr.getJoystick(event.jbutton.which);
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
