/**********************************************
**    This file is part of Lorris
**    http://tasssadar.github.com/Lorris/
**
**    See README and COPYING
***********************************************/

#include <QSpacerItem>
#include <QtUiTools/QUiLoader>

#include "inputwidget.h"

InputWidget::InputWidget(QWidget *parent) :
    DataWidget(parent)
{
    m_widgetType = WIDGET_INPUT;

    setTitle(tr("Input"));
    setIcon(":/dataWidgetIcons/input.png");

    adjustSize();
    setMinimumSize(width(), height());

    layout->addSpacerItem(new QSpacerItem(0, 0, QSizePolicy::Expanding, QSizePolicy::Expanding));
}

InputWidget::~InputWidget()
{
}

QWidget *InputWidget::newWidget(const QString &name, int stretch)
{
    static QUiLoader loader;
    QWidget *w = loader.createWidget(name, this);
    if(!w)
        return NULL;

    layout->addWidget(w, stretch);

    return w;
}
