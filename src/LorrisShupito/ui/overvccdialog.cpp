/**********************************************
**    This file is part of Lorris
**    http://tasssadar.github.com/Lorris/
**
**    See README and COPYING
***********************************************/

#include <QAccessible>

#include "overvccdialog.h"
#include "../../misc/utils.h"
#include "ui_overvccdialog.h"

OverVccDialog::OverVccDialog(bool autoclose, QWidget *parent) :
    QDialog(parent),
    ui(new Ui::OverVccDialog)
{
    ui->setupUi(this);

    QAccessible::updateAccessibility(this, 0, QAccessible::Alert);
    setFixedSize(size());

    m_autoclose = autoclose;
}

OverVccDialog::~OverVccDialog()
{
    delete ui;
}

void OverVccDialog::showEvent(QShowEvent *event)
{
    QDialog::showEvent(event);

    Utils::playErrorSound();
}

void OverVccDialog::closeEvent(QCloseEvent *event)
{
    QDialog::closeEvent(event);
    if(m_autoclose)
        deleteLater();
}
