#ifndef LORRISSHUPITO_H
#define LORRISSHUPITO_H

#include "WorkTab/WorkTab.h"
#include "shupito.h"

enum state
{
    STATE_DISCONNECTED = 0x01
};

enum responses
{
    RESPONSE_NONE,
    RESPONSE_WAITING,
    RESPONSE_GOOD,
    RESPONSE_BAD
};

namespace Ui {
    class LorrisShupito;
}

class Shupito;
class ShupitoDesc;
class QLabel;
class QComboBox;

class LorrisShupito : public WorkTab
{
    Q_OBJECT
Q_SIGNALS:
    void responseChanged();

public:
    LorrisShupito();
    ~LorrisShupito();

private slots:
    void connectButton();
    void onTabShow();

    void connectionResult(Connection*,bool);
    void connectedStatus(bool connected);
    void readData(const QByteArray& data);
    void descRead();

    void responseReceived(char error_code);
    void vccValueChanged(quint8 id, double value);
    void vddSetup(const vdd_setup& vs);
    void vddIndexChanged(int index);

private:
    void sendAndWait(const QByteArray &data);

    Ui::LorrisShupito *ui;
    quint8 m_state;
    Shupito *m_shupito;
    ShupitoDesc *m_desc;
    QTimer *responseTimer;
    volatile quint8 m_response;

    QLabel *vccLabel;
    QComboBox *vddBox;
    QString vccText;
    vdd_setup m_vdd_setup;
    double m_vcc;
    int lastVccIndex;
};

#endif // LORRISSHUPITO_H
