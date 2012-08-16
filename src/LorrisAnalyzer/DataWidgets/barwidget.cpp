/**********************************************
**    This file is part of Lorris
**    http://tasssadar.github.com/Lorris/
**
**    See README and COPYING
***********************************************/

#include <QHBoxLayout>
#include <QVBoxLayout>
#include <QSignalMapper>
#include <QMenu>
#include <QContextMenuEvent>
#include <qwt_thermo.h>
#include <float.h>
#include <QInputDialog>
#include <QGridLayout>
#include <QDialogButtonBox>
#include <QColorDialog>

#include "barwidget.h"
#include "../../ui/rangeselectdialog.h"
#include "../../misc/datafileparser.h"

REGISTER_DATAWIDGET(WIDGET_BAR, Bar)

static const QPalette::ColorRole roles[COLOR_COUNT] = { QPalette::Base, QPalette::Highlight, QPalette::ButtonText };

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

    QPalette p(m_bar->palette());
    p.setColor(QPalette::ButtonText, Qt::blue);
    p.setColor(QPalette::Highlight, Qt::red);
    m_bar->setPalette(p);

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


    QMenu *alarmMenu = contextMenu->addMenu(tr("Alarm"));
    m_alarmEnable = alarmMenu->addAction(tr("Enable alarm"));
    m_alarmEnable->setCheckable(true);
    m_alarmLevel = alarmMenu->addAction(tr("Set alarm level..."));
    m_alarmLevel->setEnabled(false);

    m_rangeAct = contextMenu->addAction(tr("Set range..."));
    m_showScaleAct = contextMenu->addAction(tr("Show scale"));
    m_showValAct = contextMenu->addAction(tr("Show value"));
    m_showScaleAct->setCheckable(true);
    m_showValAct->setCheckable(true);
    m_showScaleAct->setChecked(true);
    m_showValAct->setChecked(true);

    QAction *colorAct = contextMenu->addAction(tr("Set colors"));

    connect(m_rangeAct,     SIGNAL(triggered()),     SLOT(rangeSelected()));
    connect(m_showScaleAct, SIGNAL(triggered(bool)), SLOT(showScale(bool)));
    connect(m_showValAct,   SIGNAL(triggered(bool)), SLOT(showVal(bool)));
    connect(m_alarmEnable,  SIGNAL(triggered(bool)), SLOT(setAlarmEnabled(bool)));
    connect(m_alarmLevel,   SIGNAL(triggered()),     SLOT(alarmLevelAct()));
    connect(colorAct,       SIGNAL(triggered()),     SLOT(showColorsDialog()));
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
    setValuePrivate(value);
}

void BarWidget::setValuePrivate(double val)
{
    m_bar->setValue(val);
    m_label->setText(QString::number(val));

    emit scriptEvent(getTitle() + "_valueChanged");

    if(m_bar->alarmEnabled() && val >= m_bar->alarmLevel())
        emit scriptEvent(getTitle() + "_alarm");
}

void BarWidget::setRange(double min, double max)
{
    m_bar->setRange(min, max);

    emit scriptEvent(getTitle() + "_rangeChanged");
}

double BarWidget::getValue() const
{
    return m_bar->value();
}

double BarWidget::getMin() const
{
    return m_bar->minValue();
}

double BarWidget::getMax() const
{
    return m_bar->maxValue();
}

void BarWidget::setAlarmEnabled(bool enable)
{
    m_bar->setAlarmEnabled(enable);
    m_alarmEnable->setChecked(enable);
    m_alarmLevel->setEnabled(enable);
    emit scriptEvent(getTitle() + "_alarmEnabled");
}

void BarWidget::setAlarmLevel(double val)
{
    m_bar->setAlarmLevel(val);

    emit scriptEvent(getTitle() + "_alarmLevelChanged");
}

bool BarWidget::isAlarmEnabled() const
{
    return m_bar->alarmEnabled();
}

double BarWidget::getAlarmLevel() const
{
    return m_bar->alarmLevel();
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

    // colors
    file->writeBlockIdentifier("barWColors");
    {
        QPalette p = m_bar->palette();
        Q_ASSERT(COLOR_COUNT == 3);
        for(int i = 0; i < COLOR_COUNT; ++i)
            file->writeString(p.color(roles[i]).name());
    }

    // alarm
    file->writeBlockIdentifier("barWAlarm");
    file->writeVal(m_bar->alarmEnabled());
    file->writeVal(m_bar->alarmLevel());
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

    // colors
    if(file->seekToNextBlock("barWColors", BLOCK_WIDGET))
    {
        QPalette p = m_bar->palette();
        Q_ASSERT(COLOR_COUNT == 3);
        for(int i = 0; i < COLOR_COUNT; ++i)
            p.setColor(roles[i], QColor(file->readString()));
        m_bar->setPalette(p);
    }

    // alarm
    if(file->seekToNextBlock("barWAlarm", BLOCK_WIDGET))
    {
        m_bar->setAlarmEnabled(file->readVal<bool>());
        m_alarmEnable->setChecked(m_bar->alarmEnabled());
        m_alarmLevel->setEnabled(m_bar->alarmEnabled());

        m_bar->setAlarmLevel(file->readVal<double>());
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

void BarWidget::alarmLevelAct()
{
    bool ok;
    double val = QInputDialog::getDouble(this, tr("Alarm level"), tr("Enter alarm level"), m_bar->alarmLevel(),
                                         INT_MIN, INT_MAX, 4, &ok);
    if(!ok)
        return;

    setAlarmLevel(val);
}

void BarWidget::showColorsDialog()
{
    BarWidgetClrDialog d(m_bar->palette(), this);
    if(d.exec() != QDialog::Accepted)
        return;

    QPalette p = m_bar->palette();
    d.updatePalette(p);
    m_bar->setPalette(p);
}

BarWidgetAddBtn::BarWidgetAddBtn(QWidget *parent) : DataWidgetAddBtn(parent)
{
    setText(tr("Bar"));
    setIconSize(QSize(17, 17));
    setIcon(QIcon(":/dataWidgetIcons/bar.png"));

    m_widgetType = WIDGET_BAR;
}

BarWidgetClrDialog::BarWidgetClrDialog(const QPalette &curPalette, QWidget *parent) : QDialog(parent)
{
    QGridLayout *l = new QGridLayout;
    QHBoxLayout *layoutHor = new QHBoxLayout;

    m_bar = new QwtThermo(this);
    m_bar->setRange(0, 100);
    m_bar->setValue(66);
    m_bar->setAlarmEnabled(true);
    m_bar->setAlarmLevel(33);

    layoutHor->addWidget(m_bar, 1);
    layoutHor->addLayout(l, 3);

    QLabel *label1 = new QLabel(tr("Background:"), this);
    QLabel *label2 = new QLabel(tr("Above alarm:"), this);
    QLabel *label3 = new QLabel(tr("Fill color:"), this);

    l->addWidget(label1, 0, 0);
    l->addWidget(label2, 1, 0);
    l->addWidget(label3, 2, 0);

    QSignalMapper *map = new QSignalMapper(this);
    for(int i = 0; i < COLOR_COUNT; ++i)
    {
        m_colorBtns[i] = new QPushButton(this);
        m_colorBtns[i]->setIconSize(QSize(32, 16));
        m_colorBtns[i]->setSizePolicy(QSizePolicy::Fixed, QSizePolicy::Fixed);
        setColor(i, curPalette.color(roles[i]));
        map->setMapping(m_colorBtns[i], i);
        l->addWidget(m_colorBtns[i], i, 1);

        connect(m_colorBtns[i], SIGNAL(clicked()), map, SLOT(map()));
    }

    QDialogButtonBox *box = new QDialogButtonBox((QDialogButtonBox::Ok | QDialogButtonBox::Cancel), Qt::Horizontal, this);
    QVBoxLayout *mainLayout = new QVBoxLayout(this);
    mainLayout->addLayout(layoutHor);
    mainLayout->addWidget(box);

    connect(map, SIGNAL(mapped(int)), SLOT(btnClicked(int)));
    connect(box, SIGNAL(accepted()),  SLOT(accept()));
    connect(box, SIGNAL(rejected()),  SLOT(reject()));

    resize(250, 200);
}

void BarWidgetClrDialog::btnClicked(int role)
{
    QColor color = QColorDialog::getColor(m_colors[role], this);
    if(!color.isValid())
        return;

    setColor(role, color);
}

void BarWidgetClrDialog::setColor(int role, const QColor &clr)
{
    QPixmap map(50, 25);
    map.fill(clr);
    m_colors[role] = clr;
    m_colorBtns[role]->setIcon(QIcon(map));

    QPalette p = m_bar->palette();
    p.setColor(roles[role], clr);
    m_bar->setPalette(p);
}

void BarWidgetClrDialog::updatePalette(QPalette &p)
{
    for(int i = 0; i < COLOR_COUNT; ++i)
        p.setColor(roles[i], m_colors[i]);
}
