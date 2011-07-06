#ifndef WORKTAB_H
#define WORKTAB_H

#include "common.h"
#include <QtGui/QWidget>
#include "connection/connection.h"

class WorkTab : public QObject
{
    Q_OBJECT
    public:
        virtual ~WorkTab();
        virtual QWidget *GetTab(QWidget *parent);

        void setId(uint16_t id) { m_id = id; }
        uint16_t getId() { return m_id; }

        void setConnection(Connection *con)
        {
            m_con = con;
            connect(m_con, SIGNAL(dataRead(QByteArray)), this, SLOT(readData(QByteArray)));
            m_con->AddUsingTab(m_id);
        }

        static void DeleteAllMembers(QLayout *layout);

    protected slots:
        virtual void readData(QByteArray data);

    protected:
        explicit WorkTab();

        Connection *m_con;
        uint16_t m_id;
};

#endif // WORKTAB_H
