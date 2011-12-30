#include <QScrollArea>
#include <QList>

#include "structuretabwidget.h"
#include "labellayout.h"
#include "common.h"

StructureTabWidget::StructureTabWidget(QWidget *parent) :
    QTabWidget(parent)
{
    m_id_enabled = false;
}

StructureTabWidget::~StructureTabWidget()
{
    removeAll();
}

void StructureTabWidget::removeAll()
{
    for(dev_map::iterator itr = m_devices.begin(); itr != m_devices.end(); ++itr)
        delete itr->second;
    m_devices.clear();
    clear();
}

void StructureTabWidget::addDevice(quint8 id)
{
    QWidget *w = new QWidget();
    ScrollDataLayout *layout = new ScrollDataLayout(m_header, false, true, w);
    QScrollArea *area = new QScrollArea(this);
    area->setWidget(w);
    area->setHorizontalScrollBarPolicy(Qt::ScrollBarAlwaysOn);

    QString name;
    if(m_id_enabled)
        name = tr("Device") + " " + Utils::hexToString(id, true);
    else
        name = "Default device";
    addTab(area, name);

    m_devices.insert(std::make_pair<quint8,ScrollDataLayout*>(id, layout));
}

void StructureTabWidget::handleData(analyzer_data *data)
{
    quint8 id = 0;
    if(m_id_enabled && !data->getDeviceId(id))
        return;

    dev_map::iterator itr = m_devices.find(id);
    if(itr == m_devices.end())
        return;
    itr->second->SetData(data->getData());
}
