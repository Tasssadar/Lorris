#include "shupitoconn.h"

ShupitoConnection::ShupitoConnection()
    : Connection(CONNECTION_SHUPITO),
      m_port(0),
      m_parserState(pst_discard)
{
}

ShupitoConnection::~ShupitoConnection()
{
    this->Close();
}

void ShupitoConnection::setPort(ConnectionPointer<PortConnection> const & port)
{
    Q_ASSERT(this->state() == st_disconnected);

    if (m_port == port)
        return;

    if (m_port)
        disconnect(m_port.data(), 0, this, 0);

    m_port = port;

    if (m_port)
    {
        connect(m_port.data(), SIGNAL(disconnecting()), this, SLOT(portDisconnecting()));
        connect(m_port.data(), SIGNAL(destroyed()), this, SLOT(portDestroyed()));
        connect(m_port.data(), SIGNAL(stateChanged(ConnectionState)), this, SLOT(portStateChanged(ConnectionState)));
        connect(m_port.data(), SIGNAL(dataRead(QByteArray)), this, SLOT(portDataRead(QByteArray)));
        this->portStateChanged(m_port->state());
    }
}

void ShupitoConnection::OpenConcurrent()
{
    if (this->state() != st_disconnected)
        return;

    if (!m_port)
        return;

    this->SetState(st_connecting);
    if (m_port->state() != st_connected)
        m_port->OpenConcurrent();
}

void ShupitoConnection::Close()
{
    if (this->state() == st_connected)
    {
        emit disconnecting();
        this->SetState(st_disconnected);
        m_port->Close();
    }
}

void ShupitoConnection::portStateChanged(ConnectionState state)
{
    if (this->state() == st_disconnected && state != st_disconnected)
        this->SetState(st_connecting);

    if (this->state() == st_connecting)
    {
        if (state == st_disconnected)
            this->SetState(st_disconnected);
        if (state == st_connected)
            this->SetState(st_connected);
    }
}

void ShupitoConnection::portDisconnecting()
{
    this->Close();
}

void ShupitoConnection::portDestroyed()
{
    this->portStateChanged(st_disconnected);
}

void ShupitoConnection::portDataRead(QByteArray const & data)
{
    for (int i = 0; i < data.size(); ++i)
    {
        quint8 ch = (quint8)data[i];

        switch (m_parserState)
        {
        case pst_discard:
            if (ch == 0x80)
            {
                m_parserState = pst_cmd;
                m_partialPacket.clear();
            }
            break;
        case pst_cmd:
            m_parserLen = ch & 0xf;
            m_partialPacket.clear();
            m_partialPacket.push_back(ch >> 4);
            if (m_parserLen == 0)
            {
                emit packetRead(m_partialPacket);
                m_parserState = pst_discard;
            }
            else
            {
                m_parserState = pst_data;
            }
            break;
        case pst_data:
            Q_ASSERT(m_partialPacket.size() < m_parserLen + 1);
            m_partialPacket.push_back(ch);
            if (m_partialPacket.size() == m_parserLen + 1)
            {
                emit packetRead(m_partialPacket);
                m_parserState = pst_discard;
            }
            break;
        }
    }
}

void ShupitoConnection::sendPacket(ShupitoPacket const & packet)
{
    Q_ASSERT(packet.size() >= 1 && packet.size() <= 16);
    Q_ASSERT(packet[0] < 16);

    QByteArray data;
    data[0] = 0x80;
    data.append((char const *)packet.data(), packet.size());
    data[1] = (data[1] << 4) | (packet.size() - 1);
    m_port->SendData(data);
}
