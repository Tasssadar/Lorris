/**********************************************
**    This file is part of Lorris
**    http://tasssadar.github.com/Lorris/
**
**    See README and COPYING
***********************************************/

#include <QMessageBox>
#include <QTcpSocket>
#include <QHostAddress>

#include "lorrisproxy.h"
#include "tcpserver.h"
#include "udpserver.h"

#include "ui_lorrisproxy.h"

#include "../connection/connectionmgr2.h"

LorrisProxy::LorrisProxy()
    : ui(new Ui::LorrisProxy), m_server(NULL)
{
    ui->setupUi(this);

    connect(ui->addressEdit,   SIGNAL(textChanged(QString)), SLOT(updateAddressText()));
    connect(ui->portBox,       SIGNAL(valueChanged(int)),    SLOT(updateAddressText()));
    connect(ui->listenButon,   SIGNAL(clicked()),            SLOT(listenChanged()));
    connect(ui->connections,   SIGNAL(customContextMenuRequested(QPoint)), SLOT(connectionMenu(QPoint)));
    connect(ui->tunnelName,    SIGNAL(editingFinished()),    SLOT(tunnelNameEditFinished()));
    connect(ui->tunnelName,    SIGNAL(textEdited(QString)),  SLOT(tunnelNameEdited(QString)));
    connect(ui->tunnelBox,     SIGNAL(toggled(bool)),        SLOT(tunnelToggled(bool)));
    connect(ui->tcpRadio,      SIGNAL(toggled(bool)),        SLOT(protocolToggled(bool)));

    protocolToggled(true);

    ui->addressEdit->setText(sConfig.get(CFG_STRING_PROXY_ADDR));
    ui->portBox->setValue(sConfig.get(CFG_QUINT32_PROXY_PORT));

    ui->tunnelName->setText(sConfig.get(CFG_STRING_PROXY_TUNNEL_NAME));
    ui->tunnelBox->setChecked(sConfig.get(CFG_BOOL_PROXY_TUNNEL));

    m_connectButton = new ConnectButton(ui->connectButton);
    connect(m_connectButton, SIGNAL(connectionChosen(ConnectionPointer<Connection>)), this, SLOT(setConnection(ConnectionPointer<Connection>)));
}

LorrisProxy::~LorrisProxy()
{
    if(m_tunnel_conn)
        m_tunnel_conn->setServer(NULL);

    delete ui;
}

void LorrisProxy::setPortConnection(ConnectionPointer<PortConnection> const & con)
{
    this->PortConnWorkTab::setPortConnection(con);
    m_connectButton->setConn(con, false);
    connect(m_con.data(),     SIGNAL(dataRead(QByteArray)), m_server, SLOT(SendData(QByteArray)));
    connect(m_server, SIGNAL(newData(QByteArray)),   m_con.data(),    SLOT(SendData(QByteArray)));
}

void LorrisProxy::updateAddressText()
{
    QString color = "color :";
    QString address;
    if(m_server->isListening())
    {
        color += "green";
        address = m_server->getAddress();
    }
    else
    {
        color += "red";
        address = ui->addressEdit->text();
    }

    QString text = address + ":" + ui->portBox->text();
    ui->addressLabel->setText(text);
    ui->addressLabel->setStyleSheet(color);
}

void LorrisProxy::listenChanged()
{
    if(m_server->isListening())
    {
        m_server->stopListening();
        ui->listenButon->setText(tr("Start listening"));
        ui->addressEdit->setEnabled(true);
        ui->portBox->setEnabled(true);
        ui->tcpRadio->setEnabled(true);
        ui->udpRadio->setEnabled(true);
        ui->connections->clear();
    }
    else
    {
        sConfig.set(CFG_STRING_PROXY_ADDR, ui->addressEdit->text());
        sConfig.set(CFG_QUINT32_PROXY_PORT, ui->portBox->value());

        if(m_server->listen(ui->addressEdit->text(), ui->portBox->value()))
        {
            ui->listenButon->setText(tr("Stop listening"));
            ui->addressEdit->setEnabled(false);
            ui->portBox->setEnabled(false);
            ui->tcpRadio->setEnabled(false);
            ui->udpRadio->setEnabled(false);
        }
        else
        {
            QMessageBox box(this);
            box.setIcon(QMessageBox::Critical);
            box.setWindowTitle(tr("Error!"));
            box.setText(tr("Failed to start listening (%1)!").arg(m_server->errorString()));
            box.exec();
        }
    }
    updateAddressText();
}

void LorrisProxy::addConnection(QString address, quint32 id)
{
    QTreeWidgetItem *item = new QTreeWidgetItem(ui->connections);
    item->setText(0, QString::number(id));
    item->setText(1, address);
}

void LorrisProxy::removeConnection(quint32 id)
{
    QList<QTreeWidgetItem *> items = ui->connections->findItems(QString::number(id), Qt::MatchExactly);
    if(items.empty())
        return;
    delete items[0];
}

void LorrisProxy::onTabShow(const QString&)
{
    if (!m_con && sConfig.get(CFG_BOOL_CONN_ON_NEW_TAB))
    {
        m_connectButton->choose();
        if (m_con && !m_con->isOpen())
            m_con->OpenConcurrent();
    }
}

QString LorrisProxy::GetIdString()
{
    return "LorrisProxy";
}

void LorrisProxy::saveData(DataFileParser *file)
{
    PortConnWorkTab::saveData(file);

    file->writeBlockIdentifier("LorrProxyAddr");
    file->writeString(ui->addressEdit->text());
    file->writeVal(ui->portBox->value());

    file->writeBlockIdentifier("LorrProxyStatus");
    file->writeVal(m_server->isListening());

    file->writeBlockIdentifier("LorrProxyTunnel");
    file->writeString(ui->tunnelName->text());
    file->writeVal(ui->tunnelBox->isChecked());

    file->writeBlockIdentifier("LorrProxyProtocol");
    file->writeVal(m_protocol);
}

void LorrisProxy::loadData(DataFileParser *file)
{
    PortConnWorkTab::loadData(file);

    if(file->seekToNextBlock("LorrProxyAddr", BLOCK_WORKTAB))
    {
        ui->addressEdit->setText(file->readString());
        ui->portBox->setValue(file->readVal<int>());
    }

    if(file->seekToNextBlock("LorrProxyStatus", BLOCK_WORKTAB))
        if(file->readVal<bool>())
            listenChanged();

    if(file->seekToNextBlock("LorrProxyTunnel", BLOCK_WORKTAB))
    {
        ui->tunnelName->setText(file->readString());
        ui->tunnelBox->setChecked(file->readVal<bool>());
        tunnelToggled(ui->tunnelBox->isChecked());
    }

    if(file->seekToNextBlock("LorrProxyProtocol", BLOCK_WORKTAB))
    {
        quint8 prot = file->readVal<quint8>();
        ui->tcpRadio->setChecked(prot == PROTOCOL_TCP);
        ui->udpRadio->setChecked(prot == PROTOCOL_UDP);
        protocolToggled(prot == PROTOCOL_TCP);
    }
}

void LorrisProxy::connectionMenu(const QPoint &pos)
{
    QTreeWidgetItem *item = ui->connections->itemAt(pos);
    if(!item)
        return;

    QMenu menu;
    QAction *act = menu.addAction(tr("Disconnect"));

    if(menu.exec(ui->connections->mapToGlobal(pos)) != act)
        return;

    m_server->closeConnection(item->text(0).toUInt());
}

void LorrisProxy::tunnelNameEditFinished()
{
    if(ui->tunnelName->text().isEmpty())
        ui->tunnelName->setText(tr("Proxy tunnel"));

    createProxyTunnel(ui->tunnelName->text());
    ui->setNameBtn->setEnabled(false);

    sConfig.set(CFG_STRING_PROXY_TUNNEL_NAME, ui->tunnelName->text());
}

void LorrisProxy::tunnelToggled(bool enable)
{
    ui->tunnelName->setEnabled(enable);
    ui->setNameBtn->setEnabled(false);

    if(!enable)
        destroyProxyTunnel();
    else
        tunnelNameEditFinished();

    sConfig.set(CFG_BOOL_PROXY_TUNNEL, enable);
}

void LorrisProxy::tunnelNameEdited(const QString &/*text*/)
{
    ui->setNameBtn->setEnabled(ui->tunnelBox->isChecked());
}

void LorrisProxy::protocolToggled(bool isTcp) {
    if(m_server && isTcp == (m_protocol == PROTOCOL_TCP))
        return;

    Server *server = NULL;
    if(isTcp)
    {
        server = new TcpServer(this);
        m_protocol = PROTOCOL_TCP;
    }
    else
    {
       server = new UdpServer(this);
        m_protocol = PROTOCOL_UDP;
    }

    if(m_tunnel_conn)
        m_tunnel_conn->setServer(server);

    delete m_server;
    m_server = server;

    connect(m_server,         SIGNAL(newConnection(QString,quint32)), SLOT(addConnection(QString,quint32)));
    connect(m_server,         SIGNAL(removeConnection(quint32)), SLOT(removeConnection(quint32)));
    if(m_con)
    {
        connect(m_con.data(),     SIGNAL(dataRead(QByteArray)), m_server, SLOT(SendData(QByteArray)));
        connect(m_server, SIGNAL(newData(QByteArray)),   m_con.data(),    SLOT(SendData(QByteArray)));
    }

    tunnelToggled(ui->tunnelBox->isChecked());
}

void LorrisProxy::createProxyTunnel(const QString &name)
{
    if(!m_tunnel_conn)
    {
        m_tunnel_conn.reset(new ProxyTunnel);
        m_tunnel_conn->setServer(m_server);
        m_tunnel_conn->setRemovable(false);
        sConMgr2.addConnection(m_tunnel_conn.data());
    }
    m_tunnel_conn->setName(name);
}

void LorrisProxy::destroyProxyTunnel()
{
    if(!m_tunnel_conn)
        return;

    m_tunnel_conn->setServer(NULL);
    m_tunnel_conn.reset();
}
