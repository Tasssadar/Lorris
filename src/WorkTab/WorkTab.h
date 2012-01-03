#ifndef WORKTAB_H
#define WORKTAB_H

#include <QtGui/QWidget>
#include "connection/connection.h"

class WorkTab : public QWidget
{
    Q_OBJECT
    public:
        virtual ~WorkTab();

        void setId(quint16 id) { m_id = id; }
        quint16 getId() { return m_id; }

        void setConnection(Connection *con)
        {
            m_con = con;
            connect(m_con, SIGNAL(dataRead(QByteArray)), this, SLOT(readData(QByteArray)));
            connect(m_con, SIGNAL(connected(bool)), this, SLOT(connectedStatus(bool)));
            m_con->AddUsingTab(m_id);
        }

        static void DeleteAllMembers(QLayout *layout);

        virtual void onTabShow();

    protected slots:
        virtual void readData(const QByteArray &data);
        virtual void connectedStatus(bool connected);

    protected:
        explicit WorkTab();

        Connection *m_con;
        quint16 m_id;
};

#endif // WORKTAB_H
