/**********************************************
**    This file is part of Lorris
**    http://tasssadar.github.com/Lorris/
**
**    See README and COPYING
***********************************************/

#include <QDebug>
#include <QApplication>
#include <QIntValidator>

#include "shupitospitunnel.h"
#include "../shupito.h"
#include "../../connection/connectionmgr2.h"
#include "../../misc/config.h"

ShupitoSpiTunnel::ShupitoSpiTunnel(Shupito *shupito) : ShupitoMode(shupito)
{
    m_tunnel_speed = sConfig.get(CFG_QUINT32_SPI_TUNNEL_SPEED);

    int m = sConfig.get(CFG_QUINT32_SPI_TUNNEL_MODES);
    m_sample_mode = (m & 0xFF);
    m_ss_mode = (m >> 8) & 0xFF;

    m_lsb_first = sConfig.get(CFG_BOOL_SPI_TUNNEL_LSB_FIRST);
}

ShupitoSpiTunnel::~ShupitoSpiTunnel()
{
    if(m_tunnel_conn)
        m_tunnel_conn->setShupito(NULL);
}

chip_definition ShupitoSpiTunnel::readDeviceId()
{
    return chip_definition();
}

void ShupitoSpiTunnel::erase_device(chip_definition& /*chip*/)
{

}

void ShupitoSpiTunnel::flashPage(chip_definition::memorydef */*memdef*/, std::vector<quint8>& /*memory*/, quint32 /*address*/)
{

}

void ShupitoSpiTunnel::readMemRange(quint8 /*memid*/, QByteArray &/*memory*/, quint32 /*address*/, quint32 /*size*/)
{

}

ShupitoDesc::config const *ShupitoSpiTunnel::getModeCfg()
{
    return m_shupito->getDesc()->getConfig("633125ab-32e0-49ec-b240-7d845bb70b2d");
}

ProgrammerCapabilities ShupitoSpiTunnel::capabilities() const
{
    ProgrammerCapabilities cap;
    cap.widgets = 1;
    return cap;
}

QList<QWidget*> ShupitoSpiTunnel::widgets()
{
    ShupitoSpiTunnelWidget *w = new ShupitoSpiTunnelWidget(m_tunnel_speed, m_sample_mode, m_ss_mode, m_lsb_first);
    connect(w, SIGNAL(applySettings(int,quint8,quint8,bool)), SLOT(applySettings(int,quint8,quint8,bool)));

    return (QList<QWidget*>() << w);
}

void ShupitoSpiTunnel::switchToFlashMode(quint32 speed_hz)
{
    m_flash_mode = false;

    if(!m_prepared)
        prepare();
    m_prepared = false;

    quint32 bsel = (quint32)((m_bsel_base + (speed_hz - 1)) / speed_hz);

    bsel = std::min(bsel, (quint32)m_bsel_max);
    bsel = std::max(bsel, (quint32)m_bsel_min);

    quint8 flags = (m_sample_mode & 3) | (m_lsb_first << 2);

    ShupitoPacket pkt = makeShupitoPacket(m_prog_cmd_base, 3, (quint8)bsel, (quint8)(bsel >> 8), flags);

    pkt = m_shupito->waitForPacket(pkt, m_prog_cmd_base);
    if(pkt.size() != 2)
        throw QString(QObject::tr("Failed to switch to the flash mode"));

    if (pkt[1] != 0)
        throw QString(QObject::tr("Failed to switch to the flash mode (error %1)")).arg((int)pkt[1]);

    m_prepared = true;
    m_flash_mode = true;
}

void ShupitoSpiTunnel::setActive(bool active)
{
    if(active && !m_tunnel_conn)
    {
        const QString name = ShupitoSpiTunnelConn::getCompanionName();
        ShupitoConnection *shupito_conn = m_shupito->getConn();
        qint64 id = shupito_conn->getCompanionId(name);
        Connection *c = sConMgr2.getCompanionConnection(shupito_conn, name);

        if(id && c && c->getType() == CONNECTION_SHUPITO_SPI_TUNNEL)
            m_tunnel_conn = ConnectionPointer<ShupitoSpiTunnelConn>::fromPtr((ShupitoSpiTunnelConn*)c);
        else
        {
            c = NULL;
            if(id == 0)
                id = sConMgr2.generateCompanionId();

            m_tunnel_conn.reset(new ShupitoSpiTunnelConn());
            m_tunnel_conn->setCompanionId(name, id);

            shupito_conn->setCompanionId(name, id);
        }

        // tunnel is already created by another programmer
        if(m_tunnel_conn->hasShupito())
        {
            m_tunnel_conn.reset(NULL);
            return;
        }

        m_tunnel_conn->setName("SPI Tunnel at " + shupito_conn->GetIDString());
        m_tunnel_conn->setRemovable(false);
        m_tunnel_conn->setShupito(m_shupito);

        if(!c)
            sConMgr2.addConnection(m_tunnel_conn.data());

        connect(m_tunnel_conn.data(), SIGNAL(stateChanged(ConnectionState)), SLOT(tunnelState(ConnectionState)));
        connect(m_tunnel_conn.data(), SIGNAL(writeSpiData(QByteArray)),      SLOT(writeSpiData(QByteArray)));
        connect(this, SIGNAL(spiStateSwitchComplete(bool)), m_tunnel_conn.data(), SLOT(spiStateSwitchComplete(bool)), Qt::QueuedConnection);
        connect(shupito_conn,         SIGNAL(packetRead(ShupitoPacket)),     SLOT(packetRead(ShupitoPacket)));

        if(m_tunnel_conn->isUsedByTab())
            m_tunnel_conn->OpenConcurrent();
    }
    else if(!active && m_tunnel_conn)
    {
        m_tunnel_conn->setShupito(NULL);

        m_tunnel_conn->disconnect(this);
        this->disconnect(m_tunnel_conn.data());
        m_shupito->getConn()->disconnect(this);

        m_tunnel_conn.reset(NULL);
    }
}

void ShupitoSpiTunnel::tunnelState(ConnectionState state)
{
    switch(state)
    {
        case st_connecting:
            try {
                switchToFlashMode(m_tunnel_speed);
                emit spiStateSwitchComplete(true);
            } catch(QString ex) {
                qWarning() << ex;
                emit spiStateSwitchComplete(false);
            }
            break;
        case st_disconnecting:
            try {
                switchToRunMode();
            } catch(QString ex) {
                qWarning() << ex;
            }
            emit spiStateSwitchComplete(true);
            break;
    }
}

void ShupitoSpiTunnel::writeSpiData(const QByteArray &data)
{
    if(data.isEmpty())
        return;

    ShupitoPacket p;
    p.reserve(data.size()+2);

    p.push_back(m_prog_cmd_base+2);
    p.push_back(m_ss_mode);
    p.insert(p.end(), data.data(), data.data()+data.size());

    m_shupito->sendPacket(p);
}

void ShupitoSpiTunnel::packetRead(const ShupitoPacket &p)
{
    if (p[0] == m_prog_cmd_base+2 && p.size() >= 2 &&
        m_tunnel_conn && m_tunnel_conn->isOpen())
    {
        if(p[1] != 0)
        {
            qWarning("Spi tunnel error: %u", p[1]);
            return;
        }

        if(p.size() > 2)
            m_tunnel_conn->spiDataRead(QByteArray::fromRawData((const char*)p.data()+2, p.size()-2));
    }
}

void ShupitoSpiTunnel::applySettings(int speed, quint8 sample_mode, quint8 ss_mode, bool lsb_first)
{
    m_tunnel_speed = speed;
    m_sample_mode = sample_mode;
    m_ss_mode = ss_mode;
    m_lsb_first = lsb_first;

    if(m_tunnel_conn && m_tunnel_conn->isOpen())
    {
        try {
            switchToRunMode();
            switchToFlashMode(speed);
        } catch(QString ex) {
            qWarning() << ex;
            m_tunnel_conn->Close();
        }
    }

    sConfig.set(CFG_QUINT32_SPI_TUNNEL_SPEED, speed);
    sConfig.set(CFG_QUINT32_SPI_TUNNEL_MODES, (sample_mode | (ss_mode << 8)));
    sConfig.set(CFG_BOOL_SPI_TUNNEL_LSB_FIRST, lsb_first);
}



ShupitoSpiTunnelWidget::ShupitoSpiTunnelWidget(int speed, quint8 sample_mode, quint8 ss_mode, bool lsb_first, QWidget *parent) :
    QWidget(parent), ui(new Ui::ShupitoSpiTunnelWidget)
{
    ui->setupUi(this);

    ui->tunnel_speed->setValidator(new QIntValidator(0, INT_MAX, this));
    ui->tunnel_speed->setEditText(QString::number(speed));

    if(sample_mode & SAMPLE_LEADING)
        ui->sample_leading->setChecked(true);
    else
        ui->sample_trailing->setChecked(true);

    if(!lsb_first)
        ui->big_endian->setChecked(true);
    else
        ui->little_endian->setChecked(true);

    if(ss_mode & SS_HIGH_ON_START)
        ui->ss_start_high->setChecked(true);
    else
        ui->ss_start_low->setChecked(true);

    if(ss_mode & SS_HIGH_ON_END)
        ui->ss_end_high->setChecked(true);
    else
        ui->ss_end_low->setChecked(true);

    connect(ui->tunnel_speed,   SIGNAL(editTextChanged(QString)), SLOT(enableApplyBtn()));
    connect(ui->big_endian,     SIGNAL(toggled(bool)),            SLOT(enableApplyBtn()));
    connect(ui->sample_leading, SIGNAL(toggled(bool)),            SLOT(enableApplyBtn()));
    connect(ui->ss_start_high,  SIGNAL(toggled(bool)),            SLOT(enableApplyBtn()));
    connect(ui->ss_end_high,    SIGNAL(toggled(bool)),            SLOT(enableApplyBtn()));
}

ShupitoSpiTunnelWidget::~ShupitoSpiTunnelWidget()
{
    delete ui;
}

void ShupitoSpiTunnelWidget::on_buttonBox_clicked(QAbstractButton *button)
{
    if(ui->buttonBox->buttonRole(button) != QDialogButtonBox::ApplyRole)
        return;

    int speed = ui->tunnel_speed->currentText().toInt();
    quint8 sample_mode = ui->sample_leading->isChecked() ? SAMPLE_LEADING : SAMPLE_TRAILING;

    quint8 ss_mode = 0;
    if(ui->ss_start_high->isChecked())
        ss_mode |= SS_HIGH_ON_START;
    if(ui->ss_end_high->isChecked())
        ss_mode |= SS_HIGH_ON_END;

    bool lsb_first = ui->little_endian->isChecked();

    emit applySettings(speed, sample_mode, ss_mode, lsb_first);
    ui->buttonBox->setEnabled(false);
}

void ShupitoSpiTunnelWidget::enableApplyBtn()
{
    ui->buttonBox->setEnabled(true);
}
