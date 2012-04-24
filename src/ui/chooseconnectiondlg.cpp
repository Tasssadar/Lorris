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

#include "chooseconnectiondlg.h"
#include "ui_chooseconnectiondlg.h"
#include "../connection/connectionmgr2.h"
#include "../connection/serialport.h"
#include "../connection/tcpsocket.h"
#include <QMenu>
#include <QPushButton>
#include <QStyledItemDelegate>
#include <QPainter>

namespace {

class ConnectionListItemDelegate
        : public QStyledItemDelegate
{
public:
    QSize sizeHint(const QStyleOptionViewItem & option, const QModelIndex & index ) const
    {
        QStyleOptionViewItemV4 const & opt = static_cast<QStyleOptionViewItemV4 const &>(option);

        QSize res;

        QStyle *style = opt.widget ? opt.widget->style() : QApplication::style();
        int vmargin = style->pixelMetric(QStyle::PM_FocusFrameVMargin, &opt, opt.widget);
        res.setHeight(2 * opt.fontMetrics.lineSpacing()+2*vmargin);

        int line1w = opt.fontMetrics.width(index.data(Qt::DisplayRole).toString());
        int line2w = opt.fontMetrics.width(index.data(Qt::UserRole+1).toString());

        int margin = style->pixelMetric(QStyle::PM_FocusFrameHMargin, &opt, opt.widget);
        res.setWidth(2*opt.fontMetrics.lineSpacing() + 3*margin + (std::max)(line1w, line2w));

        return res;
    }

    void paint(QPainter * painter, const QStyleOptionViewItem & option, const QModelIndex & index ) const
    {
        QStyleOptionViewItemV4 const & opt = static_cast<QStyleOptionViewItemV4 const &>(option);
        QStyle *style = opt.widget ? opt.widget->style() : QApplication::style();

        // background
        style->drawPrimitive(QStyle::PE_PanelItemViewItem, &opt, painter, opt.widget);

        int vmargin = style->pixelMetric(QStyle::PM_FocusFrameVMargin, &opt, opt.widget);
        int margin = style->pixelMetric(QStyle::PM_FocusFrameHMargin, &opt, opt.widget);

        QRect iconRect;
        iconRect.setLeft(opt.rect.left() + margin);
        iconRect.setTop(opt.rect.top() + vmargin);
        iconRect.setHeight(opt.rect.height());
        iconRect.setWidth(opt.rect.height());

        QIcon::Mode mode = QIcon::Normal;
        if (!(opt.state & QStyle::State_Enabled))
            mode = QIcon::Disabled;
        else if (opt.state & QStyle::State_Selected)
            mode = QIcon::Selected;
        QIcon::State state = opt.state & QStyle::State_Open ? QIcon::On : QIcon::Off;
        index.data(Qt::DecorationRole).value<QIcon>().paint(painter, iconRect, opt.decorationAlignment, mode, state);

        QPalette::ColorGroup cg = opt.state & QStyle::State_Enabled ? QPalette::Normal : QPalette::Disabled;
        if (cg == QPalette::Normal && !(opt.state & QStyle::State_Active))
            cg = QPalette::Inactive;
        QColor penColor;

        // FIXME: HighlightedText is white even on Vista, where the highlight shouldn't change
        // the text color. How should we handle this?
        /*if (opt.state & QStyle::State_Selected)
            penColor = opt.palette.color(cg, QPalette::HighlightedText);
        else*/
            penColor = opt.palette.color(cg, QPalette::Text);
        painter->setPen(penColor);

        QRect textRect = opt.rect;
        textRect.setLeft(iconRect.right() + 1 + margin);
        textRect.setTop(textRect.top() + vmargin);
        painter->drawText(textRect, Qt::AlignLeft | Qt::AlignTop, index.data(Qt::DisplayRole).toString());

        penColor.setAlpha(128);
        painter->setPen(penColor);

        textRect.setTop(textRect.top() + opt.fontMetrics.lineSpacing());
        painter->drawText(textRect, Qt::AlignLeft | Qt::AlignTop, index.data(Qt::UserRole+1).toString());
    }
};

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
    menu->addAction(ui->actionCreateTcpClient);
    ui->createConnectionBtn->setMenu(menu);

    sConMgr2.refresh();
    QList<Connection *> const & conns = sConMgr2.connections();
    for (int i = 0; i < conns.size(); ++i)
        this->connAdded(conns[i]);
    ui->connectionsList->sortItems();

    ui->connectionsList->setItemDelegate(new ConnectionListItemDelegate());

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

    // TODO: set icon based on the connection type
    item->setIcon(QIcon(":/icons/icons/network-wired.png"));

    item->setData(Qt::UserRole, QVariant::fromValue(conn));
    item->setData(Qt::UserRole+1, conn->details());
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
    item->setData(Qt::UserRole+1, conn->details());
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
        ui->confirmBox->button(QDialogButtonBox::Ok)->setEnabled(false);
        ui->connectionNameEdit->setText(QString());
        ui->connectionNameEdit->setEnabled(false);
        return;
    }

    QListWidgetItem * item = selected[0];
    Connection * conn = item->data(Qt::UserRole).value<Connection *>();

    ui->confirmBox->button(QDialogButtonBox::Ok)->setEnabled(true);
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
