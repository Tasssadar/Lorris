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
#include <QFuture>
#include <QFutureWatcher>
#include <QNetworkRequest>

#include "ui_updatecheck.h"

class Updater
{
    friend class UpdateHandler;
public:
    static bool doUpdate(bool autoCheck);
    static bool startUpdater();

private:
    static bool checkForUpdate();
    static bool askForUpdate();
    static bool copyUpdater();
    static void showNotification();
    static QNetworkRequest getNetworkRequest(const QUrl &url);
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

class UpdateHandler : public QObject
{
    Q_OBJECT

    friend class Updater;
protected:
    UpdateHandler(QObject *parent);

    void createWatcher(const QFuture<bool>& f);

protected slots:
    void updateBtn();
    void updateCheckResult();

private:
    QFutureWatcher<bool> *m_watcher;
};

#endif // UPDATER_H
