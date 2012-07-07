/**********************************************
**    This file is part of Lorris
**    http://tasssadar.github.com/Lorris/
**
**    See README and COPYING
***********************************************/

#include <QDesktopServices>
#include <QFile>
#include <QDir>
#include <QMessageBox>

#include "settingsdialog.h"
#include "../misc/config.h"
#include "../misc/utils.h"
#include "../revision.h"
#include "../misc/updater.h"

QLocale::Language langs[] = { QLocale::system().language(), QLocale::English, QLocale::Czech };

SettingsDialog::SettingsDialog(QWidget *parent) :
    QDialog(parent),
    ui(new Ui::SettingsDialog)
{
    ui->setupUi(this);
    ui->versionLabel->setText(tr("%1 - git revision %2").arg(VERSION).arg(REVISION));
    loadSettings();
}

SettingsDialog::~SettingsDialog()
{
    delete ui;
}

void SettingsDialog::loadSettings()
{
    ui->langBox->clear();
    for(quint8 i = 0; i < 3; ++i)
    {
        QString langName = QLocale::languageToString(langs[i]);
        if(i == 0)
            langName.prepend(tr("Same as OS - "));

        ui->langBox->addItem(langName);
    }
    ui->langBox->setCurrentIndex(sConfig.get(CFG_QUINT32_LANGUAGE));

    QFont fnt = Utils::getFontFromString(sConfig.get(CFG_STRING_APP_FONT));
    ui->fontBox->setCurrentFont(fnt);
    ui->sizeBox->setValue(fnt.pointSize());

    ui->portableBox->setChecked(sConfig.get(CFG_BOOL_PORTABLE));
}

void SettingsDialog::on_buttonBox_clicked(QAbstractButton *btn)
{
    QDialogButtonBox::ButtonRole role = ui->buttonBox->buttonRole(btn);
    if(role == QDialogButtonBox::ApplyRole || role == QDialogButtonBox::AcceptRole)
        applySettings();
}

void SettingsDialog::applySettings()
{
    sConfig.set(CFG_QUINT32_LANGUAGE, ui->langBox->currentIndex());

    QFont fnt = ui->fontBox->currentFont();
    fnt.setPointSize(ui->sizeBox->value());
    QApplication::setFont(fnt);
    sConfig.set(CFG_STRING_APP_FONT, Utils::getFontSaveString(fnt));

    if(sConfig.get(CFG_BOOL_PORTABLE) ^ ui->portableBox->isChecked())
    {
        setPortable(ui->portableBox->isChecked());
        sConfig.set(CFG_BOOL_PORTABLE, ui->portableBox->isChecked());
    }
}

void SettingsDialog::setPortable(bool portable)
{
    // FIXME: QString::swap was introduced in Qt 4.8, I dont wanna use it yet
    QString from, to;
    if(portable)
    {
        from = QDesktopServices::storageLocation(QDesktopServices::DataLocation) + "/";
        to = "./data/";
    }
    else
    {
        from = "./data/";
        to = QDesktopServices::storageLocation(QDesktopServices::DataLocation) + "/";
    }

    QDir dir(from);
    QStringList list = dir.entryList((QStringList() << "*.ini" << "*.cldta"));

    if(!QDir::current().mkpath(to))
        goto fail;

    for(int i = 0; i < list.size(); ++i)
    {
        QFile::remove(to + list[i]);
        if(!QFile::copy(from + list[i], to + list[i]))
            goto fail;
    }

    sConfig.closeSettings();

    for(int i = 0; i < list.size(); ++i)
        QFile::remove(from + list[i]);
    dir.rmdir(dir.absolutePath());

    sConfig.openSettings();

    return;
fail:
    for(int i = 0; i < list.size(); ++i)
        QFile::remove(to + list[i]);

    Utils::ThrowException(tr("Unable to copy settings files!"));
}

void SettingsDialog::on_updateBtn_clicked()
{
#ifdef Q_OS_WIN
    ui->updateBtn->setText(tr("Checking for update..."), 0);
    if(Updater::doUpdate(false))
        emit closeLorris();
    else
        ui->updateBtn->setText(tr("No update available"));
#else
    Utils::ThrowException(tr("Update feature is available on Windows only, you have to rebuild Lorris by yourself.\n"
                             "<a href='http://tasssadar.github.com/Lorris'>http://tasssadar.github.com/Lorris</a>"));
#endif
}

void SettingsDialog::on_resetBtn_clicked()
{
    QMessageBox box(QMessageBox::Question, tr("Reset settings"), tr("Do you really wanna to reset all settings to their "
                   "default values? This will not affect already loaded settings."), (QMessageBox::Yes | QMessageBox::No));
    if(box.exec() == QMessageBox::No)
        return;

    sConfig.resetToDefault();
    loadSettings();
}
