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

#include "ui_lorrisproxy.h"

LorrisProxy::LorrisProxy()
    : ui(new Ui::LorrisProxy)
{
    m_server = new TcpServer();

    ui->setupUi(this);

    connect(ui->addressEdit,   SIGNAL(textChanged(QString)), SLOT(updateAddressText()));
    connect(ui->portBox,       SIGNAL(valueChanged(int)),    SLOT(updateAddressText()));
    connect(ui->listenButon,   SIGNAL(clicked()),            SLOT(listenChanged()));
    connect(m_server,          SIGNAL(newConnection(QTcpSocket*,quint32)), SLOT(addConnection(QTcpSocket*,quint32)));
    connect(m_server,          SIGNAL(removeConnection(quint32)), SLOT(removeConnection(quint32)));

    ui->addressEdit->setText(sConfig.get(CFG_STRING_PROXY_ADDR));
    ui->portBox->setValue(sConfig.get(CFG_QUINT32_PROXY_PORT));

    m_connectButton = new ConnectButton(ui->connectButton);
    connect(m_connectButton, SIGNAL(connectionChosen(ConnectionPointer<Connection>)), this, SLOT(setConnection(ConnectionPointer<Connection>)));
}

LorrisProxy::~LorrisProxy()
{
    delete m_server;
    delete ui;
}

void LorrisProxy::setPortConnection(ConnectionPointer<PortConnection> const & con)
{
    this->PortConnWorkTab::setPortConnection(con);
    m_connectButton->setConn(con);
    connect(m_con.data(),    SIGNAL(dataRead(QByteArray)), m_server, SLOT(SendData(QByteArray)));
    connect(m_server, SIGNAL(newData(QByteArray)),  m_con.data(),    SLOT(SendData(QByteArray)));
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
        }
        else
        {
            QMessageBox box(this);
            box.setIcon(QMessageBox::Critical);
            box.setWindowTitle(tr("Error!"));
            box.setText(tr("Failed to start listening (%1)!").arg(m_server->getLastErr()));
            box.exec();
        }
    }
    updateAddressText();
}

void LorrisProxy::addConnection(QTcpSocket *connection, quint32 id)
{
    QTreeWidgetItem *item = new QTreeWidgetItem(ui->connections);
    item->setText(0, QString::number(id));
    item->setText(1, connection->peerAddress().toString());
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
    if (!m_con)
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
}
