/**********************************************
**    This file is part of Lorris
**    http://tasssadar.github.com/Lorris/
**
**    See README and COPYING
***********************************************/

#include <QNetworkAccessManager>
#include <QNetworkRequest>
#include <QNetworkReply>
#include <QMessageBox>
#include <QProcess>
#include <QApplication>
#include <QDateTime>
#include <QFile>
#include <QPushButton>
#include <QDesktopWidget>
#include <QtConcurrentRun>
#include <QFuture>
#include <QFutureWatcher>

#include "updater.h"
#include "../revision.h"
#include "utils.h"
#include "config.h"
#include "../ui/tooltipwarn.h"

#define MANIFEST_URL "http://dl.dropbox.com/u/54372958/lorris.txt"

bool Updater::checkForUpdate(bool autoCheck)
{
    QNetworkAccessManager manager;
    QNetworkRequest request(QUrl(MANIFEST_URL));
    request.setRawHeader( "User-Agent", "Mozilla/5.0 (X11; U; Linux i686 (x86_64); "
                          "en-US; rv:1.9.0.1) Gecko/2008070206 Firefox/3.0.1" );
    request.setRawHeader( "Accept-Charset", "win1251,utf-8;q=0.7,*;q=0.7" );
    request.setRawHeader( "charset", "utf-8" );
    request.setRawHeader( "Connection", "keep-alive" );

    QNetworkReply *rep = manager.get(request);

    while(rep->error() == QNetworkReply::NoError && !rep->isFinished())
        QCoreApplication::processEvents();

    if(rep->isFinished() && rep->size() != 0)
    {
        QString ver(VERSION);
        for(QString s = rep->readLine(); !s.isNull(); s = rep->readLine())
        {
            QStringList parts = s.split(' ', QString::SkipEmptyParts);
            if(parts.size() < 3 || !ver.contains(parts[0]))
                continue;

            if(REVISION < parts[1].toInt())
                return true;
            return false;
        }
    }
    return false;
}

bool Updater::askForUpdate()
{
    UpdaterDialog d;
    return d.exec() == QDialog::Accepted;
}

bool Updater::doUpdate(bool autoCheck)
{
    quint32 time = QDateTime::currentDateTime().toTime_t();
    if(autoCheck)
    {
        if(!sConfig.get(CFG_BOOL_CHECK_FOR_UPDATE))
            return false;

        if(time < sConfig.get(CFG_QUINT32_LAST_UPDATE_CHECK)+300)
            return false;
    }

    sConfig.set(CFG_QUINT32_LAST_UPDATE_CHECK, time);

    bool update = false;
    if(autoCheck && !sConfig.get(CFG_BOOL_AUTO_UPDATE))
    {
        QFuture<bool> f = QtConcurrent::run(&Updater::checkForUpdate, autoCheck);
        UpdateHandler *h = new UpdateHandler(NULL);
        h->createWatcher(f);
        return false;
    }
    else
    {
        if(!checkForUpdate(autoCheck))
            return false;

        if(!autoCheck)
            update = askForUpdate();
        else
            update = sConfig.get(CFG_BOOL_AUTO_UPDATE);
    }

    return update && startUpdater();
}

bool Updater::copyUpdater()
{
    if(QFile::exists("tmp_updater.exe") && !QFile::remove("tmp_updater.exe"))
        return false;

    if(!QFile::copy("updater.exe", "tmp_updater.exe"))
        return false;
    return true;
}

bool Updater::startUpdater()
{
    if(!copyUpdater() ||
       !QProcess::startDetached("tmp_updater.exe", (QStringList() << VERSION << QString::number(REVISION))))
    {
        Utils::showErrorBox(QObject::tr("Could not start updater.exe, you have to download new version manually!\n"
                                 "<a href='http://tasssadar.github.com/Lorris'>http://tasssadar.github.com/Lorris</a>"));
        return false;
    }
    return true;
}

void Updater::showNotification()
{
    ToolTipWarn *w = new ToolTipWarn(QObject::tr("New update for Lorris is available"),
                                     NULL, NULL, -1, ":/actions/update");
    QPushButton *btn = new QPushButton(QObject::tr("Download"));
    w->setButton(btn);

    if(QDesktopWidget *desktop = qApp->desktop())
    {
        QRect rect = desktop->availableGeometry();
        w->move(rect.width() - w->width() - 90, rect.height() - w->height() - 15);
    }

    UpdateHandler *h = new UpdateHandler(w);
    QObject::connect(btn, SIGNAL(clicked()), h, SLOT(updateBtn()));
    QObject::connect(btn, SIGNAL(clicked()), w, SLOT(deleteLater()));
}

UpdaterDialog::UpdaterDialog(QWidget *parent) :
    QDialog(parent), ui(new Ui::UpdateCheck)
{
    ui->setupUi(this);

    ui->noAskBox->setChecked(sConfig.get(CFG_BOOL_AUTO_UPDATE));
    ui->noCheckBox->setChecked(!sConfig.get(CFG_BOOL_CHECK_FOR_UPDATE));
}

UpdaterDialog::~UpdaterDialog()
{
    delete ui;
}

void UpdaterDialog::on_noAskBox_clicked(bool checked)
{
    sConfig.set(CFG_BOOL_AUTO_UPDATE, checked);
}

void UpdaterDialog::on_noCheckBox_clicked(bool checked)
{
    sConfig.set(CFG_BOOL_CHECK_FOR_UPDATE, !checked);
}

UpdateHandler::UpdateHandler(QObject *parent) : QObject(parent)
{

}

void UpdateHandler::updateBtn()
{
    if(Updater::startUpdater())
        qApp->closeAllWindows();
}

void UpdateHandler::createWatcher(const QFuture<bool> &f)
{
    m_watcher = new QFutureWatcher<bool>(this);

    connect(m_watcher, SIGNAL(finished()), SLOT(updateCheckResult()));
    m_watcher->setFuture(f);
}

void UpdateHandler::updateCheckResult()
{
    if(m_watcher->result())
        Updater::showNotification();
    deleteLater();
}
