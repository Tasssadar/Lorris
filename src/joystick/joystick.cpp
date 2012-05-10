/**********************************************
**    This file is part of Lorris
**    http://tasssadar.github.com/Lorris/
**
**    See README and COPYING
***********************************************/

#include "joystick.h"

Joystick::Joystick(int id, QObject *parent) :
    QObject(parent)
{
    m_id = id;
    m_joy = NULL;
}

Joystick::~Joystick()
{
    if(m_joy)
        SDL_JoystickClose(m_joy);
}

bool Joystick::open()
{
    m_joy = SDL_JoystickOpen(m_id);

    if(!m_joy)
        return false;

    m_num_axes = SDL_JoystickNumAxes(m_joy);
    m_num_hats = SDL_JoystickNumHats(m_joy);
    m_num_balls = SDL_JoystickNumBalls(m_joy);
    m_num_buttons = SDL_JoystickNumButtons(m_joy);

    m_axes.resize(m_num_axes, 0);
    m_buttons.resize(m_num_buttons, 0);
    return true;
}
