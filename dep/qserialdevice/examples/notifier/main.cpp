/* Notifier
*
* This application is part of the examples on the use of the library QSerialDevice.
*
* notifier - a test console application to using any the notification signals
*
* Copyright (C) 2009  Denis Shienkov
*
* Contact Denis Shienkov:
*          e-mail: <scapig2@yandex.ru>
*             ICQ: 321789831
*/

#include <QtCore>

#include "notifier.h"

int main(int argc, char *argv[])
{
    QCoreApplication app(argc, argv);

    Notifier MyNotifier;

    return app.exec();
}
