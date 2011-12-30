#ifndef ANALYZERDATASTORAGE_H
#define ANALYZERDATASTORAGE_H

#include <QTypeInfo>
#include <vector>
#include <QByteArray>

#include "packet.h"

class AnalyzerDataStorage
{
public:
    AnalyzerDataStorage();
    ~AnalyzerDataStorage();

    void setPacket(analyzer_packet *packet)
    {
        m_packet = packet;
    }

    analyzer_data *addData(QByteArray data);
    quint32 getSize() { return m_size; }
    analyzer_data *get(quint32 index) { return m_data[index]; }

private:
    std::vector<analyzer_data*> m_data;
    analyzer_packet *m_packet;
    quint32 m_size;
};

#endif // ANALYZERDATASTORAGE_H
