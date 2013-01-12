/**********************************************
**    This file is part of Lorris
**    http://tasssadar.github.com/Lorris/
**
**    See README and COPYING
***********************************************/

#include <qextserialenumerator.h>
#include <QStringBuilder>

#include "connectionmgr2.h"
#include "serialport.h"
#include "tcpsocket.h"
#include "proxytunnel.h"
#include "../misc/config.h"
#include "../misc/utils.h"

#ifdef HAVE_LIBUSBY
#include "usbshupitoconn.h"
#endif // HAVE_LIBUSBY

#ifdef HAVE_LIBYB
#include "usbshupito23conn.h"
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
    {
        if (it->second)
            it->second->releaseAll();
    }

    // FIXME: replace with .swap() when Qt 4.8 will become broader
    QHash<QString, UsbShupitoConnection *> stand_by_conns = m_stand_by_conns;
    m_stand_by_conns.clear();

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

#ifdef HAVE_LIBYB

LibybUsbEnumerator::LibybUsbEnumerator(yb::async_runner & runner)
    : m_runner(runner), m_usb_context(m_runner)//, m_devenum(this), m_acm_conns(this), m_shupito23_conns(m_runner)
{
    connect(&m_plugin_channel, SIGNAL(dataReceived()), this, SLOT(pluginEventReceived()));
    m_usb_monitor = m_usb_context.run([this](yb::usb_plugin_event const & p) {
        m_plugin_channel.send(p);
    });
}

LibybUsbEnumerator::~LibybUsbEnumerator()
{
}

void LibybUsbEnumerator::registerUserOwnedConn(UsbAcmConnection2 * conn)
{
    connect(conn, SIGNAL(destroying()), this, SLOT(acmConnDestroying()));
    m_user_owned_acm_conns.insert(conn);
}

void LibybUsbEnumerator::acmConnDestroying()
{
    UsbAcmConnection2 * conn = static_cast<UsbAcmConnection2 *>(this->sender());
    m_user_owned_acm_conns.erase(conn);
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
                            conn->setName(GenericUsbConnection::formatDeviceName(ev.intf.device()));
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

                if (st.intfname[0] == '#' || st.intfname[0] == '.')
                    continue;

                switch (ev.action)
                {
                case yb::usb_plugin_event::a_add:
                    {
                        ConnectionPointer<UsbAcmConnection2> conn = m_standby_usb_acm_devices.extract(st);
                        if (!conn)
                        {
                            conn.reset(new UsbAcmConnection2(m_runner));
                            conn->setName(QString("%1 @ %2").arg(st.intfname).arg(GenericUsbConnection::formatDeviceName(ev.intf.device())));
                            sConMgr2.addConnection(conn.data());
                        }
                        conn->setEnumeratedIntf(ev.intf);
                        conn->setRemovable(false);
                        m_usb_acm_devices.insert(std::make_pair(ev.intf, conn));
                    }
                    break;
                case yb::usb_plugin_event::a_remove:
                    {
                        std::map<yb::usb_device_interface, ConnectionPointer<UsbAcmConnection2> >::iterator it = m_usb_acm_devices.find(ev.intf);
                        it->second->clear();
                        it->second->setRemovable(true);
                        m_standby_usb_acm_devices.add(st, it->second.data());
                        m_usb_acm_devices.erase(it);
                    }
                    break;
                }
            }
        }
    }
}

#endif // HAVE_LIBYB

ConnectionManager2::ConnectionManager2(QObject * parent)
    : QObject(parent)
{
    Q_ASSERT(psConMgr2 == 0);
    psConMgr2 = this;

    m_serialPortEnumerator.reset(new SerialPortEnumerator());
#ifdef HAVE_LIBUSBY
    m_usbShupitoEnumerator.reset(new UsbShupitoEnumerator());
#endif // HAVE_LIBUSBY
#ifdef HAVE_LIBYB
    m_libybUsbEnumerator.reset(new LibybUsbEnumerator(m_yb_runner));
#endif // HAVE_LIBYB

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
#ifdef HAVE_LIBYB
    m_libybUsbEnumerator.reset();
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

ConnectionPointer<PortConnection> ConnectionManager2::getConnWithConfig(quint8 type, const QHash<QString, QVariant> &cfg)
{
    this->refresh();

    PortConnection *enumCon = NULL;
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
                        return ConnectionPointer<PortConnection>::fromPtr(sp);
                }
                break;
            }
            case CONNECTION_TCP_SOCKET:
            {
                TcpSocket *socket = (TcpSocket*)m_conns[i];
                if(socket->host() == cfg["host"] && socket->port() == cfg["port"])
                    return ConnectionPointer<PortConnection>::fromPtr(socket);
                break;
            }
            case CONNECTION_PROXY_TUNNEL:
            {
                ProxyTunnel *tunnel = (ProxyTunnel*)m_conns[i];
                if(tunnel->name() == cfg["name"])
                    return ConnectionPointer<PortConnection>::fromPtr(tunnel);
                break;
            }
            default:
                return ConnectionPointer<PortConnection>();
        }
    }

    if(enumCon)
    {
        enumCon->applyConfig(cfg);
        return ConnectionPointer<PortConnection>::fromPtr(enumCon);
    }

    return ConnectionPointer<PortConnection>();
}
