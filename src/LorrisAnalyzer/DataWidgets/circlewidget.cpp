/**********************************************
**    This file is part of Lorris
**    http://tasssadar.github.com/Lorris/
**
**    See README and COPYING
***********************************************/

#include <QPainter>
#include <math.h>
#include <QSignalMapper>

#include "circlewidget.h"
#include "../../ui/rangeselectdialog.h"

CircleWidget::CircleWidget(QWidget *parent) :
    DataWidget(parent)
{
    setTitle(tr("Circle"));
    setIcon(":/dataWidgetIcons/circle.png");

    m_widgetType = WIDGET_CIRCLE;

    m_circle = new CircleDraw(this);
    layout->addWidget(m_circle, 1);
    resize(150, 150);
}

CircleWidget::~CircleWidget()
{

}

void CircleWidget::setUp(Storage *storage)
{
    DataWidget::setUp(storage);

    m_range_min = 0;
    m_range_max = 4096;
    m_num_type = NUM_UINT8;

    QMenu *bitsMenu = contextMenu->addMenu(tr("Data type"));

    static const QString dataTypes[] =
    {
        tr("unsigned 8bit"),
        tr("unsigned 16bit"),
        tr("unsigned 32bit"),
        tr("unsigned 64bit"),

        tr("signed 8bit"),
        tr("signed 16bit"),
        tr("signed 32bit"),
        tr("signed 64bit"),

        tr("float (4 bytes)"),
        tr("double (8 bytes)")
    };

    QSignalMapper *signalMapBits = new QSignalMapper(this);
    for(quint8 i = 0; i < NUM_COUNT; ++i)
    {
        if(i%4 == 0 && i != 0)
            bitsMenu->addSeparator();

        m_bits_act[i] = bitsMenu->addAction(dataTypes[i]);
        m_bits_act[i]->setCheckable(true);
        signalMapBits->setMapping(m_bits_act[i], i);
        connect(m_bits_act[i], SIGNAL(triggered()), signalMapBits, SLOT(map()));
    }
    m_bits_act[0]->setChecked(true);
    connect(signalMapBits, SIGNAL(mapped(int)), SLOT(setNumType(int)));

    static const QString title[ANG_MAX] = {
        tr("Radians"),
        tr("Degrees"),
        tr("Range...")
    };

    QSignalMapper *map = new QSignalMapper(this);
    for(int i = 0; i < ANG_MAX; ++i)
    {
        m_type_act[i] = contextMenu->addAction(title[i]);
        m_type_act[i]->setCheckable(true);
        map->setMapping(m_type_act[i], i);
        connect(m_type_act[i], SIGNAL(triggered()), map, SLOT(map()));
    }
    changeAngType(ANG_RAD);
    connect(map, SIGNAL(mapped(int)), SLOT(angTypeChanged(int)));
}

void CircleWidget::saveWidgetInfo(DataFileParser *file)
{
    DataWidget::saveWidgetInfo(file);

    file->writeBlockIdentifier("circleWtype");
    file->write((char*)&m_ang_type, sizeof(m_ang_type));
    file->write((char*)&m_num_type, sizeof(m_num_type));

    file->writeBlockIdentifier("circleWrange");
    file->write((char*)&m_range_min, sizeof(m_range_min));
    file->write((char*)&m_range_max, sizeof(m_range_max));
}

void CircleWidget::loadWidgetInfo(DataFileParser *file)
{
    DataWidget::loadWidgetInfo(file);

    if(file->seekToNextBlock("circleWtype", BLOCK_WIDGET))
    {
        file->read((char*)&m_ang_type, sizeof(m_ang_type));
        file->read((char*)&m_num_type, sizeof(m_num_type));
    }

    if(file->seekToNextBlock("circleWrange", BLOCK_WIDGET))
    {
        file->read((char*)&m_range_min, sizeof(m_range_min));
        file->read((char*)&m_range_max, sizeof(m_range_max));
    }

    changeAngType(m_ang_type);
    setNumType(m_num_type);
}

void CircleWidget::setValue(const QVariant &var)
{
    m_circle->setAngle(toRad(var));
}

void CircleWidget::processData(analyzer_data *data)
{
    QVariant var = DataWidget::getNumFromPacket(data, m_info.pos, m_num_type);
    setValue(var);
}

void CircleWidget::angTypeChanged(int i)
{
    if(i == ANG_RANGE)
    {
        RangeSelectDialog dialog(m_range_min, m_range_max, INT_MAX, INT_MIN, this);
        dialog.exec();
        if(dialog.getRes())
        {
            m_range_min = dialog.getMin();
            m_range_max = dialog.getMax();
        }
    }
    changeAngType(i, m_range_min, m_range_max);
}

void CircleWidget::changeAngType(int i, int min, int max)
{
    for(int y = 0; y < ANG_MAX; ++y)
        m_type_act[y]->setChecked(y == i);

    m_ang_type = i;

    if(min != -1 || max != -1)
    {
        m_range_min = min;
        m_range_max = max;
    }

    emit updateData();
}

float CircleWidget::toRad(const QVariant& var)
{
    switch(m_ang_type)
    {
        case ANG_RAD:
            return var.toFloat();
        case ANG_DEG:
        {
            float val = var.toFloat();
            val = (val * M_PI*2) / 360.f;
            return val;
        }
        case ANG_RANGE:
        {
            float val = var.toFloat()-m_range_min;
            val = (val * M_PI*2) / (m_range_max-m_range_min);
            return val;
        }
    }
    return 0.0f;
}

void CircleWidget::setNumType(int i)
{
    for(quint8 y = 0; y < NUM_COUNT; ++y)
        m_bits_act[y]->setChecked(y == i);

    m_num_type = i;
    emit updateData();
}

CircleDraw::CircleDraw(QWidget *parent) : QWidget(parent)
{
    m_angle = 0;
}

void CircleDraw::paintEvent(QPaintEvent *)
{
    QPainter p(this);
    QPen pen(Qt::black);
    pen.setWidth(4);
    p.setPen(pen);

    int r = (std::min(width(), height()) / 2) - 5;
    QPoint center(width()/2, height()/2);

    p.drawEllipse(center, r, r);

    QPoint rot(center);
    center.rx() += sin(m_angle)*r;
    center.ry() -= cos(m_angle)*r;
    p.drawLine(center, rot);
}

void CircleDraw::setAngle(float ang)
{
    ang = std::min(float(M_PI*2), ang);
    ang = std::max(ang, 0.f);
    m_angle = ang;
    update();
}

CircleWidgetAddBtn::CircleWidgetAddBtn(QWidget *parent) : DataWidgetAddBtn(parent)
{
    setText(tr("Circle"));
    setIconSize(QSize(17, 17));
    setIcon(QIcon(":/dataWidgetIcons/circle.png"));

    m_widgetType = WIDGET_CIRCLE;
}

