/**********************************************
**    This file is part of Lorris
**    http://tasssadar.github.com/Lorris/
**
**    See README and COPYING
***********************************************/

#include <QDoubleSpinBox>
#include <QSlider>
#include <QSpacerItem>
#include <QLineEdit>
#include <QIntValidator>
#include <QDoubleValidator>
#include <math.h>
#include <float.h>

#include "sliderwidget.h"

#include "ui_sliderwidget.h"

SliderWidget::SliderWidget(QWidget *parent) : DataWidget(parent), ui(new Ui::SliderWidget)
{
    setTitle(tr("Slider"));
    setIcon(":/dataWidgetIcons/slider.png");

    m_widgetType = WIDGET_SLIDER;

    QWidget *widget = new QWidget(this);
    ui->setupUi(widget);

    ui->slider->setRange(0, 100);
    ui->slider->setBackgroundStyle(QwtSlider::Groove);

    QSize s = ui->slider->handleSize();
    s.transpose();
    ui->slider->setHandleSize(s);

    layout->addWidget(widget);

    resize(250, 130);
}

SliderWidget::~SliderWidget()
{
    delete ui;
}

void SliderWidget::setUp(Storage *storage)
{
    DataWidget::setUp(storage);

    setInteger();

    connect(ui->slider,      SIGNAL(valueChanged(double)),    SLOT(on_slider_valueChanged(double)));
    connect(ui->minEdit,     SIGNAL(textChanged(QString)), SLOT(on_minEdit_textChanged(QString)));
    connect(ui->maxEdit,     SIGNAL(textChanged(QString)), SLOT(on_maxEdit_textChanged(QString)));
    connect(ui->curEdit,     SIGNAL(textEdited(QString)),  SLOT(on_curEdit_textEdited(QString)));
    connect(ui->doubleRadio, SIGNAL(toggled(bool)),        SLOT(setType(bool)));
}

void SliderWidget::saveWidgetInfo(DataFileParser *file)
{
    DataWidget::saveWidgetInfo(file);

    file->writeBlockIdentifier("sliderWValues2");
    {
        file->writeVal(ui->doubleRadio->isChecked());

        file->writeString(ui->minEdit->text());
        file->writeString(ui->maxEdit->text());

        file->writeVal(ui->slider->value());
    }
}

void SliderWidget::loadWidgetInfo(DataFileParser *file)
{
    DataWidget::loadWidgetInfo(file);

    if(file->seekToNextBlock("sliderWValues2", BLOCK_WIDGET))
    {
        setType(file->readVal<bool>());

        ui->minEdit->setText(file->readString());
        ui->maxEdit->setText(file->readString());

        ui->slider->setValue(file->readVal<double>());
    }
}

double SliderWidget::getValue()
{
    return ui->slider->value();
}

void SliderWidget::setValue(double val)
{
    ui->slider->setValue(val);
}

void SliderWidget::setMin(double min)
{
    ui->minEdit->setText(QString::number(min));
}

void SliderWidget::setMax(double max)
{
    ui->maxEdit->setText(QString::number(max));
}

double SliderWidget::getMin() const
{
    return ui->slider->minValue();
}

double SliderWidget::getMax() const
{
    return ui->slider->maxValue();
}

bool SliderWidget::isInteger() const
{
    return ui->intRadio->isChecked();
}

bool SliderWidget::isDouble() const
{
    return ui->doubleRadio->isChecked();
}

void SliderWidget::setType(bool isDouble)
{
    if(!isDouble)
    {
        ui->intRadio->setChecked(true);

        ui->minEdit->setText(fixValueToInt(ui->minEdit->text()));
        ui->maxEdit->setText(fixValueToInt(ui->maxEdit->text()));

        ui->minEdit->setValidator(new QIntValidator(INT_MIN, INT_MAX, this));
        ui->maxEdit->setValidator(new QIntValidator(INT_MIN, INT_MAX, this));
        ui->curEdit->setValidator(new QIntValidator(INT_MIN, INT_MAX, this));

        ui->slider->setRange(ui->slider->minValue(), ui->slider->maxValue(), 1.0);
        ui->slider->setValue(round(ui->slider->value()));
    }
    else
    {
        ui->doubleRadio->setChecked(true);

        ui->minEdit->setValidator(new QDoubleValidator(-DBL_MIN, DBL_MAX, 0, this));
        ui->maxEdit->setValidator(new QDoubleValidator(-DBL_MIN, DBL_MAX, 0, this));
        ui->curEdit->setValidator(new QDoubleValidator(-DBL_MIN, DBL_MAX, 0, this));

        double step = (ui->slider->maxValue() - ui->slider->minValue())/ui->slider->width();
        ui->slider->setRange(ui->slider->minValue(), ui->slider->maxValue(), step);
    }

    emit scriptEvent(getTitle() + "_typeChanged");
}

void SliderWidget::on_minEdit_textChanged(const QString &text)
{
    parseMinMax(false, text);
    emit scriptEvent(getTitle() + "_minimumChanged");
}

void SliderWidget::on_maxEdit_textChanged(const QString &text)
{
    parseMinMax(true, text);
    emit scriptEvent(getTitle() + "_maximumChanged");
}

void SliderWidget::on_curEdit_textEdited(const QString &text)
{
    ui->slider->setValue(text.toDouble());
}

void SliderWidget::on_slider_valueChanged(double val)
{
    ui->curEdit->setText(QString::number(val));

    emit scriptEvent(getTitle() + "_valueChanged");
}

void SliderWidget::parseMinMax(bool isMax, const QString& text)
{
    double val = text.toDouble();
    double step = ui->intRadio->isChecked() ? 1.0 : (ui->slider->maxValue() - ui->slider->minValue())/ui->slider->width();
    if(isMax) ui->slider->setRange(ui->slider->minValue(), val, step);
    else      ui->slider->setRange(val, ui->slider->maxValue(), step);
}

QString SliderWidget::fixValueToInt(const QString& val)
{
    int idx = val.indexOf(QRegExp("[\\.,]"));
    if(idx != -1)
        return val.left(idx);
    return val;
}

SliderWidgetAddBtn::SliderWidgetAddBtn(QWidget *parent) : DataWidgetAddBtn(parent)
{
    setText(tr("Slider"));
    setIconSize(QSize(17, 17));
    setIcon(QIcon(":/dataWidgetIcons/slider.png"));

    m_widgetType = WIDGET_SLIDER;
}
