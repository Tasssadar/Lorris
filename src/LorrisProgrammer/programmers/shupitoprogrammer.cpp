#include "shupitoprogrammer.h"
#include <QStringBuilder>

ShupitoProgrammer::ShupitoProgrammer(ConnectionPointer<ShupitoConnection> const & conn, ProgrammerLogSink * logsink)
    : Programmer(logsink), m_con(conn), m_vdd_config(0), m_tunnel_config(0), m_btn_config(0), m_led_config(0), m_cur_mode(0)
{
    m_desc = 0;
    m_shupito = new Shupito(this);

    for(quint8 i = 0; i < MODE_COUNT; ++i)
        m_modes[i] = 0;

    connect(m_shupito, SIGNAL(descRead(bool)),                  SLOT(descRead(bool)));
    connect(m_shupito, SIGNAL(vccValueChanged(quint8,double)),  SIGNAL(vccValueChanged(quint8,double)));
    connect(m_shupito, SIGNAL(vddDesc(vdd_setup)),              SIGNAL(vddDesc(vdd_setup)));
    connect(m_shupito, SIGNAL(tunnelStatus(bool)),              SIGNAL(tunnelStatus(bool)));
    connect(m_shupito, SIGNAL(tunnelData(QByteArray)),          SIGNAL(tunnelData(QByteArray)));

    connect(m_con.data(), SIGNAL(connected(bool)), this, SLOT(connectedStatus(bool)));
    connect(m_con.data(), SIGNAL(packetRead(ShupitoPacket)), this, SLOT(readPacket(ShupitoPacket)));

    this->connectedStatus(conn->isOpen());
}

ShupitoProgrammer::~ShupitoProgrammer()
{
    for(quint8 i = 0; i < MODE_COUNT; ++i)
        delete m_modes[i];
    delete m_desc;
    delete m_shupito;
}

int ShupitoProgrammer::getType()
{
    return programmer_shupito;
}

QStringList ShupitoProgrammer::getAvailableModes()
{
    static const QString modeNames[] = { "SPI", "PDI", "cc25xx", "SPI flash" };

    QStringList modes;
    for (int i = 0; i < MODE_COUNT; ++i)
    {
        if (m_modes[i])
            modes.append(modeNames[i]);
    }
    return modes;
}

int ShupitoProgrammer::getMode()
{
    int res = 0;
    for (int i = 0; i < m_cur_mode; ++i)
    {
        if (m_modes[i])
            ++res;
    }
    return res;
}

void ShupitoProgrammer::setMode(int mode)
{
    Q_ASSERT(mode >= 0 && mode < MODE_COUNT);
    for (int i = 0; i < MODE_COUNT; ++i)
    {
        if (!m_modes[i])
            continue;

        if (mode == 0)
        {
            m_cur_mode = i;
            sConfig.set(CFG_QUINT32_SHUPITO_MODE, i);
            emit capabilitiesChanged();
            return;
        }

        --mode;
    }

    m_cur_mode = 0;
    emit capabilitiesChanged();
}

void ShupitoProgrammer::readPacket(const ShupitoPacket & packet)
{
    m_shupito->readPacket(packet);

    if (m_btn_config && packet.size() >= 3 && packet[0] == m_btn_config->cmd && packet[1] == 1 && (packet[2] & 1) != 0)
        emit buttonPressed(0);
}

void ShupitoProgrammer::setVddIndex(int index)
{
    ShupitoPacket p = makeShupitoPacket(MSG_VCC, 3, 1, 2, quint8(index));
    m_con->sendPacket(p);
}

void ShupitoProgrammer::setTunnelSpeed(quint32 speed, bool send)
{
    m_shupito->setTunnelSpeed(speed, send);
}

quint32 ShupitoProgrammer::getTunnelSpeed() const
{
    return m_shupito->getTunnelSpeed();
}

void ShupitoProgrammer::setTunnelState(bool enable, bool wait)
{
    m_shupito->setTunnelState(enable, wait);
}

void ShupitoProgrammer::switchToFlashMode(quint32 prog_speed_hz)
{
    m_modes[m_cur_mode]->switchToFlashMode(prog_speed_hz);
}

void ShupitoProgrammer::switchToRunMode()
{
    m_modes[m_cur_mode]->switchToRunMode();
}

bool ShupitoProgrammer::isInFlashMode()
{
    if (!m_modes[m_cur_mode])
        return false;
    return m_modes[m_cur_mode]->isInFlashMode();
}

chip_definition ShupitoProgrammer::readDeviceId()
{
    chip_definition cd = m_modes[m_cur_mode]->readDeviceId();
    m_shupito->setChipId(cd);
    return cd;
}

QByteArray ShupitoProgrammer::readMemory(const QString& mem, chip_definition &chip)
{
    return m_modes[m_cur_mode]->readMemory(mem, chip);
}

void ShupitoProgrammer::readFuses(std::vector<quint8>& data, chip_definition &chip)
{
    m_modes[m_cur_mode]->readFuses(data, chip);
}

void ShupitoProgrammer::writeFuses(std::vector<quint8>& data, chip_definition &chip, VerifyMode verifyMode)
{
    m_modes[m_cur_mode]->writeFuses(data, chip, verifyMode);
}

void ShupitoProgrammer::flashRaw(HexFile& file, quint8 memId, chip_definition& chip, VerifyMode verifyMode)
{
    m_modes[m_cur_mode]->flashRaw(file, memId, chip, verifyMode);
}

void ShupitoProgrammer::erase_device(chip_definition& chip)
{
    m_modes[m_cur_mode]->erase_device(chip);
}

void ShupitoProgrammer::connectedStatus(bool connected)
{
    if(connected)
    {
        m_tunnel_config = 0;
        m_vdd_config = 0;

        delete m_desc;
        m_desc = new ShupitoDesc();

        m_shupito->init(m_con.data(), m_desc);
    }
}

void ShupitoProgrammer::descRead(bool correct)
{
    if(!correct)
    {
        this->log("Failed to read info from shupito!");
        return Utils::showErrorBox(tr("Failed to read info from Shupito. If you're sure "
            "you're connected to shupito, try to disconnect and "
            "connect again"));
    }

    this->log("Device GUID: " % m_desc->getGuid());

    ShupitoDesc::intf_map map = m_desc->getInterfaceMap();
    for(ShupitoDesc::intf_map::iterator itr = map.begin(); itr != map.end(); ++itr)
        this->log("Got interface GUID: " % itr.key());

    for(quint8 i = 0; i < MODE_COUNT; ++i)
    {
        delete m_modes[i];
        m_modes[i] = 0;
    }

    for(quint8 i = 0; i < MODE_COUNT; ++i)
    {
        m_modes[i] = ShupitoMode::getMode(i, m_shupito, m_desc);
        if (m_modes[i])
        {
            connect(m_modes[i], SIGNAL(updateProgressDialog(int)), this, SIGNAL(updateProgressDialog(int)));
            connect(m_modes[i], SIGNAL(updateProgressLabel(QString)), this, SIGNAL(updateProgressLabel(QString)));
        }
    }

    m_cur_mode = sConfig.get(CFG_QUINT32_SHUPITO_MODE);
    if(m_cur_mode >= MODE_COUNT)
        m_cur_mode = MODE_SPI;
    if (!m_modes[m_cur_mode])
        this->setMode(0);

    m_vdd_config = m_desc->getConfig("1d4738a0-fc34-4f71-aa73-57881b278cb1");
    m_shupito->setVddConfig(m_vdd_config);
    if(m_vdd_config)
    {
        if(!m_vdd_config->always_active())
        {
            ShupitoPacket pkt = m_vdd_config->getStateChangeCmd(true);
            pkt = m_shupito->waitForPacket(pkt, MSG_INFO);

            if(pkt.size() == 2 && pkt[1] == 0)
                this->log("VDD started!");
            else
                this->log("Could not start VDD!");
        }
        ShupitoPacket packet = makeShupitoPacket(m_vdd_config->cmd, 2, 0, 0);
        m_con->sendPacket(packet);
    }

    m_tunnel_config = m_desc->getConfig("356e9bf7-8718-4965-94a4-0be370c8797c");
    m_shupito->setTunnelConfig(m_tunnel_config);
    if(m_tunnel_config && sConfig.get(CFG_BOOL_SHUPITO_TUNNEL))
    {
        if(!m_tunnel_config->always_active())
        {
            ShupitoPacket pkt = m_tunnel_config->getStateChangeCmd(true);
            pkt = m_shupito->waitForPacket(pkt, MSG_INFO);

            if(pkt.size() == 2 && pkt[1] == 0)
                this->log("Tunnel started!");
            else
                this->log("Could not start tunnel!");
        }

        m_shupito->setTunnelState(true);
    }else
        emit this->tunnelActive(false);

    m_btn_config = m_desc->getConfig("e5e646a8-beb6-4a68-91f2-f005c72e9e57");

    m_led_config = m_desc->getConfig("9034d141-c47e-406b-a6fd-3f5887729f8f");
    if (m_led_config)
        m_shupito->sendPacket(makeShupitoPacket(m_led_config->cmd, 1, 1));
    emit blinkLedSupport(/*supported=*/m_led_config != 0);
}

void ShupitoProgrammer::stopAll(bool wait)
{
    if(m_tunnel_config)
    {
        m_shupito->setTunnelState(false);
        m_shupito->setTunnelPipe(0);

        if(!m_tunnel_config->always_active())
        {
            ShupitoPacket pkt = m_tunnel_config->getStateChangeCmd(false);
            if(wait)
                m_shupito->waitForPacket(pkt, MSG_INFO);
            else
                m_con->sendPacket(pkt);
        }
    }

    if(m_vdd_config && !m_vdd_config->always_active())
    {
        ShupitoPacket pkt = m_vdd_config->getStateChangeCmd(false);
        if(wait)
            m_shupito->waitForPacket(pkt, MSG_INFO);
        else
            m_con->sendPacket(pkt);
    }
}

void ShupitoProgrammer::sendTunnelData(QString const & data)
{
    m_shupito->sendTunnelData(data);
}

void ShupitoProgrammer::cancelRequested()
{
    m_modes[m_cur_mode]->requestCancel();
}

bool ShupitoProgrammer::canBlinkLed()
{
    return m_led_config != 0;
}

void ShupitoProgrammer::blinkLed()
{
    m_shupito->sendPacket(makeShupitoPacket(m_led_config->cmd, 1, 2));
}

ProgrammerCapabilities ShupitoProgrammer::capabilities() const
{
    ProgrammerCapabilities caps;
    if (m_modes[m_cur_mode])
    {
        caps = m_modes[m_cur_mode]->capabilities();
    }
    else
    {
        caps.flash = true;
        caps.eeprom = true;
    }
    caps.terminal = m_tunnel_config != 0;
    return caps;
}
