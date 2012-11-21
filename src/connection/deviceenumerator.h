#ifndef DEVICEENUMERATOR_H
#define DEVICEENUMERATOR_H

#include "connection.h"

class DeviceEnumeratorBase
    : public QObject
{
    Q_OBJECT

protected slots:
    virtual void connectionDestroyed() = 0;
};

template <typename Conn, typename Id>
class DeviceEnumerator
    : public DeviceEnumeratorBase
{
public:
    typedef typename Id::standby_info_type standby_info_type;

    ~DeviceEnumerator()
    {
        std::map<Id, ConnectionPointer<Conn>> seen_devices;
        seen_devices.swap(m_seen_devices);
        for (std::map<Id, ConnectionPointer<Conn>>::iterator it = seen_devices.begin(); it != seen_devices.end(); ++it)
            it->second.take()->releaseAll();

        std::map<Conn *, standby_info_type> standby_conns;
        standby_conns.swap(m_standby_conns);
        for (std::map<Conn *, standby_info_type>::iterator it = standby_conns.begin(); it != standby_conns.end(); ++it)
            it->first->releaseAll();
    }

    template <typename ConstIterator, typename CreateFn, typename ResurrectFn, typename ClearFn>
    void update(ConstIterator first, ConstIterator last, CreateFn const & create, ResurrectFn const & resurrect, ClearFn const & clear)
    {
        empty_update update;
        this->update(first, last, create, resurrect, clear, update);
    }

    template <typename ConstIterator, typename CreateFn, typename ResurrectFn, typename ClearFn, typename UpdateFn>
    void update(ConstIterator first, ConstIterator last, CreateFn const & create, ResurrectFn const & resurrect, ClearFn const & clear, UpdateFn const & update)
    {
        {
            std::set<Id> ids(first, last);
            for (std::map<Id, ConnectionPointer<Conn>>::iterator it = m_seen_devices.begin(); it != m_seen_devices.end();)
            {
                if (ids.find(it->first) != ids.end())
                {
                    ++it;
                    continue;
                }

                standby_info_type si = it->first.standby_info(it->second.data());
                clear(it->second.data());
                m_standby_conns[it->second.data()] = si;

                std::map<Id, ConnectionPointer<Conn>>::iterator del_it = it;
                ++it;
                m_seen_devices.erase(del_it);
            }
        }

        for (; first != last; ++first)
        {
            Id const & id = *first;

            std::map<Id, ConnectionPointer<Conn>>::const_iterator it = m_seen_devices.find(id);
            if (it != m_seen_devices.end())
                continue;

            Id id2 = update(id);
            std::map<Conn *, standby_info_type>::iterator compat_it;
            for (compat_it = m_standby_conns.begin(); compat_it != m_standby_conns.end(); ++compat_it)
            {
                if (id2.compatible_with(compat_it->second))
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

        for (std::map<Id, ConnectionPointer<Conn>>::iterator it = m_seen_devices.begin(); it != m_seen_devices.end(); ++it)
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
    struct empty_update
    {
        Id & operator()(Id & id) const { return id; }
        Id const & operator()(Id const & id) const { return id; }
    };

    std::map<Id, ConnectionPointer<Conn>> m_seen_devices;
    std::map<Conn *, standby_info_type> m_standby_conns;
};

#endif // DEVICEENUMERATOR_H
