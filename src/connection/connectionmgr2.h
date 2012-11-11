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
    std::set<SerialPort *> m_ownedPorts;
    QHash<QString, SerialPort *> m_portMap;

    QTimer m_refreshTimer;
};

#ifdef HAVE_LIBUSBY

#include "usbshupitoconn.h"
#include <libusby.hpp>

class UsbAcmConnection;

class UsbShupitoEnumerator : public QObject
{
    Q_OBJECT

public:
    UsbShupitoEnumerator();
    ~UsbShupitoEnumerator();

    QVariant config() const;
    bool applyConfig(QVariant const & config);

public slots:
    void refresh();

private slots:
    void shupitoConnectionDestroyed();

private:
    libusby::context m_usb_ctx;
    QScopedPointer<QThread> m_eventDispatcher;

    std::map<libusby::device, UsbShupitoConnection *> m_seen_devices;
    QHash<QString, UsbShupitoConnection *> m_stand_by_conns;
    QHash<UsbShupitoConnection *, QString> m_unique_ids;

    QTimer m_refreshTimer;
};

#endif // HAVE_LIBUSBY

#ifdef HAVE_LIBYB

#include <libyb/usb/usb_context.hpp>
#include <libyb/usb/usb_device.hpp>
#include "flipconnection.h"

class LibybUsbEnumerator : public QObject
{
    Q_OBJECT

public:
    LibybUsbEnumerator();
    ~LibybUsbEnumerator();

public slots:
    void refresh();

private slots:
    void connectionDestroyed();

private:
    yb::usb_context m_usb_context;
    std::map<yb::usb_device, FlipConnection *> m_seen_devices;
    QSet<FlipConnection *> m_stand_by_conns;
    QTimer m_refreshTimer;
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
    void refresh();

    SerialPort * createSerialPort();
    TcpSocket * createTcpSocket();
    ConnectionPointer<ShupitoConnection> createAutoShupito(PortConnection * parentConn);

    QVariant config() const;
    bool applyConfig(QVariant const & config);

    ConnectionPointer<PortConnection> getConnWithConfig(quint8 type, const QHash<QString, QVariant>& cfg);


Q_SIGNALS:
    void connAdded(Connection * conn);
    void connRemoved(Connection * conn);

private slots:
    void connectionDestroyed();
    void autoShupitoDestroyed();

private:
    void addUserOwnedConn(Connection * conn);
    void clearUserOwnedConns();

    QList<Connection *> m_conns;

    QSet<Connection *> m_userOwnedConns;
    QScopedPointer<SerialPortEnumerator> m_serialPortEnumerator;
#ifdef HAVE_LIBUSBY
    QScopedPointer<UsbShupitoEnumerator> m_usbShupitoEnumerator;
#endif // HAVE_LIBUSBY
#ifdef HAVE_LIBYB
    QScopedPointer<LibybUsbEnumerator> m_libybUsbEnumerator;
#endif // HAVE_LIBYB
    QHash<PortConnection *, ShupitoConnection *> m_autoShupitos;
    QHash<QObject *, PortConnection *> m_autoShupitosRev;
};

extern ConnectionManager2 * psConMgr2;
#define sConMgr2 (*psConMgr2)

#endif // CONNECTIONMGR2_H
