#ifndef NOTIFIER_H
#define NOTIFIER_H

#include <QtCore/QDebug>
#include <QtCore/QByteArray>

#include <abstractserial.h>
#include <iostream>

using namespace std;

class Notifier : public QObject
{
    Q_OBJECT

public:
    Notifier(QObject *parent = 0)
        : QObject(parent) {

        /*1. First - create an instance of an object.
        */
        port = new AbstractSerial(this);
        //1.1. Connect to ...
        connect(port, SIGNAL(readyRead()), this, SLOT(slotRead()));
        connect(port, SIGNAL(readyWrite()), this, SLOT(slotWrite()));
        connect(port, SIGNAL(exception()), this, SLOT(slotException()));
        connect(port, SIGNAL(ctsChanged(bool)), this, SLOT(slotCts(bool)));
        connect(port, SIGNAL(dsrChanged(bool)), this, SLOT(slotDsr(bool)));
        connect(port, SIGNAL(ringChanged(bool)), this, SLOT(slotRing(bool)));
	

        /*2. Second - set the device name.
        */

        char dn[50]; //device name
        cout << "Please enter serial device name, specific by OS, \n example: in Windows -> COMn, in GNU/Linux -> /dev/ttyXYZn: ";
        cin >> dn;
        //port->setDeviceName("COM1");
        port->setDeviceName(dn);

        /* 3. Third - open the device.
        */
        if (!port->open(AbstractSerial::ReadOnly)) {
            qDebug() << "Serial device by default: " << port->deviceName() << " open fail.";
            return;
        }

        //Here, the default current parameters (for example)
        qDebug() << "= Default parameters =";
        qDebug() << "Device name            : " << port->deviceName();
        qDebug() << "Baud rate              : " << port->baudRate();
        qDebug() << "Data bits              : " << port->dataBits();
        qDebug() << "Parity                 : " << port->parity();
        qDebug() << "Stop bits              : " << port->stopBits();
        qDebug() << "Flow                   : " << port->flowControl();
        qDebug() << "Char timeout, msec     : " << port->charIntervalTimeout();

        /* 4. Fourth - now you can set the parameters. (after successfully opened port)
        */

        //Here example set baud rate 115200 bit/sec (baud)
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


        //Here, the new set parameters (for example)
        qDebug() << "= New parameters =";
        qDebug() << "Device name            : " << port->deviceName();
        qDebug() << "Baud rate              : " << port->baudRate();
        qDebug() << "Data bits              : " << port->dataBits();
        qDebug() << "Parity                 : " << port->parity();
        qDebug() << "Stop bits              : " << port->stopBits();
        qDebug() << "Flow                   : " << port->flowControl();
        qDebug() << "Char timeout, msec     : " << port->charIntervalTimeout();
    }
    ~Notifier() {
        port->close();
    }
private slots:
    void slotRead() {
        QByteArray ba = port->readAll();
        qDebug() << "Slot: Rx ready notifier.";
    }
    void slotWrite() {
        qDebug() << "Slot: Tx empty notifier.";
    }
    void slotException() {
        qDebug() << "Slot: Exception notifier.";
    }
    void slotCts(bool value) {
        qDebug() << "Slot: Cts notifier. Value: " << value;
    }
    void slotDsr(bool value) {
        qDebug() << "Slot: Dsr notifier. Value: " << value;
    }
    void slotRing(bool value) {
        qDebug() << "Slot: Ring notifier. Value: " << value;
    }

private:
    AbstractSerial *port;
};

#endif
