/****************************************************************************
**
**    This file is part of Lorris.
**    Copyright (C) 2012 Vojtěch Boček
**
**    Contact: <vbocek@gmail.com>
**             https://github.com/Tasssadar
**
**    Lorris is free software: you can redistribute it and/or modify
**    it under the terms of the GNU General Public License as published by
**    the Free Software Foundation, either version 3 of the License, or
**    (at your option) any later version.
**
**    Lorris is distributed in the hope that it will be useful,
**    but WITHOUT ANY WARRANTY; without even the implied warranty of
**    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
**    GNU General Public License for more details.
**
**    You should have received a copy of the GNU General Public License
**    along with Lorris.  If not, see <http://www.gnu.org/licenses/>.
**
****************************************************************************/

#include "connectionmgr2.h"
#include "../connection/serialport.h"
#include "../connection/tcpsocket.h"
#include <qextserialenumerator.h>
#include "../config.h"
#include <QStringBuilder>
#include "usbshupitoconn.h"
#include "libusb/lusb0_usb_dyn.h"

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

UsbShupitoEnumerator::UsbShupitoEnumerator()
    : m_um(0)
{
    m_um = libusb0_dyn_init();
    if (m_um)
    {
        m_um->usb_init();

        connect(&m_refreshTimer, SIGNAL(timeout()), this, SLOT(refresh()));
        m_refreshTimer.start(1000);
    }
}

UsbShupitoEnumerator::~UsbShupitoEnumerator()
{
    while (!m_devmap.isEmpty())
        (*m_devmap.begin())->releaseAll();

    if (m_um)
        libusb0_dyn_destroy(m_um);
}

void UsbShupitoEnumerator::refresh()
{
    if (!m_um)
        return;

    int res = m_um->usb_find_busses();
    if (res < 0)
        return;

    bool change = res > 0;
    res = m_um->usb_find_devices();
    if (!change && res <= 0)
        return;

    // The list of devices has changed, list them now.

    QList<UsbShupitoConnection *> portsToDisown = m_devmap.values();

    usb_bus * bus = m_um->usb_get_busses();
    while (bus)
    {
        struct usb_device * dev = bus->devices;
        for (; dev; dev = dev->next)
        {
            if (!UsbShupitoConnection::isDeviceSupported(dev))
                continue;

            if (m_devmap.contains(dev))
            {
                portsToDisown.removeOne(m_devmap[dev]);
            }
            else
            {
                ConnectionPointer<UsbShupitoConnection> conn(new UsbShupitoConnection(m_um));
                conn->setRemovable(false);
                conn->setUsbDevice(dev);
                conn->setName(conn->product());
                m_devmap[dev] = conn.data();
                connect(conn.data(), SIGNAL(destroying()), this, SLOT(connectionDestroyed()));
                sConMgr2.addConnection(conn.take());
            }
        }
        bus = bus->next;
    }

    for (int i = 0; i < portsToDisown.size(); ++i)
        portsToDisown[i]->releaseAll();
}

void UsbShupitoEnumerator::connectionDestroyed()
{
    UsbShupitoConnection * conn = static_cast<UsbShupitoConnection *>(this->sender());
    m_devmap.remove(conn->usbDevice());
}

ConnectionManager2::ConnectionManager2(QObject * parent)
    : QObject(parent)
{
    m_serialPortEnumerator.reset(new SerialPortEnumerator());
    m_usbShupitoEnumerator.reset(new UsbShupitoEnumerator());

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
    m_usbShupitoEnumerator.reset();

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
