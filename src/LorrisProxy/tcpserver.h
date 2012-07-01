/**********************************************
**    This file is part of Lorris
**    http://tasssadar.github.com/Lorris/
**
**    See README and COPYING
***********************************************/

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
