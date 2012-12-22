#ifndef SHARED_PROGRAMMER_H
#define SHARED_PROGRAMMER_H

#include <QObject>

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

public:
    explicit Programmer(ProgrammerLogSink * logsink)
        : m_logsink(logsink)
    {
    }

    virtual bool supportsVdd() const { return false; }
    virtual bool supportsTunnel() const { return false; }

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
    virtual void readMemRange(quint8 memid, QByteArray& memory, quint32 address, quint32 size) = 0;
    virtual void readFuses(std::vector<quint8>& data, chip_definition &chip) = 0;
    virtual void writeFuses(std::vector<quint8>& data, chip_definition &chip, quint8 verifyMode) = 0;
    virtual void flashRaw(HexFile& file, quint8 memId, chip_definition& chip, quint8 verifyMode) = 0;

    virtual void erase_device(chip_definition& chip) = 0;

public slots:
    virtual void sendTunnelData(QString const &) {}
    virtual void cancelRequested() {}

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
