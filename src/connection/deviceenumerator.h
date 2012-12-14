#ifndef DEVICEENUMERATOR_H
#define DEVICEENUMERATOR_H

#include "connection.h"

class DeviceEnumeratorBase
    : public QObject
{
    Q_OBJECT

protected slots:
    virtual void connectionDestroyed() = 0;

protected:
    void registerConn(Connection * conn);
};

template <typename Conn, typename Id, typename StandbyInfo>
class DeviceEnumerator
    : public DeviceEnumeratorBase
{
public:
    typedef Conn connection_type;
    typedef Id id_type;
    typedef StandbyInfo standby_info_type;

    ~DeviceEnumerator()
    {
        std::map<Id, ConnectionPointer<Conn> > seen_devices;
        seen_devices.swap(m_seen_devices);
        for (typename std::map<Id, ConnectionPointer<Conn> >::iterator it = seen_devices.begin(); it != seen_devices.end(); ++it)
            it->second.take()->releaseAll();

        std::map<Conn *, standby_info_type> standby_conns;
        standby_conns.swap(m_standby_conns);
        for (typename std::map<Conn *, standby_info_type>::iterator it = standby_conns.begin(); it != standby_conns.end(); ++it)
            it->first->releaseAll();
    }

    virtual connection_type * create(id_type const & id) = 0;
    virtual void resurrect(id_type const & id, connection_type * conn) = 0;
    virtual void clear(connection_type * conn) = 0;
    virtual standby_info_type standby_info(id_type const & id, connection_type * conn) = 0;
    virtual void update_id(id_type &) {}
    virtual bool is_compatible(id_type const &, standby_info_type const &) { return true; }

    template <typename ConstIterator>
    void update(ConstIterator first, ConstIterator last)
    {
        {
            std::set<Id> ids(first, last);
            for (typename std::map<Id, ConnectionPointer<Conn> >::iterator it = m_seen_devices.begin(); it != m_seen_devices.end();)
            {
                if (ids.find(it->first) != ids.end())
                {
                    ++it;
                    continue;
                }

                standby_info_type si = standby_info(it->first, it->second.data());
                clear(it->second.data());
                m_standby_conns[it->second.data()] = si;

                typename std::map<Id, ConnectionPointer<Conn> >::iterator del_it = it;
                ++it;
                m_seen_devices.erase(del_it);
            }
        }

        for (; first != last; ++first)
        {
            Id const & id = *first;

            typename std::map<Id, ConnectionPointer<Conn> >::const_iterator it = m_seen_devices.find(id);
            if (it != m_seen_devices.end())
                continue;

            Id id2 = id;
            update_id(id2);
            typename std::map<Conn *, standby_info_type>::iterator compat_it;
            for (compat_it = m_standby_conns.begin(); compat_it != m_standby_conns.end(); ++compat_it)
            {
                if (is_compatible(id2, compat_it->second))
                    break;
            }

            if (compat_it != m_standby_conns.end())
            {
                ConnectionPointer<Conn> conn = ConnectionPointer<Conn>::fromPtr(compat_it->first);
                resurrect(id2, conn.data());
                m_seen_devices.insert(std::make_pair(id2, conn));
                m_standby_conns.erase(compat_it);
            }
            else
            {
                ConnectionPointer<Conn> new_conn(create(id2));
                this->registerConn(new_conn.data());

                resurrect(id2, new_conn.data());
                connect(new_conn.data(), SIGNAL(destroying()), static_cast<DeviceEnumeratorBase *>(this), SLOT(connectionDestroyed())); 
                m_seen_devices.insert(std::make_pair(id2, new_conn));
            }
        }
    }

protected:
    void connectionDestroyed()
    {
        Conn * conn = static_cast<Conn *>(this->sender());

        for (typename std::map<Id, ConnectionPointer<Conn> >::iterator it = m_seen_devices.begin(); it != m_seen_devices.end(); ++it)
        {
            if (it->second.data() == conn)
            {
                m_seen_devices.erase(it);
                break;
            }
        }

        m_standby_conns.erase(conn);
    }

private:
    std::map<Id, ConnectionPointer<Conn> > m_seen_devices;
    std::map<Conn *, standby_info_type> m_standby_conns;
};

#endif // DEVICEENUMERATOR_H
