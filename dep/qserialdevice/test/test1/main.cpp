/* Test
*
*/

#include <QtCore/QDebug>
#include <QtCore/QCoreApplication>
#include <abstractserial.h>
#include <iostream>

using namespace std;

int main(int argc, char *argv[])
{
    QCoreApplication app(argc, argv);

    AbstractSerial port1;
    port1.setDeviceName("/dev/ttyS0");
    bool ret = port1.open(QIODevice::ReadWrite);
    if (ret) {
        ret = port1.setBaudRate(QString("115200 baud"));
        if (ret)
            qDebug() << port1.baudRate();
        ret = port1.setBaudRate(AbstractSerial::BaudRate19200);
        if (ret)
            qDebug() << port1.baudRate();
        ret = port1.setBaudRate(9600);
        if (ret)
            qDebug() << port1.baudRate();
        ret = port1.setBaudRate(10000);
        if (ret)
            qDebug() << port1.baudRate();
    }

    port1.close();

    return app.exec();
}
