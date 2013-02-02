/**********************************************
**    This file is part of Lorris
**    http://tasssadar.github.com/Lorris/
**
**    See README and COPYING
***********************************************/

#include "chooseconnectiondlg.h"
#include "ui_chooseconnectiondlg.h"
#include "../connection/connectionmgr2.h"
#include "../connection/serialport.h"
#include "../connection/tcpsocket.h"
#include <QMenu>
#include <QPushButton>
#include <QStyledItemDelegate>
#include <QPainter>
#include <QSignalMapper>

#ifdef Q_WS_WIN
#include <QWindowsVistaStyle>
#endif

static QString connectionStateString(ConnectionState state)
{
    switch (state)
    {
    case st_connecting:
        return QObject::tr("(Connecting...)");
    case st_connected:
        return QObject::tr("(Connected)");
    case st_connect_pending:
        return QObject::tr("(Pending)");
    default:
        return QString();
    }
}

namespace {

class ConnectionListItemDelegate
        : public QStyledItemDelegate
{
public:
    explicit ConnectionListItemDelegate(QObject * parent = 0)
        : QStyledItemDelegate(parent)
    {
    }

    QSize sizeHint(const QStyleOptionViewItem & option, const QModelIndex & index ) const
    {
        Q_ASSERT(index.isValid());
        Connection * conn = index.data(Qt::UserRole).value<Connection *>();

        QStyleOptionViewItemV4 const & opt = static_cast<QStyleOptionViewItemV4 const &>(option);

        QSize res;

        QStyle *style = opt.widget ? opt.widget->style() : QApplication::style();
        int vmargin = style->pixelMetric(QStyle::PM_FocusFrameVMargin, &opt, opt.widget);
        res.setHeight(2 * opt.fontMetrics.lineSpacing()+2*vmargin);

        int namew = opt.fontMetrics.width(index.data(Qt::DisplayRole).toString());
        int statew = opt.fontMetrics.width(connectionStateString(conn->state()));

        int margin = style->pixelMetric(QStyle::PM_FocusFrameHMargin, &opt, opt.widget);
        int line1w = namew + margin + statew;
        int line2w = opt.fontMetrics.width(conn->details());

        res.setWidth(2*opt.fontMetrics.lineSpacing() + 3*margin + (std::max)(line1w, line2w));

        return res;
    }

    void paint(QPainter * painter, const QStyleOptionViewItem & option, const QModelIndex & index ) const
    {
        Q_ASSERT(index.isValid());
        Connection * conn = index.data(Qt::UserRole).value<Connection *>();

        QStyleOptionViewItemV4 const & opt = static_cast<QStyleOptionViewItemV4 const &>(option);
        QStyle *style = opt.widget ? opt.widget->style() : QApplication::style();

        // background
        style->drawPrimitive(QStyle::PE_PanelItemViewItem, &opt, painter, opt.widget);

        int vmargin = style->pixelMetric(QStyle::PM_FocusFrameVMargin, &opt, opt.widget);
        int margin = style->pixelMetric(QStyle::PM_FocusFrameHMargin, &opt, opt.widget);

        QRect iconRect;
        iconRect.setLeft(opt.rect.left() + margin);
        iconRect.setTop(opt.rect.top() + vmargin);
        iconRect.setHeight(opt.rect.height() - 2*vmargin);
        iconRect.setWidth(opt.rect.height() - 2*vmargin);

        QIcon::Mode mode = QIcon::Normal;
        if (!(opt.state & QStyle::State_Enabled))
            mode = QIcon::Disabled;
        else if (opt.state & QStyle::State_Selected)
            mode = QIcon::Selected;
        QIcon::State state = opt.state & QStyle::State_Open ? QIcon::On : QIcon::Off;

        painter->setOpacity(conn->isMissing()? 0.5: 1);
        index.data(Qt::DecorationRole).value<QIcon>().paint(painter, iconRect, opt.decorationAlignment, mode, state);

        QPalette::ColorGroup cg = opt.state & QStyle::State_Enabled ? QPalette::Normal : QPalette::Disabled;
        if (cg == QPalette::Normal && !(opt.state & QStyle::State_Active))
            cg = QPalette::Inactive;

        QColor textColor;
        if (opt.state & QStyle::State_Selected)
        {
#ifdef Q_WS_WIN
            if (dynamic_cast<QWindowsVistaStyle *>(style))
                textColor = opt.palette.color(QPalette::Active, QPalette::Text);
            else
#endif
                textColor = opt.palette.color(cg, QPalette::HighlightedText);
        }
        else
        {
            textColor = opt.palette.color(cg, QPalette::Text);
        }
        painter->setPen(textColor);

        QRect textRect = opt.rect;
        textRect.setLeft(iconRect.right() + 1 + margin);
        textRect.setTop(textRect.top() + vmargin);

        QRect nameBr;
        painter->drawText(textRect, Qt::AlignLeft | Qt::AlignTop, index.data(Qt::DisplayRole).toString(), &nameBr);

        textColor.setAlpha(128);
        painter->setPen(textColor);

        textRect.setLeft(nameBr.right() + 1 + margin);
        painter->drawText(textRect, Qt::AlignLeft | Qt::AlignTop, connectionStateString(conn->state()));

        textRect.setLeft(iconRect.right() + 1 + margin);
        textRect.setTop(textRect.top() + opt.fontMetrics.lineSpacing());
        painter->drawText(textRect, Qt::AlignLeft | Qt::AlignTop, conn->details());
    }
};

}

ChooseConnectionDlg::ChooseConnectionDlg(QWidget *parent) :
    QDialog(parent),
    ui(new Ui::ChooseConnectionDlg),
    m_allowedConns(0)
{
    ui->setupUi(this);
    this->setWindowFlags(this->windowFlags() & ~Qt::WindowContextHelpButtonHint);

    ui->removeConnectionBtn->setDefaultAction(ui->actionRemoveConnection);
    ui->spBaudRateEdit->setValidator(new QIntValidator(1, INT_MAX, this));

    QMenu * menu = new QMenu(this);
    menu->addAction(ui->actionCreateSerialPort);
    menu->addAction(ui->actionCreateTcpClient);
    menu->addAction(ui->actionCreateUsbAcmConn);
    ui->createConnectionBtn->setMenu(menu);

    sConMgr2.refresh();
    QList<Connection *> const & conns = sConMgr2.connections();
    for (int i = 0; i < conns.size(); ++i)
        this->connAdded(conns[i]);
    ui->connectionsList->sortItems();

    ui->connectionsList->insertAction(0, ui->actionConnect);
    ui->connectionsList->insertAction(0, ui->actionDisconnect);
    ui->connectionsList->insertAction(0, ui->actionClone);

    ui->connectionsList->setItemDelegate(new ConnectionListItemDelegate(this));

    m_prog_btns[programmer_flip] = NULL; // can't be selected
    m_prog_btns[programmer_shupito] = ui->progShupito;
    m_prog_btns[programmer_avr232boot] = ui->progAVR232;

    QSignalMapper *map = new QSignalMapper(this);
    for(int i = 0; i < programmer_max; ++i)
    {
        if(!m_prog_btns[i])
            continue;
        map->setMapping(m_prog_btns[i], i);
        connect(m_prog_btns[i], SIGNAL(clicked()), map, SLOT(map()));
    }

    connect(&sConMgr2, SIGNAL(connAdded(Connection *)),   this, SLOT(connAdded(Connection *)));
    connect(&sConMgr2, SIGNAL(connRemoved(Connection *)), this, SLOT(connRemoved(Connection *)));
    connect(map,       SIGNAL(mapped(int)),               this, SLOT(progBtn_clicked(int)));

    this->on_connectionsList_itemSelectionChanged();
}

ConnectionPointer<PortConnection> ChooseConnectionDlg::choosePort(ConnectionPointer<Connection> const & preselectedConn)
{
    return this->choose(pct_port, preselectedConn).dynamicCast<PortConnection>();
}

ConnectionPointer<ShupitoConnection> ChooseConnectionDlg::chooseShupito(ConnectionPointer<Connection> const & preselectedConn)
{
    return this->choose(pct_shupito, preselectedConn).dynamicCast<ShupitoConnection>();
}

ConnectionPointer<Connection> ChooseConnectionDlg::choose(PrimaryConnectionTypes allowedConns, ConnectionPointer<Connection> const & preselectedConn)
{
    m_allowedConns = allowedConns;

    this->selectConn(preselectedConn.data());
    if (this->exec() != QDialog::Accepted)
        return ConnectionPointer<Connection>();

    if (PortConnection * pc = dynamic_cast<PortConnection *>(m_current.data()))
    {
        if (pc->programmerType() == programmer_shupito)
        {
           ConnectionPointer<ShupitoConnection> sc = sConMgr2.createAutoShupito(pc);
           m_current = sc;
        }
    }

    return m_current;
}

void ChooseConnectionDlg::selectConn(Connection * conn)
{
    // Note that the preselected connection may be handled by a different manager
    // and as such may be missing in the map.
    if (m_connectionItemMap.contains(conn))
        m_connectionItemMap[conn]->setSelected(true);
}

void ChooseConnectionDlg::connAdded(Connection * conn)
{
    QListWidgetItem * item = new QListWidgetItem(conn->name(), ui->connectionsList);

    switch (conn->getType())
    {
    case CONNECTION_LIBYB_USB:
    case CONNECTION_USB_ACM2:
    case CONNECTION_SHUPITO23:
    case CONNECTION_USB_SHUPITO:
        item->setIcon(QIcon(":/icons/icons/usb-conn.png"));
        break;
    default:
        item->setIcon(QIcon(":/icons/icons/network-wired.png"));
    }

    item->setData(Qt::UserRole, QVariant::fromValue(conn));
    item->setData(Qt::UserRole+1, conn->details());
    item->setData(Qt::UserRole+2, (int)conn->state());

    m_connectionItemMap[conn] = item;
    connect(conn, SIGNAL(changed()), this, SLOT(connChanged()));
    connect(conn, SIGNAL(stateChanged(ConnectionState)), this, SLOT(connChanged()));
}

void ChooseConnectionDlg::connRemoved(Connection * conn)
{
    conn->disconnect(this);
    QListWidgetItem * item = m_connectionItemMap.take(conn);
    delete item;
}

void ChooseConnectionDlg::connChanged()
{
    Connection * conn = static_cast<Connection *>(this->sender());
    QListWidgetItem * item = m_connectionItemMap[conn];
    item->setText(conn->name());
    item->setData(Qt::UserRole+1, conn->details());
    item->setData(Qt::UserRole+2, (int)conn->state());
    if (conn == m_current.data())
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
    ui->actionConnect->setEnabled(conn->state() == st_disconnected);
    ui->actionDisconnect->setEnabled(conn->state() == st_connected || conn->state() == st_connect_pending);

    switch (conn->getType())
    {
    case CONNECTION_SERIAL_PORT:
        {
            SerialPort * sp = static_cast<SerialPort *>(conn);
            ui->settingsStack->setCurrentWidget(ui->serialPortPage);
            updateEditText(ui->spBaudRateEdit->lineEdit(), QString::number((int)sp->baudRate()));
            updateEditText(ui->spDeviceNameEdit, sp->deviceName());
            ui->spDeviceNameEdit->setEnabled(sp->devNameEditable());
            ui->programmerSelection->setVisible(m_allowedConns & pct_port_programmable);
            setActiveProgBtn(sp->programmerType());
        }
        break;
    case CONNECTION_TCP_SOCKET:
        {
            TcpSocket * tc = static_cast<TcpSocket *>(conn);
            ui->settingsStack->setCurrentWidget(ui->tcpClientPage);
            updateEditText(ui->tcHostEdit, tc->host());
            ui->tcPortEdit->setValue(tc->port());
            ui->programmerSelection->setVisible(m_allowedConns & pct_port_programmable);
            setActiveProgBtn(tc->programmerType());
        }
        break;
    case CONNECTION_USB_ACM2:
        {
            UsbAcmConnection2 * c = static_cast<UsbAcmConnection2 *>(conn);
            ui->settingsStack->setCurrentWidget(ui->usbAcmConnPage);
            ui->usbBaudRateEdit->setEditText(QString::number((int)c->baudRate()));
            updateEditText(ui->usbVidEdit, QString("%1").arg(c->vid(), 4, 16, QChar('0')));
            updateEditText(ui->usbPidEdit, QString("%1").arg(c->pid(), 4, 16, QChar('0')));
            updateEditText(ui->usbAcmSnEdit, c->serialNumber());
            updateEditText(ui->usbIntfNameEdit, c->intfName());

            bool editable = !c->enumerated() && (c->state() == st_disconnected || c->state() == st_missing);
            ui->usbVidEdit->setEnabled(editable);
            ui->usbPidEdit->setEnabled(editable);
            ui->usbAcmSnEdit->setEnabled(editable);
            ui->usbIntfNameEdit->setEnabled(editable);
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

void ChooseConnectionDlg::focusNewConn(Connection * conn)
{
    QListWidgetItem * item = m_connectionItemMap[conn];
    item->setSelected(true);
    ui->connectionsList->scrollToItem(item);
}

void ChooseConnectionDlg::setActiveProgBtn(int type)
{
    for(int i = 0; i < programmer_max; ++i)
        if(m_prog_btns[i])
            m_prog_btns[i]->setChecked(type == i);
}

void ChooseConnectionDlg::on_actionCreateSerialPort_triggered()
{
    SerialPort * port = sConMgr2.createSerialPort();
    port->setName(tr("New Serial Port"));
    port->setDeviceName("COM1");
    this->focusNewConn(port);
    ui->connectionNameEdit->setFocus();
}

void ChooseConnectionDlg::on_actionCreateTcpClient_triggered()
{
    TcpSocket * port = sConMgr2.createTcpSocket();
    port->setName(tr("New TCP client"));
    port->setHost("localhost");
    port->setPort(80);
    this->focusNewConn(port);
}

void ChooseConnectionDlg::on_actionCreateUsbAcmConn_triggered()
{
    UsbAcmConnection2 * conn = sConMgr2.createUsbAcmConn();
    conn->setName(tr("New USB connection"));
    this->focusNewConn(conn);
}

void ChooseConnectionDlg::on_actionRemoveConnection_triggered()
{
    QList<QListWidgetItem *> selected = ui->connectionsList->selectedItems();
    Q_ASSERT(selected.size() <= 1);

    for (int i = 0; i < selected.size(); ++i)
        selected[i]->data(Qt::UserRole).value<Connection *>()->releaseAll();
}

void ChooseConnectionDlg::on_connectionNameEdit_textChanged(const QString &arg1)
{
    if (m_current)
        m_current->setName(arg1);
}

void ChooseConnectionDlg::on_connectionsList_itemSelectionChanged()
{
    QList<QListWidgetItem *> selected = ui->connectionsList->selectedItems();
    Q_ASSERT(selected.size() <= 1);

    m_current.reset();

    if (selected.empty())
    {
        ui->actionConnect->setEnabled(false);
        ui->actionDisconnect->setEnabled(false);

        ui->settingsStack->setCurrentWidget(ui->homePage);
        ui->actionRemoveConnection->setEnabled(false);
        ui->confirmBox->button(QDialogButtonBox::Ok)->setEnabled(false);
        ui->connectionNameEdit->setText(QString());
        ui->connectionNameEdit->setEnabled(false);
        return;
    }

    QListWidgetItem * item = selected[0];
    Connection * conn = item->data(Qt::UserRole).value<Connection *>();
    Q_ASSERT(conn);

    bool enabled = ((m_allowedConns & pct_port) && dynamic_cast<PortConnection *>(conn))
            || ((m_allowedConns & pct_shupito) && dynamic_cast<ShupitoConnection *>(conn));

#ifdef HAVE_LIBYB
    if (!enabled && (m_allowedConns & pct_flip))
    {
        if (GenericUsbConnection * uc = dynamic_cast<GenericUsbConnection *>(conn))
            enabled = uc->isFlipDevice();
    }
#endif

    ui->confirmBox->button(QDialogButtonBox::Ok)->setEnabled(enabled);

    ui->connectionNameEdit->setEnabled(true);
    ui->actionClone->setEnabled(conn->clonable());

    this->updateDetailsUi(conn);

    m_current.reset(conn);
    m_current->addRef();
}

void ChooseConnectionDlg::on_spDeviceNameEdit_textChanged(const QString &arg1)
{
    if (!m_current)
        return;
    Q_ASSERT(m_current->getType() == CONNECTION_SERIAL_PORT);
    static_cast<SerialPort *>(m_current.data())->setDeviceName(arg1);
}

void ChooseConnectionDlg::on_connectionsList_doubleClicked(const QModelIndex &index)
{
    if (index.isValid() && ui->confirmBox->button(QDialogButtonBox::Ok)->isEnabled())
        this->accept();
}

void ChooseConnectionDlg::on_spBaudRateEdit_editTextChanged(const QString &arg1)
{
    if (!m_current)
        return;

    bool ok;
    int editValue = arg1.toInt(&ok);
    if (!ok)
        return;

    Q_ASSERT(m_current->getType() == CONNECTION_SERIAL_PORT);
    if (SerialPort * c = dynamic_cast<SerialPort *>(m_current.data()))
        c->setBaudRate(editValue);
}

void ChooseConnectionDlg::progBtn_clicked(int programmer)
{
    setActiveProgBtn(programmer);
    if (SerialPort * c = dynamic_cast<SerialPort *>(m_current.data()))
        c->setProgrammerType(programmer);
}

void ChooseConnectionDlg::on_tcHostEdit_textChanged(const QString &arg1)
{
    if (!m_current)
        return;
    Q_ASSERT(m_current->getType() == CONNECTION_TCP_SOCKET);
    static_cast<TcpSocket *>(m_current.data())->setHost(arg1);
}

void ChooseConnectionDlg::on_tcPortEdit_valueChanged(int arg1)
{
    if (!m_current)
        return;
    Q_ASSERT(m_current->getType() == CONNECTION_TCP_SOCKET);
    static_cast<TcpSocket *>(m_current.data())->setPort(arg1);
}

void ChooseConnectionDlg::on_usbVidEdit_textChanged(QString const & value)
{
    if (!m_current)
        return;
    Q_ASSERT(dynamic_cast<UsbAcmConnection2 *>(m_current.data()) != 0);

    bool ok;
    int n = value.toInt(&ok, 16);
    if (ok)
        static_cast<UsbAcmConnection2 *>(m_current.data())->setVid(n);
}

void ChooseConnectionDlg::on_usbPidEdit_textChanged(QString const & value)
{
    if (!m_current)
        return;
    Q_ASSERT(dynamic_cast<UsbAcmConnection2 *>(m_current.data()) != 0);

    bool ok;
    int n = value.toInt(&ok, 16);
    if (ok)
        static_cast<UsbAcmConnection2 *>(m_current.data())->setPid(n);
}

void ChooseConnectionDlg::on_usbAcmSnEdit_textChanged(QString const & value)
{
    if (!m_current)
        return;
    Q_ASSERT(dynamic_cast<UsbAcmConnection2 *>(m_current.data()) != 0);
    static_cast<UsbAcmConnection2 *>(m_current.data())->setSerialNumber(value);
}

void ChooseConnectionDlg::on_usbIntfNameEdit_textChanged(QString const & value)
{
    if (!m_current)
        return;
    Q_ASSERT(dynamic_cast<UsbAcmConnection2 *>(m_current.data()) != 0);
    static_cast<UsbAcmConnection2 *>(m_current.data())->setIntfName(value);
}

void ChooseConnectionDlg::on_usbBaudRateEdit_textChanged(QString const & value)
{
    if (!m_current)
        return;
    Q_ASSERT(dynamic_cast<UsbAcmConnection2 *>(m_current.data()) != 0);

    bool ok;
    uint baud_rate = value.toUInt(&ok);
    if (ok)
        static_cast<UsbAcmConnection2 *>(m_current.data())->setBaudRate(baud_rate);
}

void ChooseConnectionDlg::on_actionConnect_triggered()
{
    if (!m_current)
        return;
    m_current->OpenConcurrent();
}

void ChooseConnectionDlg::on_actionDisconnect_triggered()
{
    if (!m_current)
        return;
    m_current->Close();
}

void ChooseConnectionDlg::on_actionClone_triggered()
{
    if (!m_current)
        return;

    ConnectionPointer<Connection> new_conn(m_current->clone());
    sConMgr2.addUserOwnedConn(new_conn.data());
    this->focusNewConn(new_conn.take());
}
