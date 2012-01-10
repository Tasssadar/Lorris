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

    QPushButton *cont = findChild<QPushButton*>("contButton");
    connect(cont, SIGNAL(clicked()), this, SLOT(contButton()));

    QPushButton *browseButton = findChild<QPushButton*>("browseButton");
    connect(browseButton, SIGNAL(clicked()), this, SLOT(browse()));

    QRadioButton *loadRadio = findChild<QRadioButton*>("loadRadio");
    connect(loadRadio, SIGNAL(toggled(bool)), this, SLOT(loadRadioToggled(bool)));
}

SourceSelectDialog::~SourceSelectDialog()
{
    delete ui;
}

void SourceSelectDialog::DisableNew()
{
    QRadioButton *newRadio = findChild<QRadioButton*>("newRadio");
    newRadio->setEnabled(false);

    QRadioButton *loadRadio = findChild<QRadioButton*>("loadRadio");
    loadRadio->setChecked(true);
}

void SourceSelectDialog::contButton()
{
    QRadioButton *newRadio = findChild<QRadioButton*>("newRadio");
    if(newRadio->isChecked())
    {
        accept();
        return;
    }
    QCheckBox *structBox = findChild<QCheckBox*>("structBox");
    QCheckBox *dataBox = findChild<QCheckBox*>("dataBox");
    QCheckBox *widgetBox = findChild<QCheckBox*>("widgetBox");

    if(!structBox->isChecked() && !dataBox->isChecked() && !widgetBox->isChecked())
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

    QRadioButton *newRadio = findChild<QRadioButton*>("newRadio");
    return newRadio->isChecked();
}

quint8 SourceSelectDialog::getDataMask()
{
    quint8 res = 0;

    QCheckBox *structBox = findChild<QCheckBox*>("structBox");
    if(structBox->isChecked())
        res |= STORAGE_STRUCTURE;

    QCheckBox *dataBox = findChild<QCheckBox*>("dataBox");
    if(dataBox->isChecked())
        res |= STORAGE_DATA;

    QCheckBox *widgetBox = findChild<QCheckBox*>("widgetBox");
    if(widgetBox->isChecked())
        res |= STORAGE_WIDGETS;
    return res;
}

QString SourceSelectDialog::getFileName()
{
    QLineEdit *file = findChild<QLineEdit*>("fileEdit");
    return file->text();
}

void SourceSelectDialog::browse()
{
    QString filters = QObject::tr("Lorris data file (*.ldta)");
    QString filename = QFileDialog::getOpenFileName(NULL, QObject::tr("Import Data"), "", filters);

    QPushButton *cont = findChild<QPushButton*>("contButton");
    cont->setEnabled(filename.length() != 0);

    if(filename.length() == 0)
        return;

    QLineEdit *file = findChild<QLineEdit*>("fileEdit");
    file->setText(filename);

}

void SourceSelectDialog::loadRadioToggled(bool toggle)
{
    bool enableCont = !toggle;
    if(toggle)
    {
        QLineEdit *file = findChild<QLineEdit*>("fileEdit");
        if(file->text().length() != 0)
            enableCont = true;
    }

    QPushButton *cont = findChild<QPushButton*>("contButton");
    cont->setEnabled(enableCont);
}
