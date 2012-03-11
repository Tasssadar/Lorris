/****************************************************************************
**
**    This file is part of Lorris.
**    Copyright (C) 2012 Vojtěch Boček
**
**    Contact: <vbocek@gmail.com>
**             https://github.com/Tasssadar
**
**    Lorris is free software: you can redistribute it and/or modify
**    it under the terms of the GNU General Public License as published by
**    the Free Software Foundation, either version 3 of the License, or
**    (at your option) any later version.
**
**    Lorris is distributed in the hope that it will be useful,
**    but WITHOUT ANY WARRANTY; without even the implied warranty of
**    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
**    GNU General Public License for more details.
**
**    You should have received a copy of the GNU General Public License
**    along with Lorris.  If not, see <http://www.gnu.org/licenses/>.
**
****************************************************************************/

#ifndef JOYTHREAD_H
#define JOYTHREAD_H

#include <QThread>

#ifdef Q_OS_WIN
    #include <SDL.h>
#else // use lib from OS on other systems
    #include <SDL/SDL.h>
#endif

class JoyThread : public QThread
{
    Q_OBJECT
public:
    explicit JoyThread(QObject *parent = 0);
    
    void stop();

protected:
    void run();

private:
    volatile bool m_run;
};

#endif // JOYTHREAD_H
