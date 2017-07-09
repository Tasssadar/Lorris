/**********************************************
**    This file is part of Lorris
**    http://tasssadar.github.com/Lorris/
**
**    See README and COPYING
***********************************************/

#ifndef JOYMGR_H
#define JOYMGR_H

#include "joystick.h"

#ifdef HAVE_LIBENJOY

#include <QHash>
#include <QReadWriteLock>
#include <QStringList>
#include <QMap>

#include "../misc/singleton.h"
#include "joythread.h"

struct libenjoy_context;

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
    QList<quint32> getIdList();

    Joystick *getJoystick(quint32 id);
    Joystick *getFirstJoystick();

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
    Joystick *openNewJoystick(quint32 id);

    QHash<quint32, JoystickPrivate*> m_joysticks;
    QMap<quint32, QString> m_names;

    QReadWriteLock m_joy_lock;

    JoyThread m_thread;
    QTimer m_enum_timer;
    libenjoy_context *m_context;
};

#else // HAVE_LIBENJOY

class JoyMgr : public QObject, public Singleton<JoyMgr>
{
    Q_OBJECT
public:
    JoyMgr();
    ~JoyMgr();

    void updateJoystickNames() { }

    bool isEmpty() const { return true; }
    QStringList getNamesList() { return QStringList(); }
    QList<quint32> getIdList() { return QList<quint32>(); }

    Joystick *getJoystick(quint32) { return NULL; }
    Joystick *getFirstJoystick() { return NULL; }

    bool hasJoystick(quint32)
    {
        return false;
    }
};

#endif //HAVE_LIBENJOY

#define sJoyMgr JoyMgr::GetSingleton()

#endif // JOYMGR_H
