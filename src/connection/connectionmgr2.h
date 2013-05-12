/**********************************************
**    This file is part of Lorris
**    http://tasssadar.github.com/Lorris/
**
**    See README and COPYING
***********************************************/

#ifndef CONNECTIONMGR2_H
#define CONNECTIONMGR2_H

#include "../connection/connection.h"
#include "../connection/shupitoconn.h"
#include "../connection/usbshupito23conn.h"
#include <QString>
#include <QTimer>
#include <QHash>
#include <QSet>
#include <set>
#include <map>

class SerialPort;
class TcpSocket;

class SerialPortEnumerator : public QObject
{
    Q_OBJECT

public:
    SerialPortEnumerator();
    ~SerialPortEnumerator();

public slots:
    void refresh();

private slots:
    void connectionDestroyed();

private:
    QHash<QString, QVariant> config(const std::set<SerialPort *>& ports);

    std::set<SerialPort *> m_ownedPorts;
    QHash<QString, SerialPort *> m_portMap;

    QHash<QString, QVariant> m_connCfg;

    QTimer m_refreshTimer;
};

#ifdef HAVE_LIBYB

#include <libyb/usb/usb_context.hpp>
#include <libyb/usb/usb_device.hpp>
#include <libyb/utils/tuple_less.hpp>
#include "../connection/usbshupito22conn.h"
#include "genericusbconn.h"

class StandbyDeviceListBase
    : public QObject
{
    Q_OBJECT

public slots:
    virtual void connectionDestroyed() = 0;
};

template <typename Conn, typename Info>
class StandbyDeviceList
    : private StandbyDeviceListBase
{
public:
    void add(Info const & info, Conn * conn)
    {
        typename std::map<Info, Conn *>::iterator it = m_conns.insert(std::make_pair(info, conn)).first;
        m_inverse_map.insert(std::make_pair(conn, it));
        connect(conn, SIGNAL(destroying()), this, SLOT(connectionDestroyed()));
    }

    ConnectionPointer<Conn> extract(Info const & info)
    {
        typename std::map<Info, Conn *>::iterator it = m_conns.find(info);
        if (it == m_conns.end())
            return ConnectionPointer<Conn>();

        Conn * res = it->second;
        m_conns.erase(it);
        m_inverse_map.erase(res);
        disconnect(res, 0, this, 0);
        return ConnectionPointer<Conn>::fromPtr(res);
    }

protected:
    void connectionDestroyed()
    {
        Conn * conn = static_cast<Conn *>(this->sender());
        typename std::map<Conn *, typename std::map<Info, Conn *>::iterator>::iterator it = m_inverse_map.find(conn);
        Q_ASSERT(it != m_inverse_map.end());
        m_conns.erase(it->second);
        m_inverse_map.erase(it);
    }

private:
    std::map<Info, Conn *> m_conns;
    std::map<Conn *, typename std::map<Info, Conn *>::iterator> m_inverse_map;
};

class LibybUsbEnumerator : public QObject
{
    Q_OBJECT

public:
    explicit LibybUsbEnumerator(yb::async_runner & runner);
    ~LibybUsbEnumerator();

    void registerUserOwnedConn(UsbAcmConnection2 * conn);
    yb::usb_device_interface lookupUsbAcmConn(int vid, int pid, QString const & serialNumber, QString const & intfName);

private slots:
    void pluginEventReceived();
    void acmConnDestroying();

private:
    yb::async_runner & m_runner;
    yb::usb_context m_usb_context;
    ThreadChannel<yb::usb_plugin_event> m_plugin_channel;
    yb::async_future<void> m_usb_monitor;

    struct usb_device_standby
    {
        uint32_t vidpid;
        std::string sn;

        friend bool operator<(usb_device_standby const & lhs, usb_device_standby const & rhs)
        {
            return lhs.vidpid < rhs.vidpid
                || (lhs.vidpid == rhs.vidpid && lhs.sn < rhs.sn);
        }
    };

    std::map<yb::usb_device, ConnectionPointer<GenericUsbConnection> > m_usb_devices;
    StandbyDeviceList<GenericUsbConnection, usb_device_standby> m_standby_usb_devices;

    struct usb_interface_standby
    {
        usb_device_standby dev;
        QString intfname;

        friend bool operator<(usb_interface_standby const & lhs, usb_interface_standby const & rhs)
        {
            return lhs.intfname < rhs.intfname
                || (lhs.intfname == rhs.intfname && lhs.dev < rhs.dev);
        }
    };

    std::map<yb::usb_device_interface, ConnectionPointer<UsbShupito22Connection> > m_shupito22_devices;
    StandbyDeviceList<UsbShupito22Connection, usb_interface_standby> m_standby_shupito22_devices;

    std::map<yb::usb_device_interface, ConnectionPointer<UsbShupito23Connection> > m_shupito23_devices;
    StandbyDeviceList<UsbShupito23Connection, usb_interface_standby> m_standby_shupito23_devices;

    std::map<yb::usb_device_interface, ConnectionPointer<UsbAcmConnection2> > m_usb_acm_devices;
    StandbyDeviceList<UsbAcmConnection2, usb_interface_standby> m_standby_usb_acm_devices;
    std::map<usb_interface_standby, yb::usb_device_interface> m_usb_acm_devices_by_info;
    std::set<UsbAcmConnection2 *> m_user_owned_acm_conns;

    QList<QVariant> m_connConfigs;
    void updateConfig(UsbAcmConnection2 * conn);
    void applyConfig(UsbAcmConnection2 * conn);
};

#endif // HAVE_LIBYB

class ConnectionManager2 : public QObject
{
    Q_OBJECT

public:
    ConnectionManager2(QObject * parent = 0);
    ~ConnectionManager2();

    QList<Connection *> const & connections() const { return m_conns; }

    void addConnection(Connection * conn);
    void addUserOwnedConn(Connection * conn);
    void refresh();

    SerialPort * createSerialPort();
    TcpSocket * createTcpSocket();
#ifdef HAVE_LIBYB
    UsbAcmConnection2 * createUsbAcmConn();
    yb::usb_device_interface lookupUsbAcmConn(int vid, int pid, QString const & serialNumber, QString const & intfName);
#endif

    ConnectionPointer<ShupitoConnection> createAutoShupito(PortConnection * parentConn);

    QVariant config() const;
    bool applyConfig(QVariant const & config);

    ConnectionPointer<Connection> getConnWithConfig(quint8 type, const QHash<QString, QVariant>& cfg);

    qint64 generateCompanionId();
    Connection *getCompanionConnection(Connection *toConn);

Q_SIGNALS:
    void connAdded(Connection * conn);
    void connRemoved(Connection * conn);

public slots:
    void connectAll();
    void disconnectAll();

private slots:
    void connectionDestroyed();
    void autoShupitoDestroyed();

private:
    void clearUserOwnedConns();

    QList<Connection *> m_conns;

    QSet<Connection *> m_userOwnedConns;
    QScopedPointer<SerialPortEnumerator> m_serialPortEnumerator;
#ifdef HAVE_LIBYB
    QScopedPointer<LibybUsbEnumerator> m_libybUsbEnumerator;
    yb::async_runner m_yb_runner;
#endif // HAVE_LIBYB
    QHash<PortConnection *, ShupitoConnection *> m_autoShupitos;
    QHash<QObject *, PortConnection *> m_autoShupitosRev;
    qint64 m_lastCompanionId;
};

extern ConnectionManager2 * psConMgr2;
#define sConMgr2 (*psConMgr2)

#endif // CONNECTIONMGR2_H
