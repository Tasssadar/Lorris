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

#include "../../WorkTab/WorkTab.h"
#include "colorwidget.h"

REGISTER_DATAWIDGET(WIDGET_COLOR, Color)

ColorWidget::ColorWidget(QWidget *parent) : DataWidget(parent)
{
    setTitle(tr("Color"));
    setIcon(":/dataWidgetIcons/color.png");

    m_widgetType = WIDGET_COLOR;

    m_widget = new QWidget(this);
    m_widget->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Expanding);
    m_widget->setStyleSheet("background-color: black");

    layout->addWidget(m_widget, 1);

    resize(150, 100);

    setMinimumSize(50, 50);

    m_brightness = 0;
    m_color_layout[0] = m_color_layout[1] = m_color_layout[2] = NULL;
    m_color_cor[0] = m_color_cor[1] = m_color_cor[2] = 0;
    m_color[0] = m_color[1] = m_color[2] = 0;
}

ColorWidget::~ColorWidget()
{
    delete m_widget;
}

void ColorWidget::setUp(Storage *storage)
{
    DataWidget::setUp(storage);

    brightAct = new QAction(tr("Brightness correction"), this);
    brightAct->setCheckable(true);
    connect(brightAct, SIGNAL(triggered()), this, SLOT(brightTriggered()));

    colorAct = new QAction(tr("Color correction"), this);
    colorAct->setCheckable(true);
    connect(colorAct , SIGNAL(triggered()), this, SLOT(colorTriggered()));

    contextMenu->addAction(brightAct);
    contextMenu->addAction(colorAct);

    m_brightness_layout = NULL;
}

void ColorWidget::processData(analyzer_data *data)
{
    try
    {
        for(quint8 i = 0; i < 3; ++i)
            m_color[i] = data->getUInt8(m_info.pos + i);
        updateColor();
    }
    catch(const char*) { }
}

void ColorWidget::updateColor()
{
    static const QString css = "background-color: #";

    QString color_str = "";

    qint16 cor[] = { m_brightness, 0 };
    for(quint8 i = 0; i < 3; ++i)
    {
        quint8 color = m_color[i];
        cor[1] = m_color_cor[i];

        for(quint8 y = 0; y < 2; ++y)
        {
            if     (color + cor[y] > 0xFF) color = 0xFF;
            else if(color + cor[y] < 0)    color = 0x00;
            else                           color += cor[y];
        }

        color_str += Utils::hexToString(color);
    }
    m_widget->setStyleSheet(css + color_str);
}

void ColorWidget::setValue(int r, int g, int b)
{
    m_color[0] = r;
    m_color[1] = g;
    m_color[2] = b;

    updateColor();
}

void ColorWidget::setValue(QString hex)
{
    QColor c(hex);

    if(!c.isValid())
        return;

    m_color[0] = c.red();
    m_color[1] = c.green();
    m_color[2] = c.blue();

    updateColor();
}

void ColorWidget::setValueAr(QList<int> val)
{
    if(val.size() < 3)
        return;

    m_color[0] = val[0];
    m_color[1] = val[1];
    m_color[2] = val[2];

    updateColor();
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

ColorWidgetAddBtn::ColorWidgetAddBtn(QWidget *parent) : DataWidgetAddBtn(parent)
{
    setText(tr("Color"));
    setIconSize(QSize(17, 17));
    setIcon(QIcon(":/dataWidgetIcons/color.png"));

    m_widgetType = WIDGET_COLOR;
}
