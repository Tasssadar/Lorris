#include "serialportthread.h"

SerialPortThread::SerialPortThread(QString name, AbstractSerial::BaudRate rate, SerialPort *con) : QThread(0)
{
    m_port = NULL;
    runTh = true;
    devName = name;
    m_rate = rate;
    m_con = con;

    m_writeErrorCount = 0;
}

SerialPortThread::~SerialPortThread()
{

}

void SerialPortThread::Send(const QByteArray& data)
{
    if(runTh && m_port)
    {
        m_portMutex.lock();
        m_port->write(data);
        m_portMutex.unlock();
    }
}

bool SerialPortThread::Open()
{
    m_port = new AbstractSerial();
    connect(m_port, SIGNAL(signalStatus(const QString, QDateTime)), this, SLOT(viewStateSlot(QString, QDateTime)), Qt::QueuedConnection);

    m_port->setDeviceName(devName);
    m_port->enableEmitStatus(true);

    if(m_port->open(AbstractSerial::ReadWrite | AbstractSerial::Unbuffered))
    {
        m_port->setBaudRate(m_rate, AbstractSerial::AllBaud);
        m_port->setParity(AbstractSerial::ParityNone);
        m_port->setDataBits(AbstractSerial::DataBits8);
        m_port->setStopBits(AbstractSerial::StopBits1);
        m_port->setFlowControl(AbstractSerial::FlowControlOff);
        opened = true;
    }
    else
    {
        opened = false;
        delete m_port;
        m_port = NULL;
    }
    return opened;
}

void SerialPortThread::run()
{
    emit connectResult(Open());

    if(!opened)
        return;

    connect(m_con, SIGNAL(stopThread()), this, SLOT(stop()), Qt::QueuedConnection);

    while(runTh)
    {
        m_portMutex.lock();
        if(m_port->bytesAvailable() > 0)
        {
            QByteArray ba = m_port->readAll();
            emit dataRead(ba);
        }
        m_portMutex.unlock();
        msleep(10);
    }
    m_port->close();
    delete m_port;
    m_port = NULL;
    deleteLater();
}

void SerialPortThread::stop()
{
    runTh = false;
}

void SerialPortThread::viewStateSlot(QString /*stateMsg*/, QDateTime /*dt*/)
{
   /* if(stateMsg == "Controls::Write data to device - i/o problem. Error!")
    {
        ++m_writeErrorCount;
        if(m_writeErrorCount >= 10)
        {
            emit connectResult(false);
            opened = false;
            runTh = false;
        }
    }*/
}
