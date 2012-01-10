/****************************************************************************
**
**    This file is part of Lorris.
**    Copyright (C) 2012 Vojtěch Boček
**
**    Contact: <vbocek@gmail.com>
**             https://github.com/Tasssadar
**
**    Lorris is free software: you can redistribute it and/or modify
**    it under the terms of the GNU General Public License as published by
**    the Free Software Foundation, either version 3 of the License, or
**    (at your option) any later version.
**
**    Lorris is distributed in the hope that it will be useful,
**    but WITHOUT ANY WARRANTY; without even the implied warranty of
**    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
**    GNU General Public License for more details.
**
**    You should have received a copy of the GNU General Public License
**    along with Lorris.  If not, see <http://www.gnu.org/licenses/>.
**
****************************************************************************/

#ifndef SERIALPORTTHREAD_H
#define SERIALPORTTHREAD_H

#include <QThread>
#include <QMutex>

#include "connection.h"
#include "qserialdevice/abstractserial.h"
#include "serialport.h"

class SerialPortThread : public QThread
{
    Q_OBJECT

Q_SIGNALS:
    void connectResult(bool opened);
    void dataRead(QByteArray data);

public:
    SerialPortThread(QString name, AbstractSerial::BaudRate rate, SerialPort *con);
    ~SerialPortThread();

    bool Open();
    void Send(const QByteArray &data);

    void run();

private slots:
    void viewStateSlot(QString stateMsg, QDateTime dt);
    void stop();

private:
    volatile bool runTh;
    AbstractSerial *m_port;
    AbstractSerial::BaudRate m_rate;
    QString devName;
    bool opened;
    quint8 m_writeErrorCount;

    SerialPort *m_con;
    QMutex m_portMutex;
};
#endif // SERIALPORTTHREAD_H
