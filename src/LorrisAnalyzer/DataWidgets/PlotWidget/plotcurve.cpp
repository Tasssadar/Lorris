/**********************************************
**    This file is part of Lorris
**    http://tasssadar.github.com/Lorris/
**
**    See README and COPYING
***********************************************/

#include "dep/qcustomplot/qcustomplot.h"
#include "plotcurve.h"
#include "../../storage.h"

PlotCurve::PlotCurve(QCPGraph *graph, QObject *parent) :
    QObject(parent)
{
    m_graph = graph;
    m_last_index = 0;
    m_data_type = 0;
}

void PlotCurve::newData(quint32 index, Storage *storage)
{
    if(m_info.filter.isNull())
        return;

    analyzer_data cur;
    cur.setPacket(storage->getPacket());

    QVariant num;
    if(m_last_index > index)
    {
        m_graph->removeDataAfter(index);
        if(m_sample_size < index)
        {
            quint32 end = (std::min)(index, m_last_index-m_sample_size);
            quint32 i = m_sample_size < index ? index - m_sample_size : 0;
            for(; i <= end; ++i)
            {
                cur.setData(storage->get(i));

                if(!m_info.filter->isOkay(&cur))
                    continue;

                num = DataWidget::getNumFromPacket(&cur, m_info.pos, m_data_type);
                if(!num.isValid())
                    continue;
                m_graph->addData(i, num.toDouble());
            }
        }
    }
    else
    {
        if(m_sample_size < index)
            m_graph->removeDataBefore(index-m_sample_size);

        quint32 i = 0;
        if(m_sample_size < index)
            i = (std::max)(m_last_index, index - m_sample_size);
        else
            i = m_last_index;

        for(; i <= index; ++i)
        {
            cur.setData(storage->get(i));

            //if(!m_info.filter->isOkay(&cur))
              //  continue;

            if(cur.getData()[3] != 0x01)
                continue;

            num = DataWidget::getNumFromPacket(&cur, m_info.pos, m_data_type);
            if(!num.isValid())
                continue;
            m_graph->addData(i, num.toDouble());
        }
    }

    m_graph->rescaleAxes(true);
    m_last_index = index;
}
