/**********************************************
**    This file is part of Lorris
**    http://tasssadar.github.com/Lorris/
**
**    See README and COPYING
***********************************************/

#ifndef AVR232BOOT_H
#define AVR232BOOT_H

#include <QObject>

#include "../shared/hexfile.h"
#include "eeprom.h"

class PortConnection;
class ProgressBar;

enum states_
{
    STATE_STOPPED      = 0x01,
    STATE_WAITING_ID   = 0x02,
    STATE_EEPROM_READ  = 0x04,
    STATE_WAIT_ACK     = 0x08
};

namespace Ui {
    class LorrisTerminal;
}

class avr232boot : public QObject
{
    Q_OBJECT

Q_SIGNALS:
    void received();

public:
    explicit avr232boot(QObject *parent = 0);

    bool dataRead(const QByteArray& data);
    bool startStop();
    void setCon(PortConnection *con)
    {
        m_con = con;
    }

    bool getChipId();
    void flash(Ui::LorrisTerminal *ui);
    void readEEPROM(Ui::LorrisTerminal *ui);
    void writeEEPROM(Ui::LorrisTerminal *ui);

    HexFile *getHex() { return &m_hex; }

private:
    bool waitForAck();
    bool waitForRec();

    quint8 m_state;
    PortConnection *m_con;
    HexFile m_hex;
    QByteArray m_dev_id;
    EEPROM m_eeprom;
};

#endif // AVR232BOOT_H
