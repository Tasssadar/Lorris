#include <QDebug>
#include "joymgr.h"

JoyMgr::JoyMgr()
{
    // SDL needs video to be enabled
    SDL_Init(SDL_INIT_VIDEO | SDL_INIT_JOYSTICK);

    updateJoystickNames();
}

JoyMgr::~JoyMgr()
{
    for(QHash<int, Joystick*>::iterator itr = m_joysticks.begin(); itr != m_joysticks.end(); ++itr)
        delete *itr;

    SDL_QuitSubSystem(SDL_INIT_VIDEO | SDL_INIT_JOYSTICK);
}

void JoyMgr::updateJoystickNames()
{
    int cnt = SDL_NumJoysticks();

    qDebug() << "Joystick count: " << cnt;
    for(int i = 0; i < cnt; ++i)
    {
        m_names.insert(i, SDL_JoystickName(i));
        qDebug() << i << ": " << m_names[i];
    }

}
