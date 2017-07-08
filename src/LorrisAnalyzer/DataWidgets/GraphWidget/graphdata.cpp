/**********************************************
**    This file is part of Lorris
**    http://tasssadar.github.com/Lorris/
**
**    See README and COPYING
***********************************************/

#include <utility>

#include "graphdata.h"
#include "../datawidget.h"
#include "../../storage.h"

GraphData::GraphData(Storage *storage, data_widget_info &info, qint32 sample_size, quint8 data_type) :
    QwtSeriesData<QPointF>()
{
    m_storage = storage;
    m_info = info;

    m_sample_size = sample_size;
    m_sample_offset = UINT32_MAX;
    m_data_type = data_type;

    m_data_start = 0;
    m_data_end = 0;
    m_last_index = 0;
    m_min = m_max = 0.0;

    m_script_based = false;
}

GraphData::~GraphData()
{

}

void GraphData::clear()
{
    m_data.clear();
    m_data_start = m_data_end = 0;
    m_min = m_max = 0.0;
}

void GraphData::reloadData(bool force)
{
    if(m_script_based)
        return;

    if(force) {
        quint32 idx = m_last_index;
        clear();
        dataPosChanged(idx);
    } else {
        dataPosChanged(m_last_index);
    }
}

QPointF GraphData::sample(size_t i) const
{
    return m_data[i];
}

size_t GraphData::size() const
{
    return m_data.size();
}

QRectF GraphData::boundingRect() const
{
    if(m_data.empty())
        return QRect();
    else
        return QRect(m_data.front().x(), m_max, m_data.back().x() - m_data.front().x(), abs(m_max) + abs(m_min));
}

quint32 GraphData::getMaxX()
{
    if(m_data.empty())
        return 0;

    return m_data.back().x();
}

void GraphData::setSampleSize(quint32 size, quint32 offset)
{
    if(m_script_based || (m_sample_size == size && m_sample_offset == offset))
        return;

    m_sample_size = size;
    m_sample_offset = offset;
    reloadData(false);
}

void GraphData::setSampleOffset(quint32 offset)
{
    if(!m_script_based)
        m_sample_offset = offset;
}

void GraphData::setDataType(quint8 type)
{
    m_data_type = type;
    reloadData(true);
}

void GraphData::setInfo(data_widget_info &info)
{
    m_info = info;
    reloadData(true);
}

QPointF GraphData::getPointAtIdx(quint32 idx)
{
    analyzer_data cur(m_storage->get(idx), m_storage->getPacket());

    if(!m_info.filter->isOkay(&cur) || m_info.pos >= cur.getData().size())
        return QPointF(-1, 0);

    QVariant num = DataWidget::getNumFromPacket(&cur, m_info.pos, m_data_type);
    if(!num.isValid())
        return QPointF(-1, 0);

    double n = num.toDouble();
    if(m_eval.isActive())
        n = m_eval.evaluate(QString::number(n, 'f')).toDouble();

    setMinMax(n);

    return QPointF(idx, n);
}

void GraphData::dataPosChanged(quint32 index)
{
    if(m_info.filter.isNull() || m_storage->isEmpty())
    {
        clear();
        return;
    }

    m_last_index = index;

    if(index > m_sample_offset && m_sample_offset < m_storage->getMaxIdx()) {
        index = m_sample_offset;
    }

    // FIXME: this can be solved better, somehow
    if(m_data_end == index && m_storage->isFull())
    {
        m_data.clear();
        m_data_start = 0;
        m_data_end = 0;
    }

    // calc new range
    const quint32 start = (m_sample_size < index) ? index - m_sample_size : 0;
    const quint32 end = (std::min)(index+1, m_storage->getMaxIdx()+1);

    if(start >= m_data_end || end <= m_data_start) {
        m_data.clear();
        for(quint32 i = start; i < end; ++i) {
            auto p = getPointAtIdx(i);
            if(p.x() >= 0.0)
                m_data.push_back(p);
        }
    } else {
        if(start > m_data_start)
            removeDataBefore(start);
        if(end < m_data_end)
            removeDataAfter(end-1);

        // fill before prev range
        if(m_data_start > 0) {
            const quint32 start_limit = (std::min)(m_data_start-1, m_storage->getMaxIdx());
            for(quint32 i = start_limit; i >= start; --i) {
                auto p = getPointAtIdx(i);
                if(p.x() >= 0.0)
                    m_data.push_front(p);

                if(i == 0)
                    break;
            }
        }

        // fill after prev range
        const quint32 end_limit = (std::max)(m_data_end, start);
        for(quint32 i = end_limit; i < end; ++i) {
            auto p = getPointAtIdx(i);
            if(p.x() >= 0.0)
                m_data.push_back(p);
        }
    }

    /*qreal lastx = -1;
    for(size_t i = 0; i < m_data.size(); ++i)
    {
        Q_ASSERT(m_data[i].x() > lastx);
        lastx = m_data[i].x();
    }
    Q_ASSERT(m_data.size() == (end - start));
    */

    m_data_start = start;
    m_data_end = end;
}

void GraphData::removeDataAfter(quint32 index)
{
    for(size_t i = 0; i < m_data.size(); ++i)
    {
        if(m_data[i].x() == index)
        {
            ++i;
            m_data.erase(m_data.begin()+i, m_data.end());
            return;
        }
        else if(m_data[i].x() > index)
        {
            m_data.clear();
            return;
        }
    }
}

void GraphData::removeDataBefore(quint32 index)
{
    for(size_t i = 1; i < m_data.size(); ++i)
    {
        if(m_data[i].x() == index)
        {
            m_data.erase(m_data.begin(), m_data.begin()+i);
            return;
        }
        else if(m_data[i].x() > index)
        {
            return;
        }
    }

    m_data.clear();
}

void GraphData::setMinMax(double val)
{
    if(m_data.empty())
        m_min = m_max = val;
    else
    {
        if(val < m_min)
            m_min = val;
        else if(val > m_max)
            m_max = val;
    }
}

void GraphData::addPoint(qreal index, qreal data)
{
    setMinMax(data);

    if(m_data.empty() || m_data.back().x() < index)
        m_data.push_back(QPointF(index, data));
    else if(m_data.front().x() > index)
        m_data.push_front(QPointF(index, data));
    else
    {
        for(DataMapItr itr = m_data.begin(); itr != m_data.end(); ++itr)
        {
            if((*itr).x() == index)
            {
                (*itr).ry() = data;
                break;
            }
            else if ((*itr).x() < index && (*(itr+1)).x() > index)
            {
                m_data.insert(itr+1, QPointF(index, data));
                break;
            }
        }
    }
}
