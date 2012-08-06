/**********************************************
**    This file is part of Lorris
**    http://tasssadar.github.com/Lorris/
**
**    See README and COPYING
***********************************************/

#ifndef SETTINGSDIALOG_H
#define SETTINGSDIALOG_H

#include <QDialog>

#include "ui_settingsdialog.h"

extern QLocale::Language langs[];

class SettingsDialog : public QDialog
{
    Q_OBJECT
Q_SIGNALS:
    void closeLorris();

public:
    explicit SettingsDialog(QWidget *parent = 0);
    ~SettingsDialog();
    
private slots:
    void on_buttonBox_clicked(QAbstractButton *btn);
    void on_updateBtn_clicked();
    void on_resetBtn_clicked();

private:
    void loadSettings();
    void applySettings();
    void setPortable(bool portable);

    Ui::SettingsDialog *ui;
};

#endif // SETTINGSDIALOG_H
