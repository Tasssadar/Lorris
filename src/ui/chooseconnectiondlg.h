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

class ChooseConnectionDlg : public QDialog
{
    Q_OBJECT
    
public:
    explicit ChooseConnectionDlg(QWidget *parent = 0);
    explicit ChooseConnectionDlg(Connection * preselectedConn, QWidget *parent = 0);
    ~ChooseConnectionDlg();

    Connection * current() const { return m_current; }

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

private:
    void init(Connection * preselectedConn);
    void updateDetailsUi(Connection * conn);

    Ui::ChooseConnectionDlg *ui;
    QHash<Connection *, QListWidgetItem *> m_connectionItemMap;
    Connection * m_current;
};

#endif // CHOOSECONNECTIONDLG_H
