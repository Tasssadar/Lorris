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

        void setConnection(Connection *con)
        {
            m_con = con;
            connect(m_con, SIGNAL(dataRead(QByteArray)), this, SLOT(readData(QByteArray)));
        }

    protected slots:
        virtual void readData(QByteArray data);

    protected:
        explicit WorkTab();

        Connection *m_con;

};

#endif // WORKTAB_H
