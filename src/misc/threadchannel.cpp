#include "threadchannel.h"
#include <QCoreApplication>

struct ThreadChannelMessage
    : QEvent
{
    static const QEvent::Type type = static_cast<QEvent::Type>(2000);

    ThreadChannelMessage()
        : QEvent(type)
    {
    }
};

ThreadChannelBase::ThreadChannelBase()
    : m_notifyEvent(new ThreadChannelMessage())
{
}

ThreadChannelBase::~ThreadChannelBase()
{
    delete m_notifyEvent.fetchAndStoreOrdered(0);
}

void ThreadChannelBase::notifyDataReady()
{
    if (QEvent * ev = m_notifyEvent.fetchAndStoreOrdered(0))
        QCoreApplication::postEvent(this, ev);
}

bool ThreadChannelBase::event(QEvent * e)
{
    if (e->type() == ThreadChannelMessage::type)
    {
        QEvent * ev = new ThreadChannelMessage();
        ev = m_notifyEvent.fetchAndStoreOrdered(ev);
        delete ev;

        emit dataReceived();
        return true;
    }
    else
    {
        return this->QObject::event(e);
    }
}
