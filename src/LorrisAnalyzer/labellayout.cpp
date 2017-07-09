/**********************************************
**    This file is part of Lorris
**    http://tasssadar.github.com/Lorris/
**
**    See README and COPYING
***********************************************/

#include <QLabel>
#include <QMouseEvent>
#include <QDrag>
#include <QTimer>
#include <QMimeData>

#include "labellayout.h"
#include "sourcedialog.h"
#include "../common.h"
#include "packet.h"
#include "filtertabwidget.h"

LabelLayout::LabelLayout(analyzer_header *header, bool enable_reorder, bool enable_drag, FilterTabWidget *filters, QWidget *parent) : QHBoxLayout(parent)
{
    setSizeConstraint(QLayout::SetMinAndMaxSize);

#ifdef Q_OS_MAC
    parent->setStyleSheet("background-color: transparent;");
#endif

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

    quint16 len = m_header->hasLen() ? m_header->length : m_header->packet_length;
    lenChanged(len);

    m_filterWidget = filters;

    QTimer *freeTimer = new QTimer(this);
    freeTimer->start(1000);
    connect(freeTimer, SIGNAL(timeout()), SLOT(freeLabels()));
}

LabelLayout::~LabelLayout()
{
    ClearLabels();
    freeLabels();

    removeItem(m_spacer_r);
    removeItem(m_spacer_l);
    delete m_spacer_r;
    delete m_spacer_l;
}

void LabelLayout::ClearLabels()
{
    for(quint32 i = 0; i < m_labels.size(); ++i)
    {
        removeWidget(m_labels[i]);
        setLabelFreed(m_labels[i]);
    }
    m_labels.clear();
}

void LabelLayout::freeLabels()
{
    for(quint32 i = 0; i < m_freedLabels.size(); ++i)
        delete m_freedLabels[i];
    m_freedLabels.clear();
}

void LabelLayout::setLabelFreed(DraggableLabel *label)
{
    label->setVisible(false);
    disconnect(label, 0, this, 0);
    m_freedLabels.push_back(label);
}

void LabelLayout::AddLabel(QString value, qint8 type)
{
    if(type == -1)
        type = GetTypeForPos(m_labels.size());

    DraggableLabel *label = NULL;

    if(m_freedLabels.empty())
        label = new DraggableLabel(value, m_labels.size(), m_enableReorder, m_enableDrag, this);
    else
    {
        label = m_freedLabels.back();
        m_freedLabels.pop_back();

        label->setLabelText(value);
        label->setPos(m_labels.size());
        label->setVisible(true);
    }

    SetLabelType(label, type);

    if(m_enableReorder)
        connect(label, SIGNAL(changePos(int,int)), this, SLOT(changePos(int,int)));

    insertWidget(getFirstLabelPos(true), label);
    m_labels.push_back(label);
}

void LabelLayout::RemoveLabel(quint16 index)
{
    if(index >= m_labels.size())
        return;

    removeWidget(m_labels[index]);
    setLabelFreed(m_labels[index]);

    m_labels.erase(m_labels.begin()+index);

    for(quint16 i = index; i < m_labels.size(); ++i)
        m_labels[i]->setPos(i);
}

QString LabelLayout::getLabelText(quint32 index)
{
    if(index >= m_labels.size())
        return "";
    return m_labels[index]->GetText();
}

int LabelLayout::getLabelPos(DraggableLabel *label)
{
    for(quint32 i = 0; i < m_labels.size(); ++i)
        if(m_labels[i] == label)
            return i;
    return -1;
}

void LabelLayout::changePos(int this_label, int dragged_label)
{
    DraggableLabel *label = m_labels[this_label];
    m_labels[this_label] = m_labels[dragged_label];
    m_labels[dragged_label] = label;
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
         switch(type & ~(DATA_HEADER))
         {
            case DATA_DEVICE_ID:
                css += "#00FF4C";
                break;
            case DATA_OPCODE:
                css += "#E5FF00";
                break;
            case DATA_LEN:
                css += "#FFCC66";
                break;
            case DATA_STATIC:
                css += "#FF99FF";
                break;
            case DATA_AVAKAR:
                css = "border: 2px solid orange; background: qlineargradient(x1: 0, y1: 0, x2: 0, y2: 1,"
                        "stop: 0.50 #E5FF00, stop: 0.51 #FFCC66);";
                break;
         }
    }
    label->setLabelStyleSheet(css);
}

quint8 LabelLayout::GetTypeForPos(quint32 pos)
{
    if(m_enableReorder) {
        if(pos < 4) {
            return DATA_HEADER | m_header->order[pos];
        }
    } else if(pos < m_header->length) {
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
            case DATA_AVAKAR:
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

    quint16 len = m_header->hasLen() ? m_header->length : m_header->packet_length;
    lenChanged(len);

    UpdateTypes();
}

ScrollDataLayout::ScrollDataLayout(analyzer_header *header, bool enable_reorder, bool enable_drag,
                                   FilterTabWidget *filters, QWidget *parent) :
                                   LabelLayout(header, enable_reorder, enable_drag, filters, parent)
{
    m_format = FORMAT_HEX;
}

ScrollDataLayout::~ScrollDataLayout()
{

}

void ScrollDataLayout::fmtChanged(int fmt)
{
    m_format = fmt;
}

void ScrollDataLayout::SetData(analyzer_data *data)
{
    QString value;
    const QByteArray& bytes = data->getData();

    lenChanged(data->getLenght());

    for(quint32 i = 0; i < (quint32)bytes.size() && i < m_labels.size(); ++i)
    {
        switch(m_format)
        {
            case FORMAT_HEX:    value = Utils::hexToString((quint8)bytes[i], true); break;
            case FORMAT_BYTE:   value = QString::number((int)bytes[i]);             break;
            case FORMAT_STRING: value = Utils::parseChar(bytes[i]);                 break;
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
    valueLabel->setFont(Utils::getMonospaceFont());
    valueLabel->setAcceptDrops(m_drop);

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

    setAcceptDrops(m_drop);

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

        QByteArray data;
        QDataStream str(&data, QIODevice::WriteOnly);
        str << labelLayout->getLabelPos(this);

        if(m_drag && !m_drop && labelLayout)
        {
            DataFilter *f = labelLayout->getFilterTabs()->getCurrFilter();
            str.writeRawData((const char*)&f, sizeof(DataFilter*));
            mimeData->setData("analyzer/dragLabel", data);
        }
        else
            mimeData->setData("analyzer/dropLabel", data);

        drag->setMimeData(mimeData);

        QPixmap pixmap(size());
        this->render(&pixmap);

        drag->setPixmap(pixmap);

        drag->exec();
        delete drag;

        event->accept();
    }
}

void DraggableLabel::dragEnterEvent(QDragEnterEvent *event)
{
    const QMimeData *mime = event->mimeData();
    if(!m_drop || !mime || !mime->hasFormat("analyzer/dropLabel") || !event->source()) {
        event->ignore();
        return QWidget::dragEnterEvent(event);
    }

    int pos;
    int myPos = labelLayout->getLabelPos(this);

    QByteArray data = mime->data("analyzer/dropLabel");
    QDataStream str(data);
    str >> pos;

    if(myPos == pos)
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
    if(!m_drop) {
        event->ignore();
        QWidget::dropEvent(event);
        return;
    }

    int myPos = labelLayout->getLabelPos(this);
    int pos;

    QByteArray data = event->mimeData()->data("analyzer/dropLabel");
    QDataStream str(data);
    str >> pos;

    emit changePos(myPos, pos);

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
