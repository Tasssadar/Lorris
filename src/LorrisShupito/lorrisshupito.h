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

#ifndef LORRISSHUPITO_H
#define LORRISSHUPITO_H

#include "WorkTab/WorkTab.h"
#include "shupito.h"
#include "shupitodesc.h"

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

    void stopAll();

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

    void tunnelSpeedChanged ( const QString & text );
    void tunnelToggled(bool enable);
    void tunnelStateChanged(bool opened);

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

    ShupitoDesc::config *m_vdd_config;
    ShupitoDesc::config *m_tunnel_config;
};

#endif // LORRISSHUPITO_H
