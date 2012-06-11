/**********************************************
**    This file is part of Lorris
**    http://tasssadar.github.com/Lorris/
**
**    See README and COPYING
***********************************************/

#include "rangeselectdialog.h"
#include "ui_rangeselectdialog.h"

RangeSelectDialog::RangeSelectDialog(int val_min, int val_max, int max, int min, QWidget *parent) : QDialog(parent), ui(new Ui::RangeSelectDialog)
{
    ui->setupUi(this);

    ui->maxBox->setRange(val_min, max);
    ui->minBox->setRange(min, val_max);
    ui->maxBox->setValue(val_max);
    ui->minBox->setValue(val_min);

    connect(ui->maxBox, SIGNAL(valueChanged(int)), this, SLOT(maxChanged(int)));
    connect(ui->minBox, SIGNAL(valueChanged(int)), this, SLOT(minChanged(int)));
    connect(ui->buttonBox, SIGNAL(clicked(QAbstractButton*)), this, SLOT(boxClicked(QAbstractButton*)));

    m_minRes = val_min;
    m_maxRes = val_max;
    m_res = false;

    setFixedSize(size());
}

RangeSelectDialog::~RangeSelectDialog()
{
    delete ui;
}

void RangeSelectDialog::maxChanged(int value)
{
    m_maxRes = value;
    ui->minBox->setMaximum(value);
}

void RangeSelectDialog::minChanged(int value)
{
    m_minRes = value;
    ui->maxBox->setMinimum(value);
}

void RangeSelectDialog::boxClicked(QAbstractButton *b)
{
    if(ui->buttonBox->buttonRole(b) == QDialogButtonBox::AcceptRole)
        m_res = true;
    close();
}
