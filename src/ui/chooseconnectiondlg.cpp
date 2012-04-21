#include "chooseconnectiondlg.h"
#include "ui_chooseconnectiondlg.h"
#include "../connection/connectionmgr.h"
#include "../connection/serialport.h"
#include <QMenu>
#include <qextserialenumerator.h>

SerialPortEnumerator::SerialPortEnumerator()
{
    connect(&m_refreshTimer, SIGNAL(timeout()), this, SLOT(refresh()));
    m_refreshTimer.start(1000);
}

SerialPortEnumerator::~SerialPortEnumerator()
{
    std::set<SerialPort *> portsToClear;
    portsToClear.swap(m_ownedPorts);

    for (std::set<SerialPort *>::iterator it = portsToClear.begin(); it != portsToClear.end(); ++it)
        delete *it;
}

void SerialPortEnumerator::refresh()
{
    std::set<SerialPort *> portsToDisown = m_ownedPorts;

    QList<QextPortInfo> ports = QextSerialEnumerator::getPorts();
    for (int i = 0; i < ports.size(); ++i)
    {
        QextPortInfo info = ports[i];

        QHash<QString, SerialPort *>::const_iterator it = m_portMap.find(ports[i].physName);
        if (it == m_portMap.end())
        {
            QScopedPointer<SerialPort> portGuard(new SerialPort());
            portGuard->setName(info.portName);
            portGuard->setDeviceName(info.physName);
            portGuard->setBaudRate(BAUD38400);
            portGuard->setDevNameEditable(false);

            connect(portGuard.data(), SIGNAL(destroyed()), this, SLOT(connectionDestroyed()));
            m_portMap[info.physName] = portGuard.data();

            sConMgr2.addConnection(portGuard.data());

            m_ownedPorts.insert(portGuard.data());
            portGuard->setRemovable(false);
            portGuard.take();
        }
        else
        {
            SerialPort * port = it.value();

            if (m_ownedPorts.find(port) == m_ownedPorts.end())
            {
                m_ownedPorts.insert(port);
                port->setRemovable(false);
                port->addRef();
            }
            else
            {
                portsToDisown.erase(port);
            }
        }
    }

    for (std::set<SerialPort *>::const_iterator it = portsToDisown.begin(); it != portsToDisown.end(); ++it)
    {
        SerialPort * port = *it;
        m_ownedPorts.erase(port);
        port->setRemovable(true);
        port->release();
    }
}

void SerialPortEnumerator::connectionDestroyed()
{
    SerialPort * port = static_cast<SerialPort *>(this->sender());
    Q_ASSERT(m_ownedPorts.find(port) == m_ownedPorts.end());
    m_portMap.remove(port->deviceName());
}

ConnectionManager2::ConnectionManager2()
{
    m_serialPortEnumerator.reset(new SerialPortEnumerator());
}

ConnectionManager2::~ConnectionManager2()
{
    m_serialPortEnumerator.reset();

    // All of the remaining connections should be owned by the manager and should
    // therefore be removable.
    while (!m_conns.empty())
    {
        Q_ASSERT(m_conns.back()->removable());
        delete m_conns.back();
    }
}

void ConnectionManager2::refresh()
{
    m_serialPortEnumerator->refresh();
}

SerialPort * ConnectionManager2::createSerialPort()
{
    QScopedPointer<SerialPort> conn(new SerialPort());
    this->addConnection(conn.data());
    return conn.take();
}

void ConnectionManager2::addConnection(Connection * conn)
{
    connect(conn, SIGNAL(destroyed()), this, SLOT(connectionDestroyed()));
    m_conns.append(conn);
    emit connAdded(conn);
}

void ConnectionManager2::connectionDestroyed()
{
    Connection * conn = static_cast<Connection *>(this->sender());
    if (m_conns.removeOne(conn))
        emit connRemoved(conn);
}

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

void ChooseConnectionDlg::updateDetailsUi(Connection * conn)
{
    if (ui->connectionNameEdit->text() != conn->name())
        ui->connectionNameEdit->setText(conn->name());
    ui->actionRemoveConnection->setEnabled(conn->removable());

    switch (conn->getType())
    {
    case CONNECTION_SERIAL_PORT:
        {
            SerialPort * sp = static_cast<SerialPort *>(conn);
            ui->settingsStack->setCurrentWidget(ui->serialPortPage);
            ui->spBaudRateEdit->setEditText(QString::number((int)sp->baudRate()));
            ui->spDeviceNameEdit->setText(sp->deviceName());
            ui->spDeviceNameEdit->setEnabled(sp->devNameEditable());
        }
        break;
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
    m_connectionItemMap[port]->setSelected(true);
    ui->connectionNameEdit->setFocus();
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
