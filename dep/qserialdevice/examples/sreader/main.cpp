/* SReader
*
* This application is part of the examples on the use of the library QSerialDevice.
*
* sreader - a test console application to read data from the port using the signal readyRead()
*
* Copyright (C) 2009  Denis Shienkov
*
* Contact Denis Shienkov:
*          e-mail: <scapig2@yandex.ru>
*             ICQ: 321789831
*/

#include <QtCore>

#include "sreader.h"

int main(int argc, char *argv[])
{
    QCoreApplication app(argc, argv);

    Reader MyReader;

    return app.exec();
}
