/* AnySlave
*
* This application is part of the examples on the use of the library QSerialDevice.
*
* AnySlave - a test console application.
* This application serves as a slave owner, ie constantly waiting for a query from the serial device.
* When receiving a signal that the data are available for reading - the application reads them,
* and then sends back.
* This application should work in conjunction with the master - AnyMaster.
*
* Copyright (C) 2009  Denis Shienkov
*
* Contact Denis Shienkov:
*          e-mail: <scapig2@yandex.ru>
*             ICQ: 321789831
*/


#include <QtCore/QCoreApplication>

#include "anyslave.h"

int main(int argc, char *argv[])
{
    QCoreApplication app(argc, argv);

    AnySlave as;

    return app.exec();
}
