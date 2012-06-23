/**********************************************
**    This file is part of Lorris
**    http://tasssadar.github.com/Lorris/
**
**    See README and COPYING
***********************************************/

#include "joystick.h"

#define DEFAULT_SIGNAL_TIME 50

Joystick::Joystick(int id, SDL_Joystick *joy, QObject *parent) :
    QObject(parent)
{
    m_id = id;
    m_joy = joy;
    init();

    setSignalTimer(DEFAULT_SIGNAL_TIME);
    connect(&m_timer, SIGNAL(timeout()), SLOT(timeout()));
}

Joystick::~Joystick()
{
    if(m_joy)
        SDL_JoystickClose(m_joy);
}

void Joystick::init()
{
    m_num_axes = SDL_JoystickNumAxes(m_joy);
    m_num_hats = SDL_JoystickNumHats(m_joy);
    m_num_balls = SDL_JoystickNumBalls(m_joy);
    m_num_buttons = SDL_JoystickNumButtons(m_joy);

    m_axes.resize(m_num_axes, 0);
    m_buttons.resize(m_num_buttons, 0);
}

void Joystick::timeout()
{
    QList<int> axes;
    std::list<btn_event> btns;

    {
        QMutexLocker l(&m_lock);

        for(std::set<int>::iterator itr = m_changed_axes.begin(); itr != m_changed_axes.end(); ++itr)
            axes.push_back(*itr);
        btns = m_changed_btns;
        m_changed_axes.clear();
        m_changed_btns.clear();
    }

    if(!axes.empty())
        emit axesChanged(axes);

    for(std::list<btn_event>::iterator itr = btns.begin(); itr != btns.end(); ++itr)
        emit buttonChanged((*itr).id, (*itr).status);
}

void Joystick::setSignalTimer(int periodMS)
{
    m_timer.start(periodMS);
}

void Joystick::axisEvent(int id, qint16 val)
{
    QMutexLocker l(&m_lock);
    m_axes[id] = val;
    m_changed_axes.insert(id);
}

void Joystick::buttonEvent(int id, quint8 state)
{
    QMutexLocker l(&m_lock);
    m_buttons[id] = state;

    btn_event ev = { id, state };
    m_changed_btns.push_back(ev);
}

void Joystick::ballEvent(int /*id*/, qint16 /*x*/, qint16 /*y*/)
{
    // NYI
}

void Joystick::hatEvent(int /*id*/, quint8 /*val*/)
{
    // NYI
}


