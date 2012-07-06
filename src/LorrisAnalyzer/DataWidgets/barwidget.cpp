/**********************************************
**    This file is part of Lorris
**    http://tasssadar.github.com/Lorris/
**
**    See README and COPYING
***********************************************/

#include <QProgressBar>
#include <QHBoxLayout>
#include <QSignalMapper>
#include <QMenu>
#include <QContextMenuEvent>
#include <QSpinBox>
#include <QDialogButtonBox>

#include "barwidget.h"
#include "../../ui/rangeselectdialog.h"
#include "../../misc/datafileparser.h"

BarWidget::BarWidget(QWidget *parent) : DataWidget(parent)
{
    setTitle("BarWidget");
    setIcon(":/dataWidgetIcons/bar.png");

    m_widgetType = WIDGET_BAR;
    m_bar = new QProgressBar(this);
    m_bar->setMaximum(1000);
    m_bar->setTextVisible(false);
    m_bar->setOrientation(Qt::Vertical);
    m_bar->adjustSize();

    QHBoxLayout *bar_l = new QHBoxLayout();
    bar_l->addWidget(m_bar);
    layout->addLayout(bar_l);
    adjustSize();
}

void BarWidget::setUp(Storage *storage)
{
    DataWidget::setUp(storage);

    m_min = 0;
    m_max = 1000;
    m_numberType = NUM_UINT8;
    m_rotation = 0;

    QMenu *bitsMenu = contextMenu->addMenu(tr("Data type"));
    QMenu *rotMenu = contextMenu->addMenu(tr("Rotation"));

    static const QString dataTypes[] =
    {
        tr("unsigned 8bit"),
        tr("unsigned 16bit"),

        tr("signed 8bit"),
        tr("signed 16bit"),
        tr("signed 32bit"),
    };

    QSignalMapper *signalMapBits = new QSignalMapper(this);

    quint8 y,i;
    for(y = i = 0; i < NUM_COUNT; ++i)
    {
        if(i == 2)
            bitsMenu->addSeparator();

        if(i == NUM_UINT32 || i == NUM_UINT64 || i > NUM_INT32)
        {
            bitsAction[i] = NULL;
            continue;
        }

        bitsAction[i] = new QAction(dataTypes[y], this);
        bitsAction[i]->setCheckable(true);
        bitsMenu->addAction(bitsAction[i]);
        signalMapBits->setMapping(bitsAction[i], i);
        connect(bitsAction[i], SIGNAL(triggered()), signalMapBits, SLOT(map()));
        ++y;
    }
    bitsAction[0]->setChecked(true);
    connect(signalMapBits, SIGNAL(mapped(int)), SLOT(setDataType(int)));

    QSignalMapper *signalMapRot = new QSignalMapper(this);

    static const QString rotText[]=
    {
        tr("Vertical"),
        tr("Horizontal")
    };
    for(i = 0; i < 2; ++i)
    {
        rotAction[i] = new QAction(rotText[i], this);
        rotAction[i]->setCheckable(true);
        rotMenu->addAction(rotAction[i]);
        signalMapRot->setMapping(rotAction[i], i);
        connect(rotAction[i], SIGNAL(triggered()), signalMapRot, SLOT(map()));
    }
    rotationSelected(0);
    connect(signalMapRot, SIGNAL(mapped(int)), SLOT(rotationSelected(int)));

    rangeAction = new QAction(tr("Set range"), this);
    contextMenu->addAction(rangeAction);
    connect(rangeAction, SIGNAL(triggered()), this, SLOT(rangeSelected()));
}

void BarWidget::processData(analyzer_data *data)
{
    int value = 0;
    try
    {
        switch(m_numberType)
        {
            case NUM_UINT8:  value = data->getUInt8(m_info.pos);  break;
            case NUM_UINT16: value = data->getUInt16(m_info.pos); break;
            case NUM_INT8:   value = data->getInt8(m_info.pos);   break;
            case NUM_INT16:  value = data->getInt16(m_info.pos);  break;
            case NUM_INT32:  value = data->getInt32(m_info.pos);  break;
        }
    }
    catch(char const* e)
    {
        return;
    }
    m_bar->setValue(value);
}

void BarWidget::setValue(const QVariant &var)
{
    m_bar->setValue(var.toInt());
}

void BarWidget::setRange(int min, int max)
{
    m_bar->setRange(min, max);
}

void BarWidget::setDataType(int i)
{
    for(quint8 y = 0; y < NUM_COUNT; ++y)
        if(bitsAction[y])
            bitsAction[y]->setChecked(y == i);

    m_numberType = i;
    emit updateForMe();
}

void BarWidget::rangeSelected()
{
    int min = 0;
    int max = 0;
    switch(m_numberType)
    {
        case NUM_UINT8:  max = 0xFF; break;
        case NUM_UINT16: max = 0xFFFF; break;
        case NUM_INT8:
            min = -128;
            max = 127;
            break;
        case NUM_INT16:
            min = -32768;
            max = 32767;
            break;
        case NUM_INT32:
            min = -2147483647;
            max = 2147483646;
            break;
    }

    RangeSelectDialog dialog(m_bar->minimum(), m_bar->maximum(), max, min, this);
    dialog.exec();
    if(dialog.getRes())
        m_bar->setRange(dialog.getMin(), dialog.getMax());

    emit updateForMe();
}

void BarWidget::rotationSelected(int i)
{
    for(quint8 y = 0; y < 2; ++y)
        rotAction[y]->setChecked(y == i);

    rotate(i);

    resize(0, 0);
    adjustSize();
}

void BarWidget::rotate(int i)
{
    m_rotation = i;

    bool horizontal = false;
    //bool switchSides = false;
    switch(i)
    {
        case 0: break;
        case 1:
            horizontal = true;
            break;
        case 2:
            //switchSides = true;
            break;
        case 3:
            //horizontal = switchSides = true;
            break;

    }
    //FIXME: not working?
    //m_bar->setInvertedAppearance(switchSides);
    m_bar->setOrientation(horizontal ? Qt::Horizontal : Qt::Vertical);
}

void BarWidget::saveWidgetInfo(DataFileParser *file)
{
    DataWidget::saveWidgetInfo(file);

    // data type
    file->writeBlockIdentifier("barWType");
    file->write((char*)&m_numberType, sizeof(m_numberType));

    // range
    file->writeBlockIdentifier("barWRange");
    qint32 min = m_bar->minimum();
    qint32 max = m_bar->maximum();
    file->write((char*)&min, sizeof(qint32));
    file->write((char*)&max, sizeof(qint32));

    //rotation
    file->writeBlockIdentifier("barWRotation");
    file->write((char*)&m_rotation, sizeof(m_rotation));
}

void BarWidget::loadWidgetInfo(DataFileParser *file)
{
    DataWidget::loadWidgetInfo(file);

    // data type
    if(file->seekToNextBlock("barWType", BLOCK_WIDGET))
    {
        file->read((char*)&m_numberType, sizeof(m_numberType));
        setDataType(m_numberType);
    }

    // range
    if(file->seekToNextBlock("barWRange", BLOCK_WIDGET))
    {
        qint32 min = 0;
        qint32 max = 0;
        file->read((char*)&min, sizeof(qint32));
        file->read((char*)&max, sizeof(qint32));
        m_bar->setRange(min, max);
    }

    //rotation
    if(file->seekToNextBlock("barWRotation", BLOCK_WIDGET))
    {
        file->read((char*)&m_rotation, sizeof(m_rotation));
        rotate(m_rotation);
    }
}

BarWidgetAddBtn::BarWidgetAddBtn(QWidget *parent) : DataWidgetAddBtn(parent)
{
    setText(tr("Bar"));
    setIconSize(QSize(17, 17));
    setIcon(QIcon(":/dataWidgetIcons/bar.png"));

    m_widgetType = WIDGET_BAR;
}
