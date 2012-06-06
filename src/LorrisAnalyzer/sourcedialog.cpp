/**********************************************
**    This file is part of Lorris
**    http://tasssadar.github.com/Lorris/
**
**    See README and COPYING
***********************************************/

#include <QVBoxLayout>
#include <QPushButton>
#include <QSizePolicy>
#include <QScrollArea>
#include <QSpacerItem>
#include <QDialogButtonBox>
#include <QListWidget>

#include "../common.h"
#include "sourcedialog.h"
#include "../WorkTab/WorkTab.h"
#include "ui_sourcedialog.h"
#include "labellayout.h"
#include "packet.h"
#include "packetparser.h"

SourceDialog::SourceDialog(analyzer_packet *pkt, QWidget *parent) :
    QDialog(parent),ui(new Ui::SourceDialog)
{
    ui->setupUi(this);
    setFixedSize(width(), height());

    if(pkt)
        m_packet.copy(pkt);
    else
        m_packet.header = new analyzer_header();

    m_parser = new PacketParser(NULL, this);
    m_parser->setPacket(&m_packet);

    QWidget *w = new QWidget(this);
    scroll_layout = new ScrollDataLayout(m_packet.header, false, false, NULL, NULL, w);
    w->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Expanding);

    ui->data_scroll->setWidget(w);

    w = new QWidget(this);
    scroll_header = new LabelLayout(m_packet.header, true, true, NULL, NULL, w);
    connect(scroll_header, SIGNAL(orderChanged()), scroll_layout, SLOT(UpdateTypes()));
    connect(scroll_header, SIGNAL(orderChanged()), m_parser,      SLOT(resetCurPacket()));

    ui->header_scroll->setWidget(w);

    connect(this,               SIGNAL(readData(QByteArray)),     m_parser,      SLOT(newData(QByteArray)));
    connect(ui->len_box,        SIGNAL(valueChanged(int)),        scroll_layout, SLOT(lenChanged(int)));
    connect(ui->len_box,        SIGNAL(valueChanged(int)),                       SLOT(packetLenChanged(int)));
    connect(ui->fmt_combo,      SIGNAL(currentIndexChanged(int)), scroll_layout, SLOT(fmtChanged(int)));
    connect(ui->len_check,      SIGNAL(toggled(bool)),                           SLOT(headerLenToggled(bool)));
    connect(ui->header_len_box, SIGNAL(valueChanged(int)),                       SLOT(headerLenChanged(int)));
    connect(ui->static_check,   SIGNAL(toggled(bool)),                           SLOT(staticCheckToggled(bool)));
    connect(ui->cmd_check,      SIGNAL(toggled(bool)),                           SLOT(cmdCheckToggled(bool)));
    connect(ui->id_check,       SIGNAL(toggled(bool)),                           SLOT(idCheckToggled(bool)));
    connect(ui->static_len_box, SIGNAL(valueChanged(int)),                       SLOT(staticLenChanged(int)));
    connect(ui->len_fmt_box,    SIGNAL(currentIndexChanged(int)),                SLOT(lenFmtChanged(int)));
    connect(ui->ok_close_bBox,  SIGNAL(clicked(QAbstractButton*)),               SLOT(butonnBoxClicked(QAbstractButton*)));
    connect(ui->offsetBox,      SIGNAL(valueChanged(int)),                       SLOT(offsetChanged(int)));
    connect(ui->staticList,     SIGNAL(itemChanged(QListWidgetItem*)),           SLOT(staticDataChanged(QListWidgetItem*)));
    connect(ui->endianBox,      SIGNAL(currentIndexChanged(int)),                SLOT(endianChanged(int)));
    connect(m_parser,           SIGNAL(packetReceived(analyzer_data*,quint32)),  SLOT(packetReceived(analyzer_data*,quint32)));

    setted = false;

    if(!pkt)
        return;

    scroll_layout->lenChanged(m_packet.header->packet_length);
    ui->len_box->setValue(m_packet.header->packet_length);
    ui->offsetBox->setValue(m_packet.header->len_offset);

    for(quint8 i = 0; i < 4 && m_packet.header->order[i] != 0; ++i)
    {
        switch(m_packet.header->order[i])
        {
            case DATA_LEN:       headerLenToggled(true);   break;
            case DATA_STATIC:    staticCheckToggled(true); break;
            case DATA_DEVICE_ID: idCheckToggled(true);     break;
            case DATA_OPCODE:    cmdCheckToggled(true);    break;
        }
    }

    ui->endianBox->setCurrentIndex(!pkt->big_endian);
    lenFmtChanged(m_packet.header->len_fmt);
    headerLenChanged(m_packet.header->length);

    if(m_packet.header->data_mask & DATA_STATIC)
    {
        ui->static_len_box->setValue(pkt->header->static_len);
        staticLenChanged(pkt->header->static_len);

        for(quint8 i = 0; i < pkt->header->static_len; ++i)
        {
            ui->staticList->item(i)->setText(Utils::hexToString(pkt->static_data[i], true));
            m_packet.static_data[i] = pkt->static_data[i];
        }
    }
}

SourceDialog::~SourceDialog()
{
    delete ui;
    delete m_packet.header;
}

void SourceDialog::butonnBoxClicked(QAbstractButton *b)
{
    if(ui->ok_close_bBox->buttonRole(b) == QDialogButtonBox::AcceptRole)
    {
        if(m_packet.header->length + ui->len_box->value() == 0)
            return Utils::ThrowException(tr("You have to set something!"), this);
        else
            setted = true;
    }
    close();
}

void SourceDialog::packetReceived(analyzer_data *data, quint32)
{
    scroll_layout->SetData(data->getData());
}

void SourceDialog::AddOrRmHeaderType(bool add, quint8 type)
{
    if(add)
    {        
        m_packet.header->data_mask |= type;
        m_packet.header->AddOrder(type);
    }
    else
    {
        m_packet.header->data_mask &= ~(type);
        m_packet.header->RmOrder(type);
    }

    updateHeaderLabels();
    m_parser->resetCurPacket();
}

void SourceDialog::updateHeaderLabels()
{
    scroll_header->parentWidget()->setUpdatesEnabled(false);
    scroll_header->ClearLabels();

    for(quint8 i = 0; i < 4 && m_packet.header->order[i] != 0; ++i)
    {
        QString text = "";
        switch(m_packet.header->order[i])
        {
            case DATA_LEN:       text = tr("Length"); break;
            case DATA_STATIC:    text = tr("Static"); break;
            case DATA_DEVICE_ID: text = tr("ID");     break;
            case DATA_OPCODE:    text = tr("Cmd");    break;
        }
        scroll_header->AddLabel(text, (DATA_HEADER | m_packet.header->order[i]));
    }
    scroll_layout->UpdateTypes();
    scroll_header->parentWidget()->setUpdatesEnabled(true);
}

void SourceDialog::headerLenToggled(bool checked)
{
    if(!checked)
        ui->len_static->setChecked(true);
    ui->len_check->setChecked(checked);
    AddOrRmHeaderType(checked, DATA_LEN);
}

void SourceDialog::headerLenChanged(int value)
{
    if(value > 0)
        ui->static_check->setChecked(true);

    ui->header_len_box->setValue(value);

    m_packet.header->length = value;
    if(scroll_layout->GetLabelCount() < (quint32)value)
        scroll_layout->lenChanged(value);
    scroll_layout->UpdateTypes();

    m_parser->resetCurPacket();
}

void SourceDialog::staticCheckToggled(bool checked)
{
    AddOrRmHeaderType(checked, DATA_STATIC);
    ui->static_check->setChecked(checked);

    staticLenChanged(ui->static_len_box->value());
}

void SourceDialog::cmdCheckToggled(bool checked)
{
    AddOrRmHeaderType(checked, DATA_OPCODE);
    ui->cmd_check->setChecked(checked);
}

void SourceDialog::idCheckToggled(bool checked)
{
    AddOrRmHeaderType(checked, DATA_DEVICE_ID);
    ui->id_check->setChecked(checked);
}

void SourceDialog::staticLenChanged(int value)
{
    m_packet.header->static_len = value;
    m_packet.static_data.resize(value);
    scroll_layout->UpdateTypes();

    while(ui->staticList->count() != value)
    {
        if(ui->staticList->count() < value)
        {
            QString text = scroll_layout->getLabelText(ui->staticList->count());
            if(text.length() == 0)
                text = "0x00";

            QListWidgetItem *item = new QListWidgetItem(text, ui->staticList);
            item->setFlags(item->flags() | Qt::ItemIsEditable);
            ui->staticList->addItem(item);

            if(text.contains("0x", Qt::CaseInsensitive))
                m_packet.static_data[ui->staticList->count()-1] = text.toInt(NULL, 16);
            else
                m_packet.static_data[ui->staticList->count()-1] = text.toInt();
        }
        else
            ui->staticList->takeItem(ui->staticList->count() - 1);
    }
    m_parser->resetCurPacket();
}

void SourceDialog::lenFmtChanged(int index)
{
    m_packet.header->len_fmt = index;
    scroll_layout->UpdateTypes();
    m_parser->resetCurPacket();
}

void SourceDialog::offsetChanged(int val)
{
    m_packet.header->len_offset = val;
    m_parser->resetCurPacket();
}

void SourceDialog::endianChanged(int idx)
{
    m_packet.big_endian = (idx == 0);
    m_parser->resetCurPacket();
}

void SourceDialog::packetLenChanged(int val)
{
    m_packet.header->packet_length = val;
    m_parser->resetCurPacket();
}

void SourceDialog::staticDataChanged(QListWidgetItem *)
{
    if(!ui->staticList->currentItem() || !(m_packet.header->data_mask & DATA_STATIC))
        return;

    for(quint8 i = 0; i < ui->staticList->count(); ++i)
    {
        QString text = ui->staticList->item(i)->text();
        if(text.contains("0x", Qt::CaseInsensitive))
            m_packet.static_data[i] = text.toInt(NULL, 16);
        else
            m_packet.static_data[i] = text.toInt();
    }
    m_parser->resetCurPacket();
}

analyzer_packet *SourceDialog::getStructure()
{
    exec();
    if(!setted)
        return NULL;
    return new analyzer_packet(&m_packet);
}
