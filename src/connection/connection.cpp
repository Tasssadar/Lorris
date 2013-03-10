/**********************************************
**    This file is part of Lorris
**    http://tasssadar.github.com/Lorris/
**
**    See README and COPYING
***********************************************/

#include "connection.h"
#include "../WorkTab/WorkTab.h"
#include "../shared/programmer.h"
#include <QStringBuilder>

Connection::Connection(ConnectionType type)
    : m_state(st_disconnected), m_defaultName(true), m_refcount(1), m_tabcount(0), m_removable(true),
      m_persistent(false), m_type(type), m_companionId(0)
{
}

Connection::~Connection()
{
    // Note that m_refcount need not be 0 here. We allow connections
    // to be destroyed explicitly via releaseAll() and clients must
    // listen to either destroying() or destroyed() signal.
}

QString Connection::details() const
{
    return QString();
}


void Connection::SetState(ConnectionState state)
{
    bool oldOpen = (m_state == st_connected);
    bool newOpen = (state == st_connected);

    if(state != m_state)
    {
        m_state = state;
        if (oldOpen != newOpen)
            emit connected(newOpen);
        emit stateChanged(state);
    }
}

bool Connection::isMissing() const
{
    return m_state == st_missing || m_state == st_connect_pending;
}

void Connection::markMissing()
{
    if (m_state == st_connected || m_state == st_connect_pending)
        this->SetState(st_connect_pending);
    else
        this->SetState(st_missing);
}

void Connection::markPresent()
{
    if (m_state == st_connect_pending)
    {
        this->SetState(st_disconnected);
        this->OpenConcurrent();
    }
    else if (m_state == st_missing)
    {
        this->SetState(st_disconnected);
    }
}

void Connection::setName(const QString& str, bool isDefault)
{
    this->setIDString(str);
    m_defaultName = isDefault;
    emit changed();
}

void Connection::OpenConcurrent()
{
    if (m_state == st_disconnected)
        this->doOpen();
}

void Connection::Close()
{
    switch (m_state)
    {
    case st_connect_pending:
        this->SetState(st_missing);
        break;
    case st_connecting:
    case st_connected:
        this->doClose();
        break;
    }
}

void Connection::addRef()
{
    ++m_refcount;
}

void Connection::release()
{
    if (--m_refcount == 0)
    {
        this->Close();
        emit destroying();
        delete this;
    }
}

void Connection::addTabRef()
{
    addRef();
    ++m_tabcount;
}

void Connection::releaseTab()
{
    if(--m_tabcount == 0)
        Close();
    release();
}

QHash<QString, QVariant> Connection::config() const
{
    QHash<QString, QVariant> res;
    if (!this->hasDefaultName())
        res["name"] = this->name();
    res["companion"] = this->getCompanionId();
    return res;
}

bool Connection::applyConfig(QHash<QString, QVariant> const & config)
{
    if (config.contains("name"))
        this->setName(config.value("name").toString());
    this->setCompanionId(config.value("companion", m_companionId).toLongLong());
    return true;
}

void Connection::releaseAll()
{
    emit destroying();
    delete this;
}

void Connection::setPersistent(bool value)
{
    if (m_persistent != value)
    {
        m_persistent = value;
        if (value)
            this->addRef();
        else
            this->release();
    }
}

PortConnection::PortConnection(ConnectionType type) : Connection(type)
{
    m_programmer_type = programmer_avr232boot;
}

QHash<QString, QVariant> PortConnection::config() const
{
    QHash<QString, QVariant> res = this->Connection::config();
    res["programmer_type"] = (int)this->programmerType();
    return res;
}

bool PortConnection::applyConfig(QHash<QString, QVariant> const & config)
{
    this->setProgrammerType(config.value("programmer_type", m_programmer_type).toInt());
    return Connection::applyConfig(config);
}

ConnectionPointer<Connection> Connection::clone()
{
    return ConnectionPointer<Connection>();
}
