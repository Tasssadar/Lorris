/**********************************************
**    This file is part of Lorris
**    http://tasssadar.github.com/Lorris/
**
**    See README and COPYING
***********************************************/

#ifndef JOYMGR_H
#define JOYMGR_H

#include <QHash>
#include <QReadWriteLock>
#include <QStringList>

#if defined Q_OS_WIN || defined Q_OS_MAC 
    #include <SDL.h>
#else // use lib from OS on other systems
    #include <SDL/SDL.h>
#endif

#include "../misc/singleton.h"
#include "joystick.h"
#include "joythread.h"

class JoyMgr : public QObject, public Singleton<JoyMgr>
{
    Q_OBJECT

    friend class JoyThread;
public:
    JoyMgr();
    ~JoyMgr();

    void updateJoystickNames();

    bool isEmpty() const { return m_names.empty(); }
    const std::vector<QString>& getNames() const { return m_names; }
    QStringList getNamesList();

    Joystick *getJoystick(int id);

    bool hasJoystick(int id)
    {
        m_joy_lock.lockForRead();
        bool res = (m_joysticks[id] != NULL);
        m_joy_lock.unlock();
        return res;
    }

public slots:
    void removeJoystick(JoystickPrivate *joy);

protected:
    JoystickPrivate *getJoystickPrivate(int id);

private:
    std::vector<JoystickPrivate*> m_joysticks;
    std::vector<QString> m_names;

    QReadWriteLock m_joy_lock;

    JoyThread m_thread;
};

#define sJoyMgr JoyMgr::GetSingleton()

#endif // JOYMGR_H
