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
#include "../shared/programmer.h"

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

    ConnectionPointer<Connection> choose(PrimaryConnectionTypes allowedConns, ConnectionPointer<Connection> const & preselectedConn);

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
    void progBtn_clicked(int programmer);

    void on_actionCreateTcpClient_triggered();
    void on_actionCreateUsbAcmConn_triggered();

    void on_tcHostEdit_textChanged(const QString &arg1);
    void on_tcPortEdit_valueChanged(int arg1);

    void on_usbVidEdit_textChanged(QString const & value);
    void on_usbPidEdit_textChanged(QString const & value);
    void on_usbAcmSnEdit_textChanged(QString const & value);
    void on_usbIntfNameEdit_textChanged(QString const & value);
    void on_usbBaudRateEdit_editTextChanged(QString const & value);
    void on_usbDataBitsCombo_currentIndexChanged(int value);
    void on_usbParityCombo_currentIndexChanged(int value);
    void on_usbStopBitsCombo_currentIndexChanged(int value);

    void on_actionConnect_triggered();
    void on_actionDisconnect_triggered();
    void on_actionClone_triggered();

    void on_persistNameButton_clicked();

private:
    void focusNewConn(Connection * conn);
    void selectConn(Connection * conn);
    void updateDetailsUi(Connection * conn);
    void setActiveProgBtn(int type);

    Ui::ChooseConnectionDlg *ui;
    QHash<Connection *, QListWidgetItem *> m_connectionItemMap;
    ConnectionPointer<Connection> m_current;
    int m_allowedConns;
    QPushButton *m_prog_btns[programmer_max];
};

#endif // CHOOSECONNECTIONDLG_H
