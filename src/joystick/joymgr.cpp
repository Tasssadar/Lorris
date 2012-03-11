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

#include <QDebug>
#include "joymgr.h"

JoyMgr::JoyMgr()
{
    // SDL needs video to be enabled
    SDL_Init(SDL_INIT_VIDEO | SDL_INIT_JOYSTICK);

    SDL_JoystickEventState(SDL_ENABLE);

    updateJoystickNames();

    m_thread = new JoyThread();
    m_thread->start();
}

JoyMgr::~JoyMgr()
{
    m_thread->stop();
    delete m_thread;

    for(QHash<int, Joystick*>::iterator itr = m_joysticks.begin(); itr != m_joysticks.end(); ++itr)
        delete *itr;

    SDL_QuitSubSystem(SDL_INIT_VIDEO | SDL_INIT_JOYSTICK);
}

void JoyMgr::updateJoystickNames()
{
    int cnt = SDL_NumJoysticks();

    qDebug() << "Joystick count: " << cnt;
    for(int i = 0; i < cnt; ++i)
    {
        m_names.insert(i, SDL_JoystickName(i));
        qDebug() << i << ": " << m_names[i];
    }
}

Joystick *JoyMgr::getJoystick(int id)
{
    QMutexLocker locker(&m_joy_lock);

    if(!m_names.contains(id))
        return NULL;

    if(m_joysticks.contains(id))
        return m_joysticks[id];

    Joystick *joy = new Joystick(id);

    if(!joy->open())
    {
        delete joy;
        return NULL;
    }

    m_joysticks[id] = joy;
    return joy;
}

QStringList JoyMgr::getNamesList()
{
    QStringList list;
    for(QHash<int, QString>::iterator itr = m_names.begin(); itr != m_names.end(); ++itr)
        list << *itr;
    return list;
}
