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

SourceDialog::SourceDialog(analyzer_packet *pkt, QWidget *parent) :
    QDialog(parent),ui(new Ui::SourceDialog)
{
    ui->setupUi(this);
    setFixedSize(width(), height());

    if(pkt)
        m_header.Copy(pkt->header);

    QWidget *w = new QWidget();
    scroll_layout = new ScrollDataLayout(&m_header, false, false, NULL, NULL, w);
    w->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Expanding);

    ui->data_scroll->setWidget(w);

    w = new QWidget();
    scroll_header = new LabelLayout(&m_header, true, true, NULL, NULL, w);
    connect(scroll_header, SIGNAL(orderChanged()), scroll_layout, SLOT(UpdateTypes()));

    ui->header_scroll->setWidget(w);

    connect(ui->len_box,        SIGNAL(valueChanged(int)),         scroll_layout, SLOT(lenChanged(int)));
    connect(ui->fmt_combo,      SIGNAL(currentIndexChanged(int)),  scroll_layout, SLOT(fmtChanged(int)));
    connect(ui->len_check,      SIGNAL(toggled(bool)),             this,          SLOT(headerLenToggled(bool)));
    connect(ui->header_len_box, SIGNAL(valueChanged(int)),         this,          SLOT(headerLenChanged(int)));
    connect(ui->static_check,   SIGNAL(toggled(bool)),             this,          SLOT(staticCheckToggled(bool)));
    connect(ui->cmd_check,      SIGNAL(toggled(bool)),             this,          SLOT(cmdCheckToggled(bool)));
    connect(ui->id_check,       SIGNAL(toggled(bool)),             this,          SLOT(idCheckToggled(bool)));
    connect(ui->static_len_box, SIGNAL(valueChanged(int)),         this,          SLOT(staticLenChanged(int)));
    connect(ui->len_fmt_box,    SIGNAL(currentIndexChanged(int)),  this,          SLOT(lenFmtChanged(int)));
    connect(ui->ok_close_bBox,  SIGNAL(clicked(QAbstractButton*)), this,          SLOT(butonnBoxClicked(QAbstractButton*)));
    connect(ui->offsetBox,      SIGNAL(valueChanged(int)),         this,          SLOT(offsetChanged(int)));

    setted = false;
    setFirst = false;

    if(!pkt)
        return;

    scroll_layout->lenChanged(m_header.packet_length);
    ui->len_box->setValue(m_header.packet_length);
    ui->offsetBox->setValue(m_header.len_offset);

    for(quint8 i = 0; i < 4 && m_header.order[i] != 0; ++i)
    {
        switch(m_header.order[i])
        {
            case DATA_LEN:       headerLenToggled(true);   break;
            case DATA_STATIC:    staticCheckToggled(true); break;
            case DATA_DEVICE_ID: idCheckToggled(true);     break;
            case DATA_OPCODE:    cmdCheckToggled(true);    break;
        }
    }

    ui->endianBox->setCurrentIndex(!pkt->big_endian);
    lenFmtChanged(m_header.len_fmt);
    headerLenChanged(m_header.length);
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
    if(ui->ok_close_bBox->buttonRole(b) == QDialogButtonBox::AcceptRole)
    {
        if(m_header.length + ui->len_box->value() == 0)
            return Utils::ThrowException(tr("You have to set something!"), this);
        else
            setted = true;
    }
    close();
}

void SourceDialog::readData(const QByteArray& data)
{
    scroll_layout->SetData(data);
    if(setFirst)
    {
        setFirst = false;
        QString text = Utils::hexToString((quint8)data[0], true);
        QListWidgetItem *item = new QListWidgetItem(text, ui->staticList);
        item->setFlags(item->flags() | Qt::ItemIsEditable);
        ui->staticList->addItem(item);
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

    updateHeaderLabels();
}

void SourceDialog::updateHeaderLabels()
{
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
        ui->len_static->setChecked(true);
    ui->len_check->setChecked(checked);
    AddOrRmHeaderType(checked, DATA_LEN);
}

void SourceDialog::headerLenChanged(int value)
{
    if(value > 0)
        ui->static_check->setChecked(true);

    ui->header_len_box->setValue(value);

    m_header.length = value;
    if(scroll_layout->GetLabelCount() < (quint32)value)
        scroll_layout->lenChanged(value);
    scroll_layout->UpdateTypes();
}

void SourceDialog::staticCheckToggled(bool checked)
{
    AddOrRmHeaderType(checked, DATA_STATIC);
    ui->static_check->setChecked(checked);
    if(checked)
    {
        if(ui->staticList->count() == 0)
            setFirst = true;
    }
}

void SourceDialog::cmdCheckToggled(bool checked)
{
    AddOrRmHeaderType(checked, DATA_OPCODE);
    ui->cmd_check->setChecked(true);
}

void SourceDialog::idCheckToggled(bool checked)
{
    AddOrRmHeaderType(checked, DATA_DEVICE_ID);
    ui->id_check->setChecked(true);
}

void SourceDialog::staticLenChanged(int value)
{
    m_header.static_len = value;
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
        }
        else
            ui->staticList->takeItem(ui->staticList->count() - 1);
    }
}

void SourceDialog::lenFmtChanged(int index)
{
    m_header.len_fmt = index;
    scroll_layout->UpdateTypes();
}

void SourceDialog::offsetChanged(int val)
{
    m_header.len_offset = val;
}

analyzer_packet *SourceDialog::getStructure()
{
    exec();
    if(!setted)
        return NULL;

    bool big_endian = ui->endianBox->currentIndex() == 0;

    m_header.packet_length = ui->len_box->value();

    quint8 *static_data = NULL;
    if(m_header.data_mask & DATA_STATIC)
    {
        static_data = new quint8[m_header.static_len];
        for(quint8 i = 0; i < ui->staticList->count(); ++i)
        {
            QString text = ui->staticList->item(i)->text();
            if(text.contains("0x", Qt::CaseInsensitive))
                static_data[i] = text.toInt(NULL, 16);
            else
                static_data[i] = text.toInt();
        }
    }

    return new analyzer_packet(new analyzer_header(&m_header), big_endian, static_data);
}
