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
