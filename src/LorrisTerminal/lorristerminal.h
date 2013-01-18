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
#include "../ui/terminal.h"
#include "../ui/chooseconnectiondlg.h"
#include "../ui/connectbutton.h"

class QVBoxLayout;
class QTextEdit;

namespace Ui {
    class LorrisTerminal;
}

class LorrisTerminal : public PortConnWorkTab
{
    Q_OBJECT
public:
    explicit LorrisTerminal();
    virtual ~LorrisTerminal();

    virtual void setPortConnection(ConnectionPointer<PortConnection> const & con);

    QString GetIdString();

    void onTabShow(const QString& filename);

    void saveData(DataFileParser *file);
    void loadData(DataFileParser *file);

private slots:
    void pauseButton();
    void setPauseBtnText(bool pause);
    void fmtAction(int act);
    void checkFmtAct(int act);
    void loadText();
    void saveText();
    void saveBin();
    void inputAct(int act);
    void sendButton();

    void readData(const QByteArray& data);
    void sendKeyEvent(const QString& key);
    void connectedStatus(bool connected);
    void saveTermSettings();

private:
    void initUI();

    QAction *m_export_eeprom;
    QAction *m_import_eeprom;
    QAction *m_fmt_act[FMT_MAX];
    QAction *m_input[INPUT_MAX];

    ConnectButton * m_connectButton;
    Ui::LorrisTerminal *ui;
};

#endif // LORRISTERMINAL_H
