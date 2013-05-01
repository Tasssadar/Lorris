/**********************************************
**    This file is part of Lorris
**    http://tasssadar.github.com/Lorris/
**
**    See README and COPYING
***********************************************/

#ifndef QOBJECTPOINTER_H
#define QOBJECTPOINTER_H

#include <QObject>

class QtObjectHolder : public QObject
{
    Q_OBJECT
public:
    QtObjectHolder(QObject *p);

    QObject *data() const { return m_ptr; }
    void set(QObject *p);

private slots:
    void objDestroyed();

private:
    QObject *m_ptr;
};

template <class T>
class QtObjectPointer
{
public:
    explicit QtObjectPointer(T *p = NULL) : m_holder(p)
    {
    }

    inline T *data() const { return (T*)m_holder.data(); }
    bool isNull() const { return m_holder.data() == NULL; }

    T *take()
    {
        T *res = data();
        m_holder.set(NULL);
        return res;
    }

    inline QtObjectPointer<T> &operator=(const QtObjectPointer<T> &p)
    {
        if(data() != p.data())
            m_holder.set(p.data());
        return *this;
    }

    inline QtObjectPointer<T> &operator=(T* p)
    {
        if(data() != p)
            m_holder.set(p);

        return *this;
    }

    inline T* operator->() const { return data(); }
    inline T& operator*() const { return *data(); }
    inline operator T*() const { return data(); }

private:
    QtObjectHolder m_holder;
};

#endif // QOBJECTPOINTER_H
