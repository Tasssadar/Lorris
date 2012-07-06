/**********************************************
**    This file is part of Lorris
**    http://tasssadar.github.com/Lorris/
**
**    See README and COPYING
***********************************************/

#include <QDesktopServices>
#include <QFile>
#include <QMenu>
#include <QDir>
#include <QSignalMapper>
#include <QInputDialog>
#include <QMessageBox>

#include "sessionmgr.h"
#include "config.h"
#include "datafileparser.h"
#include "utils.h"
#include "../ui/tabview.h"
#include "../WorkTab/WorkTabMgr.h"

static const char CLDTA_DATA_MAGIC[] = { (char)0xFF, (char)0x80, 0x68 };

SessionMgr::SessionMgr(QObject *parent) : QObject(parent)
{
    m_menu = NULL;

    m_sig_map = new QSignalMapper(this);
    connect(m_sig_map, SIGNAL(mapped(QString)), SLOT(loadSession(QString)));

    updateSessions();
}

QByteArray SessionMgr::openSessionFile(const QString& name)
{
    static const QString path = QDesktopServices::storageLocation(QDesktopServices::DataLocation) + "/" + name;

    QFile file(path);
    if(!file.open(QIODevice::ReadOnly))
        return QByteArray();

    // check magic
    QByteArray magic = file.read(sizeof(CLDTA_DATA_MAGIC));
    if(magic.size() != sizeof(CLDTA_DATA_MAGIC))
        return QByteArray();

    for(quint8 i = 0; i < sizeof(CLDTA_DATA_MAGIC); ++i)
        if(magic[i] != CLDTA_DATA_MAGIC[i])
            return QByteArray();

    return file.readAll();
}

void SessionMgr::initMenu(QMenu *menu)
{
    m_menu = menu;

    if(m_sessions.empty())
    {
        QAction *empty = menu->addAction(tr("No saved sessions"));
        empty->setEnabled(false);
    }
    else
    {
        for(int i = 0; i < m_sessions.size(); ++i)
        {
            QAction *act = menu->addAction(m_sessions[i].remove(".cldta"));
            m_sig_map->setMapping(act, act->text());
            connect(act, SIGNAL(triggered()), m_sig_map, SLOT(map()));
        }
    }

    menu->addSeparator();

    QAction *saveAct = menu->addAction(tr("Save this session..."));
    QAction *managerAct = menu->addAction(tr("Session manager..."));

    connect(saveAct,    SIGNAL(triggered()), SLOT(saveSessionAct()));
    connect(managerAct, SIGNAL(triggered()), SLOT(openManager()));
}

void SessionMgr::openManager()
{
    SessionDialog d(this);
    d.exec();
}

void SessionMgr::saveSession(QString name)
{
    QFile file(QDesktopServices::storageLocation(QDesktopServices::DataLocation) + "/" + name + ".cldta");
    if(!file.open(QIODevice::WriteOnly | QIODevice::Truncate))
        return Utils::ThrowException(tr("Could not open session data file!"));

    QByteArray data;
    DataFileParser parser(&data);
    parser.open(QIODevice::WriteOnly);

    ((TabView*)parent())->saveData(&parser);

    parser.close();

    data = qCompress(data);

    file.write(CLDTA_DATA_MAGIC, sizeof(CLDTA_DATA_MAGIC));
    file.write(data);
}

void SessionMgr::saveSessionAct()
{
    QString name = QInputDialog::getText(NULL, tr("Session name"), tr("Enter new session name:"));
    if(name.isEmpty())
        return;

    name.replace(QRegExp("[/\\\\]"), "_");

    if(name.startsWith(".."))
        name.prepend("_");

    saveSession(name);
    Utils::printToStatusBar(tr("Session %1 saved.").arg(name));

    updateSessions();
}

void SessionMgr::loadSession(QString name)
{
    if(sWorkTabMgr.isAnyTabOpened())
    {
        QMessageBox box(QMessageBox::Question, tr("Load session"), tr("Tabs you've already opened will be closed. Proceed?"),
                       (QMessageBox::Yes | QMessageBox::No));

        if(box.exec() == QMessageBox::No)
            return;
    }

    if(!name.contains("cldta"))
        name.append(".cldta");

    QByteArray data = openSessionFile(name);
    if(data.isEmpty())
        return;
    data = qUncompress(data);
    DataFileParser parser(&data);
    parser.open(QIODevice::ReadOnly);

    ((TabView*)parent())->loadData(&parser);
}

void SessionMgr::updateSessions()
{
    QDir dir(QDesktopServices::storageLocation(QDesktopServices::DataLocation));
    m_sessions = dir.entryList((QStringList() << "*.cldta"));

    if(!m_menu)
        return;

    QAction *separator = NULL;
    QList<QAction*> actions = m_menu->actions();
    for(int i = 0; i < actions.size(); ++i)
    {
        if(actions[i]->isSeparator())
        {
            separator = actions[i];
            break;
        }
        delete actions[i];
    }

    if(m_sessions.empty())
    {
        QAction *empty = new QAction(tr("No saved sessions"), this);
        m_menu->insertAction(separator, empty);
        empty->setEnabled(false);
    }
    else
    {
        for(int i = 0; i < m_sessions.size(); ++i)
        {
            QAction *act = new QAction(m_sessions[i].remove(".cldta"), this);
            m_menu->insertAction(separator, act);
            m_sig_map->setMapping(act, act->text());
            connect(act, SIGNAL(triggered()), m_sig_map, SLOT(map()));
        }
    }
}

void SessionMgr::removeSession(QString name)
{
    if(!name.contains("cldta"))
        name.append(".cldta");

    QFile::remove(QDesktopServices::storageLocation(QDesktopServices::DataLocation) + "/" + name);
    updateSessions();
}

SessionDialog::SessionDialog(SessionMgr *mgr, QWidget *parent) :
    QDialog(parent), ui(new Ui::SessionDialog)
{
    ui->setupUi(this);

    m_mgr = mgr;

    ui->sessionList->addItems(m_mgr->getSessions());
    ui->loadLastBox->setChecked(sConfig.get(CFG_BOOL_LOAD_LAST_SESSION));
}

SessionDialog::~SessionDialog()
{
    delete ui;
}

void SessionDialog::on_sessionList_itemSelectionChanged()
{
    ui->removeBtn->setEnabled(!ui->sessionList->selectedItems().isEmpty());
}

void SessionDialog::on_sessionList_doubleClicked(QModelIndex idx)
{
    m_mgr->loadSession(idx.data().toString());
    close();
}

void SessionDialog::on_addBtn_clicked()
{
    m_mgr->saveSessionAct();

    ui->sessionList->clear();
    ui->sessionList->addItems(m_mgr->getSessions());
}

void SessionDialog::on_removeBtn_clicked()
{
    if(ui->sessionList->selectedItems().isEmpty())
        return;

    QString name = ui->sessionList->selectedItems().front()->text();
    m_mgr->removeSession(name);

    ui->sessionList->clear();
    ui->sessionList->addItems(m_mgr->getSessions());
}

void SessionDialog::on_loadLastBox_clicked(bool checked)
{
    sConfig.set(CFG_BOOL_LOAD_LAST_SESSION, checked);
}
