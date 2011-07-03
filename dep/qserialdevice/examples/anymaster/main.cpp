/* AnyMaster
*
* This application is part of the examples on the use of the library QSerialDevice.
*
* AnyMaster - a test console application.
* This application serves as the host, ie sends to the serial port requests and is awaiting replies.
* Queries are a set of Random bytes in length in each transaction (each step) increments from MIN (1 byte) to MAX (5000 byte).
* Selecting the MIN and MAX limits the length of the request - is arbitrary.
* This application should work in conjunction with the slave - AnySlave.
*
* Copyright (C) 2009  Denis Shienkov
*
* Contact Denis Shienkov:
*          e-mail: <scapig2@yandex.ru>
*             ICQ: 321789831
*/


#include <QtCore/QCoreApplication>
#include "anymaster.h"

int main(int argc, char *argv[])
{
    QCoreApplication app(argc, argv);
    AnyMaster am;
    am.start(true);
    return app.exec();
}
