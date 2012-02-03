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

#ifndef FLASHBUTTONMENU_H
#define FLASHBUTTONMENU_H

#include <QMenu>
#include <map>
#include <QToolButton>

enum ActionSlots
{
    ACT_FLASH  = 0,
    ACT_EEPROM,
    ACT_ALL,
    ACT_FUSES
};

class FlashButtonMenu : public QMenu
{
    Q_OBJECT
public:
    explicit FlashButtonMenu(bool read, QToolButton *btn, QWidget *parent = 0);

public slots:
    void setActiveAction(int actInt);

private:
    void createActions();

    std::map<ActionSlots, QAction*> m_actions;
    QToolButton *m_button;
    bool m_read;
    ActionSlots m_active;

    QFont m_font;
    QFont m_boldFont;
};

#endif // FLASHBUTTONMENU_H
