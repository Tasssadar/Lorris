#include "shupitoconn.h"

PortShupitoConnection::PortShupitoConnection()
    : ShupitoConnection(CONNECTION_PORT_SHUPITO),
      m_port(0),
      m_holdsTabRef(false),
      m_parserState(pst_discard)
{
}

PortShupitoConnection::~PortShupitoConnection()
{
    this->Close();
    Q_ASSERT(!m_holdsTabRef);
}

void PortShupitoConnection::setPort(ConnectionPointer<PortConnection> const & port)
{
    Q_ASSERT(this->state() == st_disconnected);

    if (m_port == port)
        return;

    if (m_port)
    {
        disconnect(m_port.data(), 0, this, 0);
        releasePortTabRef();
    }

    m_port = port;

    if (m_port)
    {
        connect(m_port.data(), SIGNAL(disconnecting()), this, SLOT(portDisconnecting()));
        connect(m_port.data(), SIGNAL(destroying()), this, SLOT(portDestroyed()));
        connect(m_port.data(), SIGNAL(stateChanged(ConnectionState)), this, SLOT(portStateChanged(ConnectionState)));
        connect(m_port.data(), SIGNAL(dataRead(QByteArray)), this, SLOT(portDataRead(QByteArray)));
        this->portStateChanged(m_port->state());
    }
}

void PortShupitoConnection::OpenConcurrent()
{
    if (this->state() != st_disconnected)
        return;

    if (!m_port)
        return;

    this->SetState(st_connecting);

    // add TabRef so that connection is not closed when another tab
    // with this PortConnection calls releaseTab();
    addPortTabRef();

    if (m_port->state() != st_connected)
        m_port->OpenConcurrent();
    else
    {
        m_parserState = pst_init0;
        this->SetState(st_connected);
    }
}

void PortShupitoConnection::Close()
{
    if (this->state() == st_connected)
    {
        emit disconnecting();
        this->SetState(st_disconnected);
        releasePortTabRef();
    }
}

void PortShupitoConnection::portStateChanged(ConnectionState state)
{
    if (state == st_disconnected)
    {
        releasePortTabRef();
        this->SetState(st_disconnected);
    }
    else if (state == st_connected && this->state() == st_connecting)
    {
        m_parserState = pst_init0;
        this->SetState(st_connected);
    }
}

void PortShupitoConnection::portDisconnecting()
{
    this->Close();
}

void PortShupitoConnection::portDestroyed()
{
    this->portStateChanged(st_disconnected);
}

void PortShupitoConnection::portDataRead(QByteArray const & data)
{
    for (int i = 0; i < data.size(); ++i)
    {
        quint8 ch = (quint8)data[i];

        switch (m_parserState)
        {
        case pst_init0:
            if (ch == 0x80)
                m_parserState = pst_init1;
            break;
        case pst_init1:
            if (ch == 0x0f)
                m_parserState = pst_init2;
            else if (ch != 0x80)
                m_parserState = pst_init0;
            break;
        case pst_init2:
            if (ch == 0x01)
            {
                m_parserState = pst_data;
                m_parserLen = 0xf;
                m_partialPacket.clear();
                m_partialPacket.push_back(0x00);
                m_partialPacket.push_back(0x01);
            }
            else if (ch == 0x80)
            {
                m_parserState = pst_init1;
            }
            else
            {
                m_parserState = pst_init0;
            }
            break;
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

void PortShupitoConnection::sendPacket(ShupitoPacket const & packet)
{
    Q_ASSERT(packet.size() >= 1 && packet.size() <= 16);
    Q_ASSERT(packet[0] < 16);

    QByteArray data;
    data[0] = 0x80;
    data.append((char const *)packet.data(), packet.size());
    data[1] = (data[1] << 4) | (packet.size() - 1);
    m_port->SendData(data);
}

void PortShupitoConnection::addPortTabRef()
{
    if(!m_port || m_holdsTabRef)
        return;

    m_holdsTabRef = true;
    m_port->addTabRef();
}

void PortShupitoConnection::releasePortTabRef()
{
    if(!m_holdsTabRef)
        return;

    m_holdsTabRef = false;

    if(m_port)
        m_port->releaseTab();
}
