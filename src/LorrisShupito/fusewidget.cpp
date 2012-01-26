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
#include <QVBoxLayout>
#include <QPushButton>
#include <QHBoxLayout>
#include <QComboBox>
#include <QFormLayout>
#include <QMenu>
#include <QContextMenuEvent>

#include "common.h"
#include "fusewidget.h"

FuseWidget::FuseWidget(QWidget *parent) :
    QFrame(parent)
{
    m_layout = new QVBoxLayout(this);
    m_fuse_layout = new QFormLayout();
    m_fuse_layout->setFormAlignment(Qt::AlignLeft);
    m_fuse_layout->setLabelAlignment(Qt::AlignLeft);

    QLabel *fuses = new QLabel(tr("Fuses"), this);
    fuses->setAlignment(Qt::AlignCenter);

    QFrame *line = new QFrame(this);
    line->setFrameStyle(QFrame::HLine | QFrame::Raised);

    readFusesBtn = new QPushButton(tr("Read fuses"), this);

    QSpacerItem *spacer = new QSpacerItem(10, 10, QSizePolicy::Minimum, QSizePolicy::Expanding);

    m_layout->addWidget(fuses);
    m_layout->addWidget(line);
    m_layout->addWidget(readFusesBtn);
    m_layout->addLayout(m_fuse_layout);
    m_layout->addItem(spacer);

    setFrameStyle(QFrame::Panel | QFrame::Plain);
    setContextMenuPolicy(Qt::DefaultContextMenu);

    contextMenu = new QMenu(this);
    QAction *read = contextMenu->addAction(tr("Read fuses"));

    contextMenu->addSeparator();

    rememberAct = contextMenu->addAction(tr("Remember fuses"));
    rememberAct->setEnabled(false);

    writeAct = contextMenu->addAction("Write fuses");
    writeAct->setEnabled(false);

    connect(read,         SIGNAL(triggered()), this, SIGNAL(readFuses()));
    connect(writeAct,     SIGNAL(triggered()), this, SIGNAL(writeFuses()));
    connect(rememberAct,  SIGNAL(triggered()), this, SLOT(rememberFuses()));
    connect(readFusesBtn, SIGNAL(clicked()),   this, SIGNAL(readFuses()));

    m_changed = false;
}

FuseWidget::~FuseWidget()
{
    clear();
}

void FuseWidget::clear()
{
    for(quint16 i = 0; i < m_fuses.size(); ++i)
    {
        delete m_fuses[i]->label;
        delete m_fuses[i]->box;
        delete m_fuses[i];
    }
    m_fuses.clear();
}

void FuseWidget::contextMenuEvent( QContextMenuEvent * event )
{
    contextMenu->exec(event->globalPos());
}

// void update_fuse_window(), avr232client.cpp
void FuseWidget::setFuses(std::vector<chip_definition::fuse>& fuses)
{
    if(fuses.size() == 0)
        return;

    if(readFusesBtn)
    {
        delete readFusesBtn;
        readFusesBtn = NULL;
    }

    disconnect(this, SLOT(changed(int)));
    clear();

    for(quint16 i = 0; i < fuses.size(); ++i)
    {
        chip_definition::fuse& f = fuses[i];

        fuse_line *line = new fuse_line();
        line->fuse = f;
        line->label = new QLabel(f.name, this);
        line->box = new QComboBox(this);

        int fuse_value = chip_definition::get_fuse_value(m_fuse_data.begin(), m_fuse_data.end(), f);
        int fuse_value_index = -1;
        if(!f.values.empty())
        {
            for(std::size_t j = 0; j < f.values.size(); ++j)
            {
                if(fuse_value == f.values[j])
                    fuse_value_index = j;
                line->box->addItem(Utils::toBinary(f.bits.size(), f.values[j]));
            }
            if(fuse_value_index == -1)
            {
                line->box->addItem(Utils::toBinary(f.bits.size(), fuse_value));
                fuse_value_index = f.values.size();
            }
        }
        else
        {
            for (std::size_t j = 0; j < (1<<f.bits.size()); ++j)
            {
                if (fuse_value == (int)j)
                    fuse_value_index = j;
                line->box->addItem(Utils::toBinary(f.bits.size(), j));
            }
        }
        line->box->setCurrentIndex(fuse_value_index);
        connect(line->box, SIGNAL(currentIndexChanged(int)), this, SLOT(changed(int)));
        m_fuse_layout->addRow(line->label, line->box);
        m_fuses.push_back(line);
    }

    rememberAct->setEnabled(true);
    writeAct->setEnabled(true);
    m_changed = false;
}

// void update_fuse_data(), avr232client.cpp
void FuseWidget::rememberFuses()
{
    for(quint8 i = 0; i < m_fuses.size(); ++i)
    {
        QString s = m_fuses[i]->box->currentText();
        Q_ASSERT(s.left(2) == "0b");

        int value = 0;
        for (int j = 2; j < s.length(); ++j)
            value = (value << 1) | (s[j] == '1');

        chip_definition::set_fuse_value(m_fuse_data.begin(), m_fuse_data.end(), m_fuses[i]->fuse, value);
    }
    m_changed = false;
    emit status(tr("Fuses had been remembered"));
}

void FuseWidget::changed(int /*index*/)
{
    m_changed = true;
}
