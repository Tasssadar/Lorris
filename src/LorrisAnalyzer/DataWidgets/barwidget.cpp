/**********************************************
**    This file is part of Lorris
**    http://tasssadar.github.com/Lorris/
**
**    See README and COPYING
***********************************************/

#include <QHBoxLayout>
#include <QSignalMapper>
#include <QMenu>
#include <QContextMenuEvent>
#include <qwt_thermo.h>
#include <float.h>

#include "barwidget.h"
#include "../../ui/rangeselectdialog.h"
#include "../../misc/datafileparser.h"

REGISTER_DATAWIDGET(WIDGET_BAR, Bar)

BarWidget::BarWidget(QWidget *parent) : DataWidget(parent)
{
    setTitle("BarWidget");
    setIcon(":/dataWidgetIcons/bar.png");

    m_widgetType = WIDGET_BAR;

    m_showScaleAct = NULL;
    m_showValAct = NULL;

    m_bar = new QwtThermo(this);
    m_bar->setOrientation(Qt::Vertical, QwtThermo::RightScale);
    m_bar->setRangeFlags(QwtInterval::IncludeBorders);
    m_bar->setRange(0, 1000);

    m_label = new QLabel("0", this);
    m_label->setAlignment(Qt::AlignCenter);
    m_label->setFont(Utils::getMonospaceFont(12));

    QHBoxLayout *bar_l = new QHBoxLayout();
    bar_l->addWidget(m_bar);
    layout->addStretch();
    layout->addLayout(bar_l, 1);
    layout->addSpacing(5);
    layout->addWidget(m_label);
    layout->addStretch();

    resize(100, 200);
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

    QSignalMapper *signalMapBits = new QSignalMapper(this);
    for(quint8 i = 0; i < NUM_COUNT; ++i)
    {
        static const QString dataTypes[] =
        {
            tr("unsigned 8bit"),
            tr("unsigned 16bit"),
            tr("unsigned 32bit"),
            tr("unsigned 64bit"),

            tr("signed 8bit"),
            tr("signed 16bit"),
            tr("signed 32bit"),
            tr("signed 64bit"),

            tr("float (4 bytes)"),
            tr("double (8 bytes)")
        };

        if(i%4 == 0 && i != 0)
            bitsMenu->addSeparator();

        m_bitsAct[i] = bitsMenu->addAction(dataTypes[i]);
        m_bitsAct[i]->setCheckable(true);
        signalMapBits->setMapping(m_bitsAct[i], i);
        connect(m_bitsAct[i], SIGNAL(triggered()), signalMapBits, SLOT(map()));
    }
    m_bitsAct[0]->setChecked(true);
    connect(signalMapBits, SIGNAL(mapped(int)), SLOT(setDataType(int)));

    QSignalMapper *signalMapRot = new QSignalMapper(this);
    for(quint8 i = 0; i < 2; ++i)
    {
        static const QString rotText[] = { tr("Vertical"), tr("Horizontal") };
        m_rotAct[i] = rotMenu->addAction(rotText[i]);
        m_rotAct[i]->setCheckable(true);
        signalMapRot->setMapping(m_rotAct[i], i);
        connect(m_rotAct[i], SIGNAL(triggered()), signalMapRot, SLOT(map()));
    }
    rotationSelected(0);
    connect(signalMapRot, SIGNAL(mapped(int)), SLOT(rotationSelected(int)));

    m_rangeAct = contextMenu->addAction(tr("Set range..."));
    m_showScaleAct = contextMenu->addAction(tr("Show scale"));
    m_showValAct = contextMenu->addAction(tr("Show value"));
    m_showScaleAct->setCheckable(true);
    m_showValAct->setCheckable(true);
    m_showScaleAct->setChecked(true);
    m_showValAct->setChecked(true);


    connect(m_rangeAct,     SIGNAL(triggered()),     SLOT(rangeSelected()));
    connect(m_showScaleAct, SIGNAL(triggered(bool)), SLOT(showScale(bool)));
    connect(m_showValAct,   SIGNAL(triggered(bool)), SLOT(showVal(bool)));
}

void BarWidget::processData(analyzer_data *data)
{
    double value;
    try
    {
        value = DataWidget::getNumFromPacket(data, m_info.pos, m_numberType).toDouble();
    }
    catch(char const* e)
    {
        return;
    }
    m_bar->setValue(value);
    m_label->setText(QString::number(value));
}

void BarWidget::setValue(const QVariant &var)
{
    m_bar->setValue(var.toDouble());
    m_label->setText(QString::number(var.toDouble()));
}

void BarWidget::setRange(double min, double max)
{
    m_bar->setRange(min, max);
}

void BarWidget::setDataType(int i)
{
    for(quint8 y = 0; y < NUM_COUNT; ++y)
        m_bitsAct[y]->setChecked(y == i);

    m_numberType = i;
    emit updateForMe();
}

void BarWidget::rangeSelected()
{
    RangeSelectDialog dialog(m_bar->minValue(), m_bar->maxValue(),
                             (m_numberType < NUM_FLOAT), this);
    if(dialog.exec())
        m_bar->setRange(dialog.getMin(), dialog.getMax());

    emit updateForMe();
}

void BarWidget::rotationSelected(int i)
{
    for(quint8 y = 0; y < 2; ++y)
        m_rotAct[y]->setChecked(y == i);

    rotate(i);
}

void BarWidget::rotate(int i)
{
    m_rotation = i;

    if(m_rotation == 0)
         m_bar->setOrientation(Qt::Vertical, (QwtThermo::ScalePos)getScalePos());
    else
         m_bar->setOrientation(Qt::Horizontal, (QwtThermo::ScalePos)getScalePos());
}

void BarWidget::saveWidgetInfo(DataFileParser *file)
{
    DataWidget::saveWidgetInfo(file);

    // data type
    file->writeBlockIdentifier("barWType");
    file->write((char*)&m_numberType, sizeof(m_numberType));

    // range
    file->writeBlockIdentifier("barWRangeDouble");
    file->writeVal(m_bar->minValue());
    file->writeVal(m_bar->maxValue());

    //rotation
    file->writeBlockIdentifier("barWRotation");
    file->write((char*)&m_rotation, sizeof(m_rotation));

    // visibility
    file->writeBlockIdentifier("barWVisibility");
    file->writeVal(m_showScaleAct->isChecked());
    file->writeVal(m_showValAct->isChecked());
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
    if(file->seekToNextBlock("barWRangeDouble", BLOCK_WIDGET))
    {
        double min = file->readVal<double>();
        double max = file->readVal<double>();
        m_bar->setRange(min, max);
    }
    else if(file->seekToNextBlock("barWRange", BLOCK_WIDGET))
    {
        double min = file->readVal<int>();
        double max = file->readVal<int>();
        m_bar->setRange(min, max);
    }

    //rotation
    if(file->seekToNextBlock("barWRotation", BLOCK_WIDGET))
    {
        file->read((char*)&m_rotation, sizeof(m_rotation));
        rotate(m_rotation);
    }

    // visibility
    if(file->seekToNextBlock("barWVisibility", BLOCK_WIDGET))
    {
        showScale(file->readVal<bool>());
        showVal(file->readVal<bool>());
    }
}

void BarWidget::showScale(bool show)
{
    m_showScaleAct->setChecked(show);
    m_bar->setScalePosition((QwtThermo::ScalePos)getScalePos());
}

int BarWidget::getScalePos()
{
    if(m_showScaleAct && !m_showScaleAct->isChecked())
        return QwtThermo::NoScale;

    if(m_rotation == 0)
        return QwtThermo::RightScale;
    else
        return QwtThermo::TopScale;
}

void BarWidget::showVal(bool show)
{
    m_showValAct->setChecked(show);
    m_label->setVisible(show);
}

BarWidgetAddBtn::BarWidgetAddBtn(QWidget *parent) : DataWidgetAddBtn(parent)
{
    setText(tr("Bar"));
    setIconSize(QSize(17, 17));
    setIcon(QIcon(":/dataWidgetIcons/bar.png"));

    m_widgetType = WIDGET_BAR;
}
