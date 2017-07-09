#ifndef SHARED_PROGRAMMER_H
#define SHARED_PROGRAMMER_H

#include <QObject>
#include <vector>
#include <QStringList>

#include "../shared/chipdefs.h"
#include "../shared/hexfile.h"

// device.hpp, 122
struct vdd_point
{
    QString name;
    std::vector<QString> drives;
    quint16 current_drive;
};

typedef std::vector<vdd_point> vdd_setup;

struct ProgrammerLogSink
{
    virtual void log(QString const & msg) = 0;
};

enum ProgrammerTypes
{
    programmer_shupito = 0,
    programmer_flip,
    programmer_avr232boot,
    programmer_atsam,
    programmer_avr109,
    programmer_stm32,
    programmer_arduino,
    programmer_zmodem,

    programmer_max
};

enum VerifyMode
{
    VERIFY_NONE,
    VERIFY_ONLY_NON_EMPTY,
    VERIFY_ALL_PAGES,
    VERIFY_MAX
};

struct ProgrammerCapabilities
{
    bool terminal;
    bool flash;
    bool eeprom;
    bool svf;
    bool fuses;
    int widgets;

    ProgrammerCapabilities()
        : terminal(false), flash(false), eeprom(false), svf(false), fuses(false),
          widgets(0)
    {
    }

    bool supports_erase() const
    {
        return flash || eeprom || fuses;
    }
};

#define PROG_WIDGET_PROPERTY "programmerWidget"

class Programmer
    : public QObject
{
    Q_OBJECT

signals:
    void modesChanged();

    void vccValueChanged(quint8 id, double value);
    void vddDesc(const vdd_setup& vs);
    void tunnelStatus(bool);

    void tunnelData(QByteArray const &);
    void tunnelActive(bool);

    void updateProgressDialog(int);
    void updateProgressLabel(QString const &);

    void buttonPressed(int btnid);

    void blinkLedSupport(bool supported);

    void capabilitiesChanged();

    void pwmChanged(uint32_t freq_hz, float duty_cycle);

public:
    explicit Programmer(ProgrammerLogSink * logsink)
        : m_logsink(logsink)
    {
    }

    virtual bool supportsPwm() const { return false; }
    virtual bool setPwmFreq(uint32_t freq_hz, float duty_cycle);

    virtual bool supportsVdd() const { return false; }
    virtual bool supportsBootseq() const { return false; }
    bool supportsTunnel() const { return this->capabilities().terminal; }

    virtual QString getBootseq() const { return QString(); }

    virtual QStringList getAvailableModes() { return QStringList(); }
    virtual int getMode() { return 0; }
    virtual void setMode(int) {}

    virtual void stopAll(bool wait) = 0;

    virtual void setVddIndex(int) {}
    virtual void setTunnelSpeed(quint32, bool = true) {}
    virtual void setTunnelState(bool, bool = false) {}
    virtual quint32 getTunnelSpeed() const { return 0; }

    virtual void switchToFlashMode(quint32 prog_speed_hz) = 0;
    virtual void switchToRunMode() = 0;
    virtual bool isInFlashMode() = 0;
    virtual chip_definition readDeviceId() = 0;

    virtual QByteArray readMemory(const QString& mem, chip_definition &chip) = 0;
    virtual void readFuses(std::vector<quint8>& data, chip_definition &chip) = 0;
    virtual void writeFuses(std::vector<quint8>& data, chip_definition &chip, VerifyMode verifyMode) = 0;
    virtual void flashRaw(HexFile& file, quint8 memId, chip_definition& chip, VerifyMode verifyMode) = 0;

    virtual void executeText(QByteArray const & data, quint8 memId, chip_definition & chip);

    virtual void erase_device(chip_definition& chip) = 0;

    virtual bool canBlinkLed() { return false; }
    virtual void blinkLed() {}

    virtual int getType() = 0;

    virtual ProgrammerCapabilities capabilities() const = 0;
    virtual QList<QWidget*> widgets() { return QList<QWidget*>(); }

public slots:
    virtual void sendTunnelData(QString const &) {}
    virtual void cancelRequested() {}
    virtual void setBootseq(const QString& /*seq*/) {}

protected:
    void log(QString const & msg)
    {
        if (m_logsink)
            m_logsink->log(msg);
    }

private:
    ProgrammerLogSink * m_logsink;
};

#endif // SHARED_PROGRAMMER_H
