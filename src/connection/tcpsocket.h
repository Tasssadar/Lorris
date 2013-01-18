/**********************************************
**    This file is part of Lorris
**    http://tasssadar.github.com/Lorris/
**
**    See README and COPYING
***********************************************/

#ifndef TCPSOCKET_H
#define TCPSOCKET_H

#include <QFuture>
#include <QFutureWatcher>

#include "connection.h"

class QSpinBox;
class QLineEdit;
class QTcpSocket;

class TcpSocket : public PortConnection
{
    Q_OBJECT
public:
    explicit TcpSocket();

    virtual QString details() const;

    void SendData(const QByteArray &data);
    void setAddress(const QString& address, quint16 port)
    {
        m_address = address;
        m_port = port;
    }

    QString host() const { return m_address; }
    void setHost(QString const & value);

    quint16 port() const { return m_port; }
    void setPort(quint16 value);

    QHash<QString, QVariant> config() const;
    bool applyConfig(QHash<QString, QVariant> const & config);
    bool canSaveToSession() const { return true; }

public slots:
    void connectResultSer(bool opened);
    void tcpConnectResult();
    void readyRead();
    void stateChanged();

protected:
    ~TcpSocket();
    void doOpen();
    void doClose();

private:
    bool connectToHost();

    QTcpSocket *m_socket;
    quint16 m_port;
    QString m_address;
    
    QFuture<bool> m_future;
    QFutureWatcher<bool> m_watcher;
};

#endif // TCPSOCKET_H
