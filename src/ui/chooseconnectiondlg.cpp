#include "chooseconnectiondlg.h"
#include "ui_chooseconnectiondlg.h"
#include "../connection/connectionmgr2.h"
#include "../connection/serialport.h"
#include "../connection/tcpsocket.h"
#include <QMenu>

ChooseConnectionDlg::ChooseConnectionDlg(QWidget *parent) :
    QDialog(parent),
    ui(new Ui::ChooseConnectionDlg),
    m_current(0)
{
    this->init(0);
}

ChooseConnectionDlg::ChooseConnectionDlg(Connection * preselectedConn, QWidget *parent) :
    QDialog(parent),
    ui(new Ui::ChooseConnectionDlg),
    m_current(0)
{
    this->init(preselectedConn);
}

void ChooseConnectionDlg::init(Connection * preselectedConn)
{
    ui->setupUi(this);
    this->setWindowFlags(this->windowFlags() & ~Qt::WindowContextHelpButtonHint);

    ui->removeConnectionBtn->setDefaultAction(ui->actionRemoveConnection);

    QMenu * menu = new QMenu(this);
    menu->addAction(ui->actionCreateSerialPort);
    menu->addAction(ui->actionCreateTcpClient);
    ui->createConnectionBtn->setMenu(menu);

    sConMgr2.refresh();
    QList<Connection *> const & conns = sConMgr2.connections();
    for (int i = 0; i < conns.size(); ++i)
        this->connAdded(conns[i]);
    ui->connectionsList->sortItems();

    // Note that the preselected connection may be handled by a different manager
    // and as such may be missing in the map.
    if (m_connectionItemMap.contains(preselectedConn))
        m_connectionItemMap[preselectedConn]->setSelected(true);

    connect(&sConMgr2, SIGNAL(connAdded(Connection *)), this, SLOT(connAdded(Connection *)));
    connect(&sConMgr2, SIGNAL(connRemoved(Connection *)), this, SLOT(connRemoved(Connection *)));

    this->on_connectionsList_itemSelectionChanged();
}

void ChooseConnectionDlg::connAdded(Connection * conn)
{
    QListWidgetItem * item = new QListWidgetItem(conn->name(), ui->connectionsList);
    item->setData(Qt::UserRole, QVariant::fromValue(conn));
    m_connectionItemMap[conn] = item;
    connect(conn, SIGNAL(changed()), this, SLOT(connChanged()));
}

void ChooseConnectionDlg::connRemoved(Connection * conn)
{
    QListWidgetItem * item = m_connectionItemMap.take(conn);
    delete item;
}

void ChooseConnectionDlg::connChanged()
{
    Connection * conn = static_cast<Connection *>(this->sender());
    QListWidgetItem * item = m_connectionItemMap[conn];
    item->setText(conn->name());
    if (conn == m_current)
        this->updateDetailsUi(conn);
}

static void updateEditText(QLineEdit * w, QString const & value)
{
    if (w->text() != value)
        w->setText(value);
}

void ChooseConnectionDlg::updateDetailsUi(Connection * conn)
{
    updateEditText(ui->connectionNameEdit, conn->name());
    ui->actionRemoveConnection->setEnabled(conn->removable());

    switch (conn->getType())
    {
    case CONNECTION_SERIAL_PORT:
        {
            SerialPort * sp = static_cast<SerialPort *>(conn);
            ui->settingsStack->setCurrentWidget(ui->serialPortPage);
            updateEditText(ui->spBaudRateEdit->lineEdit(), QString::number((int)sp->baudRate()));
            updateEditText(ui->spDeviceNameEdit, sp->deviceName());
            ui->spDeviceNameEdit->setEnabled(sp->devNameEditable());
        }
        break;
    case CONNECTION_TCP_SOCKET:
        {
            TcpSocket * tc = static_cast<TcpSocket *>(conn);
            ui->settingsStack->setCurrentWidget(ui->tcpClientPage);
            updateEditText(ui->tcHostEdit, tc->host());
            ui->tcPortEdit->setValue(tc->port());
        }
        break;
    default:
        {
            ui->settingsStack->setCurrentWidget(ui->homePage);
        }
    }
}

ChooseConnectionDlg::~ChooseConnectionDlg()
{
    delete ui;
}

void ChooseConnectionDlg::on_actionCreateSerialPort_triggered()
{
    SerialPort * port = sConMgr2.createSerialPort();
    port->setName(tr("New Serial Port"));
    port->setDeviceName("COM1");
    QListWidgetItem * item = m_connectionItemMap[port];
    item->setSelected(true);
    ui->connectionsList->scrollToItem(item);
    ui->connectionNameEdit->setFocus();
}

void ChooseConnectionDlg::on_actionCreateTcpClient_triggered()
{
    TcpSocket * port = sConMgr2.createTcpSocket();
    port->setName(tr("New TCP client"));
    port->setHost("localhost");
    port->setPort(80);
    QListWidgetItem * item = m_connectionItemMap[port];
    item->setSelected(true);
    ui->connectionsList->scrollToItem(item);
}

void ChooseConnectionDlg::on_actionRemoveConnection_triggered()
{
    QList<QListWidgetItem *> selected = ui->connectionsList->selectedItems();
    Q_ASSERT(selected.size() <= 1);

    for (int i = 0; i < selected.size(); ++i)
        delete selected[i]->data(Qt::UserRole).value<Connection *>();
}

void ChooseConnectionDlg::on_connectionNameEdit_textChanged(const QString &arg1)
{
    if (!m_current)
        return;
    m_current->setName(arg1);
}

void ChooseConnectionDlg::on_connectionsList_itemSelectionChanged()
{
    QList<QListWidgetItem *> selected = ui->connectionsList->selectedItems();
    Q_ASSERT(selected.size() <= 1);

    m_current = 0;

    if (selected.empty())
    {
        ui->settingsStack->setCurrentWidget(ui->homePage);
        ui->actionRemoveConnection->setEnabled(false);
        ui->okBtn->setEnabled(false);
        ui->connectionNameEdit->setText(QString());
        ui->connectionNameEdit->setEnabled(false);
        return;
    }

    QListWidgetItem * item = selected[0];
    Connection * conn = item->data(Qt::UserRole).value<Connection *>();

    ui->okBtn->setEnabled(true);
    ui->connectionNameEdit->setEnabled(true);

    this->updateDetailsUi(conn);

    m_current = conn;
}

void ChooseConnectionDlg::on_spDeviceNameEdit_textChanged(const QString &arg1)
{
    if (!m_current)
        return;
    Q_ASSERT(m_current->getType() == CONNECTION_SERIAL_PORT);
    static_cast<SerialPort *>(m_current)->setDeviceName(arg1);
}

void ChooseConnectionDlg::on_connectionsList_doubleClicked(const QModelIndex &index)
{
    if (index.isValid())
        this->accept();
}

void ChooseConnectionDlg::on_spBaudRateEdit_editTextChanged(const QString &arg1)
{
    if (!m_current)
        return;
    Q_ASSERT(m_current->getType() == CONNECTION_SERIAL_PORT);

    bool ok;
    int editValue = arg1.toInt(&ok);
    if (!ok)
        return;

    static_cast<SerialPort *>(m_current)->setBaudRate(BaudRateType(editValue));
}

void ChooseConnectionDlg::on_tcHostEdit_textChanged(const QString &arg1)
{
    if (!m_current)
        return;
    Q_ASSERT(m_current->getType() == CONNECTION_TCP_SOCKET);
    static_cast<TcpSocket *>(m_current)->setHost(arg1);
}

void ChooseConnectionDlg::on_tcPortEdit_valueChanged(int arg1)
{
    if (!m_current)
        return;
    Q_ASSERT(m_current->getType() == CONNECTION_TCP_SOCKET);
    static_cast<TcpSocket *>(m_current)->setPort(arg1);
}
