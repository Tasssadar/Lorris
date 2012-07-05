/**********************************************
**    This file is part of Lorris
**    http://tasssadar.github.com/Lorris/
**
**    See README and COPYING
***********************************************/

#include "connectionmgr2.h"
#include "../connection/serialport.h"
#include "../connection/tcpsocket.h"
#include <qextserialenumerator.h>
#include "../config.h"
#include <QStringBuilder>

#ifdef HAVE_LIBUSBY
#include "usbshupitoconn.h"
#endif // HAVE_LIBUSBY

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
        (*it)->releaseAll();
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
            ConnectionPointer<SerialPort> portGuard(new SerialPort());
            portGuard->setName(info.portName);
            portGuard->setDeviceName(info.physName);
            portGuard->setFriendlyName(info.friendName);
            portGuard->setBaudRate(38400);
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

#ifdef HAVE_LIBUSBY

class UsbEventDispatcher : public QThread
{
public:
    UsbEventDispatcher(libusby::context & ctx)
        : m_ctx(ctx)
    {
    }

    void run()
    {
        m_ctx.run_event_loop();
    }

private:
    libusby::context & m_ctx;
};

UsbShupitoEnumerator::UsbShupitoEnumerator()
{
    m_usb_ctx.create();
    m_eventDispatcher.reset(new UsbEventDispatcher(m_usb_ctx));
    m_eventDispatcher->start();

    connect(&m_refreshTimer, SIGNAL(timeout()), this, SLOT(refresh()));
    m_refreshTimer.start(1000);
}

UsbShupitoEnumerator::~UsbShupitoEnumerator()
{
    std::map<libusby::device, UsbShupitoConnection *> seen_devices;
    seen_devices.swap(m_seen_devices);

    for (std::map<libusby::device, UsbShupitoConnection *>::const_iterator it = seen_devices.begin(); it != seen_devices.end(); ++it)
        it->second->releaseAll();

    QHash<QString, UsbShupitoConnection *> stand_by_conns;
    stand_by_conns.swap(m_stand_by_conns);

    for (QHash<QString, UsbShupitoConnection *>::const_iterator it = stand_by_conns.begin(); it != stand_by_conns.end(); ++it)
        it.value()->releaseAll();

    m_usb_ctx.stop_event_loop();
    m_eventDispatcher->wait();
}

QVariant UsbShupitoEnumerator::config() const
{
    QHash<QString, QVariant> dev_names;

    for (QHash<UsbShupitoConnection *, QString>::const_iterator it = m_unique_ids.begin(); it != m_unique_ids.end(); ++it)
    {
        if (!it.key()->persistent())
            continue;

        QHash<QString, QVariant> dev_config;
        dev_config["name"] = it.key()->name();
        dev_names[it.value()] = dev_config;
    }

    return dev_names;
}

bool UsbShupitoEnumerator::applyConfig(QVariant const & config)
{
    QHash<QString, QVariant> dev_names = config.toHash();
    for (QHash<QString, QVariant>::const_iterator it = dev_names.begin(); it != dev_names.end(); ++it)
    {
        QHash<QString, QVariant> dev_config = it.value().toHash();

        ConnectionPointer<UsbShupitoConnection> conn(new UsbShupitoConnection(m_usb_ctx));
        connect(conn.data(), SIGNAL(destroying()), this, SLOT(shupitoConnectionDestroyed()));

        conn->setName(dev_config["name"].toString());
        m_unique_ids[conn.data()] = it.key();
        m_stand_by_conns[it.key()] = conn.data();
        conn->setPersistent(true);

        sConMgr2.addConnection(conn.data());
    }

    return true;
}

void UsbShupitoEnumerator::refresh()
{
    libusby::device_list dev_list = m_usb_ctx.get_device_list();

    std::map<libusby::device, UsbShupitoConnection *> unseen_devices = m_seen_devices;
    for (std::size_t i = 0; i < dev_list.size(); ++i)
    {
        libusby::device & dev = dev_list[i];
        if (m_seen_devices.find(dev) != m_seen_devices.end())
        {
            unseen_devices.erase(dev);
            continue;
        }
        UsbShupitoConnection *& seen_conn = m_seen_devices[dev];
        seen_conn = 0;

        if (UsbShupitoConnection::isDeviceSupported(dev))
        {
            QString uniqueId;

            libusby_device_descriptor dev_desc;
            if (libusby_get_device_descriptor_cached(dev.get(), &dev_desc) >= 0 && dev_desc.iSerialNumber != 0)
            {
                try
                {
                    libusby::device_handle handle(dev);
                    std::string sn = handle.get_string_desc_utf8(dev_desc.iSerialNumber);
                    uniqueId = QString("%1:%2:%3").arg(QString::number(dev_desc.idVendor, 16), QString::number(dev_desc.idProduct, 16), QString::fromUtf8(sn.data(), sn.size()));
                }
                catch (libusby::error const &)
                {
                }
            }

            ConnectionPointer<UsbShupitoConnection> conn;

            if (!uniqueId.isNull())
            {
                // Look for a device with the same unique id in the stand by list
                conn.reset(m_stand_by_conns.value(uniqueId));
                if (conn)
                {
                    conn->addRef();
                    m_stand_by_conns.remove(uniqueId);

                    if (!conn->setUsbDevice(dev))
                        continue;
                }
            }

            if (!conn)
            {
                conn.reset(new UsbShupitoConnection(m_usb_ctx));
                connect(conn.data(), SIGNAL(destroying()), this, SLOT(shupitoConnectionDestroyed()));

                if (!uniqueId.isNull())
                    m_unique_ids[conn.data()] = uniqueId;

                if (!conn->setUsbDevice(dev))
                    continue;

                conn->setName(conn->product());

                sConMgr2.addConnection(conn.data());
                conn->setPersistent(true);
            }

            conn->setRemovable(false);
            seen_conn = conn.data();
            conn.take();
            continue;
        }
    }

    for (std::map<libusby::device, UsbShupitoConnection *>::const_iterator it = unseen_devices.begin(); it != unseen_devices.end(); ++it)
    {
        UsbShupitoConnection * conn = it->second;

        // The device is gone, but the connection may still have clients.
        // Do not destroy the connection completely, let the clients keep using it.
        // We merely make the device removable so that it can be removed by the user.
        if (conn)
        {
            conn->setUsbDevice(libusby::device());
            conn->setRemovable(true);

            // If the connection has a unique id, keep a reference to it,
            // so that we can revive it if it is connected again.
            QString unique_id = m_unique_ids.value(conn);
            if (!unique_id.isNull())
                m_stand_by_conns[unique_id] = conn;

            conn->release();
        }

        m_seen_devices.erase(it->first);
    }
}

void UsbShupitoEnumerator::shupitoConnectionDestroyed()
{
    UsbShupitoConnection * conn = static_cast<UsbShupitoConnection *>(this->sender());
    m_seen_devices.erase(conn->usbDevice());

    QString uniqueId = m_unique_ids.value(conn);
    if (!uniqueId.isNull())
        m_stand_by_conns.remove(uniqueId);
    m_unique_ids.remove(conn);
}

#endif // HAVE_LIBUSBY

ConnectionManager2::ConnectionManager2(QObject * parent)
    : QObject(parent)
{
    Q_ASSERT(psConMgr2 == 0);
    psConMgr2 = this;

    m_serialPortEnumerator.reset(new SerialPortEnumerator());
#ifdef HAVE_LIBUSBY
    m_usbShupitoEnumerator.reset(new UsbShupitoEnumerator());
#endif // HAVE_LIBUSBY

    QVariant config = sConfig.get(CFG_VARIANT_CONNECTIONS);
    if (config.isValid())
        this->applyConfig(config);

#ifdef HAVE_LIBUSBY
    config = sConfig.get(CFG_VARIANT_USB_ENUMERATOR);
    if (config.isValid())
        m_usbShupitoEnumerator->applyConfig(config);
#endif // HAVE_LIBUSBY
}

ConnectionManager2::~ConnectionManager2()
{
    // If this were a perfect world, the config would be stored in a *structured*
    // storage, also known as JSON.
    sConfig.set(CFG_VARIANT_CONNECTIONS, this->config());
#ifdef HAVE_LIBUSBY
    sConfig.set(CFG_VARIANT_USB_ENUMERATOR, m_usbShupitoEnumerator->config());
#endif // HAVE_LIBUSBY

    m_serialPortEnumerator.reset();
#ifdef HAVE_LIBUSBY
    m_usbShupitoEnumerator.reset();
#endif // HAVE_LIBUSBY

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
        ~cleanup() { for (int i = 0; i < conns.size(); ++i) conns[i]->releaseAll(); }
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

        ConnectionPointer<Connection> conn;
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
        conn->releaseAll();
    }
}

void ConnectionManager2::refresh()
{
    m_serialPortEnumerator->refresh();

#ifdef HAVE_LIBUSBY
    m_usbShupitoEnumerator->refresh();
#endif // HAVE_LIBUSBY
}

SerialPort * ConnectionManager2::createSerialPort()
{
    ConnectionPointer<SerialPort> conn(new SerialPort());
    this->addUserOwnedConn(conn.data());
    return conn.take();
}

TcpSocket * ConnectionManager2::createTcpSocket()
{
    ConnectionPointer<TcpSocket> conn(new TcpSocket());
    this->addUserOwnedConn(conn.data());
    return conn.take();
}

void ConnectionManager2::addConnection(Connection * conn)
{
    connect(conn, SIGNAL(destroying()), this, SLOT(connectionDestroyed()));
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

ConnectionPointer<ShupitoConnection> ConnectionManager2::createAutoShupito(PortConnection * parentConn)
{
    Q_ASSERT(parentConn);

    if (m_autoShupitos.contains(parentConn))
    {
        ShupitoConnection * res = m_autoShupitos[parentConn];
        res->addRef();
        return ConnectionPointer<ShupitoConnection>(res);
    }

    ConnectionPointer<PortShupitoConnection> res(new PortShupitoConnection());
    res->setName("Shupito at " % parentConn->name());
    res->setPort(ConnectionPointer<PortConnection>::fromPtr(parentConn));
    this->addConnection(res.data());
    connect(res.data(), SIGNAL(destroying()), this, SLOT(autoShupitoDestroyed()));

    // TODO: exception safety
    m_autoShupitos[parentConn] = res.data();
    m_autoShupitosRev[res.data()] = parentConn;
    parentConn->addRef();

    return res;
}

void ConnectionManager2::autoShupitoDestroyed()
{
    // Remember that the sender's dynamic type has been demoted to QObject by now.
    QObject * conn = this->sender();
    Q_ASSERT(m_autoShupitosRev.contains(conn));

    PortConnection * port = m_autoShupitosRev[conn];
    m_autoShupitos.remove(port);
    m_autoShupitosRev.remove(conn);
    port->release();
}
