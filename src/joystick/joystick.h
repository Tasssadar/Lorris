/**********************************************
**    This file is part of Lorris
**    http://tasssadar.github.com/Lorris/
**
**    See README and COPYING
***********************************************/

#ifndef JOYSTICK_H
#define JOYSTICK_H

#include <QObject>
#include <QMutex>
#include <set>
#include <vector>
#include <QTimer>

#ifdef Q_OS_WIN
    #include <SDL.h>
#else // use lib from OS on other systems
    #include <SDL/SDL.h>
#endif

struct btn_event
{
    int id;
    quint8 status;
};

class Joystick : public QObject
{
    Q_OBJECT

Q_SIGNALS:
    void removeJoystick(Joystick *joy);
    void buttonChanged(int id, quint8 status);
    void axesChanged(const QList<int>& axes);

public:
    Joystick(int id, SDL_Joystick *joy, QObject *parent = 0);
    ~Joystick();

    void axisEvent(int id, qint16 val);
    void ballEvent(int id, qint16 x, qint16 y);
    void hatEvent(int id, quint8 val);
    void buttonEvent(int id, quint8 state);

    bool isUsed() const { return !m_used.empty(); }

public slots:
    int getId() const { return m_id; }
    int getNumAxes() const { return m_num_axes; }
    int getNumBalls() const { return m_num_balls; }
    int getNumHats() const { return m_num_hats; }
    int getNumButtons() const { return m_num_buttons; }

    int getAxisVal(int id)
    {
        QMutexLocker locker(&m_lock);
        return m_axes[id];
    }

    quint8 getButtonVal(int id)
    {
        QMutexLocker locker(&m_lock);
        return m_buttons[id];
    }

    void startUsing(QObject *object)
    {
        m_used.insert(object);
    }

    void stopUsing(QObject *object)
    {
        m_used.erase(object);

        if(m_used.empty())
            emit removeJoystick(this);
    }

    void setSignalTimer(int periodMS);

private slots:
    void timeout();

private:
    void init();

    int m_id;
    SDL_Joystick *m_joy;
    QTimer m_timer;

    int m_num_axes;
    int m_num_balls;
    int m_num_hats;
    int m_num_buttons;

    std::vector<int> m_axes;
    std::vector<quint8> m_buttons;
    std::set<int> m_changed_axes;
    std::list<btn_event> m_changed_btns;

    std::set<QObject*> m_used;

    QMutex m_lock;
};

#endif // JOYSTICK_H
