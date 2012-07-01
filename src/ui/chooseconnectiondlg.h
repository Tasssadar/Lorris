/**********************************************
**    This file is part of Lorris
**    http://tasssadar.github.com/Lorris/
**
**    See README and COPYING
***********************************************/

#ifndef CHOOSECONNECTIONDLG_H
#define CHOOSECONNECTIONDLG_H

#include <QDialog>
#include <QAbstractListModel>
#include <QItemSelection>
#include <QListWidgetItem>
#include "../connection/connection.h"

namespace Ui {
class ChooseConnectionDlg;
}

class Connection;
class PortConnection;
class ShupitoConnection;

class ChooseConnectionDlg : public QDialog
{
    Q_OBJECT
    
public:
    explicit ChooseConnectionDlg(QWidget *parent = 0);
    ~ChooseConnectionDlg();

    ConnectionPointer<PortConnection> choosePort(ConnectionPointer<Connection> const & preselectedConn);
    ConnectionPointer<ShupitoConnection> chooseShupito(ConnectionPointer<Connection> const & preselectedConn);

private slots:
    void on_actionCreateSerialPort_triggered();
    void on_actionRemoveConnection_triggered();
    void on_connectionNameEdit_textChanged(const QString &arg1);
    void on_connectionsList_itemSelectionChanged();

    void connAdded(Connection * conn);
    void connRemoved(Connection * conn);
    void connChanged();

    void on_spDeviceNameEdit_textChanged(const QString &arg1);
    void on_connectionsList_doubleClicked(const QModelIndex &index);
    void on_spBaudRateEdit_editTextChanged(const QString &arg1);

    void on_actionCreateTcpClient_triggered();

    void on_tcHostEdit_textChanged(const QString &arg1);

    void on_tcPortEdit_valueChanged(int arg1);

    void on_actionConnect_triggered();

    void on_actionDisconnect_triggered();

private:
    void selectConn(Connection * conn);
    void updateDetailsUi(Connection * conn);

    Ui::ChooseConnectionDlg *ui;
    QHash<Connection *, QListWidgetItem *> m_connectionItemMap;
    ConnectionPointer<Connection> m_current;
    int m_allowedConns;
};

#endif // CHOOSECONNECTIONDLG_H
