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
class FilterTabWidget;
class QFile;
class LorrisAnalyzer;
class DataFileParser;

class Storage : public QObject
{
    Q_OBJECT

public:
    typedef std::vector<QByteArray> DataVector;

    explicit Storage(LorrisAnalyzer *analyzer);
    ~Storage();

    void setPacket(analyzer_packet *packet);
    analyzer_packet *getPacket() const { return m_packet; }

    void Clear();

    QByteArray *addData(const QByteArray& data);
    quint32 getSize() const { return m_size; }
    quint32 getMaxIdx() const { return m_size ? m_size -1 : 0; }
    bool isEmpty() const { return m_size == 0; }
    QByteArray *get(quint32 index) { return &m_data[index]; }
    analyzer_packet *loadFromFile(QString *name, quint8 load, WidgetArea *area, FilterTabWidget *filters, quint32 &data_idx);

    const QString& getFilename() { return m_filename; }
    void clearFilename() { m_filename.clear(); }

public slots:
    void SaveToFile(QString filename, WidgetArea *area, FilterTabWidget *filters);
    void SaveToFile(WidgetArea *area, FilterTabWidget *filters);
    void ExportToBin(const QString& filename);

private:
    bool checkMagic(DataFileParser *file);
    void readLegacyStructure(DataFileParser *file, analyzer_packet *packet);

    DataVector m_data;
    analyzer_packet *m_packet;
    quint32 m_size;
    LorrisAnalyzer *m_analyzer;

    QString m_filename;
    QByteArray m_file_md5;
};

#endif // STORAGE_H
