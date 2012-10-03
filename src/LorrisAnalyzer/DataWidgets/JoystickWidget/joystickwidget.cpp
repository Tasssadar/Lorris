/**********************************************
**    This file is part of Lorris
**    http://tasssadar.github.com/Lorris/
**
**    See README and COPYING
***********************************************/

#include "joystickwidget.h"
#include "../../../joystick/joymgr.h"

#include "ui_joystickwidget.h"

REGISTER_DATAWIDGET(WIDGET_JOYSTICK, Joystick, NULL)

JoystickWidget::JoystickWidget(QWidget *parent) :
    DataWidget(parent), ui(new Ui::JoystickWidget)
{
    m_widgetType = WIDGET_JOYSTICK;

    setTitle(tr("Joystick"));
    setIcon(":/dataWidgetIcons/joystick.png");

    QWidget *main = new QWidget(this);
    ui->setupUi(main);
    layout->addWidget(main);

    refreshJoysticks();
    setEmpty(true);

    connect(ui->refreshBtn, SIGNAL(clicked()), SLOT(refreshJoysticks()));
    connect(ui->structBtn,  SIGNAL(clicked()), SLOT(showStructDialog()));

    resize(300, 150);
}

JoystickWidget::~JoystickWidget()
{
    delete ui;
}

void JoystickWidget::setUp(Storage *storage)
{
    DataWidget::setUp(storage);
}

void JoystickWidget::saveWidgetInfo(DataFileParser *file)
{
    DataWidget::saveWidgetInfo(file);
}

void JoystickWidget::loadWidgetInfo(DataFileParser *file)
{
    DataWidget::loadWidgetInfo(file);
}

void JoystickWidget::processData(analyzer_data *data)
{

}

void JoystickWidget::refreshJoysticks()
{
    sJoyMgr.updateJoystickNames();
    QStringList list = sJoyMgr.getNamesList();

    ui->joyBox->setEnabled(!list.isEmpty());
    ui->joyBox->clear();

    if(!list.isEmpty())
        ui->joyBox->addItems(list);
    else
        ui->joyBox->addItem(tr("No joystick connected"));
}

void JoystickWidget::setEmpty(bool empty)
{
    ui->axisLabel->setVisible(!empty);
    ui->btnLabel->setVisible(!empty);

    ui->structWidget->setVisible(empty);
}

void JoystickWidget::showStructDialog()
{

}

JoystickWidgetAddBtn::JoystickWidgetAddBtn(QWidget *parent) : DataWidgetAddBtn(parent)
{
    setText(tr("Joystick"));
    setIcon(QIcon(":/dataWidgetIcons/joystick.png"));

    m_widgetType = WIDGET_JOYSTICK;
}

JoyStructElement::JoyStructElement(Type t)
{
    m_type = t;
    m_len = 0;
    m_refCnt = 0;
}

JoyStructElement::~JoyStructElement()
{
    Q_ASSERT(m_refCnt == 0);
}

void JoyStructElement::addRef()
{
    ++m_refCnt;
}

void JoyStructElement::rmRef()
{
    if(--m_refCnt == 0)
        delete this;
}

JoyStruct::JoyStruct()
{
    m_len = 0;
}

JoyStruct::JoyStruct(const JoyStruct &str)
{
    m_len = str.m_len;
    m_name = str.m_name;
    m_elements = std::vector<JoyStructElement*>(str.m_elements);

    for(quint32 i = 0; i < m_elements.size(); ++i)
        m_elements[i]->addRef();
}

JoyStruct::~JoyStruct()
{
    for(quint32 i = 0; i < m_elements.size(); ++i)
        m_elements[i]->rmRef();
}
