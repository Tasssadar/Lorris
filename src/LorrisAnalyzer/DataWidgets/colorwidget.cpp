/**********************************************
**    This file is part of Lorris
**    http://tasssadar.github.com/Lorris/
**
**    See README and COPYING
***********************************************/

#include <QSlider>
#include <QAction>
#include <QMenu>
#include <QSignalMapper>
#include <QPainter>

#include "../../WorkTab/WorkTab.h"
#include "colorwidget.h"

void ColorWidget::addEnum()
{
    REGISTER_ENUM(COLOR_RGB_8);
    REGISTER_ENUM(COLOR_RGB_10);
    REGISTER_ENUM(COLOR_RGB_10_UINT);
    REGISTER_ENUM(COLOR_GRAY_8);
    REGISTER_ENUM(COLOR_GRAY_10);
}

REGISTER_DATAWIDGET(WIDGET_COLOR, Color, &ColorWidget::addEnum)

ColorWidget::ColorWidget(QWidget *parent) : DataWidget(parent)
{
    setTitle(tr("Color"));
    setIcon(":/dataWidgetIcons/color.png");

    m_widgetType = WIDGET_COLOR;

    m_widget = new ColorDisplay(this);
    m_widget->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Expanding);

    layout->addWidget(m_widget, 1);

    resize(180, 100);

    setMinimumSize(50, 50);

    m_brightness = 0;
    m_color_layout[0] = m_color_layout[1] = m_color_layout[2] = NULL;
    m_color_cor[0] = m_color_cor[1] = m_color_cor[2] = 0;
    m_color_type = COLOR_RGB_8;

    updateColor();
}

ColorWidget::~ColorWidget()
{
    delete m_widget;
}

void ColorWidget::setUp(Storage *storage)
{
    DataWidget::setUp(storage);

    QSignalMapper *map = new QSignalMapper(this);
    connect(map, SIGNAL(mapped(int)), SLOT(setColorType(int)));

    QMenu *typeMenu = contextMenu->addMenu(tr("Color type"));
    for(int i = 0; i < COLOR_MAX; ++i)
    {
        static const QString name[COLOR_MAX] = {
            tr("RGB (8b/channel, 3 uint8s)"),
            tr("RGB (10b/channel, 3 uint16s)"),
            tr("RGB (10b/channel, 1 uint32)"),
            tr("Shades of gray (8b/channel, 1 uint8)"),
            tr("Shades of gray (10b/channel, 1 uint16)")
        };

        colorType[i] = typeMenu->addAction(name[i], map, SLOT(map()));
        colorType[i]->setCheckable(true);
        map->setMapping(colorType[i], i);
    }
    setColorType(COLOR_RGB_8);

    textAct = new QAction(tr("Show RGB values"), this);
    textAct->setCheckable(true);
    connect(textAct, SIGNAL(triggered(bool)), this, SLOT(showValues(bool)));

    brightAct = new QAction(tr("Brightness correction"), this);
    brightAct->setCheckable(true);
    connect(brightAct, SIGNAL(triggered()), this, SLOT(brightTriggered()));

    colorAct = new QAction(tr("Color correction"), this);
    colorAct->setCheckable(true);
    connect(colorAct , SIGNAL(triggered()), this, SLOT(colorTriggered()));

    contextMenu->addAction(textAct);
    contextMenu->addAction(brightAct);
    contextMenu->addAction(colorAct);

    m_brightness_layout = NULL;
}

void ColorWidget::processData(analyzer_data *data)
{
    try
    {
        switch(m_color_type)
        {
            case COLOR_RGB_8:
                for(quint8 i = 0; i < 3; ++i)
                    m_widget->color()[i] = data->getUInt8(m_info.pos + i);
                break;
            case COLOR_RGB_10:
                for(quint8 i = 0; i < 3; ++i)
                    m_widget->color()[i] = (data->getUInt16(m_info.pos + i) & 0x3FF);
                break;
            case COLOR_RGB_10_UINT:
            {
                quint32 color = data->getUInt32(m_info.pos);
                m_widget->color()[0] = ((color >> 20) & 0x3FF);
                m_widget->color()[1] = ((color >> 10) & 0x3FF);
                m_widget->color()[2] = ((color) & 0x3FF);
                break;
            }
            case COLOR_GRAY_8:
                m_widget->setAll(data->getUInt8(m_info.pos));
                break;
            case COLOR_GRAY_10:
                m_widget->setAll(data->getUInt16(m_info.pos));
                break;
            default:
                return;
        }

        updateColor();
    }
    catch(const char*) { }
}

void ColorWidget::updateColor()
{
    qint16 cor[] = { m_brightness, 0 };
    QRgb res = 0;
    bool tenBit = is10bit();
    for(quint8 i = 0; i < 3; ++i)
    {
        quint16 color = m_widget->color()[i];
        if(tenBit)
            color >>= 2;

        cor[1] = m_color_cor[i];

        for(quint8 y = 0; y < 2; ++y)
        {
            if     (color + cor[y] > 0xFF) color = 0xFF;
            else if(color + cor[y] < 0)    color = 0x00;
            else                           color += cor[y];
        }

        res |= (color << (2 - i)*8);
    }

    QPalette p = m_widget->palette();
    p.setColor(QPalette::Window, QColor(res));
    m_widget->setPalette(p);

    m_widget->update();
}

void ColorWidget::setValue(int r, int g, int b)
{
    m_widget->color()[0] = r;
    m_widget->color()[1] = g;
    m_widget->color()[2] = b;

    updateColor();
}

void ColorWidget::setValue(quint32 color)
{
    int r, g, b;
    switch(m_color_type)
    {
        case COLOR_RGB_8:
            r = (color >> 16) & 0xFF;
            g = (color >> 8) & 0xFF;
            b = (color) & 0xFF;
            break;
        case COLOR_RGB_10:
        case COLOR_RGB_10_UINT:
            r = (color >> 20) & 0x3FF;
            g = (color >> 10) & 0x3FF;
            b = (color) & 0x3FF;
            break;
        case COLOR_GRAY_8:
            r = g = b = (color & 0xFF);
            break;
        case COLOR_GRAY_10:
            r = g = b = (color & 0x3FF);
            break;
        default:
            Q_ASSERT(false);
            return;
    }
    m_widget->color()[0] = r;
    m_widget->color()[1] = g;
    m_widget->color()[2] = b;

    updateColor();
}

void ColorWidget::setValue(QString hex)
{
    QColor c(hex);

    if(!c.isValid())
        return;

    m_widget->color()[0] = c.red();
    m_widget->color()[1] = c.green();
    m_widget->color()[2] = c.blue();

    updateColor();
}

void ColorWidget::setValueAr(QList<int> val)
{
    if(val.size() < 3)
        return;

    m_widget->color()[0] = val[0];
    m_widget->color()[1] = val[1];
    m_widget->color()[2] = val[2];

    updateColor();
}

void ColorWidget::showValues(bool show)
{
    textAct->setChecked(show);
    m_widget->setDrawNums(show);
    m_widget->update();
}

void ColorWidget::setColorType(int type)
{
    if(type < 0 || type >= COLOR_MAX)
        return;

    for(int i = 0; i < COLOR_MAX; ++i)
        colorType[i]->setChecked(i == type);

    m_color_type = type;

    m_widget->setGrey(type == COLOR_GRAY_8 || type == COLOR_GRAY_10);
    m_widget->update();

    emit updateForMe();
}

void ColorWidget::saveWidgetInfo(DataFileParser *file)
{
    DataWidget::saveWidgetInfo(file);

    char showed = 0;

    // Brightness correction
    file->writeBlockIdentifier("clrWBrightness");
    file->write((char*)&m_brightness, sizeof(m_brightness));
    showed = m_brightness_layout ? 1 : 0;
    file->write(&showed, 1);

    // color correction
    file->writeBlockIdentifier("clrWClrCor");
    file->write((char*)&m_color_cor[0], sizeof(qint16)*3);
    showed = m_color_layout[0] ? 1 : 0;
    file->write(&showed, 1);

    // draw nums
    file->writeBlockIdentifier("clrWShowNums");
    file->writeVal(textAct->isChecked());

    // color type
    file->writeBlockIdentifier("clrWClrType");
    file->writeVal(m_color_type);
}

void ColorWidget::loadWidgetInfo(DataFileParser *file)
{
    DataWidget::loadWidgetInfo(file);

    char showed = 0;
    // brightness correction
    if(file->seekToNextBlock("clrWBrightness", BLOCK_WIDGET))
    {
        file->read((char*)&m_brightness, sizeof(m_brightness));
        file->read(&showed, 1);
        if(showed)
        {
            brightTriggered();
            brightAct->setChecked(true);
        }
    }

    // color correction
    if(file->seekToNextBlock("clrWClrCor", BLOCK_WIDGET))
    {
        file->read((char*)&m_color_cor[0], sizeof(qint16)*3);
        file->read(&showed, 1);
        if(showed)
        {
            colorAct->setChecked(true);
            colorTriggered();
        }
    }

    // draw nums
    if(file->seekToNextBlock("clrWShowNums", BLOCK_WIDGET))
        showValues(file->readVal<bool>());

    // color type
    if(file->seekToNextBlock("clrWClrType", BLOCK_WIDGET))
        setColorType(file->readVal<int>());

    updateColor();
}

void ColorWidget::brightTriggered()
{
    if(!m_brightness_layout)
    {
        m_brightness_layout = new QHBoxLayout();

        m_brightness_layout->addWidget(new QLabel(tr("Brightness: "), this));

        QSlider *m_brightness_slider = new QSlider(Qt::Horizontal, this);
        m_brightness_slider->setMinimum(-255);
        m_brightness_slider->setMaximum(255);
        m_brightness_slider->setTickPosition(QSlider::TicksAbove);
        m_brightness_slider->setTickInterval(255);
        m_brightness_slider->setValue(m_brightness);
        connect(m_brightness_slider, SIGNAL(valueChanged(int)), this, SLOT(brightChanged(int)));

        m_brightness_layout->addWidget(m_brightness_slider);
        layout->addLayout(m_brightness_layout);
    }
    else
    {
        Utils::deleteLayoutMembers(m_brightness_layout);
        delete m_brightness_layout;
        m_brightness_layout = NULL;
    }
}

void ColorWidget::colorTriggered()
{
    for(quint8 i = 0; i < 3; ++i)
    {
        if(!m_color_layout[i])
        {
            m_color_layout[i] = new QHBoxLayout();

            static const QString colorText[]= { "R:", "G:", "B:" };
            m_color_layout[i]->addWidget(new QLabel(colorText[i], this));

            QSlider *m_color_slider = new QSlider(Qt::Horizontal, this);
            m_color_slider->setMinimum(-255);
            m_color_slider->setMaximum(255);
            m_color_slider->setTickPosition(QSlider::TicksAbove);
            m_color_slider->setTickInterval(255);
            m_color_slider->setValue(m_color_cor[i]);

            switch(i)
            {
                case 0:
                    connect(m_color_slider, SIGNAL(valueChanged(int)), this, SLOT(colorChangedR(int)));
                    break;
                case 1:
                    connect(m_color_slider, SIGNAL(valueChanged(int)), this, SLOT(colorChangedG(int)));
                    break;
                case 2:
                    connect(m_color_slider, SIGNAL(valueChanged(int)), this, SLOT(colorChangedB(int)));
                    break;
            }

            m_color_layout[i]->addWidget(m_color_slider);
            layout->addLayout(m_color_layout[i]);
        }
        else
        {
            Utils::deleteLayoutMembers(m_color_layout[i]);
            delete m_color_layout[i];
            m_color_layout[i] = NULL;
        }
    }
}

void ColorWidget::colorChangedR(int value)
{
    m_color_cor[0] = value;
    updateColor();
}

void ColorWidget::colorChangedG(int value)
{
    m_color_cor[1] = value;
    updateColor();
}

void ColorWidget::colorChangedB(int value)
{
    m_color_cor[2] = value;
    updateColor();
}

void ColorWidget::brightChanged(int value)
{
    m_brightness = value;
    updateColor();
}


ColorDisplay::ColorDisplay(QWidget *parent) : QWidget(parent)
{
    m_color[0] = m_color[1] = m_color[2] = 0;
    m_drawNums = false;

    QFont f = Utils::getMonospaceFont(20);
    f.setBold(true);
    setFont(f);

    setAutoFillBackground(true);
}

void ColorDisplay::paintEvent(QPaintEvent *ev)
{
    QWidget::paintEvent(ev);

    if(!(m_drawNums & 0x01))
        return;

    int thrd = (std::min)(width()/3, 100);
    int start = abs(width() - thrd*3)/2;

    QPointF baseline;
    baseline.ry() = height() - 10;

    QPainter p(this);
    p.setBrush(QBrush(Qt::white));

    QPen pen(Qt::black);
    pen.setWidth(1);
    p.setPen(pen);

    QPainterPath path;

    for(int i = 0; i < 3; ++i)
    {
        if((m_drawNums & 0x02) && i != 1)
            continue;

        QString str = QString::number(m_color[i]);
        int w = fontMetrics().width(str);
        baseline.rx() = start + thrd*i + (thrd - w)/2;

        path.addText(baseline, font(), str);
    }
    p.drawPath(path);
}


ColorWidgetAddBtn::ColorWidgetAddBtn(QWidget *parent) : DataWidgetAddBtn(parent)
{
    setText(tr("Color"));
    setIconSize(QSize(17, 17));
    setIcon(QIcon(":/dataWidgetIcons/color.png"));

    m_widgetType = WIDGET_COLOR;
}
