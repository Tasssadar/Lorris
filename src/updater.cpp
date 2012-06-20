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

#include "updater.h"
#include "revision.h"
#include "utils.h"

bool Updater::checkForUpdate()
{
#ifndef Q_OS_WIN
    QNetworkAccessManager manager;
    //  if(manager.networkAccessible() != QNetworkAccessManager::Accessible)
    //        return false;

    QNetworkRequest request(QUrl("http://dl.dropbox.com/u/54372958/lorris.txt"));
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
            if(parts.size() < 3)
                continue;

            if(!ver.contains(parts[0]))
                continue;

            qDebug("found %s!", parts[0].toStdString().c_str());
            if(REVISION < parts[1].toInt())
            {
                qDebug("%u < %u", REVISION, parts[1].toUInt());
                return askForUpdate();
            }
            else
            {
                qDebug("%u > %u", REVISION, parts[1].toUInt());
                return false;
            }
        }
    }
#endif
    return false;
}

bool Updater::askForUpdate()
{
    QMessageBox box(QMessageBox::Question, QObject::tr("Update available"),
                    QObject::tr("New version of Lorris is available. Should I update to this version?"));
    box.addButton(QMessageBox::Yes);
    box.addButton(QMessageBox::No);
    return box.exec() == QMessageBox::Yes;
}

bool Updater::doUpdate()
{
    if(!checkForUpdate())
        return false;

    if(!QProcess::startDetached("updater.exe", (QStringList() << VERSION << QString::number(REVISION))))
    {
        Utils::ThrowException(tr("Could not start updater.exe, you have to download new version manually!\n"
                                 "<a href='http://tasssadar.github.com/Lorris'>http://tasssadar.github.com/Lorris</a>"));
        return false;
    }
    else
    {
        QCoreApplication::exit();
        return true;
    }
}
