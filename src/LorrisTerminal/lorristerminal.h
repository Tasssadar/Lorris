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

#ifndef LORRISTERMINAL_H
#define LORRISTERMINAL_H

#include <QObject>
#include <QLineEdit>
#include <QTimer>
#include <QByteArray>

#include "WorkTab/WorkTab.h"


class QVBoxLayout;
class QTextEdit;
class HexFile;
class Terminal;
class EEPROM;
class chip_definition;
struct page;

enum states_
{
    STATE_STOPPING1    = 0x01,
    STATE_STOPPING2    = 0x02,
    STATE_STOPPED      = 0x04,
    STATE_AWAITING_ID  = 0x08,
    STATE_FLASHING     = 0x10,
    STATE_PAUSED       = 0x20,
    STATE_DISCONNECTED = 0x40,
    STATE_EEPROM_READ  = 0x80,
    STATE_EEPROM_WRITE = 0x100,
};

enum buttons_
{
    BUTTON_DISCONNECT  = 0x01,
    BUTTON_STOP        = 0x02,
    BUTTON_FLASH       = 0x04,
    BUTTON_EEPROM_READ = 0x08,
    BUTTON_EEPROM_WRITE= 0x10,
};

class LorrisTerminal : public WorkTab
{
    Q_OBJECT
public:
    explicit LorrisTerminal();
    virtual ~LorrisTerminal();

private slots:
    //Buttons
    void browseForHex();
    void clearButton();
    void stopButton();
    void flashButton();
    void pauseButton();
    void connectButton();
    void eepromButton();
    void eepromImportButton();

    void readData(const QByteArray& data);
    void sendKeyEvent(QByteArray key);
    void connectionResult(Connection *con, bool result);
    void connectedStatus(bool connected);

    //Timers
    void stopTimerSig();
    void flashTimeout();
    void deviceIdTimeout();

private:
    void flash_prepare(QString deviceId);
    void eeprom_read(QString id);
    void eeprom_write(QString id);
    bool eeprom_send_page();
    void eeprom_read_block(QByteArray data);
    bool SendNextPage();
    void EnableButtons(quint16 buttons, bool enable);
    void initUI();

    QVBoxLayout *layout;
    QLineEdit *hexLine;
    QTimer *stopTimer;
    QTimer *flashTimeoutTimer;
    QByteArray stopCmd;
    HexFile *hex;
    Terminal *terminal;

    quint16 m_state;
    quint16 m_eepromItr;
    EEPROM *m_eeprom;

    std::vector<chip_definition> m_chip_defs;
    std::vector<page> m_pages;
    quint32 m_cur_page;
};

#endif // LORRISTERMINAL_H
