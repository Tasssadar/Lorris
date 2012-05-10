/**********************************************
**    This file is part of Lorris
**    http://tasssadar.github.com/Lorris/
**
**    See README and COPYING
***********************************************/

#ifndef LORRISTERMINAL_H
#define LORRISTERMINAL_H

#include <QObject>
#include <QLineEdit>
#include <QTimer>
#include <QByteArray>
#include <QToolButton>

#include "../WorkTab/WorkTab.h"
#include "../shared/terminal.h"
#include "../ui/chooseconnectiondlg.h"
#include "../ui/connectbutton.h"

class QVBoxLayout;
class QTextEdit;
class HexFile;
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
    STATE_EEPROM_WRITE = 0x100
};

enum buttons_
{
    BUTTON_STOP        = 0x02,
    BUTTON_FLASH       = 0x04,
    BUTTON_EEPROM_READ = 0x08,
    BUTTON_EEPROM_WRITE= 0x10
};

namespace Ui {
    class LorrisTerminal;
}

class LorrisTerminal : public PortConnWorkTab
{
    Q_OBJECT
public:
    explicit LorrisTerminal();
    virtual ~LorrisTerminal();

    void onTabShow();
    virtual void setConnection(PortConnection *con);

private slots:
    //Buttons
    void browseForHex();
    void clearButton();
    void stopButton();
    void flashButton();
    void pauseButton();
    void eepromButton();
    void eepromImportButton();
    void fmtAction(int act);
    void loadText();
    void saveText();
    void inputAct(int act);

    void readData(const QByteArray& data);
    void sendKeyEvent(const QString& key);
    void connectionResult(Connection *con, bool result);
    void connectedStatus(bool connected);
    void saveTermFont(const QString& fontData);

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

    QTimer *stopTimer;
    QTimer *flashTimeoutTimer;
    QByteArray stopCmd;
    HexFile *hex;

    QAction *m_export_eeprom;
    QAction *m_import_eeprom;
    QAction *m_fmt_act[FMT_MAX];
    QAction *m_input[INPUT_MAX];

    quint16 m_state;
    quint16 m_eepromItr;
    EEPROM *m_eeprom;

    std::vector<chip_definition> m_chip_defs;
    std::vector<page> m_pages;
    quint32 m_cur_page;

    ConnectButton * m_connectButton;
    Ui::LorrisTerminal *ui;
};

#endif // LORRISTERMINAL_H
