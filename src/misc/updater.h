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

class Updater
{
public:
    static bool doUpdate(bool autoCheck);
    static bool startUpdater();

private:
    static bool checkForUpdate(bool autoCheck);
    static bool askForUpdate();
    static bool copyUpdater();
    static void showNotification();
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

class UpdateBtnHandler : public QObject
{
    Q_OBJECT

    friend class Updater;
protected:
    UpdateBtnHandler(QObject *parent);

protected slots:
    void updateBtn();
};

#endif // UPDATER_H
