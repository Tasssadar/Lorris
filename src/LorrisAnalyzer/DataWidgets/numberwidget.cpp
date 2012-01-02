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

    QFont font;
    font.setPointSize(20);
    num->setFont(font);
    addWidget(num);
    adjustSize();

    contextMenu = bitsMenu = formatMenu = NULL;
}

NumberWidget::~NumberWidget()
{
    delete bitsMenu;
    delete formatMenu;
    delete contextMenu;
}

void NumberWidget::setUp()
{
    DataWidget::setUp();

    sign = false;
    hex = false;
    level = true;
    bytes = 1;

    contextMenu = new QMenu(this);
    bitsMenu = contextMenu->addMenu(tr("Size"));
    formatMenu = contextMenu->addMenu(tr("Format"));

    QSignalMapper *signalMapBits = new QSignalMapper(this);

    for(quint8 i = 0; i < 8; ++i)
    {
        bitsAction[i] = new QAction((i < 4 ? tr("un") : tr("")) + tr("signed ") +
                                            QString::number((1 << (i%4))*8) + tr(" bits"), this);
        bitsAction[i]->setCheckable(true);
        bitsMenu->addAction(bitsAction[i]);
        signalMapBits->setMapping(bitsAction[i], i);
        connect(bitsAction[i], SIGNAL(triggered()), signalMapBits, SLOT(map()));
    }
    bitsAction[0]->setChecked(true);
    connect(signalMapBits, SIGNAL(mapped(int)), SLOT(bitsSelected(int)));

    QSignalMapper *signalMapFmt = new QSignalMapper(this);
    for(quint8 i = 0; i < 2; ++i)
    {
        fmtAction[i] = new QAction(i == 0 ? tr("Decimal") : tr("Hex"), this);
        fmtAction[i]->setCheckable(true);
        formatMenu->addAction(fmtAction[i]);
        signalMapFmt->setMapping(fmtAction[i], i);
        connect(fmtAction[i], SIGNAL(triggered()), signalMapFmt, SLOT(map()));
    }
    fmtAction[0]->setChecked(true);
    connect(signalMapFmt, SIGNAL(mapped(int)), SLOT(fmtSelected(int)));

    levelAction = new QAction(tr("Level off"), this);
    levelAction->setCheckable(true);
    levelAction->setChecked(true);
    contextMenu->addAction(levelAction);
    connect(levelAction, SIGNAL(triggered()), this, SLOT(levelSelected()));

    setContextMenuPolicy(Qt::DefaultContextMenu);
}

void NumberWidget::processData(analyzer_data *data)
{
    QString n;
    if(sign)
    {
        qint64 i = 0;
        data->getInt(i, sign, bytes, m_info.pos);
        n = QString::number(i, hex ? 16 : 10);
    }
    else
    {
        quint64 i = 0;
        data->getInt(i ,sign, bytes, m_info.pos);
        n = QString::number(i, hex ? 16 : 10).toUpper();
    }
    if(level)
    {
        quint8 pos = 0;
        if(hex)
            pos = bytes*2;
        else
        {
            switch(bytes)
            {
                case 1: pos = 3;  break;
                case 2: pos = 5;  break;
                case 4: pos = 10; break;
                case 8: pos = 19; break;
            }
        }
        pos -= n.length();
        for(quint8 y = 0; y < pos; ++y)
            n = "0" + n;
    }
    num->setText(hex ? "0x" + n : n);
    adjustSize();
}

void NumberWidget::contextMenuEvent ( QContextMenuEvent * event )
{
    if(m_assigned)
        contextMenu->exec(event->globalPos());
}

void NumberWidget::fmtSelected(int i)
{
    for(quint8 y = 0; y < 2; ++y)
        fmtAction[y]->setChecked(y == i);
    hex = (i == 1);
    emit updateData();
}

void NumberWidget::bitsSelected(int i)
{
    for(quint8 y = 0; y < 8; ++y)
        bitsAction[y]->setChecked(y == i);
    sign = i >= 4;
    bytes = 1 << (i%4);
    if(sign && hex)
        fmtSelected(0);
    fmtAction[1]->setEnabled(!sign);
    emit updateData();
}

void NumberWidget::levelSelected()
{
    level = !level;
    levelAction->setChecked(level);
    emit updateData();
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
