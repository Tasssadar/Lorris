/**********************************************
**    This file is part of Lorris
**    http://tasssadar.github.com/Lorris/
**
**    See README and COPYING
***********************************************/

#ifndef CONNECTION_H
#define CONNECTION_H

#include <QString>
#include <QObject>
#include <QDataStream>
#include <set>
#include <QGridLayout>
#include <QVector>
#include <QMetaType>

enum ConnectionState {
    st_disconnected,
    st_connecting,
    st_connected,
    st_disconnecting,
    st_missing,
    st_connect_pending
};

// TODO: maybe we should just remove this and use dynamic_cast?
enum ConnectionType
{
    CONNECTION_SERIAL_PORT         = 0,
    CONNECTION_SHUPITO_TUNNEL      = 1,
    CONNECTION_PORT_SHUPITO        = 2,
    CONNECTION_TCP_SOCKET          = 3,
    CONNECTION_USB_SHUPITO         = 4,
    CONNECTION_PROXY_TUNNEL        = 6,
    CONNECTION_FLIP                = 7,
    CONNECTION_LIBYB_USB           = 8,
    CONNECTION_USB_ACM2            = 9,
    CONNECTION_SHUPITO23           = 10,
    CONNECTION_STM32               = 11,
    CONNECTION_SHUPITO_SPI_TUNNEL  = 12,
    CONNECTION_UDP_SOCKET          = 13,

    MAX_CON_TYPE
};

enum PrimaryConnectionType {
    pct_port_data = (1<<0),
    pct_shupito = (1<<1),
    pct_flip = (1<<2),
    pct_port_programmable = (1<<3),
    pct_stm32link = (1<<4),

    pct_programmable = pct_shupito | pct_flip | pct_port_programmable | pct_stm32link,
    pct_port = pct_port_data | pct_port_programmable
};

Q_DECLARE_FLAGS(PrimaryConnectionTypes, PrimaryConnectionType)
Q_DECLARE_OPERATORS_FOR_FLAGS(PrimaryConnectionTypes)

template <typename T>
class ConnectionPointer;

class Connection : public QObject
{
    Q_OBJECT

public:
    explicit Connection(ConnectionType type);

    bool removable() const { return m_removable; }
    void setRemovable(bool value) { m_removable = value; emit changed(); }

    bool persistent() const { return m_persistent; }
    void setPersistent(bool value);

    quint8 getType() const { return m_type; }

    void setIDString(const QString& str) { if (m_idString != str) { m_idString = str; emit changed(); } }
    QString const & GetIDString() const { return m_idString; }

    QString const & name() const { return m_idString; }
    void setName(const QString& str, bool isDefault = false);
    bool hasDefaultName() const { return m_defaultName; }

    qint64 getCompanionId(const QString& name) const;
    void setCompanionId(const QString& name, qint64 id);

    virtual QString details() const;

    void OpenConcurrent();
    void Close();

    inline bool isOpen() const { return m_state == st_connected; }
    ConnectionState state() const { return m_state; }

    void addRef();
    void release();
    void addTabRef();
    void releaseTab();

    bool isUsedByTab() const { return m_tabcount > 0; }

    // Emits the `destroying` signal to break all references,
    // then deletes the object.
    void releaseAll();

    virtual QHash<QString, QVariant> config() const;
    virtual bool applyConfig(QHash<QString, QVariant> const & config);

    virtual bool canSaveToSession() const { return false; }

    virtual bool clonable() const { return false; }
    virtual ConnectionPointer<Connection> clone();

    bool isMissing() const;

    virtual bool isNamePersistable() const { return false; }
    virtual void persistName() {}

signals:
    void connected(bool connected);
    void stateChanged(ConnectionState state);
    void changed();

    // This will be emitted right before a *planned* disconnect happens
    // to allow clients to send shutdown chatter.
    void disconnecting();

    // This is emitted right before delete is called, allowing clients
    // to clean up while the connection is still fully constructed.
    // Strong ref holders must abandon their refs without releasing them!
    void destroying();

protected:
    ~Connection();
    void SetState(ConnectionState state);

    void markMissing();
    void markPresent();

    virtual void doOpen() = 0;
    virtual void doClose() = 0;

private:
    ConnectionState m_state;
    QString m_idString;
    bool m_defaultName;
    int m_refcount;
    int m_tabcount;
    bool m_removable;
    bool m_persistent;
    quint8 m_type;
    QHash<QString, qint64> m_companionIds;
};

Q_DECLARE_METATYPE(Connection *)

class PortConnection : public Connection
{
    Q_OBJECT

Q_SIGNALS:
    void dataRead(const QByteArray& data);
    void programmerTypeChanged(int type);

public:
    explicit PortConnection(ConnectionType type);

    int programmerType() const { return m_programmer_type; }
    void setProgrammerType(int type);

    virtual QHash<QString, QVariant> config() const;
    virtual bool applyConfig(QHash<QString, QVariant> const & config);

public slots:
    virtual void SendData(const QByteArray & /*data*/) {}

protected:
    int m_programmer_type;
};

template <typename T>
class ConnectionPointer
{
public:
    explicit ConnectionPointer(T * conn = 0)
        : m_conn(conn)
    {
    }

    ConnectionPointer(ConnectionPointer const & other)
        : m_conn(other.m_conn)
    {
        if (m_conn)
            m_conn->addRef();
    }

    template <typename U>
    ConnectionPointer(ConnectionPointer<U> const & other)
        : m_conn(other.data())
    {
        if (m_conn)
            m_conn->addRef();
    }

    static ConnectionPointer fromPtr(T * conn)
    {
        ConnectionPointer res(conn);
        if (res)
            res->addRef();
        return res;
    }

    ~ConnectionPointer()
    {
        this->reset();
    }

    ConnectionPointer & operator=(ConnectionPointer const & other)
    {
        if (other.m_conn)
            other.m_conn->addRef();
        if (m_conn)
            m_conn->release();
        m_conn = other.m_conn;
        return *this;
    }

    template <typename U>
    ConnectionPointer & operator=(ConnectionPointer<U> const & other)
    {
        if (other)
            other->addRef();
        if (m_conn)
            m_conn->release();
        m_conn = other.data();
        return *this;
    }

    void reset(T * conn = 0)
    {
        if (m_conn)
            m_conn->release();
        m_conn = conn;
    }

    T * take()
    {
        T * res = m_conn;
        m_conn = 0;
        return res;
    }

    T * data() const { return m_conn; }
    T * operator->() const { return m_conn; }

    template <typename U>
    ConnectionPointer<U> staticCast() const
    {
        ConnectionPointer<U> res(static_cast<U *>(m_conn));
        if (res)
            res->addRef();
        return res;
    }

    template <typename U>
    ConnectionPointer<U> dynamicCast() const
    {
        ConnectionPointer<U> res(dynamic_cast<U *>(m_conn));
        if (res)
            res->addRef();
        return res;
    }

    typedef T* (ConnectionPointer::* UnspecifiedBoolType)() const;
    operator UnspecifiedBoolType() const { return m_conn? &ConnectionPointer::data: 0; }

    friend bool operator==(ConnectionPointer const & lhs, ConnectionPointer const & rhs)
    {
        return lhs.m_conn == rhs.m_conn;
    }

    friend bool operator!=(ConnectionPointer const & lhs, ConnectionPointer const & rhs)
    {
        return lhs.m_conn != rhs.m_conn;
    }

private:
    T * m_conn;
};

#endif // CONNECTION_H
