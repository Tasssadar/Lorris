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

#ifndef LORRISPROXY_H
#define LORRISPROXY_H

#include "WorkTab/WorkTab.h"

namespace Ui {
    class LorrisProxy;
}

class TcpServer;
class QTcpSocket;

class LorrisProxy : public WorkTab
{
    Q_OBJECT
    
public:
    explicit LorrisProxy();
    ~LorrisProxy();

    void setConnection(Connection *con);

private slots:
    void updateAddressText();
    void listenChanged();
    void addConnection(QTcpSocket *connection, quint32 id);
    void removeConnection(quint32 id);
    void connectButton();
    void connectionResult(Connection *con,bool result);
    void connectedStatus(bool connected);

private:
    Ui::LorrisProxy *ui;
    TcpServer *m_server;
};

#endif // LORRISPROXY_H
