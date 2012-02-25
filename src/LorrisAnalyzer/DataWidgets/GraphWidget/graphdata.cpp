#include "graphdata.h"
#include "../datawidget.h"
#include "../../analyzerdatastorage.h"

GraphData::GraphData(AnalyzerDataStorage *storage, data_widget_info &info, qint32 sample_size, quint8 data_type) : QwtSeriesData<QPointF>()
{
    m_storage = storage;
    m_info = info;
    m_sample_size = sample_size;

    m_data_pos = 0;
    m_data_type = data_type;

    resetMinMax();
}

QPointF GraphData::sample(size_t i) const
{
    if(i < m_data.size())
        return QPointF(i, m_data[i].val);
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

    eraseSpareData(absPos, pos);

    m_data_pos = pos;

    analyzer_data *cur;
    quint8 cmd, dev;

    resetMinMax();

    QVariant v;

    for(quint32 i = getStorageBegin(absPos); i < m_data_pos && i < m_storage->getSize(); ++i)
    {
        cur = m_storage->get(i);

        if(m_info.command != -1 && (!cur->getCmd(cmd) || cmd != m_info.command))
            continue;

        if(m_info.device != -1 && (!cur->getDeviceId(dev) || dev != m_info.device))
            continue;

        v = DataWidget::getNumFromPacket(cur, m_info.pos, m_data_type).toReal();

        if(!v.isValid())
            continue;

        qreal val = v.toReal();
        m_data.push_back(graph_data_st(val, i));
        setMinMax(val);
    }
}

void GraphData::eraseSpareData(qint32 absPos, quint32 pos)
{
    if(m_sample_size == -1)
    {
        if(m_data_pos > pos)
        {
            int i = m_data.size()-1;
            for(; i != -1; --i)
                if(m_data[i].itr < pos)
                    break;

            m_data.erase(m_data.begin()+i, m_data.end());
            m_data_pos = pos;
            return;
        }
    }
    else if(absPos >= m_sample_size || pos < m_data_pos)
    {
        m_data.clear();
    }
    else if(pos > m_data_pos && ((qint32)m_data.size() >= m_sample_size || (qint32)m_data.size() > absPos))
    {
        quint32 i = 0;
        for(; i < m_data.size(); ++i)
            if(m_data[i].itr > pos)
                break;

        m_data.erase(m_data.begin(), m_data.begin()+i);
    }
}

quint32 GraphData::getStorageBegin(qint32 absPos)
{
    if(m_sample_size == -1 && !m_data.empty())
    {
        return m_data.back().itr+1;
    }
    else if(absPos < (qint32)m_data.size())
    {
        qint32 len = std::min((qint32)absPos, m_sample_size);
        if((qint32)m_data_pos > len)
            return m_data_pos - len;
    }
    else if((qint32)m_data.size() < m_sample_size)
    {
        return m_data_pos - m_sample_size + m_data.size();
    }

    return 0;
}

void GraphData::setMinMax(double val)
{
    if(val > m_max)      m_max = val;
    else if(val < m_min) m_min = val;
}

void GraphData::resetMinMax()
{
    m_max = -999999999;
    m_min = 99999999;
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
