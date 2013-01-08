#ifndef LORRIS_MISC_THREADCHANNEL_H
#define LORRIS_MISC_THREADCHANNEL_H

#include <QObject>
#include <QMutex>
#include <QAtomicPointer>
#include <vector>

class ThreadChannelBase
    : public QObject
{
    Q_OBJECT

signals:
    void dataReceived();

public:
    ThreadChannelBase();
    ~ThreadChannelBase();

protected:
    void notifyDataReady();
    bool event(QEvent * e);

private:
    QAtomicPointer<QEvent> m_notifyEvent;

    ThreadChannelBase(ThreadChannelBase const &);
    ThreadChannelBase & operator=(ThreadChannelBase const &);
};

template <typename T>
class ThreadChannel
    : public ThreadChannelBase
{
public:
    void send(T const & v)
    {
        QMutexLocker l(&m_mutex);
        m_data.push_back(v);
        this->notifyDataReady();
    }

    void send(T const * first, T const * last)
    {
        QMutexLocker l(&m_mutex);
        m_data.insert(m_data.end(), first, last);
        this->notifyDataReady();
    }

    void receive(std::vector<T> & v)
    {
        QMutexLocker l(&m_mutex);
        v.swap(m_data);
        m_data.clear();
    }

private:
    QMutex m_mutex;
    std::vector<T> m_data;
};

#endif // LORRIS_MISC_THREADCHANNEL_H
