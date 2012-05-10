/**********************************************
**    This file is part of Lorris
**    http://tasssadar.github.com/Lorris/
**
**    See README and COPYING
***********************************************/

#include <QFileDialog>
#include <QMessageBox>

#include "sourceselectdialog.h"
#include "ui_sourceselectdialog.h"
#include "storage.h"

SourceSelectDialog::SourceSelectDialog(QWidget *parent) :
    QDialog(parent),ui(new Ui::SourceSelectDialog)
{
    ui->setupUi(this);

    ui->fileEdit->setText(sConfig.get(CFG_STRING_ANALYZER_FOLDER));

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
        return Utils::ThrowException(tr("You have to select at least one thing to load."), this);

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
    QString filters = QObject::tr("Lorris data files (*.ldta *.cldta)");
    QString filename = QFileDialog::getOpenFileName(NULL, QObject::tr("Import Data"),
                                                    sConfig.get(CFG_STRING_ANALYZER_FOLDER),
                                                    filters);
    if(filename.isEmpty())
        return;

    ui->fileEdit->setText(filename);
    ui->contButton->setEnabled(true);
    sConfig.set(CFG_STRING_ANALYZER_FOLDER, filename);
}

void SourceSelectDialog::loadRadioToggled(bool toggle)
{
    ui->contButton->setEnabled(!toggle || !ui->fileEdit->text().isEmpty());
}
