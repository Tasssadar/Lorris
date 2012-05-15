/**********************************************
**    This file is part of Lorris
**    http://tasssadar.github.com/Lorris/
**
**    See README and COPYING
***********************************************/

#ifndef STORAGE_H
#define STORAGE_H

#include <QTypeInfo>
#include <vector>
#include <QByteArray>
#include <QObject>
#include <QByteArray>

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
