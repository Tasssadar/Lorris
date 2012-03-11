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

#ifndef JOYSTICK_H
#define JOYSTICK_H

#include <QObject>
#include <QMutex>

#ifdef Q_OS_WIN
    #include <SDL.h>
#else // use lib from OS on other systems
    #include <SDL/SDL.h>
#endif

class Joystick : public QObject
{
    Q_OBJECT

Q_SIGNALS:
    void axisEvent(int id, qint16 val);
    void ballEvent(int id, qint16 x, qint16 y);
    void hatEvent(int id, quint8 val);
    void buttonEvent(int id, quint8 state);

public:
    Joystick(int id, QObject *parent = 0);
    ~Joystick();

    bool open();

    void __axisEvent(int id, qint16 val)
    {
        m_lock.lock();
        m_axes[id] = val;
        m_lock.unlock();

        emit axisEvent(id, val);
    }

    void __ballEvent(int id, qint16 x, qint16 y)
    {
        emit ballEvent(id, x, y);
    }

    void __hatEvent(int id, quint8 val)
    {
        emit hatEvent(id, val);
    }

    void __buttonEvent(int id, quint8 state)
    {
        m_lock.lock();
        m_buttons[id] = state;
        m_lock.unlock();

        emit buttonEvent(id, state);
    }

public slots:
    int getNumAxes() const { return m_num_axes; }
    int getNumBalls() const { return m_num_balls; }
    int getNumHats() const { return m_num_hats; }
    int getNumButtons() const { return m_num_buttons; }

    int getAxisVal(int id)
    {
        QMutexLocker locker(&m_lock);
        return m_axes[id];
    }

    quint8 getButtonVal(int id)
    {
        QMutexLocker locker(&m_lock);
        return m_buttons[id];
    }

private:
    int m_id;
    SDL_Joystick *m_joy;

    int m_num_axes;
    int m_num_balls;
    int m_num_hats;
    int m_num_buttons;

    int *m_axes;
    quint8 *m_buttons;

    QMutex m_lock;
};

#endif // JOYSTICK_H
