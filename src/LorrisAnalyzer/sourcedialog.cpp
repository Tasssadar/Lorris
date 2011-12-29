#include <QVBoxLayout>
#include <QPushButton>
#include <QSizePolicy>
#include <QScrollArea>
#include <QSpacerItem>

#include "common.h"
#include "sourcedialog.h"
#include "WorkTab/WorkTab.h"
#include "ui_sourcedialog.h"

SourceDialog::SourceDialog(QWidget *parent) :
    QDialog(parent),ui(new Ui::SourceDialog)
{
    ui->setupUi(this);

    QWidget *w = new QWidget();
    scroll_layout = new ScrollDataLayout(w);

    QScrollArea *area = findChild<QScrollArea*>("data_scroll");
    area->setWidget(w);

    QSpinBox *len_static = findChild<QSpinBox*>("len_box");
    connect(len_static, SIGNAL(valueChanged(int)), scroll_layout, SLOT(lenChanged(int)));

    QComboBox *fmt_combo = findChild<QComboBox*>("fmt_combo");
    connect(fmt_combo, SIGNAL(currentIndexChanged(int)), scroll_layout, SLOT(fmtChanged(int)));

    QCheckBox *len_check = findChild<QCheckBox*>("len_check");
    connect(len_check, SIGNAL(toggled(bool)), this, SLOT(headerLenToggled(bool)));

    QSpinBox *header_len_box = findChild<QSpinBox*>("header_len_box");
    connect(header_len_box, SIGNAL(valueChanged(int)), this, SLOT(headerLenChanged(int)));
}

SourceDialog::~SourceDialog()
{
    delete ui;
}

void SourceDialog::readData(QByteArray data)
{
    scroll_layout->SetData(data);
}

void SourceDialog::headerLenToggled(bool checked)
{
    if(!checked)
    {
        QRadioButton *len_static = findChild<QRadioButton*>("len_static");
        len_static->setChecked(true);
    }
}

void SourceDialog::headerLenChanged(int value)
{
    if(value > 0)
    {
        QCheckBox *static_check = findChild<QCheckBox*>("static_check");
        static_check->setChecked(true);
    }
}

ScrollDataLayout::ScrollDataLayout(QWidget *parent) : QHBoxLayout(parent)
{
    setSizeConstraint(QLayout::SetMinAndMaxSize);
    m_spacer = new QSpacerItem(20, 40, QSizePolicy::Expanding, QSizePolicy::Expanding);
    addSpacerItem(m_spacer);
    m_format = FORMAT_HEX;
}

ScrollDataLayout::~ScrollDataLayout()
{
    ClearLabels();
    removeItem(m_spacer);
    delete m_spacer;
}

void ScrollDataLayout::ClearLabels()
{
    for(quint16 i = 0; i < m_labels.size(); ++i)
    {
        removeWidget(m_labels[i]);
        delete m_labels[i];
    }
    m_labels.clear();
}

void ScrollDataLayout::AddLabel(QString value, qint8 type)
{
    if(type == -1)
        type = GetTypeForPos(m_labels.size());

    QLabel *label = new QLabel(value);
    label->setFixedWidth(50);
    label->setSizePolicy(QSizePolicy::Fixed, QSizePolicy::Expanding);
    SetLabelType(label, type);
    label->setAlignment(Qt::AlignCenter);

    QFont font("Monospace");
    font.setStyleHint(QFont::TypeWriter);
    label->setFont(font);

    insertWidget(m_labels.size(), label);
    m_labels.push_back(label);
}

// Remove rightmost label
void ScrollDataLayout::RemoveLabel()
{
    quint16 index = m_labels.size()-1;
    removeWidget(m_labels[index]);
    delete m_labels[index];
    m_labels.pop_back();
}

void ScrollDataLayout::SetLabelType(QLabel *label, quint8 type)
{
    QString css;
    switch(type)
    {
        case DATA_BODY:
            css = "border: 2px solid black; background-color: #00FFFB";
            break;
        case DATA_HEADER:
            css = "border: 2px solid orange; ";
            break;
    }
    if(type & DATA_DEVICE_ID)
        css += "background-color: #00FF4C";
    else if(type & DATA_OPCODE)
        css += "background-color: #E5FF00";
    else if(type & DATA_LEN)
        css += "background-color: #FFCC66";

    label->setStyleSheet(css);
}

// TODO: implement
quint8 ScrollDataLayout::GetTypeForPos(quint32 pos)
{
    return DATA_BODY;
}

void ScrollDataLayout::lenChanged(int len)
{
    if(len == m_labels.size())
        return;

    while(m_labels.size() != len)
    {
        if((uint)len > m_labels.size())
            AddLabel("NULL", -1);
        else
            RemoveLabel();
    }
}

void ScrollDataLayout::fmtChanged(int len)
{
    m_format = len;
}

void ScrollDataLayout::SetData(QByteArray data)
{
    QString value;
    for(quint32 i = 0; i < data.length() && i < m_labels.size(); ++i)
    {
        switch(m_format)
        {
            case FORMAT_HEX:    value = Utils::hexToString((quint8)data[i], true); break;
            case FORMAT_BYTE:   value = QString::number((int)data[i]);             break;
            case FORMAT_STRING: value = Utils::parseChar(data[i]);                 break;
        }
        m_labels[i]->setText(value);
    }
}
