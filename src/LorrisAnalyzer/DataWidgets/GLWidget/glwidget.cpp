/**********************************************
**    This file is part of Lorris
**    http://tasssadar.github.com/Lorris/
**
**    See README and COPYING
***********************************************/

#include "glwidget.h"
#include "renderwidget.h"

REGISTER_DATAWIDGET(WIDGET_OPENGL, GL, NULL)

GLWidget::GLWidget(QWidget *parent) :
    DataWidget(parent)
{
    m_widgetType = WIDGET_OPENGL;

    setTitle(tr("OpenGL"));
    setIcon(":/dataWidgetIcons/opengl.png");

    m_widget = new RenderWidget(this);
    layout->addWidget(m_widget, 1);

    resize(300, 300);
}

GLWidgetAddBtn::GLWidgetAddBtn(QWidget *parent) : DataWidgetAddBtn(parent)
{
    setText(tr("OpenGL"));
    setIconSize(QSize(17, 17));
    setIcon(QIcon(":/dataWidgetIcons/opengl.png"));

    m_widgetType = WIDGET_OPENGL;
}
