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

#ifndef JOYMGR_H
#define JOYMGR_H

#include <QHash>
#include <QMutex>
#include <QStringList>

#ifdef Q_OS_WIN
    #include <SDL.h>
#else // use lib from OS on other systems
    #include <SDL/SDL.h>
#endif

#include "singleton.h"
#include "joystick.h"
#include "joythread.h"

class JoyMgr : public Singleton<JoyMgr>
{
public:
    JoyMgr();
    ~JoyMgr();

    void updateJoystickNames();

    bool isEmpty() { return m_names.isEmpty(); }
    const QHash<int, QString>& getNames() { return m_names; }
    QStringList getNamesList();

    Joystick *getJoystick(int id);

    bool hasJoystick(int id)
    {
        m_joy_lock.lock();
        bool res = m_joysticks.contains(id);
        m_joy_lock.unlock();
        return res;
    }

private:
    QHash<int, Joystick*> m_joysticks;
    QHash<int, QString> m_names;

    QMutex m_joy_lock;

    JoyThread *m_thread;
};

#define sJoyMgr JoyMgr::GetSingleton()

#endif // JOYMGR_H
