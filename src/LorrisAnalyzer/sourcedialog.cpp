#include <QVBoxLayout>
#include <QPushButton>
#include <QSizePolicy>
#include <QScrollArea>
#include <QSpacerItem>
#include <QDialogButtonBox>

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
    scroll_layout = new ScrollDataLayout(&m_header, false, false, w);

    QScrollArea *area = findChild<QScrollArea*>("data_scroll");
    area->setWidget(w);

    w = new QWidget();
    scroll_header = new LabelLayout(&m_header, true, true, w);
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

void SourceDialog::readData(QByteArray data)
{
    scroll_layout->SetData(data);
}

void SourceDialog::AddOrRmHeaderType(bool add, quint8 type)
{
    if(add)
    {
        QString text = "";
        switch(type)
        {
            case DATA_LEN:       text = tr("Length"); break;
            case DATA_STATIC:    text = tr("Static"); break;
            case DATA_DEVICE_ID: text = tr("ID");     break;
            case DATA_OPCODE:    text = tr("Cmd");    break;
        }
        scroll_header->AddLabel(text, (DATA_HEADER | type));
        m_header.data_mask |= type;
        m_header.AddOrder(type);
    }
    else
    {
        scroll_header->RemoveLabel(quint8(DATA_HEADER | type));
        m_header.data_mask &= ~(type);
        m_header.RmOrder(type);
    }
    scroll_layout->UpdateTypes();
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

    return new analyzer_packet(new analyzer_header(&m_header), big_endian);
}
