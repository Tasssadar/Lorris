#include "reloaddialog.h"
#include "ui_reloaddialog.h"

ReloadDialog::ReloadDialog(const QString& filename, QWidget *parent) :
    QDialog(parent),
    ui(new Ui::ReloadDialog)
{
    ui->setupUi(this);

    ui->text->setText(tr("File %1 was changed. Reload?").arg(filename));

    ui->reloadBtn->setIcon(style()->standardIcon(QStyle::SP_DialogOkButton));
    ui->cancleBtn->setIcon(style()->standardIcon(QStyle::SP_DialogCancelButton));
}

ReloadDialog::~ReloadDialog()
{
    delete ui;
}

quint8 ReloadDialog::getReloadState()
{
    if(ui->radioAsk->isChecked())
        return 0;

    if(ui->radioReload->isChecked())
        return 1;

    if(ui->radioIgnore->isChecked())
        return 2;

    return 0;
}
