/**********************************************
**    This file is part of Lorris
**    http://tasssadar.github.com/Lorris/
**
**    See README and COPYING
***********************************************/

#include <qextserialenumerator.h>
#include <QStringBuilder>
#include <QDateTime>
#include <QApplication>

#include "connectionmgr2.h"
#include "serialport.h"
#include "tcpsocket.h"
#include "proxytunnel.h"
#include "shupitotunnel.h"
#include "shupitospitunnelconn.h"
#include "udpsocket.h"
#include "../misc/config.h"
#include "../misc/utils.h"

#ifdef HAVE_LIBYB
#include "usbshupito22conn.h"
#include "usbshupito23conn.h"
#include "stm32connection.h"
#endif // HAVE_LIBYB

ConnectionManager2 * psConMgr2 = 0;

SerialPortEnumerator::SerialPortEnumerator()
{
    connect(&m_refreshTimer, SIGNAL(timeout()), this, SLOT(refresh()));
    m_refreshTimer.start(1000);

    QVariant cfg = sConfig.get(CFG_VARIANT_SERIAL_CONNECTIONS);
    if(cfg.type() == QVariant::Hash)
        m_connCfg = cfg.toHash();
}

SerialPortEnumerator::~SerialPortEnumerator()
{
    std::set<SerialPort *> portsToClear;
    portsToClear.swap(m_ownedPorts);

    sConfig.set(CFG_VARIANT_SERIAL_CONNECTIONS, config(portsToClear));

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

        QString physName = ports[i].physName;

#ifdef Q_OS_MAC
        physName = ports[i].portName;
#endif

        QHash<QString, SerialPort *>::const_iterator it = m_portMap.find(physName);
        if (it == m_portMap.end())
        {
            ConnectionPointer<SerialPort> portGuard(new SerialPort());
            portGuard->setName(info.portName, /*isDefault=*/true);
            portGuard->setDeviceName(physName);
            portGuard->setFriendlyName(info.friendName);
            portGuard->setBaudRate(38400);
            portGuard->setDevNameEditable(false);

            QHash<QString, QVariant>::iterator cfgIt = m_connCfg.find(physName);
            if(cfgIt != m_connCfg.end() && (*cfgIt).type() == QVariant::Hash)
                portGuard->applyConfig((*cfgIt).toHash());

            connect(portGuard.data(), SIGNAL(destroyed()), this, SLOT(connectionDestroyed()));

            m_portMap[physName] = portGuard.data();

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

QHash<QString, QVariant> SerialPortEnumerator::config(const std::set<SerialPort *>& ports)
{
    QHash<QString, QVariant> cfg;
    for(std::set<SerialPort *>::const_iterator itr = ports.begin(); itr != ports.end(); ++itr)
        cfg[(*itr)->deviceName()] = (*itr)->config();

    return cfg;
}

#ifdef HAVE_LIBYB

LibybUsbEnumerator::LibybUsbEnumerator(yb::async_runner & runner)
    : m_runner(runner), m_usb_context(m_runner)//, m_devenum(this), m_acm_conns(this), m_shupito23_conns(m_runner)
{
    connect(&m_plugin_channel, SIGNAL(dataReceived()), this, SLOT(pluginEventReceived()));
    m_usb_monitor = m_usb_context.run([this](yb::usb_plugin_event const & p) {
        m_plugin_channel.send(p);
    });

    QVariant cfg = sConfig.get(CFG_VARIANT_USB_ENUMERATOR);
    if(cfg.type() == QVariant::List)
        m_connConfigs = cfg.toList();
}

LibybUsbEnumerator::~LibybUsbEnumerator()
{
    for (auto it = m_usb_acm_devices.begin(); it != m_usb_acm_devices.end(); ++it)
        this->updateConfig(it->second.data());
    sConfig.set(CFG_VARIANT_USB_ENUMERATOR, m_connConfigs);
}

void LibybUsbEnumerator::updateConfig(UsbAcmConnection2 * conn)
{
    for (int i = 0; i < m_connConfigs.size(); ++i)
    {
        QHash<QString, QVariant> settings = m_connConfigs[i].toHash();

        int vid = settings["vid"].toInt();
        int pid = settings["pid"].toInt();
        QString sn = settings["serial_number"].toString();
        QString intf = settings["intf_name"].toString();

        if (conn->vid() == vid && conn->pid() == pid && conn->serialNumber() == sn && conn->intfName() == intf)
        {
            m_connConfigs[i] = conn->config();
            return;
        }
    }

    m_connConfigs.push_back(conn->config());
}

void LibybUsbEnumerator::applyConfig(UsbAcmConnection2 * conn)
{
    for (int i = 0; i < m_connConfigs.size(); ++i)
    {
        QHash<QString, QVariant> settings = m_connConfigs[i].toHash();

        int vid = settings["vid"].toInt();
        int pid = settings["pid"].toInt();
        QString sn = settings["serial_number"].toString();
        QString intf = settings["intf_name"].toString();

        if (conn->vid() == vid && conn->pid() == pid && conn->serialNumber() == sn && conn->intfName() == intf)
        {
            conn->applyConfig(settings);
            break;
        }
    }
}

void LibybUsbEnumerator::registerUserOwnedConn(UsbAcmConnection2 * conn)
{
    connect(conn, SIGNAL(destroying()), this, SLOT(acmConnDestroying()));
    m_user_owned_acm_conns.insert(conn);
}

void LibybUsbEnumerator::registerUserOwnedConn(STM32Connection * conn)
{
    connect(conn, SIGNAL(destroying()), this, SLOT(stm32ConnDestroying()));
    m_user_owned_stm32_conns.insert(conn);
}

void LibybUsbEnumerator::acmConnDestroying()
{
    UsbAcmConnection2 * conn = static_cast<UsbAcmConnection2 *>(this->sender());
    m_user_owned_acm_conns.erase(conn);
}

void LibybUsbEnumerator::stm32ConnDestroying()
{
    STM32Connection *conn = static_cast<STM32Connection *>(this->sender());
    m_user_owned_stm32_conns.erase(conn);
}

yb::usb_device_interface LibybUsbEnumerator::lookupUsbAcmConn(int vid, int pid, QString const & serialNumber, QString const & intfName)
{
    usb_interface_standby info;
    info.dev.vidpid = (vid << 16) | pid;
    info.dev.sn.assign(serialNumber.toUtf8().data());
    info.intfname = intfName;

    std::map<usb_interface_standby, yb::usb_device_interface>::iterator it = m_usb_acm_devices_by_info.find(info);
    if (it == m_usb_acm_devices_by_info.end())
        return yb::usb_device_interface();
    return it->second;
}

void LibybUsbEnumerator::pluginEventReceived()
{
    std::vector<yb::usb_plugin_event> events;
    m_plugin_channel.receive(events);

    for (size_t i = 0; i < events.size(); ++i)
    {
        yb::usb_plugin_event const & ev = events[i];

        if (!ev.dev.empty())
        {
            if (!GenericUsbConnection::isFlipDevice(ev.dev))
                continue;

            usb_device_standby st;
            st.sn = ev.dev.serial_number();
            st.vidpid = ev.dev.vidpid();

            switch (ev.action)
            {
            case yb::usb_plugin_event::a_add:
                {
                    ConnectionPointer<GenericUsbConnection> conn = m_standby_usb_devices.extract(st);
                    if (!conn)
                    {
                        conn.reset(new GenericUsbConnection(m_runner, ev.dev));
                        sConMgr2.addConnection(conn.data());
                    }

                    conn->setRemovable(false);
                    conn->setDevice(ev.dev);
                    m_usb_devices.insert(std::make_pair(ev.dev, conn));
                }
                break;
            case yb::usb_plugin_event::a_remove:
                {
                    std::map<yb::usb_device, ConnectionPointer<GenericUsbConnection> >::iterator it = m_usb_devices.find(ev.dev);
                    it->second->clearDevice();
                    it->second->setRemovable(true);
                    m_standby_usb_devices.add(st, it->second.data());
                    m_usb_devices.erase(it);
                }
                break;
            }
        }
        else
        {
            yb::usb_interface_descriptor const & intf = ev.intf.descriptor().altsettings[0];

            usb_interface_standby st;
            st.dev.sn = ev.intf.device().serial_number();
            st.dev.vidpid = ev.intf.device().vidpid();
            st.intfname = QString::fromUtf8(ev.intf.name().c_str());
            if (st.intfname.isEmpty())
                st.intfname = QString("#%1").arg(ev.intf.interface_index());

            if (GenericUsbConnection::isShupito23Device(ev.intf.device()) && intf.bInterfaceClass == 0xff
                && intf.in_descriptor_count() > 0 && intf.out_descriptor_count() == 1)
            {
                switch (ev.action)
                {
                case yb::usb_plugin_event::a_add:
                    {
                        ConnectionPointer<UsbShupito23Connection> conn = m_standby_shupito23_devices.extract(st);
                        if (!conn)
                        {
                            conn.reset(new UsbShupito23Connection(m_runner));
                            conn->setName(GenericUsbConnection::formatDeviceName(ev.intf.device()), /*isDefault=*/true);
                            sConMgr2.addConnection(conn.data());
                        }
                        conn->setup(ev.intf);
                        conn->setRemovable(false);
                        m_shupito23_devices.insert(std::make_pair(ev.intf, conn));
                    }
                    break;
                case yb::usb_plugin_event::a_remove:
                    {
                        std::map<yb::usb_device_interface, ConnectionPointer<UsbShupito23Connection> >::iterator it = m_shupito23_devices.find(ev.intf);
                        it->second->clear();
                        it->second->setRemovable(true);
                        m_standby_shupito23_devices.add(st, it->second.data());
                        m_shupito23_devices.erase(it);
                    }
                    break;
                }
            }
            else if (intf.bInterfaceClass == 0xa && intf.bInterfaceSubClass == 0
                && !intf.endpoints.empty())
            {
                Q_ASSERT(!st.intfname.isEmpty());

                switch (ev.action)
                {
                case yb::usb_plugin_event::a_add:
                    m_usb_acm_devices_by_info.insert(std::make_pair(st, ev.intf));
                    for (std::set<UsbAcmConnection2 *>::const_iterator it = m_user_owned_acm_conns.begin(); it != m_user_owned_acm_conns.end(); ++it)
                        (*it)->notifyIntfPlugin(ev.intf);
                    break;
                case yb::usb_plugin_event::a_remove:
                    m_usb_acm_devices_by_info.erase(st);
                    for (std::set<UsbAcmConnection2 *>::const_iterator it = m_user_owned_acm_conns.begin(); it != m_user_owned_acm_conns.end(); ++it)
                        (*it)->notifyIntfUnplug(ev.intf);
                    break;
                }

                if (GenericUsbConnection::isShupito20Device(ev.intf.device()))
                {
                    switch (ev.action)
                    {
                    case yb::usb_plugin_event::a_add:
                        {
                            ConnectionPointer<UsbShupito22Connection> conn = m_standby_shupito22_devices.extract(st);
                            if (!conn)
                            {
                                conn.reset(new UsbShupito22Connection(m_runner));
                                conn->setName(GenericUsbConnection::formatDeviceName(ev.intf.device()), /*isDefault=*/true);
                                sConMgr2.addConnection(conn.data());
                            }
                            conn->setup(ev.intf);
                            conn->setRemovable(false);
                            m_shupito22_devices.insert(std::make_pair(ev.intf, conn));
                        }
                        break;
                    case yb::usb_plugin_event::a_remove:
                        {
                            std::map<yb::usb_device_interface, ConnectionPointer<UsbShupito22Connection> >::iterator it = m_shupito22_devices.find(ev.intf);
                            it->second->clear();
                            it->second->setRemovable(true);
                            m_standby_shupito22_devices.add(st, it->second.data());
                            m_shupito22_devices.erase(it);
                        }
                        break;
                    }
                }
                else if (st.intfname[0] != '.' && st.intfname[0] != '#')
                {
                    switch (ev.action)
                    {
                    case yb::usb_plugin_event::a_add:
                        {
                            ConnectionPointer<UsbAcmConnection2> conn = m_standby_usb_acm_devices.extract(st);
                            if (!conn)
                            {
                                conn.reset(new UsbAcmConnection2(m_runner));
                                conn->setName(QString("%1 @ %2").arg(st.intfname).arg(GenericUsbConnection::formatDeviceName(ev.intf.device())), /*isDefault=*/true);
                                sConMgr2.addConnection(conn.data());
                            }
                            conn->setEnumeratedIntf(ev.intf);
                            conn->setRemovable(false);
                            this->applyConfig(conn.data());
                            m_usb_acm_devices.insert(std::make_pair(ev.intf, conn));
                        }
                        break;
                    case yb::usb_plugin_event::a_remove:
                        {
                            std::map<yb::usb_device_interface, ConnectionPointer<UsbAcmConnection2> >::iterator it = m_usb_acm_devices.find(ev.intf);
                            this->updateConfig(it->second.data());
                            it->second->clear();
                            it->second->setRemovable(true);
                            m_standby_usb_acm_devices.add(st, it->second.data());
                            m_usb_acm_devices.erase(it);
                        }
                        break;
                    }
                }
            }
            else if(GenericUsbConnection::isSTLink32LDevice(ev.intf.device()) && intf.bInterfaceClass == 0xff && st.intfname == "ST Link")
            {
                switch (ev.action)
                {
                case yb::usb_plugin_event::a_add:
                    {
                        for (std::set<STM32Connection *>::const_iterator it = m_user_owned_stm32_conns.begin(); it != m_user_owned_stm32_conns.end(); ++it)
                            (*it)->notifyIntfPlugin(ev.intf);

                        ConnectionPointer<STM32Connection> conn = m_standby_stm32_devices.extract(st);
                        if (!conn)
                        {
                            conn.reset(new STM32Connection(m_runner));
                            sConMgr2.addConnection(conn.data());
                        }
                        conn->setEnumeratedIntf(ev.intf);
                        conn->setRemovable(false);
                        m_stm32_devices.insert(std::make_pair(ev.intf, conn));
                    }
                    break;
                case yb::usb_plugin_event::a_remove:
                    {
                        for (std::set<STM32Connection *>::const_iterator it = m_user_owned_stm32_conns.begin(); it != m_user_owned_stm32_conns.end(); ++it)
                            (*it)->notifyIntfUnplug(ev.intf);

                        std::map<yb::usb_device_interface, ConnectionPointer<STM32Connection> >::iterator it = m_stm32_devices.find(ev.intf);
                        it->second->clear();
                        it->second->setRemovable(true);
                        m_standby_stm32_devices.add(st, it->second.data());
                        m_stm32_devices.erase(it);
                    }
                    break;
                }
            }
        }
    }
}

#endif // HAVE_LIBYB

ConnectionManager2::ConnectionManager2(QObject * parent)
    : QObject(parent), m_lastCompanionId(0)
{
    Q_ASSERT(psConMgr2 == 0);
    psConMgr2 = this;

    m_serialPortEnumerator.reset(new SerialPortEnumerator());
#ifdef HAVE_LIBYB
    m_libybUsbEnumerator.reset(new LibybUsbEnumerator(m_yb_runner));
#endif // HAVE_LIBYB

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
#ifdef HAVE_LIBYB
    m_libybUsbEnumerator.reset();
#endif // HAVE_LIBYB

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
            "serial_port",     // CONNECTION_SERIAL_PORT
            "shupito_tunnel",  // CONNECTION_SHUPITO_TUNNEL
            "shupito",         // CONNECTION_PORT_SHUPITO
            "tcp_client",      // CONNECTION_TCP_SOCKET
            "usb_shupito",     // CONNECTION_USB_SHUPITO
            "usb_acm",         // CONNECTION_USB_ACM
            "proxy_tunnel",    // CONNECTION_PROXY_TUNNEL
            "",                // CONNECTION_FLIP
            "",                // CONNECTION_LIBYB_USB
            "usb_yb_acm",      // CONNECTION_USB_ACM2
            "",                // CONNECTION_SHUPITO23
            "stm32",           // CONNECTION_STM32
            "",                // CONNECTION_SHUPITO_SPI_TUNNEL
            "udp_socket",      // CONNECTION_UDP_SOCKET
        };

        Q_ASSERT(conn->getType() < sizeof_array(connTypes));

        char const * connType = connTypes[conn->getType()];
        if (connType[0] == 0)
            continue;

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
        else if (type == "udp_socket")
            conn.reset(new UdpSocket());
#ifdef HAVE_LIBYB
        else if (type == "usb_yb_acm")
            conn.reset(new UsbAcmConnection2(m_yb_runner));
#endif

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

UdpSocket * ConnectionManager2::createUdpSocket()
{
    ConnectionPointer<UdpSocket> conn(new UdpSocket());
    this->addUserOwnedConn(conn.data());
    return conn.take();
}

#ifdef HAVE_LIBYB
UsbAcmConnection2 * ConnectionManager2::createUsbAcmConn()
{
    ConnectionPointer<UsbAcmConnection2> conn(new UsbAcmConnection2(m_yb_runner));
    this->addUserOwnedConn(conn.data());
    return conn.take();
}

yb::usb_device_interface ConnectionManager2::lookupUsbAcmConn(int vid, int pid, QString const & serialNumber, QString const & intfName)
{
    return m_libybUsbEnumerator->lookupUsbAcmConn(vid, pid, serialNumber, intfName);
}

STM32Connection *ConnectionManager2::createSTM32Conn()
{
    ConnectionPointer<STM32Connection> conn(new STM32Connection(m_yb_runner));
    this->addUserOwnedConn(conn.data());
    return conn.take();
}

#endif

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

#ifdef HAVE_LIBYB
    if (UsbAcmConnection2 * uc = dynamic_cast<UsbAcmConnection2 *>(conn))
        m_libybUsbEnumerator->registerUserOwnedConn(uc);
    if (STM32Connection * sc = dynamic_cast<STM32Connection *>(conn))
        m_libybUsbEnumerator->registerUserOwnedConn(sc);
#endif
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
    res->setName("Shupito at " % parentConn->name(), /*isDefault=*/true);
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

ConnectionPointer<Connection> ConnectionManager2::getConnWithConfig(quint8 type, const QHash<QString, QVariant> &cfg)
{
    this->refresh();

#ifdef HAVE_LIBYB
    // We want to receive first USB enumeration events,
    // so that connections created here can be opened
    QApplication::processEvents();
#endif

    Connection *enumCon = NULL;
    for(int i = 0; i < m_conns.size(); ++i)
    {
        if(m_conns[i]->getType() != type)
            continue;

        switch(type)
        {
            case CONNECTION_SERIAL_PORT:
            {
                SerialPort *sp = (SerialPort*)m_conns[i];
                if(sp->deviceName() == cfg["device_name"])
                {
                    if(!sp->removable())
                        enumCon = sp;
                    else if(sp->baudRate() == cfg["baud_rate"])
                        return ConnectionPointer<Connection>::fromPtr(sp);
                }
                break;
            }
            case CONNECTION_TCP_SOCKET:
            {
                TcpSocket *socket = (TcpSocket*)m_conns[i];
                if(socket->host() == cfg["host"] && socket->port() == cfg["port"])
                    return ConnectionPointer<Connection>::fromPtr(socket);
                break;
            }
            case CONNECTION_UDP_SOCKET:
            {
                UdpSocket *socket = (UdpSocket*)m_conns[i];
                if (socket->name() == cfg["name"] &&
                    socket->host() == cfg["host"] && socket->port() == cfg["port"])
                    return ConnectionPointer<Connection>::fromPtr(socket);
                break;
            }
            case CONNECTION_PROXY_TUNNEL:
            {
                ProxyTunnel *tunnel = (ProxyTunnel*)m_conns[i];
                if(tunnel->name() == cfg["name"])
                    return ConnectionPointer<Connection>::fromPtr(tunnel);
                break;
            }
            case CONNECTION_SHUPITO_TUNNEL:
            {
                QHash<QString, QVariant> c = cfg.value("companions").toHash();
                QHash<QString, QVariant>::iterator itr = c.find(ShupitoTunnel::getCompanionName());
                if(itr == c.end())
                    return ConnectionPointer<Connection>();

                qint64 id = itr.value().toLongLong();
                if(id != 0 && id == m_conns[i]->getCompanionId(ShupitoTunnel::getCompanionName()))
                    return ConnectionPointer<Connection>::fromPtr((ShupitoTunnel*)m_conns[i]);
                break;
            }
            case CONNECTION_SHUPITO_SPI_TUNNEL:
            {
                QHash<QString, QVariant> c = cfg.value("companions").toHash();
                QHash<QString, QVariant>::iterator itr = c.find(ShupitoSpiTunnelConn::getCompanionName());
                if(itr == c.end())
                    return ConnectionPointer<Connection>();

                qint64 id = itr.value().toLongLong();
                if(id != 0 && id == m_conns[i]->getCompanionId(ShupitoSpiTunnelConn::getCompanionName()))
                    return ConnectionPointer<Connection>::fromPtr((ShupitoSpiTunnelConn*)m_conns[i]);
                break;
            }
#ifdef HAVE_LIBYB
            case CONNECTION_USB_ACM2:
            {
                UsbAcmConnection2 *usb = (UsbAcmConnection2*)m_conns[i];
                if (usb->vid() == cfg.value("vid", 0).toInt() &&
                    usb->pid() == cfg.value("pid", 0).toInt() &&
                    usb->serialNumber() == cfg.value("serial_number").toString() &&
                    usb->intfName() == cfg.value("intf_name").toString() &&
                    usb->baudRate() == cfg.value("baud_rate", 115200).toInt() &&
                    usb->stopBits() == cfg.value("stop_bits", 0).toInt() &&
                    usb->parity() == (UsbAcmConnection2::parity_t)cfg.value("parity", 0).toInt() &&
                    usb->dataBits() == cfg.value("data_bits", 8).toInt())
                {
                    usb->applyConfig(cfg);
                    return ConnectionPointer<Connection>::fromPtr(usb);
                }
                break;
            }
            case CONNECTION_SHUPITO23:
            {
                UsbShupito23Connection *usb = (UsbShupito23Connection*)m_conns[i];
                if (usb->vid() == cfg.value("vid", 0).toInt() &&
                    usb->pid() == cfg.value("pid", 0).toInt() &&
                    usb->serialNumber() == cfg.value("serial_number").toString() &&
                    usb->intfName() == cfg.value("intf_name").toString())
                {
                    usb->applyConfig(cfg);
                    return ConnectionPointer<Connection>::fromPtr(usb);
                }
                break;
            }
            case CONNECTION_STM32:
            {
                STM32Connection *stm32 = (STM32Connection*)m_conns[i];
                if (stm32->vid() == cfg.value("vid", 0).toInt() &&
                    stm32->pid() == cfg.value("pid", 0).toInt() &&
                    stm32->serialNumber() == cfg.value("serial_number").toString() &&
                    stm32->intfName() == cfg.value("intf_name").toString())
                {
                    return ConnectionPointer<Connection>::fromPtr(stm32);
                }
                break;
            }
#endif
            default:
                return ConnectionPointer<Connection>();
        }
    }

    if(enumCon)
    {
        enumCon->applyConfig(cfg);
        return ConnectionPointer<Connection>::fromPtr(enumCon);
    }

    if(type == CONNECTION_SHUPITO_TUNNEL)
    {
        QHash<QString, QVariant> c = cfg.value("companions").toHash();
        QHash<QString, QVariant>::iterator itr = c.find(ShupitoTunnel::getCompanionName());
        if(itr == c.end())
            return ConnectionPointer<Connection>();

        qint64 id = itr.value().toLongLong();
        if(id == 0)
            return ConnectionPointer<Connection>();

        ConnectionPointer<Connection> tunnel(new ShupitoTunnel());
        tunnel->applyConfig(cfg);
        tunnel->setRemovable(false);
        this->addConnection(tunnel.data());
        return tunnel;
    }
    else if(type == CONNECTION_SHUPITO_SPI_TUNNEL)
    {
        QHash<QString, QVariant> c = cfg.value("companions").toHash();
        QHash<QString, QVariant>::iterator itr = c.find(ShupitoSpiTunnelConn::getCompanionName());
        if(itr == c.end())
            return ConnectionPointer<Connection>();

        qint64 id = itr.value().toLongLong();
        if(id == 0)
            return ConnectionPointer<Connection>();

        ConnectionPointer<Connection> tunnel(new ShupitoSpiTunnelConn());
        tunnel->applyConfig(cfg);
        tunnel->setRemovable(false);
        this->addConnection(tunnel.data());
        return tunnel;
    }
#ifdef HAVE_LIBYB
    else if(type == CONNECTION_USB_ACM2)
    {
        ConnectionPointer<Connection> usb(createUsbAcmConn());
        usb->applyConfig(cfg);
        return usb;
    }
    else if(type == CONNECTION_STM32)
    {
        ConnectionPointer<Connection> stm32(createSTM32Conn());
        stm32->applyConfig(cfg);
        return stm32;
    }
#endif

    return ConnectionPointer<Connection>();
}

void ConnectionManager2::connectAll()
{
    for(int i = 0; i < m_conns.size(); ++i)
        if(m_conns[i]->isUsedByTab())
            m_conns[i]->OpenConcurrent();
}

void ConnectionManager2::disconnectAll()
{
    for(int i = 0; i < m_conns.size(); ++i)
        m_conns[i]->Close();
}

qint64 ConnectionManager2::generateCompanionId()
{
    qint64 id = QDateTime::currentMSecsSinceEpoch();
    while(id <= m_lastCompanionId)
        ++id;
    m_lastCompanionId = id;
    return id;
}

Connection *ConnectionManager2::getCompanionConnection(Connection *toConn, const QString& name)
{
    qint64 id = toConn->getCompanionId(name);
    if(!toConn || id == 0)
        return NULL;

    for(int i = 0; i < m_conns.size(); ++i)
    {
        Connection *c = m_conns[i];
        if(c != toConn && c->getCompanionId(name) == id)
            return c;
    }
    return NULL;
}
