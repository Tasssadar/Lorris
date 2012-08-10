/**********************************************
**    This file is part of Lorris
**    http://tasssadar.github.com/Lorris/
**
**    See README and COPYING
***********************************************/

#include <QDebug>
#include "joymgr.h"

JoyMgr::JoyMgr() : QObject()
{
    // SDL needs video initialized
    SDL_Init(SDL_INIT_VIDEO);

    updateJoystickNames();
}

JoyMgr::~JoyMgr()
{
    m_thread.setStopped(true, true);

    for(size_t i = 0; i < m_joysticks.size(); ++i)
        delete m_joysticks[i];

    SDL_Quit();
}

void JoyMgr::updateJoystickNames()
{
    QWriteLocker locker(&m_joy_lock);

    // Reinit joystick system to load newly connected joysticks
    if(m_joysticks.empty())
    {
        SDL_QuitSubSystem(SDL_INIT_JOYSTICK);
        SDL_InitSubSystem(SDL_INIT_JOYSTICK);
        SDL_JoystickEventState(SDL_ENABLE);
    }

    int cnt = SDL_NumJoysticks();

    // Maybe if(m_joystick.size() > cnt) then delete the joysticks above cnt?
    // I think SDL keeps joystick id alive no mater what, as long as it is open.
    // I only have one joystick, so can't test it. Assert it is.
    for(size_t i = cnt; i < m_joysticks.size(); ++i)
        Q_ASSERT(!m_joysticks[i]);

    m_joysticks.resize(cnt, NULL);

    m_names.resize(cnt);
    for(int i = 0; i < cnt; ++i)
        m_names[i] = SDL_JoystickName(i);

    m_thread.setStopped(cnt == 0);
}

Joystick *JoyMgr::getJoystick(int id)
{
    {
        QReadLocker locker(&m_joy_lock);

        if(m_names.size() <= (size_t)id)
            return NULL;

        if(m_joysticks[id])
            return new Joystick(m_joysticks[id]);
    }

    {
        QWriteLocker locker(&m_joy_lock);

        SDL_Joystick *sdl_joy = SDL_JoystickOpen(id);
        if(!sdl_joy)
            return NULL;

        JoystickPrivate *joy = new JoystickPrivate(id, sdl_joy);
        connect(joy, SIGNAL(removeJoystick(JoystickPrivate*)), SLOT(removeJoystick(JoystickPrivate*)));

        m_joysticks[id] = joy;
        return new Joystick(joy);
    }
}

QStringList JoyMgr::getNamesList()
{
    QStringList list;
    for(size_t i = 0; i < m_names.size(); ++i)
        list << m_names[i];
    return list;
}

void JoyMgr::removeJoystick(JoystickPrivate *joy)
{
    if(joy->isUsed())
        return;

    QWriteLocker locker(&m_joy_lock);

    m_joysticks[joy->getId()] = NULL;
    delete joy;
}

JoystickPrivate *JoyMgr::getJoystickPrivate(int id)
{
    QReadLocker locker(&m_joy_lock);
    return m_joysticks[id];
}
