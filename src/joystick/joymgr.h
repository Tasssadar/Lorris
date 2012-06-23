/**********************************************
**    This file is part of Lorris
**    http://tasssadar.github.com/Lorris/
**
**    See README and COPYING
***********************************************/

#ifndef JOYMGR_H
#define JOYMGR_H

#include <QHash>
#include <QMutex>
#include <QStringList>

#if defined Q_OS_WIN || defined Q_OS_MAC 
    #include <SDL.h>
#else // use lib from OS on other systems
    #include <SDL/SDL.h>
#endif

#include "../singleton.h"
#include "joystick.h"
#include "joythread.h"

class JoyMgr : public QObject, public Singleton<JoyMgr>
{
    Q_OBJECT
public:
    JoyMgr();
    ~JoyMgr();

    void updateJoystickNames();

    bool isEmpty() { return m_names.isEmpty(); }
    const QHash<int, QString>& getNames() { return m_names; }
    QStringList getNamesList();

    Joystick *getJoystick(int id, bool create = true);

    bool hasJoystick(int id)
    {
        m_joy_lock.lock();
        bool res = m_joysticks.contains(id);
        m_joy_lock.unlock();
        return res;
    }

public slots:
    void removeJoystick(Joystick *joy);

private:
    QHash<int, Joystick*> m_joysticks;
    QHash<int, QString> m_names;

    QMutex m_joy_lock;

    JoyThread *m_thread;
};

#define sJoyMgr JoyMgr::GetSingleton()

#endif // JOYMGR_H
