/**********************************************
**    This file is part of Lorris
**    http://tasssadar.github.com/Lorris/
**
**    See README and COPYING
***********************************************/

#ifndef UPDATER_H
#define UPDATER_H

#include <QObject>
#include <QDialog>

#include "ui_updatecheck.h"

class Updater : public QObject
{
    Q_OBJECT
public:
    static bool doUpdate(bool autoCheck);

private:
    static bool checkForUpdate(bool autoCheck);
    static bool askForUpdate();
    static bool copyUpdater();
};

class UpdaterDialog : public QDialog, private Ui::UpdateCheck
{
    Q_OBJECT
public:
    UpdaterDialog(QWidget *paren = 0);
    ~UpdaterDialog();

private slots:
    void on_noAskBox_clicked(bool checked);
    void on_noCheckBox_clicked(bool checked);

private:
    Ui::UpdateCheck *ui;
};

#endif // UPDATER_H
