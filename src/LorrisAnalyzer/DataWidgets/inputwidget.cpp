/**********************************************
**    This file is part of Lorris
**    http://tasssadar.github.com/Lorris/
**
**    See README and COPYING
***********************************************/

#include <QSpacerItem>
#include <QtUiTools/QUiLoader>
#include "../../ui/colorbutton.h"

#include "inputwidget.h"

REGISTER_DATAWIDGET(WIDGET_INPUT, Input, NULL)

InputWidget::InputWidget(QWidget *parent) :
    DataWidget(parent)
{
    m_widgetType = WIDGET_INPUT;

    setTitle(tr("Input"));
    setIcon(":/dataWidgetIcons/input.png");

    m_layout = new QVBoxLayout;
    m_layout->addSpacerItem(new QSpacerItem(0, 0, QSizePolicy::Expanding, QSizePolicy::Expanding));
    m_layout->setSpacing(0);

    layout->addLayout(m_layout);
}

InputWidget::~InputWidget()
{
}

QWidget *InputWidget::newWidget(const QString &name, int stretch)
{
    static QUiLoader loader;
    QWidget *w = loader.createWidget(name, this);
    if(!w)
    {
        int id = QMetaType::type(name.toStdString().c_str());
        if(id)
            w = (QWidget*)QMetaType::construct(id);
    }

    if(!w)
        return NULL;

    m_layout->insertWidget(m_layout->count()-1, w, stretch);
    return w;
}

void InputWidget::setHorizontal(bool horizontal)
{
    if(horizontal ^ m_layout->inherits("QVBoxLayout"))
        return;

    QBoxLayout *newLayout;
    if(horizontal) newLayout = new QHBoxLayout;
    else           newLayout = new QVBoxLayout;

    layout->addLayout(newLayout);

    while(m_layout->count() != 0)
        newLayout->addItem(m_layout->takeAt(0));
    delete m_layout;
    m_layout = newLayout;
}

void InputWidget::clear()
{
    while(m_layout->count() != 0)
    {
        QLayoutItem *i = m_layout->takeAt(0);
        if(i->widget())
            delete i->widget();
        delete i;
    }
    m_layout->addSpacerItem(new QSpacerItem(0, 0, QSizePolicy::Expanding, QSizePolicy::Expanding));
}

void InputWidget::removeWidget(QWidget *widget)
{
    if(!widget)
        return;

    m_layout->removeWidget(widget);
    delete widget;
}

InputWidgetAddBtn::InputWidgetAddBtn(QWidget *parent) : DataWidgetAddBtn(parent)
{
    setText(tr("Input"));
    setIconSize(QSize(17, 17));
    setIcon(QIcon(":/dataWidgetIcons/input.png"));

    m_widgetType = WIDGET_INPUT;
}

