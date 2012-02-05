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
