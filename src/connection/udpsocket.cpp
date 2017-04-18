/**********************************************
**    This file is part of Lorris
**    http://tasssadar.github.com/Lorris/
**
**    See README and COPYING
***********************************************/

#include <QUdpSocket>
#include <QStringBuilder>
#include <QtConcurrent>

#include "../common.h"
#include "../WorkTab/WorkTabInfo.h"
#include "../WorkTab/WorkTab.h"
#include "../WorkTab/WorkTabMgr.h"

#include "udpsocket.h"

UdpSocket::UdpSocket() : PortConnection(CONNECTION_UDP_SOCKET)
{
    m_port = 0;
    m_socket = new QUdpSocket(this);

    connect(m_socket,   SIGNAL(readyRead()),                                SLOT(readyRead()));
    connect(m_socket,   SIGNAL(stateChanged(QAbstractSocket::SocketState)), SLOT(stateChanged()));
    connect(&m_watcher, SIGNAL(finished()),                                 SLOT(connectResult()));
}

UdpSocket::~UdpSocket() {
    Close();
    delete m_socket;
}

QString UdpSocket::details() const
{
    QString res = Connection::details();
    if (!res.isEmpty())
        res += ", ";
    return res % this->host() % ":" % QString::number(this->port());
}

void UdpSocket::doClose()
{
    if (m_future.isRunning())
    {
        m_future.cancel();
        m_future.waitForFinished();
    }

    m_socket->close();

    this->SetState(st_disconnected);
}

void UdpSocket::connectResultSer(bool opened)
{
    this->SetState(opened? st_connected: st_disconnected);
}

void UdpSocket::doOpen()
{
    this->SetState(st_connecting);
    if(!m_socket->bind(m_port)) {
        // even if the bind fails, we can still send data.
        sWorkTabMgr.printToAllStatusBars(tr("Failed to bind UDP socket to port %1 (\"%2\")").arg(m_port).arg(name()));
    }
    this->SetState(st_connected);
}

void UdpSocket::connectResult()
{
    connectResultSer(m_future.result());
}

void UdpSocket::SendData(const QByteArray &data)
{
    if(this->isOpen())
        m_socket->writeDatagram(data, QHostAddress(m_address), m_port);
}

void UdpSocket::readyRead()
{
    QByteArray data;
    while(m_socket->hasPendingDatagrams()) {
        data.resize(m_socket->pendingDatagramSize());
        if(m_socket->readDatagram(data.data(), data.size()) > 0)
            emit dataRead(data);
    }
}

void UdpSocket::stateChanged()
{
    if(this->isOpen() && m_socket->state() != QAbstractSocket::BoundState)
    {
        sWorkTabMgr.printToAllStatusBars(tr("Connection to %1:%2 lost!").arg(m_address).arg(m_port));
        Close();
    }
}

void UdpSocket::setHost(QString const & value)
{
    if (value != m_address)
    {
        m_address = value;
        emit changed();
    }
}

void UdpSocket::setPort(quint16 value)
{
    if (value != m_port)
    {
        m_port = value;
        emit changed();
    }
}

QHash<QString, QVariant> UdpSocket::config() const
{
    QHash<QString, QVariant> res = this->PortConnection::config();
    res["host"] = this->host();
    res["port"] = this->port();
    return res;
}

bool UdpSocket::applyConfig(QHash<QString, QVariant> const & config)
{
    this->setHost(config.value("host").toString());
    this->setPort(config.value("port", 80).toInt());
    return this->PortConnection::applyConfig(config);
}

ConnectionPointer<Connection> UdpSocket::clone()
{
    ConnectionPointer<UdpSocket> res(new UdpSocket());
    res->applyConfig(this->config());
    res->setName(tr("Clone of ") + this->name());
    return res;
}
