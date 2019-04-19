/**********************************************
**    This file is part of Lorris
**    http://tasssadar.github.com/Lorris/
**
**    See README and COPYING
***********************************************/

#ifndef TCPSERVER_H
#define TCPSERVER_H

#include <QObject>
#include <QHash>
#include <QTcpServer>
#include <QSignalMapper>

#include "server.h"

class QTcpSocket;

class TcpServer : public Server
{
    Q_OBJECT

public:
    typedef QHash<quint32, QTcpSocket*> socketMap;

    TcpServer(QObject *parent = NULL);
    virtual ~TcpServer();

    bool listen(const QString& address, quint16 port);
    void stopListening();
    void closeConnection(quint32 id);
    bool isListening() const { return m_server.isListening(); }
    QString errorString() const { return m_server.errorString(); }

public slots:
    void SendData(const QByteArray& data);

protected:
    QHostAddress serverAddress() const { return m_server.serverAddress(); }

private slots:
    void onNewConnection();
    void disconnected(int con);
    void readyRead(int con);

private:
    QTcpServer m_server;
    QSignalMapper m_disconnect_map;
    QSignalMapper m_ready_map;
    socketMap m_socket_map;
};

#endif // TCPSERVER_H
