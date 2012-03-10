#include <QDebug>
#include "joymgr.h"

JoyMgr::JoyMgr()
{
    // SDL needs video to be enabled
    SDL_Init(SDL_INIT_VIDEO | SDL_INIT_JOYSTICK);
}

void JoyMgr::updateJoystickNames()
{
    int cnt = SDL_NumJoysticks();

    for(int i = 0; i < cnt; ++i)
        m_names.insert(i, SDL_JoystickName(i));

}
