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
W_TR(QT_TRANSLATE_NOOP("DataWidget", "Status"))

StatusWidget::StatusWidget(QWidget *parent) :
    DataWidget(parent)
{
    m_dataType = 0;
    m_curUnknown = false;
    m_lastVal = 0;

    resize(200, 80);

    m_emptyLabel = new QLabel(tr("NONE"), this);
    m_emptyLabel->setAlignment(Qt::AlignCenter);
    layout->addWidget(m_emptyLabel);
    layout->addStretch(1);

    m_unknown.text = tr("Unknown: %v");
    m_unknown.color = Qt::white;
    m_unknown.textColor = Qt::black;

    addStatus(0, false, "FALSE", "red", "black");
    addStatus(1, false, "TRUE", "#00FF00", "black");

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
    file->writeBlockIdentifier("statusWStatesV2");
    file->writeVal((quint32)m_states.size());
    for(QLinkedList<status>::iterator itr = m_states.begin(); itr != m_states.end(); ++itr)
    {
        file->writeVal((*itr).id);
        file->writeVal((*itr).mask);
        file->writeString((*itr).text);
        file->writeColor((*itr).color);
        file->writeColor((*itr).textColor);
    }
    file->writeVal(m_lastVal);

    // unk state
    file->writeBlockIdentifier("statusWUnkState");
    file->writeString(m_unknown.text);
    file->writeColor(m_unknown.color);
    file->writeColor(m_unknown.textColor);
    file->writeVal(m_curUnknown);
}

void StatusWidget::loadWidgetInfo(DataFileParser *file)
{
    DataWidget::loadWidgetInfo(file);

    // data type
    if(file->seekToNextBlock("statusWDataType", BLOCK_WIDGET))
        setDataType(file->readVal<int>());

    // states
    bool hasStates = file->seekToNextBlock("statusWStates", BLOCK_WIDGET);
    bool hasStatesV2 = !hasStates ? file->seekToNextBlock("statusWStatesV2", BLOCK_WIDGET) : false;
    if(hasStates || hasStatesV2)
    {
        while(!m_states.isEmpty())
            removeStatus(m_states.front().id,  m_states.front().mask);

        quint32 count = file->readVal<quint32>();
        for(quint32 i = 0; i < count; ++i)
        {
            status s;
            s.id = file->readVal<quint64>();

            if(hasStates)
                s.mask = false;
            else // statesV2
                s.mask = file->readVal<bool>();

            s.text = file->readString();
            s.color = file->readColor();
            s.textColor = file->readColor();

            m_states.push_back(s);
        }
        quint64 val = file->readVal<quint64>();
        m_lastVal = val+1;
        setValue(val);

        m_emptyLabel->setVisible(m_states.isEmpty());
    }

    // unk state
    if(file->seekToNextBlock("statusWUnkState", BLOCK_WIDGET))
    {
        m_unknown.text = file->readString();
        m_unknown.color = file->readColor();
        m_unknown.textColor = file->readColor();
        m_curUnknown = file->readVal<bool>();
        updateUnknown();
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

void StatusWidget::addStatus(quint64 id, bool mask, const QString &text, const QString &color, const QString &textColor)
{
    status s = { id, mask, text, QColor(color), QColor(textColor) };
    addStatus(s);
}

void StatusWidget::addStatus(const status &s)
{
    if(m_states.isEmpty())
    {
        m_emptyLabel->hide();
        m_states.push_back(s);
        setValue(m_lastVal);
    }
    else
    {
        for(QLinkedList<status>::iterator itr = m_states.begin(); itr != m_states.end(); ++itr)
        {
            if((*itr).id == s.id && (*itr).mask == s.mask)
            {
                *itr = s;
                return;
            }
        }
        m_states.push_back(s);
    }
}

void StatusWidget::removeStatus(quint64 id, bool mask)
{
    bool active = false;
    for(QLinkedList<status*>::iterator itr = m_active.begin(); itr != m_active.end(); ++itr)
    {
        if((*itr)->id == id && (*itr)->mask == mask)
        {
            active = true;
            m_active.erase(itr);
            break;
        }
    }

    for(QLinkedList<status>::iterator itr = m_states.begin(); itr != m_states.end(); ++itr)
    {
        if((*itr).id == id && (*itr).mask == mask)
        {
            m_states.erase(itr);
            break;
        }
    }

    if(m_states.isEmpty())
    {
        Q_ASSERT(m_active.isEmpty() || m_curUnknown);

        m_emptyLabel->show();
        updateLabels();
    }
    else if(active)
    {
        updateActive();
    }
}

void StatusWidget::setValue(quint64 id, bool force)
{
    if(!force && id == m_lastVal && !m_curUnknown)
         return;

    m_lastVal = id;

    m_active.clear();

    for(QLinkedList<status>::iterator itr = m_states.begin(); itr != m_states.end(); ++itr)
    {
        if((!(*itr).mask && (*itr).id == id) || ((*itr).mask && (id & (*itr).id)))
            m_active.push_back(&(*itr));
    }

    m_curUnknown = m_active.isEmpty();
    if(m_curUnknown)
    {
        m_unknown.id = id;
        m_active.push_back(&m_unknown);
    }

    updateLabels();
}

void StatusWidget::updateUnknown()
{
    if(!m_curUnknown)
        return;
    if(m_active.isEmpty())
        m_active.push_back(&m_unknown);
    updateLabels();
}

void StatusWidget::setFromStatus(status *st, QLabel *label)
{
    QString text = st->text;
    text.replace("%v", QString::number(st->id));
    text.replace("%h", "0x" + QString::number(st->id, 16).toUpper());
    label->setText(text);

    QPalette p = label->palette();
    p.setColor(QPalette::Window, st->color);
    p.setColor(QPalette::WindowText, st->textColor);
    label->setPalette(p);
}

void StatusWidget::updateActive()
{
    setValue(m_lastVal, true);
}

void StatusWidget::updateLabels()
{
    while((quint32)m_active.size() != m_labels.size())
    {
        if((quint32)m_active.size() > m_labels.size())
        {
            QLabel *label = new QLabel(this);
            label->setAlignment(Qt::AlignCenter);
            label->setMinimumHeight(25);
            label->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Expanding);
            label->setAutoFillBackground(true);
            m_labels.push_back(label);
            layout->insertWidget(layout->count()-1, label);
        }
        else
        {
            delete m_labels.back();
            m_labels.pop_back();
        }
    }

    QLinkedList<status*>::iterator sitr = m_active.begin();
    std::vector<QLabel*>::iterator litr = m_labels.begin();
    for(; sitr != m_active.end(); ++sitr,++litr)
        setFromStatus(*sitr, *litr);
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

void StatusWidget::setUnknownText(const QString &text)
{
    m_unknown.text = text;
    updateUnknown();
}

void StatusWidget::setUnknownColor(const QString &color)
{
    m_unknown.color = QColor(color);
    updateUnknown();
}

void StatusWidget::setUnknownTextColor(const QString &color)
{
    m_unknown.textColor = QColor(color);
    updateUnknown();
}

StatusManager::StatusManager(StatusWidget *widget) : QDialog(widget), ui(new Ui::StatusManager)
{
    ui->setupUi(this);

    m_clrMap = NULL;
    m_textClrMap = NULL;

    QHeaderView *header = ui->table->horizontalHeader();
#if QT_VERSION < 0x050000
    header->setResizeMode(0, QHeaderView::ResizeToContents);
    header->setResizeMode(1, QHeaderView::Stretch);
    header->setResizeMode(2, QHeaderView::ResizeToContents);
    header->setResizeMode(3, QHeaderView::ResizeToContents);
#else
    header->setSectionResizeMode(0, QHeaderView::ResizeToContents);
    header->setSectionResizeMode(1, QHeaderView::Stretch);
    header->setSectionResizeMode(2, QHeaderView::ResizeToContents);
    header->setSectionResizeMode(3, QHeaderView::ResizeToContents);
#endif

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

    QLinkedList<StatusWidget::status>& states = widget()->states();

    ui->table->setRowCount(states.size());

    QLinkedList<StatusWidget::status>::iterator itr = states.begin();
    for(int rowItr = 0; itr != states.end(); ++itr,++rowItr)
    {
        m_rowVals.push_back(std::make_pair((*itr).id, (*itr).mask));

        QTableWidgetItem *item = new QTableWidgetItem();
        item->setText(QString("%1%2").arg((*itr).mask ? "&" : "").arg((*itr).id));
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

    const StatusWidget::status& unk = widget()->unknown();
    ui->unkText->setText(unk.text);
    ui->unkColor->setColor(unk.color);
    ui->unkTextColor->setColor(unk.textColor);
}

void StatusManager::colorChanged(int row)
{
    QColor clr = ((ColorButton*)ui->table->cellWidget(row, 2))->getColor();

    getStatus(m_rowVals[row].first, m_rowVals[row].second)->color = clr;
    widget()->updateActive();
}

void StatusManager::textColorChanged(int row)
{
    QColor clr = ((ColorButton*)ui->table->cellWidget(row, 3))->getColor();

    getStatus(m_rowVals[row].first, m_rowVals[row].second)->textColor = clr;
    widget()->updateActive();
}

StatusWidget::status *StatusManager::getStatus(quint64 val, bool mask)
{
    QLinkedList<StatusWidget::status>& states = widget()->states();
    QLinkedList<StatusWidget::status>::iterator itr = states.begin();

    for(; itr != states.end(); ++itr)
        if((*itr).id == val && (*itr).mask == mask)
            return &(*itr);

    return NULL;
}

void StatusManager::on_table_itemChanged(QTableWidgetItem *item)
{
    if(item->column() >= 2)
        return;

    quint64 val = m_rowVals[item->row()].first;
    bool val_mask = m_rowVals[item->row()].second;
    switch(item->column())
    {
        case 0:
        {
            bool ok;
            QString str = item->text();
            str.remove(' ');

            bool mask = false;
            quint64 id = 0;

            int base = 10;
            if(str.contains("0x", Qt::CaseInsensitive))
                base = 16;

            if(str.startsWith('&'))
            {
                mask = true;
                id = str.mid(1).toULongLong(&ok, base);
            }
            else
                id = str.toULongLong(&ok, base);

            if(ok)
            {
                if(id == val && val_mask == mask)
                    break;
                StatusWidget::status s = *getStatus(val, val_mask);
                widget()->removeStatus(val, val_mask);
                s.id = id;
                s.mask = mask;
                widget()->addStatus(s);
                updateItems();
            }
            else
                item->setText(QString::number(val));
            break;
        }
        case 1:
        {
            getStatus(val, val_mask)->text = item->text();
            widget()->updateActive();
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
        max = (std::max)(max, m_rowVals[i].first);
    ++max;
    widget()->addStatus(max, "", "white", "black");
    updateItems();
}

void StatusManager::on_rmBtn_clicked()
{
    QTableWidgetItem *item = ui->table->currentItem();
    if(!item)
        return;
    widget()->removeStatus(m_rowVals[item->row()].first, m_rowVals[item->row()].second);
    updateItems();
}

void StatusManager::on_unkText_editingFinished()
{
    widget()->setUnknownText(ui->unkText->text());
}

void StatusManager::on_unkColor_colorChanged(const QColor &color)
{
    widget()->setUnknownColor(color.name());
}

void StatusManager::on_unkTextColor_colorChanged(const QColor &color)
{
    widget()->setUnknownTextColor(color.name());
}
