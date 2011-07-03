#include <QtCore/QDebug>

#include <abstractserial.h>
#include "anyslave.h"

#include <iostream>

using namespace std;


AnySlave::AnySlave(QObject *parent)
        : QObject(parent)
{
    port = new AbstractSerial(this);

    char dn[50]; //device name
    cout << "Please enter serial device name, specific by OS, \n example: in Windows -> COMn, in GNU/Linux -> /dev/ttyXYZn: ";
    cin >> dn;

    port->setDeviceName(dn);

    connect(port, SIGNAL(readyRead()), this, SLOT(transaction()));

    /*
        Here using the open flag "Unbuffered". 
        This flag disables the internal buffer class, 
        and also disables the automatic data acquisition (disables asynchronous mode).
        In this case, we have disabled the asynchronous mode to read from port data using timeouts on the packet.
        Ie if we call, for example, read(5) and in buffer UART 
        not yet available - then the method will wait for a time (total read timeout and/or char interval timeout) until the data arrive.
        
        Note: Behavior would be different if you open a port without a flag "Unbuffered". 
              I will not describe it - test/check it yourself. ;)
    */
    if ( !port->open(AbstractSerial::ReadWrite | AbstractSerial::Unbuffered) ) {
        qDebug() << "Serial device by default: " << port->deviceName() << " open fail.";
        return;
    }
    if (!port->setBaudRate(AbstractSerial::BaudRate115200)) {
        qDebug() << "Set baud rate " <<  AbstractSerial::BaudRate115200 << " error.";
        return;
    };
    if (!port->setDataBits(AbstractSerial::DataBits8)) {
        qDebug() << "Set data bits " <<  AbstractSerial::DataBits8 << " error.";
        return;
    }
    if (!port->setParity(AbstractSerial::ParityNone)) {
        qDebug() << "Set parity " <<  AbstractSerial::ParityNone << " error.";
        return;
    }
    if (!port->setStopBits(AbstractSerial::StopBits1)) {
        qDebug() << "Set stop bits " <<  AbstractSerial::StopBits1 << " error.";
        return;
    }
    if (!port->setFlowControl(AbstractSerial::FlowControlOff)) {
        qDebug() << "Set flow " <<  AbstractSerial::FlowControlOff << " error.";
        return;
    }

    /*
        Important Note:

        1. For All OS:
        If you use buffered mode (ie, at the opening did not put the flag AbstractSerial::Unbuffered),
        there is no need to set timeouts reading (Ie they are to remain the default = 0)/
        Any value other than 0 will only slow down data acquisition.

        2. For Windows:
        If you are using unbuffered mode, the timeouts have the effect of reading!
        Necessary for the total timeout to set the value of reading At least 1 ms,
        or (at 0) will not be read.

        PS: I have not figured out yet what the reason.

        3. For *.nix:
        If you are using unbuffered mode, the timeouts have the effect of reading!
        Necessary for the total timeout to set the value of reading At least 1 ms,
        as if the value is 0 read will return immediately,
        so you can not read the requested number of bytes (ie, reading function can return fewer bytes).

        In any case, experiment with options for treatment with buffered/unbuffered,
        as well as the timeout values from 0 to N and find the differences. :)
    */

#if defined (Q_OS_UNIX)
        // Method setTotalReadConstantTimeout() not supported in *.nix.
        if (port->openMode() & AbstractSerial::Unbuffered)
            port->setCharIntervalTimeout(5000);//5 msec
#elif defined (Q_OS_WIN)
        if (port->openMode() & AbstractSerial::Unbuffered)
            port->setTotalReadConstantTimeout(100);
#endif

    qDebug() << "= New parameters =";
    qDebug() << "Device name            : " << port->deviceName();
    qDebug() << "Baud rate              : " << port->baudRate();
    qDebug() << "Data bits              : " << port->dataBits();
    qDebug() << "Parity                 : " << port->parity();
    qDebug() << "Stop bits              : " << port->stopBits();
    qDebug() << "Flow                   : " << port->flowControl();
    qDebug() << "Total read timeout constant, msec : " << port->totalReadConstantTimeout();
    qDebug() << "Char interval timeout, usec       : " << port->charIntervalTimeout();
}

AnySlave::~AnySlave()
{
    port->close();
}

void AnySlave::transaction()
{
    QByteArray data = port->readAll();
    qDebug() << "Readed: " << data.size() << " bytes";
    qint64 len = port->write(data);

    if (len == qint64(data.size())) {
        qDebug() << "Writed: " << len << " bytes";
    }
    else qDebug() << "Bytes writed small: " << len;
}

