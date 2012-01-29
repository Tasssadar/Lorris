#include "graphdata.h"
#include "../datawidget.h"
#include "../../analyzerdatastorage.h"

GraphData::GraphData(AnalyzerDataStorage *storage, data_widget_info &info, qint32 sample_size, quint8 data_type) : QwtSeriesData<QPointF>()
{
    m_storage = storage;
    m_info = info;
    m_sample_size = sample_size;

    m_max = -999999999;
    m_min = 99999999;
    m_data_pos = 0;
    m_data_type = data_type;
}

QPointF GraphData::sample(size_t i) const
{
    if(i < m_data.size())
        return QPointF(i, m_data[i]);
    else
        return QPointF(0, 0);
}

size_t GraphData::size() const
{
    return m_data.size();
}

QRectF GraphData::boundingRect() const
{
    return QRect(0, m_max, m_data.size(), abs(m_max) + abs(m_min));
}

void GraphData::setSampleSize(qint32 size)
{
    m_sample_size = size;

    quint32 pos = m_data_pos;
    m_data_pos = 0;
    m_data.clear();
    dataPosChanged(pos);
}

void GraphData::dataPosChanged(quint32 pos)
{
    if(pos == m_data_pos)
        return;

    qint32 absPos = abs(m_data_pos - pos);
    if(m_sample_size == -1)
    {
        if(m_data.size() > pos)
        {
            m_data.erase(m_data.begin()+pos, m_data.end());
            m_data_pos = pos;
            return;
        }
    }
    else if(absPos >= m_sample_size || pos < m_data_pos)
        m_data.clear();
    else if((qint32)m_data.size() >= m_sample_size && pos > m_data_pos && (qint32)m_data.size() > absPos)
        m_data.erase(m_data.begin(), m_data.begin()+absPos);

    m_data_pos = pos;


    quint32 i = 0;
    if(m_sample_size == -1)
        i = m_data.size();
    else if(absPos < (qint32)m_data.size())
    {
        qint32 len = std::min((qint32)absPos, m_sample_size);
        if((qint32)m_data_pos > len)
            i = m_data_pos - len;
    }
    else if((qint32)m_data_pos > m_sample_size)
        i = m_data_pos - m_sample_size;

    for(; i < m_data_pos; ++i)
    {
        analyzer_data *cur = m_storage->get(i);
        quint8 cmd, dev;

        if(m_info.command != -1 && (!cur->getCmd(cmd) || cmd != m_info.command))
            continue;

        if(m_info.device != -1 && (!cur->getDeviceId(dev) || dev != m_info.device))
            continue;

        try
        {
            double val = DataWidget::getNumFromPacket(cur, m_info.pos, m_data_type).toDouble();
            m_data.push_back(val);

            if(m_max < val)
                m_max = val;
            if(m_min > val)
                m_min = val;
        }
        catch(const char*) { }
    }
}

void GraphData::setDataType(quint8 type)
{
    m_data_type = type;

    quint32 pos = m_data_pos;
    m_data_pos = 0;
    m_data.clear();
    dataPosChanged(pos);
}

void GraphData::setInfo(data_widget_info &info)
{
    m_info = info;

    quint32 pos = m_data_pos;
    m_data_pos = 0;
    m_data.clear();
    dataPosChanged(pos);
}

