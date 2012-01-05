#include <QProgressBar>
#include <QHBoxLayout>
#include <QSignalMapper>
#include <QMenu>
#include <QContextMenuEvent>
#include <QSpinBox>
#include <QDialogButtonBox>

#include "barwidget.h"
#include "ui_rangeselectdialog.h"

BarWidget::BarWidget(QWidget *parent) : DataWidget(parent)
{
    setTitle("BarWidget");

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
    setMinimumSize(size());
}

void BarWidget::setUp()
{
    DataWidget::setUp();

    m_min = 0;
    m_max = 1000;
    m_numberType = NUM_UINT8;

    contextMenu = new QMenu(this);
    QMenu *bitsMenu = contextMenu->addMenu(tr("Data type"));

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
    connect(signalMapBits, SIGNAL(mapped(int)), SLOT(bitsSelected(int)));

    rangeAction = new QAction(tr("Set range"), this);
    contextMenu->addAction(rangeAction);
    connect(rangeAction, SIGNAL(triggered()), this, SLOT(rangeSelected()));

    setContextMenuPolicy(Qt::DefaultContextMenu);
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

void BarWidget::contextMenuEvent ( QContextMenuEvent * event )
{
    contextMenu->exec(event->globalPos());
}

void BarWidget::bitsSelected(int i)
{
    for(quint8 y = 0; y < NUM_COUNT; ++y)
        if(bitsAction[y])
            bitsAction[y]->setChecked(y == i);

    m_numberType = i;
    emit updateData();
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

    RangeSelectDialog *dialog = new RangeSelectDialog(m_bar->minimum(), m_bar->maximum(), max, min, this);
    dialog->exec();
    if(dialog->getRes())
    {
        m_bar->setMaximum(dialog->getMax());
        m_bar->setMinimum(dialog->getMin());
    }
    delete dialog;
    emit updateData();
}

BarWidgetAddBtn::BarWidgetAddBtn(QWidget *parent) : DataWidgetAddBtn(parent)
{
    setText(tr("Bar"));
    setIconSize(QSize(17, 17));
    setIcon(QIcon(":/dataWidgetIcons/bar.png"));

    m_widgetType = WIDGET_BAR;
}

QPixmap BarWidgetAddBtn::getRender()
{
    BarWidget *w = new BarWidget(this);
    QPixmap map(w->size());
    w->render(&map);
    delete w;
    return map;
}

RangeSelectDialog::RangeSelectDialog(int val_min, int val_max, int max, int min, QWidget *parent) : QDialog(parent), ui(new Ui::RangeSelectDialog)
{
    ui->setupUi(this);

    m_max = findChild<QSpinBox*>("maxBox");
    m_min = findChild<QSpinBox*>("minBox");
    connect(m_max, SIGNAL(valueChanged(int)), this, SLOT(maxChanged(int)));
    connect(m_min, SIGNAL(valueChanged(int)), this, SLOT(minChanged(int)));

    m_max->setMaximum(max);
    m_max->setMinimum(min);
    m_min->setMaximum(max);
    m_min->setMinimum(min);
    m_max->setValue(val_max);
    m_min->setValue(val_min);

    QDialogButtonBox *box = findChild<QDialogButtonBox*>("buttonBox");
    connect(box, SIGNAL(clicked(QAbstractButton*)), this, SLOT(boxClicked(QAbstractButton*)));

    m_res = false;

    setFixedSize(size());
}

RangeSelectDialog::~RangeSelectDialog()
{
    delete ui;
}

void RangeSelectDialog::maxChanged(int value)
{
    m_maxRes = value;
    m_min->setMaximum(value);
}

void RangeSelectDialog::minChanged(int value)
{
    m_minRes = value;
    m_max->setMinimum(value);
}

void RangeSelectDialog::boxClicked(QAbstractButton *b)
{
    QDialogButtonBox *buttonBox = findChild<QDialogButtonBox*>("buttonBox");
    if(buttonBox->buttonRole(b) == QDialogButtonBox::AcceptRole)
        m_res = true;
    close();
}
