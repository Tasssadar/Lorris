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

    bool isEmpty() const { return m_names.isEmpty(); }
    QStringList getNamesList();

    Joystick *getJoystick(quint32 id);

    bool hasJoystick(quint32 id)
    {
        m_joy_lock.lockForRead();
        bool res = m_joysticks.contains(id);
        m_joy_lock.unlock();
        return res;
    }

public slots:
    void removeJoystick(JoystickPrivate *joy);

protected:
    JoystickPrivate *getJoystickPrivate(int id);

private slots:
    void enumerate();

private:
    QHash<quint32, JoystickPrivate*> m_joysticks;
    QHash<quint32, QString> m_names;

    QReadWriteLock m_joy_lock;

    JoyThread m_thread;
    QTimer m_enum_timer;
};

#define sJoyMgr JoyMgr::GetSingleton()

#endif // JOYMGR_H
