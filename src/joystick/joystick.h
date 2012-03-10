#ifndef JOYSTICK_H
#define JOYSTICK_H

#include <QObject>

class Joystick : public QObject
{
    Q_OBJECT
public:
    explicit Joystick(QObject *parent = 0);
    
signals:
    
public slots:
    
};

#endif // JOYSTICK_H
