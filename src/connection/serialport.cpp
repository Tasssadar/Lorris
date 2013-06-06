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
#include "../shared/programmer.h"

SerialPort::SerialPort()
    : PortConnection(CONNECTION_SERIAL_PORT),
      m_rate(38400),
      m_devNameEditable(true)
{
    m_port = NULL;
    m_openThread = NULL;

#ifdef Q_OS_WIN
    m_thread = new SerialPortThread(this);
#endif
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

void SerialPort::connectResultSer(bool opened)
{
    this->SetState(opened? st_connected: st_disconnected);
}

void SerialPort::doClose()
{
    if(m_port)
        emit disconnecting();

    // port is currently opening
    if(m_openThread)
    {
        m_openThread->stop(); // make sure QextSerialPort will be destroyed
        if(m_port_mutex.tryLock(100))
        {
            m_openThread->wait(5000);
            delete m_openThread->claimPort();
            delete m_openThread;
            m_openThread = NULL;

            m_port_mutex.unlock();
        }
        else // let it connect/fail to connect in the background
        {
            disconnect(m_openThread, SIGNAL(finished()), this, SLOT(openResult()));
            connect(m_openThread, SIGNAL(finished()), m_openThread, SLOT(deleteLater()));
            m_openThread = NULL;
        }
    }
    else
    {
        QMutexLocker l(&m_port_mutex);
#ifdef Q_OS_WIN
        m_thread->setPort(NULL);
#endif
        if(m_port)
        {
            m_port->close();
            delete m_port;
            m_port = NULL;
        }
    }

    this->SetState(st_disconnected);
}

void SerialPort::SendData(const QByteArray& data)
{
    if(this->isOpen())
    {
        QMutexLocker l(&m_port_mutex);
        m_port->write(data);
    }
}

void SerialPort::doOpen()
{
    this->SetState(st_connecting);

    Q_ASSERT(!m_openThread);

    m_openThread = new SerialPortOpenThread(this);
    connect(m_openThread, SIGNAL(finished()), SLOT(openResult()));
    m_openThread->start();
}

void SerialPort::readyRead()
{
    if(!isOpen())
        return;

    lockMutex();
    QByteArray data = m_port->readAll();
    unlockMutex();

    emit dataRead(data);
}

void SerialPort::openResult()
{
    m_port = m_openThread->claimPort();

    delete m_openThread;
    m_openThread = NULL;

    if(m_port)
    {
        connect(m_port, SIGNAL(readyRead()),              SLOT(readyRead()));
        connect(m_port, SIGNAL(socketError(SocketError)), SLOT(socketError(SocketError)));
    }

#ifdef Q_OS_WIN
    m_port_mutex.lock();
    m_thread->setPort(m_port);
    m_port_mutex.unlock();
#endif
    connectResultSer(m_port != NULL);
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

        // FIXME: better way to detect shupito?
        if(m_friendlyName.startsWith("Shupito Programmer"))
            m_programmer_type = programmer_shupito;

        emit changed();
    }
}

void SerialPort::setBaudRate(int value)
{
    m_rate = value;
    if(isOpen())
        m_port->setBaudRate(value);
    emit changed();
}

void SerialPort::socketError(SocketError err)
{
    if(err == ERR_IOCTL_FAILED && isOpen())
    {
        sWorkTabMgr.printToAllStatusBars(tr("Connection to %1 lost!").arg(m_deviceName));
        Close();
    }
}

QHash<QString, QVariant> SerialPort::config() const
{
    QHash<QString, QVariant> res = this->PortConnection::config();
    res["device_name"] = this->deviceName();
    res["baud_rate"] = (int)this->baudRate();
    return res;
}

bool SerialPort::applyConfig(QHash<QString, QVariant> const & config)
{
    this->setDeviceName(config.value("device_name").toString());
    this->setBaudRate(config.value("baud_rate", 38400).toInt());
    return this->PortConnection::applyConfig(config);
}

ConnectionPointer<Connection> SerialPort::clone()
{
    ConnectionPointer<SerialPort> res(new SerialPort());
    res->setName(tr("Clone of ") + this->name());
    res->setDeviceName(this->deviceName());
    res->setBaudRate(this->baudRate());
    res->setProgrammerType(this->programmerType());
    return res;
}

SerialPortOpenThread::SerialPortOpenThread(SerialPort *conn) : QThread(NULL)
{
    m_conn = conn;
    m_port = NULL;
    m_run = true;
}

void SerialPortOpenThread::run()
{
    m_run = true;
    Q_ASSERT(!m_port);
    if(m_port)
        return;

    m_conn->lockMutex();

#ifdef Q_OS_WIN
    m_port = new QextSerialPort(m_conn->deviceName(), QextSerialPort::Polling);
    m_port->setTimeout(-1);
#else
    m_port = new QextSerialPort(m_conn->deviceName(), QextSerialPort::EventDriven);
    m_port->setTimeout(500);
#endif

    m_port->setBaudRate(m_conn->baudRate());
    m_port->setParity(PAR_NONE);
    m_port->setDataBits(DATA_8);
    m_port->setStopBits(STOP_1);
    m_port->setFlowControl(FLOW_OFF);

    bool res = m_port->open(QIODevice::ReadWrite | QIODevice::Unbuffered);
    if(res && m_run)
        m_port->moveToThread(QApplication::instance()->thread());
    else
    {
        delete m_port;
        m_port = NULL;
    }

    m_conn->unlockMutex();
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
