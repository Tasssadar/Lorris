/* EcWin7 - Support library for integrating Windows 7 taskbar features
 * into any Qt application
 * Copyright (C) 2010 Emanuele Colombo
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 59 Temple Place - Suite 330, Boston, MA 02111-1307, USA.
 */

#include "ecwin7.h"

// Windows only GUID definitions
#if defined(Q_OS_WIN)
DEFINE_GUID(CLSID_TaskbarList,0x56fdf344,0xfd6d,0x11d0,0x95,0x8a,0x0,0x60,0x97,0xc9,0xa0,0x90);
DEFINE_GUID(IID_ITaskbarList3,0xea1afb91,0x9e28,0x4b86,0x90,0xE9,0x9e,0x9f,0x8a,0x5e,0xef,0xaf);
#endif

// Constructor: variabiles initialization
EcWin7::EcWin7()
{
#ifdef Q_OS_WIN
    mTaskbar = NULL;
    mOverlayIcon = NULL;

    CoCreateInstance(CLSID_TaskbarList, 0,CLSCTX_INPROC_SERVER,
                     IID_ITaskbarList3, reinterpret_cast<void**> (&(mTaskbar)));
#endif
}

// Init taskbar communication
void EcWin7::init(WId wid)
{
    mWindowId = wid;
}

// Set progress bar current value
#ifdef Q_OS_WIN
void EcWin7::setProgressValue(int value, int max)
{
    if(mTaskbar)
        mTaskbar->SetProgressValue((HWND)mWindowId, value, max);
}
#else
void EcWin7::setProgressValue(int, int) { }
#endif

// Set progress bar current state (active, error, pause, ecc...)
#ifdef Q_OS_WIN
void EcWin7::setProgressState(ToolBarProgressState state)
{
    if(mTaskbar)
        mTaskbar->SetProgressState((HWND)mWindowId, (TBPFLAG)state);
}
#else
void EcWin7::setProgressState(ToolBarProgressState) { }
#endif

// Set new overlay icon and corresponding description (for accessibility)
// (call with iconName == "" and description == "" to remove any previous overlay icon)
#ifdef Q_OS_WIN
void EcWin7::setOverlayIcon(QString iconName, QString description)
{
    if(!mTaskbar)
        return;

    HICON oldIcon = NULL;
    if (mOverlayIcon != NULL) oldIcon = mOverlayIcon;
    if (iconName == "")
    {
        mTaskbar->SetOverlayIcon((HWND)mWindowId, NULL, NULL);
        mOverlayIcon = NULL;
    }
    else
    {
        mOverlayIcon = (HICON) LoadImage(GetModuleHandle(NULL),
                                 iconName.toStdWString().c_str(),
                                 IMAGE_ICON,
                                 0,
                                 0,
                                 NULL);
        mTaskbar->SetOverlayIcon((HWND)mWindowId, mOverlayIcon, description.toStdWString().c_str());
    }
    if ((oldIcon != NULL) && (oldIcon != mOverlayIcon))
    {
        DestroyIcon(oldIcon);
    }
}
#else
void EcWin7::setOverlayIcon(QString, QString) { }
#endif
