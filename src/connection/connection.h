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

enum ConnectionType
{
    CONNECTION_SERIAL_PORT = 0,
    CONNECTION_FILE        = 1,
    CONNECTION_SHUPITO     = 2, // Do not use in WorkTabs, when connected to shupito, it identifies
                                // as serial port from WorkTab's point of view
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

    virtual QHash<QString, QVariant> config() const;
    virtual bool applyConfig(QHash<QString, QVariant> const & config);

Q_SIGNALS:
    void connectResult(Connection *con, bool open);
    void connected(bool connected);
    void stateChanged(ConnectionState state);
    void changed();
    void dataRead(const QByteArray& data);

    // This will be emitted right before a *planned* disconnect happens
    // to allow clients to send shutdown chatter.
    void disconnecting();

public slots:
    virtual void SendData(const QByteArray & data);

protected:
    void SetState(ConnectionState state);
    void SetOpen(bool open);

    quint8 m_type;

private:
    ConnectionState m_state;
    QString m_idString;
    int m_refcount;
    bool m_removable;
};

Q_DECLARE_METATYPE(Connection *)

class ConnectionBuilder : public QObject
{
    Q_OBJECT

Q_SIGNALS:
    void setCreateBtnStatus(bool connecting);
    void connectionFailed(const QString& msg);
    void connectionSuccess(Connection *con, const QString& tabName, WorkTab *tab, qint16 conType = -1);

public:
    ConnectionBuilder(QWidget *parent,  int moduleIdx);
    ~ConnectionBuilder();

    virtual void addOptToTabDialog(QGridLayout *layout);
    virtual void CreateConnection(WorkTab *tab);

protected:
    QWidget *m_parent;
    int m_module_idx;

    WorkTab *m_tab;
};

#endif // CONNECTION_H
