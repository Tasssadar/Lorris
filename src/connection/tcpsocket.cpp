#include <QtNetwork/QTcpSocket>
#include <QtCore/QtConcurrentRun>
#include <QtCore/QThreadPool>

#include "common.h"
#include "connectionmgr.h"
#include "tcpsocket.h"

TcpSocket::TcpSocket() : Connection()
{
    m_type = CONNECTION_TCP_SOCKET;

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

bool TcpSocket::Open()
{
    return false;
}

void TcpSocket::Close()
{
    m_socket->close();

    emit connected(false);
    opened = false;
}

void TcpSocket::connectResultSer(bool opened)
{
    this->opened = opened;
    emit connectResult(this, opened);

    if(opened)
        emit connected(true);
}

void TcpSocket::OpenConcurrent()
{
    m_socket->connectToHost(m_address, m_port);

    m_future = QtConcurrent::run(this, &TcpSocket::connectToHost);
    m_watcher.setFuture(m_future);
}

bool TcpSocket::connectToHost()
{
    while(m_socket->state() == QAbstractSocket::HostLookupState)
    {
        Utils::msleep(50);
    }

    while(m_socket->state() == QAbstractSocket::ConnectingState)
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
    if(opened && m_socket->state() != QAbstractSocket::ConnectedState)
        Close();
}
