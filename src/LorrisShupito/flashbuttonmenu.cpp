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

#include "flashbuttonmenu.h"

FlashButtonMenu::FlashButtonMenu(bool read, QToolButton *btn, QWidget *parent) :
    QMenu(parent)
{
    m_button = btn;
    m_read = read;
    m_active = ACT_ALL;

    createActions();

    m_button->setMenu(this);
}

void FlashButtonMenu::createActions()
{
    QAction *Flash  = NULL;
    QAction *All    = NULL;
    QAction *EEPROM = NULL;
    QAction *Fuses  = NULL;

    if(m_read)
    {
        Flash  = addAction(tr("Read flash"));
        All    = addAction(tr("Read all"));
        EEPROM = addAction(tr("Read EEPROM"));
        Fuses  = addAction(tr("Read fuses"));

        connect(All,      SIGNAL(triggered()), parent(), SLOT(readAll()));
        connect(Flash,    SIGNAL(triggered()), parent(), SLOT(readMemButton()));
        connect(EEPROM,   SIGNAL(triggered()), parent(), SLOT(readEEPROMBtn()));
        connect(Fuses,    SIGNAL(triggered()), parent(), SLOT(readFusesInFlash()));
    }
    else
    {
        Flash  = addAction(tr("Write flash"));
        All    = addAction(tr("Write all"));
        EEPROM = addAction(tr("Write EEPROM"));
        Fuses  = addAction(tr("Write fuses"));

        connect(All,      SIGNAL(triggered()), parent(), SLOT(writeAll()));
        connect(Flash,    SIGNAL(triggered()), parent(), SLOT(writeFlashBtn()));
        connect(EEPROM,   SIGNAL(triggered()), parent(), SLOT(writeEEPROMBtn()));
        connect(Fuses,    SIGNAL(triggered()), parent(), SLOT(writeFusesInFlash()));
    }

    insertSeparator(All);

    m_font     = All->font();
    m_boldFont = m_font;
    m_boldFont.setBold(true);

    m_actions[ACT_ALL]    = All;
    m_actions[ACT_FLASH]  = Flash;
    m_actions[ACT_EEPROM] = EEPROM;
    m_actions[ACT_FUSES]  = Fuses;
}

void FlashButtonMenu::setActiveAction(int actInt)
{
    ActionSlots act = ActionSlots(actInt);

    if(act == ACT_ALL) // set flash as active for terminal
        act = ACT_FLASH;

    if((act == m_active) || (act != ACT_FLASH && act != ACT_EEPROM))
        return;

    if(m_active != ACT_ALL)
    {
        insertAction(m_actions[m_active], m_actions[act]);
        insertAction(m_actions[ACT_FUSES], m_actions[m_active]);
    }

    m_actions[m_active]->setFont(m_font);
    m_actions[act]->setFont(m_boldFont);

    m_button->disconnect();
    if(act == ACT_FLASH)
    {
        if(m_read) connect(m_button, SIGNAL(clicked()), parent(), SLOT(readMemButton()));
        else       connect(m_button, SIGNAL(clicked()), parent(), SLOT(writeFlashBtn()));
    }
    else
    {
        if(m_read) connect(m_button, SIGNAL(clicked()), parent(), SLOT(readEEPROMBtn()));
        else       connect(m_button, SIGNAL(clicked()), parent(), SLOT(writeEEPROMBtn()));
    }

    m_active = act;
}
