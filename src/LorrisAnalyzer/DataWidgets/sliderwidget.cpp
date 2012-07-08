/**********************************************
**    This file is part of Lorris
**    http://tasssadar.github.com/Lorris/
**
**    See README and COPYING
***********************************************/

#include <QIntValidator>
#include <QDoubleValidator>
#include <math.h>
#include <float.h>
#include <QSignalMapper>

#include "sliderwidget.h"

#include "ui_sliderwidget_horizontal.h"
#include "ui_sliderwidget_vertical.h"

SliderWidget::SliderWidget(QWidget *parent) : DataWidget(parent),
    ui_hor(new Ui::SliderWidget_horizontal), ui_ver(new Ui::SliderWidget_vertical)
{
    setTitle(tr("Slider"));
    setIcon(":/dataWidgetIcons/slider.png");

    m_widgetType = WIDGET_SLIDER;
    m_widget = NULL;

    m_min = 0;
    m_max = 100;

    m_ori_act[0] = NULL;
    m_hide_act = NULL;

    m_orientation = ORI_MAX+1;

    setOrientation(ORI_HOR_L_R);
    resize(250, 130);
}

SliderWidget::~SliderWidget()
{
    delete ui_ver;
    delete ui_hor;
}

void SliderWidget::setUp(Storage *storage)
{
    DataWidget::setUp(storage);

    QMenu *typeMenu = contextMenu->addMenu(tr("Data type"));

    m_int_act = typeMenu->addAction(tr("Integer"));
    m_double_act = typeMenu->addAction(tr("Double"));
    m_int_act->setCheckable(true);
    m_double_act->setCheckable(true);

    QMenu *oriMenu = contextMenu->addMenu(tr("Orientation"));
    QSignalMapper *map = new QSignalMapper(this);
    for(int i = 0; i < ORI_MAX; ++i)
    {
        static const QString names[ORI_MAX] = {
            tr("Horizontal, left to right"),
            tr("Vertical, bottom to top"),
            tr("Horizontal, right to left"),
            tr("Vertical, top to bottom")
        };

        m_ori_act[i] = oriMenu->addAction(names[i]);
        m_ori_act[i]->setCheckable(true);
        map->setMapping(m_ori_act[i], i);
        connect(m_ori_act[i], SIGNAL(triggered()), map, SLOT(map()));
    }
    m_ori_act[0]->setChecked(true);

    m_hide_act = contextMenu->addAction(tr("Hide min and max setting"));
    m_hide_act->setCheckable(true);

    setInteger();

    connect(m_int_act,       SIGNAL(triggered(bool)), SLOT(intAct(bool)));
    connect(m_double_act,    SIGNAL(triggered(bool)), SLOT(doubleAct(bool)));
    connect(m_hide_act,      SIGNAL(triggered(bool)), SLOT(hideMinMax(bool)));
    connect(map,             SIGNAL(mapped(int)),     SLOT(setOrientation(int)));
}

void SliderWidget::saveWidgetInfo(DataFileParser *file)
{
    DataWidget::saveWidgetInfo(file);

    file->writeBlockIdentifier("sliderWValues2");
    {
        file->writeVal(m_double_act->isChecked());

        file->writeString(minEdit()->text());
        file->writeString(maxEdit()->text());

        file->writeVal(slider()->value());
    }

    file->writeBlockIdentifier("sliderWValues3");
    {
        file->writeVal(m_hide_act->isChecked());
        file->writeVal(m_orientation);
    }
}

void SliderWidget::loadWidgetInfo(DataFileParser *file)
{
    DataWidget::loadWidgetInfo(file);

    if(file->seekToNextBlock("sliderWValues2", BLOCK_WIDGET))
    {
        setType(file->readVal<bool>());

        minEdit()->setText(file->readString());
        maxEdit()->setText(file->readString());

        slider()->setValue(file->readVal<double>());
    }

    if(file->seekToNextBlock("sliderWValues3", BLOCK_WIDGET))
    {
        hideMinMax(file->readVal<bool>());
        setOrientation(file->readVal<quint8>());
    }
}

double SliderWidget::getValue()
{
    return slider()->value();
}

void SliderWidget::setValue(double val)
{
    slider()->setValue(val);
}

void SliderWidget::setMin(double min)
{
    minEdit()->setText(QString::number(min));
}

void SliderWidget::setMax(double max)
{
    maxEdit()->setText(QString::number(max));
}

double SliderWidget::getMin() const
{
    return slider()->minValue();
}

double SliderWidget::getMax() const
{
    return slider()->maxValue();
}

bool SliderWidget::isInteger() const
{
    return m_int_act->isChecked();
}

bool SliderWidget::isDouble() const
{
    return m_double_act->isChecked();
}

void SliderWidget::setType(bool isDouble)
{
    m_int_act->setChecked(!isDouble);
    m_double_act->setChecked(isDouble);

    if(!isDouble)
    {
        minEdit()->setText(fixValueToInt(minEdit()->text()));
        maxEdit()->setText(fixValueToInt(maxEdit()->text()));

        minEdit()->setValidator(new QIntValidator(INT_MIN, INT_MAX, this));
        maxEdit()->setValidator(new QIntValidator(INT_MIN, INT_MAX, this));
        curEdit()->setValidator(new QIntValidator(INT_MIN, INT_MAX, this));

        setRange(m_min, m_max, 1.0);
        slider()->setValue(round(slider()->value()));
    }
    else
    {
        minEdit()->setValidator(new QDoubleValidator(-DBL_MIN, DBL_MAX, 0, this));
        maxEdit()->setValidator(new QDoubleValidator(-DBL_MIN, DBL_MAX, 0, this));
        curEdit()->setValidator(new QDoubleValidator(-DBL_MIN, DBL_MAX, 0, this));

        double step = (m_max - m_min)/slider()->width();
        setRange(m_min, m_max, step);
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
    slider()->setValue(text.toDouble());
}

void SliderWidget::on_slider_valueChanged(double val)
{
    curEdit()->setText(QString::number(val));

    emit scriptEvent(getTitle() + "_valueChanged");
}

void SliderWidget::parseMinMax(bool isMax, const QString& text)
{
    double val = text.toDouble();
    double step = m_int_act->isChecked() ? 1.0 : (slider()->maxValue() - slider()->minValue())/slider()->width();
    if(isMax) setRange(m_min, val, step);
    else      setRange(val, m_max, step);
}

QString SliderWidget::fixValueToInt(const QString& val)
{
    int idx = val.indexOf(QRegExp("[\\.,]"));
    if(idx != -1)
        return val.left(idx);
    return val;
}

void SliderWidget::intAct(bool checked)
{
    if(checked)
        setInteger();
}

void SliderWidget::doubleAct(bool checked)
{
    if(checked)
        setDouble();
}

QwtSlider *SliderWidget::slider() const
{
    if(m_orientation%2)
        return ui_ver->slider;
    else
        return ui_hor->slider;
}

QLineEdit *SliderWidget::minEdit() const
{
    if(m_orientation%2)
        return ui_ver->minEdit;
    else
        return ui_hor->minEdit;
}

QLineEdit *SliderWidget::maxEdit() const
{
    if(m_orientation%2)
        return ui_ver->maxEdit;
    else
        return ui_hor->maxEdit;
}

QLineEdit *SliderWidget::curEdit() const
{
    if(m_orientation%2)
        return ui_ver->curEdit;
    else
        return ui_hor->curEdit;
}

void SliderWidget::setOrientation(int ori)
{
    if(ori == m_orientation)
        return;

    for(int i = 0; m_ori_act[0] && i < ORI_MAX; ++i)
        m_ori_act[i]->setChecked(i == ori);

    double value = 0;
    if(m_widget)
        value = slider()->value();

    delete m_widget;
    m_widget = new QWidget(this);

    m_orientation = ori;
    switch(ori)
    {
        case ORI_HOR_L_R:
        case ORI_HOR_R_L:
        {
            ui_hor->setupUi(m_widget);

            QSize s = ui_hor->slider->handleSize();
            s.transpose();
            ui_hor->slider->setHandleSize(s);
            break;
        }
        case ORI_VER_B_T:
        case ORI_VER_T_B:
            ui_ver->setupUi(m_widget);
            ui_ver->slider->setOrientation(Qt::Vertical);
            break;
    }

    slider()->setBackgroundStyle(QwtSlider::Groove);
    setRange(m_min, m_max);
    slider()->setValue(value);
    minEdit()->setText(QString::number(m_min));
    maxEdit()->setText(QString::number(m_max));
    curEdit()->setText(QString::number(value));

    if(m_hide_act)
        hideMinMax(m_hide_act->isChecked());

    layout->addWidget(m_widget);

    connect(slider(),      SIGNAL(valueChanged(double)), SLOT(on_slider_valueChanged(double)));
    connect(minEdit(),     SIGNAL(textChanged(QString)), SLOT(on_minEdit_textChanged(QString)));
    connect(maxEdit(),     SIGNAL(textChanged(QString)), SLOT(on_maxEdit_textChanged(QString)));
    connect(curEdit(),     SIGNAL(textEdited(QString)),  SLOT(on_curEdit_textEdited(QString)));

    emit scriptEvent(getTitle() + "_orientationChanged");
}

void SliderWidget::setRange(double min, double max, double step)
{
    m_min = min;
    m_max = max;

    if(m_orientation == ORI_VER_B_T || m_orientation == ORI_HOR_R_L)
        std::swap(min, max);

    slider()->setRange(min, max, step);
}

void SliderWidget::hideMinMax(bool hide)
{
    m_hide_act->setChecked(hide);

    minEdit()->setVisible(!hide);
    maxEdit()->setVisible(!hide);

    if(m_orientation%2)
    {
        ui_ver->minLabel->setVisible(!hide);
        ui_ver->maxLabel->setVisible(!hide);
    }

    emit scriptEvent(getTitle() + "_visibilityChanged");
}

bool SliderWidget::isMinMaxVisible() const
{
    return !m_hide_act->isChecked();
}

SliderWidgetAddBtn::SliderWidgetAddBtn(QWidget *parent) : DataWidgetAddBtn(parent)
{
    setText(tr("Slider"));
    setIconSize(QSize(17, 17));
    setIcon(QIcon(":/dataWidgetIcons/slider.png"));

    m_widgetType = WIDGET_SLIDER;
}
