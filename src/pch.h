/**********************************************
**    This file is part of Lorris
**    http://tasssadar.github.com/Lorris/
**
**    See README and COPYING
***********************************************/

#include <vector>
#include <set>
#include <QLabel>
#include <QPushButton>
#include <QString>
#include <QVBoxLayout>
#include <QHBoxLayout>

#if defined Q_OS_WIN || defined Q_OS_MAC
    #include <SDL.h>
#else // use lib from OS on other systems
    #include <SDL/SDL.h>
#endif
#include "common.h"
#include "shared/hexfile.h"
