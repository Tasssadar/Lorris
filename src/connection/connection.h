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

class WorkTab;
class WorkTabInfo;

enum ConnectionState {
    st_disconnected,
    st_connecting,
    st_connected
};

// TODO: maybe we should just remove this and use dynamic_cast?
enum ConnectionType
{
    CONNECTION_SERIAL_PORT = 0,
    CONNECTION_SHUPITO_TUNNEL = 1,
    CONNECTION_SHUPITO     = 2,
    CONNECTION_TCP_SOCKET  = 3,
    MAX_CON_TYPE           = 4
};

class Connection : public QObject
{
    Q_OBJECT

public:
    Connection();
    ~Connection();

    bool removable() const { return m_removable; }
    void setRemovable(bool value) { m_removable = value; emit changed(); }

    quint8 getType() { return m_type; }

    void setIDString(const QString& str) { if (m_idString != str) { m_idString = str; emit changed(); } }
    QString const & GetIDString() const { return m_idString; }

    QString const & name() const { return m_idString; }
    void setName(const QString& str) { this->setIDString(str); }

    virtual bool Open() = 0;
    virtual void OpenConcurrent() = 0;
    virtual void Close() {}

    bool isOpen() const { return m_state == st_connected; }
    ConnectionState state() const { return m_state; }

    void addRef();
    void release();
    void addTabRef();
    void releaseTab();

    virtual QHash<QString, QVariant> config() const;
    virtual bool applyConfig(QHash<QString, QVariant> const & config);

Q_SIGNALS:
    void connectResult(Connection *con, bool open);
    void connected(bool connected);
    void stateChanged(ConnectionState state);
    void changed();

    // This will be emitted right before a *planned* disconnect happens
    // to allow clients to send shutdown chatter.
    void disconnecting();

protected:
    void SetState(ConnectionState state);
    void SetOpen(bool open);

    quint8 m_type;

private:
    ConnectionState m_state;
    QString m_idString;
    int m_refcount;
    int m_tabcount;
    bool m_removable;
};

Q_DECLARE_METATYPE(Connection *)

class PortConnection : public Connection
{
    Q_OBJECT

Q_SIGNALS:
    void dataRead(const QByteArray& data);

public slots:
    virtual void SendData(const QByteArray & /*data*/) {}
};

#endif // CONNECTION_H
