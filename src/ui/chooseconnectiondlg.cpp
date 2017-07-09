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
#include "../connection/shupitotunnel.h"
#include "../connection/udpsocket.h"
#include "../connection/proxytunnel.h"
#include "../misc/config.h"
#include <QMenu>
#include <QPushButton>
#include <QStyledItemDelegate>
#include <QPainter>
#include <QSignalMapper>

#if QT_VERSION < 0x050000 && defined(Q_OS_WIN)
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
    case st_disconnecting:
        return QObject::tr("(Disconnecting...)");
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
#if QT_VERSION < 0x050000 && defined(Q_OS_WIN)
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
    menu->addAction(ui->actionCreateUdpSocket);
#ifdef HAVE_LIBYB
    menu->addAction(ui->actionCreateUsbAcmConn);
#endif
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
    m_prog_btns[programmer_stm32] = NULL;
    m_prog_btns[programmer_shupito] = ui->progShupito;
    m_prog_btns[programmer_avr232boot] = ui->progAVR232;
    m_prog_btns[programmer_atsam] = ui->progAtsam;
    m_prog_btns[programmer_avr109] = ui->progAVR109;
    m_prog_btns[programmer_arduino] = ui->progArduino;
    m_prog_btns[programmer_zmodem] = ui->progZmodem;

    ui->programmerSelection->setVisible(false);

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
        if (pc->programmerType() == programmer_shupito && (m_allowedConns & pct_port_data) == 0)
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
    ConnectionListItem * item = new ConnectionListItem(conn->name(), ui->connectionsList);

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

static void updateComboText(QComboBox * w, QString const & value)
{
    if (w->currentText() != value)
    {
        int idx = w->findText(value);
        w->setCurrentIndex(idx);
        if (idx < 0)
            w->setEditText(value);
    }
}

static void updateComboIndex(QComboBox * w, int value)
{
    if (w->currentIndex() != value)
        w->setCurrentIndex(value);
}

void ChooseConnectionDlg::updateDetailsUi(Connection * conn)
{
    updateEditText(ui->connectionNameEdit, conn->name());
    ui->actionRemoveConnection->setEnabled(conn->removable());
    ui->actionConnect->setEnabled(conn->state() == st_disconnected);
    ui->actionDisconnect->setEnabled(conn->state() == st_connected || conn->state() == st_connect_pending || conn->state() == st_disconnecting);
    ui->programmerSelection->setVisible(false);
    ui->persistNameButton->setVisible(conn->isNamePersistable());
    ui->persistNameButton->setEnabled(!conn->hasDefaultName());
    ui->progArduino->setEnabled(conn->getType() == CONNECTION_SERIAL_PORT);

    switch (conn->getType())
    {
    case CONNECTION_SERIAL_PORT:
        {
            SerialPort * sp = static_cast<SerialPort *>(conn);
            ui->settingsStack->setCurrentWidget(ui->serialPortPage);
            updateEditText(ui->spBaudRateEdit->lineEdit(), QString::number((int)sp->baudRate()));
            updateEditText(ui->spDeviceNameEdit, sp->deviceName());
            updateComboIndex(ui->spParity, (int)sp->parity());
            updateComboIndex(ui->spStopBits, (int)sp->stopBits());
            updateComboText(ui->spDataBits, QString::number(sp->dataBits()));
            updateComboIndex(ui->spFlowControl, (int)sp->flowControl());
            ui->spRts->setChecked(sp->rts());
            ui->spRts->setEnabled(sp->flowControl() != FLOW_HARDWARE);
            ui->spDtr->setChecked(sp->dtr());
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
    case CONNECTION_UDP_SOCKET:
        {
            UdpSocket * tc = static_cast<UdpSocket *>(conn);
            ui->settingsStack->setCurrentWidget(ui->tcpClientPage);
            updateEditText(ui->tcHostEdit, tc->host());
            ui->tcPortEdit->setValue(tc->port());
            ui->programmerSelection->setVisible(m_allowedConns & pct_port_programmable);
            setActiveProgBtn(tc->programmerType());
        }
        break;
#ifdef HAVE_LIBYB
    case CONNECTION_USB_ACM2:
        {
            UsbAcmConnection2 * c = static_cast<UsbAcmConnection2 *>(conn);
            ui->settingsStack->setCurrentWidget(ui->usbAcmConnPage);

            updateComboText(ui->usbBaudRateEdit, QString::number((int)c->baudRate()));
            updateComboIndex(ui->usbParityCombo, (int)c->parity());
            updateComboIndex(ui->usbStopBitsCombo, (int)c->stopBits());
            updateComboText(ui->usbDataBitsCombo, QString::number(c->dataBits()));

            updateEditText(ui->usbVidEdit, QString("%1").arg(c->vid(), 4, 16, QChar('0')));
            updateEditText(ui->usbPidEdit, QString("%1").arg(c->pid(), 4, 16, QChar('0')));
            updateEditText(ui->usbAcmSnEdit, c->serialNumber());
            updateEditText(ui->usbIntfNameEdit, c->intfName());

            bool editable = !c->enumerated() && (c->state() == st_disconnected || c->state() == st_missing);
            ui->usbVidEdit->setEnabled(editable);
            ui->usbPidEdit->setEnabled(editable);
            ui->usbAcmSnEdit->setEnabled(editable);
            ui->usbIntfNameEdit->setEnabled(editable);

            ui->programmerSelection->setVisible(m_allowedConns & pct_port_programmable);
            setActiveProgBtn(c->programmerType());
        }
        break;
    case CONNECTION_USB_SHUPITO:
    case CONNECTION_SHUPITO23:
        {
            ShupitoConnection * c = static_cast<ShupitoConnection *>(conn);
            ShupitoFirmwareDetails fd;
            if (c->getFirmwareDetails(fd))
            {
                ui->shupito23HardwareLabel->setText(QString("%1.%2").arg(fd.hw_major).arg(fd.hw_minor));
                ui->shupito23FirmwareLabel->setText(fd.firmwareFilename());
                ui->settingsStack->setCurrentWidget(ui->shupito23Page);
            }
            else
            {
                ui->settingsStack->setCurrentWidget(ui->noSettingsPage);
            }
        }
        break;
#endif
    case CONNECTION_SHUPITO_TUNNEL:
    case CONNECTION_PROXY_TUNNEL:
        {
            PortConnection * st = static_cast<PortConnection *>(conn);
            ui->programmerSelection->setVisible(m_allowedConns & pct_port_programmable);
            setActiveProgBtn(st->programmerType());

            ui->settingsStack->setCurrentWidget(ui->noSettingsPage);
        }
        break;
    default:
        {
            ui->settingsStack->setCurrentWidget(ui->noSettingsPage);
        }
        break;
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

void ChooseConnectionDlg::on_actionCreateUdpSocket_triggered()
{
    UdpSocket * port = sConMgr2.createUdpSocket();
    port->setName(tr("New UDP socket"));
    port->setHost("localhost");
    port->setPort(80);
    this->focusNewConn(port);
}

void ChooseConnectionDlg::on_actionCreateUsbAcmConn_triggered()
{
#ifdef HAVE_LIBYB
    UsbAcmConnection2 * conn = sConMgr2.createUsbAcmConn();
    conn->setName(tr("New USB connection"));
    this->focusNewConn(conn);
#endif
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
        ui->persistNameButton->setVisible(false);
        return;
    }

    QListWidgetItem * item = selected[0];
    Connection * conn = item->data(Qt::UserRole).value<Connection *>();
    Q_ASSERT(conn);

    bool enabled = ((m_allowedConns & pct_port) && dynamic_cast<PortConnection *>(conn))
            || ((m_allowedConns & pct_shupito) && dynamic_cast<ShupitoConnection *>(conn));

#ifdef HAVE_LIBYB
    if (!enabled && (m_allowedConns & (pct_flip | pct_stm32link)))
    {
        if(dynamic_cast<STM32Connection *>(conn))
        {
            enabled = true;
        }
        else if (GenericUsbConnection * uc = dynamic_cast<GenericUsbConnection *>(conn))
        {
            enabled = uc->isFlipDevice();
        }
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
    {
        c->setBaudRate(editValue);
        sConfig.set(CFG_QUINT32_SERIAL_BAUD, editValue);
    }
}

void ChooseConnectionDlg::on_spParity_currentIndexChanged(int value)
{
    if (!m_current)
        return;
    Q_ASSERT(dynamic_cast<SerialPort *>(m_current.data()) != 0);
    static_cast<SerialPort *>(m_current.data())->setParity((ParityType)value);
}

void ChooseConnectionDlg::on_spStopBits_currentIndexChanged(int value)
{
    if (!m_current)
        return;
    Q_ASSERT(dynamic_cast<SerialPort *>(m_current.data()) != 0);
    static_cast<SerialPort *>(m_current.data())->setStopBits((StopBitsType)value);
}

void ChooseConnectionDlg::on_spDataBits_currentIndexChanged(int value)
{
    if (!m_current)
        return;
    Q_ASSERT(dynamic_cast<SerialPort *>(m_current.data()) != 0);
    static_cast<SerialPort *>(m_current.data())->setDataBits((DataBitsType)ui->spDataBits->itemText(value).toInt());
}

void ChooseConnectionDlg::on_spFlowControl_currentIndexChanged(int value)
{
    if(!m_current)
        return;
    Q_ASSERT(dynamic_cast<SerialPort *>(m_current.data()) != 0);
    static_cast<SerialPort *>(m_current.data())->setFlowControl((FlowType)value);
    ui->spRts->setEnabled(value != FLOW_HARDWARE);
}

void ChooseConnectionDlg::on_spRts_stateChanged(int state)
{
    if(!m_current)
        return;
    Q_ASSERT(dynamic_cast<SerialPort *>(m_current.data()) != 0);
    static_cast<SerialPort *>(m_current.data())->setRts(state == Qt::Checked);
}

void ChooseConnectionDlg::on_spDtr_stateChanged(int state)
{
    if(!m_current)
        return;
    Q_ASSERT(dynamic_cast<SerialPort *>(m_current.data()) != 0);
    static_cast<SerialPort *>(m_current.data())->setDtr(state == Qt::Checked);
}

void ChooseConnectionDlg::progBtn_clicked(int programmer)
{
    setActiveProgBtn(programmer);
    if (PortConnection * c = dynamic_cast<PortConnection *>(m_current.data()))
        c->setProgrammerType(programmer);
}

void ChooseConnectionDlg::on_tcHostEdit_textChanged(const QString &arg1)
{
    if (!m_current)
        return;

    switch(m_current->getType()) {
    case CONNECTION_TCP_SOCKET:
        static_cast<TcpSocket *>(m_current.data())->setHost(arg1);
        break;
    case CONNECTION_UDP_SOCKET:
        static_cast<UdpSocket *>(m_current.data())->setHost(arg1);
        break;
    default:
        Q_ASSERT(false);
    }
}

void ChooseConnectionDlg::on_tcPortEdit_valueChanged(int arg1)
{
    if (!m_current)
        return;

    switch(m_current->getType()) {
    case CONNECTION_TCP_SOCKET:
        static_cast<TcpSocket *>(m_current.data())->setPort(arg1);
        break;
    case CONNECTION_UDP_SOCKET:
        static_cast<UdpSocket *>(m_current.data())->setPort(arg1);
        break;
    default:
        Q_ASSERT(false);
    }
}

void ChooseConnectionDlg::on_usbVidEdit_textChanged(QString const & value)
{
#ifdef HAVE_LIBYB
    if (!m_current)
        return;
    Q_ASSERT(dynamic_cast<UsbAcmConnection2 *>(m_current.data()) != 0);

    bool ok;
    int n = value.toInt(&ok, 16);
    if (ok)
        static_cast<UsbAcmConnection2 *>(m_current.data())->setVid(n);
#endif
}

void ChooseConnectionDlg::on_usbPidEdit_textChanged(QString const & value)
{
#ifdef HAVE_LIBYB
    if (!m_current)
        return;
    Q_ASSERT(dynamic_cast<UsbAcmConnection2 *>(m_current.data()) != 0);

    bool ok;
    int n = value.toInt(&ok, 16);
    if (ok)
        static_cast<UsbAcmConnection2 *>(m_current.data())->setPid(n);
#endif
}

void ChooseConnectionDlg::on_usbAcmSnEdit_textChanged(QString const & value)
{
#ifdef HAVE_LIBYB
    if (!m_current)
        return;
    Q_ASSERT(dynamic_cast<UsbAcmConnection2 *>(m_current.data()) != 0);
    static_cast<UsbAcmConnection2 *>(m_current.data())->setSerialNumber(value);
#endif
}

void ChooseConnectionDlg::on_usbIntfNameEdit_textChanged(QString const & value)
{
#ifdef HAVE_LIBYB
    if (!m_current)
        return;
    Q_ASSERT(dynamic_cast<UsbAcmConnection2 *>(m_current.data()) != 0);
    static_cast<UsbAcmConnection2 *>(m_current.data())->setIntfName(value);
#endif
}

void ChooseConnectionDlg::on_usbBaudRateEdit_editTextChanged(QString const & value)
{
#ifdef HAVE_LIBYB
    if (!m_current)
        return;
    Q_ASSERT(dynamic_cast<UsbAcmConnection2 *>(m_current.data()) != 0);

    bool ok;
    uint baud_rate = value.toUInt(&ok);
    if (ok)
        static_cast<UsbAcmConnection2 *>(m_current.data())->setBaudRate(baud_rate);
#endif
}

void ChooseConnectionDlg::on_usbDataBitsCombo_currentIndexChanged(int value)
{
#ifdef HAVE_LIBYB
    if (!m_current)
        return;
    Q_ASSERT(dynamic_cast<UsbAcmConnection2 *>(m_current.data()) != 0);

    static_cast<UsbAcmConnection2 *>(m_current.data())->setDataBits(ui->usbDataBitsCombo->itemText(value).toInt());
#endif
}

void ChooseConnectionDlg::on_usbParityCombo_currentIndexChanged(int value)
{
#ifdef HAVE_LIBYB
    if (!m_current)
        return;
    Q_ASSERT(dynamic_cast<UsbAcmConnection2 *>(m_current.data()) != 0);

    static_cast<UsbAcmConnection2 *>(m_current.data())->setParity((UsbAcmConnection2::parity_t)value);
#endif
}

void ChooseConnectionDlg::on_usbStopBitsCombo_currentIndexChanged(int value)
{
#ifdef HAVE_LIBYB
    if (!m_current)
        return;
    Q_ASSERT(dynamic_cast<UsbAcmConnection2 *>(m_current.data()) != 0);

    static_cast<UsbAcmConnection2 *>(m_current.data())->setStopBits((UsbAcmConnection2::stop_bits_t)value);
#endif
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

void ChooseConnectionDlg::on_persistNameButton_clicked()
{
    Q_ASSERT(m_current && m_current->isNamePersistable());
    m_current->persistName();
}

ConnectionListItem::ConnectionListItem(const QString& text, QListWidget *parent, int type) :
    QListWidgetItem(text, parent, type)
{

}

bool ConnectionListItem::operator<(const QListWidgetItem& other) const
{
    const QString val1 = this->text();
    const QString val2 = other.text();

    int numStart = -1;

    int i;
    for(i = 0; i < val1.size() && i < val2.size(); ++i)
    {
        if(numStart == -1 && val1[i].isDigit() && val2[i].isDigit())
            numStart = i;
        else if(numStart != -1 && (!val1[i].isDigit() || !val2[i].isDigit()))
            numStart = -1;

        if(val1[i] != val2[i])
        {
            if(numStart == -1)
                return val1[i] < val2[i];

            int numLen1 = 0;
            int numLen2 = 0;
            for(int x = numStart; x < val1.size() && val1[x].isDigit(); ++x)
                ++numLen1;
            for(int x = numStart; x < val2.size() && val2[x].isDigit(); ++x)
                ++numLen2;

            const int num1 = val1.mid(numStart, numLen1).toInt();
            const int num2 = val2.mid(numStart, numLen2).toInt();
            return num1 < num2;
        }
    }
    return false;
}
