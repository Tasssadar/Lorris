/**********************************************
**    This file is part of Lorris
**    http://tasssadar.github.com/Lorris/
**
**    See README and COPYING
***********************************************/

#ifndef TERMINALSETTINGS_H
#define TERMINALSETTINGS_H

#include <QDialog>

#include "terminal.h"
#include "ui_terminalsettings.h"

class TerminalSettings : public QDialog, private Ui::TerminalSettings
{
    Q_OBJECT

Q_SIGNALS:
    void applySettings(const terminal_settings& set);
    
public:
    TerminalSettings(const terminal_settings& set, QWidget *parent = 0);
    ~TerminalSettings();

    terminal_settings getSettings();
    
private slots:
    void on_buttonBox_clicked(QAbstractButton *button);

private:
    Ui::TerminalSettings *ui;
};

#endif // TERMINALSETTINGS_H
