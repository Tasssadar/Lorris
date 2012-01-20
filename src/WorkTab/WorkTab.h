/****************************************************************************
**
**    This file is part of Lorris.
**    Copyright (C) 2012 Vojtěch Boček
**
**    Contact: <vbocek@gmail.com>
**             https://github.com/Tasssadar
**
**    Lorris is free software: you can redistribute it and/or modify
**    it under the terms of the GNU General Public License as published by
**    the Free Software Foundation, either version 3 of the License, or
**    (at your option) any later version.
**
**    Lorris is distributed in the hope that it will be useful,
**    but WITHOUT ANY WARRANTY; without even the implied warranty of
**    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
**    GNU General Public License for more details.
**
**    You should have received a copy of the GNU General Public License
**    along with Lorris.  If not, see <http://www.gnu.org/licenses/>.
**
****************************************************************************/

#ifndef WORKTAB_H
#define WORKTAB_H

#include <QtGui/QWidget>

#include "common.h"
#include "connection/connection.h"

class WorkTab : public QWidget
{
    Q_OBJECT
    public:
        virtual ~WorkTab();

        void setId(quint16 id) { m_id = id; }
        quint16 getId() { return m_id; }

        void setConnection(Connection *con);

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
