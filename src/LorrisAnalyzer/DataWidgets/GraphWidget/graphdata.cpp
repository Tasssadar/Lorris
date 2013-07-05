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
    m_data_type = data_type;

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
    m_last_index = 0;
    m_min = m_max = 0.0;
}

void GraphData::reloadData()
{
    if(m_script_based)
        return;

    quint32 idx = m_last_index;
    clear();
    dataPosChanged(idx);
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

void GraphData::setSampleSize(quint32 size)
{
    if(m_script_based)
        return;

    m_sample_size = size;
    reloadData();
}

void GraphData::setDataType(quint8 type)
{
    m_data_type = type;
    reloadData();
}

void GraphData::setInfo(data_widget_info &info)
{
    m_info = info;
    reloadData();
}

void GraphData::dataPosChanged(quint32 index)
{
    if(m_info.filter.isNull() || m_storage->isEmpty())
    {
        clear();
        return;
    }

    double n;
    QVariant num;
    analyzer_data cur(NULL, m_storage->getPacket());

    // FIXME: this can be solved better, somehow
    if(m_last_index == index && m_storage->isFull())
    {
        m_data.clear();
        m_last_index = 0;
    }

    if(m_last_index > index)
    {
        removeDataAfter(index);

        if(m_sample_size < index)
        {
            quint32 i = (std::min)(index, m_last_index-m_sample_size);
            qint64 start = m_sample_size <= index ? index - m_sample_size : -1;
            for(; i > start; --i)
            {
                cur.setData(m_storage->get(i));

                if(!m_info.filter->isOkay(&cur))
                    continue;

                num = DataWidget::getNumFromPacket(&cur, m_info.pos, m_data_type);
                if(!num.isValid())
                    continue;

                n = num.toDouble();
                setMinMax(n);

                m_data.push_front(QPointF(i, n));
            }
        }
    }
    else
    {
        if(m_sample_size < index)
            removeDataBefore(index-m_sample_size+1);

        quint32 i = 0;
        if(m_sample_size < index)
            i = (std::max)(m_last_index, index - m_sample_size);
        else
            i = m_last_index;

        for(++i; i <= index; ++i)
        {
            cur.setData(m_storage->get(i));

            if(!m_info.filter->isOkay(&cur))
                continue;

            num = DataWidget::getNumFromPacket(&cur, m_info.pos, m_data_type);
            if(!num.isValid())
                continue;

            n = num.toDouble();
            if(m_eval.isActive())
                n = m_eval.evaluate(QString::number(n, 'f')).toDouble();

            setMinMax(n);

            m_data.push_back(QPointF(i, n));
        }
    }

    m_last_index = index;
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
