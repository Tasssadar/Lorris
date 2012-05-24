/**********************************************
**    This file is part of Lorris
**    http://tasssadar.github.com/Lorris/
**
**    See README and COPYING
***********************************************/

#include <QLabel>
#include <QVBoxLayout>
#include <QPushButton>
#include <QHBoxLayout>
#include <QComboBox>
#include <QFormLayout>
#include <QMenu>
#include <QContextMenuEvent>

#include "../common.h"
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

    writeAct = contextMenu->addAction(tr("Write fuses"));
    writeAct->setEnabled(false);

    connect(read,         SIGNAL(triggered()), this, SIGNAL(readFuses()));
    connect(writeAct,     SIGNAL(triggered()), this, SIGNAL(writeFuses()));
    connect(rememberAct,  SIGNAL(triggered()), this, SLOT(rememberFuses()));
    connect(readFusesBtn, SIGNAL(clicked()),   this, SIGNAL(readFuses()));

    m_changed = false;

    fuse_desc::parse_default_fuses(m_fusedesc);
}

FuseWidget::~FuseWidget()
{
    clear();
}

void FuseWidget::clear(bool addButton)
{
    for(quint16 i = 0; i < m_fuses.size(); ++i)
    {
        delete m_fuses[i]->label;
        delete m_fuses[i]->box;
        delete m_fuses[i];
    }
    m_fuses.clear();
    m_fuse_data.clear();

    if(addButton && !readFusesBtn)
    {
        readFusesBtn = new QPushButton(tr("Read fuses"), this);
        m_layout->insertWidget(2, readFusesBtn);
        connect(readFusesBtn, SIGNAL(clicked()), this, SIGNAL(readFuses()));
    }
}

void FuseWidget::contextMenuEvent( QContextMenuEvent * event )
{
    contextMenu->exec(event->globalPos());
}

// void update_fuse_window(), avr232client.cpp
void FuseWidget::setFuses(chip_definition &chip)
{
    m_chip = chip;

    chip_definition::memorydef *memdef = chip.getMemDef("fuses");
    if(!memdef || chip.getFuses().size() == 0)
        return;

    std::vector<chip_definition::fuse>& fuses = chip.getFuses();

    if(readFusesBtn)
    {
        delete readFusesBtn;
        readFusesBtn = NULL;
    }

    disconnect(this, SLOT(changed(int)));

    for(quint16 i = 0; i < fuses.size(); ++i)
    {
        chip_definition::fuse& f = fuses[i];

        fuse_line *line = new fuse_line();
        line->fuse = f;
        line->label = new QLabel(f.name, this);
        line->box = new QComboBox(this);
        line->box->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Preferred);

        translateFuseName(line);

        int fuse_value = chip_definition::get_fuse_value(m_fuse_data.begin(), m_fuse_data.end(), f);
        int fuse_value_index = -1;
        if(!f.values.empty())
        {
            for(std::size_t j = 0; j < f.values.size(); ++j)
            {
                if(fuse_value == f.values[j])
                    fuse_value_index = j;
                addFuseOpt(line, Utils::toBinary(f.bits.size(), f.values[j]));
            }
            if(fuse_value_index == -1)
            {
                addFuseOpt(line, Utils::toBinary(f.bits.size(), fuse_value));
                fuse_value_index = f.values.size();
            }
        }
        else
        {
            for (std::size_t j = 0; j < (1<<f.bits.size()); ++j)
            {
                if (fuse_value == (int)j)
                    fuse_value_index = j;

                addFuseOpt(line, Utils::toBinary(f.bits.size(), j));
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
        int idx = m_fuses[i]->box->currentIndex();
        QString s = m_fuses[i]->box->itemData(idx).toString();
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

void FuseWidget::translateFuseName(fuse_line *line)
{
    fuse_desc *desc = fuse_desc::findDesc(line->fuse.name, m_chip.getSign(), m_fusedesc);
    if(desc)
    {
        line->label->setToolTip(desc->getDesc());
        line->box->setToolTip(desc->getDesc());
    }
}

void FuseWidget::addFuseOpt(fuse_line *line, const QString &bin)
{
    fuse_desc *desc = fuse_desc::findDesc(line->fuse.name, m_chip.getSign(), m_fusedesc);
    QString text = desc ? desc->getOptDesc(bin) : "";

    line->box->addItem(text.isEmpty() ? bin : text, QVariant(bin));
}
