#ifndef CONNECTION_H
#define CONNECTION_H

#include <QString>
#include <QObject>
#include <set>

class Connection : public QObject
{
    Q_OBJECT

Q_SIGNALS:
    void dataRead(QByteArray data);

public:
    virtual ~Connection();
    explicit Connection();

    uint8_t getType() { return m_type; }

    QString GetIDString() { return m_idString; }
    virtual bool Open() { return false; }
    virtual void Close() { }
    virtual void SendData(QByteArray data);

    void AddUsingTab(uint16_t id)
    {
        m_usingTabsIDs.insert(id);
    }

    void RemoveUsingTab(uint16_t id)
    {
        m_usingTabsIDs.erase(id);
    }

    bool IsUsedByTab()
    {
        return !(m_usingTabsIDs.empty());
    }

protected:
    bool opened;

    uint8_t m_type;
    std::set<uint16_t> m_usingTabsIDs;

    QString m_idString;
};

#endif // CONNECTION_H
