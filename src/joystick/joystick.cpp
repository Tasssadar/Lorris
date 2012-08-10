/**********************************************
**    This file is part of Lorris
**    http://tasssadar.github.com/Lorris/
**
**    See README and COPYING
***********************************************/

#include "joystick.h"

#define DEFAULT_SIGNAL_TIME 50

JoystickPrivate::JoystickPrivate(int id, SDL_Joystick *joy, QObject *parent) :
    QObject(parent)
{
    m_id = id;
    m_joy = joy;
    init();

    setSignalTimer(DEFAULT_SIGNAL_TIME);
    connect(&m_timer, SIGNAL(timeout()), SLOT(timeout()));
}

JoystickPrivate::~JoystickPrivate()
{
    if(m_joy)
        SDL_JoystickClose(m_joy);
}

void JoystickPrivate::init()
{
    m_num_axes = SDL_JoystickNumAxes(m_joy);
    m_num_hats = SDL_JoystickNumHats(m_joy);
    m_num_balls = SDL_JoystickNumBalls(m_joy);
    m_num_buttons = SDL_JoystickNumButtons(m_joy);

    m_axes.resize(m_num_axes, 0);
    m_buttons.resize(m_num_buttons, 0);
    m_changed_axes.resize(m_num_axes, false);
}

void JoystickPrivate::timeout()
{
    QList<int> axes;
    std::vector<btn_event> btns;

    {
        QWriteLocker l(&m_lock);

        for(size_t i = 0; i < m_changed_axes.size(); ++i)
        {
            if(m_changed_axes[i])
            {
                m_changed_axes[i] = false;
                axes.push_back(i);
            }
        }

        m_changed_btns.swap(btns);
    }

    if(!axes.empty())
        emit axesChanged(axes);

    for(std::vector<btn_event>::iterator itr = btns.begin(); itr != btns.end(); ++itr)
        emit buttonChanged((*itr).id, (*itr).status);
}

void JoystickPrivate::setSignalTimer(int periodMS)
{
    m_timer.start(periodMS);
}

void JoystickPrivate::axisEvent(int id, qint16 val)
{
    QWriteLocker l(&m_lock);
    m_axes[id] = val;
    m_changed_axes[id] = true;
}

void JoystickPrivate::buttonEvent(int id, quint8 state)
{
    QWriteLocker l(&m_lock);
    m_buttons[id] = state;

    btn_event ev = { id, state };
    m_changed_btns.push_back(ev);
}

void JoystickPrivate::ballEvent(int /*id*/, qint16 /*x*/, qint16 /*y*/)
{
    // NYI
}

void JoystickPrivate::hatEvent(int /*id*/, quint8 /*val*/)
{
    // NYI
}


