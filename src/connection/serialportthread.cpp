#include "serialportthread.h"

SerialPortThread::SerialPortThread(QString name, AbstractSerial::BaudRate rate) : QThread(0)
{
    m_port = NULL;
    runTh = true;
    devName = name;
    m_rate = rate;
}

SerialPortThread::~SerialPortThread()
{
    if(m_port)
        delete m_port;
}

void SerialPortThread::Stop()
{
    if(m_port)
        m_port->close();
    runTh = false;
}

void SerialPortThread::Send(QByteArray data)
{
    if(m_port)
        m_port->write(data);
}

bool SerialPortThread::Open()
{
    m_port = new AbstractSerial();
    connect(m_port, SIGNAL(signalStatus(const QString, QDateTime)), this, SLOT(viewStateSlot(QString, QDateTime)));

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

    while(runTh)
    {
        if(m_port->bytesAvailable() > 0)
        {
            QByteArray ba = m_port->readAll();
            emit dataRead(ba);
        }
        msleep(5);
    }
}

void SerialPortThread::viewStateSlot(QString /*stateMsg*/, QDateTime /*dt*/)
{

}
