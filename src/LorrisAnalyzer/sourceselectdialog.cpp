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
    ui->importEdit->setText(sConfig.get(CFG_STRING_ANALYZER_IMPORT));
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

void SourceSelectDialog::on_contButton_clicked()
{
    if(ui->loadRadio->isChecked() && !ui->structBox->isChecked() &&
       !ui->dataBox->isChecked() && !ui->widgetBox->isChecked())
    {
        return Utils::showErrorBox(tr("You have to select at least one thing to load."), this);
    }
    accept();
}

qint8 SourceSelectDialog::get()
{
    if(exec() == QDialog::Rejected)
        return -1;

    if(ui->newRadio->isChecked())
        return 0;
    if(ui->loadRadio->isChecked())
        return 1;
    return 2;
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
    if(ui->loadRadio->isChecked())
        return ui->fileEdit->text();
    else
        return ui->importEdit->text();
}

void SourceSelectDialog::on_newRadio_toggled(bool toggle)
{
    if(toggle)
    {
        ui->contButton->setEnabled(true);
        ui->stack->setCurrentIndex(0);
    }
}

void SourceSelectDialog::on_loadRadio_toggled(bool toggle)
{
    if(toggle)
    {
        ui->contButton->setEnabled(!ui->fileEdit->text().isEmpty());
        ui->stack->setCurrentIndex(1);
    }
}

void SourceSelectDialog::on_binRadio_toggled(bool toggle)
{
    if(toggle)
    {
        ui->contButton->setEnabled(!ui->importEdit->text().isEmpty());
        ui->stack->setCurrentIndex(2);
    }
}

void SourceSelectDialog::on_loadBrowse_clicked()
{
    static const QString filters = tr("Lorris data files (*.ldta *.cldta)");
    QString filename = QFileDialog::getOpenFileName(NULL, QObject::tr("Load Data"),
                                                    sConfig.get(CFG_STRING_ANALYZER_FOLDER),
                                                    filters);
    if(filename.isEmpty())
        return;

    ui->fileEdit->setText(filename);
    ui->contButton->setEnabled(true);
    sConfig.set(CFG_STRING_ANALYZER_FOLDER, filename);
}

void SourceSelectDialog::on_importBrowse_clicked()
{
    static const QString filters = tr("Any file (*.*)");
    QString filename = QFileDialog::getOpenFileName(NULL, QObject::tr("Import binary data"),
                                                    sConfig.get(CFG_STRING_ANALYZER_IMPORT),
                                                    filters);
    if(filename.isEmpty())
        return;

    ui->importEdit->setText(filename);
    ui->contButton->setEnabled(true);
    sConfig.set(CFG_STRING_ANALYZER_IMPORT, filename);
}
