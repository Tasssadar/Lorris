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

#ifndef SHUPITOTUNNEL_H
#define SHUPITOTUNNEL_H

#include "connection.h"
#include "connectionmgr.h"

class QComboBox;
class Shupito;

class ShupitoTunnel : public Connection
{
    Q_OBJECT

public:
    ShupitoTunnel();
    ~ShupitoTunnel();

    bool Open();
    void OpenConcurrent();
    void Close();
    void SendData(const QByteArray &data);

    void setShupito(Shupito* s);
    void removeFromMgr();

private:
    Shupito *m_shupito;
    bool dataSigConnected;
};

class ShupitoTunnelBuilder : public ConnectionBuilder
{
    Q_OBJECT
public:
    ShupitoTunnelBuilder(QWidget *parent, int moduleIdx) : ConnectionBuilder(parent, moduleIdx)
    {
    }

    void addOptToTabDialog(QGridLayout *layout);
    void CreateConnection(WorkTabInfo *info);

private:
    QComboBox *m_portBox;
};


#endif // SHUPITOTUNNEL_H
