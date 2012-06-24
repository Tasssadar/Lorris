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
#include "avr232boot.h"

class QVBoxLayout;
class QTextEdit;
class HexFile;
class EEPROM;
class chip_definition;
struct page;

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

    void onTabShow(const QString& filename);
    virtual void setConnection(PortConnection *con);

private slots:
    //Buttons
    void browseForHex();
    void stopButton();
    void flashButton();
    void pauseButton();
    void setPauseBtnText(bool pause);
    void eepromExportButton();
    void eepromImportButton();
    void fmtAction(int act);
    void checkFmtAct(int act);
    void loadText();
    void saveText();
    void saveBin();
    void inputAct(int act);

    void readData(const QByteArray& data);
    void sendKeyEvent(const QString& key);
    void connectionResult(Connection *con, bool result);
    void connectedStatus(bool connected);
    void saveTermSettings();
    void showBootloader(bool show);
    void showWarn(bool show);

private:
    void setHexName(QString name = QString());
    void EnableButtons(quint16 buttons, bool enable);
    void initUI();

    QString m_filename;
    QDateTime m_filedate;
    QDateTime m_flashdate;

    QAction *m_export_eeprom;
    QAction *m_import_eeprom;
    QAction *m_fmt_act[FMT_MAX];
    QAction *m_input[INPUT_MAX];

    bool m_stopped;

    ConnectButton * m_connectButton;
    Ui::LorrisTerminal *ui;

    avr232boot m_bootloader;
};

#endif // LORRISTERMINAL_H
