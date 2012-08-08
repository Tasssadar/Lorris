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

    m_thread = new JoyThread();
    m_thread->start();
}

JoyMgr::~JoyMgr()
{
    m_thread->stop();
    delete m_thread;

    for(QHash<int, JoystickPrivate*>::iterator itr = m_joysticks.begin(); itr != m_joysticks.end(); ++itr)
        delete *itr;

    SDL_Quit();
}

void JoyMgr::updateJoystickNames()
{
    // Reinit joystick system to load newly connected joysticks
    if(m_joysticks.empty())
    {
        SDL_QuitSubSystem(SDL_INIT_JOYSTICK);
        SDL_InitSubSystem(SDL_INIT_JOYSTICK);
        SDL_JoystickEventState(SDL_ENABLE);
    }

    int cnt = SDL_NumJoysticks();
    qDebug() << "Joystick count: " << cnt;

    m_names.clear();
    for(int i = 0; i < cnt; ++i)
    {
        m_names.insert(i, SDL_JoystickName(i));
        qDebug() << i << ": " << m_names[i];
    }
}

Joystick *JoyMgr::getJoystick(int id, bool create)
{
    QMutexLocker locker(&m_joy_lock);

    if(!m_names.contains(id))
        return NULL;

    if(m_joysticks.contains(id))
        return new Joystick(m_joysticks[id]);

    else if(!create)
        return NULL;

    SDL_Joystick *sdl_joy = SDL_JoystickOpen(id);
    if(!sdl_joy)
        return NULL;

    JoystickPrivate *joy = new JoystickPrivate(id, sdl_joy);
    connect(joy, SIGNAL(removeJoystick(JoystickPrivate*)), SLOT(removeJoystick(JoystickPrivate*)));

    m_joysticks[id] = joy;
    return new Joystick(joy);
}

QStringList JoyMgr::getNamesList()
{
    QStringList list;
    for(QHash<int, QString>::iterator itr = m_names.begin(); itr != m_names.end(); ++itr)
        list << *itr;
    return list;
}

void JoyMgr::removeJoystick(JoystickPrivate *joy)
{
    if(joy->isUsed())
        return;

    m_joysticks.remove(joy->getId());
    delete joy;
}

JoystickPrivate *JoyMgr::getJoystickPrivate(int id)
{
    QMutexLocker locker(&m_joy_lock);

    QHash<int, JoystickPrivate*>::iterator itr = m_joysticks.find(id);
    if(itr != m_joysticks.end())
        return *itr;

    return NULL;
}
