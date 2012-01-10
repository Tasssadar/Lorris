#include "colorwidget.h"

ColorWidget::ColorWidget(QWidget *parent) : DataWidget(parent)
{
    setTitle("Color");

    m_widgetType = WIDGET_COLOR;

    m_widget = new QWidget(this);
    m_widget->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Expanding);
    m_widget->setStyleSheet("border: 2px solid white; background-color: black");

    layout->addWidget(m_widget, 1);

    adjustSize();

    setMinimumSize(width(), width());
}

ColorWidget::~ColorWidget()
{
    delete m_widget;
}

void ColorWidget::processData(analyzer_data *data)
{
    try
    {
        QString color = "border: 2px solid white; background-color: #";
        for(quint8 i = 0; i < 3; ++i)
            color += Utils::hexToString(data->getUInt8(m_info.pos + i));
        m_widget->setStyleSheet(color);
    }
    catch(const char*) { }
}

void ColorWidget::setUp()
{
    DataWidget::setUp();
}

void ColorWidget::saveWidgetInfo(AnalyzerDataFile *file)
{
    DataWidget::saveWidgetInfo(file);
}

void ColorWidget::loadWidgetInfo(AnalyzerDataFile *file)
{
    DataWidget::loadWidgetInfo(file);
}


ColorWidgetAddBtn::ColorWidgetAddBtn(QWidget *parent) : DataWidgetAddBtn(parent)
{
    setText(tr("Color"));
    setIconSize(QSize(17, 17));
    setIcon(QIcon(":/dataWidgetIcons/color.png"));

    m_widgetType = WIDGET_COLOR;
}

QPixmap ColorWidgetAddBtn::getRender()
{
    ColorWidget *w = new ColorWidget(this);
    QPixmap map(w->size());
    w->render(&map);
    delete w;
    return map;
}
