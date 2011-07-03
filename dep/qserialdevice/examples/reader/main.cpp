/* Reader
*
* This application is part of the examples on the use of the library QSerialDevice.
*
* reader - a test console application to read data from the port using the method of expectations waitForReadyRead()
*
* Copyright (C) 2009  Denis Shienkov
*
* Contact Denis Shienkov:
*          e-mail: <scapig2@yandex.ru>
*             ICQ: 321789831
*/

#include <QtCore/QDebug>
#include <QtCore/QCoreApplication>
#include <abstractserial.h>
#include <iostream>

using namespace std;

int main(int argc, char *argv[])
{
    QCoreApplication app(argc, argv);

    /* 1. First - create an instance of an object.
    */
    AbstractSerial *port = new AbstractSerial();

    char dn[50]; //device name
    cout << "Please enter serial device name, specific by OS, \n example: in Windows -> COMn, in GNU/Linux -> /dev/ttyXYZn: ";
    cin >> dn;

    /* 2. Second - set the device name.
    */
    port->setDeviceName(dn);

    /* 3. Third - open the device.
    
        Here using the open flag "Unbuffered". 
        This flag disables the internal buffer class, 
        and also disables the automatic data acquisition (disables asynchronous mode).
        In this case, we have disabled the asynchronous mode to read from port data using timeouts on the packet.
        Ie if we call, for example, read(5) and in buffer UART 
        not yet available - then the method will wait for a time (total read timeout and/or char interval timeout) until the data arrive.
        
        Note: Behavior would be different if you open a port without a flag "Unbuffered". 
              I will not describe it - test/check it yourself. ;)
    */
    if (port->open(AbstractSerial::ReadOnly | AbstractSerial::Unbuffered)) {
        qDebug() << "Serial device " << port->deviceName() << " open in " << port->openMode();

        //Here, the default current parameters (for example)
        qDebug() << "= Default parameters =";
        qDebug() << "Device name            : " << port->deviceName();
        qDebug() << "Baud rate              : " << port->baudRate();
        qDebug() << "Data bits              : " << port->dataBits();
        qDebug() << "Parity                 : " << port->parity();
        qDebug() << "Stop bits              : " << port->stopBits();
        qDebug() << "Flow                   : " << port->flowControl();
        qDebug() << "Total read timeout constant, msec : " << port->totalReadConstantTimeout();
        qDebug() << "Char interval timeout, usec       : " << port->charIntervalTimeout();

        /* 4. Fourth - now you can set the parameters. (after successfully opened port)
        */

        //Here example set baud rate 115200 bit/sec (baud)
        if (!port->setBaudRate(AbstractSerial::BaudRate115200)) {
            qDebug() << "Set baud rate " <<  AbstractSerial::BaudRate115200 << " error.";
            goto label;
        };

        if (!port->setDataBits(AbstractSerial::DataBits8)) {
            qDebug() << "Set data bits " <<  AbstractSerial::DataBits8 << " error.";
            goto label;
        }

        if (!port->setParity(AbstractSerial::ParityNone)) {
            qDebug() << "Set parity " <<  AbstractSerial::ParityNone << " error.";
            goto label;
        }

        if (!port->setStopBits(AbstractSerial::StopBits1)) {
            qDebug() << "Set stop bits " <<  AbstractSerial::StopBits1 << " error.";
            goto label;
        }

        if (!port->setFlowControl(AbstractSerial::FlowControlOff)) {
            qDebug() << "Set flow " <<  AbstractSerial::FlowControlOff << " error.";
            goto label;
        }

        /*
            ...
            here you can set other parameters.
            ...
        */

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

        // Here set total timeout for read 1 ms, if open mode is Unbuffered only!
#if defined (Q_OS_UNIX)
        // Method setTotalReadConstantTimeout() not supported in *.nix.
        if (port->openMode() & AbstractSerial::Unbuffered)
            port->setCharIntervalTimeout(5000);//5 msec
#elif defined (Q_OS_WIN)
        if (port->openMode() & AbstractSerial::Unbuffered)
            port->setTotalReadConstantTimeout(100);
#endif

        //Here, the new set parameters (for example)
        qDebug() << "= New parameters =";
        qDebug() << "Device name            : " << port->deviceName();
        qDebug() << "Baud rate              : " << port->baudRate();
        qDebug() << "Data bits              : " << port->dataBits();
        qDebug() << "Parity                 : " << port->parity();
        qDebug() << "Stop bits              : " << port->stopBits();
        qDebug() << "Flow                   : " << port->flowControl();
        qDebug() << "Total read timeout constant, msec : " << port->totalReadConstantTimeout();
        qDebug() << "Char interval timeout, usec       : " << port->charIntervalTimeout();

        int rrto = 0; //timeout for ready read
        cout << "Please enter wait timeout for ready read, msec : ";
        cin >> rrto;

        int len = 0; //len data for read
        cout << "Please enter len data for read, bytes : ";
        cin >> len;

        qDebug() << "Enter is " << rrto << " msecs, " << len << " bytes";
        qDebug() << "Starting waiting ready read in time : " << QTime::currentTime();

        QByteArray ba; //received data

        /* 5. Fifth - you can now read / write device, or further modify its settings, etc.
        */
        while (1) {
            if ((port->bytesAvailable() > 0) ||  port->waitForReadyRead(rrto)) {
                ba.clear();
                ba = port->read(len);
                qDebug() << "Readed is : " << ba.size() << " bytes";
            }
            else {
                qDebug() << "Timeout read data in time : " << QTime::currentTime();
            }
        }//while
    }
    else {
        qDebug() << "Error opened serial device " << port->deviceName();
    }

    label:

    port->close();
    qDebug() << "Serial device " << port->deviceName() << " is closed";
    delete port;
    port = 0;
    return app.exec();
}
