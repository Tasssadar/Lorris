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

#include <QFileDialog>
#include <QMessageBox>

#include "sourceselectdialog.h"
#include "ui_sourceselectdialog.h"
#include "analyzerdatastorage.h"

SourceSelectDialog::SourceSelectDialog(QWidget *parent) :
    QDialog(parent),ui(new Ui::SourceSelectDialog)
{
    ui->setupUi(this);

    connect(ui->contButton, SIGNAL(clicked()), this, SLOT(contButton()));
    connect(ui->browseButton, SIGNAL(clicked()), this, SLOT(browse()));
    connect(ui->loadRadio, SIGNAL(toggled(bool)), this, SLOT(loadRadioToggled(bool)));
}

SourceSelectDialog::~SourceSelectDialog()
{
    delete ui;
}

void SourceSelectDialog::DisableNew()
{
    ui->newRadio->setEnabled(false);
    ui->loadRadio->setChecked(true);
}

void SourceSelectDialog::contButton()
{
    if(ui->newRadio->isChecked())
    {
        accept();
        return;
    }

    if(!ui->structBox->isChecked() && !ui->dataBox->isChecked() && !ui->widgetBox->isChecked())
    {
        QMessageBox *box = new QMessageBox();
        box->setWindowTitle(QObject::tr("Error!"));
        box->setText(QObject::tr("You have to select at least one thing to load."));
        box->setIcon(QMessageBox::Critical);
        box->exec();
        delete box;
        return;
    }

    accept();
}

qint8 SourceSelectDialog::get()
{
    if(exec() == QDialog::Rejected)
        return -1;

    return ui->newRadio->isChecked();
}

quint8 SourceSelectDialog::getDataMask()
{
    quint8 res = 0;

    if(ui->structBox->isChecked())
        res |= STORAGE_STRUCTURE;

    if(ui->dataBox->isChecked())
        res |= STORAGE_DATA;

    if(ui->widgetBox->isChecked())
        res |= STORAGE_WIDGETS;
    return res;
}

QString SourceSelectDialog::getFileName()
{
    return ui->fileEdit->text();
}

void SourceSelectDialog::browse()
{
    QString filters = QObject::tr("Compressed Lorris data file(*.cldta);;Lorris data file (*.ldta)");
    QString filename = QFileDialog::getOpenFileName(NULL, QObject::tr("Import Data"),
                                                    sConfig.get(CFG_STRING_ANALYZER_FOLDER),
                                                    filters);

    ui->contButton->setEnabled(filename.length() != 0);

    if(filename.length() == 0)
        return;

    ui->fileEdit->setText(filename);
    sConfig.set(CFG_STRING_ANALYZER_FOLDER, filename.left(filename.lastIndexOf(QRegExp("[\\/]"))));
}

void SourceSelectDialog::loadRadioToggled(bool toggle)
{
    bool enableCont = !toggle;
    if(toggle && ui->fileEdit->text().length() != 0)
        enableCont = true;

    ui->contButton->setEnabled(enableCont);
}
