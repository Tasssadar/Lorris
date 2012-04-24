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

class QComboBox;
class SerialPortThread;

class SerialPort : public PortConnection
{
    Q_OBJECT

Q_SIGNALS:
    void stopThread();

public:
    SerialPort();
    virtual ~SerialPort();

    virtual QString details() const;

    bool Open();
    void Close();
    void OpenConcurrent();

    void SendData(const QByteArray &data);

    BaudRateType baudRate() const { return m_rate; }
    void setBaudRate(BaudRateType value) { m_rate = value; emit changed(); }

    QString deviceName() const { return m_deviceName; }
    void setDeviceName(QString const & value);

    QString friendlyName() const { return m_deviceName; }
    void setFriendlyName(QString const & value);

    bool devNameEditable() const { return m_devNameEditable; }
    void setDevNameEditable(bool value) { m_devNameEditable = value; }

    QHash<QString, QVariant> config() const;
    bool applyConfig(QHash<QString, QVariant> const & config);

private slots:
    void connectResultSer(bool opened);
    void openResult();
    void readyRead();

private:
    bool openPort();

    QString m_deviceName;
    QString m_friendlyName;

    QextSerialPort *m_port;
    BaudRateType m_rate;

    QFuture<bool> m_future;
    QFutureWatcher<bool> m_watcher;

    QMutex m_port_mutex;
    bool m_devNameEditable;
};

#endif // SERIALPORT_H
