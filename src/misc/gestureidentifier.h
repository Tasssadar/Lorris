/**********************************************
**    This file is part of Lorris
**    http://tasssadar.github.com/Lorris/
**
**    See README and COPYING
***********************************************/

#ifndef GESTUREIDENTIFIER_H
#define GESTUREIDENTIFIER_H

#include <QObject>
#include <QPoint>
#include <vector>

enum gestures
{
    GESTURE_SHAKE_LEFT = 0,
    GESTURE_SHAKE_RIGHT,

    GESTURE_MAX
};

class GestureIdentifier : public QObject
{
    Q_OBJECT

    enum states
    {
        STATE_NONE = 0,
        STATE_PROGRESS,
        STATE_COMPLETE,
        STATE_WRONG
    };

    enum directions
    {
        DIR_UP = 0,
        DIR_DOWN,
        DIR_LEFT,
        DIR_RIGHT,

        DIR_INVALID
    };

Q_SIGNALS:
    void gestureCompleted(int gesture);

public:
    GestureIdentifier(QObject *parent = NULL);
    virtual ~GestureIdentifier();

public slots:
    void clear();
    void moveEvent(const QPoint& pos);

private:
    bool addDir(directions dir);
    void checkGestures();
    directions getDirection(const QPoint& diff);

    states m_state;
    std::vector<directions> m_gestures[GESTURE_MAX];
    int m_correct[GESTURE_MAX];
    directions m_current;
    QPoint m_last_point;
};

#endif // GESTUREIDENTIFIER_H
