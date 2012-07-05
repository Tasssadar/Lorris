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
    void on_backBtn_clicked();
    void on_textBtn_clicked();
    void on_cursorBtn_clicked();

private:
    void setBtnColor(QPushButton *btn, QColor clr, int idx);

    Ui::TerminalSettings *ui;
    QColor m_colors[COLOR_MAX];
};

#endif // TERMINALSETTINGS_H
