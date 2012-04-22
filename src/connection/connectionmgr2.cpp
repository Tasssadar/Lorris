#include "connectionmgr2.h"
#include "../connection/serialport.h"
#include "../connection/tcpsocket.h"
#include <qextserialenumerator.h>
#include "../config.h"

ConnectionManager2 * psConMgr2 = 0;

SerialPortEnumerator::SerialPortEnumerator()
{
    connect(&m_refreshTimer, SIGNAL(timeout()), this, SLOT(refresh()));
    m_refreshTimer.start(1000);
}

SerialPortEnumerator::~SerialPortEnumerator()
{
    std::set<SerialPort *> portsToClear;
    portsToClear.swap(m_ownedPorts);

    for (std::set<SerialPort *>::iterator it = portsToClear.begin(); it != portsToClear.end(); ++it)
        delete *it;
}

void SerialPortEnumerator::refresh()
{
    std::set<SerialPort *> portsToDisown = m_ownedPorts;

    QList<QextPortInfo> ports = QextSerialEnumerator::getPorts();
    for (int i = 0; i < ports.size(); ++i)
    {
        QextPortInfo info = ports[i];

        QHash<QString, SerialPort *>::const_iterator it = m_portMap.find(ports[i].physName);
        if (it == m_portMap.end())
        {
            QScopedPointer<SerialPort> portGuard(new SerialPort());
            portGuard->setName(info.portName);
            portGuard->setDeviceName(info.physName);
            portGuard->setBaudRate(BAUD38400);
            portGuard->setDevNameEditable(false);

            connect(portGuard.data(), SIGNAL(destroyed()), this, SLOT(connectionDestroyed()));
            m_portMap[info.physName] = portGuard.data();

            sConMgr2.addConnection(portGuard.data());

            m_ownedPorts.insert(portGuard.data());
            portGuard->setRemovable(false);
            portGuard.take();
        }
        else
        {
            SerialPort * port = it.value();

            if (m_ownedPorts.find(port) == m_ownedPorts.end())
            {
                m_ownedPorts.insert(port);
                port->setRemovable(false);
                port->addRef();
            }
            else
            {
                portsToDisown.erase(port);
            }
        }
    }

    for (std::set<SerialPort *>::const_iterator it = portsToDisown.begin(); it != portsToDisown.end(); ++it)
    {
        SerialPort * port = *it;
        m_ownedPorts.erase(port);
        port->setRemovable(true);
        port->release();
    }
}

void SerialPortEnumerator::connectionDestroyed()
{
    SerialPort * port = static_cast<SerialPort *>(this->sender());
    Q_ASSERT(m_ownedPorts.find(port) == m_ownedPorts.end());
    m_portMap.remove(port->deviceName());
}

ConnectionManager2::ConnectionManager2(QObject * parent)
    : QObject(parent)
{
    m_serialPortEnumerator.reset(new SerialPortEnumerator());

    QVariant config = sConfig.get(CFG_VARIANT_CONNECTIONS);
    if (config.isValid())
        this->applyConfig(config);
}

ConnectionManager2::~ConnectionManager2()
{
    // If this were a perfect world, the config would be stored in a *structured*
    // storage, also known as JSON.
    sConfig.set(CFG_VARIANT_CONNECTIONS, this->config());

    m_serialPortEnumerator.reset();

    // All of the remaining connections should be owned by the manager and should
    // therefore be removable.
    Q_ASSERT(m_conns.size() == m_userOwnedConns.size());
    this->clearUserOwnedConns();

    Q_ASSERT(m_conns.size() == 0);
    Q_ASSERT(m_userOwnedConns.size() == 0);
}

QVariant ConnectionManager2::config() const
{
    QList<QVariant> connConfigs;
    for (QSet<Connection *>::const_iterator it = m_userOwnedConns.begin(); it != m_userOwnedConns.end(); ++it)
    {
        Connection * conn = *it;
        Q_ASSERT(conn);

        static char const * connTypes[] = {
            "serial_port", // CONNECTION_SERIAL_PORT
            "file",        // CONNECTION_FILE
            "shupito",     // CONNECTION_SHUPITO
            "tcp_client"   // CONNECTION_TCP_SOCKET
        };
        Q_ASSERT(sizeof connTypes / sizeof connTypes[0] == MAX_CON_TYPE);
        Q_ASSERT(conn->getType() < MAX_CON_TYPE);

        QHash<QString, QVariant> connConfig;
        connConfig["type"] = connTypes[conn->getType()];
        connConfig["settings"] = conn->config();

        connConfigs.push_back(connConfig);
    }

    return connConfigs;
}

bool ConnectionManager2::applyConfig(QVariant const & config)
{
    if (config.type() != QVariant::List)
        return false;

    QList<Connection *> newConns;
    struct cleanup
    {
        QList<Connection *> & conns;
        cleanup(QList<Connection *> & conns) : conns(conns) {}
        ~cleanup() { for (int i = 0; i < conns.size(); ++i) delete conns[i]; }
    } cleanupGuard(newConns);

    QList<QVariant> const & connConfigs = config.toList();

    for (int i = 0; i < connConfigs.size(); ++i)
    {
        if (connConfigs[i].type() != QVariant::Hash)
            return false;
        QHash<QString, QVariant> const & connConfig = connConfigs[i].toHash();

        QVariant typev = connConfig.value("type");
        if (typev.type() != QVariant::String)
            return false;

        QString const & type = typev.toString();

        QScopedPointer<Connection> conn;
        if (type == "serial_port")
            conn.reset(new SerialPort());
        else if (type == "tcp_client")
            conn.reset(new TcpSocket());

        if (!conn)
            return false;

        QVariant settings = connConfig.value("settings");
        if (settings.type() != QVariant::Hash || !conn->applyConfig(settings.toHash()))
            return false;

        newConns.push_back(conn.data());
        conn.take();
    }

    this->clearUserOwnedConns();
    while (!newConns.empty())
    {
        this->addUserOwnedConn(newConns.back());
        newConns.pop_back();
    }

    return true;
}

void ConnectionManager2::clearUserOwnedConns()
{
    while (!m_userOwnedConns.empty())
    {
        Connection * conn = *m_userOwnedConns.begin();
        Q_ASSERT(conn->removable());
        delete conn;
    }
}

void ConnectionManager2::refresh()
{
    m_serialPortEnumerator->refresh();
}

SerialPort * ConnectionManager2::createSerialPort()
{
    QScopedPointer<SerialPort> conn(new SerialPort());
    this->addUserOwnedConn(conn.data());
    return conn.take();
}

TcpSocket * ConnectionManager2::createTcpSocket()
{
    QScopedPointer<TcpSocket> conn(new TcpSocket());
    this->addUserOwnedConn(conn.data());
    return conn.take();
}

void ConnectionManager2::addConnection(Connection * conn)
{
    connect(conn, SIGNAL(destroyed()), this, SLOT(connectionDestroyed()));
    m_conns.append(conn);
    emit connAdded(conn);
}

void ConnectionManager2::addUserOwnedConn(Connection * conn)
{
    m_userOwnedConns.insert(conn);
    this->addConnection(conn);
}

void ConnectionManager2::connectionDestroyed()
{
    Connection * conn = static_cast<Connection *>(this->sender());
    m_userOwnedConns.remove(conn);
    if (m_conns.removeOne(conn))
        emit connRemoved(conn);
}
