/**********************************************
**    This file is part of Lorris
**    http://tasssadar.github.com/Lorris/
**
**    See README and COPYING
***********************************************/

#include <algorithm>
#include <QDateTime>
#include "gestureidentifier.h"

#define GESTURE_THRESHOLD 100
#define TIME_LIMIT 1500

GestureIdentifier::GestureIdentifier(QObject *parent) : QObject(parent)
{
    m_gestures[GESTURE_SHAKE_LEFT].push_back(DIR_LEFT);
    m_gestures[GESTURE_SHAKE_LEFT].push_back(DIR_RIGHT);
    m_gestures[GESTURE_SHAKE_LEFT].push_back(DIR_LEFT);

    m_gestures[GESTURE_SHAKE_RIGHT].push_back(DIR_RIGHT);
    m_gestures[GESTURE_SHAKE_RIGHT].push_back(DIR_LEFT);
    m_gestures[GESTURE_SHAKE_RIGHT].push_back(DIR_RIGHT);

    clear();
}

GestureIdentifier::~GestureIdentifier()
{
}

void GestureIdentifier::clear()
{
    m_state = STATE_NONE;
    m_last_point = QPoint();
    m_current = DIR_INVALID;

    std::fill(m_correct, m_correct+GESTURE_MAX, 0);
}

void GestureIdentifier::moveEvent(const QPoint &pos)
{
    switch(m_state)
    {
        case STATE_NONE:
            m_last_point = pos;
            m_state = STATE_PROGRESS;
            m_start_time = QDateTime::currentMSecsSinceEpoch();
            break;
        case STATE_PROGRESS:
        {
            if(QDateTime::currentMSecsSinceEpoch() - m_start_time >= TIME_LIMIT)
            {
                m_state = STATE_WRONG;
                break;
            }

            QPoint diff = pos - m_last_point;
            if(diff.manhattanLength() < GESTURE_THRESHOLD)
                break;

            directions dir = getDirection(diff);
            if(addDir(dir))
                checkGestures();

            m_last_point = pos;
            break;
        }
        case STATE_WRONG:
        case STATE_COMPLETE:
            break;
    }
}

bool GestureIdentifier::addDir(directions dir)
{
    if(m_current == dir)
        return false;

    m_current = dir;
    return true;
}

void GestureIdentifier::checkGestures()
{
    bool leastOne = false;
    for(int i = 0; i < GESTURE_MAX; ++i)
    {
        if(m_correct[i] == -1)
            continue;

        if(m_gestures[i].at(m_correct[i]) == m_current)
        {
            ++m_correct[i];

            if(m_correct[i] == (int)m_gestures[i].size())
            {
                m_state = STATE_COMPLETE;
                emit gestureCompleted(i);
                return;
            }

            leastOne = true;
        }
        else
            m_correct[i] = -1;
    }

    if(!leastOne)
        m_state = STATE_WRONG;
}

GestureIdentifier::directions GestureIdentifier::getDirection(const QPoint& diff)
{
    if(abs(diff.x()) > abs(diff.y()))
    {
        if(diff.x() > 0)
            return DIR_RIGHT;
        else
            return DIR_LEFT;
    }
    else
    {
        if(diff.y() > 0)
            return DIR_DOWN;
        else
            return DIR_UP;
    }
}
