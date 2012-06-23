/**********************************************
**    This file is part of Lorris
**    http://tasssadar.github.com/Lorris/
**
**    See README and COPYING
***********************************************/

#ifndef SERIALPORT_H
#define SERIALPORT_H

#include <QFuture>
#include <QFutureWatcher>
#include <qextserialport.h>

#include "connection.h"

class QComboBox;
class SerialPortThread;

class SerialPort : public PortConnection
{
    Q_OBJECT

Q_SIGNALS:
    void stopThread();

public:
    SerialPort();
    virtual ~SerialPort();

    bool Open();
    void Close();
    void OpenConcurrent();

    void SendData(const QByteArray &data);

    int baudRate() const { return m_rate; }
    void setBaudRate(int value) { m_rate = value; emit changed(); }

    QString deviceName() const { return m_deviceName; } // FIXME: id and devname should be separate
    void setDeviceName(QString const & value);

    bool devNameEditable() const { return m_devNameEditable; }
    void setDevNameEditable(bool value) { m_devNameEditable = value; }

    QHash<QString, QVariant> config() const;
    bool applyConfig(QHash<QString, QVariant> const & config);

private slots:
    void connectResultSer(bool opened);
    void openResult();
    void readyRead();
    void socketError(SocketError err);

private:
    bool openPort();

    QString m_deviceName;

    QextSerialPort *m_port;
    int m_rate;

    QFuture<bool> m_future;
    QFutureWatcher<bool> m_watcher;

    QMutex m_port_mutex;
    bool m_devNameEditable;
};

#endif // SERIALPORT_H
