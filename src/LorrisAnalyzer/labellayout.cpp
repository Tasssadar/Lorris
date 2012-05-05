/****************************************************************************
**
**    This file is part of Lorris.
**    Copyright (C) 2012 Vojtěch Boček
**
**    Contact: <vbocek@gmail.com>
**             https://github.com/Tasssadar
**
**    Lorris is free software: you can redistribute it and/or modify
**    it under the terms of the GNU General Public License as published by
**    the Free Software Foundation, either version 3 of the License, or
**    (at your option) any later version.
**
**    Lorris is distributed in the hope that it will be useful,
**    but WITHOUT ANY WARRANTY; without even the implied warranty of
**    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
**    GNU General Public License for more details.
**
**    You should have received a copy of the GNU General Public License
**    along with Lorris.  If not, see <http://www.gnu.org/licenses/>.
**
****************************************************************************/

#include <QLabel>
#include <QMouseEvent>
#include <QDrag>

#include "labellayout.h"
#include "sourcedialog.h"
#include "../common.h"
#include "packet.h"
#include "devicetabwidget.h"
#include "cmdtabwidget.h"

LabelLayout::LabelLayout(analyzer_header *header, bool enable_reorder, bool enable_drag, CmdTabWidget *cmd, DeviceTabWidget *dev, QWidget *parent) : QHBoxLayout(parent)
{
    setSizeConstraint(QLayout::SetMinAndMaxSize);

    if(enable_reorder && enable_drag)
    {
        m_spacer_l = new QSpacerItem(20, 40, QSizePolicy::Expanding, QSizePolicy::Expanding);
        addSpacerItem(m_spacer_l);
    }
    else m_spacer_l = NULL;

    m_spacer_r = new QSpacerItem(20, 40, QSizePolicy::Expanding, QSizePolicy::Expanding);
    addSpacerItem(m_spacer_r);

    m_header = header;
    m_enableReorder = enable_reorder;
    m_enableDrag = enable_drag;
    if(m_enableReorder)
        ((QWidget*)parent)->setAcceptDrops(true);

    quint16 len = (m_header->data_mask & DATA_LEN) ? m_header->length : m_header->packet_length;
    lenChanged(len);
    cmd_w = cmd;
    dev_w = dev;
}

LabelLayout::~LabelLayout()
{
    ClearLabels();
    removeItem(m_spacer_r);
    removeItem(m_spacer_l);
    delete m_spacer_r;
    delete m_spacer_l;
}

void LabelLayout::ClearLabels()
{
    for(quint16 i = 0; i < m_labels.size(); ++i)
    {
        removeWidget(m_labels[i]);
        delete m_labels[i];
    }
    m_labels.clear();
}

void LabelLayout::AddLabel(QString value, qint8 type)
{
    if(type == -1)
        type = GetTypeForPos(m_labels.size());

    DraggableLabel *label = new DraggableLabel(value, m_labels.size(), m_enableReorder, m_enableDrag, this);
    SetLabelType(label, type);
    label->setObjectName(QString::number(m_labels.size()));
    if(m_enableReorder)
        connect(label, SIGNAL(changePos(int,int)), this, SLOT(changePos(int,int)));

    insertWidget(getFirstLabelPos(true), label);
    m_labels.push_back(label);
}

void LabelLayout::RemoveLabel(quint16 index)
{
    removeWidget(m_labels[index]);
    delete m_labels[index];

    std::vector<DraggableLabel*>::iterator itr = m_labels.begin();
    for(quint16 i = 0; i < index; ++i)
        ++itr;
    m_labels.erase(itr);

    for(quint16 i = 0; i < m_labels.size(); ++i)
    {
        m_labels[i]->setObjectName(QString::number(i));
        m_labels[i]->setPos(i);
    }
}

QString LabelLayout::getLabelText(quint32 index)
{
    if(index >= m_labels.size())
        return "";
    return m_labels[index]->GetText();
}

void LabelLayout::changePos(int this_label, int dragged_label)
{
    DraggableLabel *label = m_labels[this_label];
    m_labels[this_label] = m_labels[dragged_label];
    m_labels[dragged_label] = label;
    m_labels[this_label]->setObjectName(QString::number(this_label));
    m_labels[dragged_label]->setObjectName(QString::number(dragged_label));
    m_labels[this_label]->setPos(this_label);
    m_labels[dragged_label]->setPos(dragged_label);

    quint8 type = m_header->order[this_label];
    m_header->order[this_label] = m_header->order[dragged_label];
    m_header->order[dragged_label] = type;

    for(quint16 i = 0; i < m_labels.size(); ++i)
        removeWidget(m_labels[i]);

    for(quint16 i = 0; i < m_labels.size(); ++i)
    {
        SetLabelType(m_labels[i], GetTypeForPos(i));
        insertWidget(getFirstLabelPos(false) + i, m_labels[i]);
    }
    emit orderChanged();
}

void LabelLayout::SetLabelType(DraggableLabel *label, quint8 type)
{
    QString css;
    if(type == DATA_BODY)
        css = "border: 2px solid black; background-color: #00FFFB";
    else if(type & DATA_HEADER)
    {
         css = "border: 2px solid orange; background-color: ";
         if     (type & DATA_DEVICE_ID)  css += "#00FF4C";
         else if(type & DATA_OPCODE)     css += "#E5FF00";
         else if(type & DATA_LEN)        css += "#FFCC66";
         else if(type & DATA_STATIC)     css += "#FF99FF";
    }
    label->setLabelStyleSheet(css);
}

quint8 LabelLayout::GetTypeForPos(quint32 pos)
{
    if(pos < m_header->length)
        return (DATA_HEADER | m_header->order[pos]);
    else
        return DATA_BODY;
}

void LabelLayout::UpdateTypes()
{
    for(quint16 i = 0; i < m_labels.size(); ++i)
        SetLabelType(m_labels[i], GetTypeForPos(i));
}

void LabelLayout::lenChanged(int len)
{
    if(len < 0 || len == (quint16)m_labels.size())
        return;

    while(m_labels.size() != (quint16)len)
    {
        if((uint)len > m_labels.size())
            AddLabel("NULL", -1);
        else
            RemoveLabel();
    }
}

bool LabelLayout::setHightlightLabel(quint32 pos, bool highlight)
{
    if(m_labels.size() <= pos)
        return false;
    m_labels[pos]->setHighlighted(highlight);
    return true;
}

void LabelLayout::setHeader(analyzer_header *header)
{
    m_header = header;

    quint16 len = (m_header->data_mask & DATA_LEN) ? m_header->length : m_header->packet_length;
    lenChanged(len);

    UpdateTypes();
}

ScrollDataLayout::ScrollDataLayout(analyzer_header *header, bool enable_reorder, bool enable_drag,
                                   CmdTabWidget *cmd, DeviceTabWidget *dev, QWidget *parent) :
                                   LabelLayout(header, enable_reorder, enable_drag, cmd, dev, parent)
{
    m_format = FORMAT_HEX;
}

ScrollDataLayout::~ScrollDataLayout()
{

}

void ScrollDataLayout::fmtChanged(int len)
{
    m_format = len;
}

quint8 ScrollDataLayout::GetTypeForPos(quint32 pos)
{
    if(pos < m_header->length)
    {
        quint8 type = DATA_HEADER;
        quint8 pos_h = 0;
        for(quint8 i = 0; i < 4; ++i)
        {
            switch(m_header->order[i])
            {
                case DATA_STATIC:
                    pos_h += m_header->static_len;
                    break;
                case DATA_LEN:
                    pos_h += (1 << m_header->len_fmt);
                    break;
                case DATA_DEVICE_ID:
                case DATA_OPCODE:
                    ++pos_h;
                    break;
                default:
                    return type;
            }
            if(pos < pos_h)
            {
                type |= m_header->order[i];
                break;
            }
        }

        return type;
    }
    return DATA_BODY;
}

void ScrollDataLayout::SetData(const QByteArray& data)
{
    QString value;
    if(m_header->data_mask & DATA_LEN)
    {
        quint8 pos_h = 0;
        for(quint8 i = 0; i < 4; ++i)
        {
            switch(m_header->order[i])
            {
                case DATA_STATIC:
                    pos_h += m_header->static_len;
                    break;
                case DATA_LEN:
                    pos_h += (1 << m_header->len_fmt);
                    break;
                case DATA_DEVICE_ID:
                case DATA_OPCODE:
                    ++pos_h;
                    break;
            }
            if(m_header->order[i] == DATA_LEN)
                break;
        }
        if(data.length() >= pos_h)
        {
            switch(m_header->len_fmt)
            {
                case 0:
                {
                    int len = m_header->length + (int)data[pos_h-1] + m_header->len_offset;
                    lenChanged(len >= 0 ? len : 0);
                    break;
                }
                //TODO: implement
                case 1:
                case 2:
                    break;
            }
        }
    }
    for(quint32 i = 0; i < (quint32)data.length() && i < m_labels.size(); ++i)
    {
        switch(m_format)
        {
            case FORMAT_HEX:    value = Utils::hexToString((quint8)data[i], true); break;
            case FORMAT_BYTE:   value = QString::number((int)data[i]);             break;
            case FORMAT_STRING: value = Utils::parseChar(data[i]);                 break;
        }
        m_labels[i]->setLabelText(value);
    }
}

DraggableLabel::DraggableLabel(const QString &text, quint32 pos, bool drop, bool drag,
                               LabelLayout *l, QWidget *parent, Qt::WindowFlags f) :
    QWidget(parent, f)
{
    setMinimumHeight(50);
    setFixedWidth(50);
    setSizePolicy(QSizePolicy::Fixed, QSizePolicy::Expanding);

    QVBoxLayout *layout = new QVBoxLayout(this);

    valueLabel = new QLabel(text, this);
    valueLabel->setAlignment(Qt::AlignCenter);

    setFont(Utils::getMonospaceFont());
    valueLabel->setAcceptDrops(true);

    layout->setSpacing(0);
    layout->setMargin(0);
    layout->addWidget(valueLabel, 2);

    if(!drop || !drag)
    {
        posLabel = new QLabel(QString::number(pos), this);
        posLabel->setAlignment(Qt::AlignCenter);
        QFont font = posLabel->font();
        font.setPointSize(6);
        posLabel->setFont(font);
        layout->addWidget(posLabel);
    }else posLabel = NULL;

    m_highlighted = false;
    labelLayout = l;
    m_drop = drop;
    m_drag = drag;

    setAcceptDrops(true);

    this->setAutoFillBackground(true);
}

DraggableLabel::~DraggableLabel()
{

}

void DraggableLabel::setLabelStyleSheet(const QString& css)
{
    valueLabel->setStyleSheet(css);
}

void DraggableLabel::setLabelText(const QString& text)
{
    valueLabel->setText(text);
}

void DraggableLabel::setPos(quint32 pos)
{
    if(posLabel)
        posLabel->setText(QString::number(pos));
}

void DraggableLabel::mousePressEvent(QMouseEvent *event)
{
    if(!m_drag)
    {
        QWidget::mousePressEvent(event);
        return;
    }

    if (event->button() == Qt::LeftButton)
    {
        QDrag *drag = new QDrag(this);
        QMimeData *mimeData = new QMimeData;
        if(m_drag && !m_drop && labelLayout)
        {
            mimeData->setText(objectName() + " " +
                              QString::number(labelLayout->getDeviceTab()->getCurrentDevice()) + " " +
                              QString::number(labelLayout->getCmdTab()->getCurrentCmd()));
        }
        else
            mimeData->setText(objectName());
        drag->setMimeData(mimeData);

        QPixmap pixmap(size());
        this->render(&pixmap);

        drag->setPixmap(pixmap);

        drag->exec();
        event->accept();
    }
}

void DraggableLabel::dragEnterEvent(QDragEnterEvent *event)
{
    if(!m_drop || !event->source() || event->mimeData()->text() == objectName())
        return QWidget::dragEnterEvent(event);

    event->acceptProposedAction();
    QString css = valueLabel->styleSheet();
    css.replace(QRegExp("orange"), "red");
    valueLabel->setStyleSheet(css);
}

void DraggableLabel::dragLeaveEvent(QDragLeaveEvent */*event*/)
{
    if(!m_drop)
        return;
    QString css = valueLabel->styleSheet();
    css.replace(QRegExp("red"), "orange");
    valueLabel->setStyleSheet(css);
}

void DraggableLabel::dropEvent(QDropEvent *event)
{
    if(!m_drop)
        return;
    emit changePos(objectName().toInt(), event->mimeData()->text().toInt());
    QString css = valueLabel->styleSheet();
    css.replace(QRegExp("red"), "orange");
    valueLabel->setStyleSheet(css);

    event->acceptProposedAction();
}

void DraggableLabel::setHighlighted(bool highlight)
{
    m_highlighted = highlight;
    QPalette p;
    if(highlight)
        p.setColor(QPalette::Window, Qt::red);
    setPalette(p);
}

QString DraggableLabel::GetText()
{
    return valueLabel->text();
}

