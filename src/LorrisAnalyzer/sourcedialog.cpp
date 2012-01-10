#include <QVBoxLayout>
#include <QPushButton>
#include <QSizePolicy>
#include <QScrollArea>
#include <QSpacerItem>
#include <QDialogButtonBox>
#include <QListWidget>

#include "common.h"
#include "sourcedialog.h"
#include "WorkTab/WorkTab.h"
#include "ui_sourcedialog.h"
#include "labellayout.h"
#include "packet.h"

SourceDialog::SourceDialog(QWidget *parent) :
    QDialog(parent),ui(new Ui::SourceDialog)
{
    ui->setupUi(this);
    setFixedSize(width(), height());

    QWidget *w = new QWidget();
    scroll_layout = new ScrollDataLayout(&m_header, false, false, NULL, NULL, w);

    QScrollArea *area = findChild<QScrollArea*>("data_scroll");
    area->setWidget(w);

    w = new QWidget();
    scroll_header = new LabelLayout(&m_header, true, true, NULL, NULL, w);
    connect(scroll_header, SIGNAL(orderChanged()), scroll_layout, SLOT(UpdateTypes()));

    area = findChild<QScrollArea*>("header_scroll");
    area->setWidget(w);

    QSpinBox *len_static = findChild<QSpinBox*>("len_box");
    connect(len_static, SIGNAL(valueChanged(int)), scroll_layout, SLOT(lenChanged(int)));

    QComboBox *fmt_combo = findChild<QComboBox*>("fmt_combo");
    connect(fmt_combo, SIGNAL(currentIndexChanged(int)), scroll_layout, SLOT(fmtChanged(int)));

    QCheckBox *len_check = findChild<QCheckBox*>("len_check");
    connect(len_check, SIGNAL(toggled(bool)), this, SLOT(headerLenToggled(bool)));

    QSpinBox *header_len_box = findChild<QSpinBox*>("header_len_box");
    connect(header_len_box, SIGNAL(valueChanged(int)), this, SLOT(headerLenChanged(int)));

    QCheckBox *static_check = findChild<QCheckBox*>("static_check");
    connect(static_check, SIGNAL(toggled(bool)), this, SLOT(staticCheckToggled(bool)));

    QCheckBox *cmd_check = findChild<QCheckBox*>("cmd_check");
    connect(cmd_check, SIGNAL(toggled(bool)), this, SLOT(cmdCheckToggled(bool)));

    QCheckBox *id_check = findChild<QCheckBox*>("id_check");
    connect(id_check, SIGNAL(toggled(bool)), this, SLOT(idCheckToggled(bool)));

    QSpinBox *static_len_box = findChild<QSpinBox*>("static_len_box");
    connect(static_len_box, SIGNAL(valueChanged(int)), this, SLOT(staticLenChanged(int)));

    QComboBox *len_fmt_box = findChild<QComboBox*>("len_fmt_box");
    connect(len_fmt_box, SIGNAL(currentIndexChanged(int)), this, SLOT(lenFmtChanged(int)));

    QDialogButtonBox *ok_close_bBox = findChild<QDialogButtonBox*>("ok_close_bBox");
    connect(ok_close_bBox, SIGNAL(clicked(QAbstractButton*)), this, SLOT(butonnBoxClicked(QAbstractButton*)));

    setted = false;
    setFirst = false;
}

SourceDialog::~SourceDialog()
{
    delete ui;
    QObject *w = scroll_layout->parent();
    delete scroll_layout;
    delete w;
    w = scroll_header->parent();
    delete scroll_header;
    delete w;
}

void SourceDialog::butonnBoxClicked(QAbstractButton *b)
{
    QDialogButtonBox *ok_close_bBox = findChild<QDialogButtonBox*>("ok_close_bBox");
    if(ok_close_bBox->buttonRole(b) == QDialogButtonBox::AcceptRole)
        setted = true;
    close();
}

void SourceDialog::readData(const QByteArray& data)
{
    scroll_layout->SetData(data);
    if(setFirst)
    {
        setFirst = false;
        QListWidget *list = findChild<QListWidget*>("staticList");
        QString text = Utils::hexToString((quint8)data[0], true);
        QListWidgetItem *item = new QListWidgetItem(text, list);
        item->setFlags(item->flags() | Qt::ItemIsEditable);
        list->addItem(item);
    }
}

void SourceDialog::AddOrRmHeaderType(bool add, quint8 type)
{
    if(add)
    {        
        m_header.data_mask |= type;
        m_header.AddOrder(type);
    }
    else
    {
        m_header.data_mask &= ~(type);
        m_header.RmOrder(type);
    }

    scroll_header->parentWidget()->setUpdatesEnabled(false);
    scroll_header->ClearLabels();

    for(quint8 i = 0; i < 4 && m_header.order[i] != 0; ++i)
    {
        QString text = "";
        switch(m_header.order[i])
        {
            case DATA_LEN:       text = tr("Length"); break;
            case DATA_STATIC:    text = tr("Static"); break;
            case DATA_DEVICE_ID: text = tr("ID");     break;
            case DATA_OPCODE:    text = tr("Cmd");    break;
        }
        scroll_header->AddLabel(text, (DATA_HEADER | m_header.order[i]));
    }
    scroll_layout->UpdateTypes();
    scroll_header->parentWidget()->setUpdatesEnabled(true);
}

void SourceDialog::headerLenToggled(bool checked)
{
    if(!checked)
    {
        QRadioButton *len_static = findChild<QRadioButton*>("len_static");
        len_static->setChecked(true);
    }
    AddOrRmHeaderType(checked, DATA_LEN);
}

void SourceDialog::headerLenChanged(int value)
{
    if(value > 0)
    {
        QCheckBox *static_check = findChild<QCheckBox*>("static_check");
        static_check->setChecked(true);
    }
    m_header.length = value;
    if(scroll_layout->GetLabelCount() < (quint32)value)
        scroll_layout->lenChanged(value);
    scroll_layout->UpdateTypes();
}

void SourceDialog::staticCheckToggled(bool checked)
{
    AddOrRmHeaderType(checked, DATA_STATIC);
    if(checked)
    {
        QListWidget *list = findChild<QListWidget*>("staticList");
        if(list->count() == 0)
            setFirst = true;
    }
}

void SourceDialog::cmdCheckToggled(bool checked)
{
    AddOrRmHeaderType(checked, DATA_OPCODE);
}

void SourceDialog::idCheckToggled(bool checked)
{
    AddOrRmHeaderType(checked, DATA_DEVICE_ID);
}

void SourceDialog::staticLenChanged(int value)
{
    m_header.static_len = value;
    scroll_layout->UpdateTypes();

    QListWidget *list = findChild<QListWidget*>("staticList");
    while(list->count() != value)
    {
        if(list->count() < value)
        {
            QString text = scroll_layout->getLabelText(list->count());
            if(text.length() == 0)
                text = "0x00";
            QListWidgetItem *item = new QListWidgetItem(text, list);
            item->setFlags(item->flags() | Qt::ItemIsEditable);
            list->addItem(item);
        }
        else
            list->takeItem(list->count() - 1);
    }
}

void SourceDialog::lenFmtChanged(int index)
{
    m_header.len_fmt = index;
    scroll_layout->UpdateTypes();
}

analyzer_packet *SourceDialog::getStructure()
{
    exec();
    if(!setted)
        return NULL;

    QComboBox *endianBox = findChild<QComboBox*>("endianBox");
    bool big_endian = endianBox->currentIndex() == 0;

    QSpinBox *len_static = findChild<QSpinBox*>("len_box");
    m_header.packet_length = len_static->value();

    quint8 *static_data = NULL;
    if(m_header.data_mask & DATA_STATIC)
    {
        static_data = new quint8[m_header.static_len];
        QListWidget *list = findChild<QListWidget*>("staticList");
        for(quint8 i = 0; i < list->count(); ++i)
        {
            QString text = list->item(i)->text();
            if(text.contains("0x", Qt::CaseInsensitive))
                static_data[i] = text.toInt(NULL, 16);
            else
                static_data[i] = text.toInt();
        }
    }

    return new analyzer_packet(new analyzer_header(&m_header), big_endian, static_data);
}
