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
#include "genericusbconn.h"
#include "deviceenumerator.h"

class LibybUsbEnumerator : public QObject
{
    Q_OBJECT

public:
	explicit LibybUsbEnumerator(yb::async_runner & runner);
    ~LibybUsbEnumerator();

public slots:
    void refresh();

private:
	yb::async_runner & m_runner;
    yb::usb_context m_usb_context;
    QTimer m_refreshTimer;

    struct standby_info_type
    {
        uint32_t vidpid;
        QString sn;
    };

    struct dev_id
    {
        yb::usb_device dev;
        QString sn;

        friend bool operator<(dev_id const & lhs, dev_id const & rhs)
        {
            return lhs.dev < rhs.dev;
        }
    };

    class GenericUsbEnumerator
        : public DeviceEnumerator<GenericUsbConnection, dev_id, standby_info_type>
    {
    public:
        GenericUsbEnumerator(LibybUsbEnumerator * self)
            : m_self(self)
        {
        }

        virtual GenericUsbConnection * create(dev_id const & id)
        {
            ConnectionPointer<GenericUsbConnection> conn(new GenericUsbConnection(m_self->m_runner, id.dev));
            conn->setPersistent(!conn->serialNumber().isEmpty());
            return conn.take();
        }

        virtual void resurrect(id_type const & id, connection_type * conn)
        {
            conn->setDevice(id.dev);
            conn->setRemovable(false);
        }

        virtual void clear(connection_type * conn)
        {
            conn->clearDevice();
            conn->setRemovable(true);
        }

        virtual standby_info_type standby_info(id_type const & id, connection_type * conn)
        {
            standby_info_type info;
            info.vidpid = id.dev.vidpid();
            info.sn = conn->serialNumber();
            return info;
        }

        virtual void update_id(id_type & id)
        {
            std::string s = id.dev.serial_number();
            id.sn = QString::fromUtf8(s.data(), s.size());
        }

        virtual bool is_compatible(id_type const & id, standby_info_type const & si)
        {
            return id.dev.vidpid() == si.vidpid && id.sn == si.sn;
        }

    private:
        LibybUsbEnumerator * m_self;
    };

    GenericUsbEnumerator m_devenum;

    struct acm_id_standby
    {
        uint8_t intfno;
        std::string intfname;
    };

    struct acm_id
    {
		uint8_t cfg_value;
        uint8_t intfno;
        std::string intfname;

        uint8_t outep;
        uint8_t inep;

        yb::usb_device dev;

        friend bool operator<(acm_id const & lhs, acm_id const & rhs)
        {
            return lhs.intfno < rhs.intfno
                || (lhs.intfno == rhs.intfno && lhs.intfname < rhs.intfname);
        }
    };

    class UsbAcmEnumerator
        : public DeviceEnumerator<UsbAcmConnection2, acm_id, acm_id_standby>
    {
    public:
        explicit UsbAcmEnumerator(LibybUsbEnumerator * self)
            : m_self(self)
        {
        }

        virtual UsbAcmConnection2 * create(acm_id const &)
        {
            return new UsbAcmConnection2(m_self->m_runner);
        }

        virtual void resurrect(id_type const & id, UsbAcmConnection2 * conn)
        {
			conn->setup(id.dev, id.cfg_value, id.intfno, id.outep, id.inep);

            QString deviceName = GenericUsbConnection::formatDeviceName(id.dev);
            QString name;
            if (id.intfname.empty())
                name = QString("ACM%1 @ %2").arg(QString::number(id.intfno), deviceName);
            else
                name = QString("%1 @ %2").arg(QString::fromUtf8(id.intfname.data(), id.intfname.size()), deviceName);
            conn->setName(name);
            conn->setRemovable(false);
            conn->setPersistent(!id.intfname.empty());
        }

        virtual void clear(connection_type * conn)
        {
            conn->clear();
            conn->setRemovable(true);
        }

        virtual acm_id_standby standby_info(id_type const & id, UsbAcmConnection2 *)
        {
            acm_id_standby info;
            info.intfno = id.intfno;
            info.intfname = id.intfname;
            return info;
        }

        virtual bool is_compatible(id_type const & id, acm_id_standby const & si)
        {
            return (!si.intfname.empty() && !id.intfname.empty() && si.intfname == id.intfname)
                || (si.intfname.empty() && id.intfname.empty() && si.intfno == id.intfno);
        }

    private:
        LibybUsbEnumerator * m_self;
    };

    UsbAcmEnumerator m_acm_conns;
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
    yb::async_runner m_yb_runner;
#endif // HAVE_LIBYB
    QHash<PortConnection *, ShupitoConnection *> m_autoShupitos;
    QHash<QObject *, PortConnection *> m_autoShupitosRev;
};

extern ConnectionManager2 * psConMgr2;
#define sConMgr2 (*psConMgr2)

#endif // CONNECTIONMGR2_H
