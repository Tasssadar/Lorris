/**********************************************
**    This file is part of Lorris
**    http://tasssadar.github.com/Lorris/
**
**    See README and COPYING
***********************************************/

#ifndef JOYSTICK_H
#define JOYSTICK_H

#include <QObject>
#include <QReadWriteLock>
#include <set>
#include <vector>
#include <QTimer>

#ifdef HAVE_LIBENJOY
#include <libenjoy.h>

struct btn_event
{
    int id;
    quint8 status;
};

class JoystickPrivate : public QObject
{
    Q_OBJECT

    friend class Joystick;
    friend class JoyMgr;
    friend class JoyThread;

Q_SIGNALS:
    void removeJoystick(JoystickPrivate *joy);
    void buttonChanged(int id, quint8 status);
    void axesChanged(const QList<int>& axes);

protected:
    JoystickPrivate(libenjoy_joystick *joy, QObject *parent = 0);
    ~JoystickPrivate();

    void axisEvent(int id, qint16 val);
    void buttonEvent(int id, quint8 state);

    bool isUsed() const { return !m_used.empty(); }

protected slots:
    quint32 getId() const { return m_id; }
    int getNumAxes() const { return m_num_axes; }
    int getNumButtons() const { return m_num_buttons; }

    qint16 getAxisVal(int id)
    {
        QReadLocker locker(&m_lock);
        return m_axes[id];
    }

    quint8 getButtonVal(int id)
    {
        QReadLocker locker(&m_lock);
        return m_buttons[id];
    }

    void startUsing(QObject *object)
    {
        m_used.insert(object);
    }

    bool stopUsing(QObject *object)
    {
        m_used.erase(object);

        if(m_used.empty())
        {
            emit removeJoystick(this);
            return true;
        }
        return false;
    }

    void setSignalTimer(int periodMS);

private slots:
    void timeout();

private:
    void init();

    quint32 m_id;
    libenjoy_joystick *m_joy;
    QTimer m_timer;

    int m_num_axes;
    int m_num_buttons;

    std::vector<qint16> m_axes;
    std::vector<quint8> m_buttons;
    std::vector<bool> m_changed_axes;
    std::vector<btn_event> m_changed_btns;

    std::set<QObject*> m_used;

    QReadWriteLock m_lock;
};

class Joystick : public QObject
{
    Q_OBJECT

public:
    Joystick(JoystickPrivate *joy) : QObject(joy)
    {
        this->joy = joy;
        connect(joy, SIGNAL(buttonChanged(int,quint8)), SIGNAL(buttonChanged(int,quint8)));
        connect(joy, SIGNAL(axesChanged(QList<int>)), SIGNAL(axesChanged(QList<int>)));
    }

Q_SIGNALS:
    void buttonChanged(int id, quint8 status);
    void axesChanged(const QList<int>& axes);

public slots:
    quint32 getId() const { return joy->getId(); }
    int getNumAxes() const { return joy->getNumAxes(); }
    int getNumButtons() const { return joy->getNumButtons(); }

    qint16 getAxisVal(int id) { return joy->getAxisVal(id); }
    quint8 getButtonVal(int id) { return joy->getButtonVal(id); }

    void startUsing(QObject *object) { joy->startUsing(object); }
    bool stopUsing(QObject *object) { return joy->stopUsing(object); }
    void setSignalTimer(int periodMS) { joy->setSignalTimer(periodMS); }

private:
    JoystickPrivate *joy;
};

#else // HAVE_LIBENJOY

class Joystick : public QObject
{
    Q_OBJECT

public:
    Joystick(QObject *joy) : QObject(joy)
    {

    }

Q_SIGNALS:
    void buttonChanged(int id, quint8 status);
    void axesChanged(const QList<int>& axes);

public slots:
    quint32 getId() const { return 0; }
    int getNumAxes() const { return 0; }
    int getNumButtons() const { return 0; }

    qint16 getAxisVal(int) { return 0; }
    quint8 getButtonVal(int) { return 0; }

    void startUsing(QObject *) {  }
    bool stopUsing(QObject *) { return false; }
    void setSignalTimer() {  }
};

#endif // HAVE_LIBENJOY

#endif // JOYSTICK_H
