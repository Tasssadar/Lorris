#define QT_USE_FAST_CONCATENATION

#include <QLabel>
#include <QAction>
#include <QContextMenuEvent>
#include <QMenu>
#include <QSignalMapper>

#include "numberwidget.h"

NumberWidget::NumberWidget(QWidget *parent) : DataWidget(parent)
{
    setTitle("NumberWidget");
    m_widgetType = WIDGET_NUMBERS;

    num = new QLabel("0", this);
    num->setAlignment(Qt::AlignCenter);
    // FIXME
    //num->setTextInteractionFlags(Qt::TextSelectableByMouse | Qt::TextSelectableByKeyboard);

    QFont font("Monospace");
    font.setStyleHint(QFont::Monospace);
    font.setPixelSize(20);
    num->setFont(font);
    layout->addWidget(num);
    adjustSize();
    setMinimumSize(size());
}

NumberWidget::~NumberWidget()
{

}

void NumberWidget::setUp()
{
    DataWidget::setUp();

    numberType = NUM_UINT8;
    format = FMT_DECIMAL;
    level = false;

    QMenu *bitsMenu = contextMenu->addMenu(tr("Data type"));
    QMenu *formatMenu = contextMenu->addMenu(tr("Format"));

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

    QSignalMapper *signalMapBits = new QSignalMapper(this);

    for(quint8 i = 0; i < NUM_COUNT; ++i)
    {
        if(i%4 == 0 && i != 0)
            bitsMenu->addSeparator();

        bitsAction[i] = new QAction(dataTypes[i], this);
        bitsAction[i]->setCheckable(true);
        bitsMenu->addAction(bitsAction[i]);
        signalMapBits->setMapping(bitsAction[i], i);
        connect(bitsAction[i], SIGNAL(triggered()), signalMapBits, SLOT(map()));
    }
    bitsAction[0]->setChecked(true);
    connect(signalMapBits, SIGNAL(mapped(int)), SLOT(bitsSelected(int)));

    static const QString formatStr[] =
    {
        tr("Decimal"),
        tr("Decimal (w/ exponent)"),
        tr("Hex")
    };

    QSignalMapper *signalMapFmt = new QSignalMapper(this);
    for(quint8 i = 0; i < FMT_COUNT; ++i)
    {
        fmtAction[i] = new QAction(formatStr[i], this);
        fmtAction[i]->setCheckable(true);
        formatMenu->addAction(fmtAction[i]);
        signalMapFmt->setMapping(fmtAction[i], i);
        connect(fmtAction[i], SIGNAL(triggered()), signalMapFmt, SLOT(map()));
    }
    fmtAction[FMT_DECIMAL]->setChecked(true);
    fmtAction[FMT_EXPONENT]->setEnabled(false);
    connect(signalMapFmt, SIGNAL(mapped(int)), SLOT(fmtSelected(int)));

    levelAction = new QAction(tr("Level off"), this);
    levelAction->setCheckable(true);
    contextMenu->addAction(levelAction);
    connect(levelAction, SIGNAL(triggered()), this, SLOT(levelSelected()));
}

void NumberWidget::processData(analyzer_data *data)
{
    QString n;
    try
    {
        static const char fmt[] = { 'f', 'e' };
        static const quint8 base[] = { 10, 10, 16 };

        switch(numberType)
        {
            case NUM_INT8:  n = QString::number(data->getInt8(m_info.pos));  break;
            case NUM_INT16: n = QString::number(data->getInt16(m_info.pos)); break;
            case NUM_INT32: n = QString::number(data->getInt32(m_info.pos)); break;
            case NUM_INT64: n = QString::number(data->getInt64(m_info.pos)); break;

            case NUM_UINT8:  n = QString::number(data->getUInt8(m_info.pos),  base[format]); break;
            case NUM_UINT16: n = QString::number(data->getUInt16(m_info.pos), base[format]); break;
            case NUM_UINT32: n = QString::number(data->getUInt32(m_info.pos), base[format]); break;
            case NUM_UINT64: n = QString::number(data->getUInt64(m_info.pos), base[format]); break;

            case NUM_FLOAT:  n = QString::number(data->getFloat(m_info.pos), fmt[format]);  break;
            case NUM_DOUBLE: n = QString::number(data->getDouble(m_info.pos), fmt[format]); break;
        }
    }
    catch(char const* e)
    {
        num->setText("N/A");
        return;
    }

    if(level)
    {
        static const quint8 levelPos[] =
        {
            3,  //NUM_INT8,  NUM_UINT8,
            5,  //NUM_INT16, NUM_UINT16,
            10, //NUM_INT32, NUM_UINT32,
            19, //NUM_INT64, NUM_UINT64,
            0,  //NUM_FLOAT
            0,  //NUM_DOUBLE
        };

        quint8 pos = format == FMT_HEX ? (numberType%4+1)*2 : levelPos[numberType >= 4 ? numberType - 4 : numberType];
        if(pos)
        {
            bool negative = n.contains("-");
            if(negative)
                n.replace("-", "");

            pos -= n.length();
            for(quint8 y = 0; y < pos; ++y)
                n = "0" % n;

            if(negative)
                n = "-" % n;
        }
    }
    num->setText(format == FMT_HEX ? "0x" % n.toUpper() : n);
}

void NumberWidget::fmtSelected(int i)
{
    for(quint8 y = 0; y < FMT_COUNT; ++y)
        fmtAction[y]->setChecked(y == i);
    format = i;
    emit updateData();
}

void NumberWidget::bitsSelected(int i)
{
    for(quint8 y = 0; y < NUM_COUNT; ++y)
        bitsAction[y]->setChecked(y == i);

    if(i >= NUM_INT8 && numberType < NUM_FLOAT)
        fmtSelected(FMT_DECIMAL);

    numberType = i;

    fmtAction[FMT_HEX]->setEnabled(i < NUM_INT8);
    fmtAction[FMT_EXPONENT]->setEnabled(i >= NUM_FLOAT);
    emit updateData();
}

void NumberWidget::levelSelected()
{
    level = !level;
    levelAction->setChecked(level);
    emit updateData();
}

void NumberWidget::resizeEvent(QResizeEvent *event)
{
    if(event->oldSize().height() < minimumHeight())
        return;
    QFont f = num->font();
    f.setPixelSize(f.pixelSize() + event->size().height() - event->oldSize().height());
    num->setFont(f);
    DataWidget::resizeEvent(event);
}

void NumberWidget::saveWidgetInfo(QFile *file)
{
    DataWidget::saveWidgetInfo(file);

    // data type
    file->write((char*)&numberType, sizeof(numberType));

    // Format
    file->write((char*)&format, sizeof(format));

    // Level off
    file->write((char*)&level, sizeof(level));
}

void NumberWidget::loadWidgetInfo(QFile *file)
{
    DataWidget::loadWidgetInfo(file);

    // data type
    file->read((char*)&numberType, sizeof(numberType));

    // Format
    file->read((char*)&format, sizeof(format));

    // Level off
    file->read((char*)&level, sizeof(level));
}

NumberWidgetAddBtn::NumberWidgetAddBtn(QWidget *parent) : DataWidgetAddBtn(parent)
{
    setText(tr("Number"));
    setIconSize(QSize(17, 17));
    setIcon(QIcon(":/dataWidgetIcons/num.png"));

    m_widgetType = WIDGET_NUMBERS;
}

NumberWidgetAddBtn::~NumberWidgetAddBtn()
{
}

QPixmap NumberWidgetAddBtn::getRender()
{
    NumberWidget *w = new NumberWidget(this);
    QPixmap map(w->size());
    w->render(&map);
    delete w;
    return map;
}
