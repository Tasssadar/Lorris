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

#ifndef SERIALPORT_H
#define SERIALPORT_H

#include <QFuture>
#include <QFutureWatcher>
#include <qextserialport.h>

#include "connection.h"

class SerialPortThread;

class SerialPort : public Connection
{
    Q_OBJECT

Q_SIGNALS:
    void stopThread();

public:
    explicit SerialPort();
    virtual ~SerialPort();

    bool Open();
    void Close();
    void SendData(const QByteArray &data);
    void SetNameAndRate(QString name, BaudRateType rate)
    {
        m_idString = name;
        m_rate = rate;
    }
    void OpenConcurrent();

private slots:
    void connectResultSer(bool opened);
    void openResult();

private:
    bool openPort();

    QextSerialPort *m_port;
    SerialPortThread *m_thread;
    BaudRateType m_rate;

    QFuture<bool> m_future;
    QFutureWatcher<bool> m_watcher;
};



#endif // SERIALPORT_H
