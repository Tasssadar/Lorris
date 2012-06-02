/**********************************************
**    This file is part of Lorris
**    http://tasssadar.github.com/Lorris/
**
**    See README and COPYING
***********************************************/

#ifndef CONNECTIONMGR2_H
#define CONNECTIONMGR2_H

#include "../connection/connection.h"
#include <QTimer>
#include <QHash>
#include <QSet>
#include <set>

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

    QVariant config() const;
    bool applyConfig(QVariant const & config);

Q_SIGNALS:
    void connAdded(Connection * conn);
    void connRemoved(Connection * conn);

private slots:
    void connectionDestroyed();

private:
    void addUserOwnedConn(Connection * conn);
    void clearUserOwnedConns();
    QSet<Connection *> m_userOwnedConns;

    QList<Connection *> m_conns;
    QScopedPointer<SerialPortEnumerator> m_serialPortEnumerator;
};

extern ConnectionManager2 * psConMgr2;
#define sConMgr2 (*psConMgr2)

#endif // CONNECTIONMGR2_H
