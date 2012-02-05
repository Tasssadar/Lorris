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

class Connection : public QObject
{
    Q_OBJECT

Q_SIGNALS:
    void dataRead(const QByteArray& data);
    void connectResult(Connection *con, bool open);
    void connected(bool connected);

public:
    virtual ~Connection();
    explicit Connection();

    virtual quint8 getType() { return m_type; }

    void setIDString(const QString& str) { m_idString = str; }
    QString GetIDString() { return m_idString; }
    virtual bool Open();
    virtual void OpenConcurrent();
    virtual void Close() { }

    bool isOpen() { return opened; }

    void AddUsingTab(quint16 id)
    {
        m_usingTabsIDs.insert(id);
    }

    void RemoveUsingTab(quint16 id)
    {
        m_usingTabsIDs.erase(id);
    }

    bool IsUsedByTab()
    {
        return !(m_usingTabsIDs.empty());
    }

public slots:
    virtual void SendData(const QByteArray &data);

protected:
    bool opened;

    quint8 m_type;
    std::set<quint16> m_usingTabsIDs;

    QString m_idString;
};

#endif // CONNECTION_H
