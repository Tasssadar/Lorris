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
#include "../WorkTab/WorkTabMgr.h"

#define MANIFEST_URL "http://tasemnice.eu/lorris/updater_manifest.txt"

QNetworkRequest Updater::getNetworkRequest(const QUrl& url)
{
    QNetworkRequest req(url);
    req.setRawHeader( "User-Agent", "Mozilla/5.0 (X11; U; Linux i686 (x86_64); "
                          "en-US; rv:1.9.0.1) Gecko/2008070206 Firefox/3.0.1" );
    req.setRawHeader( "Accept-Charset", "win1251,utf-8;q=0.7,*;q=0.7" );
    req.setRawHeader( "charset", "utf-8" );
    req.setRawHeader( "Connection", "keep-alive" );
    return req;
}

int Updater::checkManifest()
{
    QUrl baseUrl(MANIFEST_URL);
    QNetworkAccessManager manager;
    QNetworkRequest request = getNetworkRequest(baseUrl);
    QNetworkReply *rep = manager.get(request);

    while(rep->error() == QNetworkReply::NoError && !rep->isFinished())
        QCoreApplication::processEvents();

    while(true)
    {
        while(!rep->isFinished())
            QCoreApplication::processEvents();

        if(rep->error() != QNetworkReply::NoError)
            return RES_CHECK_FAILED;

        QVariant redirect = rep->attribute(QNetworkRequest::RedirectionTargetAttribute);
        if(redirect.type() != QVariant::Url)
            break;

        // redirect
        baseUrl = baseUrl.resolved(redirect.toUrl());
        request = getNetworkRequest(baseUrl);
        rep = manager.get(request);
    }

    if(rep->isFinished() && rep->size() != 0)
    {
        QString s;
        QString ver(VERSION);
        while(!rep->atEnd())
        {
            s = rep->readLine();

            QStringList parts = s.split(' ', QString::SkipEmptyParts);
            if(parts.size() < 3 || !ver.contains(parts[0]))
                continue;

            if(REVISION < parts[1].toInt())
                return RES_UPDATE_AVAILABLE;
            else
                return RES_NO_UPDATE;
        }
    }
    return RES_NO_UPDATE;
}

void Updater::checkForUpdate(bool autoCheck)
{
    quint32 time = QDateTime::currentDateTime().toTime_t();
    if(autoCheck)
    {
        if(!sConfig.get(CFG_BOOL_CHECK_FOR_UPDATE))
            return;

        if(time < sConfig.get(CFG_QUINT32_LAST_UPDATE_CHECK)+300)
            return;
    }

    sConfig.set(CFG_QUINT32_LAST_UPDATE_CHECK, time);

    UpdateHandler *h = new UpdateHandler(autoCheck);
    QFuture<int> f = QtConcurrent::run(&Updater::checkManifest);
    h->createWatcher(f);
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

UpdateHandler::UpdateHandler(bool autoCheck, QObject *parent) : QObject(parent)
{
    m_autoCheck = autoCheck;
    if(!autoCheck)
    {
        sWorkTabMgr.printToAllStatusBars(tr("Checking for updates..."), 0);
        m_progress = new ToolTipWarn(tr("Checking for updates..."), NULL, NULL, -1);
        m_progress->showSpinner();
        m_progress->toRightBottom();
    }
}

void UpdateHandler::updateBtn()
{
    QPushButton *btn = dynamic_cast<QPushButton*>(sender());
    if(btn) {
        btn->setEnabled(false);
        btn->setText(tr("Starting..."));
        qApp->processEvents();
    }

    if(Updater::startUpdater())
        qApp->closeAllWindows();
    deleteLater();
}

void UpdateHandler::createWatcher(const QFuture<int> &f)
{
    m_watcher = new QFutureWatcher<int>(this);

    connect(m_watcher, SIGNAL(finished()), SLOT(updateCheckResult()));
    m_watcher->setFuture(f);
}

void UpdateHandler::updateCheckResult()
{
    if(!m_progress.isNull())
        m_progress->deleteLater();

    int res = m_watcher->result();
    if(m_autoCheck && (res == RES_CHECK_FAILED || res == RES_NO_UPDATE))
    {
        deleteLater();
        return;
    }

    static const QString texts[] = {
        tr("Update check has failed!"),
        tr("No update available"),
        tr("New update for Lorris is available")
    };

    switch(res)
    {
        case RES_CHECK_FAILED:
        {
            sWorkTabMgr.printToAllStatusBars(texts[res]);
            ToolTipWarn *w = new ToolTipWarn(texts[res], NULL, NULL, 4000, ":/icons/warning");
            w->toRightBottom();
            deleteLater();
            break;
        }
        case RES_NO_UPDATE:
        {
            sWorkTabMgr.printToAllStatusBars(texts[res]);
            ToolTipWarn *w = new ToolTipWarn(texts[res], NULL, NULL, 3000, ":/actions/info");
            w->toRightBottom();
            deleteLater();
            break;
        }
        case RES_UPDATE_AVAILABLE:
        {
            sWorkTabMgr.printToAllStatusBars(texts[res]);
            ToolTipWarn *w = new ToolTipWarn(texts[res], NULL, NULL, 30000, ":/actions/update");
            QPushButton *btn = new QPushButton(tr("Download"));
            w->setButton(btn);
            w->toRightBottom();

            connect(btn, SIGNAL(clicked()), this, SLOT(updateBtn()));
            connect(btn, SIGNAL(clicked()), w, SLOT(deleteLater()));
            break;
        }
        default:
        {
            sWorkTabMgr.printToAllStatusBars("");
            deleteLater();
            break;
        }
    }
}


