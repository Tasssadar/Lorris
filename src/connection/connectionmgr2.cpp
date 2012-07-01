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
    UsbEventDispatcher(libusby_context * ctx)
        : m_ctx(ctx)
    {
    }

    void run()
    {
        libusby_run_event_loop(m_ctx);
    }

private:
    libusby_context * m_ctx;
};

UsbShupitoEnumerator::UsbShupitoEnumerator()
    : m_usb_ctx(0)
{
    int res = libusby_init(&m_usb_ctx);
    m_eventDispatcher.reset(new UsbEventDispatcher(m_usb_ctx));
    m_eventDispatcher->start();

    connect(&m_refreshTimer, SIGNAL(timeout()), this, SLOT(refresh()));
    m_refreshTimer.start(1000);
}

UsbShupitoEnumerator::~UsbShupitoEnumerator()
{
    while (!m_devmap.isEmpty())
        (*m_devmap.begin())->releaseAll();

    libusby_stop_event_loop(m_usb_ctx);
    m_eventDispatcher->wait();
    libusby_exit(m_usb_ctx);
}

QVariant UsbShupitoEnumerator::config() const
{
    QHash<QString, QVariant> dev_names;

    for (QHash<QString, QString>::const_iterator it = m_dev_names.begin(); it != m_dev_names.end(); ++it)
    {
        QHash<QString, QVariant> dev_config;
        dev_config["name"] = it.key();
        dev_names[it.value()] = dev_config;
    }

    for (QHash<Connection *, QString>::const_iterator it = m_unique_ids.begin(); it != m_unique_ids.end(); ++it)
    {
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
        m_dev_names[it.key()] = dev_config["name"].toString();
    }

    return true;
}

void UsbShupitoEnumerator::refresh()
{
    if (!m_usb_ctx)
        return;

    libusby_device ** dev_list = 0;
    int dev_count = libusby_get_device_list(m_usb_ctx, &dev_list);
    if (dev_count < 0)
        return;

    QList<Connection *> portsToDisown = m_devmap.values();
    for (int i = 0; i < dev_count; ++i)
    {
        libusby_device * dev = dev_list[i];
        if (UsbShupitoConnection::isDeviceSupported(dev))
        {
            if (m_devmap.contains(dev))
            {
                portsToDisown.removeOne(m_devmap[dev]);
            }
            else
            {
                ConnectionPointer<UsbShupitoConnection> conn(new UsbShupitoConnection(m_usb_ctx));
                if (!conn->setUsbDevice(dev))
                    continue;
                conn->setRemovable(false);
                conn->setName(conn->product());

                QString const & sn = conn->serialNumber();
                if (!sn.isEmpty())
                {
                    libusby_device_descriptor dev_desc;
                    if (libusby_get_device_descriptor_cached(dev, &dev_desc) >= 0)
                    {
                        QString uniqueId = QString("%1:%2:%3").arg(QString::number(dev_desc.idVendor, 16), QString::number(dev_desc.idProduct, 16), sn);
                        m_unique_ids.insert(conn.data(), uniqueId);

                        QString name = m_dev_names.value(uniqueId);
                        if (!name.isNull())
                            conn->setName(name);
                    }
                }

                m_devmap[dev] = conn.data();
                connect(conn.data(), SIGNAL(destroying()), this, SLOT(shupitoConnectionDestroyed()));
                sConMgr2.addConnection(conn.take());
            }
            continue;
        }

        if (UsbAcmConnection::isDeviceSupported(dev))
        {
            if (m_devmap.contains(dev))
            {
                portsToDisown.removeOne(m_devmap[dev]);
            }
            else
            {
                ConnectionPointer<UsbAcmConnection> conn(new UsbAcmConnection(m_usb_ctx));
                if (!conn->setUsbDevice(dev))
                    continue;
                conn->setRemovable(false);
                conn->setName(conn->product());
                m_devmap[dev] = conn.data();
                connect(conn.data(), SIGNAL(destroying()), this, SLOT(acmConnectionDestroyed()));
                sConMgr2.addConnection(conn.take());
            }
            continue;
        }
    }

    libusby_free_device_list(dev_list, 1);

    for (int i = 0; i < portsToDisown.size(); ++i)
        portsToDisown[i]->releaseAll();
}

void UsbShupitoEnumerator::acmConnectionDestroyed()
{
    UsbAcmConnection * conn = static_cast<UsbAcmConnection *>(this->sender());
    m_devmap.remove(conn->usbDevice());
}

void UsbShupitoEnumerator::shupitoConnectionDestroyed()
{
    UsbShupitoConnection * conn = static_cast<UsbShupitoConnection *>(this->sender());
    m_devmap.remove(conn->usbDevice());

    QString uniqueId = m_unique_ids.value(conn);
    if (!uniqueId.isNull())
        m_dev_names[uniqueId] = conn->name();
    m_unique_ids.remove(conn);
}

#endif // HAVE_LIBUSBY

ConnectionManager2::ConnectionManager2(QObject * parent)
    : QObject(parent)
{
    m_serialPortEnumerator.reset(new SerialPortEnumerator());
#ifdef HAVE_LIBUSBY
    m_usbShupitoEnumerator.reset(new UsbShupitoEnumerator());
#endif // HAVE_LIBUSBY

    QVariant config = sConfig.get(CFG_VARIANT_CONNECTIONS);
    if (config.isValid())
        this->applyConfig(config);

    config = sConfig.get(CFG_VARIANT_USB_ENUMERATOR);
    if (config.isValid())
        m_usbShupitoEnumerator->applyConfig(config);
}

ConnectionManager2::~ConnectionManager2()
{
    // If this were a perfect world, the config would be stored in a *structured*
    // storage, also known as JSON.
    sConfig.set(CFG_VARIANT_CONNECTIONS, this->config());
    sConfig.set(CFG_VARIANT_USB_ENUMERATOR, m_usbShupitoEnumerator->config());

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
