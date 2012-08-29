/**********************************************
**    This file is part of Lorris
**    http://tasssadar.github.com/Lorris/
**
**    See README and COPYING
***********************************************/

#include <QVBoxLayout>
#include <QGridLayout>
#include <QDialogButtonBox>
#include <QLabel>
#include <QPushButton>
#include <QSignalMapper>
#include <QColorDialog>

#include "colordialog.h"

ColorDialog::ColorDialog(QList<QColor> colors, QStringList labels, QWidget *parent) :
    QDialog(parent)
{
    m_colors = colors;

    QVBoxLayout *l = new QVBoxLayout(this);
    QGridLayout *g = new QGridLayout;

    QSignalMapper *map = new QSignalMapper(this);
    for(int i = 0; i < labels.size() && i < colors.size(); ++i)
    {
        QLabel *label = new QLabel(labels[i], this);
        QPushButton *btn = new QPushButton(this);
        btn->setIconSize(QSize(32, 16));
        btn->setSizePolicy(QSizePolicy::Fixed, QSizePolicy::Fixed);
        m_buttons.push_back(btn);

        setButtonColor(i, colors[i]);

        g->addWidget(label, i, 0);
        g->addWidget(btn, i, 1);

        map->setMapping(btn, i);
        connect(btn, SIGNAL(clicked()), map, SLOT(map()));
    }

    QDialogButtonBox *buttons = new QDialogButtonBox(this);
    buttons->setStandardButtons(QDialogButtonBox::Ok | QDialogButtonBox::Cancel);
    buttons->setOrientation(Qt::Horizontal);

    l->addLayout(g);
    l->addWidget(buttons);

    connect(buttons, SIGNAL(accepted()),  SLOT(accept()));
    connect(buttons, SIGNAL(rejected()),  SLOT(reject()));
    connect(map,     SIGNAL(mapped(int)), SLOT(btnClicked(int)));
}

bool ColorDialog::getColors(QList<QColor>& colors, QStringList labels, QWidget *parent)
{
    if(colors.size() != labels.size())
        return false;

    ColorDialog d(colors, labels, parent);
    if(d.exec() == QDialog::Rejected)
        return false;

    colors = d.m_colors;
    return true;
}

void ColorDialog::setButtonColor(int idx, const QColor& clr)
{
    if((size_t)idx >= m_buttons.size())
        return;

    QPixmap map(50, 25);
    map.fill(clr);
    m_colors[idx] = clr;
    m_buttons[idx]->setIcon(QIcon(map));
}

void ColorDialog::btnClicked(int idx)
{
    QColor clr = QColorDialog::getColor(m_colors[idx], this);
    if(!clr.isValid())
        return;

    setButtonColor(idx, clr);
}

