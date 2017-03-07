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
#include <QThread>
#include <qextserialport.h>

#include "connection.h"

class QComboBox;
class SerialPortOpenThread;
#ifdef Q_OS_WIN
    class SerialPortThread;
#endif



class SerialPort : public PortConnection
{
    Q_OBJECT

Q_SIGNALS:
    void stopThread();

public:
    SerialPort();

    virtual QString details() const;

    void SendData(const QByteArray &data);

    int baudRate() const { return m_rate; }
    void setBaudRate(int value);
    ParityType parity() const { return m_parity; }
    void setParity(ParityType p);
    StopBitsType stopBits() const { return m_stopBits; }
    void setStopBits(StopBitsType st);
    DataBitsType dataBits() const { return m_dataBits; }
    void setDataBits(DataBitsType dt);
    FlowType flowControl() const { return m_flowControl; }
    void setFlowControl(FlowType type);

    bool dtr() const { return m_dtrToggled; }
    void setDtr(bool set);
    bool rts() const { return m_rtsToggled; }
    void setRts(bool set);

    QString deviceName() const { return m_deviceName; }
    void setDeviceName(QString const & value);

    QString friendlyName() const { return m_deviceName; }
    void setFriendlyName(QString const & value);

    bool devNameEditable() const { return m_devNameEditable; }
    void setDevNameEditable(bool value) { m_devNameEditable = value; }

    QHash<QString, QVariant> config() const;
    bool applyConfig(QHash<QString, QVariant> const & config);
    bool canSaveToSession() const { return true; }

    void lockMutex()
    {
        m_port_mutex.lock();
    }

    void unlockMutex()
    {
        m_port_mutex.unlock();
    }

    bool clonable() const { return true; }
    ConnectionPointer<Connection> clone();

protected:
    ~SerialPort();
    void doClose();
    void doOpen();

private slots:
    void connectResultSer(bool opened);
    void openResult();
    void readyRead();
    void socketError(SocketError err);

private:
    QString m_deviceName;
    QString m_friendlyName;

    bool m_devNameEditable;

    QextSerialPort *m_port;
    int m_rate;
    ParityType m_parity;
    StopBitsType m_stopBits;
    DataBitsType m_dataBits;
    FlowType m_flowControl;
    bool m_rtsToggled;
    bool m_dtrToggled;

    QMutex m_port_mutex;

#ifdef Q_OS_WIN
    SerialPortThread *m_thread;
#endif
    SerialPortOpenThread *m_openThread;
};

class SerialPortOpenThread : public QThread
{
    Q_OBJECT
public:
    SerialPortOpenThread(SerialPort *conn);

    QextSerialPort *claimPort()
    {
        QextSerialPort *res = m_port;
        m_port = NULL;
        return res;
    }

    void stop() { m_run = false; }

protected:
    void run();

private:
    QextSerialPort *m_port;
    SerialPort *m_conn;
    volatile bool m_run;
};

#ifdef Q_OS_WIN

class SerialPortThread : public QThread
{
    Q_OBJECT

Q_SIGNALS:
    void readyRead();

public:
    SerialPortThread(SerialPort *port);

    void setPort(QextSerialPort *port);

    void stop()
    {
        m_run = false;
    }

protected:
    void run();

private:
    volatile bool m_run;
    QextSerialPort *m_port;
    SerialPort *m_con;
};

#endif // Q_OS_WIN

#endif // SERIALPORT_H
