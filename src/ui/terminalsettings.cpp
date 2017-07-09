/**********************************************
**    This file is part of Lorris
**    http://tasssadar.github.com/Lorris/
**
**    See README and COPYING
***********************************************/

#include <QColorDialog>

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
    ui->backBox->setChecked(set.chars[SET_BACKSPACE]);
    ui->formBox->setChecked(set.chars[SET_FORMFEED]);
    ui->nullBox->setChecked(set.chars[SET_IGNORE_NULL]);
    ui->enterSendBox->setCurrentIndex(set.chars[SET_ENTER_SEND]);
    ui->escapeBox->setChecked(set.chars[SET_HANDLE_ESCAPE]);

    ui->widthBox->setValue(set.tabReplace);
    ui->fontBox->setCurrentFont(set.font);
    ui->sizeBox->setEditText(QString::number(set.font.pointSize()));

    setBtnColor(ui->backBtn, set.colors[COLOR_BG], COLOR_BG);
    setBtnColor(ui->textBtn, set.colors[COLOR_TEXT], COLOR_TEXT);
    setBtnColor(ui->cursorBtn, set.colors[COLOR_CURSOR], COLOR_CURSOR);
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
    set.chars[SET_BACKSPACE] = ui->backBox->isChecked();
    set.chars[SET_FORMFEED] = ui->formBox->isChecked();
    set.chars[SET_IGNORE_NULL] = ui->nullBox->isChecked();
    set.chars[SET_ENTER_SEND] = ui->enterSendBox->currentIndex();
    set.chars[SET_HANDLE_ESCAPE] = ui->escapeBox->isChecked();

    set.tabReplace = ui->widthBox->value();
    set.font = ui->fontBox->currentFont();

    int size = ui->sizeBox->currentText().toUInt();
    set.font.setPointSize(size ? size : 9);

    for(int i = 0; i < COLOR_MAX; ++i)
        set.colors[i] = m_colors[i];

    return set;
}

void TerminalSettings::on_buttonBox_clicked(QAbstractButton *button)
{
    if(ui->buttonBox->buttonRole(button) == QDialogButtonBox::ApplyRole)
        emit applySettings(getSettings());
}

void TerminalSettings::on_backBtn_clicked()
{
    QColor clr = QColorDialog::getColor(m_colors[COLOR_BG], this);
    if(!clr.isValid())
        return;
    setBtnColor(ui->backBtn, clr, COLOR_BG);
}

void TerminalSettings::on_textBtn_clicked()
{
    QColor clr = QColorDialog::getColor(m_colors[COLOR_TEXT], this);
    if(!clr.isValid())
        return;
    setBtnColor(ui->textBtn, clr, COLOR_TEXT);
}

void TerminalSettings::on_cursorBtn_clicked()
{
    QColor clr = QColorDialog::getColor(m_colors[COLOR_CURSOR], this);
    if(!clr.isValid())
        return;
    setBtnColor(ui->cursorBtn, clr, COLOR_CURSOR);
}

void TerminalSettings::setBtnColor(QPushButton *btn, QColor clr, int idx)
{
    m_colors[idx] = clr;

    QPixmap map(btn->iconSize());
    map.fill(clr);
    btn->setIcon(QIcon(map));
}
