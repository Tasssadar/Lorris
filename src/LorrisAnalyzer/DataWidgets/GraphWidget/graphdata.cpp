/**********************************************
**    This file is part of Lorris
**    http://tasssadar.github.com/Lorris/
**
**    See README and COPYING
***********************************************/

#include "graphdata.h"
#include "../datawidget.h"
#include "../../storage.h"

GraphDataSimple::GraphDataSimple() : QwtSeriesData<QPointF>()
{
    resetMinMax();
}

GraphDataSimple::~GraphDataSimple()
{
    clear();
}

QPointF GraphDataSimple::sample(size_t i) const
{
    if(i < m_data.size())
        return QPointF(i, m_data[i]->val);
    else
        return QPointF(0, 0);
}

size_t GraphDataSimple::size() const
{
    return m_data.size();
}

QRectF GraphDataSimple::boundingRect() const
{
    return QRect(0, m_max, m_data.size(), abs(m_max) + abs(m_min));
}

void GraphDataSimple::setMinMax(double val)
{
    if(val > m_max)      m_max = val;
    else if(val < m_min) m_min = val;
}

void GraphDataSimple::resetMinMax()
{
    m_max = INT_MIN;
    m_min = INT_MAX;
}

void GraphDataSimple::addPoint(quint32 index, qreal data)
{
    QHash<quint32, graph_data_st*>::iterator itr = m_indexes.find(index);
    if(itr != m_indexes.end())
        (*itr)->val = data;
    else
    {
        graph_data_st *dta = new graph_data_st(data, index);
        m_indexes.insert(index, dta);

        if(m_data.empty() || index > m_data.back()->itr)
            m_data.push_back(dta);
        else if(index < m_data.front()->itr)
            m_data.push_front(dta);
        else
        {
            for(storage::iterator itr = m_data.begin(); itr != m_data.end(); ++itr)
            {
                if((*itr)->itr < index)
                {
                    m_data.insert(++itr, dta);
                    break;
                }
            }
        }
    }
}

void GraphDataSimple::clear()
{
    for(storage::iterator itr = m_data.begin(); itr != m_data.end(); ++itr)
        delete *itr;
    m_data.clear();
    m_indexes.clear();
}

GraphData::GraphData(Storage *storage, data_widget_info &info, qint32 sample_size, quint8 data_type) :
    GraphDataSimple()
{
    m_storage = storage;
    m_info = info;
    m_sample_size = sample_size;

    m_data_pos = 0;
    m_data_type = data_type;
}

void GraphData::setSampleSize(qint32 size)
{
    m_sample_size = size;

    quint32 pos = m_data_pos;
    m_data_pos = 0;
    clear();
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

        v = DataWidget::getNumFromPacket(cur, m_info.pos, m_data_type);

        if(!v.isValid())
            continue;

        qreal val = v.toReal();
        m_data.push_back(new graph_data_st(val, i));
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
                if(m_data[i]->itr < pos)
                    break;

            if(i == -1)
                i = 0;

            for(storage::iterator itr = m_data.begin()+i; itr != m_data.end(); ++itr)
                delete *itr;
            m_data.erase(m_data.begin()+i, m_data.end());
            m_data_pos = pos;
            return;
        }
    }
    else if(absPos >= m_sample_size || pos < m_data_pos)
    {
        clear();
    }
    else if(pos > m_data_pos && ((qint32)m_data.size() >= m_sample_size || (qint32)m_data.size() > absPos))
    {
        quint32 i = 0;
        for(; i < m_data.size(); ++i)
            if(m_data[i]->itr > pos)
                break;

        for(storage::iterator itr = m_data.begin(); itr != m_data.begin()+i; ++itr)
            delete *itr;
        m_data.erase(m_data.begin(), m_data.begin()+i);
    }
}

quint32 GraphData::getStorageBegin(qint32 absPos)
{
    if(m_sample_size == -1 && !m_data.empty())
    {
        return m_data.back()->itr+1;
    }
    else if(absPos < (qint32)m_data.size())
    {
        qint32 len = std::min((qint32)absPos, m_sample_size);
        if((qint32)m_data_pos > len)
            return m_data_pos - len;
    }
    else if((qint32)m_data.size() < m_sample_size)
    {
        if(m_data_pos > (quint32)m_sample_size)
            return m_data_pos - m_sample_size + m_data.size();

    }

    return 0;
}

void GraphData::setDataType(quint8 type)
{
    m_data_type = type;

    quint32 pos = m_data_pos;
    m_data_pos = 0;
    clear();
    dataPosChanged(pos);
}

void GraphData::setInfo(data_widget_info &info)
{
    m_info = info;

    quint32 pos = m_data_pos;
    m_data_pos = 0;
    clear();
    dataPosChanged(pos);
}
