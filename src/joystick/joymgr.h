#ifndef JOYMGR_H
#define JOYMGR_H


#include <QHash>

#ifdef Q_OS_WIN
    #include <SDL.h>
#else // use lib from OS on other systems
    #include <SDL/SDL.h>
#endif

#include "singleton.h"
#include "joystick.h"

class JoyMgr : public Singleton<JoyMgr>
{
public:
    JoyMgr();

    void updateJoystickNames();

    bool isEmpty() { return m_names.isEmpty(); }
    const QHash<int, QString>& getNames() { return m_names; }

private:
    QHash<int, Joystick*> m_joysticks;
    QHash<int, QString> m_names;
};

#define sJoyMgr JoyMgr::GetSingleton()

#endif // JOYMGR_H
