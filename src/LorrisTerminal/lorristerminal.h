#ifndef LORRISTERMINAL_H
#define LORRISTERMINAL_H

#include <QObject>
#include <QLineEdit>
#include <QTimer>
#include <QByteArray>

#include "WorkTab.h"

class QVBoxLayout;
class QTextEdit;
class HexFile;
class Terminal;

enum states_
{
    STATE_STOPPING1    = 0x01,
    STATE_STOPPING2    = 0x02,
    STATE_STOPPED      = 0x04,
    STATE_AWAITING_ID  = 0x08,
    STATE_FLASHING     = 0x10,
    STATE_PAUSED       = 0x20,
    STATE_DISCONNECTED = 0x40,
};

class LorrisTerminal : public WorkTab
{
    Q_OBJECT
public:
    explicit LorrisTerminal();
    virtual ~LorrisTerminal();

    QWidget *GetTab(QWidget *parent = NULL);

private slots:
    //Buttons
    void browseForHex();
    void clearButton();
    void stopButton();
    void flashButton();
    void pauseButton();
    void connectButton();

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
    bool SendNextPage();
    void initUI();

    QWidget *mainWidget;
    QVBoxLayout *layout;
    QLineEdit *hexLine;
    QTimer *stopTimer;
    QTimer *flashTimeoutTimer;
    QByteArray stopCmd;
    HexFile *hex;
    Terminal *terminal;

    quint8 m_state;
};

#endif // LORRISTERMINAL_H
