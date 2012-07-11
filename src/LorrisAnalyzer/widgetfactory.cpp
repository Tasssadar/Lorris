/**********************************************
**    This file is part of Lorris
**    http://tasssadar.github.com/Lorris/
**
**    See README and COPYING
***********************************************/

#include "widgetfactory.h"
#include "DataWidgets/datawidget.h"

WidgetFactory::WidgetFactory()
{
}

void WidgetFactory::addWidgetInit(quint32 type, widgetInit init)
{
    m_widgetInits.insert(type, init);
}

void WidgetFactory::addBtnInit(btnInit init)
{
    m_btnInits.push_back(init);
}

DataWidget *WidgetFactory::getWidget(quint32 type, QWidget *parent)
{
    if(!m_widgetInits.contains(type))
        return NULL;
    return m_widgetInits[type](parent);
}

std::vector<DataWidgetAddBtn*> WidgetFactory::getButtons(QWidget *parent)
{
    std::vector<DataWidgetAddBtn*> res;
    for(quint32 i = 0; i < m_btnInits.size(); ++i)
        res.push_back(m_btnInits[i](parent));
    return res;
}
