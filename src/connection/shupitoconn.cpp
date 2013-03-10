#include "shupitoconn.h"
#include <stdint.h>

ShupitoConnection::ShupitoConnection(ConnectionType type)
    : Connection(type), m_renameConfig(nullptr), m_persistScheduled(false)
{
    connect(this, SIGNAL(descRead(ShupitoDesc)), this, SLOT(descriptorChanged(ShupitoDesc)));
    connect(this, SIGNAL(stateChanged(ConnectionState)), this, SLOT(connectionStateChanged(ConnectionState)));
}

bool ShupitoConnection::isNamePersistable() const
{
    return m_renameConfig != nullptr;
}

void ShupitoConnection::persistName()
{
    Q_ASSERT(m_renameConfig);

    // TODO: how can we improve UX in here?
    if (!this->isOpen())
    {
        m_persistScheduled = true;
        this->OpenConcurrent();
    }
    else
    {
        this->doPersist();
    }
}

void ShupitoConnection::connectionStateChanged(ConnectionState state)
{
    if (state == st_connected && m_persistScheduled)
    {
        m_persistScheduled = false;
        this->doPersist();
        this->Close();
    }
}

void ShupitoConnection::doPersist()
{
    Q_ASSERT(this->isOpen());

    QString name = this->name();
    ShupitoPacket p = makeShupitoPacket(m_renameConfig->cmd, 1, 0);
    p.insert(p.end(), (uint8_t const *)name.data(), (uint8_t const *)(name.data() + name.size()));
    this->sendPacket(p);
    this->setName(name, /*isDefault=*/true);
}

void ShupitoConnection::descriptorChanged(ShupitoDesc const & desc)
{
    m_renameConfig = desc.getConfig("64d5bf39-468a-4fbb-80bb-334d8ca3ad81");
}

PortShupitoConnection::PortShupitoConnection()
    : ShupitoConnection(CONNECTION_PORT_SHUPITO),
      m_port(0),
      m_holdsTabRef(false),
      m_parserState(pst_discard),
      m_readDesc(false)
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

void PortShupitoConnection::doOpen()
{
    if (!m_port)
        return;

    m_readDesc = false;
    this->SetState(st_connecting);

    // add TabRef so that connection is not closed when another tab
    // with this PortConnection calls releaseTab();
    addPortTabRef();

    switch (m_port->state())
    {
    case st_connected:
        m_parserState = pst_init0;
        this->SetState(st_connected);
        break;
    case st_disconnecting:
        this->SetState(st_disconnected);
        break;
    default:
        m_port->OpenConcurrent();
        if (m_port->state() == st_disconnected)
        {
            this->SetState(st_disconnected);
            releasePortTabRef();
        }
    }
}

void PortShupitoConnection::doClose()
{
    emit disconnecting();
    this->SetState(st_disconnected);
    releasePortTabRef();
}

void PortShupitoConnection::portStateChanged(ConnectionState state)
{
    switch (state)
    {
    case st_missing:
        this->SetState(st_missing);
        break;
    case st_connect_pending:
        this->SetState(st_connect_pending);
        break;
    case st_disconnected:
        releasePortTabRef();
        this->SetState(st_disconnected);
        break;
    case st_disconnecting:
        this->SetState(st_disconnecting);
        break;
    case st_connected:
        m_parserState = pst_init0;
        this->SetState(st_connected);
        break;
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
                this->handlePacket(m_partialPacket);
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
                this->handlePacket(m_partialPacket);
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

void PortShupitoConnection::requestDesc()
{
    if (!m_readDesc)
        this->sendPacket(makeShupitoPacket(0, 1, 0x00));
    m_readDesc = true;
}

void PortShupitoConnection::handlePacket(ShupitoPacket const & packet)
{
    if (m_readDesc && packet[0] == 0)
    {
        m_partialDesc.append((char const *)packet.data() + 1, packet.size() - 1);
        if (packet.size() < 16)
        {
            m_readDesc = false;

            ShupitoDesc desc;
            try
            {
                desc.AddData(m_partialDesc);
                emit descRead(desc);
            }
            catch (...)
            {
            }

            m_partialPacket.clear();
        }
    }
    else
    {
        emit packetRead(packet);
    }
}
