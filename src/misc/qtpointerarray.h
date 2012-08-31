/**********************************************
**    This file is part of Lorris
**    http://tasssadar.github.com/Lorris/
**
**    See README and COPYING
***********************************************/

#ifndef QTPOINTERARRAY_H
#define QTPOINTERARRAY_H

#include <QObject>
#include <vector>

class VectorHolder : public QObject
{
    Q_OBJECT

public:
    QObject *operator [](const quint32& idx) const
    {
        return m_objects[idx];
    }

    inline std::vector<QObject*>::iterator begin() { return m_objects.begin(); }
    inline std::vector<QObject*>::iterator end() { return m_objects.end(); }

    inline bool empty() const { return m_objects.empty(); }
    inline size_t size() const { return m_objects.size(); }
    inline void clear() { m_objects.clear(); }
    inline void clearAndDisconnect()
    {
        for(quint32 i = 0; i < m_objects.size(); ++i)
            disconnect(m_objects[i], 0, this, 0);
        m_objects.clear();
    }

    inline std::vector<QObject*>::iterator erase(std::vector<QObject*>::iterator pos)
    {
        return m_objects.erase(pos);
    }

    const std::vector<QObject*>& data() const { return m_objects; }

    std::vector<QObject*>::iterator find(QObject *obj)
    {
        for(std::vector<QObject*>::iterator itr = m_objects.begin(); itr != m_objects.end(); ++itr)
            if(*itr == obj)
                return itr;
        return m_objects.end();
    }

    inline void push_back(QObject* obj)
    {
        m_objects.push_back(obj);
    }

public slots:
    void destroyedObj(QObject *obj)
    {
        std::vector<QObject*>::iterator itr = find(obj);
        if(itr == m_objects.end())
            return;
        m_objects.erase(itr);
    }

private:
    std::vector<QObject*> m_objects;
};

template <class T>
class QtPointerArray
{
    typedef typename std::vector<QObject*>::iterator iterator;
public:
    explicit QtPointerArray()
    {
    }

    quint32 add(T *obj);
    void take(QObject *obj);
    T *take(quint32 idx);
    void destroyAll();
    std::vector<T*> takeAll();
    void clear() { m_objects.clearAndDisconnect(); }

    T *at(quint32 idx) const
    {
        return static_cast<T*>(m_objects[idx]);
    }

    T *operator [](const quint32& idx) const
    {
        return static_cast<T*>(m_objects[idx]);
    }

    bool contains(T *obj)
    {
        return find(obj) != m_objects.end();
    }

    iterator find(T *obj) { return m_objects.find(static_cast<QObject*>(obj)); }
    quint32 size() const { return m_objects.size(); }
    bool empty() const { return m_objects.empty(); }

    std::vector<T*> data() const
    {
        std::vector<T*> res;
        for(quint32 i = 0; i < m_objects.size(); ++i)
            res.push_back(static_cast<T*>(m_objects[i]));
        return res;
    }

private:
    VectorHolder m_objects;
};

template <class T>
quint32 QtPointerArray<T>::add(T *obj)
{
    if(contains(obj))
        throw QObject::tr("QtPointerArray: object is already in array");

    QObject::connect(obj, SIGNAL(destroyed(QObject*)), &m_objects, SLOT(destroyedObj(QObject*)));

    m_objects.push_back(obj);
    return m_objects.size()-1;
}

template <class T>
void QtPointerArray<T>::take(QObject *obj)
{
    iterator itr = find(obj);
    if(itr == m_objects.end())
        return;

    m_objects.erase(itr);
}

template <class T>
T *QtPointerArray<T>::take(quint32 idx)
{
    if(idx >= m_objects.size())
        return NULL;

    iterator itr = m_objects.begin()+idx;
    T *res = static_cast<T*>(*itr);
    m_objects.erase(itr);
    return res;
}

template <class T>
void QtPointerArray<T>::destroyAll()
{
    for(quint32 i = 0; i < m_objects.size(); ++i)
    {
        QObject::disconnect(m_objects[i], 0, &m_objects, 0);
        delete m_objects[i];
    }
    m_objects.clear();
}

template <class T>
std::vector<T*> QtPointerArray<T>::takeAll()
{
    std::vector<T*> res;
    for(quint32 i = 0; i < m_objects.size(); ++i)
    {
        QObject::disconnect(m_objects[i], 0, &m_objects, 0);
        res.push_back(static_cast<T*>(m_objects[i]));
    }
    m_objects.clear();
    return res;
}

#endif // QTPOINTERARRAY_H
