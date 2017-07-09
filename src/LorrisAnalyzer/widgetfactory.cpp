/**********************************************
**    This file is part of Lorris
**    http://tasssadar.github.com/Lorris/
**
**    See README and COPYING
***********************************************/

#include <algorithm>
#include <QApplication>

#include "widgetfactory.h"
#include "DataWidgets/datawidget.h"
#include "widgetarea.h"

WidgetFactory::WidgetFactory()
{
    for(int i = 0; i < WIDGET_MAX; ++i)
        m_widgetInits[i] = NULL;
    m_names.resize(WIDGET_MAX);
}

void WidgetFactory::addWidgetInit(quint32 type, widgetInit init)
{
    Q_ASSERT(type < WIDGET_MAX);
    m_widgetInits[type] = init;
}

void WidgetFactory::addScriptEnum(const char *text, quint32 val)
{
    m_scriptEnums[QString(text)] = val;
}

void WidgetFactory::addWidgetName(quint32 id, const char *name)
{
    m_names[id] = name;
}

DataWidget *WidgetFactory::getWidget(quint32 type, QWidget *parent)
{
    if(type >= WIDGET_MAX || !m_widgetInits[type])
        return NULL;

    translateWidgetNames();

    DataWidget *w = m_widgetInits[type](parent);
    w->setType(type);
    w->setTitle(m_namesTranslated[type]);
    w->setIcon(QString(":/dataWidgetIcons/%1").arg(m_names[type]));
    return w;
}

std::vector<DataWidgetAddBtn*> WidgetFactory::getButtons(QWidget *parent)
{
    translateWidgetNames();

    std::vector<DataWidgetAddBtn*> res;
    for(quint32 i = 0; i < WIDGET_MAX; ++i)
    {
        DataWidgetAddBtn *b = new DataWidgetAddBtn(i, m_namesTranslated[i], parent);
        b->setIcon(QIcon(QString(":/dataWidgetIcons/%1").arg(m_names[i])));
        res.push_back(b);
    }
    return res;
}

DataWidget *WidgetFactory::copy(DataWidget *w)
{
    Q_ASSERT(w);
    if(!w)
        return NULL;

    DataWidget *res = ((WidgetArea*)w->parent())->addWidget(w->pos(), w->getWidgetType());

    QByteArray data;
    DataFileParser p(&data, QIODevice::ReadWrite);

    w->saveWidgetInfo(&p);
    p.seek(0);
    res->loadWidgetInfo(&p);

    res->updateForThis();

    res->resize(w->size());
    return res;
}

void WidgetFactory::translateWidgetNames()
{
    if(!m_namesTranslated.empty())
        return;

    for(size_t i = 0; i < m_names.size(); ++i)
    {
        std::string name = m_names[i].toStdString();
        m_namesTranslated.push_back(qApp->translate("DataWidget", name.c_str()));
    }
}

const std::vector<QString>& WidgetFactory::getWidgetNames() const
{
    return m_names;
}

const std::vector<QString>& WidgetFactory::getTranslatedWidgetNames()
{
    translateWidgetNames();
    return m_namesTranslated;
}
