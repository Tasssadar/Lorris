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

    quint8 getType() { return m_type; }

    QString GetIDString() { return m_idString; }
    virtual bool Open();
    virtual void OpenConcurrent();
    virtual void Close() { }
    virtual void SendData(const QByteArray &data);

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

protected:
    bool opened;

    quint8 m_type;
    std::set<quint16> m_usingTabsIDs;

    QString m_idString;
};

#endif // CONNECTION_H
