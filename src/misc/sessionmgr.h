/**********************************************
**    This file is part of Lorris
**    http://tasssadar.github.com/Lorris/
**
**    See README and COPYING
***********************************************/

#ifndef SESSIONMGR_H
#define SESSIONMGR_H

#include <QFile>
#include <vector>
#include <QHash>

#include "ui_sessiondialog.h"
#include "ui_sessionsavedialog.h"

class QMenu;
class QSignalMapper;

class SessionMgr : public QObject
{
    Q_OBJECT
public:
    SessionMgr(QObject *parent);

    void initMenu(QMenu *menu);
    QByteArray openSession(const QString& name);
    QByteArray openSessionFilePath(const QString& path);

    void saveSession(QString name = tr("[Last session]"));

    const QStringList& getSessions() const { return m_sessions; }

    void startBatchOperation();
    void endBatchOperation();
    bool isBatchStarted() const { return m_batch_started; }
    void setBatchVar(const QString& name, const QVariant &var);
    QVariant getBatchVar(const QString& name, QVariant def = QVariant());

public slots:
    void saveSessionAct();
    void loadSession(QString name = QString(), bool closeOthers = false);
    void removeSession(QString name);

private slots:
    void openManager();
    void removeMenu(QObject *menu);

private:
    void updateSessions();
    QString getFolder();

    QStringList m_sessions;
    std::set<QMenu*> m_menus;
    QSignalMapper *m_sig_map;
    QHash<QString, QVariant> m_batch_vars;
    bool m_batch_started;
};

class SessionDialog : public QDialog, private Ui::SessionDialog
{
    Q_OBJECT
public:
    SessionDialog(SessionMgr *mgr, QWidget *parent = NULL);
    ~SessionDialog();

private slots:
    void on_sessionList_itemSelectionChanged();
    void on_sessionList_doubleClicked(QModelIndex idx);
    void on_addBtn_clicked();
    void on_removeBtn_clicked();
    void on_loadLastBox_clicked(bool checked);
    void on_autoConnBox_clicked(bool checked);

private:
    Ui::SessionDialog *ui;
    SessionMgr *m_mgr;
};

class SessionSaveDialog : public QDialog, private Ui::SessionSaveDialog
{
    Q_OBJECT
public:
    SessionSaveDialog(SessionMgr *mgr, QWidget *parent = NULL);
    ~SessionSaveDialog();

    static QString getSessionName(SessionMgr *mgr);

private slots:
    void on_nameBox_editTextChanged(const QString& text);

private:
    Ui::SessionSaveDialog *ui;
};

#endif // SESSIONMGR_H
