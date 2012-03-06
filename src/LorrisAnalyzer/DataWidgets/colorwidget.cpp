/****************************************************************************
**
**    This file is part of Lorris.
**    Copyright (C) 2012 Vojtěch Boček
**
**    Contact: <vbocek@gmail.com>
**             https://github.com/Tasssadar
**
**    Lorris is free software: you can redistribute it and/or modify
**    it under the terms of the GNU General Public License as published by
**    the Free Software Foundation, either version 3 of the License, or
**    (at your option) any later version.
**
**    Lorris is distributed in the hope that it will be useful,
**    but WITHOUT ANY WARRANTY; without even the implied warranty of
**    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
**    GNU General Public License for more details.
**
**    You should have received a copy of the GNU General Public License
**    along with Lorris.  If not, see <http://www.gnu.org/licenses/>.
**
****************************************************************************/

#include <QSlider>
#include <QAction>
#include <QMenu>
#include <QSignalMapper>

#include "WorkTab/WorkTab.h"
#include "colorwidget.h"

ColorWidget::ColorWidget(QWidget *parent) : DataWidget(parent)
{
    setTitle(tr("Color"));
    setIcon(":/dataWidgetIcons/color.png");

    m_widgetType = WIDGET_COLOR;

    m_widget = new QWidget(this);
    m_widget->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Expanding);
    m_widget->setStyleSheet("background-color: black");

    layout->setContentsMargins(3, 0, 3, 3);
    layout->addWidget(m_widget, 1);

    adjustSize();

    setMinimumSize(width(), width());

    m_brightness = 0;
    m_color_layout[0] = m_color_layout[1] = m_color_layout[2] = NULL;
    m_color_cor[0] = m_color_cor[1] = m_color_cor[2] = 0;
    m_color[0] = m_color[1] = m_color[2] = 0;
}

ColorWidget::~ColorWidget()
{
    delete m_widget;
}

void ColorWidget::setUp(AnalyzerDataStorage *storage)
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
    hex.remove("#");
    m_color[0] = hex.mid(0, 2).toInt(NULL, 16);
    m_color[1] = hex.mid(2, 2).toInt(NULL, 16);
    m_color[2] = hex.mid(4, 2).toInt(NULL, 16);

    updateColor();
}

void ColorWidget::saveWidgetInfo(AnalyzerDataFile *file)
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

void ColorWidget::loadWidgetInfo(AnalyzerDataFile *file)
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
        WorkTab::DeleteAllMembers(m_brightness_layout);
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
            WorkTab::DeleteAllMembers(m_color_layout[i]);
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
