/**********************************************
**    This file is part of Lorris
**    http://tasssadar.github.com/Lorris/
**
**    See README and COPYING
***********************************************/

#include "terminalsettings.h"
#include "ui_terminalsettings.h"

TerminalSettings::TerminalSettings(const terminal_settings &set, QWidget *parent) :
    QDialog(parent),
    ui(new Ui::TerminalSettings)
{
    ui->setupUi(this);
    ui->sizeBox->setValidator(new QIntValidator(0, 1000, this));

    // Load settings
    ui->tabBox->setChecked(set.chars[SET_REPLACE_TAB]);
    ui->alarmBox->setChecked(set.chars[SET_ALARM]);
    ui->newBox->setCurrentIndex(set.chars[SET_NEWLINE]);
    ui->returnBox->setCurrentIndex(set.chars[SET_RETURN]);

    ui->widthBox->setValue(set.tabReplace);
    ui->fontBox->setCurrentFont(set.font);
    ui->sizeBox->setEditText(QString::number(set.font.pointSize()));
}

TerminalSettings::~TerminalSettings()
{
    delete ui;
}

terminal_settings TerminalSettings::getSettings()
{
    terminal_settings set;
    set.chars[SET_REPLACE_TAB] = ui->tabBox->isChecked();
    set.chars[SET_ALARM] = ui->alarmBox->isChecked();
    set.chars[SET_NEWLINE] = ui->newBox->currentIndex();
    set.chars[SET_RETURN] = ui->returnBox->currentIndex();

    set.tabReplace = ui->widthBox->value();
    set.font = ui->fontBox->currentFont();

    int size = ui->sizeBox->currentText().toUInt();
    set.font.setPointSize(size ? size : 9);

    return set;
}

void TerminalSettings::on_buttonBox_clicked(QAbstractButton *button)
{
    if(ui->buttonBox->buttonRole(button) == QDialogButtonBox::ApplyRole)
        emit applySettings(getSettings());
}
