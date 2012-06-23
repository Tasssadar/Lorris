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
#include <algorithm>

#include "../common.h"
#include "fusewidget.h"
#include "../shared/defmgr.h"

//#define DEBUG_FUSES "avr:1e9403" // Atmega16

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

    contextMenu->addSeparator();

    translateFuseAct = contextMenu->addAction(tr("Translate fuse values"));
    hideReservedAct = contextMenu->addAction(tr("Hide reserved values"));
    translateFuseAct->setCheckable(true);
    hideReservedAct->setCheckable(true);

    connect(read,             SIGNAL(triggered()),      SIGNAL(readFuses()));
    connect(writeAct,         SIGNAL(triggered()),      SIGNAL(writeFuses()));
    connect(rememberAct,      SIGNAL(triggered()),      SLOT(rememberFuses()));
    connect(readFusesBtn,     SIGNAL(clicked()),        SIGNAL(readFuses()));
    connect(translateFuseAct, SIGNAL(triggered(bool)),  SLOT(translateFuses(bool)));
    connect(hideReservedAct,  SIGNAL(triggered(bool)),  SLOT(hideReserved(bool)));

    m_changed = false;

    translateFuses(sConfig.get(CFG_BOOL_SHUPITO_TRANSLATE_FUSES));
    hideReserved(sConfig.get(CFG_BOOL_SHUPITO_HIDE_RESERVED));

#ifdef DEBUG_FUSES
    chip_definition cd = sDefMgr.findChipdef(DEBUG_FUSES);
    setFuses(cd);
#endif
}

FuseWidget::~FuseWidget()
{
    clear();
}

void FuseWidget::clear(bool addButton, bool widgetsOnly)
{
    for(quint16 i = 0; i < m_fuses.size(); ++i)
    {
        delete m_fuses[i]->label;
        delete m_fuses[i]->box;
        delete m_fuses[i];
    }
    m_fuses.clear();

    if(!widgetsOnly)
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

static bool compareBoxVals(const std::pair<QString, QVariant>& first, const std::pair<QString, QVariant>& second)
{
    return first.first < second.first;
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

    bool hideRes = hideReservedAct->isChecked();

    for(quint16 i = 0; i < fuses.size(); ++i)
    {
        chip_definition::fuse& f = fuses[i];

        fuse_line *line = new fuse_line();
        line->fuse = f;
        line->label = new QLabel(f.name, this);
        line->box = new QComboBox(this);
        line->box->setSizePolicy(QSizePolicy::Minimum, QSizePolicy::Preferred);

        if(translateFuseAct->isChecked())
            translateFuseName(line);

        int fuse_value = chip_definition::get_fuse_value(m_fuse_data.begin(), m_fuse_data.end(), f);
        int fuse_value_index = -1;
        bool sortVals = false;
        std::vector<std::pair<QString, QVariant> > list;

        if(!f.values.empty())
        {
            for(std::size_t j = 0; j < f.values.size(); ++j)
            {
                if(addFuseOpt(line, Utils::toBinary(f.bits.size(), f.values[j]), list))
                    sortVals = true;

                if(fuse_value == f.values[j])
                    fuse_value_index = list.size()-1;
                else if(hideRes && list.back().first == "<reserved>")
                    list.pop_back();
            }
            if(fuse_value_index == -1)
            {
                if(addFuseOpt(line, Utils::toBinary(f.bits.size(), fuse_value), list))
                    sortVals = true;
                fuse_value_index = list.size()-1;
            }
        }
        else
        {
            for (std::size_t j = 0; j < std::size_t(1<<f.bits.size()); ++j)
            {
                if(addFuseOpt(line, Utils::toBinary(f.bits.size(), j), list))
                    sortVals = true;

                if (fuse_value == (int)j)
                    fuse_value_index = list.size()-1;
                else if(hideRes && list.back().first == "<reserved>")
                    list.pop_back();
            }
        }

        QVariant curIdx = list[fuse_value_index].second;
        if(sortVals)
            std::sort(list.begin(), list.end(), compareBoxVals);

        for(quint32 i = 0; i < list.size(); ++i)
        {
            std::pair<QString, QVariant>& it = list[i];
            line->box->addItem(it.first, it.second);

            if(it.second == curIdx)
                line->box->setCurrentIndex(i);
        }

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
    fuse_desc *desc = sDefMgr.findFuse_desc(line->fuse.name, m_chip.getSign());
    if(desc)
    {
        line->label->setToolTip(desc->getDesc());
        line->box->setToolTip(desc->getDesc());
    }
}

bool FuseWidget::addFuseOpt(fuse_line *line, const QString &bin, std::vector<std::pair<QString, QVariant> >& list)
{
    fuse_desc *desc = NULL;

    if(translateFuseAct->isChecked())
        desc = sDefMgr.findFuse_desc(line->fuse.name, m_chip.getSign());

    QString text = desc ? desc->getOptDesc(bin) : "";
    list.push_back(std::pair<QString, QVariant>(text.isEmpty() ? bin : text, QVariant(bin)));
    return desc != NULL;
}

void FuseWidget::translateFuses(bool checked)
{
    translateFuseAct->setChecked(checked);
    sConfig.set(CFG_BOOL_SHUPITO_TRANSLATE_FUSES, checked);

    clear(false, true);
    setFuses(m_chip);

    hideReservedAct->setEnabled(checked);
}

void FuseWidget::hideReserved(bool checked)
{
    hideReservedAct->setChecked(checked);
    sConfig.set(CFG_BOOL_SHUPITO_HIDE_RESERVED, checked);

    clear(false, true);
    setFuses(m_chip);
}
