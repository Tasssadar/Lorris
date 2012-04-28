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

#ifndef UI_CONNECTBUTTON_H
#define UI_CONNECTBUTTON_H

#include <QObject>
#include <QMenu>
#include <QAction>
#include <QToolButton>
#include "../connection/connection.h"
#include "../connection/shupitoconn.h"

class ConnectButton : public QObject
{
    Q_OBJECT

public:
    ConnectButton(QToolButton * btn);
    void setConn(ConnectionPointer<Connection> const & conn);
    void setConnectionType(PrimaryConnectionType type);

public slots:
    ConnectionPointer<Connection> choose();

Q_SIGNALS:
    void connectionChosen(ConnectionPointer<Connection> const & newConnection);

private slots:
    void connectTriggered();
    void connectionStateChanged(ConnectionState state);

private:
    QMenu m_menu;
    QAction * m_connectAction;
    QAction * m_chooseAction;

    QToolButton * m_btn;
    ConnectionPointer<Connection> m_conn;
    PrimaryConnectionType m_connType;
};

#endif // UI_CONNECTBUTTON_H
