#ifndef CONNECTION_H
#define CONNECTION_H

#include <QString>
#include <QObject>
#include <set>
#include <QFuture>
#include <QFutureWatcher>

class Connection : public QObject
{
    Q_OBJECT

Q_SIGNALS:
    void dataRead(QByteArray data);
    void connectResult(Connection *con, bool open);

public:
    virtual ~Connection();
    explicit Connection();

    uint8_t getType() { return m_type; }

    QString GetIDString() { return m_idString; }
    virtual bool Open();
    virtual void OpenConcurrent();
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

protected slots:
   // virtual void concurrentFinished();

protected:
    bool opened;
    QFuture<bool> *future;
    QFutureWatcher<bool> *watcher;

    uint8_t m_type;
    std::set<uint16_t> m_usingTabsIDs;

    QString m_idString;
};

#endif // CONNECTION_H
