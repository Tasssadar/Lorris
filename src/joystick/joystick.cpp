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
