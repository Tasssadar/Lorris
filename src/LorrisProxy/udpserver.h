/**********************************************
**    This file is part of Lorris
**    http://tasssadar.github.com/Lorris/
**
**    See README and COPYING
***********************************************/

#ifndef UDPSERVER_H
#define UDPSERVER_H

#include <QObject>
#include <QUdpSocket>
#include <QSet>

#include "server.h"

class UdpServer : public Server
{
    Q_OBJECT

public:
    UdpServer(QObject *parent = NULL);
    virtual ~UdpServer();

    bool listen(const QString& address, quint16 port);
    void stopListening();
    bool isListening() const { return m_isBound; }
    QString errorString() const { return m_socket.errorString(); }

public slots:
    void SendData(const QByteArray& data);

protected:
    QHostAddress serverAddress() const { return m_socket.peerAddress(); }

private slots:
    void readyRead();

private:
    QUdpSocket m_socket;
    QHostAddress m_address;
    QSet<QString> m_clients;
    quint16 m_port;
    bool m_isBound;
};

#endif // UDPSERVER_H
