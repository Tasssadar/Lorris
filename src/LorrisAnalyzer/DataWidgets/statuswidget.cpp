/**********************************************
**    This file is part of Lorris
**    http://tasssadar.github.com/Lorris/
**
**    See README and COPYING
***********************************************/

#include <QSignalMapper>

#include "statuswidget.h"
#include "../../ui/colorbutton.h"

REGISTER_DATAWIDGET(WIDGET_STATUS, Status, NULL)

StatusWidget::StatusWidget(QWidget *parent) :
    DataWidget(parent)
{
    m_widgetType = WIDGET_STATUS;

    setTitle(tr("Status"));
    setIcon(":/dataWidgetIcons/status.png");

    m_status = 0;
    m_dataType = 0;

    m_label = new QLabel(this);
    m_label->setAlignment(Qt::AlignCenter);
    m_label->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Expanding);
    m_label->setAutoFillBackground(true);

    layout->addWidget(m_label);

    resize(200, 60);

    addStatus(0, "FALSE", "red", "black");
    addStatus(1, "TRUE", "#00FF00", "black");

    setValue(1);
}

void StatusWidget::setUp(Storage *storage)
{
    DataWidget::setUp(storage);

    QMenu *typeMenu = contextMenu->addMenu(tr("Data type"));

    QSignalMapper *signalMapType = new QSignalMapper(this);
    for(quint8 i = 0; i < NUM_FLOAT; ++i)
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
        };

        if(i%4 == 0 && i != 0)
            typeMenu->addSeparator();

        m_typeAction[i] = typeMenu->addAction(dataTypes[i]);
        m_typeAction[i]->setCheckable(true);
        signalMapType->setMapping(m_typeAction[i], i);
        connect(m_typeAction[i], SIGNAL(triggered()), signalMapType, SLOT(map()));
    }
    m_typeAction[0]->setChecked(true);
    connect(signalMapType, SIGNAL(mapped(int)), SLOT(setDataType(int)));

    contextMenu->addAction(tr("Manage states..."), this, SLOT(showStatusManager()));
}

void StatusWidget::saveWidgetInfo(DataFileParser *file)
{
    DataWidget::saveWidgetInfo(file);

    // data type
    file->writeBlockIdentifier("statusWDataType");
    file->writeVal(m_dataType);

    // states
    file->writeBlockIdentifier("statusWStates");
    file->writeVal((quint32)m_states.size());
    for(QHash<quint64, status>::iterator itr = m_states.begin(); itr != m_states.end(); ++itr)
    {
        file->writeVal(itr.key());
        file->writeString((*itr).text);
        file->writeColor((*itr).color);
        file->writeColor((*itr).textColor);
    }
    file->writeVal(m_status);
}

void StatusWidget::loadWidgetInfo(DataFileParser *file)
{
    DataWidget::loadWidgetInfo(file);

    // data type
    if(file->seekToNextBlock("statusWDataType", BLOCK_WIDGET))
        setDataType(file->readVal<int>());

    // states
    if(file->seekToNextBlock("statusWStates", BLOCK_WIDGET))
    {
        quint32 count = file->readVal<quint32>();
        for(quint32 i = 0; i < count; ++i)
        {
            quint64 val = file->readVal<quint64>();
            status s;
            s.text = file->readString();
            s.color = file->readColor();
            s.textColor = file->readColor();

            m_states[val] = s;
        }
        setValue(file->readVal<quint64>());
    }
}

void StatusWidget::processData(analyzer_data *data)
{
    quint64 value;
    try
    {
        value = DataWidget::getNumFromPacket(data, m_info.pos, m_dataType).toULongLong();
    }
    catch(char const* e)
    {
        return;
    }
    setValue(value);
}

void StatusWidget::addStatus(quint64 id, const QString &text, const QString &color, const QString &textColor)
{
    status s = { text, QColor(color), QColor(textColor) };
    addStatus(id, s);
}

void StatusWidget::addStatus(quint64 id, const status &s)
{
    if(m_states.isEmpty())
    {
        m_states.insert(id, s);
        setValue(id);
    }
    else
        m_states[id] = s;
}

void StatusWidget::removeStatus(quint64 id)
{
    m_states.remove(id);

    if(m_states.isEmpty())
    {
        m_status = 0;
        m_label->setText("NONE");
        m_label->setPalette(QPalette());
    }
    else if(m_status == id)
    {
        setValue(m_states.begin().key());
    }
}

void StatusWidget::setValue(quint64 id)
{
    if(id == m_status)
        return;

    QHash<quint64, status>::iterator itr = m_states.find(id);
    if(itr == m_states.end())
        return;

    m_status = id;

    m_label->setText((*itr).text);

    QPalette p = m_label->palette();
    p.setColor(QPalette::Window, (*itr).color);
    p.setColor(QPalette::WindowText, (*itr).textColor);
    m_label->setPalette(p);
}

void StatusWidget::updateValue(quint64 id)
{
    if(m_status != id)
        return;

    QHash<quint64, status>::iterator itr = m_states.find(id);
    if(itr == m_states.end())
        return;

    m_label->setText((*itr).text);

    QPalette p = m_label->palette();
    p.setColor(QPalette::Window, (*itr).color);
    p.setColor(QPalette::WindowText, (*itr).textColor);
    m_label->setPalette(p);
}

void StatusWidget::setDataType(int type)
{
    if(type < 0 || type >= NUM_FLOAT)
        return;

    for(int i = 0; i < NUM_FLOAT; ++i)
        m_typeAction[i]->setChecked(type == i);
    m_dataType = type;

    emit updateForMe();
}

void StatusWidget::showStatusManager()
{
    StatusManager m(this);
    m.exec();
}

StatusWidgetAddBtn::StatusWidgetAddBtn(QWidget *parent) : DataWidgetAddBtn(parent)
{
    setText(tr("Status"));
    setIconSize(QSize(17, 17));
    setIcon(QIcon(":/dataWidgetIcons/status.png"));

    m_widgetType = WIDGET_STATUS;
}

StatusManager::StatusManager(StatusWidget *widget) : QDialog(widget), ui(new Ui::StatusManager)
{
    ui->setupUi(this);

    m_clrMap = NULL;
    m_textClrMap = NULL;

    QHeaderView *header = ui->table->horizontalHeader();
    header->setResizeMode(0, QHeaderView::ResizeToContents);
    header->setResizeMode(1, QHeaderView::Stretch);
    header->setResizeMode(2, QHeaderView::ResizeToContents);
    header->setResizeMode(3, QHeaderView::ResizeToContents);

    updateItems();
}

StatusManager::~StatusManager()
{
    delete ui;
}

void StatusManager::updateItems()
{
    ui->table->clearContents();
    m_rowVals.clear();

    delete m_clrMap;
    delete m_textClrMap;

    m_clrMap = new QSignalMapper(this);
    m_textClrMap = new QSignalMapper(this);

    QHash<quint64, StatusWidget::status>& states = widget()->states();
    ui->table->setRowCount(states.size());

    int rowItr = 0;
    for(QHash<quint64, StatusWidget::status>::iterator itr = states.begin(); itr != states.end(); ++itr,++rowItr)
    {
        m_rowVals.push_back(itr.key());

        QTableWidgetItem *item = new QTableWidgetItem();
        item->setText(QString::number(itr.key()));
        ui->table->setItem(rowItr, 0, item);

        item = new QTableWidgetItem((*itr).text);
        item->setToolTip((*itr).text);
        ui->table->setItem(rowItr, 1, item);

        ColorButton *btn = new ColorButton((*itr).color, this);
        ui->table->setCellWidget(rowItr, 2, btn);
        m_clrMap->setMapping(btn, rowItr);
        connect(btn, SIGNAL(colorChanged(QColor)), m_clrMap, SLOT(map()));

        btn = new ColorButton((*itr).textColor, this);
        ui->table->setCellWidget(rowItr, 3, btn);
        m_textClrMap->setMapping(btn, rowItr);
        connect(btn, SIGNAL(colorChanged(QColor)), m_textClrMap, SLOT(map()));
    }

    connect(m_clrMap,     SIGNAL(mapped(int)), SLOT(colorChanged(int)));
    connect(m_textClrMap, SIGNAL(mapped(int)), SLOT(textColorChanged(int)));
}

void StatusManager::colorChanged(int row)
{
    QColor clr = ((ColorButton*)ui->table->cellWidget(row, 2))->getColor();
    quint64 val = m_rowVals[row];

    getStatus(val)->color = clr;
    widget()->updateValue(val);
}

void StatusManager::textColorChanged(int row)
{
    QColor clr = ((ColorButton*)ui->table->cellWidget(row, 3))->getColor();
    quint64 val = m_rowVals[row];

    getStatus(val)->textColor = clr;
    widget()->updateValue(val);
}

StatusWidget::status *StatusManager::getStatus(quint64 val)
{
    QHash<quint64, StatusWidget::status>::iterator itr = widget()->states().find(val);
    if(itr == widget()->states().end())
        return NULL;
    return &(*itr);
}

void StatusManager::on_table_itemChanged(QTableWidgetItem *item)
{
    if(item->column() >= 2)
        return;

    quint64 val = m_rowVals[item->row()];
    switch(item->column())
    {
        case 0:
        {
            bool ok;
            quint64 id = item->text().toULongLong(&ok);
            if(ok)
            {
                if(id == val)
                    break;
                StatusWidget::status s = *getStatus(val);
                widget()->removeStatus(val);
                widget()->addStatus(id, s);
                updateItems();
            }
            else
                item->setText(QString::number(val));
            break;
        }
        case 1:
        {
            getStatus(val)->text = item->text();
            widget()->updateValue(val);
            break;
        }
    }
}

void StatusManager::on_table_currentItemChanged(QTableWidgetItem *current, QTableWidgetItem *)
{
    ui->rmBtn->setEnabled(current != NULL);
}

void StatusManager::on_addBtn_clicked()
{
    quint64 max = 0;
    for(quint32 i = 0; i < m_rowVals.size(); ++i)
        max = (std::max)(max, m_rowVals[i]);
    ++max;
    widget()->addStatus(max, "", "white", "black");
    updateItems();
}

void StatusManager::on_rmBtn_clicked()
{
    QTableWidgetItem *item = ui->table->currentItem();
    if(!item)
        return;
    widget()->removeStatus(m_rowVals[item->row()]);
    updateItems();
}
