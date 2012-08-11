/**********************************************
**    This file is part of Lorris
**    http://tasssadar.github.com/Lorris/
**
**    See README and COPYING
***********************************************/

#include <QDebug>
#include <libenjoy.h>
#include "joymgr.h"

JoyMgr::JoyMgr() : QObject()
{
    libenjoy_init();
    libenjoy_enumerate();

    updateJoystickNames();

    m_enum_timer.start(100);
    connect(&m_enum_timer, SIGNAL(timeout()), SLOT(enumerate()));
}

JoyMgr::~JoyMgr()
{
    m_thread.setStopped(true, true);

    for(int i = 0; i < m_joysticks.size(); ++i)
        delete m_joysticks[i];

    libenjoy_close();
}

void JoyMgr::enumerate()
{
    libenjoy_enumerate();
}

void JoyMgr::updateJoystickNames()
{
    QWriteLocker locker(&m_joy_lock);

    libenjoy_joy_info_list *info = libenjoy_get_info_list();

    m_names.clear();
    for(quint32 i = 0; i < info->count; ++i)
    {
        m_names[info->list[i]->id] = info->list[i]->name;
        qDebug("%d: %s", info->list[i]->id, info->list[i]->name);
    }

    libenjoy_free_info_list(info);
}

Joystick *JoyMgr::getJoystick(quint32 id)
{
    {
        QReadLocker locker(&m_joy_lock);

        if(!m_names.contains(id))
            return NULL;

        QHash<quint32, JoystickPrivate*>::iterator itr = m_joysticks.find(id);
        if(itr != m_joysticks.end())
            return new Joystick(*itr);
    }

    {
        QWriteLocker locker(&m_joy_lock);

        libenjoy_joystick *joy = libenjoy_open_joystick(id);
        if(!joy)
            return NULL;

        JoystickPrivate *joy_priv = new JoystickPrivate(joy);
        connect(joy_priv, SIGNAL(removeJoystick(JoystickPrivate*)), SLOT(removeJoystick(JoystickPrivate*)));

        m_thread.setStopped(false);

        m_joysticks[id] = joy_priv;
        return new Joystick(joy_priv);
    }
}

QStringList JoyMgr::getNamesList()
{
    QStringList list;
    for(int i = 0; i < m_names.size(); ++i)
        list << m_names[i];
    return list;
}

void JoyMgr::removeJoystick(JoystickPrivate *joy)
{
    if(joy->isUsed())
        return;

    QWriteLocker locker(&m_joy_lock);

    m_joysticks.remove(joy->getId());
    delete joy;

    m_thread.setStopped(m_joysticks.empty());
}

JoystickPrivate *JoyMgr::getJoystickPrivate(int id)
{
    QReadLocker locker(&m_joy_lock);
    return m_joysticks[id];
}
