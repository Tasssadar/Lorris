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

#ifndef STORAGE_H
#define STORAGE_H

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

class WidgetArea;
class DeviceTabWidget;
class QFile;
class LorrisAnalyzer;
class DataFileParser;

class Storage : public QObject
{
    Q_OBJECT

public:
    explicit Storage(LorrisAnalyzer *analyzer);
    ~Storage();

    void setPacket(analyzer_packet *packet);

    void Clear();

    void addData(analyzer_data *data);
    quint32 getSize() { return m_size; }
    analyzer_data *get(quint32 index) { return m_data[index]; }
    analyzer_packet *loadFromFile(QString *name, quint8 load, WidgetArea *area, DeviceTabWidget *devices, quint32 &data_idx);

    const QString& getFilename() { return m_filename; }

public slots:
    void SaveToFile(QString filename, WidgetArea *area, DeviceTabWidget *devices);
    void SaveToFile(WidgetArea *area, DeviceTabWidget *devices);

private:
    bool checkMagic(DataFileParser *file);

    std::vector<analyzer_data*> m_data;
    analyzer_packet *m_packet;
    quint32 m_size;
    LorrisAnalyzer *m_analyzer;

    QString m_filename;
    QByteArray m_file_md5;
};

#endif // STORAGE_H
