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

#ifndef ANALYZERDATASTORAGE_H
#define ANALYZERDATASTORAGE_H

#include <QTypeInfo>
#include <vector>
#include <QByteArray>
#include <QObject>

#include "packet.h"

enum StorageDataType
{
    STORAGE_STRUCTURE = 0x01,
    STORAGE_DATA      = 0x02,
    STORAGE_WIDGETS   = 0x04
};

class AnalyzerDataArea;
class DeviceTabWidget;
class QFile;

class AnalyzerDataStorage : public QObject
{
    Q_OBJECT

public:
    explicit AnalyzerDataStorage();
    ~AnalyzerDataStorage();

    void setPacket(analyzer_packet *packet)
    {
        m_packet = packet;
    }

    void Clear();

    void addData(analyzer_data *data);
    quint32 getSize() { return m_size; }
    analyzer_data *get(quint32 index) { return m_data[index]; }
    analyzer_packet *loadFromFile(QString *name, quint8 load, AnalyzerDataArea *area, DeviceTabWidget *devices);

public slots:
    void SaveToFile(AnalyzerDataArea *area, DeviceTabWidget *devices);

private:
    bool checkMagic(QFile *file);

    std::vector<analyzer_data*> m_data;
    analyzer_packet *m_packet;
    quint32 m_size;
};

#endif // ANALYZERDATASTORAGE_H
