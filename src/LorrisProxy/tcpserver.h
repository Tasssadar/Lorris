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

#ifndef TCPSERVER_H
#define TCPSERVER_H

#include <QObject>
#include <map>

class QTcpServer;
class QTcpSocket;
class QSignalMapper;

class TcpServer : public QObject
{
    Q_OBJECT

Q_SIGNALS:
    void newData(const QByteArray& data);
    void newConnection(QTcpSocket *socket, quint32 id);
    void removeConnection(quint32 id);

public:
    typedef std::map<quint32, QTcpSocket*> socketMap;

    TcpServer();
    ~TcpServer();

    bool listen(const QString& address, quint16 port);
    void stopListening();

    QString getLastErr();
    bool isListening();
    QString getAddress();

public slots:
    void SendData(const QByteArray& data);

private slots:
    void newConnection();
    void disconnected(int con);
    void readyRead(int con);

private:
    QTcpServer *m_server;
    QSignalMapper *m_disconnect_map;
    QSignalMapper *m_ready_map;
    socketMap m_socket_map;

    quint32 m_con_counter;
};

#endif // TCPSERVER_H
