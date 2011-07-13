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

    void readData(QByteArray data);
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
};

#endif // LORRISTERMINAL_H
