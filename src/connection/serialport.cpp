/**********************************************
**    This file is part of Lorris
**    http://tasssadar.github.com/Lorris/
**
**    See README and COPYING
***********************************************/

#include <QtCore/QtConcurrentRun>
#include <QtCore/QThreadPool>
#include <qextserialport.h>
#include <QLabel>
#include <QComboBox>
#include <QApplication>
#include <QPushButton>
#include <QStyle>
#include <qextserialenumerator.h>
#include <QStringBuilder>

#include "serialport.h"
#include "../misc/config.h"
#include "../WorkTab/WorkTabMgr.h"
#include "../WorkTab/WorkTab.h"
#include "../WorkTab/WorkTabInfo.h"

SerialPort::SerialPort()
    : PortConnection(CONNECTION_SERIAL_PORT),
      m_rate(38400),
      m_devNameEditable(true)
{
    m_port = NULL;

#ifdef Q_OS_WIN
    m_thread = new SerialPortThread(this);
    m_thread->start();
#endif

    connect(&m_watcher, SIGNAL(finished()), SLOT(openResult()));
}

SerialPort::~SerialPort()
{
    Close();

#ifdef Q_OS_WIN
    m_thread->stop();
    m_thread->wait(500);
#endif
}

QString SerialPort::details() const
{
    QString res = Connection::details();
    if (!res.isEmpty())
        res += ", ";
    return res + (m_friendlyName.isEmpty()? m_deviceName: m_friendlyName);
}

bool SerialPort::Open()
{
    return false;
}

void SerialPort::connectResultSer(bool opened)
{
    this->SetOpen(opened);
    emit connectResult(this, opened);
}

void SerialPort::Close()
{
    if(m_port)
        emit disconnecting();

#ifdef Q_OS_WIN
    m_thread->setPort(NULL);
#endif

    {
        QMutexLocker l(&m_port_mutex);

        if(m_port)
        {
            m_port->close();
            delete m_port;
            m_port = NULL;
        }
    }

    this->SetOpen(false);
}

void SerialPort::SendData(const QByteArray& data)
{
    if(this->isOpen())
    {
        QMutexLocker l(&m_port_mutex);
        m_port->write(data);
    }
}

void SerialPort::OpenConcurrent()
{
    if(this->state() != st_disconnected)
        return;

    this->SetState(st_connecting);

    m_future = QtConcurrent::run(this, &SerialPort::openPort);
    m_watcher.setFuture(m_future);
}

bool SerialPort::openPort()
{
    m_port_mutex.lock();

#ifdef Q_OS_WIN
    m_port = new QextSerialPort(this->deviceName(), QextSerialPort::Polling);
    m_port->setTimeout(0);
#else
    m_port = new QextSerialPort(this->deviceName(), QextSerialPort::EventDriven);
    m_port->setTimeout(500);
#endif

    m_port->setBaudRate(m_rate);
    m_port->setParity(PAR_NONE);
    m_port->setDataBits(DATA_8);
    m_port->setStopBits(STOP_1);
    m_port->setFlowControl(FLOW_OFF);

    bool res = m_port->open(QIODevice::ReadWrite | QIODevice::Unbuffered);
    if(!res)
    {
        delete m_port;
        m_port = NULL;
    }
    else
    {
        m_port->moveToThread(QApplication::instance()->thread());
        connect(m_port, SIGNAL(readyRead()),              SLOT(readyRead()));
        connect(m_port, SIGNAL(socketError(SocketError)), SLOT(socketError(SocketError)));
    }

    m_port_mutex.unlock();
    return res;
}

void SerialPort::readyRead()
{
    lockMutex();
    QByteArray data = m_port->readAll();
    unlockMutex();

    emit dataRead(data);
}

void SerialPort::openResult()
{
#ifdef Q_OS_WIN
    m_thread->setPort(m_port);
#endif
    connectResultSer(m_future.result());
}

void SerialPort::setDeviceName(QString const & value)
{
    if (m_deviceName != value)
    {
        m_deviceName = value;
        emit changed();
    }
}

void SerialPort::setFriendlyName(QString const & value)
{
    if (m_friendlyName != value)
    {
        m_friendlyName = value;
        emit changed();
    }
}

void SerialPort::socketError(SocketError err)
{
    if(err == ERR_IOCTL_FAILED && isOpen())
    {
        Utils::printToStatusBar(tr("Connection to %1 lost!").arg(m_deviceName));
        Close();
    }
}

QHash<QString, QVariant> SerialPort::config() const
{
    QHash<QString, QVariant> res = this->Connection::config();
    res["device_name"] = this->deviceName();
    res["baud_rate"] = (int)this->baudRate();
    return res;
}

bool SerialPort::applyConfig(QHash<QString, QVariant> const & config)
{
    this->setDeviceName(config.value("device_name").toString());
    this->setBaudRate(config.value("baud_rate", 38400).toInt());
    return this->Connection::applyConfig(config);
}

#ifdef Q_OS_WIN

SerialPortThread::SerialPortThread(SerialPort *con) : QThread(con)
{
    m_run = true;
    m_port = NULL;
    m_con = con;
    connect(this, SIGNAL(readyRead()), con, SLOT(readyRead()), Qt::QueuedConnection);
}

void SerialPortThread::setPort(QextSerialPort *port)
{
    m_con->lockMutex();

    if(!m_port && port)
    {
        m_run = true;
        m_port = port;
        start();
    }
    else if(m_port && !port)
    {
        m_run = false;
        m_port = port;
        wait(500);
    }

    m_con->unlockMutex();
}

void SerialPortThread::run()
{
    while(m_run)
    {
        m_con->lockMutex();

        if(m_port && m_port->isOpen())
            if(m_port->bytesAvailable() > 0)
                emit readyRead();

        m_con->unlockMutex();

        msleep(1);
    }
}

#endif // Q_OS_WIN
