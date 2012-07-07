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

#include "sliderwidget.h"

#include "ui_sliderwidget.h"

#define DOUBLE_DIV 1000

SliderWidget::SliderWidget(QWidget *parent) : DataWidget(parent), ui(new Ui::SliderWidget)
{
    setTitle(tr("Slider"));
    setIcon(":/dataWidgetIcons/slider.png");

    m_widgetType = WIDGET_SLIDER;

    QWidget *widget = new QWidget(this);
    ui->setupUi(widget);
    layout->addWidget(widget);

    resize(250, 120);
}

SliderWidget::~SliderWidget()
{
    delete ui;
}

void SliderWidget::setUp(Storage *storage)
{
    DataWidget::setUp(storage);

    m_isDouble = false;
    setInteger();

    connect(ui->slider,      SIGNAL(valueChanged(int)),    SLOT(on_slider_valueChanged(int)));
    connect(ui->minEdit,     SIGNAL(textChanged(QString)), SLOT(on_minEdit_textChanged(QString)));
    connect(ui->maxEdit,     SIGNAL(textChanged(QString)), SLOT(on_maxEdit_textChanged(QString)));
    connect(ui->doubleRadio, SIGNAL(toggled(bool)),        SLOT(setType(bool)));
}

void SliderWidget::saveWidgetInfo(DataFileParser *file)
{
    DataWidget::saveWidgetInfo(file);

    file->writeBlockIdentifier("sliderWValues");
    {
        file->writeVal(m_isDouble);

        file->writeString(ui->minEdit->text());
        file->writeString(ui->maxEdit->text());

        file->writeVal(ui->slider->value());
    }
}

void SliderWidget::loadWidgetInfo(DataFileParser *file)
{
    DataWidget::loadWidgetInfo(file);

    if(file->seekToNextBlock("sliderWValues", BLOCK_WIDGET))
    {
        setType(file->readVal<bool>());

        ui->minEdit->setText(file->readString());
        ui->maxEdit->setText(file->readString());

        ui->slider->setValue(file->readVal<int>());
    }
}

double SliderWidget::getValue()
{
    double value = ui->slider->value();

    if(m_isDouble)
        return value/DOUBLE_DIV;

    return value;
}

void SliderWidget::setValue(double val)
{
    if(m_isDouble)
        val *= DOUBLE_DIV;
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
    double value = ui->slider->minimum();

    if(m_isDouble)
        return value/DOUBLE_DIV;

    return value;
}

double SliderWidget::getMax() const
{
    double value = ui->slider->maximum();

    if(m_isDouble)
        return value/DOUBLE_DIV;

    return value;
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
    int val = ui->slider->value();
    if(!isDouble)
    {
        ui->intRadio->setChecked(true);

        ui->minEdit->setText(fixValueToInt(ui->minEdit->text()));
        ui->maxEdit->setText(fixValueToInt(ui->maxEdit->text()));

        ui->minEdit->setValidator(new QIntValidator(INT_MIN, INT_MAX, this));
        ui->maxEdit->setValidator(new QIntValidator(INT_MIN, INT_MAX, this));

        if(m_isDouble)
            val /= DOUBLE_DIV;
    }
    else
    {
        ui->doubleRadio->setChecked(true);

        ui->minEdit->setValidator(new QDoubleValidator(INT_MIN/DOUBLE_DIV, INT_MAX/DOUBLE_DIV, 3, this));
        ui->maxEdit->setValidator(new QDoubleValidator(INT_MIN/DOUBLE_DIV, INT_MAX/DOUBLE_DIV, 3, this));

        if(!m_isDouble)
            val *= DOUBLE_DIV;
    }
    m_isDouble = isDouble;

    on_minEdit_textChanged(ui->minEdit->text());
    on_maxEdit_textChanged(ui->maxEdit->text());
    ui->slider->setValue(val);

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

void SliderWidget::on_slider_valueChanged(int val)
{
    if(!m_isDouble)
        ui->curLabel->setText(QString::number(val));
    else
        ui->curLabel->setText(QString::number(double(val)/DOUBLE_DIV));

    emit scriptEvent(getTitle() + "_valueChanged");
}

void SliderWidget::parseMinMax(bool isMax, const QString& text)
{
    int val = 0;

    if(!m_isDouble)
        val = text.toInt();
    else
        val = text.toDouble()*DOUBLE_DIV;

    if(isMax) ui->slider->setMaximum(val);
    else      ui->slider->setMinimum(val);
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
