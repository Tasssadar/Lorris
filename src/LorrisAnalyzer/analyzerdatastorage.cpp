#include "analyzerdatastorage.h"

AnalyzerDataStorage::AnalyzerDataStorage()
{
    m_packet = NULL;
    m_size = 0;
}

AnalyzerDataStorage::~AnalyzerDataStorage()
{
    for(quint32 i = 0; i < m_data.size(); ++i)
        delete m_data[i];
    m_data.clear();
}

analyzer_data *AnalyzerDataStorage::addData(QByteArray data)
{
    if(!m_packet)
        return NULL;
    analyzer_data *a_data = new analyzer_data(m_packet);
    a_data->setData(data);
    m_data.push_back(a_data);
    ++m_size;
    return a_data;
}
