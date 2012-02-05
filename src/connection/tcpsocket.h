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

#ifndef TCPSOCKET_H
#define TCPSOCKET_H

#include <QFuture>
#include <QFutureWatcher>

#include "connection.h"

class QTcpSocket;

class TcpSocket : public Connection
{
    Q_OBJECT
public:
    explicit TcpSocket();
    ~TcpSocket();

    bool Open();
    void OpenConcurrent();
    void Close();
    void SendData(const QByteArray &data);
    void setAddress(const QString& address, quint16 port)
    {
        m_address = address;
        m_port = port;
        m_idString = address + ":" + QString::number(port);
    }
    
public slots:
    void connectResultSer(bool opened);
    void tcpConnectResult();
    void readyRead();
    void stateChanged();

private:
    bool connectToHost();

    QTcpSocket *m_socket;
    quint16 m_port;
    QString m_address;
    
    QFuture<bool> m_future;
    QFutureWatcher<bool> m_watcher;
};

#endif // TCPSOCKET_H
