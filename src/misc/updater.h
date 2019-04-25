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
#include <QNetworkReply>

#include "../ui/tooltipwarn.h"
#include "qtobjectpointer.h"

enum
{
    RES_CHECK_FAILED     = 0,
    RES_NO_UPDATE,
    RES_UPDATE_AVAILABLE,

    RES_MAX
};

struct UpdateCheckResult {
    UpdateCheckResult() {
        UpdateCheckResult(0);
    }

    UpdateCheckResult(int code, QNetworkReply::NetworkError error = QNetworkReply::NetworkError::NoError, QString errorString = QString()) {
        this->code = code;
        this->error = error;
        this->errorString = errorString;
    }

    int code;
    QNetworkReply::NetworkError error;
    QString errorString;
};

class Updater
{
    friend class UpdateHandler;
public:
    static void checkForUpdate(bool autoCheck);
    static bool startUpdater();

private:
    static UpdateCheckResult checkManifest();
    static bool copyUpdater();
    static QNetworkRequest getNetworkRequest(const QUrl &url);
};

class UpdateHandler : public QObject
{
    Q_OBJECT

    friend class Updater;
protected:
    UpdateHandler(bool autoCheck, QObject *parent = NULL);

    void createWatcher(const QFuture<UpdateCheckResult>& f);

protected slots:
    void updateBtn();
    void updateCheckResult();

private:
    QFutureWatcher<UpdateCheckResult> *m_watcher;
    QtObjectPointer<ToolTipWarn> m_progress;
    bool m_autoCheck;
};

#endif // UPDATER_H
