/**********************************************
**    This file is part of Lorris
**    http://tasssadar.github.com/Lorris/
**
**    See README and COPYING
***********************************************/

#include <QtNetwork/QTcpSocket>
#include <QtCore/QtConcurrentRun>
#include <QtCore/QThreadPool>
#include <QLabel>
#include <QLineEdit>
#include <QSpinBox>
#include <QStringBuilder>

#include "../common.h"
#include "tcpsocket.h"
#include "../WorkTab/WorkTabInfo.h"
#include "../WorkTab/WorkTab.h"
#include "../WorkTab/WorkTabMgr.h"

static const int CONNECT_TIMEOUT = 10000 / 50; // 10s

TcpSocket::TcpSocket()
    : PortConnection(CONNECTION_TCP_SOCKET)
{
    m_port = 0;

    m_socket = new QTcpSocket(this);

    connect(m_socket,   SIGNAL(readyRead()),                                SLOT(readyRead()));
    connect(m_socket,   SIGNAL(stateChanged(QAbstractSocket::SocketState)), SLOT(stateChanged()));
    connect(&m_watcher, SIGNAL(finished()),                                 SLOT(tcpConnectResult()));
}

TcpSocket::~TcpSocket()
{
    Close();
    delete m_socket;
}

QString TcpSocket::details() const
{
    QString res = Connection::details();
    if (!res.isEmpty())
        res += ", ";
    return res % this->host() % ":" % QString::number(this->port());
}

bool TcpSocket::Open()
{
    return false;
}

void TcpSocket::Close()
{
    if(m_future.isRunning())
    {
        m_future.cancel();
        m_future.waitForFinished();
    }

    m_socket->close();

    this->SetOpen(false);
}

void TcpSocket::connectResultSer(bool opened)
{
    this->SetOpen(opened);
}

void TcpSocket::OpenConcurrent()
{
    if(this->isOpen())
        return;

    this->SetState(st_connecting);
    m_socket->connectToHost(m_address, m_port);

    m_future = QtConcurrent::run(this, &TcpSocket::connectToHost);
    m_watcher.setFuture(m_future);
}

bool TcpSocket::connectToHost()
{
    int time = 0;
    for(; !m_future.isCanceled() && time < CONNECT_TIMEOUT && m_socket->state() == QAbstractSocket::HostLookupState; ++time)
    {
        Utils::msleep(50);
    }

    for(; !m_future.isCanceled() && time < CONNECT_TIMEOUT && m_socket->state() == QAbstractSocket::ConnectingState; ++time)
    {
        Utils::msleep(50);
    }

    return (m_socket->state() == QAbstractSocket::ConnectedState);
}

void TcpSocket::tcpConnectResult()
{
    connectResultSer(m_future.result());
}

void TcpSocket::SendData(const QByteArray &data)
{
    m_socket->write(data);
}

void TcpSocket::readyRead()
{
    QByteArray data = m_socket->readAll();
    emit dataRead(data);
}

void TcpSocket::stateChanged()
{
    if(this->isOpen() && m_socket->state() != QAbstractSocket::ConnectedState)
    {
        sWorkTabMgr.printToAllStatusBars(tr("Connection to %1:%2 lost!").arg(m_address).arg(m_port));
        Close();
    }
}

void TcpSocket::setHost(QString const & value)
{
    if (value != m_address)
    {
        m_address = value;
        emit changed();
    }
}

void TcpSocket::setPort(quint16 value)
{
    if (value != m_port)
    {
        m_port = value;
        emit changed();
    }
}

QHash<QString, QVariant> TcpSocket::config() const
{
    QHash<QString, QVariant> res = this->Connection::config();
    res["host"] = this->host();
    res["port"] = this->port();
    return res;
}

bool TcpSocket::applyConfig(QHash<QString, QVariant> const & config)
{
    this->setHost(config.value("host").toString());
    this->setPort(config.value("port", 80).toInt());
    return this->Connection::applyConfig(config);
}
